/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;


import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.FrameLayout;

import org.libreoffice.LibreOfficeMainActivity;
import org.libreoffice.R;
import org.mozilla.gecko.OnInterceptTouchListener;
import org.mozilla.gecko.OnSlideSwipeListener;

/**
 * A view rendered by the layer compositor.
 *
 * This view delegates to LayerRenderer to actually do the drawing. Its role is largely that of a
 * mediator between the LayerRenderer and the LayerController.
 *
 * Note that LayerView is accessed by Robocop via reflection.
 */
public class LayerView extends FrameLayout {
    private static String LOGTAG = LayerView.class.getName();

    private GeckoLayerClient mLayerClient;
    private PanZoomController mPanZoomController;
    private GLController mGLController;
    private InputConnectionHandler mInputConnectionHandler;
    private LayerRenderer mRenderer;

    /* Must be a PAINT_xxx constant */
    private int mPaintState = PAINT_NONE;
    private boolean mFullScreen = false;

    private SurfaceView mSurfaceView;
    private TextureView mTextureView;

    private Listener mListener;
    private OnInterceptTouchListener mTouchIntercepter;

    /* Flags used to determine when to show the painted surface. The integer
     * order must correspond to the order in which these states occur. */
    public static final int PAINT_NONE = 0;
    public static final int PAINT_BEFORE_FIRST = 1;
    public static final int PAINT_AFTER_FIRST = 2;

    boolean shouldUseTextureView() {
        // we can only use TextureView on ICS or higher
        /*if (Build.VERSION.SDK_INT < Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
            Log.i(LOGTAG, "Not using TextureView: not on ICS+");
            return false;
        }

        try {
            // and then we can only use it if we have a hardware accelerated window
            Method m = View.class.getMethod("isHardwareAccelerated", new Class[0]);
            return (Boolean) m.invoke(this);
        } catch (Exception e) {
            Log.i(LOGTAG, "Not using TextureView: caught exception checking for hw accel: " + e.toString());
            return false;
        }*/
        return false;
    }

    public LayerView(Context context, AttributeSet attrs) {
        super(context, attrs);

        if (shouldUseTextureView()) {
            mTextureView = new TextureView(context);
            mTextureView.setSurfaceTextureListener(new SurfaceTextureListener());

            addView(mTextureView, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        } else {
            mSurfaceView = new SurfaceView(context);
            addView(mSurfaceView, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

            SurfaceHolder holder = mSurfaceView.getHolder();
            holder.addCallback(new SurfaceListener());
            holder.setFormat(PixelFormat.RGB_565);
        }

        mGLController = new GLController(this);
    }

    void connect(GeckoLayerClient layerClient) {
        mLayerClient = layerClient;
        mPanZoomController = mLayerClient.getPanZoomController();
        mRenderer = new LayerRenderer(this);
        mInputConnectionHandler = null;

        setFocusable(true);
        setFocusableInTouchMode(true);

        createGLThread();
        setOnTouchListener(new OnSlideSwipeListener(getContext(), mLayerClient));
    }

    public void show() {
        // Fix this if TextureView support is turned back on above
        mSurfaceView.setVisibility(View.VISIBLE);
    }

    public void hide() {
        // Fix this if TextureView support is turned back on above
        mSurfaceView.setVisibility(View.INVISIBLE);
    }

    public void destroy() {
        if (mLayerClient != null) {
            mLayerClient.destroy();
        }
        if (mRenderer != null) {
            mRenderer.destroy();
        }
    }

    public void setTouchIntercepter(final OnInterceptTouchListener touchIntercepter) {
        // this gets run on the gecko thread, but for thread safety we want the assignment
        // on the UI thread.
        post(new Runnable() {
            public void run() {
                mTouchIntercepter = touchIntercepter;
            }
        });
    }

    public void setInputConnectionHandler(InputConnectionHandler inputConnectionHandler) {
        mInputConnectionHandler = inputConnectionHandler;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
            requestFocus();
        }

        if (mTouchIntercepter != null && mTouchIntercepter.onInterceptTouchEvent(this, event)) {
            return true;
        }
        if (mPanZoomController != null && mPanZoomController.onTouchEvent(event)) {
            return true;
        }
        if (mTouchIntercepter != null && mTouchIntercepter.onTouch(this, event)) {
            return true;
        }
        return false;
    }

    @Override
    public boolean onHoverEvent(MotionEvent event) {
        if (mTouchIntercepter != null && mTouchIntercepter.onTouch(this, event)) {
            return true;
        }
        return false;
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if (mPanZoomController != null && mPanZoomController.onMotionEvent(event)) {
            return true;
        }
        return false;
    }

    public GeckoLayerClient getLayerClient() { return mLayerClient; }
    public PanZoomController getPanZoomController() { return mPanZoomController; }

    public void setViewportSize(IntSize size) {
        mLayerClient.setViewportSize(new FloatSize(size));
    }

    public ImmutableViewportMetrics getViewportMetrics() {
        return mLayerClient.getViewportMetrics();
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if (mInputConnectionHandler != null)
            return mInputConnectionHandler.onCreateInputConnection(outAttrs);
        return null;
    }

    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        if (mInputConnectionHandler != null)
            return mInputConnectionHandler.onKeyPreIme(keyCode, event);
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mInputConnectionHandler != null)
            return mInputConnectionHandler.onKeyDown(keyCode, event);
        return false;
    }

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        if (mInputConnectionHandler != null)
            return mInputConnectionHandler.onKeyLongPress(keyCode, event);
        return false;
    }

    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        if (mInputConnectionHandler != null)
            return mInputConnectionHandler.onKeyMultiple(keyCode, repeatCount, event);
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (mInputConnectionHandler != null)
            return mInputConnectionHandler.onKeyUp(keyCode, event);
        return false;
    }

    public boolean isIMEEnabled() {
        /*if (mInputConnectionHandler != null) {
            return mInputConnectionHandler.isIMEEnabled();
        }*/
        return false;
    }

    public void requestRender() {
        if (mListener != null) {
            mListener.renderRequested();
        }
    }

    public void addLayer(Layer layer) {
        mRenderer.addLayer(layer);
    }

    public void removeLayer(Layer layer) {
        mRenderer.removeLayer(layer);
    }

    public int getMaxTextureSize() {
        return mRenderer.getMaxTextureSize();
    }

    public void setLayerRenderer(LayerRenderer renderer) {
        mRenderer = renderer;
    }

    public LayerRenderer getLayerRenderer() {
        return mRenderer;
    }

    /* paintState must be a PAINT_xxx constant. The state will only be changed
     * if paintState represents a state that occurs after the current state. */
    public void setPaintState(int paintState) {
        if (paintState > mPaintState) {
            Log.d(LOGTAG, "LayerView paint state set to " + paintState);
            mPaintState = paintState;
        }
    }

    public int getPaintState() {
        return mPaintState;
    }

    public LayerRenderer getRenderer() {
        return mRenderer;
    }

    public void setListener(Listener listener) {
        mListener = listener;
    }

    Listener getListener() {
        return mListener;
    }

    public GLController getGLController() {
        return mGLController;
    }

    public Bitmap getDrawable(String name) {
        Context context = getContext();
        Resources resources = context.getResources();
        String packageName = resources.getResourcePackageName(R.id.dummy_id_for_package_name_resolution);
        int resourceID = resources.getIdentifier(name, "drawable", packageName);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        return BitmapFactory.decodeResource(context.getResources(), resourceID, options);
    }

    Bitmap getBackgroundPattern() {
        return getDrawable("background");
    }

    Bitmap getShadowPattern() {
        return getDrawable("shadow");
    }

    private void onSizeChanged(int width, int height) {
        mGLController.surfaceChanged(width, height);

        mLayerClient.setViewportSize(new FloatSize(width, height));

        if (mListener != null) {
            mListener.surfaceChanged(width, height);
        }
    }

    private void onDestroyed() {
        mGLController.surfaceDestroyed();

        if (mListener != null) {
            mListener.compositionPauseRequested();
        }
    }

    public Object getNativeWindow() {
        if (mSurfaceView != null)
            return mSurfaceView.getHolder();

        return mTextureView.getSurfaceTexture();
    }

    /** This function is invoked by Gecko (compositor thread) via JNI; be careful when modifying signature. */
    public static GLController registerCxxCompositor() {
        try {
            LayerView layerView = LibreOfficeMainActivity.getLayerClient().getView();
            layerView.mListener.compositorCreated();
            return layerView.getGLController();
        } catch (Exception e) {
            Log.e(LOGTAG, "Error registering compositor!", e);
            return null;
        }
    }

    public interface Listener {
        void compositorCreated();
        void renderRequested();
        void compositionPauseRequested();
        void compositionResumeRequested(int width, int height);
        void surfaceChanged(int width, int height);
    }

    private class SurfaceListener implements SurfaceHolder.Callback {
        public void surfaceChanged(SurfaceHolder holder, int format, int width,
                                                int height) {
            onSizeChanged(width, height);
        }

        public void surfaceCreated(SurfaceHolder holder) {
            if (mRenderControllerThread != null) {
                mRenderControllerThread.surfaceCreated();
            }
        }

        public void surfaceDestroyed(SurfaceHolder holder) {
            onDestroyed();
        }
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        if (changed) {
            setViewportSize(new IntSize(right - left, bottom - top));
        }
    }

    private class SurfaceTextureListener implements TextureView.SurfaceTextureListener {
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            // We don't do this for surfaceCreated above because it is always followed by a surfaceChanged,
            // but that is not the case here.
            if (mRenderControllerThread != null) {
                mRenderControllerThread.surfaceCreated();
            }
            onSizeChanged(width, height);
        }

        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            onDestroyed();
            return true; // allow Android to call release() on the SurfaceTexture, we are done drawing to it
        }

        public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
            onSizeChanged(width, height);
        }

        public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        }
    }

    private RenderControllerThread mRenderControllerThread;

    public synchronized void createGLThread() {
        if (mRenderControllerThread != null) {
            throw new LayerViewException ("createGLThread() called with a GL thread already in place!");
        }

        Log.e(LOGTAG, "### Creating GL thread!");
        mRenderControllerThread = new RenderControllerThread(mGLController);
        mRenderControllerThread.start();
        setListener(mRenderControllerThread);
        notifyAll();
    }

    public synchronized Thread destroyGLThread() {
        // Wait for the GL thread to be started.
        Log.e(LOGTAG, "### Waiting for GL thread to be created...");
        while (mRenderControllerThread == null) {
            try {
                wait();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }

        Log.e(LOGTAG, "### Destroying GL thread!");
        Thread thread = mRenderControllerThread;
        mRenderControllerThread.shutdown();
        setListener(null);
        mRenderControllerThread = null;
        return thread;
    }

    public static class LayerViewException extends RuntimeException {
        public static final long serialVersionUID = 1L;

        LayerViewException(String e) {
            super(e);
        }
    }

    public void setFullScreen(boolean fullScreen) {
        mFullScreen = fullScreen;
    }

    public boolean isFullScreen() {
        return mFullScreen;
    }
}
