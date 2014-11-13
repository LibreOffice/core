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

void X11OpenGLSalGraphicsImpl::Init()
{
    X11WindowProvider *pProvider = dynamic_cast<X11WindowProvider*>(mrParent.m_pFrame);
    if (pProvider)
    {
        Window aWin = pProvider->GetX11Window();
        maContext.init( mrParent.GetXDisplay(), aWin, mrParent.m_nXScreen.getXScreen());
        SetOffscreen( false );
    }
    else if( mrParent.m_pVDev )
    {
        maContext.init( mrParent.GetXDisplay(), mrParent.m_pVDev->GetDrawable(),
                        mrParent.m_pVDev->GetWidth(), mrParent.m_pVDev->GetHeight(),
                        mrParent.m_nXScreen.getXScreen() );
        SetOffscreen( true );
    }
    else
    {
        SAL_WARN( "vcl.opengl", "what happened here?" );
    }
}

X11Pixmap* X11OpenGLSalGraphicsImpl::GetPixmapFromScreen( const Rectangle& rRect )
{
    Display* pDisplay = mrParent.GetXDisplay();
    SalX11Screen nScreen = mrParent.GetScreenNumber();
    XVisualInfo aVisualInfo;
    X11Pixmap* pPixmap;
    XImage* pImage;
    char* pData;

    SAL_INFO( "vcl.opengl", "GetPixmapFromScreen" );
    // TODO: lfrb: Use context depth
    pPixmap = new X11Pixmap( pDisplay, nScreen, rRect.GetWidth(), rRect.GetHeight(), 24 );

    if( !OpenGLHelper::GetVisualInfo( pDisplay, nScreen.getXScreen(), aVisualInfo ) )
        return pPixmap;

    // make sure everything is synced up before reading back
    maContext.makeCurrent();
    glXWaitX();

    // TODO: lfrb: What if offscreen?
    pData = (char*) malloc( rRect.GetWidth() * rRect.GetHeight() * 4 );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( rRect.Left(), GetHeight() - rRect.Top(), rRect.GetWidth(), rRect.GetHeight(),
                  GL_RGBA, GL_UNSIGNED_BYTE, pData );

    pImage = XCreateImage( pDisplay, aVisualInfo.visual, 24, ZPixmap, 0, pData,
                           rRect.GetWidth(), rRect.GetHeight(), 8, 0 );
    XInitImage( pImage );
    GC aGC = XCreateGC( pDisplay, pPixmap->GetPixmap(), 0, NULL );
    XPutImage( pDisplay, pPixmap->GetDrawable(), aGC, pImage,
               0, 0, 0, 0, rRect.GetWidth(), rRect.GetHeight() );
    XFreeGC( pDisplay, aGC );
    XDestroyImage( pImage );

    return pPixmap;
}

bool X11OpenGLSalGraphicsImpl::RenderPixmapToScreen( X11Pixmap* pPixmap, int nX, int nY )
{
    const int aAttribs[] = {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
        None
    };
    Display* pDisplay = mrParent.GetXDisplay();
    GLXFBConfig pFbConfig;
    GLXPixmap pGlxPixmap;
    GLuint nTexture;
    SalTwoRect aPosAry;
    bool bInverted;

    SAL_INFO( "vcl.opengl", "RenderPixmapToScreen (" << nX << " " << nY << ")" );

    aPosAry.mnSrcX = 0;
    aPosAry.mnSrcY = 0;
    aPosAry.mnDestX = nX;
    aPosAry.mnDestY = nY;
    aPosAry.mnSrcWidth = aPosAry.mnDestWidth = pPixmap->GetWidth();
    aPosAry.mnSrcHeight = aPosAry.mnDestHeight = pPixmap->GetHeight();

    XSync( pDisplay, 0 );
    pFbConfig = OpenGLHelper::GetPixmapFBConfig( pDisplay, bInverted );
    pGlxPixmap = glXCreatePixmap( pDisplay, pFbConfig, pPixmap->GetPixmap(), aAttribs);
    XSync( pDisplay, 0 );

    PreDraw();

    glGenTextures( 1, &nTexture );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, nTexture );

    //TODO: lfrb: glXGetProc to get the functions
    glXBindTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT, NULL );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    DrawTexture( nTexture, pPixmap->GetSize(), aPosAry, !bInverted );

    glXReleaseTexImageEXT( pDisplay, pGlxPixmap, GLX_FRONT_LEFT_EXT );
    glDeleteTextures( 1, &nTexture );
    glXDestroyPixmap( pDisplay, pGlxPixmap );

    PostDraw();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
