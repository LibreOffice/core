/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <poll.h>
#include "salgdiimpl.hxx"

#include "vcl/salbtype.hxx"

#include "unx/pixmap.hxx"
#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salbmp.h"
#include "unx/salgdi.h"
#include "unx/salframe.h"
#include "unx/salvd.h"
#include "unx/x11/x11gdiimpl.h"
#include <unx/x11/xlimits.hxx>
#include "xrender_peer.hxx"

#include "generic/printergfx.hxx"

#include "vcl/bitmapaccess.hxx"
#include <outdata.hxx>

void X11SalGraphics::CopyScreenArea( Display* pDisplay,
                                     Drawable aSrc, SalX11Screen nXScreenSrc, int nSrcDepth,
                                     Drawable aDest, SalX11Screen nXScreenDest, int nDestDepth,
                                     GC aDestGC,
                                     int src_x, int src_y,
                                     unsigned int w, unsigned int h,
                                     int dest_x, int dest_y )
{
    if( nSrcDepth == nDestDepth )
    {
        if( nXScreenSrc == nXScreenDest )
            XCopyArea( pDisplay, aSrc, aDest, aDestGC,
                       src_x, src_y, w, h, dest_x, dest_y );
        else
        {
            GetGenericData()->ErrorTrapPush();
            XImage* pImage = XGetImage( pDisplay, aSrc, src_x, src_y, w, h,
                                        AllPlanes, ZPixmap );
            if( pImage )
            {
                if( pImage->data )
                    XPutImage( pDisplay, aDest, aDestGC, pImage,
                               0, 0, dest_x, dest_y, w, h );
                XDestroyImage( pImage );
            }
            GetGenericData()->ErrorTrapPop();
        }
    }
    else
    {
        X11SalBitmap aBM;
        aBM.ImplCreateFromDrawable( aSrc, nXScreenSrc, nSrcDepth, src_x, src_y, w, h );
        SalTwoRect aTwoRect(0, 0, w, h, dest_x, dest_y, w, h);
        aBM.ImplDraw(aDest, nXScreenDest, nDestDepth, aTwoRect,aDestGC);
    }
}

bool X11SalGraphics::FillPixmapFromScreen( X11Pixmap* pPixmap, int nX, int nY )
{
    X11GraphicsImpl& rImpl = dynamic_cast<X11GraphicsImpl&>(*mxImpl.get());
    return rImpl.FillPixmapFromScreen( pPixmap, nX, nY );
}

bool X11SalGraphics::RenderPixmapToScreen( X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY )
{
    SAL_INFO( "vcl", "RenderPixmapToScreen" );
    X11GraphicsImpl& rImpl = dynamic_cast<X11GraphicsImpl&>(*mxImpl.get());
    return rImpl.RenderPixmapToScreen( pPixmap, pMask, nX, nY );
}

bool X11SalGraphics::TryRenderCachedNativeControl(ControlCacheKey& rControlCacheKey, int nX, int nY)
{
    SAL_INFO( "vcl", "TryRenderCachedNativeControl" );
    X11GraphicsImpl& rImpl = dynamic_cast<X11GraphicsImpl&>(*mxImpl.get());
    return rImpl.TryRenderCachedNativeControl(rControlCacheKey, nX, nY);
}

bool X11SalGraphics::RenderAndCacheNativeControl(X11Pixmap* pPixmap, X11Pixmap* pMask, int nX, int nY,
                                                 ControlCacheKey& rControlCacheKey)
{
    SAL_INFO( "vcl", "RenderAndCachePixmap" );
    X11GraphicsImpl& rImpl = dynamic_cast<X11GraphicsImpl&>(*mxImpl.get());
    return rImpl.RenderAndCacheNativeControl(pPixmap, pMask, nX, nY, rControlCacheKey);
}

extern "C"
{
    static Bool GraphicsExposePredicate( Display*, XEvent* pEvent, XPointer pFrameWindow )
    {
        Bool bRet = False;
        if( (pEvent->type == GraphicsExpose || pEvent->type == NoExpose) &&
            pEvent->xnoexpose.drawable == reinterpret_cast<Drawable>(pFrameWindow) )
        {
            bRet = True;
        }
        return bRet;
    }
}

void X11SalGraphics::YieldGraphicsExpose()
{
    // get frame if necessary
    SalFrame* pFrame    = m_pFrame;
    Display* pDisplay   = GetXDisplay();
    ::Window aWindow = GetDrawable();
    if( ! pFrame )
    {
        const std::list< SalFrame* >& rFrames = vcl_sal::getSalDisplay(GetGenericData())->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end() && ! pFrame; ++it )
        {
            const SystemEnvData* pEnvData = (*it)->GetSystemData();
            if( Drawable(pEnvData->aWindow) == aWindow )
                pFrame = *it;
        }
        if( ! pFrame )
            return;
    }

    XEvent aEvent;
    while( XCheckTypedWindowEvent( pDisplay, aWindow, Expose, &aEvent ) )
    {
        SalPaintEvent aPEvt( aEvent.xexpose.x, aEvent.xexpose.y, aEvent.xexpose.width+1, aEvent.xexpose.height+1 );
        pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
    }

    do
    {
        if( ! GetDisplay()->XIfEventWithTimeout( &aEvent, reinterpret_cast<XPointer>(aWindow), GraphicsExposePredicate ) )
            // this should not happen at all; still sometimes it happens
            break;

        if( aEvent.type == NoExpose )
            break;

        if( pFrame )
        {
            SalPaintEvent aPEvt( aEvent.xgraphicsexpose.x, aEvent.xgraphicsexpose.y, aEvent.xgraphicsexpose.width+1, aEvent.xgraphicsexpose.height+1 );
            pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
        }
    } while( aEvent.xgraphicsexpose.count != 0 );
}

void X11SalGraphics::copyBits( const SalTwoRect& rPosAry,
                                  SalGraphics      *pSSrcGraphics )
{
    mxImpl->copyBits( rPosAry, pSSrcGraphics );
}

void X11SalGraphics::copyArea ( long nDestX,    long nDestY,
                                long nSrcX,     long nSrcY,
                                long nSrcWidth, long nSrcHeight,
                                sal_uInt16 n )
{
    mxImpl->copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, n );
}

bool X11SalGraphics::blendBitmap( const SalTwoRect& rTR,
                                  const SalBitmap& rBitmap )
{
    return mxImpl->blendBitmap( rTR, rBitmap );
}

bool X11SalGraphics::blendAlphaBitmap( const SalTwoRect& rTR,
                                       const SalBitmap& rSrcBitmap,
                                       const SalBitmap& rMaskBitmap,
                                       const SalBitmap& rAlphaBitmap )
{
    return mxImpl->blendAlphaBitmap( rTR, rSrcBitmap, rMaskBitmap, rAlphaBitmap );
}

void X11SalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    mxImpl->drawBitmap( rPosAry, rSalBitmap );
}

void X11SalGraphics::drawBitmap( const SalTwoRect& rPosAry,
                                 const SalBitmap& rSrcBitmap,
                                 const SalBitmap& rMaskBitmap )
{
    mxImpl->drawBitmap( rPosAry, rSrcBitmap, rMaskBitmap );
}

bool X11SalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap, const SalBitmap& rAlphaBmp )
{
    return mxImpl->drawAlphaBitmap( rTR, rSrcBitmap, rAlphaBmp );
}

bool X11SalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    return mxImpl->drawTransformedBitmap( rNull, rX, rY, rSourceBitmap, pAlphaBitmap );
}

bool X11SalGraphics::drawAlphaRect( long nX, long nY, long nWidth,
                                    long nHeight, sal_uInt8 nTransparency )
{
    return mxImpl->drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

void X11SalGraphics::drawMask( const SalTwoRect& rPosAry,
                               const SalBitmap &rSalBitmap,
                               SalColor nMaskColor )
{
    mxImpl->drawMask( rPosAry, rSalBitmap, nMaskColor );
}

SalBitmap *X11SalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
    return mxImpl->getBitmap( nX, nY, nDX, nDY );
}

SalColor X11SalGraphics::getPixel( long nX, long nY )
{
    return mxImpl->getPixel( nX, nY );
}

void X11SalGraphics::invert( long       nX,
                                long        nY,
                                long        nDX,
                                long        nDY,
                                SalInvert   nFlags )
{
    mxImpl->invert( nX, nY, nDX, nDY, nFlags );
}

bool X11SalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    bool bRet = false;
    switch( eType )
    {
    case OutDevSupport_TransparentRect:
    case OutDevSupport_B2DDraw:
        {
            XRenderPeer& rPeer = XRenderPeer::GetInstance();
            const SalDisplay* pSalDisp = GetDisplay();
            const SalVisual& rSalVis = pSalDisp->GetVisual( m_nXScreen );

            Visual* pDstXVisual = rSalVis.GetVisual();
            XRenderPictFormat* pDstVisFmt = rPeer.FindVisualFormat( pDstXVisual );
            if( pDstVisFmt )
                bRet = true;
        }
        break;
    default: break;
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
