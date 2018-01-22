/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <vcl/salbtype.hxx>
#include <vcl/lazydelete.hxx>

#include <svdata.hxx>

#include <unx/pixmap.hxx>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salgdi.h>
#include <unx/salinst.h>
#include <unx/salvd.h>
#include <unx/x11/xlimits.hxx>

#include <opengl/texture.hxx>
#include <opengl/zone.hxx>
#include <opengl/RenderState.hxx>
#include <opengl/x11/gdiimpl.hxx>
#include <opengl/x11/salvd.hxx>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <o3tl/lru_map.hxx>
#include <ControlCacheKey.hxx>

static std::vector<GLXContext> g_vShareList;
static bool g_bAnyCurrent;

class X11OpenGLContext : public OpenGLContext
{
public:
    void init(Display* dpy, Window win, int screen);
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

namespace
{

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

    GLXFBConfig* getFBConfig(Display* dpy, Window win, int& nBestFBC, bool bUseDoubleBufferedRendering)
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

    Visual* getVisual(Display* dpy, Window win)
    {
        OpenGLZone aZone;

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

    const SystemEnvData* sysData(pParent->GetSystemData());

    Display *dpy = static_cast<Display*>(sysData->pDisplay);
    Window win = sysData->aWindow;

    if( dpy == nullptr || !glXQueryExtension( dpy, nullptr, nullptr ) )
        return aWinData;

    int best_fbc = -1;
    GLXFBConfig* pFBC = getFBConfig(dpy, win, best_fbc, true);

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

    if (glXCreateContextAttribsARB && !mbRequestLegacyContext)
    {
        int best_fbc = -1;
        GLXFBConfig* pFBC = getFBConfig(m_aGLWin.dpy, m_aGLWin.win, best_fbc, mbUseDoubleBufferedRendering);

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
    if(m_aGLWin.ctx)
    {
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
}

void X11OpenGLContext::init(Display* dpy, Window win, int screen)
{
    if (isInitialized())
        return;

    if (!dpy)
        return;

    OpenGLZone aZone;

    m_aGLWin.dpy = dpy;
    m_aGLWin.win = win;
    m_aGLWin.screen = screen;

    Visual* pVisual = getVisual(dpy, win);

    initGLWindow(pVisual);

    ImplInit();
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
    m_aGLWin.win = pChildSysData->aWindow;
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

X11OpenGLSalGraphicsImpl::X11OpenGLSalGraphicsImpl( X11SalGraphics& rParent ):
    OpenGLSalGraphicsImpl(rParent,rParent.GetGeometryProvider()),
    mrX11Parent(rParent)
{
}

X11OpenGLSalGraphicsImpl::~X11OpenGLSalGraphicsImpl()
{
}

void X11OpenGLSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    mpProvider = mrX11Parent.GetGeometryProvider();
    OpenGLSalGraphicsImpl::Init();
}

rtl::Reference<OpenGLContext> X11OpenGLSalGraphicsImpl::CreateWinContext()
{
    NativeWindowHandleProvider *pProvider = dynamic_cast<NativeWindowHandleProvider*>(mrX11Parent.m_pFrame);

    if( !pProvider )
        return nullptr;

    sal_uIntPtr aWin = pProvider->GetNativeWindowHandle();
    rtl::Reference<X11OpenGLContext> xContext = new X11OpenGLContext;
    xContext->setVCLOnly();
    xContext->init( mrX11Parent.GetXDisplay(), aWin,
                    mrX11Parent.m_nXScreen.getXScreen() );
    return rtl::Reference<OpenGLContext>(xContext.get());
}

void X11OpenGLSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    OpenGLSalGraphicsImpl *pImpl = pSrcGraphics ? static_cast< OpenGLSalGraphicsImpl* >(pSrcGraphics->GetImpl()) : static_cast< OpenGLSalGraphicsImpl *>(mrX11Parent.GetImpl());
    OpenGLSalGraphicsImpl::DoCopyBits( rPosAry, *pImpl );
}

void X11OpenGLSalGraphicsImpl::FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY )
{
    Display* pDisplay = mrX11Parent.GetXDisplay();
    SalX11Screen nScreen = mrX11Parent.GetScreenNumber();
    XVisualInfo aVisualInfo;
    XImage* pImage;
    char* pData;

    SAL_INFO( "vcl.opengl", "FillPixmapFromScreen" );

    if (!SalDisplay::BestOpenGLVisual(pDisplay, nScreen.getXScreen(), aVisualInfo))
        return;

    // make sure everything is synced up before reading back
    mpContext->makeCurrent();
    glXWaitX();

    // TODO: lfrb: What if offscreen?
    pData = static_cast<char*>(malloc( pPixmap->GetWidth() * pPixmap->GetHeight() * 4 ));
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    CHECK_GL_ERROR();
    glReadPixels( nX, GetHeight() - nY, pPixmap->GetWidth(), pPixmap->GetHeight(),
                  GL_RGBA, GL_UNSIGNED_BYTE, pData );
    CHECK_GL_ERROR();

    pImage = XCreateImage( pDisplay, aVisualInfo.visual, 24, ZPixmap, 0, pData,
                           pPixmap->GetWidth(), pPixmap->GetHeight(), 8, 0 );
    XInitImage( pImage );
    GC aGC = XCreateGC( pDisplay, pPixmap->GetPixmap(), 0, nullptr );
    XPutImage( pDisplay, pPixmap->GetDrawable(), aGC, pImage,
               0, 0, 0, 0, pPixmap->GetWidth(), pPixmap->GetHeight() );
    XFreeGC( pDisplay, aGC );
    XDestroyImage( pImage );
}

typedef typename std::pair<ControlCacheKey, std::unique_ptr<TextureCombo>> ControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, std::unique_ptr<TextureCombo>, ControlCacheHashFunction> ControlCacheType;

static vcl::DeleteOnDeinit<ControlCacheType> gTextureCache(new ControlCacheType(200));

namespace
{
    GLXFBConfig GetPixmapFBConfig( Display* pDisplay, bool& bInverted )
    {
        OpenGLZone aZone;

        int nScreen = DefaultScreen( pDisplay );
        GLXFBConfig *aFbConfigs;
        int i, nFbConfigs, nValue;

        aFbConfigs = glXGetFBConfigs( pDisplay, nScreen, &nFbConfigs );
        for( i = 0; i < nFbConfigs; i++ )
        {
            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_DRAWABLE_TYPE, &nValue );
            if( !(nValue & GLX_PIXMAP_BIT) )
                continue;

            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &nValue );
            if( !(nValue & GLX_TEXTURE_2D_BIT_EXT) )
                continue;

            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_DEPTH_SIZE, &nValue );
            if( nValue != 24 )
                continue;

            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_RED_SIZE, &nValue );
            if( nValue != 8 )
                continue;
            SAL_INFO( "vcl.opengl", "Red is " << nValue );

            // TODO: lfrb: Make it configurable wrt RGB/RGBA
            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_BIND_TO_TEXTURE_RGB_EXT, &nValue );
            if( nValue == False )
            {
                glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &nValue );
                if( nValue == False )
                    continue;
            }

            glXGetFBConfigAttrib( pDisplay, aFbConfigs[i], GLX_Y_INVERTED_EXT, &nValue );

            // Looks like that X sends GLX_DONT_CARE but this usually means "true" for most
            // of the X implementations. Investigation on internet pointed that this could be
            // safely "true" all the time (for example gnome-shell always assumes "true").
            bInverted = nValue == True || nValue == int(GLX_DONT_CARE);

            break;
        }

        if( i == nFbConfigs )
        {
            SAL_WARN( "vcl.opengl", "Unable to find FBconfig for pixmap texturing" );
            return nullptr;
        }

        CHECK_GL_ERROR();
        return aFbConfigs[i];
    }
}

bool X11OpenGLSalGraphicsImpl::RenderPixmap(X11Pixmap const * pPixmap, X11Pixmap const * pMask, int nX, int nY, TextureCombo& rCombo)
{
    const int aAttribs[] =
    {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
    };

    Display* pDisplay = mrX11Parent.GetXDisplay();
    bool bInverted = false;

    const long nWidth = pPixmap->GetWidth();
    const long nHeight = pPixmap->GetHeight();
    SalTwoRect aPosAry(0, 0, nWidth, nHeight, nX, nY, nWidth, nHeight);

    PreDraw();
    //glClear( GL_COLOR_BUFFER_BIT );

    XSync( pDisplay, 0 );
    GLXFBConfig pFbConfig = GetPixmapFBConfig( pDisplay, bInverted );
    GLXPixmap pGlxPixmap = glXCreatePixmap( pDisplay, pFbConfig, pPixmap->GetPixmap(), aAttribs);
    GLXPixmap pGlxMask;
    if( pMask != nullptr )
        pGlxMask = glXCreatePixmap( pDisplay, pFbConfig, pMask->GetPixmap(), aAttribs);
    else
        pGlxMask = 0;
    XSync( pDisplay, 0 );

    if( !pGlxPixmap )
        SAL_WARN( "vcl.opengl", "Couldn't create GLXPixmap" );

    //TODO: lfrb: glXGetProc to get the functions

    rCombo.mpTexture.reset(new OpenGLTexture(pPixmap->GetWidth(), pPixmap->GetHeight(), false));

    mpContext->state().texture().active(0);

    rCombo.mpTexture->Bind();
    glXBindTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT, nullptr );
    rCombo.mpTexture->Unbind();

    if( pMask != nullptr && pGlxMask )
    {
        rCombo.mpMask.reset(new OpenGLTexture(pPixmap->GetWidth(), pPixmap->GetHeight(), false));
        rCombo.mpMask->Bind();
        glXBindTexImageEXT( pDisplay, pGlxMask, GLX_FRONT_LEFT_EXT, nullptr );
        rCombo.mpMask->Unbind();

        DrawTextureDiff(*rCombo.mpTexture, *rCombo.mpMask, aPosAry, bInverted);

        glXReleaseTexImageEXT( pDisplay, pGlxMask, GLX_FRONT_LEFT_EXT );
        glXDestroyPixmap( pDisplay, pGlxMask );
    }
    else
    {
        DrawTexture(*rCombo.mpTexture, aPosAry, bInverted);
    }

    CHECK_GL_ERROR();

    glXReleaseTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT );
    glXDestroyPixmap( pDisplay, pGlxPixmap );

    PostDraw();

    CHECK_GL_ERROR();

    return true;
}

bool X11OpenGLSalGraphicsImpl::RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY )
{
    SAL_INFO( "vcl.opengl", "RenderPixmapToScreen (" << nX << " " << nY << ")" );

    TextureCombo aCombo;
    return RenderPixmap(pPixmap, pMask, nX, nY, aCombo);
}

bool X11OpenGLSalGraphicsImpl::TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey, int nX, int nY)
{
    static bool gbCacheEnabled = !getenv("SAL_WITHOUT_WIDGET_CACHE");

    if (!gbCacheEnabled || !gTextureCache.get())
        return false;

    ControlCacheType::const_iterator iterator = gTextureCache.get()->find(rControlCacheKey);

    if (iterator == gTextureCache.get()->end())
        return false;

    const std::unique_ptr<TextureCombo>& pCombo = iterator->second;

    PreDraw();

    OpenGLTexture& rTexture = *pCombo->mpTexture;

    SalTwoRect aPosAry(0,  0,  rTexture.GetWidth(), rTexture.GetHeight(),
                       nX, nY, rTexture.GetWidth(), rTexture.GetHeight());

    if (pCombo->mpMask)
        DrawTextureDiff(rTexture, *pCombo->mpMask, aPosAry, true);
    else
        DrawTexture(rTexture, aPosAry, true);

    PostDraw();

    return true;
}

bool X11OpenGLSalGraphicsImpl::RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                                           ControlCacheKey& aControlCacheKey)
{
    std::unique_ptr<TextureCombo> pCombo(new TextureCombo);
    bool bResult = RenderPixmap(pPixmap, pMask, nX, nY, *pCombo);
    if (!bResult)
        return false;

    if (!aControlCacheKey.canCacheControl())
        return true;

    ControlCachePair pair(aControlCacheKey, std::move(pCombo));
    if (gTextureCache.get())
        gTextureCache.get()->insert(std::move(pair));

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
