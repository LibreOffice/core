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
#include <boost/make_shared.hpp>
#include <vcl/pngwrite.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>

#if defined(MACOSX)
#include <premac.h>
#include "OpenGLWrapper.hxx"
#include <postmac.h>
#endif

#if defined( WNT )
#include <win/saldata.hxx>
#endif

#include "svdata.hxx"

#include <opengl/framebuffer.hxx>
#include <opengl/program.hxx>
#include <opengl/texture.hxx>
#include <opengl/zone.hxx>

using namespace com::sun::star;

#define MAX_FRAMEBUFFER_COUNT 30

// TODO use rtl::Static instead of 'static'
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
static std::vector<GLXContext> g_vShareList;
#elif defined(WNT)
static std::vector<HGLRC> g_vShareList;
#endif

GLWindow::~GLWindow()
{
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    XFree(vi);
#endif
}

OpenGLContext::OpenGLContext():
    mpWindow(NULL),
    m_pChildWindow(NULL),
    mbInitialized(false),
    mnRefCount(1),
    mbRequestLegacyContext(false),
    mbUseDoubleBufferedRendering(true),
    mbRequestVirtualDevice(false),
    mnFramebufferCount(0),
    mpCurrentFramebuffer(NULL),
    mpFirstFramebuffer(NULL),
    mpLastFramebuffer(NULL),
    mpCurrentProgram(NULL),
    mnPainting(0),
    mpPrevContext(NULL),
    mpNextContext(NULL)
{
    SAL_INFO("vcl.opengl", "new context: " << this);

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    mbPixmap = false;
#endif

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maGDIData.mpLastContext )
    {
        pSVData->maGDIData.mpLastContext->mpNextContext = this;
        mpPrevContext = pSVData->maGDIData.mpLastContext;
    }
    else
        pSVData->maGDIData.mpFirstContext = this;
    pSVData->maGDIData.mpLastContext = this;

    // FIXME: better hope we call 'makeCurrent' soon to preserve
    // the invariant that the last item is the current context.
}

OpenGLContext::~OpenGLContext()
{
    SAL_INFO("vcl.opengl", "delete context: " << this);
    reset();

    ImplSVData* pSVData = ImplGetSVData();
    if( mpPrevContext )
        mpPrevContext->mpNextContext = mpNextContext;
    else
        pSVData->maGDIData.mpFirstContext = mpNextContext;
    if( mpNextContext )
        mpNextContext->mpPrevContext = mpPrevContext;
    else
        pSVData->maGDIData.mpLastContext = mpPrevContext;

    m_pChildWindow.disposeAndClear();
}

#ifdef DBG_UTIL
void OpenGLContext::AddRef(SalGraphicsImpl* pImpl)
{
    assert(mnRefCount > 0);
    mnRefCount++;

    maParents.insert(pImpl);
}

void OpenGLContext::DeRef(SalGraphicsImpl* pImpl)
{

    auto it = maParents.find(pImpl);
    if(it != maParents.end())
        maParents.erase(it);

    assert(mnRefCount > 0);
    if( --mnRefCount == 0 )
        delete this;
}
#else
void OpenGLContext::AddRef()
{
    assert(mnRefCount > 0);
    mnRefCount++;
}

void OpenGLContext::DeRef()
{
    assert(mnRefCount > 0);
    if( --mnRefCount == 0 )
        delete this;
}
#endif

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

int InitTempWindow(HWND *hwnd, int width, int height, const PIXELFORMATDESCRIPTOR& inPfd, GLWindow& glWin)
{
    OpenGLZone aZone;

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

    CHECK_GL_ERROR();
    return 0;
}

bool WGLisExtensionSupported(const char *extension)
{
    OpenGLZone aZone;

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

bool InitMultisample(const PIXELFORMATDESCRIPTOR& pfd, int& rPixelFormat,
        bool bUseDoubleBufferedRendering, bool bRequestVirtualDevice)
{
    OpenGLZone aZone;

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

    return bArbMultisampleSupported;
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

#if defined UNX && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)

namespace {

#ifdef DBG_UTIL
int unxErrorHandler(Display* dpy, XErrorEvent* event)
{
    char err[256];
    char req[256];
    char minor[256];
    XGetErrorText(dpy, event->error_code, err, 256);
    XGetErrorText(dpy, event->request_code, req, 256);
    XGetErrorText(dpy, event->minor_code, minor, 256);
    SAL_WARN("vcl.opengl", "Error: " << err << ", Req: " << req << ", Minor: " << minor);
    return 0;
}
#endif

typedef int (*errorHandler)(Display* /*dpy*/, XErrorEvent* /*evnt*/);

class TempErrorHandler
{
private:
    errorHandler oldErrorHandler;
    Display* mdpy;

public:
    TempErrorHandler(Display* dpy, errorHandler newErrorHandler):
        mdpy(dpy)
    {
        XLockDisplay(dpy);
        XSync(dpy, false);
        oldErrorHandler = XSetErrorHandler(newErrorHandler);
    }

    ~TempErrorHandler()
    {
        // sync so that we possibly get an XError
        glXWaitGL();
        XSync(mdpy, false);
        XSetErrorHandler(oldErrorHandler);
        XUnlockDisplay(mdpy);
    }
};

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

    CHECK_GL_ERROR();
    return pFBC;
}

GLXFBConfig* getFBConfig(Display* dpy, Window win, int& nBestFBC, bool bUseDoubleBufferedRendering, bool bWithSameVisualID)
{
    OpenGLZone aZone;

    if( dpy == 0 || !glXQueryExtension( dpy, NULL, NULL ) )
        return NULL;

    SAL_INFO("vcl.opengl", "window: " << win);

    XWindowAttributes xattr;
    if( !XGetWindowAttributes( dpy, win, &xattr ) )
    {
        SAL_WARN("vcl.opengl", "Failed to get window attributes for fbconfig " << win);
        xattr.screen = 0;
        xattr.visual = NULL;
    }

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
        if(pVi && (!bWithSameVisualID || (xattr.visual && pVi->visualid == xattr.visual->visualid)) )
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

// we need them before glew can initialize them
// glew needs an OpenGL context so we need to get the address manually
void initOpenGLFunctionPointers()
{
    glXChooseFBConfig = reinterpret_cast<GLXFBConfig*(*)(Display *dpy, int screen, const int *attrib_list, int *nelements)>(glXGetProcAddressARB(reinterpret_cast<GLubyte const *>("glXChooseFBConfig")));
    glXGetVisualFromFBConfig = reinterpret_cast<XVisualInfo*(*)(Display *dpy, GLXFBConfig config)>(glXGetProcAddressARB(reinterpret_cast<GLubyte const *>("glXGetVisualFromFBConfig")));    // try to find a visual for the current set of attributes
    glXGetFBConfigAttrib = reinterpret_cast<int(*)(Display *dpy, GLXFBConfig config, int attribute, int* value)>(glXGetProcAddressARB(reinterpret_cast<GLubyte const *>("glXGetFBConfigAttrib")));
    glXCreateContextAttribsARB = reinterpret_cast<GLXContext(*)(Display*, GLXFBConfig, GLXContext, Bool, const int*)>(glXGetProcAddressARB(reinterpret_cast<const GLubyte *>("glXCreateContextAttribsARB")));
    glXCreatePixmap = reinterpret_cast<GLXPixmap(*)(Display*, GLXFBConfig, Pixmap, const int*)>(glXGetProcAddressARB(reinterpret_cast<const GLubyte *>("glXCreatePixmap")));
}

Visual* getVisual(Display* dpy, Window win)
{
    OpenGLZone aZone;

    initOpenGLFunctionPointers();

    XWindowAttributes xattr;
    if( !XGetWindowAttributes( dpy, win, &xattr ) )
    {
        SAL_WARN("vcl.opengl", "Failed to get window attributes for getVisual " << win);
        xattr.visual = NULL;
    }
    SAL_INFO("vcl.opengl", "using VisualID " << xattr.visual);
    return xattr.visual;
}

}

#endif

bool OpenGLContext::init( vcl::Window* pParent )
{
    if(mbInitialized)
        return true;

    OpenGLZone aZone;

    m_xWindow.reset(pParent ? nullptr : VclPtr<vcl::Window>::Create(nullptr, WB_NOBORDER|WB_NODIALOGCONTROL));
    mpWindow = pParent ? pParent : m_xWindow.get();
    if(m_xWindow)
        m_xWindow->setPosSizePixel(0,0,0,0);
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

    OpenGLZone aZone;

    mpWindow = pChildWindow->GetParent();
    m_pChildWindow = pChildWindow;
    initWindow();
    return ImplInit();
}

#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
bool OpenGLContext::init(Display* dpy, Window win, int screen)
{
    if(mbInitialized)
        return true;

    if (!dpy)
        return false;

    OpenGLZone aZone;

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

    initOpenGLFunctionPointers();

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

    return ImplInit();
}

bool OpenGLContext::ImplInit()
{
    if (!m_aGLWin.dpy)
    {
        return false;
    }

    OpenGLZone aZone;

    GLXContext pSharedCtx( NULL );
#ifdef DBG_UTIL
    TempErrorHandler aErrorHandler(m_aGLWin.dpy, unxErrorHandler);
#endif

    SAL_INFO("vcl.opengl", "OpenGLContext::ImplInit----start");

    if (!g_vShareList.empty())
        pSharedCtx = g_vShareList.front();

#ifdef DBG_UTIL
    if (!mbPixmap && glXCreateContextAttribsARB && !mbRequestLegacyContext)
    {
        int best_fbc = -1;
        GLXFBConfig* pFBC = getFBConfig(m_aGLWin.dpy, m_aGLWin.win, best_fbc, mbUseDoubleBufferedRendering, true);
        if (!pFBC)
            return false;

        if (best_fbc != -1)
        {
            int pContextAttribs[] =
            {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                None
            };
            m_aGLWin.ctx = glXCreateContextAttribsARB(m_aGLWin.dpy, pFBC[best_fbc], pSharedCtx, GL_TRUE, pContextAttribs);
            SAL_INFO_IF(m_aGLWin.ctx, "vcl.opengl", "created a 3.2 core context");
        }
        else
            SAL_WARN("vcl.opengl", "unable to find correct FBC");

    }
#endif

    if (!m_aGLWin.ctx)
    {
        if (!m_aGLWin.vi)
           return false;

        m_aGLWin.ctx = glXCreateContext(m_aGLWin.dpy,
                m_aGLWin.vi,
                pSharedCtx,
                GL_TRUE);
    }

    if( m_aGLWin.ctx )
    {
        g_vShareList.push_back( m_aGLWin.ctx );
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
    if( mbPixmap )
    {
        m_aGLWin.Width = 0; // FIXME: correct ?
        m_aGLWin.Height = 0;
    }
    else if( !XGetWindowAttributes( m_aGLWin.dpy, m_aGLWin.win, &xWinAttr ) )
    {
        SAL_WARN("vcl.opengl", "Failed to get window attributes on " << m_aGLWin.win);
        m_aGLWin.Width = 0;
        m_aGLWin.Height = 0;
    }
    else
    {
        m_aGLWin.Width = xWinAttr.width;
        m_aGLWin.Height = xWinAttr.height;
    }

    if( m_aGLWin.HasGLXExtension("GLX_SGI_swap_control" ) )
    {
        // enable vsync
        typedef GLint (*glXSwapIntervalProc)(GLint);
        glXSwapIntervalProc glXSwapInterval = reinterpret_cast<glXSwapIntervalProc>(glXGetProcAddress( reinterpret_cast<const GLubyte*>("glXSwapIntervalSGI") ));
        if( glXSwapInterval )
        {
            TempErrorHandler aLocalErrorHandler(m_aGLWin.dpy, oglErrorHandler);

            errorTriggered = false;

            glXSwapInterval( 1 );

            if( errorTriggered )
                SAL_WARN("vcl.opengl", "error when trying to set swap interval, NVIDIA or Mesa bug?");
            else
                SAL_INFO("vcl.opengl", "set swap interval to 1 (enable vsync)");
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
    OpenGLZone aZone;

    SAL_INFO("vcl.opengl", "OpenGLContext::ImplInit----start");
    // PixelFormat tells Windows how we want things to be
    PIXELFORMATDESCRIPTOR PixelFormatFront =
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
        8,                              // stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    // interestingly we need this flag being set even if we use single buffer
    // rendering - otherwise we get errors with virtual devices
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
#if OSL_DEBUG_LEVEL > 0
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(m_aGLWin.hDC, WindowPix, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        SAL_WARN("vcl.opengl", "Render Target: Window: " << (int) ((pfd.dwFlags & PFD_DRAW_TO_WINDOW) != 0) << ", Bitmap: " << (int) ((pfd.dwFlags & PFD_DRAW_TO_BITMAP) != 0));
        SAL_WARN("vcl.opengl", "Supports OpenGL: " << (int) ((pfd.dwFlags & PFD_SUPPORT_OPENGL) != 0));
#endif
    }

    if (WindowPix == 0)
    {
        SAL_WARN("vcl.opengl", "Invalid pixelformat");
        return false;
    }

    if (!SetPixelFormat(m_aGLWin.hDC, WindowPix, &PixelFormatFront))
    {
        ImplWriteLastError(GetLastError(), "SetPixelFormat in OpenGLContext::ImplInit");
        SAL_WARN("vcl.opengl", "SetPixelFormat failed");
        return false;
    }

    HGLRC hTempRC = wglCreateContext(m_aGLWin.hDC);
    if (hTempRC == NULL)
    {
        ImplWriteLastError(GetLastError(), "wglCreateContext in OpenGLContext::ImplInit");
        SAL_WARN("vcl.opengl", "wglCreateContext failed");
        return false;
    }

    if (!wglMakeCurrent(m_aGLWin.hDC, hTempRC))
    {
        ImplWriteLastError(GetLastError(), "wglMakeCurrent in OpenGLContext::ImplInit");
        SAL_WARN("vcl.opengl", "wglMakeCurrent failed");
        return false;
    }

    if (!InitGLEW())
        return false;

    HGLRC hSharedCtx = 0;
    if (!g_vShareList.empty())
        hSharedCtx = g_vShareList.front();

    if (!wglCreateContextAttribsARB)
        return false;

    // now setup the shared context; this needs a temporary context already
    // set up in order to work
    m_aGLWin.hRC = wglCreateContextAttribsARB(m_aGLWin.hDC, hSharedCtx, NULL);
    if (m_aGLWin.hRC == 0)
    {
        ImplWriteLastError(GetLastError(), "wglCreateContextAttribsARB in OpenGLContext::ImplInit");
        SAL_WARN("vcl.opengl", "wglCreateContextAttribsARB failed");
        return false;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hTempRC);

    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        ImplWriteLastError(GetLastError(), "wglMakeCurrent (with shared context) in OpenGLContext::ImplInit");
        SAL_WARN("vcl.opengl", "wglMakeCurrent failed");
        return false;
    }

    g_vShareList.push_back(m_aGLWin.hRC);

    RECT clientRect;
    GetClientRect(WindowFromDC(m_aGLWin.hDC), &clientRect);
    m_aGLWin.Width = clientRect.right - clientRect.left;
    m_aGLWin.Height = clientRect.bottom - clientRect.top;

    return true;
}

#elif defined( MACOSX )

bool OpenGLContext::ImplInit()
{
    OpenGLZone aZone;

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
        OpenGLZone aZone;

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
    if( GLEW_ARB_debug_output)
    {
        OpenGLZone aZone;

        if (glDebugMessageCallbackARB)
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
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
    if(m_xWindow)
        m_xWindow->SetPosSizePixel(rPos, rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetPosSizePixel(rPos, rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void OpenGLContext::setWinSize(const Size& rSize)
{
    if(m_xWindow)
        m_xWindow->SetSizePixel(rSize);
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
    OUString aName = "file:///home/moggi/Documents/work/output" + OUString::number( nIdx++ ) + ".png";
    OpenGLHelper::renderToFile(iWidth, iHeight, aName);
}

#if defined( WNT )

bool OpenGLContext::initWindow()
{
    if( !m_pChildWindow )
    {
        SystemWindowData winData = generateWinData(mpWindow, false);
        m_pChildWindow = VclPtr<SystemChildWindow>::Create(mpWindow, 0, &winData, false);
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
        m_pChildWindow = VclPtr<SystemChildWindow>::Create(mpWindow, 0, &winData, false);
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

#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)

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
            m_pChildWindow = VclPtr<SystemChildWindow>::Create(mpWindow, 0, &winData, false);
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

    m_aGLWin.dpy = static_cast<Display*>(pChildSysData->pDisplay);
    m_aGLWin.win = pChildSysData->aWindow;
    m_aGLWin.screen = pChildSysData->nScreen;

    Visual* pVisual = static_cast<Visual*>(pChildSysData->pVisual);
    initGLWindow(pVisual);

    return true;
}

void OpenGLContext::initGLWindow(Visual* pVisual)
{
    OpenGLZone aZone;

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

void OpenGLContext::reset()
{
    if( !mbInitialized )
        return;

    OpenGLZone aZone;

    // reset the clip region
    maClipRegion.SetEmpty();

    // destroy all framebuffers
    if( mpLastFramebuffer )
    {
        OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;

        makeCurrent();
        while( pFramebuffer )
        {
            OpenGLFramebuffer* pPrevFramebuffer = pFramebuffer->mpPrevFramebuffer;
            delete pFramebuffer;
            pFramebuffer = pPrevFramebuffer;
        }
        mpFirstFramebuffer = NULL;
        mpLastFramebuffer = NULL;
    }

    // destroy all programs
    if( !maPrograms.empty() )
    {

        makeCurrent();
        maPrograms.clear();
    }

    if( isCurrent() )
        resetCurrent();

    mbInitialized = false;

    // destroy the context itself
#if defined( WNT )
    if (m_aGLWin.hRC)
    {
        std::vector<HGLRC>::iterator itr = std::remove(g_vShareList.begin(), g_vShareList.end(), m_aGLWin.hRC);
        if (itr != g_vShareList.end())
            g_vShareList.erase(itr);

        wglMakeCurrent( m_aGLWin.hDC, 0 );
        wglDeleteContext( m_aGLWin.hRC );
        ReleaseDC( m_aGLWin.hWnd, m_aGLWin.hDC );
    }
#elif defined( MACOSX )
    OpenGLWrapper::resetCurrent();
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
    if(m_aGLWin.ctx)
    {
        std::vector<GLXContext>::iterator itr = std::remove( g_vShareList.begin(), g_vShareList.end(), m_aGLWin.ctx );
        if (itr != g_vShareList.end())
            g_vShareList.erase(itr);

        glXMakeCurrent(m_aGLWin.dpy, None, NULL);
        if( glGetError() != GL_NO_ERROR )
        {
            SAL_WARN("vcl.opengl", "glError: " << glGetError());
        }
        glXDestroyContext(m_aGLWin.dpy, m_aGLWin.ctx);

        if (mbPixmap && m_aGLWin.glPix != None)
            glXDestroyPixmap(m_aGLWin.dpy, m_aGLWin.glPix);
    }
#endif
}

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
    OpenGLZone aZone;

    SystemWindowData aWinData;
    aWinData.nSize = sizeof(aWinData);
    aWinData.pVisual = NULL;

#if !defined(LIBO_HEADLESS)
    const SystemEnvData* sysData(pParent->GetSystemData());

    Display *dpy = static_cast<Display*>(sysData->pDisplay);
    Window win = sysData->aWindow;

    if( dpy == 0 || !glXQueryExtension( dpy, NULL, NULL ) )
        return aWinData;

    initOpenGLFunctionPointers();

    int best_fbc = -1;
    GLXFBConfig* pFBC = getFBConfig(dpy, win, best_fbc, true, false);

    if (!pFBC)
        return aWinData;

    XVisualInfo* vi = 0;
    if( best_fbc != -1 )
        vi = glXGetVisualFromFBConfig( dpy, pFBC[best_fbc] );

    XFree(pFBC);

    if( vi )
    {
        SAL_INFO("vcl.opengl", "using VisualID " << vi->visualid);
        aWinData.pVisual = (void*)(vi->visual);
    }
#endif

    return aWinData;
}

#endif

bool OpenGLContext::isCurrent()
{
    OpenGLZone aZone;

#if defined( WNT )
    return (wglGetCurrentContext() == m_aGLWin.hRC &&
            wglGetCurrentDC() == m_aGLWin.hDC);
#elif defined( MACOSX )
    (void) this; // loplugin:staticmethods
    return false;
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    return false;
#elif defined( UNX )
    GLXDrawable nDrawable = mbPixmap ? m_aGLWin.glPix : m_aGLWin.win;
    return (glXGetCurrentContext() == m_aGLWin.ctx &&
            glXGetCurrentDrawable() == nDrawable);
#endif
}

void OpenGLContext::clearCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();

    // release all framebuffers from the old context so we can re-attach the
    // texture in the new context
    OpenGLContext* pCurrentCtx = pSVData->maGDIData.mpLastContext;
    if( pCurrentCtx && pCurrentCtx->isCurrent() )
        pCurrentCtx->ReleaseFramebuffers();
}

void OpenGLContext::makeCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();

    if (isCurrent())
        return;

    OpenGLZone aZone;

    clearCurrent();

#if defined( WNT )
    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        SAL_WARN("vcl.opengl", "OpenGLContext::makeCurrent(): wglMakeCurrent failed: " << GetLastError());
        return;
    }
#elif defined( MACOSX )
    NSOpenGLView* pView = getOpenGLView();
    OpenGLWrapper::makeCurrent(pView);
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
#ifdef DBG_UTIL
    TempErrorHandler aErrorHandler(m_aGLWin.dpy, unxErrorHandler);
#endif

    GLXDrawable nDrawable = mbPixmap ? m_aGLWin.glPix : m_aGLWin.win;
    if (!glXMakeCurrent( m_aGLWin.dpy, nDrawable, m_aGLWin.ctx ))
    {
        SAL_WARN("vcl.opengl", "OpenGLContext::makeCurrent failed on drawable " << nDrawable << " pixmap? " << mbPixmap);
        return;
    }
#endif

    // move the context to the end of the contexts list
    static int nSwitch = 0;
    SAL_INFO("vcl.opengl", "******* CONTEXT SWITCH " << ++nSwitch << " *********");
    if( mpNextContext )
    {
        if( mpPrevContext )
            mpPrevContext->mpNextContext = mpNextContext;
        else
            pSVData->maGDIData.mpFirstContext = mpNextContext;
        mpNextContext->mpPrevContext = mpPrevContext;

        mpPrevContext = pSVData->maGDIData.mpLastContext;
        mpNextContext = NULL;
        pSVData->maGDIData.mpLastContext->mpNextContext = this;
        pSVData->maGDIData.mpLastContext = this;
    }
}

void OpenGLContext::resetCurrent()
{
    clearCurrent();

    OpenGLZone aZone;

#if defined( WNT )
    wglMakeCurrent( m_aGLWin.hDC, 0 );
#elif defined( MACOSX )
    (void) this; // loplugin:staticmethods
    OpenGLWrapper::resetCurrent();
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
    glXMakeCurrent(m_aGLWin.dpy, None, NULL);
#endif
}

void OpenGLContext::swapBuffers()
{
    OpenGLZone aZone;

#if defined( WNT )
    SwapBuffers(m_aGLWin.hDC);
#elif defined( MACOSX )
    NSOpenGLView* pView = getOpenGLView();
    OpenGLWrapper::swapBuffers(pView);
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
    glXSwapBuffers(m_aGLWin.dpy, mbPixmap ? m_aGLWin.glPix : m_aGLWin.win);
#endif
}

void OpenGLContext::sync()
{
    OpenGLZone aZone;

#if defined( WNT )
    // nothing
#elif defined( MACOSX ) || defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    (void) this; // loplugin:staticmethods
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
    else if (m_xWindow)
        m_xWindow->Show();
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

bool OpenGLContext::BindFramebuffer( OpenGLFramebuffer* pFramebuffer )
{
    OpenGLZone aZone;

    if( pFramebuffer != mpCurrentFramebuffer )
    {
        if( pFramebuffer )
            pFramebuffer->Bind();
        else
            OpenGLFramebuffer::Unbind();
        mpCurrentFramebuffer = pFramebuffer;
    }

    return true;
}

bool OpenGLContext::AcquireDefaultFramebuffer()
{
    return BindFramebuffer( NULL );
}

OpenGLFramebuffer* OpenGLContext::AcquireFramebuffer( const OpenGLTexture& rTexture )
{
    OpenGLZone aZone;

    OpenGLFramebuffer* pFramebuffer = NULL;
    OpenGLFramebuffer* pFreeFbo = NULL;
    OpenGLFramebuffer* pSameSizeFbo = NULL;

    // check if there is already a framebuffer attached to that texture
    pFramebuffer = mpLastFramebuffer;
    while( pFramebuffer )
    {
        if( pFramebuffer->IsAttached( rTexture ) )
            break;
        if( !pFreeFbo && pFramebuffer->IsFree() )
            pFreeFbo = pFramebuffer;
        if( !pSameSizeFbo &&
            pFramebuffer->GetWidth() == rTexture.GetWidth() &&
            pFramebuffer->GetHeight() == rTexture.GetHeight() )
            pSameSizeFbo = pFramebuffer;
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }

    // else use any framebuffer having the same size
    if( !pFramebuffer && pSameSizeFbo )
        pFramebuffer = pSameSizeFbo;

    // else use the first free framebuffer
    if( !pFramebuffer && pFreeFbo )
        pFramebuffer = pFreeFbo;

    // if there isn't any free one, create a new one if the limit isn't reached
    if( !pFramebuffer && mnFramebufferCount < MAX_FRAMEBUFFER_COUNT )
    {
        mnFramebufferCount++;
        pFramebuffer = new OpenGLFramebuffer();
        if( mpLastFramebuffer )
        {
            pFramebuffer->mpPrevFramebuffer = mpLastFramebuffer;
            mpLastFramebuffer->mpNextFramebuffer = pFramebuffer;
            mpLastFramebuffer = pFramebuffer;
        }
        else
        {
            mpFirstFramebuffer = pFramebuffer;
            mpLastFramebuffer = pFramebuffer;
        }
    }

    // last try, use any framebuffer
    // TODO order the list of framebuffers as a LRU
    if( !pFramebuffer )
        pFramebuffer = mpFirstFramebuffer;

    assert( pFramebuffer );
    BindFramebuffer( pFramebuffer );
    pFramebuffer->AttachTexture( rTexture );
    glViewport( 0, 0, rTexture.GetWidth(), rTexture.GetHeight() );

    return pFramebuffer;
}

void OpenGLContext::ReleaseFramebuffer( OpenGLFramebuffer* pFramebuffer )
{
    if( pFramebuffer )
        pFramebuffer->DetachTexture();
}

void OpenGLContext::ReleaseFramebuffer( const OpenGLTexture& rTexture )
{
    OpenGLZone aZone;
    OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;

    while( pFramebuffer )
    {
        if( pFramebuffer->IsAttached( rTexture ) )
        {
            BindFramebuffer( pFramebuffer );
            pFramebuffer->DetachTexture();
        }
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }
}

void OpenGLContext::ReleaseFramebuffers()
{
    OpenGLZone aZone;
    OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;
    while( pFramebuffer )
    {
        BindFramebuffer( pFramebuffer );
        pFramebuffer->DetachTexture();
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }
}

OpenGLProgram* OpenGLContext::GetProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble )
{
    OpenGLZone aZone;

    ProgramKey aKey( rVertexShader, rFragmentShader, preamble );

    std::map< ProgramKey, boost::shared_ptr<OpenGLProgram> >::iterator
        it = maPrograms.find( aKey );
    if( it != maPrograms.end() )
        return it->second.get();

    boost::shared_ptr<OpenGLProgram> pProgram = boost::make_shared<OpenGLProgram>();
    if( !pProgram->Load( rVertexShader, rFragmentShader, preamble ) )
        return NULL;

    maPrograms.insert(std::pair<ProgramKey, boost::shared_ptr<OpenGLProgram> >(aKey, pProgram));
    return pProgram.get();
}

OpenGLProgram* OpenGLContext::UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble )
{
    OpenGLZone aZone;

    OpenGLProgram* pProgram = GetProgram( rVertexShader, rFragmentShader, preamble );

    if( pProgram == mpCurrentProgram )
        return pProgram;

    mpCurrentProgram = pProgram;
    mpCurrentProgram->Use();

    return mpCurrentProgram;
}

inline
OpenGLContext::ProgramKey::ProgramKey( const OUString& v, const OUString& f, const OString& p )
: vertexShader( v ), fragmentShader( f ), preamble( p )
{
}

inline
bool OpenGLContext::ProgramKey::operator< ( const ProgramKey& other ) const
{
    if( vertexShader != other.vertexShader )
        return vertexShader < other.vertexShader;
    if( fragmentShader != other.fragmentShader )
        return fragmentShader < other.fragmentShader;
    return preamble < other.preamble;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
