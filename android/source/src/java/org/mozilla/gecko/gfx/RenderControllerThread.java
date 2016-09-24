package org.mozilla.gecko.gfx;

import android.opengl.GLSurfaceView;

import java.util.concurrent.LinkedBlockingQueue;

import javax.microedition.khronos.opengles.GL10;

/**
 * Thread which controls the rendering to OpenGL context. Render commands are queued and
 * processed and delegated by this thread.
 */
public class RenderControllerThread extends Thread implements LayerView.Listener {
    private LinkedBlockingQueue<RenderCommand> queue = new LinkedBlockingQueue<RenderCommand>();
    private GLController controller;
    private boolean renderQueued = false;
    private int width;
    private int height;

    public RenderControllerThread(GLController controller) {
        this.controller = controller;
    }

    @Override
    public void run() {
        while (true) {
            RenderCommand command;
            try {
                command = queue.take();
                execute(command);
                if (command == RenderCommand.SHUTDOWN) {
                    return;
                }
            } catch (InterruptedException exception) {
                throw new RuntimeException(exception);
            }
        }
    }

    void execute(RenderCommand command) {
        switch (command) {
            case SHUTDOWN:
                doShutdown();
                break;
            case RENDER_FRAME:
                doRenderFrame();
                break;
            case SIZE_CHANGED:
                doSizeChanged();
                break;
            case SURFACE_CREATED:
                doSurfaceCreated();
                break;
            case SURFACE_DESTROYED:
                doSurfaceDestroyed();
                break;
        }
    }

    public void shutdown() {
        queue.add(RenderCommand.SHUTDOWN);
    }

    @Override
    public void compositorCreated() {

    }

    @Override
    public void renderRequested() {
        synchronized (this) {
            if (!renderQueued) {
                queue.add(RenderCommand.RENDER_FRAME);
                renderQueued = true;
            }
        }
    }

    @Override
    public void compositionPauseRequested() {
        queue.add(RenderCommand.SURFACE_DESTROYED);
    }

    @Override
    public void compositionResumeRequested(int width, int height) {

    }

    @Override
    public void surfaceChanged(int width, int height) {
        this.width = width;
        this.height = height;
        queue.add(RenderCommand.SIZE_CHANGED);
    }

    public void surfaceCreated() {
        queue.add(RenderCommand.SURFACE_CREATED);
    }

    private GLSurfaceView.Renderer getRenderer() {
        return controller.getView().getRenderer();
    }

    private void doShutdown() {
        controller.disposeGLContext();
        controller = null;
    }

    private void doRenderFrame() {
        synchronized (this) {
            renderQueued = false;
        }
        if (controller.getEGLSurface() == null) {
            return;
        }
        GLSurfaceView.Renderer renderer = getRenderer();
        if (renderer != null) {
            renderer.onDrawFrame((GL10) controller.getGL());
        }
        controller.swapBuffers();
    }

    private void doSizeChanged() {
        GLSurfaceView.Renderer renderer = getRenderer();
        if (renderer != null) {
            renderer.onSurfaceChanged((GL10) controller.getGL(), width, height);
        }
    }

    private void doSurfaceCreated() {
        if (!controller.hasSurface()) {
            controller.initGLContext();
        }
    }

    private void doSurfaceDestroyed() {
        controller.disposeGLContext();
    }

    public enum RenderCommand {
        SHUTDOWN,
        RECREATE_SURFACE,
        RENDER_FRAME,
        SIZE_CHANGED,
        SURFACE_CREATED,
        SURFACE_DESTROYED,
    }
}
