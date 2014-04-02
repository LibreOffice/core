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

#if defined( _WIN32 )
static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        return 0;
    case WM_KEYDOWN:
        switch(wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;

        case VK_SPACE:
            break;
        }
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

int InitTempWindow(HWND *hwnd, int width, int height, PIXELFORMATDESCRIPTOR inPfd, GLWindow glWin)
{
    PIXELFORMATDESCRIPTOR  pfd = inPfd;
    int  pfmt;
    int ret;
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = (LPCSTR)"GLRenderer";
    RegisterClass(&wc);
    *hwnd = CreateWindow(wc.lpszClassName, NULL, WS_DISABLED, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);
    glWin.hDC = GetDC(*hwnd);
    pfmt = ChoosePixelFormat(glWin.hDC, &pfd);
    if (!pfmt)
    {
        return -1;
    }
    ret = SetPixelFormat(glWin.hDC, pfmt, &pfd);
    if(!ret)
    {
        return -1;
    }
    glWin.hRC = wglCreateContext(glWin.hDC);
    if(!(glWin.hRC))
    {
        return -1;
    }
    ret = wglMakeCurrent(glWin.hDC, glWin.hRC);
    if(!ret)
    {
        return -1;
    }
    return 0;
}

bool WGLisExtensionSupported(const char *extension)
{
    const size_t extlen = strlen(extension);
    const char *supported = NULL;

    // Try To Use wglGetExtensionStringARB On Current DC, If Possible
    PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtString)
        supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
    // If That Failed, Try Standard Opengl Extensions String
    if (supported == NULL)
        supported = (char*)glGetString(GL_EXTENSIONS);
    // If That Failed Too, Must Be No Extensions Supported
    if (supported == NULL)
        return 0;

    // Begin Examination At Start Of String, Increment By 1 On False Match
    for (const char* p = supported; ; p++)
    {
        // Advance p Up To The Next Possible Match
        p = strstr(p, extension);

        if (p == NULL)
            return 0; // No Match

        // Make Sure That Match Is At The Start Of The String Or That
        // The Previous Char Is A Space, Or Else We Could Accidentally
        // Match "wglFunkywglExtension" With "wglExtension"

        // Also, Make Sure That The Following Character Is Space Or NULL
        // Or Else "wglExtensionTwo" Might Match "wglExtension"
        if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
            return 1; // Match
    }
}

bool InitMultisample(PIXELFORMATDESCRIPTOR pfd, int& rPixelFormat)
{
    HWND hWnd = NULL;
    GLWindow glWin;
    //create a temp windwo to check whether support multi-sample, if support, get the format
    if (InitTempWindow(&hWnd, 1, 1, pfd, glWin) < 0)
    {
        SAL_WARN("vcl.opengl", "Can't create temp window to test");
        return false;
    }

    // See If The String Exists In WGL!
    if (!WGLisExtensionSupported("WGL_ARB_multisample"))
    {
        SAL_WARN("vcl.opengl", "Device doesn't support multi sample");
        return false;
    }
    // Get Our Pixel Format
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        return false;
    }
    // Get Our Current Device Context
    HDC hDC = GetDC(hWnd);

    int pixelFormat;
    int valid;
    UINT    numFormats;
    float   fAttributes[] = {0,0};
    // These Attributes Are The Bits We Want To Test For In Our Sample
    // Everything Is Pretty Standard, The Only One We Want To
    // Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
    // These Two Are Going To Do The Main Testing For Whether Or Not
    // We Support Multisampling On This Hardware.
    int iAttributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
        WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,24,
        WGL_ALPHA_BITS_ARB,8,
        WGL_DEPTH_BITS_ARB,16,
        WGL_STENCIL_BITS_ARB,0,
        WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
        WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
        WGL_SAMPLES_ARB,8,
        0,0
    };

    bool bArbMultisampleSupported = true;

    // First We Check To See If We Can Get A Pixel Format For 4 Samples
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    // If We Returned True, And Our Format Count Is Greater Than 1
    if (valid && numFormats >= 1)
    {
        bArbMultisampleSupported = true;
        rPixelFormat = pixelFormat;
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return bArbMultisampleSupported;
    }
    // Our Pixel Format With 4 Samples Failed, Test For 2 Samples
    iAttributes[19] = 2;
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    if (valid && numFormats >= 1)
    {
        bArbMultisampleSupported = true;
        rPixelFormat = pixelFormat;
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return bArbMultisampleSupported;
    }
    // Return The Valid Format
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glWin.hRC);
    ReleaseDC(hWnd, glWin.hDC);
    DestroyWindow(hWnd);

    return  bArbMultisampleSupported;
}
#endif

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

bool OpenGLContext::init( Window* pParent )
{
    m_pWindow.reset(pParent ? pParent : new Window(0, WB_NOBORDER|WB_NODIALOGCONTROL));
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
    bool bMultiSampleSupport = InitMultisample(PixelFormatFront, WindowPix);
    if (bMultiSampleSupport)
    {
        m_aGLWin.bMultiSampleSupported = true;
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

    SAL_INFO("vcl.opengl", "OpenGLContext::init----end");
    return true;
}

void OpenGLContext::setWinSize(const Size& rSize)
{
    m_pWindow->SetSizePixel(rSize);
    m_pChildWindow->SetSizePixel(rSize);
}

GLWindow& OpenGLContext::getOpenGLWindow()
{
    return m_aGLWin;
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

    if(best_num_samp > 0)
        m_aGLWin.bMultiSampleSupported = true;

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
