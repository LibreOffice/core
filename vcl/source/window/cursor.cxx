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

#include <memory>

#include <comphelper/lok.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/cursor.hxx>

#include <window.h>

#include <tools/poly.hxx>

struct ImplCursorData
{
    AutoTimer       maTimer;            // Timer
    Point           maPixPos;           // Pixel-Position
    Point           maPixRotOff;        // Pixel-Offset-Position
    Size            maPixSize;          // Pixel-Size
    short           mnOrientation;      // Pixel-Orientation
    CursorDirection mnDirection;        // indicates writing direction
    sal_uInt16      mnStyle;            // Cursor-Style
    bool            mbCurVisible;       // Is cursor currently visible
    VclPtr<vcl::Window> mpWindow;           // assigned window
};

static tools::Rectangle ImplCursorInvert(vcl::RenderContext* pRenderContext, ImplCursorData const * pData)
{
    tools::Rectangle aPaintRect;
    bool    bMapMode = pRenderContext->IsMapModeEnabled();
    pRenderContext->EnableMapMode( false );
    InvertFlags nInvertStyle;
    if ( pData->mnStyle & CURSOR_SHADOW )
        nInvertStyle = InvertFlags::N50;
    else
        nInvertStyle = InvertFlags::NONE;

    tools::Rectangle aRect( pData->maPixPos, pData->maPixSize );
    if ( pData->mnDirection != CursorDirection::NONE || pData->mnOrientation )
    {
        tools::Polygon aPoly( aRect );
        if( aPoly.GetSize() == 5 )
        {
            aPoly[1].AdjustX(1 );  // include the right border
            aPoly[2].AdjustX(1 );

            // apply direction flag after slant to use the correct shape
            if ( pData->mnDirection != CursorDirection::NONE)
            {
                Point pAry[7];
                int delta = 3*aRect.getWidth()+1;
                if( pData->mnDirection == CursorDirection::LTR )
                {
                    // left-to-right
                    pAry[0] = aPoly.GetPoint( 0 );
                    pAry[1] = aPoly.GetPoint( 1 );
                    pAry[2] = pAry[1];
                    pAry[2].AdjustX(delta );
                    pAry[3] =  pAry[1];
                    pAry[3].AdjustY(delta );
                    pAry[4] = aPoly.GetPoint( 2 );
                    pAry[5] = aPoly.GetPoint( 3 );
                    pAry[6] = aPoly.GetPoint( 4 );
                }
                else if( pData->mnDirection == CursorDirection::RTL )
                {
                    // right-to-left
                    pAry[0] = aPoly.GetPoint( 0 );
                    pAry[1] = aPoly.GetPoint( 1 );
                    pAry[2] = aPoly.GetPoint( 2 );
                    pAry[3] = aPoly.GetPoint( 3 );
                    pAry[4] = pAry[0];
                    pAry[4].AdjustY(delta );
                    pAry[5] =  pAry[0];
                    pAry[5].AdjustX( -delta );
                    pAry[6] = aPoly.GetPoint( 4 );
                }
                aPoly = tools::Polygon( 7, pAry);
            }

            if ( pData->mnOrientation )
                aPoly.Rotate( pData->maPixRotOff, pData->mnOrientation );
            pRenderContext->Invert( aPoly, nInvertStyle );
            aPaintRect = aPoly.GetBoundRect();
        }
    }
    else
    {
        pRenderContext->Invert( aRect, nInvertStyle );
        aPaintRect = aRect;
    }
    pRenderContext->EnableMapMode( bMapMode );
    return aPaintRect;
}

static void ImplCursorInvert(vcl::Window* pWindow, ImplCursorData const * pData)
{
    std::unique_ptr<vcl::PaintBufferGuard> pGuard;
    const bool bDoubleBuffering = pWindow->SupportsDoubleBuffering();
    if (bDoubleBuffering)
        pGuard.reset(new vcl::PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));

    vcl::RenderContext* pRenderContext = bDoubleBuffering ? pGuard->GetRenderContext() : pWindow;

    tools::Rectangle aPaintRect = ImplCursorInvert(pRenderContext, pData);
    if (bDoubleBuffering)
        pGuard->SetPaintRect(pRenderContext->PixelToLogic(aPaintRect));
}

bool vcl::Cursor::ImplPrepForDraw(const OutputDevice* pDevice, ImplCursorData& rData)
{
    if (pDevice && !rData.mbCurVisible)
    {
        rData.maPixPos        = pDevice->LogicToPixel( maPos );
        rData.maPixSize       = pDevice->LogicToPixel( maSize );
        rData.mnOrientation   = mnOrientation;
        rData.mnDirection     = mnDirection;

        // correct the position with the offset
        rData.maPixRotOff = rData.maPixPos;

        // use width (as set in Settings) if size is 0,
        if (!rData.maPixSize.Width())
            rData.maPixSize.setWidth(pDevice->GetSettings().GetStyleSettings().GetCursorSize());
        return true;
    }
    return false;
}

void vcl::Cursor::ImplDraw()
{
    if (mpData && mpData->mpWindow)
    {
        // calculate output area
        if (ImplPrepForDraw(mpData->mpWindow, *mpData))
        {
            // display
            ImplCursorInvert(mpData->mpWindow, mpData.get());
            mpData->mbCurVisible = true;
        }
    }
}

void vcl::Cursor::DrawToDevice(OutputDevice& rRenderContext)
{
    ImplCursorData aData;
    aData.mnStyle = 0;
    aData.mbCurVisible = false;
    // calculate output area
    if (ImplPrepForDraw(&rRenderContext, aData))
    {
        // display
        ImplCursorInvert(&rRenderContext, &aData);
    }
}

void vcl::Cursor::ImplRestore()
{
    assert( mpData && mpData->mbCurVisible );

    ImplCursorInvert(mpData->mpWindow, mpData.get());
    mpData->mbCurVisible = false;
}

void vcl::Cursor::ImplDoShow( bool bDrawDirect, bool bRestore )
{
    if ( mbVisible )
    {
        vcl::Window* pWindow;
        if ( mpWindow )
            pWindow = mpWindow;
        else
        {
            // show the cursor, if there is an active window and the cursor
            // has been selected in this window
            pWindow = Application::GetFocusWindow();
            if ( !pWindow || (pWindow->mpWindowImpl->mpCursor != this) || pWindow->mpWindowImpl->mbInPaint
                || !pWindow->mpWindowImpl->mpFrameData->mbHasFocus )
                pWindow = nullptr;
        }

        if ( pWindow )
        {
            if ( !mpData )
            {
                mpData.reset( new ImplCursorData );
                mpData->mbCurVisible = false;
                mpData->maTimer.SetInvokeHandler( LINK( this, Cursor, ImplTimerHdl ) );
                mpData->maTimer.SetDebugName( "vcl ImplCursorData maTimer" );
            }

            mpData->mpWindow    = pWindow;
            mpData->mnStyle     = mnStyle;
            if ( bDrawDirect || bRestore )
                ImplDraw();

            if ( !mpWindow && ! ( ! bDrawDirect && mpData->maTimer.IsActive()) )
            {
                mpData->maTimer.SetTimeout( pWindow->GetSettings().GetStyleSettings().GetCursorBlinkTime() );
                if ( mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
                    mpData->maTimer.Start();
                else if ( !mpData->mbCurVisible )
                    ImplDraw();
                LOKNotify( pWindow, "cursor_invalidate" );
                LOKNotify( pWindow, "cursor_visible" );
            }
        }
    }
}

void vcl::Cursor::LOKNotify( vcl::Window* pWindow, const OUString& rAction )
{
    if (VclPtr<vcl::Window> pParent = pWindow->GetParentWithLOKNotifier())
    {
        assert(pWindow && "Cannot notify without a window");
        assert(mpData && "Require ImplCursorData");
        assert(comphelper::LibreOfficeKit::isActive());

        if (comphelper::LibreOfficeKit::isDialogPainting())
            return;

        const vcl::ILibreOfficeKitNotifier* pNotifier = pParent->GetLOKNotifier();
        std::vector<vcl::LOKPayloadItem> aItems;
        if (rAction == "cursor_visible")
            aItems.emplace_back("visible", mpData->mbCurVisible ? "true" : "false");
        else if (rAction == "cursor_invalidate")
        {
            const long nX = pWindow->GetOutOffXPixel() + pWindow->LogicToPixel(GetPos()).X() - pParent->GetOutOffXPixel();
            const long nY = pWindow->GetOutOffYPixel() + pWindow->LogicToPixel(GetPos()).Y() - pParent->GetOutOffYPixel();
            Size aSize = pWindow->LogicToPixel(GetSize());
            if (!aSize.Width())
                aSize.setWidth( pWindow->GetSettings().GetStyleSettings().GetCursorSize() );

            const tools::Rectangle aRect(Point(nX, nY), aSize);
            aItems.emplace_back("rectangle", aRect.toString());
        }

        pNotifier->notifyWindow(pParent->GetLOKWindowId(), rAction, aItems);
    }
}

bool vcl::Cursor::ImplDoHide( bool bSuspend )
{
    bool bWasCurVisible = false;
    if ( mpData && mpData->mpWindow )
    {
        bWasCurVisible = mpData->mbCurVisible;
        if ( mpData->mbCurVisible )
            ImplRestore();

        if ( !bSuspend )
        {
            LOKNotify( mpData->mpWindow, "cursor_visible" );
            mpData->maTimer.Stop();
            mpData->mpWindow = nullptr;
        }
    }
    return bWasCurVisible;
}

void vcl::Cursor::ImplShow()
{
    ImplDoShow( true/*bDrawDirect*/, false );
}

void vcl::Cursor::ImplHide()
{
    ImplDoHide( false );
}

void vcl::Cursor::ImplResume( bool bRestore )
{
    ImplDoShow( false, bRestore );
}

bool vcl::Cursor::ImplSuspend()
{
    return ImplDoHide( true );
}

void vcl::Cursor::ImplNew()
{
    if ( mbVisible && mpData && mpData->mpWindow )
    {
        if ( mpData->mbCurVisible )
            ImplRestore();

        ImplDraw();
        if ( !mpWindow )
        {
            LOKNotify( mpData->mpWindow, "cursor_invalidate" );
            if ( mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
                mpData->maTimer.Start();
        }
    }
}

IMPL_LINK_NOARG(vcl::Cursor, ImplTimerHdl, Timer *, void)
{
    if ( mpData->mbCurVisible )
        ImplRestore();
    else
        ImplDraw();
}

vcl::Cursor::Cursor()
{
    mpData          = nullptr;
    mpWindow        = nullptr;
    mnOrientation   = 0;
    mnDirection     = CursorDirection::NONE;
    mnStyle         = 0;
    mbVisible       = false;
}

vcl::Cursor::Cursor( const Cursor& rCursor ) :
    maSize( rCursor.maSize ),
    maPos( rCursor.maPos )
{
    mpData          = nullptr;
    mpWindow        = nullptr;
    mnOrientation   = rCursor.mnOrientation;
    mnDirection     = rCursor.mnDirection;
    mnStyle         = 0;
    mbVisible       = rCursor.mbVisible;
}

vcl::Cursor::~Cursor()
{
    if (mpData && mpData->mbCurVisible)
        ImplRestore();
}

void vcl::Cursor::SetStyle( sal_uInt16 nStyle )
{
    if ( mnStyle != nStyle )
    {
        mnStyle = nStyle;
        ImplNew();
    }
}

void vcl::Cursor::Show()
{
    if ( !mbVisible )
    {
        mbVisible = true;
        ImplShow();
    }
}

void vcl::Cursor::Hide()
{
    if ( mbVisible )
    {
        mbVisible = false;
        ImplHide();
    }
}

void vcl::Cursor::SetWindow( vcl::Window* pWindow )
{
    if ( mpWindow.get() != pWindow )
    {
        mpWindow = pWindow;
        ImplNew();
    }
}

void vcl::Cursor::SetPos( const Point& rPoint )
{
    if ( maPos != rPoint )
    {
        maPos = rPoint;
        ImplNew();
    }
}

void vcl::Cursor::SetSize( const Size& rSize )
{
    if ( maSize != rSize )
    {
        maSize = rSize;
        ImplNew();
    }
}

void vcl::Cursor::SetWidth( long nNewWidth )
{
    if ( maSize.Width() != nNewWidth )
    {
        maSize.setWidth( nNewWidth );
        ImplNew();
    }
}

void vcl::Cursor::SetOrientation( short nNewOrientation )
{
    if ( mnOrientation != nNewOrientation )
    {
        mnOrientation = nNewOrientation;
        ImplNew();
    }
}

void vcl::Cursor::SetDirection( CursorDirection nNewDirection )
{
    if ( mnDirection != nNewDirection )
    {
        mnDirection = nNewDirection;
        ImplNew();
    }
}

vcl::Cursor& vcl::Cursor::operator=( const vcl::Cursor& rCursor )
{
    maPos           = rCursor.maPos;
    maSize          = rCursor.maSize;
    mnOrientation   = rCursor.mnOrientation;
    mnDirection     = rCursor.mnDirection;
    mbVisible       = rCursor.mbVisible;
    ImplNew();

    return *this;
}

bool vcl::Cursor::operator==( const vcl::Cursor& rCursor ) const
{
    return
        ((maPos         == rCursor.maPos)           &&
         (maSize        == rCursor.maSize)          &&
         (mnOrientation == rCursor.mnOrientation)   &&
         (mnDirection   == rCursor.mnDirection)     &&
         (mbVisible     == rCursor.mbVisible))
        ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
