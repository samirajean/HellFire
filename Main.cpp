#include <GL\freeglut.h>
#include <FreeImage.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <md2.h>
#include <algorithm>



CMD2Model model1;

float animationTime = 0;

using namespace std;

#pragma comment(lib, "freeimage.lib")

GLfloat ambientLight[] = {0.5f, 0.5f, 1.0f, 0.5f }; //refliction
GLfloat diffuseLight[] = {1.0f, 1.0f, 1.0f, 1.0f }; //brightness' color
GLfloat lightPosition[] = { 0.0f, 0.0f, 2.0f, 0.0f }; //the position of light

GLfloat ambientLightr[] = {1.0f, 1.0f, 1.0f,0.0f};
GLfloat diffuseLightr[] = {1.0f, 0.5f, 0.0f, 1.0f };
GLfloat lightPositionr[] = { 0.0f, 0.0f, 2.0f, 0.0f };
bool isdown[256];



GLuint terrainTexture;
GLuint backTexture;
GLuint roofTexture;
GLuint lrTexture;

float cameraX = 0, cameraZ = 200;
float cameraY = 45;
float cameraAngle = 0;
float headingX = 0, headingZ = -1;

float s = 0.3;

GLuint loadTexture( char* filename, bool isInverted )
{
	GLuint texture;
	FIBITMAP *image = FreeImage_Load( FreeImage_GetFileType(filename, 0), filename);
	FIBITMAP *fImage = FreeImage_ConvertTo32Bits( image );

	int width = FreeImage_GetWidth( fImage );
	int height = FreeImage_GetHeight( fImage );

	if(isInverted)
		FreeImage_FlipVertical(fImage);

	glGenTextures(1, &texture);

	glBindTexture( GL_TEXTURE_2D, texture);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(fImage) );  

		return texture;
}

/*GLuint loadTextures( char* location  )
{
	GLuint texture;
	FIBITMAP *image = FreeImage_Load(FreeImage_GetFileType(location, 0), location);
	FIBITMAP *fImage = FreeImage_ConvertTo32Bits(image);
	int width = FreeImage_GetWidth(fImage);
	int height = FreeImage_GetHeight(fImage);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(fImage));
	FreeImage_Unload(image);
	FreeImage_Unload(fImage);


	return texture;
}*/


class FireballEngine
{
	struct fireball
	{
		GLuint texture;
		float x, y, z; //position 
		float xr, yr, zr; //rotation vertix
		float rotVel;//velocity
		float rotAng; // angel
		float gravity; //-0.8
		float fading; //Random (remoal) 
		float lifetime; // if 0, flake is removed 
		bool isAlive; //if the flake is alive or not
		float speed; // initial speed
		float scale; // size of flake
	};
	struct fireballRemaining //for remaining
	{
		float x, y, z;
		float lifetime;
		float fading;
		float scale;

		fireballRemaining()
		{
			fading = 0.01; //initial value
		}
	};

	fireball *flakes; //pointer 3shan el user hy7dd el 3dd
	int count; //3ddhom
	int skyLevel, groundLevel;
	int regionSize;
	float windX, windZ; //to make it more realistic
	char fireballTexturesLocation[3][50]; //location of textures
	GLuint fireballTextures[3]; //actual textures
	vector<fireballRemaining> remainings; //vector 3shan hya random f ana m3rfsh eih el flakes eli hatnzel w eih 3adadhom f wa2t mo3ayan

public:

	FireballEngine(int max, int sky, int ground, int region)
	{
		strcpy_s(fireballTexturesLocation[0], "fireball0.jpg");
		strcpy_s(fireballTexturesLocation[1], "fireball1.jpg");
		strcpy_s(fireballTexturesLocation[2], "fireball2.jpg");

		count = max; 
		skyLevel = sky;
		groundLevel = ground;
		regionSize = region;
		flakes = new fireball[max]; //dinamic allocation
		windX = -0.05f; //left
		windZ = 0;
	}

	void initializeAFlake(int i) 
	{
		flakes[i].y = skyLevel; 
		flakes[i].x = rand() % (regionSize+1) - regionSize/2.0f; //normalization
		flakes[i].z = rand() % (regionSize + 1) - regionSize / 2.0f;
		flakes[i].scale = (rand() % 50) / 50.0f + 0.5f; //I don't want the scale to be zero

		flakes[i].xr = (rand() % 1000) / 1000.0f; // if %1000 is increased the accuracy will increase 
		flakes[i].yr = (rand() % 1000) / 1000.0f; 
		flakes[i].zr = (rand() % 1000) / 1000.0f;
		flakes[i].rotVel = (rand() % 1000) / 1000.0f;
		flakes[i].rotAng = 0;


		int textureIndex = rand() % 3; ///choose testures randomly
		flakes[i].texture = fireballTextures[textureIndex];

		flakes[i].isAlive = true;
		flakes[i].lifetime = 1.0f;
		flakes[i].fading = (rand() % 10) / 1000.0f + 0.0001; //random value
		flakes[i].gravity = -0.8f;
		flakes[i].speed = 0.0f;

	}
	void initialize()
	{
		for (int i = 0; i < 3; i++)
			fireballTextures[i] = loadTexture(fireballTexturesLocation[i],false); 
		for (int i = 0; i < count; i++)
			initializeAFlake(i);
	}
	void update()
	{
		for (int i = 0; i < count; i++)
		{
			if (flakes[i].isAlive) 
			{
				flakes[i].y += flakes[i].speed / 500.0f; ///speed/vector
				flakes[i].x += windX * (rand() % 5); //3shan akhleha tnzel randomly
				flakes[i].speed += flakes[i].gravity; //speed + gravity 
				flakes[i].rotAng += flakes[i].rotVel;

				flakes[i].lifetime -= flakes[i].fading;
				if (flakes[i].y <= groundLevel || flakes[i].lifetime <= 0)
				{
					flakes[i].isAlive = false;
					if (flakes[i].y <= groundLevel)
					{
						fireballRemaining r; 
						r.x = flakes[i].x;
						r.z = flakes[i].z; 
						
						r.y = groundLevel+0.1f; //3shan tb2a marfo3a fo2
						r.scale = flakes[i].scale;//lazem tkon nfs 7agmaha fo2
						r.lifetime = flakes[i].scale * 10;	//give a lifetime based on scale (flake physical size) elkbeer hy3od fatra akbar 3al ard
						remainings.push_back(r);
					}
				}
			}
			else
			{
				initializeAFlake(i);
			}

		}

		//update remainings and remove dead ones
		vector<fireballRemaining>::iterator it = remainings.begin(); //linked list, iteraror ymshy 3ala remainings
		while (it != remainings.end())
		{
			it->lifetime -= it->fading; //yin2s el live time mn el flake eli mwgoda 3al ard
			it++;
		}
		remainings.erase(remove_if(remainings.begin(), remainings.end(), [](fireballRemaining r) { return r.lifetime<=0?true:false; }), remainings.end()); 
		//algorithm built in; starting point, ending, func to return true or false, remove if btraga3ly el 7aga eli kant true...erase hya eli btmsa7 w m7taga el location
	}
	void render()
	{
		for (int i = 0; i < count; i++)
		{
			if (flakes[i].isAlive)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE); // kol 7aga lonha esswed yib2a transparent
				glEnable(GL_BLEND);
				glColor4f(1, 1, 1, 1);
				
				glPushMatrix();
				glScalef(flakes[i].scale, flakes[i].scale, flakes[i].scale);
				glTranslatef(flakes[i].x, flakes[i].y, flakes[i].z);
				glRotatef(flakes[i].rotAng, flakes[i].xr, flakes[i].yr, flakes[i].zr);

				glBindTexture(GL_TEXTURE_2D, flakes[i].texture);
				glBegin(GL_QUADS);
				glTexCoord2d(0, 0); glVertex3f(-1, -1, 0);
				glTexCoord2d(0, 1); glVertex3f(-1, 1, 0);
				glTexCoord2d(1, 1); glVertex3f(1, 1, 0);
				glTexCoord2d(1, 0); glVertex3f(1, -1, 0);
				glEnd();
				glPopMatrix();
				glDisable(GL_BLEND);
			}
		}
		for (int i = 0; i < remainings.size(); i++)
		{
			glDisable(GL_TEXTURE_2D);
			glPushMatrix();
			glTranslatef(remainings[i].x, remainings[i].y, remainings[i].z);
			glScalef(remainings[i].scale, remainings[i].scale, remainings[i].scale);
			glColor3f(1.0f, 0.5f, 0.0f);
			glutSolidSphere(0.5f, 10, 10);
			glPopMatrix();
			glEnable(GL_TEXTURE_2D);

		}

	}
};

FireballEngine se(1500, 250, 0, 500);


void reshape( int width, int height )
{
	if(height == 0 )
		height = 1;
	float aspectRatio = width/height;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glViewport(0,0,width,height);
	gluPerspective( 45, aspectRatio, 1, 1000);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();



}



void render()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glLoadIdentity();
	gluLookAt(cameraX, cameraY, cameraZ, cameraX+headingX, cameraY, cameraZ+headingZ, 0, 1, 0);

	//Draw Ground on y = 0
	glBindTexture(GL_TEXTURE_2D, terrainTexture);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-500, 0, 500);
	glTexCoord2f(1, 0); glVertex3f(500, 0, 500);
	glTexCoord2f(1, 1); glVertex3f(500, 0, -500);
	glTexCoord2f(0, 1); glVertex3f(-500, 0, -500);

	glEnd();
	glPopMatrix();

	//left
	glBindTexture(GL_TEXTURE_2D, lrTexture);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-500, -500, 500);
	glTexCoord2f(1, 0); glVertex3f(-500, 500, 500);
	glTexCoord2f(1, 1); glVertex3f(-500, 500, -500);
	glTexCoord2f(0, 1); glVertex3f(-500, -500, -500);
	glEnd();
	glPopMatrix();

	//right
	glBindTexture(GL_TEXTURE_2D, lrTexture);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(500, -500, 500);
	glTexCoord2f(1, 0); glVertex3f(500, 500, 500);
	glTexCoord2f(1, 1); glVertex3f(500, 500, -500);
	glTexCoord2f(0, 1); glVertex3f(500, -500, -500);
	glEnd();
	glPopMatrix();

	//background
	glBindTexture(GL_TEXTURE_2D, backTexture);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-500, 500, -500);
	glTexCoord2f(1, 0); glVertex3f(500, 500, -500);
	glTexCoord2f(1, 1); glVertex3f(500, -500, -500);
	glTexCoord2f(0, 1); glVertex3f(-500, -500, -500);
	glEnd();
	glPopMatrix();

	//roof
	glBindTexture(GL_TEXTURE_2D, roofTexture);
	glPushMatrix();
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-500, 500, 500);
	glTexCoord2f(1, 0); glVertex3f(500, 500, 500);
	glTexCoord2f(1, 1); glVertex3f(500, 500, -500);
	glTexCoord2f(0, 1); glVertex3f(-500, 500, -500);
	glEnd();
	glPopMatrix();

	

	

	//Render Snow Engine
	se.render();
	
	
	glPushMatrix();
	model1.DrawModel(animationTime+=0.01);
	glPopMatrix();
	
	glutSwapBuffers();
}

void myKeyboardFunc( unsigned char k, int x, int y)
{
	
	if(k == 'a')
			model1.SetAnim(1);

	if(k == 'd')
	{
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	}

	if(k == 'r')
	{
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLightr);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLightr);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPositionr);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	}

	if(k == 'l')
	{

	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHTING);
	}

	if(k == 'z')
		s += 0.02;
	model1.ScaleModel(s);

	if(k == 's')
		s -= 0.02;
	model1.ScaleModel(s);
	if(k == 'n')
		model1.ScaleModel(0.3);
}

/*void mySpecialFunc( int k, int x, int y )
{
	if(k == GLUT_KEY_LEFT)
	{
		cameraAngle -= 0.4;
		headingX = sin(cameraAngle);
		headingZ = -cos( cameraAngle );
	
	}
	if(k == GLUT_KEY_RIGHT )
	{
		cameraAngle += 0.4;
		headingX = sin(cameraAngle);
		headingZ = -cos( cameraAngle );
	}
	if(k == GLUT_KEY_UP)
	{
		cameraZ += headingZ * 0.8;
		cameraX += headingX * 0.8;
	}
	if(k == GLUT_KEY_DOWN)
	{
		cameraZ -= headingZ * 0.8;
		cameraX -= headingX * 0.8; 
	}

	if(k == GLUT_KEY_PAGE_UP)
	{
		cameraY += 1;
	}
	if(k == GLUT_KEY_PAGE_DOWN)
	{
		cameraY -= 1;
	}

}*/

void keyboardControl()
{
	for(int i = 30; i < 69; ++i)
		isdown[i] = GetAsyncKeyState(i);

	if(isdown[VK_DOWN])
	{
		cameraZ -= headingZ * 0.8;
		cameraX -= headingX * 0.8; 
	}

	if(isdown[VK_UP])
	{
		cameraZ += headingZ * 0.8;
		cameraX += headingX * 0.8;
	}

	if(isdown[VK_RIGHT])
	{
		cameraAngle += 0.4;
		headingX = sin(cameraAngle);
		headingZ = -cos( cameraAngle );
	}

	if(isdown[VK_LEFT])
	{
		cameraAngle -= 0.4;
		headingX = sin(cameraAngle);
		headingZ = -cos( cameraAngle );
	}

	if(isdown[VK_PRIOR])
	{
		cameraY += 1;
	}

	if(isdown[VK_NEXT])
	{
		cameraY -= 1;
	}

}

void timerFunc( int time )
{
	glutTimerFunc( 40, timerFunc, 40);
	keyboardControl();
	se.update();
	glutPostRedisplay();
}

int main( int argc, char* argv[] )
{
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition(100,100);
	glutInitWindowSize(500, 500);

	glutCreateWindow("Hell");

	glutTimerFunc(40, timerFunc, 40);
	glutDisplayFunc( render );
	glutReshapeFunc( reshape );

	glutKeyboardFunc( myKeyboardFunc );
	//glutSpecialFunc( mySpecialFunc ); //it only detects if the button is pushed 	

	//init
	glEnable( GL_DEPTH_TEST );
	
	

	glClearColor(0, 0, 0.2, 1);
	
	glEnable( GL_DEPTH_TEST );
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);
	
	
	
	glEnable(GL_TEXTURE_2D);

	terrainTexture = loadTexture("lava.jpg",true);
	backTexture = loadTexture("hell_st.jpg",false);
	roofTexture = loadTexture("bloodyfire.png",true);
	lrTexture = loadTexture("hell_st1.jpg",true);

	//Initialize Snow Engine
	se.initialize();

	model1.LoadModel("Cyber.md2");
	model1.LoadSkin("cyber.jpg");
	model1.ScaleModel(s);
	model1.SetAnim(0);


	
	

	glutMainLoop();


	return 0;
}