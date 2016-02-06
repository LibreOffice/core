/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_opengl.h>

#include <desktop/exithelper.h>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include <vcl/pngwrite.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/graphic.hxx>

#include <osl/thread.hxx>

#if defined(MACOSX)
#include <premac.h>
#include <AppKit/NSOpenGLView.h>
#include <AppKit/NSOpenGL.h>
#include <postmac.h>
#endif

#if defined( WNT )
#include <win/saldata.hxx>
#endif

#include "svdata.hxx"
#include "salgdi.hxx"

#include <opengl/framebuffer.hxx>
#include <opengl/program.hxx>
#include <opengl/texture.hxx>
#include <opengl/zone.hxx>

#include "opengl/RenderState.hxx"

using namespace com::sun::star;

#define MAX_FRAMEBUFFER_COUNT 30

// TODO use rtl::Static instead of 'static'
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
static std::vector<GLXContext> g_vShareList;
#elif defined(WNT)
static std::vector<HGLRC> g_vShareList;
#endif

static sal_Int64 nBufferSwapCounter = 0;

GLWindow::~GLWindow()
{
#if defined( UNX ) && !defined MACOSX && !defined IOS && !defined ANDROID && !defined(LIBO_HEADLESS)
    XFree(vi);
#endif
}

OpenGLContext::OpenGLContext():
    mpWindow(nullptr),
    m_pChildWindow(nullptr),
    mbInitialized(false),
    mnRefCount(0),
    mbRequestLegacyContext(false),
    mbUseDoubleBufferedRendering(true),
    mbVCLOnly(false),
    mnFramebufferCount(0),
    mpCurrentFramebuffer(nullptr),
    mpFirstFramebuffer(nullptr),
    mpLastFramebuffer(nullptr),
    mpCurrentProgram(nullptr),
    mpRenderState(new RenderState),
    mnPainting(0),
    mpPrevContext(nullptr),
    mpNextContext(nullptr)
{
    VCL_GL_INFO("new context: " << this);

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
    VCL_GL_INFO("delete context: " << this);
    assert (mnRefCount == 0);

    mnRefCount = 1; // guard the shutdown paths.
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
    assert (mnRefCount == 1);
}

// release associated child-window if we have one
void OpenGLContext::dispose()
{
    reset();
    m_pChildWindow.disposeAndClear();
}

rtl::Reference<OpenGLContext> OpenGLContext::Create()
{
    return rtl::Reference<OpenGLContext>(new OpenGLContext);
}

void OpenGLContext::requestLegacyContext()
{
    mbRequestLegacyContext = true;
}

void OpenGLContext::requestSingleBufferedRendering()
{
    mbUseDoubleBufferedRendering = false;
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

    return 0;
}

bool WGLisExtensionSupported(const char *extension)
{
    OpenGLZone aZone;

    const size_t extlen = strlen(extension);
    const char *supported = NULL;

    // Try to use wglGetExtensionStringARB on current DC, if possible
    PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtString)
        supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
    // If that failed, try standard OpenGL extensions string
    if (supported == NULL)
        supported = (char*)glGetString(GL_EXTENSIONS);
    // If that failed too, must be no extensions supported
    if (supported == NULL)
        return false;

    // Begin examination at start of string, increment by 1 on false match
    for (const char* p = supported; ; p++)
    {
        // Advance p up to the next possible match
        p = strstr(p, extension);

        if (p == NULL)
            return 0; // No Match

        // Make sure that match is at the start of the string or that
        // the previous char is a space, or else we could accidentally
        // match "wglFunkywglExtension" with "wglExtension"

        // Also, make sure that the following character is space or null
        // or else "wglExtensionTwo" might match "wglExtension"
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
    // Create a temp window to check whether support multi-sample, if support, get the format
    if (InitTempWindow(&hWnd, 1, 1, pfd, glWin) < 0)
    {
        SAL_WARN("vcl.opengl", "Can't create temp window to test");
        return false;
    }

    // See if the string exists in WGL
    if (!WGLisExtensionSupported("WGL_ARB_multisample"))
    {
        SAL_WARN("vcl.opengl", "Device doesn't support multisample");
        return false;
    }
    // Get our pixel format
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        return false;
    }
    // Get our current device context
    HDC hDC = GetDC(hWnd);

    int pixelFormat;
    int valid;
    UINT    numFormats;
    float   fAttributes[] = {0,0};
    // These attributes are the bits we want to test for in our sample.
    // Everything is pretty standard, the only one we want to
    // really focus on is the WGL_SAMPLE_BUFFERS_ARB and WGL_SAMPLES_ARB.
    // These two are going to do the main testing for whether or not
    // we support multisampling on this hardware.
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
    {
        // Use asserts to make sure the iAttributes array is not changed without changing these ugly
        // hardcode indexes into it.
        assert(iAttributes[0] == WGL_DOUBLE_BUFFER_ARB);
        iAttributes[1] = GL_FALSE;
    }

    if (bRequestVirtualDevice)
    {
        assert(iAttributes[2] == WGL_DRAW_TO_WINDOW_ARB);
        iAttributes[2] = WGL_DRAW_TO_BITMAP_ARB;
    }

    bool bArbMultisampleSupported = true;

    // First we check to see if we can get a pixel format for 8 samples
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    // If we returned true, and our format count is greater than 1
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
    // Our pixel format with 8 samples failed, test for 2 samples
    assert(iAttributes[18] == WGL_SAMPLES_ARB);
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
    // Return the valid format
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
        GLenum severity, GLsizei , const GLchar* message,
#if HAVE_GLEW_1_12
        const GLvoid*
#else
        GLvoid*
#endif
        )
{
    // ignore Nvidia's : "Program/shader state performance warning: Fragment Shader is going to be recompiled because the shader key based on GL state mismatches."
    // the GLSL compiler is a bit too aggressive in optimizing the state based on the current OpenGL state
    if (id == 131218)
        return;

    SAL_WARN("vcl.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << ", with message: " << message);
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
    TempErrorHandler(Display* dpy, errorHandler newErrorHandler)
        : oldErrorHandler(nullptr)
        , mdpy(dpy)
    {
        if (mdpy)
        {
            XLockDisplay(dpy);
            XSync(dpy, false);
            oldErrorHandler = XSetErrorHandler(newErrorHandler);
        }
    }

    ~TempErrorHandler()
    {
        if (mdpy)
        {
            // sync so that we possibly get an XError
            glXWaitGL();
            XSync(mdpy, false);
            XSetErrorHandler(oldErrorHandler);
            XUnlockDisplay(mdpy);
        }
    }
};

static bool errorTriggered;
int oglErrorHandler( Display* /*dpy*/, XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

GLXFBConfig* getFBConfig(Display* dpy, Window win, int& nBestFBC, bool bUseDoubleBufferedRendering, bool bWithSameVisualID)
{
    OpenGLZone aZone;

    if( dpy == nullptr || !glXQueryExtension( dpy, nullptr, nullptr ) )
        return nullptr;

    VCL_GL_INFO("window: " << win);

    XWindowAttributes xattr;
    if( !XGetWindowAttributes( dpy, win, &xattr ) )
    {
        SAL_WARN("vcl.opengl", "Failed to get window attributes for fbconfig " << win);
        xattr.screen = nullptr;
        xattr.visual = nullptr;
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
        return nullptr;
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
        xattr.visual = nullptr;
    }
    VCL_GL_INFO("using VisualID " << xattr.visual);
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
    m_pChildWindow = nullptr;
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

// Copy of gluCheckExtension(), from the Apache-licensed
// https://code.google.com/p/glues/source/browse/trunk/glues/source/glues_registry.c
static GLboolean checkExtension(const GLubyte* extName, const GLubyte* extString)
{
  GLboolean flag=GL_FALSE;
  char* word;
  char* lookHere;
  char* deleteThis;

  if (extString==nullptr)
  {
     return GL_FALSE;
  }

  deleteThis=lookHere=static_cast<char*>(malloc(strlen(reinterpret_cast<const char*>(extString))+1));
  if (lookHere==nullptr)
  {
     return GL_FALSE;
  }

  /* strtok() will modify string, so copy it somewhere */
  strcpy(lookHere, reinterpret_cast<const char*>(extString));

  while ((word=strtok(lookHere, " "))!=nullptr)
  {
     if (strcmp(word, reinterpret_cast<const char*>(extName))==0)
     {
        flag=GL_TRUE;
        break;
     }
     lookHere=nullptr; /* get next token */
  }
  free(static_cast<void*>(deleteThis));

  return flag;
}

bool GLWindow::HasGLXExtension( const char* name ) const
{
    return checkExtension( reinterpret_cast<const GLubyte*>(name), reinterpret_cast<const GLubyte*>(GLXExtensions) );
}

bool OpenGLContext::ImplInit()
{
    if (!m_aGLWin.dpy)
        return false;

    OpenGLZone aZone;

    GLXContext pSharedCtx( nullptr );
#ifdef DBG_UTIL
    TempErrorHandler aErrorHandler(m_aGLWin.dpy, unxErrorHandler);
#endif

    VCL_GL_INFO("OpenGLContext::ImplInit----start");

    if (!g_vShareList.empty())
        pSharedCtx = g_vShareList.front();

    if (glXCreateContextAttribsARB && !mbRequestLegacyContext)
    {
        int best_fbc = -1;
        GLXFBConfig* pFBC = getFBConfig(m_aGLWin.dpy, m_aGLWin.win, best_fbc, mbUseDoubleBufferedRendering, false);
        if (!pFBC)
            return false;

        if (best_fbc != -1)
        {
            int pContextAttribs[] =
            {
#if 0 // defined(DBG_UTIL)
                GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                GLX_CONTEXT_MINOR_VERSION_ARB, 2,
#endif
                None

            };
            m_aGLWin.ctx = glXCreateContextAttribsARB(m_aGLWin.dpy, pFBC[best_fbc], pSharedCtx, /* direct, not via X */ GL_TRUE, pContextAttribs);
            SAL_INFO_IF(m_aGLWin.ctx, "vcl.opengl", "created a 3.2 core context");
        }
        else
            SAL_WARN("vcl.opengl", "unable to find correct FBC");
    }

    if (!m_aGLWin.ctx)
    {
        if (!m_aGLWin.vi)
           return false;

        SAL_WARN("vcl.opengl", "attempting to create a non-double-buffered "
                               "visual matching the context");

        m_aGLWin.ctx = glXCreateContext(m_aGLWin.dpy,
                m_aGLWin.vi,
                pSharedCtx,
                GL_TRUE /* direct, not via X server */);
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

    if( !glXMakeCurrent( m_aGLWin.dpy, m_aGLWin.win, m_aGLWin.ctx ) )
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

    XWindowAttributes aWinAttr;
    if( !XGetWindowAttributes( m_aGLWin.dpy, m_aGLWin.win, &aWinAttr ) )
    {
        SAL_WARN("vcl.opengl", "Failed to get window attributes on " << m_aGLWin.win);
        m_aGLWin.Width = 0;
        m_aGLWin.Height = 0;
    }
    else
    {
        m_aGLWin.Width = aWinAttr.width;
        m_aGLWin.Height = aWinAttr.height;
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
                VCL_GL_INFO("set swap interval to 1 (enable vsync)");
        }
    }

    bool bRet = InitGLEW();
    InitGLEWDebugging();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    registerAsCurrent();

    return bRet;
}

#elif defined( _WIN32 )

namespace
{

bool tryShaders(const OUString& rVertexShader, const OUString& rFragmentShader, const OUString& rGeometryShader = "", const OString& rPreamble = "")
{
    GLint nId;

    // Somewhat mysteriously, the OpenGLHelper::LoadShaders() API saves a compiled binary of the
    // shader only if you give it the digest of the shaders. We have API to calculate the digest
    // only of the combination of vertex and fragment (but not geometry) shader. So if we have a
    // geometry shader, we should not save the binary.
    if (rGeometryShader.isEmpty())
    {
        nId = OpenGLHelper::LoadShaders(rVertexShader, rFragmentShader, rPreamble, OpenGLHelper::GetDigest( rVertexShader, rFragmentShader, rPreamble));
    }
    else
    {
        assert(rPreamble.isEmpty());
        nId = OpenGLHelper::LoadShaders(rVertexShader, rFragmentShader, rGeometryShader);
    }
    if (!nId)
        return false;
    glDeleteProgram(nId);
    return glGetError() == GL_NO_ERROR;
}

bool compiledShaderBinariesWork()
{
    static bool bBeenHere = false;
    static bool bResult;

    if (bBeenHere)
        return bResult;

    bBeenHere = true;

    bResult =
        (
#if 0 // Only look at shaders used by vcl for now
         // canvas
         tryShaders("dummyVertexShader", "linearMultiColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "linearTwoColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "radialMultiColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "radialTwoColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "rectangularMultiColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "rectangularTwoColorGradientFragmentShader") &&
         // chart2
         (GLEW_VERSION_3_3 ?
          (tryShaders("shape3DVertexShader", "shape3DFragmentShader") &&
           tryShaders("shape3DVertexShaderBatchScroll", "shape3DFragmentShaderBatchScroll") &&
           tryShaders("shape3DVertexShaderBatch", "shape3DFragmentShaderBatch") &&
           tryShaders("textVertexShaderBatch", "textFragmentShaderBatch")) :
          (tryShaders("shape3DVertexShaderV300", "shape3DFragmentShaderV300"))) &&
         tryShaders("textVertexShader", "textFragmentShader") &&
         tryShaders("screenTextVertexShader", "screenTextFragmentShader") &&
         tryShaders("commonVertexShader", "commonFragmentShader") &&
         tryShaders("pickingVertexShader", "pickingFragmentShader") &&
         tryShaders("backgroundVertexShader", "backgroundFragmentShader") &&
         tryShaders("symbolVertexShader", "symbolFragmentShader") &&
         tryShaders("symbolVertexShader", "symbolFragmentShader") &&
         // slideshow
         tryShaders("reflectionVertexShader", "reflectionFragmentShader") &&
         tryShaders("basicVertexShader", "basicFragmentShader") &&
         tryShaders("vortexVertexShader", "vortexFragmentShader", "vortexGeometryShader") &&
         tryShaders("basicVertexShader", "rippleFragmentShader") &&
         tryShaders("glitterVertexShader", "glitterFragmentShader") &&
         tryShaders("honeycombVertexShader", "honeycombFragmentShader", "honeycombGeometryShader") &&
#endif
         // vcl
         tryShaders("combinedVertexShader", "combinedFragmentShader") &&
         tryShaders("dumbVertexShader", "invert50FragmentShader") &&
         tryShaders("combinedTextureVertexShader", "combinedTextureFragmentShader") &&
         tryShaders("textureVertexShader", "areaScaleFragmentShader") &&
         tryShaders("transformedTextureVertexShader", "maskedTextureFragmentShader") &&
         tryShaders("transformedTextureVertexShader", "areaScaleFastFragmentShader") &&
         tryShaders("transformedTextureVertexShader", "areaScaleFastFragmentShader", "", "#define MASKED") &&
         tryShaders("transformedTextureVertexShader", "areaScaleFragmentShader") &&
         tryShaders("transformedTextureVertexShader", "areaScaleFragmentShader", "", "#define MASKED") &&
         tryShaders("transformedTextureVertexShader", "textureFragmentShader") &&
         tryShaders("combinedTextureVertexShader", "combinedTextureFragmentShader") &&
         tryShaders("combinedTextureVertexShader", "combinedTextureFragmentShader", "", "// flush shader\n") &&
         tryShaders("textureVertexShader", "linearGradientFragmentShader") &&
         tryShaders("textureVertexShader", "radialGradientFragmentShader") &&
         tryShaders("textureVertexShader", "areaHashCRC64TFragmentShader") &&
         tryShaders("textureVertexShader", "replaceColorFragmentShader") &&
         tryShaders("textureVertexShader", "greyscaleFragmentShader") &&
         tryShaders("textureVertexShader", "textureFragmentShader") &&
         tryShaders("textureVertexShader", "convolutionFragmentShader") &&
         tryShaders("textureVertexShader", "areaScaleFastFragmentShader"));

    if (!bResult)
    {
        OpenGLZone::hardDisable();
        TerminateProcess(GetCurrentProcess(), EXITHELPER_NORMAL_RESTART);
    }

    return bResult;
}

} // unnamed namespace

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

    VCL_GL_INFO("OpenGLContext::ImplInit----start");
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
        24,                             // 24 bit z-buffer
        8,                              // stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    if (mbUseDoubleBufferedRendering)
        PixelFormatFront.dwFlags |= PFD_DOUBLEBUFFER;

    PixelFormatFront.dwFlags |= PFD_DRAW_TO_WINDOW;

    //  we must check whether can set the MSAA
    int WindowPix = 0;
    bool bMultiSampleSupport = InitMultisample(PixelFormatFront, WindowPix,
            mbUseDoubleBufferedRendering, false);
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
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hTempRC);
        return false;
    }

    HGLRC hSharedCtx = 0;
    if (!g_vShareList.empty())
        hSharedCtx = g_vShareList.front();

    if (!wglCreateContextAttribsARB)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hTempRC);
        return false;
    }

    // now setup the shared context; this needs a temporary context already
    // set up in order to work
    int attribs [] =
    {
#ifdef DBG_UTIL
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };
    m_aGLWin.hRC = wglCreateContextAttribsARB(m_aGLWin.hDC, hSharedCtx, attribs);
    if (m_aGLWin.hRC == 0)
    {
        ImplWriteLastError(GetLastError(), "wglCreateContextAttribsARB in OpenGLContext::ImplInit");
        SAL_WARN("vcl.opengl", "wglCreateContextAttribsARB failed");
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hTempRC);
        return false;
    }

    if (!compiledShaderBinariesWork())
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hTempRC);
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

    InitGLEWDebugging();

    g_vShareList.push_back(m_aGLWin.hRC);

    RECT clientRect;
    GetClientRect(WindowFromDC(m_aGLWin.hDC), &clientRect);
    m_aGLWin.Width = clientRect.right - clientRect.left;
    m_aGLWin.Height = clientRect.bottom - clientRect.top;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    registerAsCurrent();

    return true;
}

#elif defined( MACOSX )

bool OpenGLContext::ImplInit()
{
    OpenGLZone aZone;

    VCL_GL_INFO("OpenGLContext::ImplInit----start");
    NSOpenGLView* pView = getOpenGLView();
    [[pView openGLContext] makeCurrentContext];

    bool bRet = InitGLEW();
    InitGLEWDebugging();
    return bRet;
}

#else

bool OpenGLContext::ImplInit()
{
    VCL_GL_INFO("OpenGLContext not implemented for this platform");
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

    VCL_GL_INFO("OpenGLContext::ImplInit----end");
    mbInitialized = true;
    return true;
}

void OpenGLContext::InitGLEWDebugging()
{
#ifdef DBG_UTIL
    // only enable debug output in dbgutil build
    if( GLEW_ARB_debug_output)
    {
        OpenGLZone aZone;

        if (glDebugMessageCallbackARB)
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            glDebugMessageCallbackARB(&debug_callback, nullptr);

#ifdef GL_DEBUG_SEVERITY_NOTIFICATION_ARB
            // Ignore i965’s shader compiler notification flood.
            glDebugMessageControlARB(GL_DEBUG_SOURCE_SHADER_COMPILER_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DEBUG_SEVERITY_NOTIFICATION_ARB, 0, nullptr, true);
#endif
        }
        else if ( glDebugMessageCallback )
        {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(&debug_callback, nullptr);

            // Ignore i965’s shader compiler notification flood.
            glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, true);
        }
    }

    // Test hooks for inserting tracing messages into the stream
    VCL_GL_INFO("LibreOffice GLContext initialized: " << this);
#endif
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
        m_pChildWindow->SetParentClipMode(ParentClipMode::Clip);
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
        m_pChildWindow->SetParentClipMode(ParentClipMode::Clip);
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
    const SystemEnvData* pChildSysData = nullptr;
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
    m_pChildWindow->SetParentClipMode( ParentClipMode::NoClip );
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

    // Check multisample support
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

    // don't reset a context in the middle of stack frames rendering to it
    assert( mnPainting == 0 );

    // reset the clip region
    maClipRegion.SetEmpty();
    mpRenderState.reset(new RenderState);

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
        mpFirstFramebuffer = nullptr;
        mpLastFramebuffer = nullptr;
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

        if (wglGetCurrentContext() != NULL)
            wglMakeCurrent(NULL, NULL);
        wglDeleteContext( m_aGLWin.hRC );
        ReleaseDC( m_aGLWin.hWnd, m_aGLWin.hDC );
        m_aGLWin.hRC = 0;
    }
#elif defined( MACOSX )
    [NSOpenGLContext clearCurrentContext];
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
    if(m_aGLWin.ctx)
    {
        std::vector<GLXContext>::iterator itr = std::remove( g_vShareList.begin(), g_vShareList.end(), m_aGLWin.ctx );
        if (itr != g_vShareList.end())
            g_vShareList.erase(itr);

        glXMakeCurrent(m_aGLWin.dpy, None, nullptr);
        if( glGetError() != GL_NO_ERROR )
        {
            SAL_WARN("vcl.opengl", "glError: " << glGetError());
        }
        glXDestroyContext(m_aGLWin.dpy, m_aGLWin.ctx);
        m_aGLWin.ctx = nullptr;
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
    aWinData.pVisual = nullptr;

#if !defined(LIBO_HEADLESS)
    const SystemEnvData* sysData(pParent->GetSystemData());

    Display *dpy = static_cast<Display*>(sysData->pDisplay);
    Window win = sysData->aWindow;

    if( dpy == nullptr || !glXQueryExtension( dpy, nullptr, nullptr ) )
        return aWinData;

    initOpenGLFunctionPointers();

    int best_fbc = -1;
    GLXFBConfig* pFBC = getFBConfig(dpy, win, best_fbc, true, false);

    if (!pFBC)
        return aWinData;

    XVisualInfo* vi = nullptr;
    if( best_fbc != -1 )
        vi = glXGetVisualFromFBConfig( dpy, pFBC[best_fbc] );

    XFree(pFBC);

    if( vi )
    {
        VCL_GL_INFO("using VisualID " << vi->visualid);
        aWinData.pVisual = static_cast<void*>(vi->visual);
    }
#endif

    return aWinData;
}

#endif

bool OpenGLContext::isCurrent()
{
    OpenGLZone aZone;

#if defined( WNT )
    return wglGetCurrentContext() == m_aGLWin.hRC &&
           wglGetCurrentDC() == m_aGLWin.hDC;
#elif defined( MACOSX )
    (void) this; // loplugin:staticmethods
    return false;
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    return false;
#elif defined( UNX )
    return m_aGLWin.ctx && glXGetCurrentContext() == m_aGLWin.ctx &&
           glXGetCurrentDrawable() == m_aGLWin.win;
#endif
}

bool OpenGLContext::hasCurrent()
{
#if defined( WNT )
    return wglGetCurrentContext() != NULL;
#elif defined( MACOSX ) || defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    return false;
#elif defined( UNX )
    return glXGetCurrentContext() != None;
#endif
}

void OpenGLContext::clearCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();

    // release all framebuffers from the old context so we can re-attach the
    // texture in the new context
    rtl::Reference<OpenGLContext> pCurrentCtx = pSVData->maGDIData.mpLastContext;
    if( pCurrentCtx.is() && pCurrentCtx->isCurrent() )
        pCurrentCtx->ReleaseFramebuffers();
}

void OpenGLContext::prepareForYield()
{
    ImplSVData* pSVData = ImplGetSVData();

    // release all framebuffers from the old context so we can re-attach the
    // texture in the new context
    rtl::Reference<OpenGLContext> pCurrentCtx = pSVData->maGDIData.mpLastContext;

    if ( !pCurrentCtx.is() )
        return;                 // Not using OpenGL

    SAL_INFO("vcl.opengl", "Unbinding contexts in preparation for yield");

    if( pCurrentCtx->isCurrent() )
        pCurrentCtx->resetCurrent();

    assert (!hasCurrent());
}

void OpenGLContext::makeCurrent()
{
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
    [[pView openGLContext] makeCurrentContext];
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
#ifdef DBG_UTIL
    TempErrorHandler aErrorHandler(m_aGLWin.dpy, unxErrorHandler);
#endif

    if (m_aGLWin.dpy)
    {
        if (!glXMakeCurrent( m_aGLWin.dpy, m_aGLWin.win, m_aGLWin.ctx ))
        {
            SAL_WARN("vcl.opengl", "OpenGLContext::makeCurrent failed "
                     "on drawable " << m_aGLWin.win);
            return;
        }
    }
#endif

    registerAsCurrent();
}

rtl::Reference<OpenGLContext> OpenGLContext::getVCLContext(bool bMakeIfNecessary)
{
    ImplSVData* pSVData = ImplGetSVData();
    OpenGLContext *pContext = pSVData->maGDIData.mpLastContext;
    while( pContext )
    {
        // check if this context is usable
        if( pContext->isInitialized() && pContext->isVCLOnly() )
            break;
        pContext = pContext->mpPrevContext;
    }
    rtl::Reference<OpenGLContext> xContext;
    if( !pContext && bMakeIfNecessary )
    {
        // create our magic fallback window context.
        xContext = ImplGetDefaultContextWindow()->GetGraphics()->GetOpenGLContext();
        assert(xContext.is());
    }
    else
        xContext = pContext;

    if( xContext.is() )
        xContext->makeCurrent();

    return xContext;
}

/*
 * We don't care what context we have, but we want one that is live,
 * ie. not reset underneath us, and is setup for VCL usage - ideally
 * not swapping context at all.
 */
void OpenGLContext::makeVCLCurrent()
{
    getVCLContext(true);
}

void OpenGLContext::registerAsCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();

    // move the context to the end of the contexts list
    static int nSwitch = 0;
    VCL_GL_INFO("******* CONTEXT SWITCH " << ++nSwitch << " *********");
    if( mpNextContext )
    {
        if( mpPrevContext )
            mpPrevContext->mpNextContext = mpNextContext;
        else
            pSVData->maGDIData.mpFirstContext = mpNextContext;
        mpNextContext->mpPrevContext = mpPrevContext;

        mpPrevContext = pSVData->maGDIData.mpLastContext;
        mpNextContext = nullptr;
        pSVData->maGDIData.mpLastContext->mpNextContext = this;
        pSVData->maGDIData.mpLastContext = this;
    }
}

void OpenGLContext::resetCurrent()
{
    clearCurrent();

    OpenGLZone aZone;

#if defined( WNT )
    wglMakeCurrent(NULL, NULL);
#elif defined( MACOSX )
    (void) this; // loplugin:staticmethods
    [NSOpenGLContext clearCurrentContext];
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
    if (m_aGLWin.dpy)
        glXMakeCurrent(m_aGLWin.dpy, None, nullptr);
#endif
}

void OpenGLContext::swapBuffers()
{
    OpenGLZone aZone;

#if defined( WNT )
    SwapBuffers(m_aGLWin.hDC);
#elif defined( MACOSX )
    NSOpenGLView* pView = getOpenGLView();
    [[pView openGLContext] flushBuffer];
#elif defined( IOS ) || defined( ANDROID ) || defined(LIBO_HEADLESS)
    // nothing
#elif defined( UNX )
    glXSwapBuffers(m_aGLWin.dpy, m_aGLWin.win);
#endif

    nBufferSwapCounter++;

    static bool bSleep = getenv("SAL_GL_SLEEP_ON_SWAP");
    if (bSleep)
    {
        // half a second.
        TimeValue aSleep( 0, 500*1000*1000 );
        osl::Thread::wait( aSleep );
    }
}

sal_Int64 OpenGLWrapper::getBufferSwapCounter()
{
    return nBufferSwapCounter;
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
    return BindFramebuffer( nullptr );
}

OpenGLFramebuffer* OpenGLContext::AcquireFramebuffer( const OpenGLTexture& rTexture )
{
    OpenGLZone aZone;

    OpenGLFramebuffer* pFramebuffer = nullptr;
    OpenGLFramebuffer* pFreeFbo = nullptr;
    OpenGLFramebuffer* pSameSizeFbo = nullptr;

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

    state()->viewport(Rectangle(Point(), Size(rTexture.GetWidth(), rTexture.GetHeight())));

    return pFramebuffer;
}

// FIXME: this method is rather grim from a perf. perspective.
// We should instead (eventually) use pointers to associate the
// framebuffer and texture cleanly.
void OpenGLContext::UnbindTextureFromFramebuffers( GLuint nTexture )
{
    OpenGLFramebuffer* pFramebuffer;

    // see if there is a framebuffer attached to that texture
    pFramebuffer = mpLastFramebuffer;
    while( pFramebuffer )
    {
        if (pFramebuffer->IsAttached(nTexture))
        {
            BindFramebuffer(pFramebuffer);
            pFramebuffer->DetachTexture();
        }
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }
}

void OpenGLContext::ReleaseFramebuffer( OpenGLFramebuffer* pFramebuffer )
{
    if( pFramebuffer )
        pFramebuffer->DetachTexture();
}

void OpenGLContext::ReleaseFramebuffer( const OpenGLTexture& rTexture )
{
    OpenGLZone aZone;

    if (!rTexture) // no texture to release.
        return;

    OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;

    while( pFramebuffer )
    {
        if( pFramebuffer->IsAttached( rTexture ) )
        {
            BindFramebuffer( pFramebuffer );
            pFramebuffer->DetachTexture();
            if (mpCurrentFramebuffer == pFramebuffer)
                BindFramebuffer( nullptr );
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
        if (!pFramebuffer->IsFree())
        {
            BindFramebuffer( pFramebuffer );
            pFramebuffer->DetachTexture();
        }
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }
    BindFramebuffer( nullptr );
}

OpenGLProgram* OpenGLContext::GetProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const rtl::OString& preamble )
{
    OpenGLZone aZone;

    // We cache the shader programs in a per-process run-time cache
    // based on only the names and the preamble. We don't expect
    // shader source files to change during the lifetime of a
    // LibreOffice process.
    rtl::OString aNameBasedKey = OUStringToOString(rVertexShader + "+" + rFragmentShader, RTL_TEXTENCODING_UTF8) + "+" + preamble;
    if( !aNameBasedKey.isEmpty() )
    {
        ProgramCollection::iterator it = maPrograms.find( aNameBasedKey );
        if( it != maPrograms.end() )
            return it->second.get();
    }

    // Binary shader programs are cached persistently (between
    // LibreOffice process instances) based on a hash of their source
    // code, as the source code can and will change between
    // LibreOffice versions even if the shader names don't change.
    rtl::OString aPersistentKey = OpenGLHelper::GetDigest( rVertexShader, rFragmentShader, preamble );
    std::shared_ptr<OpenGLProgram> pProgram = std::make_shared<OpenGLProgram>();
    if( !pProgram->Load( rVertexShader, rFragmentShader, preamble, aPersistentKey ) )
        return nullptr;

    maPrograms.insert(std::make_pair(aNameBasedKey, pProgram));
    return pProgram.get();
}

OpenGLProgram* OpenGLContext::UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble )
{
    OpenGLZone aZone;

    OpenGLProgram* pProgram = GetProgram( rVertexShader, rFragmentShader, preamble );

    if (pProgram == mpCurrentProgram)
    {
        VCL_GL_INFO("Context::UseProgram: Reusing existing program " << pProgram->Id());
        pProgram->Reuse();
        return pProgram;
    }

    mpCurrentProgram = pProgram;

    if (!mpCurrentProgram)
    {
        SAL_WARN("vcl.opengl", "OpenGLContext::UseProgram: mpCurrentProgram is 0");
        return nullptr;
    }

    mpCurrentProgram->Use();

    return mpCurrentProgram;
}

void OpenGLContext::UseNoProgram()
{
    if( mpCurrentProgram == nullptr )
        return;

    mpCurrentProgram = nullptr;
    glUseProgram( 0 );
    CHECK_GL_ERROR();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
