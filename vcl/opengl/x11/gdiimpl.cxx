/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/salbtype.hxx>

#include <svdata.hxx>

#include <unx/pixmap.hxx>
#include <unx/saldisp.hxx>
#include <unx/salframe.h>
#include <unx/salgdi.h>
#include <unx/salvd.h>
#include <unx/x11/xlimits.hxx>

#include <opengl/texture.hxx>
#include <opengl/x11/gdiimpl.hxx>
#include <opengl/x11/salvd.hxx>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <o3tl/lru_map.hxx>

X11OpenGLSalGraphicsImpl::X11OpenGLSalGraphicsImpl( X11SalGraphics& rParent ):
    OpenGLSalGraphicsImpl(rParent,rParent.GetGeometryProvider()),
    mrParent(rParent)
{
}

X11OpenGLSalGraphicsImpl::~X11OpenGLSalGraphicsImpl()
{
}

void X11OpenGLSalGraphicsImpl::Init()
{
    // The m_pFrame and m_pVDev pointers are updated late in X11
    mpProvider = mrParent.GetGeometryProvider();
    OpenGLSalGraphicsImpl::Init();
}

rtl::Reference<OpenGLContext> X11OpenGLSalGraphicsImpl::CreateWinContext()
{
    X11WindowProvider *pProvider = dynamic_cast<X11WindowProvider*>(mrParent.m_pFrame);

    if( !pProvider )
        return NULL;

    Window aWin = pProvider->GetX11Window();
    rtl::Reference<OpenGLContext> pContext = OpenGLContext::Create();
    pContext->init( mrParent.GetXDisplay(), aWin,
                    mrParent.m_nXScreen.getXScreen() );
    return pContext;
}

bool X11OpenGLSalGraphicsImpl::UseContext( const rtl::Reference<OpenGLContext> &pContext )
{
    X11WindowProvider *pProvider = dynamic_cast<X11WindowProvider*>(mrParent.m_pFrame);

    if( !pContext->isInitialized() || IsForeignContext( pContext ) )
        return false;
    if( !pProvider )
        return pContext->getOpenGLWindow().win != None;
    else
        return pContext->getOpenGLWindow().win == pProvider->GetX11Window();
}

void X11OpenGLSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    OpenGLSalGraphicsImpl *pImpl = pSrcGraphics ? static_cast< OpenGLSalGraphicsImpl* >(pSrcGraphics->GetImpl()) : static_cast< OpenGLSalGraphicsImpl *>(mrParent.GetImpl());
    OpenGLSalGraphicsImpl::DoCopyBits( rPosAry, *pImpl );
}

bool X11OpenGLSalGraphicsImpl::FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY )
{
    Display* pDisplay = mrParent.GetXDisplay();
    SalX11Screen nScreen = mrParent.GetScreenNumber();
    XVisualInfo aVisualInfo;
    XImage* pImage;
    char* pData;

    SAL_INFO( "vcl.opengl", "FillPixmapFromScreen" );

    if( !OpenGLHelper::GetVisualInfo( pDisplay, nScreen.getXScreen(), aVisualInfo ) )
        return false;

    // make sure everything is synced up before reading back
    mpContext->makeCurrent();
    glXWaitX();

    // TODO: lfrb: What if offscreen?
    pData = static_cast<char*>(malloc( pPixmap->GetWidth() * pPixmap->GetHeight() * 4 ));
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( nX, GetHeight() - nY, pPixmap->GetWidth(), pPixmap->GetHeight(),
                  GL_RGBA, GL_UNSIGNED_BYTE, pData );

    pImage = XCreateImage( pDisplay, aVisualInfo.visual, 24, ZPixmap, 0, pData,
                           pPixmap->GetWidth(), pPixmap->GetHeight(), 8, 0 );
    XInitImage( pImage );
    GC aGC = XCreateGC( pDisplay, pPixmap->GetPixmap(), 0, NULL );
    XPutImage( pDisplay, pPixmap->GetDrawable(), aGC, pImage,
               0, 0, 0, 0, pPixmap->GetWidth(), pPixmap->GetHeight() );
    XFreeGC( pDisplay, aGC );
    XDestroyImage( pImage );

    CHECK_GL_ERROR();
    return true;
}

typedef typename std::pair<ControlCacheKey, std::unique_ptr<TextureCombo>> ControlCachePair;
typedef o3tl::lru_map<ControlCacheKey, std::unique_ptr<TextureCombo>, ControlCacheHashFunction> ControlCacheType;

ControlCacheType gTextureCache(200);

bool X11OpenGLSalGraphicsImpl::RenderPixmap(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY, TextureCombo& rCombo)
{
    const int aAttribs[] =
    {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
    };

    Display* pDisplay = mrParent.GetXDisplay();
    bool bInverted;

    const long nWidth = pPixmap->GetWidth();
    const long nHeight = pPixmap->GetHeight();
    SalTwoRect aPosAry(0, 0, nWidth, nHeight, nX, nY, nWidth, nHeight);

    PreDraw();
    //glClear( GL_COLOR_BUFFER_BIT );

    XSync( pDisplay, 0 );
    GLXFBConfig pFbConfig = OpenGLHelper::GetPixmapFBConfig( pDisplay, bInverted );
    GLXPixmap pGlxPixmap = glXCreatePixmap( pDisplay, pFbConfig, pPixmap->GetPixmap(), aAttribs);
    GLXPixmap pGlxMask;
    if( pMask != NULL )
        pGlxMask = glXCreatePixmap( pDisplay, pFbConfig, pMask->GetPixmap(), aAttribs);
    else
        pGlxMask = 0;
    XSync( pDisplay, 0 );

    if( !pGlxPixmap )
        SAL_WARN( "vcl.opengl", "Couldn't create GLXPixmap" );

    //TODO: lfrb: glXGetProc to get the functions

    rCombo.mpTexture.reset(new OpenGLTexture(pPixmap->GetWidth(), pPixmap->GetHeight(), false));

    glActiveTexture( GL_TEXTURE0 );
    rCombo.mpTexture->Bind();
    glXBindTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT, NULL );
    rCombo.mpTexture->Unbind();

    if( pMask != NULL && pGlxMask )
    {
        rCombo.mpMask.reset(new OpenGLTexture(pPixmap->GetWidth(), pPixmap->GetHeight(), false));
        rCombo.mpMask->Bind();
        glXBindTexImageEXT( pDisplay, pGlxMask, GLX_FRONT_LEFT_EXT, NULL );
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

    if (!gbCacheEnabled)
        return false;

    ControlCacheType::const_iterator iterator = gTextureCache.find(rControlCacheKey);

    if (iterator == gTextureCache.end())
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
    gTextureCache.insert(std::move(pair));

    return bResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
