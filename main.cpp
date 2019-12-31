
#include "main.h"

CLoad3DS g_Load3ds;									
t3DModel g_3DModel;
int   g_ViewMode= GL_TRIANGLES;
bool  g_bLighting= true;

//全局变量
int mButton = -1;  
int mOldY, mOldX; 

//旋转、平移向量  
float eye[3] = {0.0f, 0.0f, 7.0f};  
float rot[3] = {45.0f, 45.0f, 0.0f}; 

UINT g_Texture[MAX_TEXTURES] = {0};	

//是否启用线框模式
int wireframe = 0;

//创建纹理
void CreateTexture(UINT textureArray[], LPSTR strFileName, int textureID) {
	AUX_RGBImageRec *pBitmap = NULL;
	
	if(!strFileName) // 如果无此文件，则直接返回
		return;

	pBitmap = auxDIBImageLoad(strFileName);	// 装入位图，并保存数据
	
	if(pBitmap == NULL)	// 如果装入位图失败，则退出
		exit(0);

	// 生成纹理
	glGenTextures(1, &textureArray[textureID]);

	// 设置像素对齐格式
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pBitmap->sizeX, pBitmap->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pBitmap->data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);

	if (pBitmap) { // 释放位图占用的资源
		if (pBitmap->data)						
			free(pBitmap->data);				
		free(pBitmap);					
	}
} 

 
//窗口调整
void glutResize(int width, int height) {  
	if (height==0) {// 防止被零除							
		height=1; // 将Height设为1						
	}
	
	// 重置当前的视口。前两个参数定义了视口左下角（0,0表示最左下方），后两个参数分别是宽度和高度				
	glViewport(0,0,width,height); 

	//透视图设置
	glMatrixMode(GL_PROJECTION);// 选择投影矩阵
    glLoadIdentity(); // 重置投影矩阵	
	gluPerspective(10.0f,(GLfloat)width/(GLfloat)height, 0.5f ,150.0f);// 设置视口的大小

	glMatrixMode(GL_MODELVIEW);	// 选择模型观察矩阵					
	glLoadIdentity();			// 重置模型观察矩阵		 
}  
  
//接收键盘输入：ESC-退出、W/w-线框模型转换
void glutKeyboard(unsigned char key, int x, int y) {  
    switch (key) {  
        case ESC:
			exit(0);  
        case 'W':  
        case 'w':  
            wireframe = !wireframe;  
    }  
}  
  
//旋转角度超出-360°~360°，则置0
void clamp(float *v) {  
    int i;  
    for (i = 0; i <3; i ++)  
        if (v[i] > 360 || v[i]<-360)  
            v[i] = 0;  
}  
  
//根据鼠标按键，变换视角
void glutMotion(int x, int y) {  
    if (mButton == BUTTON_LEFT) {  
		//旋转：x、y值判断
        rot[0] -= (mOldY - y)*0.5f;  
        rot[1] -= (mOldX - x)*0.5f;  
        clamp (rot);  
    }  
    else if (mButton == BUTTON_RIGHT) {  
        //缩放：y值判断
        eye[2] -= (mOldY - y) * 0.2f; // 乘以一个系数来控制缩放速度
       // clamp (rot);  
    }   
    else if (mButton == BUTTON_LEFT_TRANSLATE) {  
		//平移：x、y值判断
        eye[0] += (mOldX - x) * 0.05f;  
        eye[1] -= (mOldY - y) * 0.05f;  
      //  clamp (rot);  
    }  
  
    mOldX = x;  
    mOldY = y;  
}  
  
//接收鼠标动作
void glutMouse(int button, int state, int x, int y) {  
    if(state == GLUT_DOWN) {//鼠标按下
        mOldX = x;  
        mOldY = y;  
        switch(button) {  
            case GLUT_LEFT_BUTTON: //左键按下
                if (glutGetModifiers() == GLUT_ACTIVE_CTRL) { //是否修饰键CTRL同时按下
                   mButton = BUTTON_LEFT_TRANSLATE;  
                   break;  
                } else {  
                   mButton = BUTTON_LEFT;  
                   break;  
                }  
            case GLUT_RIGHT_BUTTON://右键按下
                mButton = BUTTON_RIGHT;  
                break;  
        }  
    } else if (state == GLUT_UP)  
		mButton = -1;  
}  
   
//绘制
void glutDisplay(void) {  

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
  	glLoadIdentity(); // 重置当前的模型观察矩阵
    gluLookAt(0, 1.5f, 80, 0, 0.5f, -80, 0, 1, 0);// 场景（0，0.5，0）的视点中心 (0,1.5,8)
    
	if (wireframe)  
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
    else  
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  

	glPushMatrix();  
  
    //将camera平移到得到的视角坐标  
    glTranslatef (-eye[0], -eye[1], -eye[2]);  
  
    //用得到的旋转向量旋转视角  
    glRotatef(rot[0], 1.0f, 0.0f, 0.0f);  
    glRotatef(rot[1], 0.0f, 1.0f, 0.0f);  
    glRotatef(rot[2], 0.0f, 0.0f, 1.0f);  
  
    // 遍历模型中所有的对象
	for(int i = 0; i < g_3DModel.numOfObjects; i++) {
		// 如果对象的大小小于0，则退出
		if (g_3DModel.vctObject.size() <= 0)
			break;

		// 获得当前显示的对象
		t3DObject *pObject = &g_3DModel.vctObject[i];
			
		// 判断该对象是否有纹理映射
		if (pObject->bHasTexture) {
			glEnable(GL_TEXTURE_2D);// 打开纹理映射
			glColor3ub(255, 255, 255);
			glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
		} else {
			glDisable(GL_TEXTURE_2D);// 关闭纹理映射
			//glColor3ub(255, 255, 255);
		}
		// 开始以g_ViewMode模式绘制
		glBegin(g_ViewMode);					
			// 遍历所有的面
		for(int j = 0; j < pObject->numOfFaces; j++) {// 遍历三角形的所有点
			for(int whichVertex = 0; whichVertex < 3; whichVertex++) {// 获得面对每个点的索引
				
				int index = pObject->pFaces[j].vertIndex[whichVertex];
			
				// 给出法向量
				glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);
				
				// 如果对象具有纹理
				if (pObject->bHasTexture) {
					// 确定是否有UVW纹理坐标
					if (pObject->pTexVerts) {
						glTexCoord2f(pObject->pTexVerts[ index ].x, pObject->pTexVerts[ index ].y);
					}
				} else {
					if (g_3DModel.vctMaterials.size() && pObject->materialID >= 0) {
						BYTE *pColor = g_3DModel.vctMaterials[pObject->materialID].color;
						glColor3ub(pColor[0], pColor[1], pColor[2]);
						//printf("1=%uc,2=%uc,3=%uc",pColor[0],pColor[1],pColor[2]);
					}
				}
				glVertex3f(pObject->pVerts[ index ].x, pObject->pVerts[ index ].y, pObject->pVerts[ index ].z);
			}
		}

		glEnd();// 绘制结束
	}

    glFlush();  
    glutSwapBuffers();
}  
  
//纹理与光照
void InitializeOGL() { 
	glClearColor(1.0,1.0,1.0,0.0);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);  
    glShadeModel(GL_SMOOTH);  
    glEnable(GL_DEPTH_TEST);  
   
    //打开纹理，并绑定到矩阵的第一个元素   
    glEnable(GL_TEXTURE_2D);     
	g_Load3ds.Import3DS(&g_3DModel, FILE_NAME);	// 将3ds文件装入到模型结构体中
	//printf("numOfMaterials=%d ",g_3DModel.numOfMaterials);
	
	// 遍历所有的材质
	for (int i = 0; i < g_3DModel.numOfMaterials; i++) {	
		//printf("strFile=%s ",g_3DModel.vctMaterials[i].strFile);
		
		if(strlen(g_3DModel.vctMaterials[i].strFile) > 0) {
			//使用纹理文件名称来装入位图
			CreateTexture(g_Texture,g_3DModel.vctMaterials[i].strFile , i);		
		}

		// 设置材质的纹理ID
		g_3DModel.vctMaterials[i].texureId = i;
	}

	//设置灯光
	
	//GL_COLOR_MATERIAL用颜色来映射纹理。否则纹理将始终保持原来的颜色，glColor3f(r,g,b)也不会起作用。
	glEnable(GL_CULL_FACE);
	GLfloat ambientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat lightPos[] = {0.0f, 0.0f, 0.0f, 1.0f};	
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glEnable(GL_LIGHT0);		// 使用默认的0号灯					
	glEnable(GL_LIGHTING);			// 使用灯光					
	glEnable(GL_COLOR_MATERIAL);	// 使用颜色材质			
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight); 
}  


int main(int argc, char** argv)  
{  
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );  
    glutInitWindowPosition( GL_WIN_INITIAL_X, GL_WIN_INITIAL_Y );  
    glutInitWindowSize( GL_WIN_WIDTH, GL_WIN_HEIGHT );  
    glutInit( &argc, argv );  
    glutCreateWindow("左键控制旋转，右键上下控制缩放，CTRL+左键控制平移");  
    glutReshapeFunc(glutResize);       // called every time  the screen is resized   
    glutDisplayFunc(glutDisplay);      // called when window needs to be redisplayed   
    glutIdleFunc(glutDisplay);         // called whenever the application is idle   
    glutKeyboardFunc(glutKeyboard);    // called when the application receives a input from the keyboard   
    glutMouseFunc(glutMouse);          // called when the application receives a input from the mouse   
    glutMotionFunc(glutMotion);        // called when the mouse moves over the screen with one of this button pressed   
    InitializeOGL();  
  
    glutMainLoop();  
  
    return 0;  
}  
