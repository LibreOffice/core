/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GL/glew.h>

#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include <boost/scoped_array.hpp>
#include <vcl/pngwrite.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>

#include "unx/x11/X11OpenGLContext.hxx"

using namespace com::sun::star;

X11OpenGLContext::X11OpenGLContext():
    mpWindow(NULL),
    m_pChildWindow(NULL),
    mbInitialized(false)
{
}

X11OpenGLContext::~X11OpenGLContext()
{
    if(m_aGLWin.ctx)
    {
        glXMakeCurrent(m_aGLWin.dpy, None, NULL);
        if( glGetError() != GL_NO_ERROR )
        {
            SAL_WARN("vcl.opengl", "glError: " << (char *)gluErrorString(glGetError()));
        }
        glXDestroyContext(m_aGLWin.dpy, m_aGLWin.ctx);
    }
}

#ifdef DBG_UTIL

namespace {

const char* getSeverityString(GLenum severity)
{
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
            return "low";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "medium";
        case GL_DEBUG_SEVERITY_HIGH:
            return "high";
        default:
            ;
    }

    return "unknown";
}

const char* getSourceString(GLenum source)
{
    switch(source)
    {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "shader compiler";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "window system";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "third party";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "Libreoffice";
        case GL_DEBUG_SOURCE_OTHER:
            return "unknown";
        default:
            ;
    }

    return "unknown";
}

const char* getTypeString(GLenum type)
{
    switch(type)
    {
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "deprecated behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "undefined behavior";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "performance";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "portability";
        case GL_DEBUG_TYPE_MARKER:
            return "marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "push group";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "pop group";
        case GL_DEBUG_TYPE_OTHER:
            return "other";
        case GL_DEBUG_TYPE_ERROR:
            return "error";
        default:
            ;
    }

    return "unkown";
}

extern "C" void
debug_callback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei , const GLchar* message, GLvoid* )
{
    SAL_WARN("vcl.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << " with message: " << message);
}

}

#endif

namespace {

static bool errorTriggered;
int oglErrorHandler( Display* /*dpy*/, XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

}

bool X11OpenGLContext::init( Window* pParent )
{
    if(mbInitialized)
        return true;

    m_pWindow.reset(pParent ? NULL : new Window(0, WB_NOBORDER|WB_NODIALOGCONTROL));
    mpWindow = pParent ? pParent : m_pWindow.get();
    m_pChildWindow = 0;
    initWindow();
    return ImplInit();
}

bool X11OpenGLContext::init(SystemChildWindow* pChildWindow)
{
    if(mbInitialized)
        return true;

    if( !pChildWindow )
        return false;

    mpWindow = pChildWindow->GetParent();
    m_pChildWindow = pChildWindow;
    initWindow();
    return ImplInit();
}

bool X11OpenGLContext::ImplInit()
{
    SAL_INFO("vcl.opengl", "X11OpenGLContext::ImplInit----start");
    if(m_pWindow)
        m_pWindow->setPosSizePixel(0,0,0,0);
    m_aGLWin.Width = 0;
    m_aGLWin.Height = 0;

    m_aGLWin.ctx = m_aGLWin.dpy == 0 ? 0 : glXCreateContext(m_aGLWin.dpy,
                                 m_aGLWin.vi,
                                 0,
                                 GL_TRUE);
    if( m_aGLWin.ctx == NULL )
    {
        SAL_INFO("vcl.opengl", "unable to create GLX context");
        return false;
    }

    if( !glXMakeCurrent( m_aGLWin.dpy, m_aGLWin.win, m_aGLWin.ctx ) )
    {
        SAL_INFO("vcl.opengl", "unable to select current GLX context");
        return false;
    }

    int glxMinor, glxMajor;
    double nGLXVersion = 0;
    if( glXQueryVersion( m_aGLWin.dpy, &glxMajor, &glxMinor ) )
      nGLXVersion = glxMajor + 0.1*glxMinor;
    SAL_INFO("vcl.opengl", "available GLX version: " << nGLXVersion);

    m_aGLWin.GLExtensions = glGetString( GL_EXTENSIONS );
    SAL_INFO("vcl.opengl", "available GL  extensions: " << m_aGLWin.GLExtensions);

    if( m_aGLWin.HasGLXExtension("GLX_SGI_swap_control" ) )
    {
        // enable vsync
        typedef GLint (*glXSwapIntervalProc)(GLint);
        glXSwapIntervalProc glXSwapInterval = (glXSwapIntervalProc) glXGetProcAddress( (const GLubyte*) "glXSwapIntervalSGI" );
        if( glXSwapInterval ) {
        int (*oldHandler)(Display* /*dpy*/, XErrorEvent* /*evnt*/);

        // replace error handler temporarily
        oldHandler = XSetErrorHandler( oglErrorHandler );

        errorTriggered = false;

        glXSwapInterval( 1 );

        // sync so that we possibly get an XError
        glXWaitGL();
        XSync(m_aGLWin.dpy, false);

        if( errorTriggered )
            SAL_INFO("vcl.opengl", "error when trying to set swap interval, NVIDIA or Mesa bug?");
        else
            SAL_INFO("vcl.opengl", "set swap interval to 1 (enable vsync)");

        // restore the error handler
        XSetErrorHandler( oldHandler );
        }
    }

    //rGLRender.InitOpenGL(m_aGLWin);

    static bool bGlewInit = false;
    if(!bGlewInit)
    {
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            SAL_WARN("vcl.opengl", "Failed to initialize GLEW: " << glewGetErrorString(err));
            return false;
        }
        else
            bGlewInit = true;
    }

#ifdef DBG_UTIL
    // only enable debug output in dbgutil build
    // somehow there are implementations where the feature is present and the function
    // pointer is still NULL
    if( GLEW_ARB_debug_output && glDebugMessageCallback )
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(&debug_callback, NULL);
    }

#endif

    SAL_INFO("vcl.opengl", "X11OpenGLContext::ImplInit----end");
    mbInitialized = true;
    return true;
}

void X11OpenGLContext::setWinPosAndSize(const Point &rPos, const Size& rSize)
{
    if(m_pWindow)
        m_pWindow->SetPosSizePixel(rPos, rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetPosSizePixel(rPos, rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void X11OpenGLContext::setWinSize(const Size& rSize)
{
    if(m_pWindow)
        m_pWindow->SetSizePixel(rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetSizePixel(rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void X11OpenGLContext::renderToFile()
{
    int iWidth = m_aGLWin.Width;
    int iHeight = m_aGLWin.Height;
    static int nIdx = 0;
    OUString aName = OUString( "file:///home/moggi/Documents/work/output" ) + OUString::number( nIdx++ ) + ".png";
    OpenGLHelper::renderToFile(iWidth, iHeight, aName);
}

bool X11OpenGLContext::isInitialized() const
{
    return mbInitialized;
}

bool X11OpenGLContext::initWindow()
{
    const SystemEnvData* pChildSysData = 0;
    SystemWindowData winData = vcl::generateSystemWindowData(mpWindow);
    if( winData.pVisual )
    {
        if( !m_pChildWindow )
        {
            m_pChildWindow = new SystemChildWindow(mpWindow, 0, &winData, false);
            m_pChildWindowGC.reset(m_pChildWindow);
        }
        pChildSysData = m_pChildWindow->GetSystemData();
    }

    if (!m_pChildWindow || !pChildSysData)
        return false;

    m_pChildWindow->SetMouseTransparent( true );
    m_pChildWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
    m_pChildWindow->EnableEraseBackground( false );
    m_pChildWindow->SetControlForeground();
    m_pChildWindow->SetControlBackground();

    m_aGLWin.dpy = reinterpret_cast<Display*>(pChildSysData->pDisplay);
    m_aGLWin.win = pChildSysData->aWindow;
    m_aGLWin.screen = pChildSysData->nScreen;

    // Get visual info
    {
        Visual* pVisual = (Visual*)pChildSysData->pVisual;
        XVisualInfo aTemplate;
        aTemplate.visualid = XVisualIDFromVisual( pVisual );;
        int nVisuals = 0;
        XVisualInfo* pInfos = XGetVisualInfo( m_aGLWin.dpy, VisualIDMask, &aTemplate, &nVisuals );
        if( nVisuals != 1 )
            SAL_WARN( "vcl.opengl", "match count for visual id is not 1" );
        m_aGLWin.vi = pInfos;
    }

    // Check multi sample support
    int nSamples = 0;
    glXGetConfig(m_aGLWin.dpy, m_aGLWin.vi, GLX_SAMPLES, &nSamples);
    if( nSamples > 0 )
        m_aGLWin.bMultiSampleSupported = true;

    m_aGLWin.GLXExtensions = glXQueryExtensionsString( m_aGLWin.dpy, m_aGLWin.screen );
    SAL_INFO("vcl.opengl", "available GLX extensions: " << m_aGLWin.GLXExtensions);

    return true;
}

void X11OpenGLContext::makeCurrent()
{
    glXMakeCurrent( m_aGLWin.dpy, m_aGLWin.win, m_aGLWin.ctx );
}

void X11OpenGLContext::swapBuffers()
{
    glXSwapBuffers(m_aGLWin.dpy, m_aGLWin.win);
}

void X11OpenGLContext::sync()
{
    glXWaitGL();
    XSync(m_aGLWin.dpy, false);
}

void X11OpenGLContext::show()
{
    if (m_pChildWindow)
        m_pChildWindow->Show();
    else if (m_pWindow)
        m_pWindow->Show();
}

bool X11OpenGLContext::hasGLExtension(const char *const pName) const
{
    return m_aGLWin.HasGLExtension(pName);
}

unsigned X11OpenGLContext::getWidth() const
{
    return m_aGLWin.Width;
}

unsigned X11OpenGLContext::getHeight() const
{
    return m_aGLWin.Height;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
