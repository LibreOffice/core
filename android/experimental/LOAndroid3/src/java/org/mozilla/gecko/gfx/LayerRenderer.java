/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.SystemClock;
import android.util.Log;

import org.libreoffice.kit.DirectBufferAllocator;
import org.mozilla.gecko.gfx.Layer.RenderContext;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.concurrent.CopyOnWriteArrayList;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * The layer renderer implements the rendering logic for a layer view.
 */
public class LayerRenderer implements GLSurfaceView.Renderer {
    private static final String LOGTAG = "GeckoLayerRenderer";

    /*
     * The amount of time a frame is allowed to take to render before we declare it a dropped
     * frame.
     */
    private static final int MAX_FRAME_TIME = 16;   /* 1000 ms / 60 FPS */

    private final LayerView mView;
    private final SingleTileLayer mBackgroundLayer;
    private final NinePatchTileLayer mShadowLayer;
    private final ScrollbarLayer mHorizScrollLayer;
    private final ScrollbarLayer mVertScrollLayer;
    private final FadeRunnable mFadeRunnable;
    private ByteBuffer mCoordByteBuffer;
    private FloatBuffer mCoordBuffer;
    private RenderContext mLastPageContext;
    private int mMaxTextureSize;
    private int mBackgroundColor;

    private CopyOnWriteArrayList<Layer> mExtraLayers = new CopyOnWriteArrayList<Layer>();

    /* Used by robocop for testing purposes */
    private IntBuffer mPixelBuffer;

    // Used by GLES 2.0
    private int mProgram;
    private int mPositionHandle;
    private int mTextureHandle;
    private int mSampleHandle;
    private int mTMatrixHandle;

    // column-major matrix applied to each vertex to shift the viewport from
    // one ranging from (-1, -1),(1,1) to (0,0),(1,1) and to scale all sizes by
    // a factor of 2 to fill up the screen
    public static final float[] DEFAULT_TEXTURE_MATRIX = {
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };

    private static final int COORD_BUFFER_SIZE = 20;

    // The shaders run on the GPU directly, the vertex shader is only applying the
    // matrix transform detailed above

    // Note we flip the y-coordinate in the vertex shader from a
    // coordinate system with (0,0) in the top left to one with (0,0) in
    // the bottom left.

    public static final String DEFAULT_VERTEX_SHADER =
        "uniform mat4 uTMatrix;\n" +
        "attribute vec4 vPosition;\n" +
        "attribute vec2 aTexCoord;\n" +
        "varying vec2 vTexCoord;\n" +
        "void main() {\n" +
        "    gl_Position = uTMatrix * vPosition;\n" +
        "    vTexCoord.x = aTexCoord.x;\n" +
        "    vTexCoord.y = 1.0 - aTexCoord.y;\n" +
        "}\n";

    // We use highp because the screenshot textures
    // we use are large and we stretch them a lot
    // so we need all the precision we can get.
    // Unfortunately, highp is not required by ES 2.0
    // so on GPU's like Mali we end up getting mediump
    public static final String DEFAULT_FRAGMENT_SHADER =
        "precision highp float;\n" +
        "varying vec2 vTexCoord;\n" +
        "uniform sampler2D sTexture;\n" +
        "void main() {\n" +
        "    gl_FragColor = texture2D(sTexture, vTexCoord);\n" +
        "}\n";

    public LayerRenderer(LayerView view) {
        mView = view;

        CairoImage backgroundImage = new BufferedCairoImage(view.getBackgroundPattern());
        mBackgroundLayer = new SingleTileLayer(true, backgroundImage);

        CairoImage shadowImage = new BufferedCairoImage(view.getShadowPattern());
        mShadowLayer = new NinePatchTileLayer(shadowImage);

        mHorizScrollLayer = ScrollbarLayer.create(this, false);
        mVertScrollLayer = ScrollbarLayer.create(this, true);
        mFadeRunnable = new FadeRunnable();

        // Initialize the FloatBuffer that will be used to store all vertices and texture
        // coordinates in draw() commands.
        mCoordByteBuffer = DirectBufferAllocator.allocate(COORD_BUFFER_SIZE * 4);
        mCoordByteBuffer.order(ByteOrder.nativeOrder());
        mCoordBuffer = mCoordByteBuffer.asFloatBuffer();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            DirectBufferAllocator.free(mCoordByteBuffer);
            mCoordByteBuffer = null;
            mCoordBuffer = null;
        } finally {
            super.finalize();
        }
    }

    public void destroy() {
        DirectBufferAllocator.free(mCoordByteBuffer);
        mCoordByteBuffer = null;
        mCoordBuffer = null;
        mBackgroundLayer.destroy();
        mShadowLayer.destroy();
        mHorizScrollLayer.destroy();
        mVertScrollLayer.destroy();
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        createDefaultProgram();
        activateDefaultProgram();
    }

    public void createDefaultProgram() {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER, DEFAULT_VERTEX_SHADER);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER, DEFAULT_FRAGMENT_SHADER);

        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, vertexShader);   // add the vertex shader to program
        GLES20.glAttachShader(mProgram, fragmentShader); // add the fragment shader to program
        GLES20.glLinkProgram(mProgram);                  // creates OpenGL program executables

        // Get handles to the vertex shader's vPosition, aTexCoord, sTexture, and uTMatrix members.
        mPositionHandle = GLES20.glGetAttribLocation(mProgram, "vPosition");
        mTextureHandle = GLES20.glGetAttribLocation(mProgram, "aTexCoord");
        mSampleHandle = GLES20.glGetUniformLocation(mProgram, "sTexture");
        mTMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uTMatrix");

        int maxTextureSizeResult[] = new int[1];
        GLES20.glGetIntegerv(GLES20.GL_MAX_TEXTURE_SIZE, maxTextureSizeResult, 0);
        mMaxTextureSize = maxTextureSizeResult[0];
    }

    // Activates the shader program.
    public void activateDefaultProgram() {
        // Add the program to the OpenGL environment
        GLES20.glUseProgram(mProgram);

        // Set the transformation matrix
        GLES20.glUniformMatrix4fv(mTMatrixHandle, 1, false, DEFAULT_TEXTURE_MATRIX, 0);

        // Enable the arrays from which we get the vertex and texture coordinates
        GLES20.glEnableVertexAttribArray(mPositionHandle);
        GLES20.glEnableVertexAttribArray(mTextureHandle);

        GLES20.glUniform1i(mSampleHandle, 0);

        // TODO: Move these calls into a separate deactivate() call that is called after the
        // underlay and overlay are rendered.
    }

    // Deactivates the shader program. This must be done to avoid crashes after returning to the
    // Gecko C++ compositor from Java.
    public void deactivateDefaultProgram() {
        GLES20.glDisableVertexAttribArray(mTextureHandle);
        GLES20.glDisableVertexAttribArray(mPositionHandle);
        GLES20.glUseProgram(0);
    }

    public int getMaxTextureSize() {
        return mMaxTextureSize;
    }

    public void addLayer(Layer layer) {
        synchronized (mExtraLayers) {
            if (mExtraLayers.contains(layer)) {
                mExtraLayers.remove(layer);
            }

            mExtraLayers.add(layer);
        }
    }

    public void removeLayer(Layer layer) {
        synchronized (mExtraLayers) {
            mExtraLayers.remove(layer);
        }
    }

    /**
     * Called whenever a new frame is about to be drawn.
     */
    public void onDrawFrame(GL10 gl) {
        Frame frame = createFrame(mView.getLayerClient().getViewportMetrics());
        synchronized (mView.getLayerClient()) {
            frame.beginDrawing();
            frame.drawBackground();
            frame.drawRootLayer();
            frame.drawForeground();
            frame.endDrawing();
        }
    }

    private RenderContext createScreenContext(ImmutableViewportMetrics metrics) {
        RectF viewport = new RectF(0.0f, 0.0f, metrics.getWidth(), metrics.getHeight());
        RectF pageRect = new RectF(metrics.getPageRect());
        return createContext(viewport, pageRect, 1.0f);
    }

    private RenderContext createPageContext(ImmutableViewportMetrics metrics) {
        Rect viewport = RectUtils.round(metrics.getViewport());
        RectF pageRect = metrics.getPageRect();
        float zoomFactor = metrics.zoomFactor;
        return createContext(new RectF(viewport), pageRect, zoomFactor);
    }

    private RenderContext createContext(RectF viewport, RectF pageRect, float zoomFactor) {
        return new RenderContext(viewport, pageRect, zoomFactor, mPositionHandle, mTextureHandle,
                                 mCoordBuffer);
    }

    public void onSurfaceChanged(GL10 gl, final int width, final int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    /*
     * create a vertex shader type (GLES20.GL_VERTEX_SHADER)
     * or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
     */
    public static int loadShader(int type, String shaderCode) {
        int shader = GLES20.glCreateShader(type);
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);
        return shader;
    }

    public Frame createFrame(ImmutableViewportMetrics metrics) {
        return new Frame(metrics);
    }

    class FadeRunnable implements Runnable {
        private boolean mStarted;
        private long mRunAt;

        void scheduleStartFade(long delay) {
            mRunAt = SystemClock.elapsedRealtime() + delay;
            if (!mStarted) {
                mView.postDelayed(this, delay);
                mStarted = true;
            }
        }

        void scheduleNextFadeFrame() {
            if (mStarted) {
                Log.e(LOGTAG, "scheduleNextFadeFrame() called while scheduled for starting fade");
            }
            mView.postDelayed(this, 1000L / 60L); // request another frame at 60fps
        }

        boolean timeToFade() {
            return !mStarted;
        }

        public void run() {
            long timeDelta = mRunAt - SystemClock.elapsedRealtime();
            if (timeDelta > 0) {
                // the run-at time was pushed back, so reschedule
                mView.postDelayed(this, timeDelta);
            } else {
                // reached the run-at time, execute
                mStarted = false;
                mView.requestRender();
            }
        }
    }

    public class Frame {
        // The timestamp recording the start of this frame.
        private long mFrameStartTime;
        // A fixed snapshot of the viewport metrics that this frame is using to render content.
        private ImmutableViewportMetrics mFrameMetrics;
        // A rendering context for page-positioned layers, and one for screen-positioned layers.
        private RenderContext mPageContext, mScreenContext;
        // Whether a layer was updated.
        private boolean mUpdated;
        private final Rect mPageRect;

        public Frame(ImmutableViewportMetrics metrics) {
            mFrameMetrics = metrics;
            mPageContext = createPageContext(metrics);
            mScreenContext = createScreenContext(metrics);
            mPageRect = getPageRect();
        }

        private void setScissorRect() {
            Rect scissorRect = transformToScissorRect(mPageRect);
            GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
            GLES20.glScissor(scissorRect.left, scissorRect.top,
                             scissorRect.width(), scissorRect.height());
        }

        private Rect transformToScissorRect(Rect rect) {
            IntSize screenSize = new IntSize(mFrameMetrics.getSize());

            int left = Math.max(0, rect.left);
            int top = Math.max(0, rect.top);
            int right = Math.min(screenSize.width, rect.right);
            int bottom = Math.min(screenSize.height, rect.bottom);

            return new Rect(left, screenSize.height - bottom, right,
                            (screenSize.height - bottom) + (bottom - top));
        }

        private Rect getPageRect() {
            Point origin = PointUtils.round(mFrameMetrics.getOrigin());
            Rect pageRect = RectUtils.round(mFrameMetrics.getPageRect());
            pageRect.offset(-origin.x, -origin.y);
            return pageRect;
        }

        /** This function is invoked via JNI; be careful when modifying signature. */
        public void beginDrawing() {
            mFrameStartTime = SystemClock.uptimeMillis();

            TextureReaper.get().reap();
            TextureGenerator.get().fill();

            mUpdated = true;

            Layer rootLayer = mView.getLayerClient().getRoot();
            Layer lowResLayer = mView.getLayerClient().getLowResLayer();

            if (!mPageContext.fuzzyEquals(mLastPageContext)) {
                // the viewport or page changed, so show the scrollbars again
                // as per UX decision
                mVertScrollLayer.unfade();
                mHorizScrollLayer.unfade();
                mFadeRunnable.scheduleStartFade(ScrollbarLayer.FADE_DELAY);
            } else if (mFadeRunnable.timeToFade()) {
                boolean stillFading = mVertScrollLayer.fade() | mHorizScrollLayer.fade();
                if (stillFading) {
                    mFadeRunnable.scheduleNextFadeFrame();
                }
            }
            mLastPageContext = mPageContext;

            /* Update layers. */
            if (rootLayer != null) mUpdated &= rootLayer.update(mPageContext);  // called on compositor thread
            if (lowResLayer != null) mUpdated &= lowResLayer.update(mPageContext);  // called on compositor thread
            mUpdated &= mBackgroundLayer.update(mScreenContext);    // called on compositor thread
            mUpdated &= mShadowLayer.update(mPageContext);  // called on compositor thread
            mUpdated &= mVertScrollLayer.update(mPageContext);  // called on compositor thread
            mUpdated &= mHorizScrollLayer.update(mPageContext); // called on compositor thread

            for (Layer layer : mExtraLayers)
                mUpdated &= layer.update(mPageContext); // called on compositor thread
        }

        /** Retrieves the bounds for the layer, rounded in such a way that it
         * can be used as a mask for something that will render underneath it.
         * This will round the bounds inwards, but stretch the mask towards any
         * near page edge, where near is considered to be 'within 2 pixels'.
         * Returns null if the given layer is null.
         */
        private Rect getMaskForLayer(Layer layer) {
            if (layer == null) {
                return null;
            }

            RectF bounds = RectUtils.contract(layer.getBounds(mPageContext), 1.0f, 1.0f);
            Rect mask = RectUtils.roundIn(bounds);

            // If the mask is within two pixels of any page edge, stretch it over
            // that edge. This is to avoid drawing thin slivers when masking
            // layers.
            if (mask.top <= 2) {
                mask.top = -1;
            }
            if (mask.left <= 2) {
                mask.left = -1;
            }

            // Because we're drawing relative to the page-rect, we only need to
            // take into account its width and height (and not its origin)
            int pageRight = mPageRect.width();
            int pageBottom = mPageRect.height();

            if (mask.right >= pageRight - 2) {
                mask.right = pageRight + 1;
            }
            if (mask.bottom >= pageBottom - 2) {
                mask.bottom = pageBottom + 1;
            }

            return mask;
        }

        /** This function is invoked via JNI; be careful when modifying signature. */
        public void drawBackground() {
            GLES20.glDisable(GLES20.GL_SCISSOR_TEST);

            /* Update background color. */
            mBackgroundColor = Color.WHITE;

            /* Clear to the page background colour. The bits set here need to
             * match up with those used in gfx/layers/opengl/LayerManagerOGL.cpp.
             */
            GLES20.glClearColor(((mBackgroundColor>>16)&0xFF) / 255.0f,
                                ((mBackgroundColor>>8)&0xFF) / 255.0f,
                                (mBackgroundColor&0xFF) / 255.0f,
                                0.0f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT |
                           GLES20.GL_DEPTH_BUFFER_BIT);

            /* Draw the background. */
            mBackgroundLayer.setMask(mPageRect);
            mBackgroundLayer.draw(mScreenContext);

            /* Draw the drop shadow, if we need to. */
            RectF untransformedPageRect = new RectF(0.0f, 0.0f, mPageRect.width(),
                                                    mPageRect.height());
            if (!untransformedPageRect.contains(mFrameMetrics.getViewport()))
                mShadowLayer.draw(mPageContext);

            /* Scissor around the page-rect, in case the page has shrunk
             * since the screenshot layer was last updated.
             */
            setScissorRect(); // Calls glEnable(GL_SCISSOR_TEST))
        }

        // Draws the layer the client added to us.
        void drawRootLayer() {
            Layer lowResLayer = mView.getLayerClient().getLowResLayer();
            if (lowResLayer == null) {
                return;
            }
            lowResLayer.draw(mPageContext);

            Layer rootLayer = mView.getLayerClient().getRoot();
            if (rootLayer == null) {
                return;
            }

            rootLayer.draw(mPageContext);
        }

        /** This function is invoked via JNI; be careful when modifying signature. */
        public void drawForeground() {
            /* Draw any extra layers that were added (likely plugins) */
            if (mExtraLayers.size() > 0) {
                for (Layer layer : mExtraLayers) {
                    if (!layer.usesDefaultProgram())
                        deactivateDefaultProgram();

                    layer.draw(mPageContext);

                    if (!layer.usesDefaultProgram())
                        activateDefaultProgram();
                }
            }

            /* Draw the vertical scrollbar. */
            if (mPageRect.height() > mFrameMetrics.getHeight())
                mVertScrollLayer.draw(mPageContext);

            /* Draw the horizontal scrollbar. */
            if (mPageRect.width() > mFrameMetrics.getWidth())
                mHorizScrollLayer.draw(mPageContext);
        }

        /** This function is invoked via JNI; be careful when modifying signature. */
        public void endDrawing() {
            // If a layer update requires further work, schedule another redraw
            if (!mUpdated)
                mView.requestRender();

            /* Used by robocop for testing purposes */
            IntBuffer pixelBuffer = mPixelBuffer;
            if (mUpdated && pixelBuffer != null) {
                synchronized (pixelBuffer) {
                    pixelBuffer.position(0);
                    GLES20.glReadPixels(0, 0, (int)mScreenContext.viewport.width(),
                                        (int)mScreenContext.viewport.height(), GLES20.GL_RGBA,
                                        GLES20.GL_UNSIGNED_BYTE, pixelBuffer);
                    pixelBuffer.notify();
                }
            }
        }
    }
}
