/*
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
  Jerry Sivaram Reddy Rajasimha Reddy
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>

GLboolean stop = false;

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;
Pic * textureData;

//Main code
int polygon_state = 0;
bool triangle_map = false;

int width;
int height;
float* vertices = 0;
int* indices = 0;
float* color = 0;

char * screenshot_name = NULL;

int half;

//Get total count of the vertices for the traingular strips
int getVerticesCount( int width, int height ) {
  return 3 * (width * height);
}

//Get total number of the indices required for the heightmap
int getIndicesCount( int width, int height ) {
  return ((width*height) + (width-1)*(height-2));
}

//Store the vertices coordinates
float* getVertices( int width, int height ) {
    
    half = height/2;
    
    if ( vertices ) return vertices;

    vertices = new float[ getVerticesCount( width, height ) ];
    int i = 0;

    for ( int row=0; row<height; row++ ) {
        for ( int col=0; col<width; col++ ) {
            vertices[i++] = (float) col - half;
            //printf("%f,",(float)PIC_PIXEL(g_pHeightData,row,col,0));
            vertices[i++] = (float) PIC_PIXEL(g_pHeightData,col,row,0)/16;
            //vertices[i++] = 0.0f;
            vertices[i++] = (float) row - half;
        }
    }

    return vertices;
}

//Store the indices
int* getIndices( int width, int height ) {
    if ( indices ) return indices;

    indices = new int[ getVerticesCount( width, height ) ];
    int i = 0;

    for ( int row=0; row<height-1; row++ ) {
        if ( (row&1)==0 ) { //even rows
            for ( int col=0; col<width; col++ ) {
                indices[i++] = col + row * width;
                indices[i++] = col + (row+1) * width;
            }
        } else { //odd rows
            for ( int col=width-1; col>0; col-- ) {
                indices[i++] = col + (row+1) * width;
                indices[i++] = col - 1 + row * width;
            }
        }
    }

    return indices;
}

//Store the color for the heightmap primitives
float* getColor( int width, int height ){
  if ( color ) return color;
  
  color = new float[ getVerticesCount( width, height ) ];
  int i = 0;
  
  Pic * texture;
  
  if(textureData) {
    texture = textureData; // Check whether the texture data is available
  }
  else texture = g_pHeightData;

  for ( int row=0; row<height; row++ ) {
    
    width = g_pHeightData->nx;
    
    for ( int col=0; col<width; col++ ){
          
          color[i++] = (float) PIC_PIXEL(texture,col,row,0)/255;
          //color[i++] = 1.0f;
          //color[i++] = 1.0f;
          color[i++] = (float) PIC_PIXEL(texture,col,row,1)/255;
          color[i++] = (float) PIC_PIXEL(texture,col,row,2)/255;
        
          // if texture data is smaller than the rendered heightmap, reset the texture data
          if(col == texture->nx - 1 && width>0){
            //printf("\nlimit reached for col");
            width -= texture->nx;
            col = 0;
          }
      }
    
    // if texture data is smaller than the rendered heightmap, reset the texture data
    if(row == texture->ny - 1 && height > 0){
      //printf("\nlimit reached for row");
      height -= texture->ny;
      row = 0;
    }
    
  }
  
  printf("\n(texture) width = %d,height = %d",texture->nx,texture->ny);
  
  return color;
  
}

//Generate the required heightmap
void render() {
    glEnableClientState( GL_VERTEX_ARRAY );
    
    glVertexPointer( 3, GL_FLOAT, 0, getVertices( width, height ) );
    glDrawElements( GL_TRIANGLE_STRIP, getIndicesCount( width, height ), GL_UNSIGNED_INT, getIndices( width, height ));
    //glDrawArrays(GL_POINTS,0,getVerticesCount(width,height));
    glDisableClientState( GL_VERTEX_ARRAY );
}

//Main code ends here

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void reshape (int width, int height) {
glViewport(0, 0, (GLsizei)width, (GLsizei)height); //Set our viewport to the size of our window
glMatrixMode(GL_PROJECTION); //Switch to the projection matrix so that we can manipulate how our scene is viewed
glLoadIdentity(); //Reset the projection matrix to the identity matrix so that we don't get any artifacts (cleaning up)
gluPerspective(90, (GLfloat)width / (GLfloat)height, 0.01, 1000.0); //Set the Field of view angle (in degrees), the aspect ratio of our window, and the near and far planes
glMatrixMode(GL_MODELVIEW); //Switch back to the model view matrix, so that we can start drawing shapes correctly
}

void myinit()
{
  // enable depth buffering
  glEnable(GL_DEPTH_TEST);
  // interpolate colors during rasterization
  glShadeModel(GL_SMOOTH);
  
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void movement(){
  
  glTranslatef(0.0f,-100.0f,0.0f); // Move the heightmap down so that it is visible in the begining
  
  //Rotate the heightmap
  glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
  glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
  glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
  
  //Translate the heightmap
  glTranslatef(g_vLandTranslate[0], 0.0 , 0.0);
  glTranslatef(0.0, g_vLandTranslate[1] , 0.0);
  glTranslatef(0.0, 0.0, g_vLandTranslate[2]);
  
  //Scaling the heightmap
  glScalef(g_vLandScale[0], 1.0 ,1.0);
  glScalef(1.0, g_vLandScale[1] , 1.0);
  glScalef(1.0, 1.0 ,g_vLandScale[2]);
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* draw 1x1 cube about origin */
  /* replace this code with your height field implementation */
  /* you may also want to precede it with your 
rotation/translation/scaling */
  
  //Render the traingle mesh above the heightmap
  glPushMatrix();
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  movement();
  //glEnableClientState( GL_COLOR_ARRAY );
  //glColorPointer( 3, GL_FLOAT, 0, getColor( width, height ) );
  glColor4f(1.0f,0.0f,0.0f,0.5f);
  if(triangle_map)
    render();
  //glDisableClientState( GL_COLOR_ARRAY );
  glPopMatrix();
  
  //Render the heightmap
  glPushMatrix();
  glEnable (GL_POLYGON_OFFSET_FILL);
  glPolygonOffset (1.0f, 1.0f);

  switch (polygon_state) {
    case 0:
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      break;
    case 1:
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      break;
    case 2:
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      break;
    default:
      break;
  }
  
  //Push eveything x units back into the scene
  movement();
  
  glEnableClientState( GL_COLOR_ARRAY );
  glColorPointer( 3, GL_FLOAT, 0, getColor( width, height ) );
  render();
  glDisableClientState( GL_COLOR_ARRAY );
  
  glDisable (GL_POLYGON_OFFSET_FILL);
  
  glPopMatrix();
  
  //glDisable(GL_BLEND);
  
  glutSwapBuffers();
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void doIdle()
{
  /* do some stuff... */

  /* make the screen update */
  glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*5;
        g_vLandTranslate[1] -= vMouseDelta[1]*5;
      }
      if (g_iRightMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*5;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iRightMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iRightMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void keyboard(unsigned char key, int x, int y)
{
    if (key=='q' || key == 'Q')
        exit(0);

    if (key=='z')
      g_ControlState = ROTATE; 
    if (key=='x')
      g_ControlState = SCALE; 
    if (key=='c')
      g_ControlState = TRANSLATE; 
    
  if (key=='w')
    polygon_state = 0;
  if (key=='e')
    polygon_state = 1;
  if (key=='r')
    polygon_state = 2;
  
  if (key=='f')
    triangle_map = !triangle_map;
  
  if (key=='s')
    if (screenshot_name != NULL) {
      saveScreenshot(screenshot_name);
    }
}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  g_pHeightData = jpeg_read(argv[1], NULL);
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }

  width = g_pHeightData->nx;
  height = g_pHeightData->ny;
  printf("width = %d, height = %d",width,height);
  //width = 10;
  //height = 5;
  
  screenshot_name = argv[2];
  textureData = jpeg_read(argv[3],NULL);
  
  glutInit(&argc,argv);
  
  /*
    create a window here..should be double buffered and use depth testing
  
    the code past here will segfault if you don't have a window set up....
    replace the exit once you add those calls.
  */
  
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
  
  // set window size
  glutInitWindowSize(640, 480);
  
  // set window position
  glutInitWindowPosition(100, 100);
  
  // creates a window
  glutCreateWindow("Test");
  
  //exit(0);

  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);
  
  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_MIDDLE_BUTTON);
  
  /* replace with any animate code */
  glutIdleFunc(doIdle);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);
  
  glutKeyboardFunc(keyboard);//Keyboard functions for mac

  glutReshapeFunc(reshape);//To set up the camera for the scene
  
  /* do initialization */
  myinit();

  glutMainLoop();
  return(0);
}
