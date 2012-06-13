package libreoffice.android.ui;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLES20;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLSurfaceView;
import android.opengl.GLU;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;

public class TextturePageActivity extends Activity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Page page = new Page(1, 1);
        PageView view = new PageView(this,page);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
		page.loadBitmap(BitmapFactory.decodeResource(getResources(),
				R.drawable.ph_800_1280_dddddd,options) );
   		view.setRenderer(new PageRenderer(page));
   		view.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
   		setContentView(view);
    }
}

class PageView extends GLSurfaceView {
	Page mPage;
	float zCumulative = 0;//can use this to limit zoom etc.

	
	float dragX = -1.0f;
	float dragY = -1.0f;
	
	float ptr0x = 0;
	float ptr0y = 0;
	float ptr1x = 0;
	float ptr1y = 0;
	
	//The smaller this is the more sensitive the surface is to pinching
	float zoomThreshold = 0.025f;
	//The amout to zoom per event. 
	//Could calculate this dynamically but it seems smoother this way. 
	float zoomIncrement = 0.025f;
	float dragIncrement = 0.01f;
	
	public PageView(Context context, Page mPage) {
		super(context);
		this.mPage = mPage;
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent ev) {
		Log.d("onTouch", "********");
		String str = new String();
		
		Log.d(" Screen -> Object ","" );
		Log.d("ptr count", Integer.toString( ev.getPointerCount() ));
		Log.d("action", Integer.toString( ev.getAction() ) );
		Log.d("action-masked", Integer.toString( ev.getActionMasked() ) );
		
		int action = ev.getActionMasked();
		int numDigits = ev.getPointerCount();
		float z = 0;
		
		
		if( action == ev.ACTION_MOVE && numDigits == 2){
			MotionEvent.PointerCoords ptr0Curr = new MotionEvent.PointerCoords();
			MotionEvent.PointerCoords ptr1Curr = new MotionEvent.PointerCoords();
			ev.getPointerCoords(0, ptr0Curr);
			ev.getPointerCoords(1, ptr1Curr);
			float currentDistance = (ptr1Curr.x - ptr0Curr.x)*(ptr1Curr.x - ptr0Curr.x) +
					(ptr1Curr.y - ptr0Curr.y)*(ptr1Curr.y - ptr0Curr.y);
			float prevDistance = ( ptr1x - ptr0x )*( ptr1x - ptr0x ) + 
					( ptr1y - ptr0y)*( ptr1y - ptr0y );
			Log.d("distance", " Curr = " + Float.toString(currentDistance) + 
					" Prev = " + Float.toString(prevDistance) );
			if(currentDistance >= (1+zoomThreshold)*prevDistance){//zoom in
				z = zoomIncrement;
				zCumulative += z;
			}
			if(currentDistance <= (1-zoomThreshold)*prevDistance){//zoom in
				z = -zoomIncrement;
				zCumulative -= z;
			}

			mPage.zoomBy( z );
			this.requestRender();
			
			ptr0x = ptr0Curr.x;
			ptr0y = ptr0Curr.y;
			ptr1x = ptr1Curr.x;
			ptr1y = ptr1Curr.y;
		}
		
		if(action == ev.ACTION_POINTER_UP && numDigits == 1){
			// End two finger gesture
			//end zoom
		}
		
		if(action == ev.ACTION_UP ){
			// gesture finished
			//reset state variables
			float zCumulative = 0;
			float ptr0x = 0;
			float ptr0y = 0;
			float ptr1x = 0;
			float ptr1y = 0;
		}
		
		if( action == ev.ACTION_DOWN && numDigits == 1){
			//one finger pressed
			dragX = ev.getX();
			dragY = ev.getY();
		}
		
		if( action == ev.ACTION_MOVE && numDigits == 1){
			//get normal vector in direction of motion
			//multiply by increment scalar
			//translate by x , y
			float currX = ev.getX();
			float currY = ev.getY();
			
			if( dragX == -1.0f || dragY == -1.0f ){//a bit dodgey
				dragX = currX;
				dragY = currY;
			}else{
				Log.d("drag", Float.toString( dragX ) + " " + Float.toString( dragY ) );
				Log.d("drag-diff", Float.toString( currX - dragX ) + " " + Float.toString( currY - dragY ) );
				
				float mag = ( currX - dragX )*( currX - dragX ) + 
						( currY - dragY )*( currY - dragY );
				float xComp = ( currX - dragX )/mag;
				float yComp = -( currY - dragY )/mag;
				
				mPage.translateBy( 0.025f*xComp , 0.025f*yComp );
				this.requestRender();
				
				dragX = currX;
				dragY = currY;
			}
		
		}
		return true;
	}

	//public float[] getObjectCoordinates
}

class PageRenderer implements GLSurfaceView.Renderer {
		private Page mPage;

		public PageRenderer(Page page) {
			// Initialize our square. 
			mPage = page;
		}
		/*
		 * (non-Javadoc)
		 * 
		 * @see
		 * android.opengl.GLSurfaceView.Renderer#onSurfaceCreated(javax.microedition
		 * .khronos.opengles.GL10, javax.microedition.khronos.egl.EGLConfig)
		 */
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			// Set the background color to black ( rgba ).
			gl.glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
			// Enable Smooth Shading, default not really needed.
			gl.glShadeModel(GL10.GL_SMOOTH);
			// Depth buffer setup.
			gl.glClearDepthf(1.0f);
			// Enables depth testing.
			gl.glEnable(GL10.GL_DEPTH_TEST);
			// The type of depth testing to do.
			gl.glDepthFunc(GL10.GL_LEQUAL);
			// Really nice perspective calculations.
			gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_NICEST);
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see
		 * android.opengl.GLSurfaceView.Renderer#onDrawFrame(javax.microedition.
		 * khronos.opengles.GL10)
		 */
		public void onDrawFrame(GL10 gl) {
			// Clears the screen and depth buffer.
			gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
			// Replace the current matrix with the identity matrix
			gl.glLoadIdentity();
			// Translates 4 units into the screen.
			//gl.glTranslatef(0, 0, -2); 
			// Draw our square.
			mPage.draw(gl); 
		}

		/*
		 * (non-Javadoc)
		 * 
		 * @see
		 * android.opengl.GLSurfaceView.Renderer#onSurfaceChanged(javax.microedition
		 * .khronos.opengles.GL10, int, int)
		 */
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			// Sets the current view port to the new size.
			gl.glViewport(0, 0, width, height);
			// Select the projection matrix
			gl.glMatrixMode(GL10.GL_PROJECTION);//TODO Should really use ModelView...
			// Reset the projection matrix
			gl.glLoadIdentity();
			// Calculate the aspect ratio of the window
			GLU.gluPerspective(gl, 45.0f, (float) width / (float) height, 0.1f,
					100.0f);
			// Select the modelview matrix
			gl.glMatrixMode(GL10.GL_MODELVIEW);
			// Reset the modelview matrix
			gl.glLoadIdentity();
		}
	}