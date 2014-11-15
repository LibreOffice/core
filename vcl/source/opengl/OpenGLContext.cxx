/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include <boost/scoped_array.hpp>
#include <vcl/pngwrite.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>

#if defined(MACOSX)
#include <premac.h>
#include "OpenGLWrapper.hxx"
#include <postmac.h>
#endif

using namespace com::sun::star;

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
static std::vector< GLXContext > vShareList;
#endif

GLWindow::~GLWindow()
{
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
    XFree(vi);
#endif
}

OpenGLContext::OpenGLContext():
    mpWindow(NULL),
    m_pChildWindow(NULL),
    mbInitialized(false),
    mbRequestLegacyContext(false),
    mbUseDoubleBufferedRendering(true),
    mbRequestVirtualDevice(false)
{
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
    mbPixmap = false;
#endif
}

OpenGLContext::~OpenGLContext()
{
#if defined( WNT )
    if (m_aGLWin.hRC)
    {
        wglMakeCurrent( m_aGLWin.hDC, 0 );
        wglDeleteContext( m_aGLWin.hRC );
        ReleaseDC( m_aGLWin.hWnd, m_aGLWin.hDC );
    }
#elif defined( MACOSX )
    OpenGLWrapper::resetCurrent();
#elif defined( IOS ) || defined( ANDROID )
    // nothing
#elif defined( UNX )
    if(m_aGLWin.ctx)
    {
        std::remove( vShareList.begin(), vShareList.end(), m_aGLWin.ctx );

        glXMakeCurrent(m_aGLWin.dpy, None, NULL);
        if( glGetError() != GL_NO_ERROR )
        {
            SAL_WARN("vcl.opengl", "glError: " << (char *)gluErrorString(glGetError()));
        }
        glXDestroyContext(m_aGLWin.dpy, m_aGLWin.ctx);

        if (mbPixmap)
            glXDestroyGLXPixmap(m_aGLWin.dpy, m_aGLWin.glPix);
    }
#endif
}

void OpenGLContext::requestLegacyContext()
{
    mbRequestLegacyContext = true;
}

void OpenGLContext::requestSingleBufferedRendering()
{
    mbUseDoubleBufferedRendering = false;
}

void OpenGLContext::requestVirtualDevice()
{
    mbRequestVirtualDevice = true;
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
        return false;

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

bool InitMultisample(PIXELFORMATDESCRIPTOR pfd, int& rPixelFormat,
        bool bUseDoubleBufferedRendering, bool bRequestVirtualDevice)
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
        WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
        WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
        WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,24,
        WGL_ALPHA_BITS_ARB,8,
        WGL_DEPTH_BITS_ARB,24,
        WGL_STENCIL_BITS_ARB,0,
        WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
        WGL_SAMPLES_ARB,8,
        0,0
    };

    if (!bUseDoubleBufferedRendering)
        iAttributes[1] = GL_FALSE;

    if (bRequestVirtualDevice)
    {
        iAttributes[2] = WGL_DRAW_TO_BITMAP_ARB;
    }

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
        case GL_DEBUG_TYPE_ERROR:
            return "error";
        default:
            ;
    }

    return "unknown";
}

extern "C" void
#if defined _WIN32
APIENTRY
#endif
debug_callback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei , const GLchar* message, GLvoid* )
{
    // ignore Nvidia's : "Program/shader state performance warning: Fragment Shader is going to be recompiled because the shader key based on GL state mismatches."
    // the GLSL compiler is a bit too aggressive in optimizing the state based on the current OpenGL state
    if (id == 131218)
        return;

    SAL_WARN("vcl.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << " with message: " << message);
}

}

#endif

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID

namespace {

static bool errorTriggered;
int oglErrorHandler( Display* /*dpy*/, XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

GLXFBConfig* getFBConfigForPixmap(Display* dpy, int& nBestFBC, bool bUseDoubleBufferedRendering, int screen)
{
    static int visual_attribs[] =
    {
        GLX_DOUBLEBUFFER,       False,
        GLX_DRAWABLE_TYPE,      GLX_PIXMAP_BIT,
        GLX_X_RENDERABLE,       True,
        GLX_RED_SIZE,           8,
        GLX_GREEN_SIZE,         8,
        GLX_BLUE_SIZE,          8,
        GLX_ALPHA_SIZE,         8,
        GLX_DEPTH_SIZE,         24,
        GLX_X_VISUAL_TYPE,      GLX_TRUE_COLOR,
        None
    };

    if (bUseDoubleBufferedRendering)
        visual_attribs[1] = True;

    int fbCount = 0;
    GLXFBConfig* pFBC = glXChooseFBConfig( dpy,
            screen,
            visual_attribs, &fbCount );

    if(!pFBC)
    {
        SAL_WARN("vcl.opengl", "no suitable fb format found");
        return NULL;
    }

    int best_num_samp = -1;
    for(int i = 0; i < fbCount; ++i)
    {
        // pick the one with the most samples per pixel
        int nSampleBuf = 0;
        int nSamples = 0;
        glXGetFBConfigAttrib( dpy, pFBC[i], GLX_SAMPLE_BUFFERS, &nSampleBuf );
        glXGetFBConfigAttrib( dpy, pFBC[i], GLX_SAMPLES       , &nSamples  );

        if ( nBestFBC < 0 || (nSampleBuf && ( nSamples > best_num_samp )) )
        {
            nBestFBC = i;
            best_num_samp = nSamples;
        }
    }

    return pFBC;
}

#ifdef DBG_UTIL
GLXFBConfig* getFBConfig(Display* dpy, Window win, int& nBestFBC, bool bUseDoubleBufferedRendering)
{
    if( dpy == 0 || !glXQueryExtension( dpy, NULL, NULL ) )
        return NULL;

    SAL_INFO("vcl.opengl", "window: " << win);

    XWindowAttributes xattr;
    XGetWindowAttributes( dpy, win, &xattr );

    int screen = XScreenNumberOfScreen( xattr.screen );

    // TODO: moggi: Select colour channel depth based on visual attributes, not hardcoded */
    static int visual_attribs[] =
    {
        GLX_DOUBLEBUFFER,       True,
        GLX_X_RENDERABLE,       True,
        GLX_RED_SIZE,           8,
        GLX_GREEN_SIZE,         8,
        GLX_BLUE_SIZE,          8,
        GLX_ALPHA_SIZE,         8,
        GLX_DEPTH_SIZE,         24,
        GLX_X_VISUAL_TYPE,      GLX_TRUE_COLOR,
        None
    };

    if (!bUseDoubleBufferedRendering)
        visual_attribs[1] = False;

    int fbCount = 0;
    GLXFBConfig* pFBC = glXChooseFBConfig( dpy,
            screen,
            visual_attribs, &fbCount );

    if(!pFBC)
    {
        SAL_WARN("vcl.opengl", "no suitable fb format found");
        return NULL;
    }

    int best_num_samp = -1;
    for(int i = 0; i < fbCount; ++i)
    {
        XVisualInfo* pVi = glXGetVisualFromFBConfig( dpy, pFBC[i] );
        if(pVi && pVi->visualid == xattr.visual->visualid)
        {
            // pick the one with the most samples per pixel
            int nSampleBuf = 0;
            int nSamples = 0;
            glXGetFBConfigAttrib( dpy, pFBC[i], GLX_SAMPLE_BUFFERS, &nSampleBuf );
            glXGetFBConfigAttrib( dpy, pFBC[i], GLX_SAMPLES       , &nSamples  );

            if ( nBestFBC < 0 || (nSampleBuf && ( nSamples > best_num_samp )) )
            {
                nBestFBC = i;
                best_num_samp = nSamples;
            }
        }
        XFree( pVi );
    }

    return pFBC;
}
#endif

// we need them before glew can initialize them
// glew needs an OpenGL context so we need to get the address manually
void initOpenGLFunctionPointers()
{
    glXChooseFBConfig = (GLXFBConfig*(*)(Display *dpy, int screen, const int *attrib_list, int *nelements))glXGetProcAddressARB((GLubyte*)"glXChooseFBConfig");
    glXGetVisualFromFBConfig = (XVisualInfo*(*)(Display *dpy, GLXFBConfig config))glXGetProcAddressARB((GLubyte*)"glXGetVisualFromFBConfig");    // try to find a visual for the current set of attributes
    glXGetFBConfigAttrib = (int(*)(Display *dpy, GLXFBConfig config, int attribute, int* value))glXGetProcAddressARB((GLubyte*)"glXGetFBConfigAttrib");
    glXCreateContextAttribsARB = (GLXContext(*) (Display*, GLXFBConfig, GLXContext, Bool, const int*)) glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");;
}

Visual* getVisual(Display* dpy, Window win)
{
    initOpenGLFunctionPointers();

    XWindowAttributes xattr;
    XGetWindowAttributes( dpy, win, &xattr );
    SAL_INFO("vcl.opengl", "using VisualID " << xattr.visual);
    return xattr.visual;
}

}

#endif

bool OpenGLContext::init( vcl::Window* pParent )
{
    if(mbInitialized)
        return true;

    m_pWindow.reset(pParent ? NULL : new vcl::Window(0, WB_NOBORDER|WB_NODIALOGCONTROL));
    mpWindow = pParent ? pParent : m_pWindow.get();
    if(m_pWindow)
        m_pWindow->setPosSizePixel(0,0,0,0);
    m_pChildWindow = 0;
    initWindow();
    return ImplInit();
}

bool OpenGLContext::init(SystemChildWindow* pChildWindow)
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

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID
bool OpenGLContext::init(Display* dpy, Window win, int screen)
{
    if(mbInitialized)
        return true;

    if (!dpy)
        return false;

    m_aGLWin.dpy = dpy;
    m_aGLWin.win = win;
    m_aGLWin.screen = screen;

    Visual* pVisual = getVisual(dpy, win);

    initGLWindow(pVisual);

    return ImplInit();
}

bool OpenGLContext::init(Display* dpy, Pixmap pix, unsigned int width, unsigned int height, int nScreen)
{
    if(mbInitialized)
        return true;

    if (!dpy)
        return false;

    SAL_INFO("vcl.opengl", "init with pixmap");
    m_aGLWin.dpy = dpy;
    m_aGLWin.Width = width;
    m_aGLWin.Height = height;
    m_aGLWin.pix = pix;
    const int attrib_list[] = { GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
          GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
          None};
    int best_fbc = -1;
    GLXFBConfig* config = getFBConfigForPixmap(dpy, best_fbc, mbUseDoubleBufferedRendering, nScreen);
    if (best_fbc == -1)
        return false;

    m_aGLWin.vi = glXGetVisualFromFBConfig( dpy, config[best_fbc] );
    m_aGLWin.glPix = glXCreatePixmap(dpy, config[best_fbc], pix, attrib_list);

    mbPixmap = true;

    initOpenGLFunctionPointers();

    return ImplInit();
}

bool OpenGLContext::ImplInit()
{
    GLXContext pSharedCtx( NULL );

    SAL_INFO("vcl.opengl", "OpenGLContext::ImplInit----start");

    if( !vShareList.empty() )
        pSharedCtx = vShareList.front();

#ifdef DBG_UTIL
    if (!mbPixmap && glXCreateContextAttribsARB && !mbRequestLegacyContext)
    {
        int best_fbc = -1;
        GLXFBConfig* pFBC = getFBConfig(m_aGLWin.dpy, m_aGLWin.win, best_fbc, mbUseDoubleBufferedRendering);
        if (!pFBC)
            return false;

        if (best_fbc != -1)
        {
            int nContextAttribs[] =
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                None
            };
            m_aGLWin.ctx = glXCreateContextAttribsARB(m_aGLWin.dpy, pFBC[best_fbc], pSharedCtx, GL_TRUE, nContextAttribs);
            SAL_INFO_IF(m_aGLWin.ctx, "vcl.opengl", "created a 3.2 core context");
        }
        else
            SAL_WARN("vcl.opengl", "unable to find correct FBC");

    }
#endif

    if (!m_aGLWin.ctx)
    {
        if (!m_aGLWin.dpy || !m_aGLWin.vi)
           return false;

        m_aGLWin.ctx = m_aGLWin.dpy == 0 ? 0 : glXCreateContext(m_aGLWin.dpy,
                m_aGLWin.vi,
                pSharedCtx,
                GL_TRUE);
    }



    if( m_aGLWin.ctx )
    {
        vShareList.push_back( m_aGLWin.ctx );
    }
    else
    {
        SAL_WARN("vcl.opengl", "unable to create GLX context");
        return false;
    }

    if( !glXMakeCurrent( m_aGLWin.dpy, mbPixmap ? m_aGLWin.glPix : m_aGLWin.win, m_aGLWin.ctx ) )
    {
        SAL_WARN("vcl.opengl", "unable to select current GLX context");
        return false;
    }

    int glxMinor, glxMajor;
    double nGLXVersion = 0;
    if( glXQueryVersion( m_aGLWin.dpy, &glxMajor, &glxMinor ) )
      nGLXVersion = glxMajor + 0.1*glxMinor;
    SAL_INFO("vcl.opengl", "available GLX version: " << nGLXVersion);

    m_aGLWin.GLExtensions = glGetString( GL_EXTENSIONS );
    SAL_INFO("vcl.opengl", "available GL  extensions: " << m_aGLWin.GLExtensions);

    XWindowAttributes xWinAttr;
    XGetWindowAttributes( m_aGLWin.dpy, m_aGLWin.win, &xWinAttr );
    m_aGLWin.Width = xWinAttr.width;
    m_aGLWin.Height = xWinAttr.height;

    if( m_aGLWin.HasGLXExtension("GLX_SGI_swap_control" ) )
    {
        // enable vsync
        typedef GLint (*glXSwapIntervalProc)(GLint);
        glXSwapIntervalProc glXSwapInterval = (glXSwapIntervalProc) glXGetProcAddress( (const GLubyte*) "glXSwapIntervalSGI" );
        if( glXSwapInterval )
        {
            int (*oldHandler)(Display* /*dpy*/, XErrorEvent* /*evnt*/);

            XLockDisplay(m_aGLWin.dpy);
            XSync(m_aGLWin.dpy, false);
            // replace error handler temporarily
            oldHandler = XSetErrorHandler( oglErrorHandler );

            errorTriggered = false;

            glXSwapInterval( 1 );

            // sync so that we possibly get an XError
            glXWaitGL();
            XSync(m_aGLWin.dpy, false);

            if( errorTriggered )
                SAL_WARN("vcl.opengl", "error when trying to set swap interval, NVIDIA or Mesa bug?");
            else
                SAL_INFO("vcl.opengl", "set swap interval to 1 (enable vsync)");

            // restore the error handler
            XSetErrorHandler( oldHandler );
            XUnlockDisplay(m_aGLWin.dpy);
        }
    }
    return InitGLEW();
}

#elif defined( _WIN32 )

bool OpenGLContext::init(HDC hDC, HWND hWnd)
{
    if (mbInitialized)
        return false;

    m_aGLWin.hDC = hDC;
    m_aGLWin.hWnd = hWnd;
    return ImplInit();
}

bool OpenGLContext::ImplInit()
{
    SAL_INFO("vcl.opengl", "OpenGLContext::ImplInit----start");
    PIXELFORMATDESCRIPTOR PixelFormatFront = // PixelFormat Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_SUPPORT_OPENGL,
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

    if (mbUseDoubleBufferedRendering)
        PixelFormatFront.dwFlags |= PFD_DOUBLEBUFFER;

    if (mbRequestVirtualDevice)
        PixelFormatFront.dwFlags |= PFD_DRAW_TO_BITMAP;
    else
        PixelFormatFront.dwFlags |= PFD_DRAW_TO_WINDOW;

    //  we must check whether can set the MSAA
    int WindowPix = 0;
    bool bMultiSampleSupport = InitMultisample(PixelFormatFront, WindowPix,
            mbUseDoubleBufferedRendering, mbRequestVirtualDevice);
    if (bMultiSampleSupport && WindowPix != 0)
    {
        m_aGLWin.bMultiSampleSupported = true;
    }
    else
    {
        WindowPix = ChoosePixelFormat(m_aGLWin.hDC, &PixelFormatFront);
    }

    if (WindowPix == 0)
    {
        SAL_WARN("vcl.opengl", "Invalid pixelformat");
        return false;
    }

    SetPixelFormat(m_aGLWin.hDC, WindowPix, &PixelFormatFront);
    m_aGLWin.hRC = wglCreateContext(m_aGLWin.hDC);
    if (m_aGLWin.hRC == NULL)
    {
        SAL_WARN("vcl.opengl", "wglCreateContext failed");
        return false;
    }

    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        SAL_WARN("vcl.opengl", "wglMakeCurrent failed: " << GetLastError());
        return false;
    }

    RECT clientRect;
    GetClientRect(WindowFromDC(m_aGLWin.hDC), &clientRect);
    m_aGLWin.Width = clientRect.right - clientRect.left;
    m_aGLWin.Height = clientRect.bottom - clientRect.top;

    return InitGLEW();
}

#elif defined( MACOSX )

bool OpenGLContext::ImplInit()
{
    SAL_INFO("vcl.opengl", "OpenGLContext::ImplInit----start");
    NSOpenGLView* pView = getOpenGLView();
    OpenGLWrapper::makeCurrent(pView);

    return InitGLEW();
}

#else

bool OpenGLContext::ImplInit()
{
    SAL_INFO("vcl.opengl", "OpenGLContext not implemented for this platform");
    return false;
}

#endif

bool OpenGLContext::InitGLEW()
{
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
    if( GLEW_ARB_debug_output)
    {
        if (glDebugMessageCallbackARB)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallbackARB(&debug_callback, NULL);
        }
        else if ( glDebugMessageCallback )
        {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(&debug_callback, NULL);
        }
    }

#endif

    SAL_INFO("vcl.opengl", "OpenGLContext::ImplInit----end");
    mbInitialized = true;
    return true;
}

void OpenGLContext::setWinPosAndSize(const Point &rPos, const Size& rSize)
{
    if(m_pWindow)
        m_pWindow->SetPosSizePixel(rPos, rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetPosSizePixel(rPos, rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void OpenGLContext::setWinSize(const Size& rSize)
{
    if(m_pWindow)
        m_pWindow->SetSizePixel(rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetSizePixel(rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void OpenGLContext::renderToFile()
{
    int iWidth = m_aGLWin.Width;
    int iHeight = m_aGLWin.Height;
    static int nIdx = 0;
    OUString aName = OUString( "file:///home/moggi/Documents/work/output" ) + OUString::number( nIdx++ ) + ".png";
    OpenGLHelper::renderToFile(iWidth, iHeight, aName);
}

#if defined( WNT )

bool OpenGLContext::initWindow()
{
    if( !m_pChildWindow )
    {
        SystemWindowData winData = generateWinData(mpWindow, false);
        m_pChildWindow = new SystemChildWindow(mpWindow, 0, &winData, false);
        m_pChildWindowGC.reset(m_pChildWindow);
    }

    if( m_pChildWindow )
    {
        m_pChildWindow->SetMouseTransparent( true );
        m_pChildWindow->SetParentClipMode(PARENTCLIPMODE_CLIP);
        m_pChildWindow->EnableEraseBackground( false );
        m_pChildWindow->SetControlForeground();
        m_pChildWindow->SetControlBackground();
        //m_pChildWindow->EnablePaint(false);

        const SystemEnvData* sysData(m_pChildWindow->GetSystemData());
        m_aGLWin.hWnd = sysData->hWnd;
    }

    m_aGLWin.hDC = GetDC(m_aGLWin.hWnd);
    return true;
}

#elif defined( MACOSX )

bool OpenGLContext::initWindow()
{
    if( !m_pChildWindow )
    {
        SystemWindowData winData = generateWinData(mpWindow, mbRequestLegacyContext);
        m_pChildWindow = new SystemChildWindow(mpWindow, 0, &winData, false);
        m_pChildWindowGC.reset(m_pChildWindow);
    }

    if( m_pChildWindow )
    {
        m_pChildWindow->SetMouseTransparent( true );
        m_pChildWindow->SetParentClipMode(PARENTCLIPMODE_CLIP);
        m_pChildWindow->EnableEraseBackground( false );
        m_pChildWindow->SetControlForeground();
        m_pChildWindow->SetControlBackground();
        //m_pChildWindow->EnablePaint(false);

    }

    return true;
}

#elif defined( IOS ) || defined( ANDROID )

bool OpenGLContext::initWindow()
{
    return false;
}

#elif defined( UNX )

bool OpenGLContext::initWindow()
{
    const SystemEnvData* pChildSysData = 0;
    SystemWindowData winData = generateWinData(mpWindow, false);
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

    Visual* pVisual = (Visual*)pChildSysData->pVisual;
    initGLWindow(pVisual);

    return true;
}

void OpenGLContext::initGLWindow(Visual* pVisual)
{
    // Get visual info
    {
        XVisualInfo aTemplate;
        aTemplate.visualid = XVisualIDFromVisual( pVisual );
        int nVisuals = 0;
        XVisualInfo* pInfos = XGetVisualInfo( m_aGLWin.dpy, VisualIDMask, &aTemplate, &nVisuals );
        if( nVisuals != 1 )
            SAL_WARN( "vcl.opengl", "match count for visual id is not 1" );
        m_aGLWin.vi = pInfos;
    }

    // Check multi sample support
    /* TODO: moggi: This is not necessarily correct in the DBG_UTIL path, as it picks
     *      an FBConfig instead ... */
    int nSamples = 0;
    glXGetConfig(m_aGLWin.dpy, m_aGLWin.vi, GLX_SAMPLES, &nSamples);
    if( nSamples > 0 )
        m_aGLWin.bMultiSampleSupported = true;

    m_aGLWin.GLXExtensions = glXQueryExtensionsString( m_aGLWin.dpy, m_aGLWin.screen );
    SAL_INFO("vcl.opengl", "available GLX extensions: " << m_aGLWin.GLXExtensions);
}

#endif

#if defined( WNT ) || defined( MACOSX ) || defined( IOS ) || defined( ANDROID )

SystemWindowData OpenGLContext::generateWinData(vcl::Window* /*pParent*/, bool bRequestLegacyContext)
{
    (void) bRequestLegacyContext;
    SystemWindowData aWinData;
#if defined(MACOSX)
    aWinData.bOpenGL = true;
    aWinData.bLegacy = bRequestLegacyContext;
#endif
    aWinData.nSize = sizeof(aWinData);
    return aWinData;
}

#elif defined( UNX )

SystemWindowData OpenGLContext::generateWinData(vcl::Window* pParent, bool)
{
    SystemWindowData aWinData;
    aWinData.nSize = sizeof(aWinData);
    aWinData.pVisual = NULL;

    const SystemEnvData* sysData(pParent->GetSystemData());

    Display *dpy = reinterpret_cast<Display*>(sysData->pDisplay);
    Window win = sysData->aWindow;

    if( dpy == 0 || !glXQueryExtension( dpy, NULL, NULL ) )
        return aWinData;

    aWinData.pVisual = getVisual(dpy, win);

    return aWinData;
}

#endif

void OpenGLContext::makeCurrent()
{
#if defined( WNT )
    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        SAL_WARN("vcl.opengl", "OpenGLContext::makeCurrent(): wglMakeCurrent failed: " << GetLastError());
    }
#elif defined( MACOSX )
    NSOpenGLView* pView = getOpenGLView();
    OpenGLWrapper::makeCurrent(pView);
#elif defined( IOS ) || defined( ANDROID )
    // nothing
#elif defined( UNX )
    if (!glXMakeCurrent( m_aGLWin.dpy, mbPixmap ? m_aGLWin.glPix : m_aGLWin.win, m_aGLWin.ctx ))
        SAL_WARN("vcl.opengl", "OpenGLContext::makeCurrent failed");
#endif
}

void OpenGLContext::resetCurrent()
{
#if defined( WNT )
    wglMakeCurrent( m_aGLWin.hDC, 0 );
#elif defined( MACOSX )
    OpenGLWrapper::resetCurrent();
#elif defined( IOS ) || defined( ANDROID )
    // nothing
#elif defined( UNX )
    glXMakeCurrent(m_aGLWin.dpy, None, NULL);
#endif
}

void OpenGLContext::swapBuffers()
{
#if defined( WNT )
    SwapBuffers(m_aGLWin.hDC);
#elif defined( MACOSX )
    NSOpenGLView* pView = getOpenGLView();
    OpenGLWrapper::swapBuffers(pView);
#elif defined( IOS ) || defined( ANDROID )
    // nothing
#elif defined( UNX )
    glXSwapBuffers(m_aGLWin.dpy, mbPixmap ? m_aGLWin.glPix : m_aGLWin.win);
#endif
}

void OpenGLContext::sync()
{
#if defined( WNT )
    // nothing
#elif defined( MACOSX ) || defined( IOS ) || defined( ANDROID )
    // nothing
#elif defined( UNX )
    glXWaitGL();
    XSync(m_aGLWin.dpy, false);
#endif
}

void OpenGLContext::show()
{
    if (m_pChildWindow)
        m_pChildWindow->Show();
    else if (m_pWindow)
        m_pWindow->Show();
}

SystemChildWindow* OpenGLContext::getChildWindow()
{
    return m_pChildWindow;
}

const SystemChildWindow* OpenGLContext::getChildWindow() const
{
    return m_pChildWindow;
}

bool OpenGLContext::supportMultiSampling() const
{
    return m_aGLWin.bMultiSampleSupported;
}

#if defined(MACOSX)
NSOpenGLView* OpenGLContext::getOpenGLView()
{
    return reinterpret_cast<NSOpenGLView*>(m_pChildWindow->GetSystemData()->mpNSView);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
