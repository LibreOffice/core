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

#include <limits.h>
#include <tools/time.hxx>

#include <strings.hrc>
#include <bitmaps.hlst>
#include <svdata.hxx>
#include <scrwnd.hxx>

#include <vcl/timer.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>
#include <sal/log.hxx>

#include <math.h>

#define WHEEL_WIDTH     25
#define WHEEL_RADIUS    ((WHEEL_WIDTH) >> 1 )
#define MAX_TIME        300
#define MIN_TIME        20
#define DEF_TIMEOUT     50

ImplWheelWindow::ImplWheelWindow( vcl::Window* pParent ) :
            FloatingWindow  ( pParent, 0 ),
            mnRepaintTime   ( 1 ),
            mnTimeout       ( DEF_TIMEOUT ),
            mnWheelMode     ( WheelMode::NONE ),
            mnActDist       ( 0 ),
            mnActDeltaX     ( 0 ),
            mnActDeltaY     ( 0 )
{
    // we need a parent
    SAL_WARN_IF( !pParent, "vcl", "ImplWheelWindow::ImplWheelWindow(): Parent not set!" );

    const Size      aSize( pParent->GetOutputSizePixel() );
    const StartAutoScrollFlags nFlags = ImplGetSVData()->maWinData.mnAutoScrollFlags;
    const bool      bHorz( nFlags & StartAutoScrollFlags::Horz );
    const bool      bVert( nFlags & StartAutoScrollFlags::Vert );

    // calculate maximum speed distance
    mnMaxWidth = static_cast<sal_uLong>( 0.4 * hypot( static_cast<double>(aSize.Width()), aSize.Height() ) );

    // create wheel window
    SetTitleType( FloatWinTitleType::NONE );
    ImplCreateImageList();
    BitmapEx aBmp(SV_RESID_BITMAP_SCROLLMSK);
    ImplSetRegion(aBmp.GetBitmap());

    // set wheel mode
    if( bHorz && bVert )
        ImplSetWheelMode( WheelMode::VH );
    else if( bHorz )
        ImplSetWheelMode( WheelMode::H );
    else
        ImplSetWheelMode( WheelMode::V );

    // init timer
    mpTimer.reset(new Timer("WheelWindowTimer"));
    mpTimer->SetInvokeHandler( LINK( this, ImplWheelWindow, ImplScrollHdl ) );
    mpTimer->SetTimeout( mnTimeout );
    mpTimer->Start();

    CaptureMouse();
}

ImplWheelWindow::~ImplWheelWindow()
{
    disposeOnce();
}

void ImplWheelWindow::dispose()
{
    ImplStop();
    mpTimer.reset();
    FloatingWindow::dispose();
}

void ImplWheelWindow::ImplStop()
{
    ReleaseMouse();
    mpTimer->Stop();
    Show(false);
}

void ImplWheelWindow::ImplSetRegion( const Bitmap& rRegionBmp )
{
    Point           aPos( GetPointerPosPixel() );
    const Size      aSize( rRegionBmp.GetSizePixel() );
    const tools::Rectangle aRect( Point(), aSize );

    maCenter = maLastMousePos = aPos;
    aPos.AdjustX( -(aSize.Width() >> 1) );
    aPos.AdjustY( -(aSize.Height() >> 1) );

    SetPosSizePixel( aPos, aSize );
    SetWindowRegionPixel( rRegionBmp.CreateRegion( COL_BLACK, aRect ) );
}

void ImplWheelWindow::ImplCreateImageList()
{
    maImgList.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_SCROLLVH));
    maImgList.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_SCROLLV));
    maImgList.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_SCROLLH));
    maImgList.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_WHEELVH));
    maImgList.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_WHEELV));
    maImgList.emplace_back(Image(StockImage::Yes, SV_RESID_BITMAP_WHEELH));
}

void ImplWheelWindow::ImplSetWheelMode( WheelMode nWheelMode )
{
    if( nWheelMode != mnWheelMode )
    {
        mnWheelMode = nWheelMode;

        if( WheelMode::NONE == mnWheelMode )
        {
            if( IsVisible() )
                Hide();
        }
        else
        {
            if( !IsVisible() )
                Show();

            Invalidate();
        }
    }
}

void ImplWheelWindow::ImplDrawWheel(vcl::RenderContext& rRenderContext)
{
    int nIndex;

    switch (mnWheelMode)
    {
        case WheelMode::VH:
            nIndex = 0;
        break;
        case WheelMode::V:
            nIndex = 1;
        break;
        case WheelMode::H:
            nIndex = 2;
        break;
        case WheelMode::ScrollVH:
            nIndex = 3;
        break;
        case WheelMode::ScrollV:
            nIndex = 4;
        break;
        case WheelMode::ScrollH:
            nIndex = 5;
        break;
        default:
            nIndex = -1;
        break;
    }

    if (nIndex >= 0)
        rRenderContext.DrawImage(Point(), maImgList[nIndex]);
}

void ImplWheelWindow::ImplRecalcScrollValues()
{
    if( mnActDist < WHEEL_RADIUS )
    {
        mnActDeltaX = mnActDeltaY = 0;
        mnTimeout = DEF_TIMEOUT;
    }
    else
    {
        sal_uInt64 nCurTime;

        // calc current time
        if( mnMaxWidth )
        {
            const double fExp = ( static_cast<double>(mnActDist) / mnMaxWidth ) * log10( double(MAX_TIME) / MIN_TIME );
            nCurTime = static_cast<sal_uInt64>( MAX_TIME / pow( 10., fExp ) );
        }
        else
            nCurTime = MAX_TIME;

        if( !nCurTime )
            nCurTime = 1;

        if( mnRepaintTime <= nCurTime )
            mnTimeout = nCurTime - mnRepaintTime;
        else
        {
            sal_uInt64 nMult = mnRepaintTime / nCurTime;

            if( !( mnRepaintTime % nCurTime ) )
                mnTimeout = 0;
            else
                mnTimeout = ++nMult * nCurTime - mnRepaintTime;

            double fValX = static_cast<double>(mnActDeltaX) * nMult;
            double fValY = static_cast<double>(mnActDeltaY) * nMult;

            if( fValX > LONG_MAX )
                mnActDeltaX = LONG_MAX;
            else if( fValX < LONG_MIN )
                mnActDeltaX = LONG_MIN;
            else
                mnActDeltaX = static_cast<long>(fValX);

            if( fValY > LONG_MAX )
                mnActDeltaY = LONG_MAX;
            else if( fValY < LONG_MIN )
                mnActDeltaY = LONG_MIN;
            else
                mnActDeltaY = static_cast<long>(fValY);
        }
    }
}

PointerStyle ImplWheelWindow::ImplGetMousePointer( long nDistX, long nDistY )
{
    PointerStyle    eStyle;
    const StartAutoScrollFlags nFlags = ImplGetSVData()->maWinData.mnAutoScrollFlags;
    const bool      bHorz( nFlags & StartAutoScrollFlags::Horz );
    const bool      bVert( nFlags & StartAutoScrollFlags::Vert );

    if( bHorz || bVert )
    {
        if( mnActDist < WHEEL_RADIUS )
        {
            if( bHorz && bVert )
                eStyle = PointerStyle::AutoScrollNSWE;
            else if( bHorz )
                eStyle = PointerStyle::AutoScrollWE;
            else
                eStyle = PointerStyle::AutoScrollNS;
        }
        else
        {
            double fAngle = basegfx::rad2deg(atan2(static_cast<double>(-nDistY), nDistX));

            if( fAngle < 0.0 )
                fAngle += 360.;

            if( bHorz && bVert )
            {
                if( fAngle >= 22.5 && fAngle <= 67.5 )
                    eStyle = PointerStyle::AutoScrollNE;
                else if( fAngle >= 67.5 && fAngle <= 112.5 )
                    eStyle = PointerStyle::AutoScrollN;
                else if( fAngle >= 112.5 && fAngle <= 157.5 )
                    eStyle = PointerStyle::AutoScrollNW;
                else if( fAngle >= 157.5 && fAngle <= 202.5 )
                    eStyle = PointerStyle::AutoScrollW;
                else if( fAngle >= 202.5 && fAngle <= 247.5 )
                    eStyle = PointerStyle::AutoScrollSW;
                else if( fAngle >= 247.5 && fAngle <= 292.5 )
                    eStyle = PointerStyle::AutoScrollS;
                else if( fAngle >= 292.5 && fAngle <= 337.5 )
                    eStyle = PointerStyle::AutoScrollSE;
                else
                    eStyle = PointerStyle::AutoScrollE;
            }
            else if( bHorz )
            {
                if( fAngle >= 270. || fAngle <= 90. )
                    eStyle = PointerStyle::AutoScrollE;
                else
                    eStyle = PointerStyle::AutoScrollW;
            }
            else
            {
                if( fAngle >= 0. && fAngle <= 180. )
                    eStyle = PointerStyle::AutoScrollN;
                else
                    eStyle = PointerStyle::AutoScrollS;
            }
        }
    }
    else
        eStyle = PointerStyle::Arrow;

    return eStyle;
}

void ImplWheelWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ImplDrawWheel(rRenderContext);
}

void ImplWheelWindow::MouseMove( const MouseEvent& rMEvt )
{
    FloatingWindow::MouseMove( rMEvt );

    const Point aMousePos( OutputToScreenPixel( rMEvt.GetPosPixel() ) );
    const long  nDistX = aMousePos.X() - maCenter.X();
    const long  nDistY = aMousePos.Y() - maCenter.Y();

    mnActDist = static_cast<sal_uLong>(hypot( static_cast<double>(nDistX), nDistY ));

    const PointerStyle  eActStyle = ImplGetMousePointer( nDistX, nDistY );
    const StartAutoScrollFlags nFlags = ImplGetSVData()->maWinData.mnAutoScrollFlags;
    const bool          bHorz( nFlags & StartAutoScrollFlags::Horz );
    const bool          bVert( nFlags & StartAutoScrollFlags::Vert );
    const bool          bOuter = mnActDist > WHEEL_RADIUS;

    if( bOuter && ( maLastMousePos != aMousePos ) )
    {
        switch( eActStyle )
        {
            case PointerStyle::AutoScrollN:   mnActDeltaX = +0; mnActDeltaY = +1; break;
            case PointerStyle::AutoScrollS:   mnActDeltaX = +0; mnActDeltaY = -1; break;
            case PointerStyle::AutoScrollW:   mnActDeltaX = +1; mnActDeltaY = +0; break;
            case PointerStyle::AutoScrollE:   mnActDeltaX = -1; mnActDeltaY = +0; break;
            case PointerStyle::AutoScrollNW:  mnActDeltaX = +1; mnActDeltaY = +1; break;
            case PointerStyle::AutoScrollNE:  mnActDeltaX = -1; mnActDeltaY = +1; break;
            case PointerStyle::AutoScrollSW:  mnActDeltaX = +1; mnActDeltaY = -1; break;
            case PointerStyle::AutoScrollSE:  mnActDeltaX = -1; mnActDeltaY = -1; break;

            default:
            break;
        }
    }

    ImplRecalcScrollValues();
    maLastMousePos = aMousePos;
    SetPointer( eActStyle );

    if( bHorz && bVert )
        ImplSetWheelMode( bOuter ? WheelMode::ScrollVH : WheelMode::VH );
    else if( bHorz )
        ImplSetWheelMode( bOuter ? WheelMode::ScrollH : WheelMode::H );
    else
        ImplSetWheelMode( bOuter ? WheelMode::ScrollV : WheelMode::V );
}

void ImplWheelWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( mnActDist > WHEEL_RADIUS )
        GetParent()->EndAutoScroll();
    else
        FloatingWindow::MouseButtonUp( rMEvt );
}

IMPL_LINK_NOARG(ImplWheelWindow, ImplScrollHdl, Timer *, void)
{
    if ( mnActDeltaX || mnActDeltaY )
    {
        vcl::Window*             pWindow = GetParent();
        const Point         aMousePos( pWindow->OutputToScreenPixel( pWindow->GetPointerPosPixel() ) );
        Point               aCmdMousePos( pWindow->ImplFrameToOutput( aMousePos ) );
        CommandScrollData   aScrollData( mnActDeltaX, mnActDeltaY );
        CommandEvent        aCEvt( aCmdMousePos, CommandEventId::AutoScroll, true, &aScrollData );
        NotifyEvent         aNCmdEvt( MouseNotifyEvent::COMMAND, pWindow, &aCEvt );

        if ( !ImplCallPreNotify( aNCmdEvt ) )
        {
            const sal_uInt64 nTime = tools::Time::GetSystemTicks();
            VclPtr<ImplWheelWindow> xWin(this);
            pWindow->Command( aCEvt );
            if( xWin->IsDisposed() )
                return;
            mnRepaintTime = std::max( tools::Time::GetSystemTicks() - nTime, sal_uInt64(1) );
            ImplRecalcScrollValues();
        }
    }

    if ( mnTimeout != mpTimer->GetTimeout() )
        mpTimer->SetTimeout( mnTimeout );
    mpTimer->Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
