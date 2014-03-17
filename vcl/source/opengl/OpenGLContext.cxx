/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/OpenGLContext.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

using namespace com::sun::star;

OpenGLContext::OpenGLContext()
{
}

OpenGLContext::~OpenGLContext()
{
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
        default:
            ;
    }

    return "unkown";
}

extern "C" void
#if defined _WIN32
APIENTRY
#endif
debug_callback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei , const GLchar* message, GLvoid* )
{
    SAL_WARN("vcl.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << " with message: " << message);
}

}

#endif

#if defined UNX && !defined MACOSX

namespace {

static bool errorTriggered;
int oglErrorHandler( Display* /*dpy*/, XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

}

#endif

bool OpenGLContext::init()
{
    m_pWindow.reset(new Window(0, WB_NOBORDER|WB_NODIALOGCONTROL));
    SAL_INFO("vcl.opengl", "OpenGLContext::OpenGLContext----start");
    initWindow();
    m_pWindow->setPosSizePixel(0,0,0,0);
    m_aGLWin.Width = 0;
    m_aGLWin.Height = 0;

#if defined( WNT )
    m_aGLWin.hDC = GetDC(m_aGLWin.hWnd);
#elif defined( MACOSX )

#elif defined( UNX )
    m_aGLWin.ctx = glXCreateContext(m_aGLWin.dpy,
                                 m_aGLWin.vi,
                                 0,
                                 GL_TRUE);
    if( m_aGLWin.ctx == NULL )
    {
        OSL_TRACE("unable to create GLX context");
        return false;
    }
#endif

#if defined( WNT )
    PIXELFORMATDESCRIPTOR PixelFormatFront =                    // PixelFormat Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        (BYTE)32,                       // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        64,                             // 32 bit Z-BUFFER
        0,                              // 0 bit stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    //  we must check whether can set the MSAA
    int WindowPix;
    rGLRender.InitMultisample(PixelFormatFront);
    if (rGLRender.GetMSAASupport())
    {
        WindowPix = rGLRender.GetMSAAFormat();
    }
    else
    {
        WindowPix = ChoosePixelFormat(m_aGLWin.hDC,&PixelFormatFront);
    }
    SetPixelFormat(m_aGLWin.hDC,WindowPix,&PixelFormatFront);
    m_aGLWin.hRC  = wglCreateContext(m_aGLWin.hDC);
    wglMakeCurrent(m_aGLWin.hDC,m_aGLWin.hRC);

#elif defined( MACOSX )

#elif defined( UNX )
    if( !glXMakeCurrent( m_aGLWin.dpy, m_aGLWin.win, m_aGLWin.ctx ) )
    {
        OSL_TRACE("unable to select current GLX context");
        return false;
    }

    int glxMinor, glxMajor;
    double nGLXVersion = 0;
    if( glXQueryVersion( m_aGLWin.dpy, &glxMajor, &glxMinor ) )
      nGLXVersion = glxMajor + 0.1*glxMinor;
    OSL_TRACE("available GLX version: %f", nGLXVersion);

    m_aGLWin.GLExtensions = glGetString( GL_EXTENSIONS );
    OSL_TRACE("available GL  extensions: %s", m_aGLWin.GLExtensions);

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
            OSL_TRACE("error when trying to set swap interval, NVIDIA or Mesa bug?");
        else
            OSL_TRACE("set swap interval to 1 (enable vsync)");

        // restore the error handler
        XSetErrorHandler( oldHandler );
        }
    }

#endif

    //rGLRender.InitOpenGL(m_aGLWin);

#ifdef DBG_UTIL
    // only enable debug output in dbgutil build
    if( GLEW_ARB_debug_output )
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(&debug_callback, NULL);
    }

#endif

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

#if defined( WNT )
    SwapBuffers(m_aGLWin.hDC);
    glFlush();
#elif defined( MACOSX )

#elif defined( UNX )
    glXSwapBuffers(m_aGLWin.dpy, m_aGLWin.win);
#endif
    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    SAL_INFO("vcl.opengl", "OpenGLContext::init----end");
    return true;
}

void OpenGLContext::setWinSize(const Size& rSize)
{
    m_pWindow->SetSizePixel(rSize);
    m_pChildWindow->SetSizePixel(rSize);
}

#if defined( WNT )

bool OpenGLContext::initWindow()
{
    const SystemEnvData* sysData(m_pWindow->GetSystemData());
    m_aGLWin.hWnd = sysData->hWnd;
    SystemWindowData winData;
    winData.nSize = sizeof(winData);
    m_pChildWindow.reset(new SystemChildWindow(m_pWindow.get(), 0, &winData, sal_False));


    if( m_pChildWindow )
    {
        m_pChildWindow->SetMouseTransparent( sal_True );
        m_pChildWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        m_pChildWindow->EnableEraseBackground( sal_False );
        m_pChildWindow->SetControlForeground();
        m_pChildWindow->SetControlBackground();
        m_pChildWindow->EnablePaint(sal_False);
        m_aGLWin.hWnd = sysData->hWnd;
    }

    return true;
}

#elif defined( MACOSX )

bool OpenGLContext::initWindow()
{
    return false;
}

#elif defined( UNX )

namespace {

// we need them before glew can initialize them
// glew needs an OpenGL context so we need to get the address manually
void initOpenGLFunctionPointers()
{
    glXChooseFBConfig = (GLXFBConfig*(*)(Display *dpy, int screen, const int *attrib_list, int *nelements))glXGetProcAddressARB((GLubyte*)"glXChooseFBConfig");
    glXGetVisualFromFBConfig = (XVisualInfo*(*)(Display *dpy, GLXFBConfig config))glXGetProcAddressARB((GLubyte*)"glXGetVisualFromFBConfig");    // try to find a visual for the current set of attributes
    glXGetFBConfigAttrib = (int(*)(Display *dpy, GLXFBConfig config, int attribute, int* value))glXGetProcAddressARB((GLubyte*)"glXGetFBConfigAttrib");

}

}

bool OpenGLContext::initWindow()
{
    const SystemEnvData* sysData(m_pWindow->GetSystemData());

    m_aGLWin.dpy = reinterpret_cast<Display*>(sysData->pDisplay);

    if( !glXQueryExtension( m_aGLWin.dpy, NULL, NULL ) )
        return false;

    m_aGLWin.win = sysData->aWindow;

    OSL_TRACE("parent window: %d", m_aGLWin.win);

    XWindowAttributes xattr;
    XGetWindowAttributes( m_aGLWin.dpy, m_aGLWin.win, &xattr );

    m_aGLWin.screen = XScreenNumberOfScreen( xattr.screen );

    static int visual_attribs[] =
    {
        GLX_RED_SIZE,           8,
        GLX_GREEN_SIZE,         8,
        GLX_BLUE_SIZE,          8,
        GLX_ALPHA_SIZE,         8,
        GLX_DEPTH_SIZE,         24,
        GLX_X_VISUAL_TYPE,      GLX_TRUE_COLOR,
        None
    };

    const SystemEnvData* pChildSysData = NULL;
    m_pChildWindow.reset();

    initOpenGLFunctionPointers();

    int fbCount = 0;
    GLXFBConfig* pFBC = glXChooseFBConfig( m_aGLWin.dpy,
            m_aGLWin.screen,
            visual_attribs, &fbCount );

    if(!pFBC)
    {
        SAL_WARN("vcl.opengl", "no suitable fb format found");
        return false;
    }

    int best_fbc = -1, best_num_samp = -1;
    for(int i = 0; i < fbCount; ++i)
    {
        XVisualInfo* pVi = glXGetVisualFromFBConfig( m_aGLWin.dpy, pFBC[i] );
        if(pVi)
        {
            // pick the one with the most samples per pixel
            int nSampleBuf = 0;
            int nSamples = 0;
            glXGetFBConfigAttrib( m_aGLWin.dpy, pFBC[i], GLX_SAMPLE_BUFFERS, &nSampleBuf );
            glXGetFBConfigAttrib( m_aGLWin.dpy, pFBC[i], GLX_SAMPLES       , &nSamples  );

            if ( best_fbc < 0 || (nSampleBuf && ( nSamples > best_num_samp )) )
            {
                best_fbc = i;
                best_num_samp = nSamples;
            }
        }
        XFree( pVi );
    }

    XVisualInfo* vi = glXGetVisualFromFBConfig( m_aGLWin.dpy, pFBC[best_fbc] );
    if( vi )
    {
        SystemWindowData winData;
        winData.nSize = sizeof(winData);
        OSL_TRACE("using VisualID %08X", vi->visualid);
        winData.pVisual = (void*)(vi->visual);
        m_pChildWindow.reset(new SystemChildWindow(m_pWindow.get(), 0, &winData, false));
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
    m_aGLWin.vi = vi;
    m_aGLWin.GLXExtensions = glXQueryExtensionsString( m_aGLWin.dpy, m_aGLWin.screen );
    OSL_TRACE("available GLX extensions: %s", m_aGLWin.GLXExtensions);

    return true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
