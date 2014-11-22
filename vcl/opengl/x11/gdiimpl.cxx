/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vcl/salbtype.hxx"

#include "unx/pixmap.hxx"
#include "unx/saldisp.hxx"
#include "unx/salframe.h"
#include "unx/salgdi.h"
#include "unx/salvd.h"

#include "opengl/texture.hxx"
#include "opengl/x11/gdiimpl.hxx"

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

X11OpenGLSalGraphicsImpl::X11OpenGLSalGraphicsImpl( X11SalGraphics& rParent ):
    OpenGLSalGraphicsImpl(),
    mrParent(rParent)
{
}

X11OpenGLSalGraphicsImpl::~X11OpenGLSalGraphicsImpl()
{
}

GLfloat X11OpenGLSalGraphicsImpl::GetWidth() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.nWidth;
    else if( mrParent.m_pVDev )
    {
        long nWidth = 0;
        long nHeight = 0;
        mrParent.m_pVDev->GetSize( nWidth, nHeight );
        return nWidth;
    }
    return 1;
}

GLfloat X11OpenGLSalGraphicsImpl::GetHeight() const
{
    if( mrParent.m_pFrame )
        return mrParent.m_pFrame->maGeometry.nHeight;
    else if( mrParent.m_pVDev )
    {
        long nWidth = 0;
        long nHeight = 0;
        mrParent.m_pVDev->GetSize( nWidth, nHeight );
        return nHeight;
    }
    return 1;
}

bool X11OpenGLSalGraphicsImpl::IsOffscreen() const
{
    X11WindowProvider *pProvider = dynamic_cast<X11WindowProvider*>(mrParent.m_pFrame);
    if( pProvider )
        return false;
    else if( mrParent.m_pVDev )
        return true;
    else
    {
        SAL_WARN( "vcl.opengl", "what happened here?" );
        return true;
    }
}

OpenGLContext* X11OpenGLSalGraphicsImpl::CreateWinContext()
{
    X11WindowProvider *pProvider = dynamic_cast<X11WindowProvider*>(mrParent.m_pFrame);

    if( !pProvider )
        return NULL;
    Window aWin = pProvider->GetX11Window();
    OpenGLContext* pContext = new OpenGLContext();
    pContext->init( mrParent.GetXDisplay(), aWin,
                    mrParent.m_nXScreen.getXScreen() );
    return pContext;
}

bool X11OpenGLSalGraphicsImpl::CompareWinContext( OpenGLContext* pContext )
{
    X11WindowProvider *pProvider = dynamic_cast<X11WindowProvider*>(mrParent.m_pFrame);

    if( !pProvider || !pContext->isInitialized() )
        return false;
    return ( pContext->getOpenGLWindow().win == pProvider->GetX11Window() );
}

OpenGLContext* X11OpenGLSalGraphicsImpl::CreatePixmapContext()
{
    if( mrParent.m_pVDev == NULL )
        return NULL;
    OpenGLContext* pContext = new OpenGLContext();
    pContext->init( mrParent.GetXDisplay(), mrParent.m_pVDev->GetDrawable(),
                    mrParent.m_pVDev->GetWidth(), mrParent.m_pVDev->GetHeight(),
                    mrParent.m_nXScreen.getXScreen() );
    return pContext;
}

void X11OpenGLSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    OpenGLSalGraphicsImpl *pImpl = pSrcGraphics ? dynamic_cast< OpenGLSalGraphicsImpl* >(pSrcGraphics->GetImpl()) : static_cast< OpenGLSalGraphicsImpl *>(mrParent.GetImpl());
    OpenGLSalGraphicsImpl::DoCopyBits( rPosAry, pImpl );
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
    pData = (char*) malloc( pPixmap->GetWidth() * pPixmap->GetHeight() * 4 );
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

bool X11OpenGLSalGraphicsImpl::RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY )
{
    const int aAttribs[] = {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT,
        None
    };
    Display* pDisplay = mrParent.GetXDisplay();
    GLXFBConfig pFbConfig;
    GLXPixmap pGlxPixmap;
    GLXPixmap pGlxMask;
    SalTwoRect aPosAry;
    bool bInverted;

    SAL_INFO( "vcl.opengl", "RenderPixmapToScreen (" << nX << " " << nY << ")" );

    aPosAry.mnSrcX = 0;
    aPosAry.mnSrcY = 0;
    aPosAry.mnDestX = nX;
    aPosAry.mnDestY = nY;
    aPosAry.mnSrcWidth = aPosAry.mnDestWidth = pPixmap->GetWidth();
    aPosAry.mnSrcHeight = aPosAry.mnDestHeight = pPixmap->GetHeight();

    PreDraw();
    //glClear( GL_COLOR_BUFFER_BIT );

    XSync( pDisplay, 0 );
    pFbConfig = OpenGLHelper::GetPixmapFBConfig( pDisplay, bInverted );
    pGlxPixmap = glXCreatePixmap( pDisplay, pFbConfig, pPixmap->GetPixmap(), aAttribs);
    if( pMask != NULL )
        pGlxMask = glXCreatePixmap( pDisplay, pFbConfig, pMask->GetPixmap(), aAttribs);
    XSync( pDisplay, 0 );

    if( !pGlxPixmap )
        SAL_WARN( "vcl.opengl", "Couldn't create GLXPixmap" );

    //TODO: lfrb: glXGetProc to get the functions

    OpenGLTexture aTexture( pPixmap->GetWidth(), pPixmap->GetHeight(), false );
    glActiveTexture( GL_TEXTURE0 );
    aTexture.Bind();
    glXBindTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT, NULL );
    aTexture.Unbind();

    if( pMask != NULL && pGlxMask )
    {
        OpenGLTexture aMaskTexture( pMask->GetWidth(), pMask->GetHeight(), false );
        aMaskTexture.Bind();
        glXBindTexImageEXT( pDisplay, pGlxMask, GLX_FRONT_LEFT_EXT, NULL );
        aMaskTexture.Unbind();

        DrawTextureDiff( aTexture, aMaskTexture, aPosAry, bInverted );

        glXReleaseTexImageEXT( pDisplay, pGlxMask, GLX_FRONT_LEFT_EXT );
        glXDestroyPixmap( pDisplay, pGlxMask );
    }
    else
    {
        DrawTexture( aTexture, aPosAry, bInverted );
    }

    CHECK_GL_ERROR();

    glXReleaseTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT );
    glXDestroyPixmap( pDisplay, pGlxPixmap );

    PostDraw();

    CHECK_GL_ERROR();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
