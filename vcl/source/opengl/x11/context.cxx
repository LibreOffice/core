/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/lazydelete.hxx>

#include <svdata.hxx>

#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salgdi.h>
#include <unx/salinst.h>
#include <unx/salvd.h>
#include <unx/x11/xlimits.hxx>

#include <opengl/zone.hxx>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <sal/log.hxx>

static std::vector<GLXContext> g_vShareList;
static bool g_bAnyCurrent;

namespace {

class X11OpenGLContext : public OpenGLContext
{
public:
    virtual void initWindow() override;
private:
    GLX11Window m_aGLWin;
    virtual const GLWindow& getOpenGLWindow() const override { return m_aGLWin; }
    virtual GLWindow& getModifiableOpenGLWindow() override { return m_aGLWin; }
    virtual bool ImplInit() override;
    void initGLWindow(Visual* pVisual);
    virtual SystemWindowData generateWinData(vcl::Window* pParent, bool bRequestLegacyContext) override;
    virtual void makeCurrent() override;
    virtual void destroyCurrentContext() override;
    virtual bool isCurrent() override;
    virtual bool isAnyCurrent() override;
    virtual void sync() override;
    virtual void resetCurrent() override;
    virtual void swapBuffers() override;
};

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

    bool errorTriggered;
    int oglErrorHandler( Display* /*dpy*/, XErrorEvent* /*evnt*/ )
    {
        errorTriggered = true;

        return 0;
    }

    GLXFBConfig* getFBConfig(Display* dpy, Window win, int& nBestFBC)
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
            if(pVi && (xattr.visual && pVi->visualid == xattr.visual->visualid) )
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
}

void X11OpenGLContext::sync()
{
    OpenGLZone aZone;
    glXWaitGL();
    XSync(m_aGLWin.dpy, false);
}

void X11OpenGLContext::swapBuffers()
{
    OpenGLZone aZone;

    glXSwapBuffers(m_aGLWin.dpy, m_aGLWin.win);

    BuffersSwapped();
}

void X11OpenGLContext::resetCurrent()
{
    clearCurrent();

    OpenGLZone aZone;

    if (m_aGLWin.dpy)
    {
        glXMakeCurrent(m_aGLWin.dpy, None, nullptr);
        g_bAnyCurrent = false;
    }
}

bool X11OpenGLContext::isCurrent()
{
    OpenGLZone aZone;
    return g_bAnyCurrent && m_aGLWin.ctx && glXGetCurrentContext() == m_aGLWin.ctx &&
           glXGetCurrentDrawable() == m_aGLWin.win;
}

bool X11OpenGLContext::isAnyCurrent()
{
    return g_bAnyCurrent && glXGetCurrentContext() != None;
}

SystemWindowData X11OpenGLContext::generateWinData(vcl::Window* pParent, bool /*bRequestLegacyContext*/)
{
    OpenGLZone aZone;

    SystemWindowData aWinData;
    aWinData.pVisual = nullptr;
    aWinData.bClipUsingNativeWidget = false;

    const SystemEnvData* sysData(pParent->GetSystemData());

    Display *dpy = static_cast<Display*>(sysData->pDisplay);
    Window win = sysData->GetWindowHandle(pParent->ImplGetFrame());

    if( dpy == nullptr || !glXQueryExtension( dpy, nullptr, nullptr ) )
        return aWinData;

    int best_fbc = -1;
    GLXFBConfig* pFBC = getFBConfig(dpy, win, best_fbc);

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

    return aWinData;
}

bool X11OpenGLContext::ImplInit()
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

    //tdf#112166 for, e.g. VirtualBox GL, claiming OpenGL 2.1
    static bool hasCreateContextAttribsARB = glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")) != nullptr;
    if (hasCreateContextAttribsARB && !mbRequestLegacyContext)
    {
        int best_fbc = -1;
        GLXFBConfig* pFBC = getFBConfig(m_aGLWin.dpy, m_aGLWin.win, best_fbc);

        if (pFBC && best_fbc != -1)
        {
            int const pContextAttribs[] =
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
        g_bAnyCurrent = false;
        SAL_WARN("vcl.opengl", "unable to select current GLX context");
        return false;
    }

    g_bAnyCurrent = true;

    int glxMinor, glxMajor;
    double nGLXVersion = 0;
    if( glXQueryVersion( m_aGLWin.dpy, &glxMajor, &glxMinor ) )
      nGLXVersion = glxMajor + 0.1*glxMinor;
    SAL_INFO("vcl.opengl", "available GLX version: " << nGLXVersion);

    SAL_INFO("vcl.opengl", "available GL  extensions: " << glGetString(GL_EXTENSIONS));

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

    bool bRet = InitGL();
    InitGLDebugging();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    registerAsCurrent();

    return bRet;
}

void X11OpenGLContext::makeCurrent()
{
    if (isCurrent())
        return;

    OpenGLZone aZone;

    clearCurrent();

#ifdef DBG_UTIL
    TempErrorHandler aErrorHandler(m_aGLWin.dpy, unxErrorHandler);
#endif

    if (m_aGLWin.dpy)
    {
        if (!glXMakeCurrent( m_aGLWin.dpy, m_aGLWin.win, m_aGLWin.ctx ))
        {
            g_bAnyCurrent = false;
            SAL_WARN("vcl.opengl", "OpenGLContext::makeCurrent failed "
                     "on drawable " << m_aGLWin.win);
            return;
        }
        g_bAnyCurrent = true;
    }

    registerAsCurrent();
}

void X11OpenGLContext::destroyCurrentContext()
{
    if(!m_aGLWin.ctx)
        return;

    std::vector<GLXContext>::iterator itr = std::remove( g_vShareList.begin(), g_vShareList.end(), m_aGLWin.ctx );
    if (itr != g_vShareList.end())
        g_vShareList.erase(itr);

    glXMakeCurrent(m_aGLWin.dpy, None, nullptr);
    g_bAnyCurrent = false;
    if( glGetError() != GL_NO_ERROR )
    {
        SAL_WARN("vcl.opengl", "glError: " << glGetError());
    }
    glXDestroyContext(m_aGLWin.dpy, m_aGLWin.ctx);
    m_aGLWin.ctx = nullptr;
}

void X11OpenGLContext::initGLWindow(Visual* pVisual)
{
    OpenGLZone aZone;

    // Get visual info
    {
        XVisualInfo aTemplate;
        aTemplate.visualid = XVisualIDFromVisual( pVisual );
        int nVisuals = 0;
        XVisualInfo* pInfo = XGetVisualInfo( m_aGLWin.dpy, VisualIDMask, &aTemplate, &nVisuals );
        if( nVisuals != 1 )
            SAL_WARN( "vcl.opengl", "match count for visual id is not 1" );
        m_aGLWin.vi = pInfo;
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

void X11OpenGLContext::initWindow()
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
        return;

    InitChildWindow(m_pChildWindow.get());

    m_aGLWin.dpy = static_cast<Display*>(pChildSysData->pDisplay);
    m_aGLWin.win = pChildSysData->GetWindowHandle(m_pChildWindow->ImplGetFrame());
    m_aGLWin.screen = pChildSysData->nScreen;

    Visual* pVisual = static_cast<Visual*>(pChildSysData->pVisual);
    initGLWindow(pVisual);
}

GLX11Window::GLX11Window()
    : dpy(nullptr)
    , screen(0)
    , win(0)
    , vi(nullptr)
    , ctx(nullptr)
{
}

bool GLX11Window::HasGLXExtension( const char* name ) const
{
    for (sal_Int32 i = 0; i != -1;) {
        if (GLXExtensions.getToken(0, ' ', i) == name) {
            return true;
        }
    }
    return false;
}

GLX11Window::~GLX11Window()
{
    XFree(vi);
}

bool GLX11Window::Synchronize(bool bOnoff) const
{
    XSynchronize(dpy, bOnoff);
    return true;
}

OpenGLContext* X11SalInstance::CreateOpenGLContext()
{
    return new X11OpenGLContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
