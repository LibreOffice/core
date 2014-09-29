/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;


import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.PixelFormat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import org.libreoffice.LibreOfficeMainActivity;

import java.nio.IntBuffer;

/**
 * A view rendered by the layer compositor.
 *
 * This view delegates to LayerRenderer to actually do the drawing. Its role is largely that of a
 * mediator between the LayerRenderer and the LayerController.
 *
 * Note that LayerView is accessed by Robocop via reflection.
 */
public class LayerView extends SurfaceView implements SurfaceHolder.Callback {
    private static String LOGTAG = "GeckoLayerView";

    private Context mContext;
    private LayerController mController;
    private TouchEventHandler mTouchEventHandler;
    private GLController mGLController;
    private InputConnectionHandler mInputConnectionHandler;
    private LayerRenderer mRenderer;
    private long mRenderTime;
    private boolean mRenderTimeReset;
    /* Must be a PAINT_xxx constant */
    private int mPaintState = PAINT_NONE;

    private Listener mListener;

    /* Flags used to determine when to show the painted surface. The integer
     * order must correspond to the order in which these states occur. */
    public static final int PAINT_NONE = 0;
    public static final int PAINT_BEFORE_FIRST = 1;
    public static final int PAINT_AFTER_FIRST = 2;


    public LayerView(Context context, LayerController controller) {
        super(context);

        SurfaceHolder holder = getHolder();
        holder.addCallback(this);
        holder.setFormat(PixelFormat.RGB_565);

        mGLController = new GLController(this);
        mContext = context;
        mController = controller;
        mTouchEventHandler = new TouchEventHandler(context, this, mController);
        mRenderer = new LayerRenderer(this);
        mInputConnectionHandler = null;

        setFocusable(true);
        setFocusableInTouchMode(true);

        createGLThread();
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getActionMasked() == MotionEvent.ACTION_DOWN)
            requestFocus();
        return mTouchEventHandler.handleEvent(event);
    }

    @Override
    public boolean onHoverEvent(MotionEvent event) {
        return mTouchEventHandler.handleEvent(event);
    }

    public LayerController getController() { return mController; }
    public TouchEventHandler getTouchEventHandler() { return mTouchEventHandler; }

    /** The LayerRenderer calls this to indicate that the window has changed size. */
    public void setViewportSize(IntSize size) {
        mController.setViewportSize(new FloatSize(size));
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

    public void requestRender() {
        if (mGLThread != null) {
            mGLThread.renderFrame();
        }
        if (mListener != null) {
            mListener.renderRequested();
        }

        synchronized(this) {
            if (!mRenderTimeReset) {
                mRenderTimeReset = true;
                mRenderTime = System.nanoTime();
            }
        }
    }

    public void addLayer(Layer layer) {
        mRenderer.addLayer(layer);
    }

    public void removeLayer(Layer layer) {
        mRenderer.removeLayer(layer);
    }

    /**
     * Returns the time elapsed between the first call of requestRender() after
     * the last call of getRenderTime(), in nanoseconds.
     */
    public long getRenderTime() {
        synchronized(this) {
            mRenderTimeReset = false;
            return System.nanoTime() - mRenderTime;
        }
    }

    public int getMaxTextureSize() {
        return mRenderer.getMaxTextureSize();
    }

    /** Used by robocop for testing purposes. Not for production use! This is called via reflection by robocop. */
    public IntBuffer getPixels() {
        return mRenderer.getPixels();
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

    public GLController getGLController() {
        return mGLController;
    }

    /** Implementation of SurfaceHolder.Callback */
    public synchronized void surfaceChanged(SurfaceHolder holder, int format, int width,
                                            int height) {
        mGLController.sizeChanged(width, height);

        if (mGLThread != null) {
            mGLThread.surfaceChanged(width, height);
        }

        if (mListener != null) {
            mListener.surfaceChanged(width, height);
        }
    }

    /** Implementation of SurfaceHolder.Callback */
    public synchronized void surfaceCreated(SurfaceHolder holder) {
        mGLController.surfaceCreated();
        if (mGLThread != null) {
            mGLThread.surfaceCreated();
        }
    }

    /** Implementation of SurfaceHolder.Callback */
    public synchronized void surfaceDestroyed(SurfaceHolder holder) {
        mGLController.surfaceDestroyed();

        if (mGLThread != null) {
            mGLThread.surfaceDestroyed();
        }

        if (mListener != null) {
            mListener.compositionPauseRequested();
        }
    }

    /** This function is invoked by Gecko (compositor thread) via JNI; be careful when modifying signature. */
    public static GLController registerCxxCompositor() {
        try {
            LayerView layerView = LibreOfficeMainActivity.mAppContext.getLayerController().getView();
            return layerView.getGLController();
        } catch (Exception e) {
            Log.e(LOGTAG, "### Exception! " + e);
            return null;
        }
    }

    public interface Listener {
        void renderRequested();
        void compositionPauseRequested();
        void compositionResumeRequested(int width, int height);
        void surfaceChanged(int width, int height);
    }

    private GLThread mGLThread; // Protected by this class's monitor.

    /**
     * Creates a Java GL thread. After this is called, the FlexibleGLSurfaceView may be used just
     * like a GLSurfaceView. It is illegal to access the controller after this has been called.
     */
    public synchronized void createGLThread() {
        if (mGLThread != null) {
            throw new LayerViewException ("createGLThread() called with a GL thread already in place!");
        }

        Log.e(LOGTAG, "### Creating GL thread!");
        mGLThread = new GLThread(mGLController);
        mGLThread.start();
        notifyAll();
    }

    /**
     * Destroys the Java GL thread. Returns a Thread that completes when the Java GL thread is
     * fully shut down.
     */
    public synchronized Thread destroyGLThread() {
        // Wait for the GL thread to be started.
        Log.e(LOGTAG, "### Waiting for GL thread to be created...");
        while (mGLThread == null) {
            try {
                wait();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }
        }

        Log.e(LOGTAG, "### Destroying GL thread!");
        Thread glThread = mGLThread;
        mGLThread.shutdown();
        mGLThread = null;
        return glThread;
    }

    public synchronized void recreateSurface() {
        if (mGLThread == null) {
            throw new LayerViewException("recreateSurface() called with no GL " +
                    "thread active!");
        }

        mGLThread.recreateSurface();
    }

    public static class LayerViewException extends RuntimeException {
        public static final long serialVersionUID = 1L;

        LayerViewException(String e) {
            super(e);
        }
    }

    public void changeCheckerboardBitmap(Bitmap bitmap, float pageWidth, float pageHeight) {
        mRenderer.setCheckerboardBitmap(bitmap, pageWidth, pageHeight);
    }
}
