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
#include <tools/mapunit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/cursor.hxx>

#include <window.h>

#include <tools/poly.hxx>

struct ImplCursorData
{
    AutoTimer       maTimer { "vcl ImplCursorData maTimer" };            // Timer
    Point           maPixPos;           // Pixel-Position
    Point           maPixRotOff;        // Pixel-Offset-Position
    Size            maPixSize;          // Pixel-Size
    Degree10        mnOrientation;      // Pixel-Orientation
    CursorDirection mnDirection = CursorDirection::NONE; // indicates writing direction
    sal_uInt16      mnStyle = 0;        // Cursor-Style
    bool            mbCurVisible = false; // Is cursor currently visible
    VclPtr<vcl::Window> mpWindow;           // assigned window
};

namespace
{
const char* pDisableCursorIndicator(getenv("SAL_DISABLE_CURSOR_INDICATOR"));
bool bDisableCursorIndicator(nullptr != pDisableCursorIndicator);

// Build the cursor shape polygon accounting for direction indicators
// and orientation, or return empty polygon for simple rectangular cursors
tools::Polygon ImplCursorPoly(ImplCursorData const* pData)
{
    tools::Rectangle aRect(pData->maPixPos, pData->maPixSize);
    if (pData->mnDirection == CursorDirection::NONE && !pData->mnOrientation)
        return {};

    tools::Polygon aPoly(aRect);
    if (aPoly.GetSize() != 5)
        return {};

    aPoly[1].AdjustX(1);  // include the right border
    aPoly[2].AdjustX(1);

    // apply direction flag after slant to use the correct shape
    if (!bDisableCursorIndicator && pData->mnDirection != CursorDirection::NONE)
    {
        Point pAry[7];
        // Related system settings for "delta" could be:
        // gtk cursor-aspect-ratio and  windows SPI_GETCARETWIDTH
        int delta = (aRect.getOpenHeight() * 4 / 100) + 1;
        if (pData->mnDirection == CursorDirection::LTR)
        {
            // left-to-right
            pAry[0] = aPoly.GetPoint(0);
            pAry[1] = aPoly.GetPoint(1);
            pAry[2] = pAry[1];
            pAry[2].AdjustX(delta);
            pAry[2].AdjustY(delta);
            pAry[3] = pAry[1];
            pAry[3].AdjustY(delta * 2);
            pAry[4] = aPoly.GetPoint(2);
            pAry[5] = aPoly.GetPoint(3);
            pAry[6] = aPoly.GetPoint(4);
        }
        else if (pData->mnDirection == CursorDirection::RTL)
        {
            // right-to-left
            pAry[0] = aPoly.GetPoint(0);
            pAry[1] = aPoly.GetPoint(1);
            pAry[2] = aPoly.GetPoint(2);
            pAry[3] = aPoly.GetPoint(3);
            pAry[4] = pAry[0];
            pAry[4].AdjustY(delta * 2);
            pAry[5] = pAry[0];
            pAry[5].AdjustX(-delta);
            pAry[5].AdjustY(delta);
            pAry[6] = aPoly.GetPoint(4);
        }
        aPoly = tools::Polygon(7, pAry);
    }

    if (pData->mnOrientation)
        aPoly.Rotate(pData->maPixRotOff, pData->mnOrientation);
    return aPoly;
}

// Calculate the pixel bounding rect of the cursor
tools::Rectangle ImplCursorBoundRect(ImplCursorData const* pData)
{
    tools::Polygon aPoly = ImplCursorPoly(pData);
    if (aPoly.GetSize())
        return aPoly.GetBoundRect();
    return tools::Rectangle(pData->maPixPos, pData->maPixSize);
}
}

static tools::Rectangle ImplCursorInvert(vcl::RenderContext* pRenderContext, ImplCursorData const * pData)
{
    bool bMapMode = pRenderContext->IsMapModeEnabled();
    pRenderContext->EnableMapMode( false );
    InvertFlags nInvertStyle;
    if ( pData->mnStyle & CURSOR_SHADOW )
        nInvertStyle = InvertFlags::N50;
    else
        nInvertStyle = InvertFlags::NONE;

    tools::Rectangle aRect;
    tools::Polygon aPoly = ImplCursorPoly(pData);
    if (aPoly.GetSize())
    {
        pRenderContext->Invert(aPoly, nInvertStyle);
        aRect = aPoly.GetBoundRect();
    }
    else
    {
        aRect = tools::Rectangle(pData->maPixPos, pData->maPixSize);
        pRenderContext->Invert(aRect, nInvertStyle);
    }

    pRenderContext->EnableMapMode(bMapMode);
    return aRect;
}

static void ImplCursorInvert(vcl::Window* pWindow, ImplCursorData const * pData)
{
    if (!pWindow || pWindow->isDisposed())
        return;

    vcl::PaintBufferGuardPtr pGuard;
    const bool bDoubleBuffering = pWindow->SupportsDoubleBuffering();
    if (bDoubleBuffering)
        pGuard.reset(new vcl::PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));

    vcl::RenderContext* pRenderContext = bDoubleBuffering ? pGuard->GetRenderContext() : pWindow->GetOutDev();

    tools::Rectangle aPaintRect = ImplCursorInvert(pRenderContext, pData);
    if (bDoubleBuffering)
        pGuard->SetPaintRect(pRenderContext->PixelToLogic(aPaintRect));
}

bool vcl::Cursor::ImplPrepForDraw(const OutputDevice* pDevice, ImplCursorData& rData) const
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
        if (ImplPrepForDraw(mpData->mpWindow->GetOutDev(), *mpData))
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
    // calculate output area
    if (ImplPrepForDraw(&rRenderContext, aData))
    {
        // display
        ImplCursorInvert(&rRenderContext, &aData);
    }
}

tools::Rectangle vcl::Cursor::GetBoundRect(OutputDevice const& rRenderContext) const
{
    ImplCursorData aData;
    if (ImplPrepForDraw(&rRenderContext, aData))
        return ImplCursorBoundRect(&aData);
    return {};
}

void vcl::Cursor::ImplRestore()
{
    assert( mpData && mpData->mbCurVisible );

    ImplCursorInvert(mpData->mpWindow, mpData.get());
    mpData->mbCurVisible = false;
}

void vcl::Cursor::ImplDoShow( bool bDrawDirect, bool bRestore )
{
    if ( !mbVisible )
        return;

    vcl::Window* pWindow;
    if ( mpWindow )
        pWindow = mpWindow;
    else
    {
        // show the cursor, if there is an active window and the cursor
        // has been selected in this window
        pWindow = Application::GetFocusWindow();
        if (!pWindow || !pWindow->mpWindowImpl || (pWindow->mpWindowImpl->mpCursor != this)
            || pWindow->mpWindowImpl->mbInPaint
            || !pWindow->mpWindowImpl->mpFrameData->mbHasFocus)
            pWindow = nullptr;
    }

    if ( !pWindow )
        return;

    if ( !mpData )
    {
        mpData.reset( new ImplCursorData );
        mpData->maTimer.SetInvokeHandler( LINK( this, Cursor, ImplTimerHdl ) );
    }

    mpData->mpWindow    = pWindow;
    mpData->mnStyle     = mnStyle;
    if ( bDrawDirect || bRestore )
        ImplDraw();

    if ( !mpWindow && (bDrawDirect || !mpData->maTimer.IsActive()) )
    {
        mpData->maTimer.SetTimeout( pWindow->GetSettings().GetStyleSettings().GetCursorBlinkTime() );
        if ( mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
            mpData->maTimer.Start();
        else if ( !mpData->mbCurVisible )
            ImplDraw();
        LOKNotify( pWindow, u"cursor_invalidate"_ustr );
        LOKNotify( pWindow, u"cursor_visible"_ustr );
    }
}

namespace
{

tools::Rectangle calcualteCursorRect(Point const& rPosition, Size const rSize, vcl::Window* pWindow, vcl::Window* pParent)
{
    Point aPositionPixel = pWindow->LogicToPixel(rPosition);
    const tools::Long nX = pWindow->GetOutOffXPixel() + aPositionPixel.X() - pParent->GetOutOffXPixel();
    const tools::Long nY = pWindow->GetOutOffYPixel() + aPositionPixel.Y() - pParent->GetOutOffYPixel();

    Size aSizePixel = pWindow->LogicToPixel(rSize);
    if (!aSizePixel.Width())
        aSizePixel.setWidth( pWindow->GetSettings().GetStyleSettings().GetCursorSize() );

    Point aPosition(nX, nY);

    if (pWindow->IsRTLEnabled() && pWindow->GetOutDev() && pParent->GetOutDev()
        && !pWindow->GetOutDev()->ImplIsAntiparallel())
        pParent->GetOutDev()->ReMirror(aPosition);

    if (!pWindow->IsRTLEnabled() && pWindow->GetOutDev() && pParent->GetOutDev()
        && pWindow->GetOutDev()->ImplIsAntiparallel())
    {
        pWindow->GetOutDev()->ReMirror(aPosition);
        pParent->GetOutDev()->ReMirror(aPosition);
    }

    return tools::Rectangle(aPosition, aSizePixel);
}

} // end anonymous namespace

void vcl::Cursor::LOKNotify(vcl::Window* pWindow, const OUString& rAction)
{
    VclPtr<vcl::Window> pParent = pWindow->GetParentWithLOKNotifier();
    if (!pParent)
        return;

    assert(pWindow && "Cannot notify without a window");
    assert(mpData && "Require ImplCursorData");
    assert(comphelper::LibreOfficeKit::isActive());

    const vcl::ILibreOfficeKitNotifier* pNotifier = pParent->GetLOKNotifier();

    if (pWindow->IsFormControl() || (pWindow->GetParent() && pWindow->GetParent()->IsFormControl()))
    {
        if (rAction == "cursor_invalidate")
        {
            tools::Rectangle aRect;
            if (pWindow->IsFormControl())
                aRect = calcualteCursorRect(GetPos(), GetSize(), pWindow, pWindow->GetParent());
            else
                aRect = calcualteCursorRect(GetPos(), GetSize(), pWindow, pWindow->GetParent()->GetParent());

            OutputDevice* pDevice = mpData->mpWindow->GetOutDev();
            const tools::Rectangle aRectTwip = pDevice->PixelToLogic(aRect, MapMode(MapUnit::MapTwip));
            pNotifier->notifyCursorInvalidation(&aRectTwip, true);
        }
    }
    else
    {
        if (comphelper::LibreOfficeKit::isDialogPainting())
            return;

        std::vector<vcl::LOKPayloadItem> aItems;
        if (rAction == "cursor_visible")
        {
            aItems.emplace_back("visible", mpData->mbCurVisible ? "true" : "false");
        }
        else if (rAction == "cursor_invalidate")
        {
            const tools::Rectangle aRect = calcualteCursorRect(GetPos(), GetSize(), pWindow, pParent);
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
            LOKNotify( mpData->mpWindow, u"cursor_visible"_ustr );
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
    if ( !(mbVisible && mpData && mpData->mpWindow) )
        return;

    if ( mpData->mbCurVisible )
        ImplRestore();

    ImplDraw();
    if ( !mpWindow )
    {
        LOKNotify( mpData->mpWindow, u"cursor_invalidate"_ustr );
        if ( mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
            mpData->maTimer.Start();
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
    mnOrientation   = 0_deg10;
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

void vcl::Cursor::SetWidth( tools::Long nNewWidth )
{
    if ( maSize.Width() != nNewWidth )
    {
        maSize.setWidth( nNewWidth );
        ImplNew();
    }
}

void vcl::Cursor::SetOrientation( Degree10 nNewOrientation )
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
