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

#include <salgdiimpl.hxx>

#include <vcl/sysdata.hxx>

#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/x11/xrender_peer.hxx>
#include <salframe.hxx>

extern "C"
{
    static Bool GraphicsExposePredicate( Display*, XEvent* pEvent, const XPointer pFrameWindow )
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
        for (auto pSalFrame : vcl_sal::getSalDisplay(GetGenericUnixSalData())->getFrames() )
        {
            const SystemEnvData* pEnvData = pSalFrame->GetSystemData();
            if( Drawable(pEnvData->GetWindowHandle(pSalFrame)) == aWindow )
            {
                pFrame = pSalFrame;
                break;
            }
        }
        if( ! pFrame )
            return;
    }

    XEvent aEvent;
    while( XCheckTypedWindowEvent( pDisplay, aWindow, Expose, &aEvent ) )
    {
        SalPaintEvent aPEvt( aEvent.xexpose.x, aEvent.xexpose.y, aEvent.xexpose.width+1, aEvent.xexpose.height+1 );
        pFrame->CallCallback( SalEvent::Paint, &aPEvt );
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
            pFrame->CallCallback( SalEvent::Paint, &aPEvt );
        }
    } while( aEvent.xgraphicsexpose.count != 0 );
}

void X11SalGraphics::copyBits( const SalTwoRect& rPosAry,
                                  SalGraphics      *pSSrcGraphics )
{
    mxImpl->copyBits( rPosAry, pSSrcGraphics );
}

void X11SalGraphics::copyArea ( tools::Long nDestX,    tools::Long nDestY,
                                tools::Long nSrcX,     tools::Long nSrcY,
                                tools::Long nSrcWidth, tools::Long nSrcHeight,
                                bool bWindowInvalidate)
{
    mxImpl->copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, bWindowInvalidate );
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
    const SalBitmap* pAlphaBitmap,
    double fAlpha)
{
    // called mxImpl->drawTransformedBitmap anyways returns false, but spares a call
    if( fAlpha != 1.0 )
        return false;

    return mxImpl->drawTransformedBitmap( rNull, rX, rY, rSourceBitmap, pAlphaBitmap, fAlpha );
}

bool X11SalGraphics::drawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth,
                                    tools::Long nHeight, sal_uInt8 nTransparency )
{
    return mxImpl->drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

void X11SalGraphics::drawMask( const SalTwoRect& rPosAry,
                               const SalBitmap &rSalBitmap,
                               Color nMaskColor )
{
    mxImpl->drawMask( rPosAry, rSalBitmap, nMaskColor );
}

std::shared_ptr<SalBitmap> X11SalGraphics::getBitmap( tools::Long nX, tools::Long nY, tools::Long nDX, tools::Long nDY )
{
    return mxImpl->getBitmap( nX, nY, nDX, nDY );
}

Color X11SalGraphics::getPixel( tools::Long nX, tools::Long nY )
{
    return mxImpl->getPixel( nX, nY );
}

void X11SalGraphics::invert( tools::Long       nX,
                                tools::Long        nY,
                                tools::Long        nDX,
                                tools::Long        nDY,
                                SalInvert   nFlags )
{
    mxImpl->invert( nX, nY, nDX, nDY, nFlags );
}

bool X11SalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    return mxImpl->supportsOperation(eType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
