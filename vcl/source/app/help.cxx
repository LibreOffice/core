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

#include <comphelper/string.hxx>

#include "tools/debug.hxx"
#include "tools/diagnose_ex.h"
#include "tools/time.hxx"

#include <vcl/window.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>

#include "helpwin.hxx"
#include "salframe.hxx"
#include "svdata.hxx"

#define HELPWINSTYLE_QUICK      0
#define HELPWINSTYLE_BALLOON    1

#define HELPTEXTMARGIN_QUICK    3
#define HELPTEXTMARGIN_BALLOON  6

#define HELPDELAY_NORMAL        1
#define HELPDELAY_SHORT         2
#define HELPDELAY_NONE          3

#define HELPTEXTMAXLEN        150

Help::Help()
{
}

Help::~Help()
{
}

bool Help::Start( const OUString&, const vcl::Window* )
{
    return false;
}

bool Help::SearchKeyword( const OUString& )
{
    return false;
}

OUString Help::GetHelpText( const OUString&, const vcl::Window* )
{
    return OUString();
}

void Help::EnableContextHelp()
{
    ImplGetSVData()->maHelpData.mbContextHelp = true;
}

void Help::DisableContextHelp()
{
    ImplGetSVData()->maHelpData.mbContextHelp = false;
}

bool Help::IsContextHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbContextHelp;
}

void Help::EnableExtHelp()
{
    ImplGetSVData()->maHelpData.mbExtHelp = true;
}

void Help::DisableExtHelp()
{
    ImplGetSVData()->maHelpData.mbExtHelp = false;
}

bool Help::IsExtHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbExtHelp;
}

bool Help::StartExtHelp()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mbExtHelp && !pSVData->maHelpData.mbExtHelpMode )
    {
        pSVData->maHelpData.mbExtHelpMode = true;
        pSVData->maHelpData.mbOldBalloonMode = pSVData->maHelpData.mbBalloonHelp;
        pSVData->maHelpData.mbBalloonHelp = true;
        if ( pSVData->maWinData.mpAppWin )
            pSVData->maWinData.mpAppWin->ImplGenerateMouseMove();
        return true;
    }

    return false;
}

bool Help::EndExtHelp()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mbExtHelp && pSVData->maHelpData.mbExtHelpMode )
    {
        pSVData->maHelpData.mbExtHelpMode = false;
        pSVData->maHelpData.mbBalloonHelp = pSVData->maHelpData.mbOldBalloonMode;
        if ( pSVData->maWinData.mpAppWin )
            pSVData->maWinData.mpAppWin->ImplGenerateMouseMove();
        return true;
    }

    return false;
}

void Help::EnableBalloonHelp()
{
    ImplGetSVData()->maHelpData.mbBalloonHelp = true;
}

void Help::DisableBalloonHelp()
{
    ImplGetSVData()->maHelpData.mbBalloonHelp = false;
}

bool Help::IsBalloonHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbBalloonHelp;
}

bool Help::ShowBalloon( vcl::Window* pParent,
                        const Point& rScreenPos, const Rectangle& rRect,
                        const OUString& rHelpText )
{
    ImplShowHelpWindow( pParent, HELPWINSTYLE_BALLOON, QuickHelpFlags::NONE,
                        rHelpText, OUString(), rScreenPos, rRect );

    return true;
}

void Help::EnableQuickHelp()
{
    ImplGetSVData()->maHelpData.mbQuickHelp = true;
}

void Help::DisableQuickHelp()
{
    ImplGetSVData()->maHelpData.mbQuickHelp = false;
}

bool Help::IsQuickHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbQuickHelp;
}

bool Help::ShowQuickHelp( vcl::Window* pParent,
                          const Rectangle& rScreenRect,
                          const OUString& rHelpText,
                          const OUString& rLongHelpText,
                          QuickHelpFlags nStyle )
{
    sal_uInt16 nHelpWinStyle = ( nStyle & QuickHelpFlags::TipStyleBalloon ) ? HELPWINSTYLE_BALLOON : HELPWINSTYLE_QUICK;
    ImplShowHelpWindow( pParent, nHelpWinStyle, nStyle,
                        rHelpText, rLongHelpText,
                        pParent->OutputToScreenPixel( pParent->GetPointerPosPixel() ), rScreenRect );
    return true;
}

void Help::HideBalloonAndQuickHelp()
{
    HelpTextWindow const * pHelpWin = ImplGetSVData()->maHelpData.mpHelpWin;
    bool const bIsVisible = ( pHelpWin != nullptr ) && pHelpWin->IsVisible();
    ImplDestroyHelpWindow( bIsVisible );
}

sal_uIntPtr Help::ShowPopover(vcl::Window* pParent, const Rectangle& rScreenRect,
                              const OUString& rText, QuickHelpFlags nStyle)
{
    sal_uIntPtr nId = pParent->ImplGetFrame()->ShowPopover(rText, rScreenRect, nStyle);
    if (nId)
    {
        //popovers are handled natively, return early
        return nId;
    }

    sal_uInt16 nHelpWinStyle = ( nStyle & QuickHelpFlags::TipStyleBalloon ) ? HELPWINSTYLE_BALLOON : HELPWINSTYLE_QUICK;
    VclPtrInstance<HelpTextWindow> pHelpWin( pParent, rText, nHelpWinStyle, nStyle );

    nId = reinterpret_cast< sal_uIntPtr >( pHelpWin.get() );
    UpdatePopover(nId, pParent, rScreenRect, rText);

    pHelpWin->ShowHelp( HELPDELAY_NONE );
    return nId;
}

void Help::UpdatePopover(sal_uIntPtr nId, vcl::Window* pParent, const Rectangle& rScreenRect,
                         const OUString& rText)
{
    if (pParent->ImplGetFrame()->UpdatePopover(nId, rText, rScreenRect))
    {
        //popovers are handled natively, return early
        return;
    }

    HelpTextWindow* pHelpWin = reinterpret_cast< HelpTextWindow* >( nId );
    ENSURE_OR_RETURN_VOID( pHelpWin != nullptr, "Help::UpdatePopover: invalid ID!" );

    Size aSz = pHelpWin->CalcOutSize();
    pHelpWin->SetOutputSizePixel( aSz );
    ImplSetHelpWindowPos( pHelpWin, pHelpWin->GetWinStyle(), pHelpWin->GetStyle(),
        pParent->OutputToScreenPixel( pParent->GetPointerPosPixel() ), rScreenRect );

    pHelpWin->SetHelpText( rText );
    pHelpWin->Invalidate();
}

void Help::HidePopover(vcl::Window* pParent, sal_uLong nId)
{
    if (pParent->ImplGetFrame()->HidePopover(nId))
    {
        //popovers are handled natively, return early
        return;
    }

    VclPtr<HelpTextWindow> pHelpWin = reinterpret_cast<HelpTextWindow*>(nId);
    vcl::Window* pFrameWindow = pHelpWin->ImplGetFrameWindow();
    pHelpWin->Hide();
    // trigger update, so that a Paint is instantly triggered since we do not save the background
    pFrameWindow->ImplUpdateAll();
    pHelpWin.disposeAndClear();
    ImplGetSVData()->maHelpData.mnLastHelpHideTime = tools::Time::GetSystemTicks();
}

HelpTextWindow::HelpTextWindow( vcl::Window* pParent, const OUString& rText, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle ) :
    FloatingWindow( pParent, WB_SYSTEMWINDOW|WB_TOOLTIPWIN ), // #105827# if we change the parent, mirroring will not work correctly when positioning this window
    maHelpText( rText )
{
    SetType( WINDOW_HELPTEXTWINDOW );
    ImplSetMouseTransparent( true );
    mnHelpWinStyle = nHelpWinStyle;
    mnStyle = nStyle;

    if( mnStyle & QuickHelpFlags::BiDiRtl )
    {
        ComplexTextLayoutFlags nLayoutMode = GetLayoutMode();
        nLayoutMode |= ComplexTextLayoutFlags::BiDiRtl | ComplexTextLayoutFlags::TextOriginLeft;
        SetLayoutMode( nLayoutMode );
    }
    SetHelpText( rText );
    Window::SetHelpText( rText );

    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maHelpData.mbSetKeyboardHelp )
        pSVData->maHelpData.mbKeyboardHelp = true;


    maShowTimer.SetTimeoutHdl( LINK( this, HelpTextWindow, TimerHdl ) );
    maShowTimer.SetDebugName( "vcl::HelpTextWindow maShowTimer" );

    const HelpSettings& rHelpSettings = pParent->GetSettings().GetHelpSettings();
    maHideTimer.SetTimeout( rHelpSettings.GetTipTimeout() );
    maHideTimer.SetTimeoutHdl( LINK( this, HelpTextWindow, TimerHdl ) );
    maHideTimer.SetDebugName( "vcl::HelpTextWindow maHideTimer" );
}

void HelpTextWindow::StateChanged(StateChangedType nType)
{
    FloatingWindow::StateChanged(nType);
    if (nType == StateChangedType::InitShow)
    {
        ApplySettings(*this);
        SetHelpText(maHelpText);
        Invalidate();
    }
}

void HelpTextWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    SetPointFont(rRenderContext, rStyleSettings.GetHelpFont());
    rRenderContext.SetTextColor(rStyleSettings.GetHelpTextColor());
    rRenderContext.SetTextAlign(ALIGN_TOP);

    if (rRenderContext.IsNativeControlSupported(ControlType::Tooltip, ControlPart::Entire))
    {
        EnableChildTransparentMode();
        SetParentClipMode(ParentClipMode::NoClip);
        SetPaintTransparent(true);
        rRenderContext.SetBackground();
    }
    else
        rRenderContext.SetBackground(Wallpaper(rStyleSettings.GetHelpColor()));

    if (rStyleSettings.GetHelpColor().IsDark())
        rRenderContext.SetLineColor(COL_WHITE);
    else
        rRenderContext.SetLineColor(COL_BLACK);
    rRenderContext.SetFillColor();
}

HelpTextWindow::~HelpTextWindow()
{
    disposeOnce();
}

void HelpTextWindow::dispose()
{
    maShowTimer.Stop();
    maHideTimer.Stop();

    if( this == ImplGetSVData()->maHelpData.mpHelpWin )
        ImplGetSVData()->maHelpData.mpHelpWin = nullptr;
    FloatingWindow::dispose();
}

void HelpTextWindow::SetHelpText( const OUString& rHelpText )
{
    maHelpText = rHelpText;
    if ( mnHelpWinStyle == HELPWINSTYLE_QUICK && maHelpText.getLength() < HELPTEXTMAXLEN)
    {
        Size aSize;
        aSize.Height() = GetTextHeight();
        if ( mnStyle & QuickHelpFlags::CtrlText )
            aSize.Width() = GetCtrlTextWidth( maHelpText );
        else
            aSize.Width() = GetTextWidth( maHelpText );
        maTextRect = Rectangle( Point( HELPTEXTMARGIN_QUICK, HELPTEXTMARGIN_QUICK ), aSize );
    }
    else // HELPWINSTYLE_BALLOON
    {
        Point       aTmpPoint;
        sal_Int32 nCharsInLine = 35 + ((maHelpText.getLength()/100)*5);
        // average width to have all windows consistent
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, nCharsInLine, 'x');
        OUString aXXX = aBuf.makeStringAndClear();
        long nWidth = GetTextWidth( aXXX );
        Size aTmpSize( nWidth, 0x7FFFFFFF );
        Rectangle aTry1( aTmpPoint, aTmpSize );
        DrawTextFlags nDrawFlags = DrawTextFlags::MultiLine | DrawTextFlags::WordBreak |
                            DrawTextFlags::Left | DrawTextFlags::Top;
        if ( mnStyle & QuickHelpFlags::CtrlText )
            nDrawFlags |= DrawTextFlags::Mnemonic;
        Rectangle aTextRect = GetTextRect( aTry1, maHelpText, nDrawFlags );

        // get a better width later...
        maTextRect = aTextRect;

        // safety distance...
        maTextRect.SetPos( Point( HELPTEXTMARGIN_BALLOON, HELPTEXTMARGIN_BALLOON ) );
    }

    Size aSize( CalcOutSize() );
    SetOutputSizePixel( aSize );
}

void HelpTextWindow::ImplShow()
{
    VclPtr<HelpTextWindow> xWindow( this );
    Show( true, ShowFlags::NoActivate );
    if( !xWindow->IsDisposed() )
    Update();
}

void HelpTextWindow::Paint( vcl::RenderContext& rRenderContext, const Rectangle& )
{
    // paint native background
    bool bNativeOK = false;
    if (rRenderContext.IsNativeControlSupported(ControlType::Tooltip, ControlPart::Entire))
    {
        // #i46472# workaround gcc3.3 temporary problem
        Rectangle aCtrlRegion(Point(0, 0), GetOutputSizePixel());
        ImplControlValue aControlValue;
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::Tooltip, ControlPart::Entire, aCtrlRegion,
                                                     ControlState::NONE, aControlValue, OUString());
    }

    // paint text
    if (mnHelpWinStyle == HELPWINSTYLE_QUICK && maHelpText.getLength() < HELPTEXTMAXLEN)
    {
        if ( mnStyle & QuickHelpFlags::CtrlText )
            rRenderContext.DrawCtrlText(maTextRect.TopLeft(), maHelpText);
        else
            rRenderContext.DrawText(maTextRect.TopLeft(), maHelpText);
    }
    else // HELPWINSTYLE_BALLOON
    {
        DrawTextFlags nDrawFlags = DrawTextFlags::MultiLine|DrawTextFlags::WordBreak|
                                DrawTextFlags::Left|DrawTextFlags::Top;
        if (mnStyle & QuickHelpFlags::CtrlText)
            nDrawFlags |= DrawTextFlags::Mnemonic;
        rRenderContext.DrawText(maTextRect, maHelpText, nDrawFlags);
    }

    // border
    if (!bNativeOK)
    {
        Size aSz = GetOutputSizePixel();
        rRenderContext.DrawRect(Rectangle(Point(), aSz));
        if (mnHelpWinStyle == HELPWINSTYLE_BALLOON)
        {
            aSz.Width() -= 2;
            aSz.Height() -= 2;
            Color aColor(rRenderContext.GetLineColor());
            rRenderContext.SetLineColor(COL_GRAY);
            rRenderContext.DrawRect(Rectangle(Point(1, 1), aSz));
            rRenderContext.SetLineColor(aColor);
        }
    }
}

void HelpTextWindow::ShowHelp( sal_uInt16 nDelayMode )
{
    sal_uLong nTimeout = 0;
    if ( nDelayMode != HELPDELAY_NONE )
    {
        // In case of ExtendedHelp display help sooner
        if ( ImplGetSVData()->maHelpData.mbExtHelpMode )
            nTimeout = 15;
        else
        {
            const HelpSettings& rHelpSettings = GetSettings().GetHelpSettings();
            if ( mnHelpWinStyle == HELPWINSTYLE_QUICK )
                nTimeout = rHelpSettings.GetTipDelay();
            else
                nTimeout = rHelpSettings.GetBalloonDelay();
        }

        if ( nDelayMode == HELPDELAY_SHORT )
            nTimeout /= 3;
    }

    maShowTimer.SetTimeout( nTimeout );
    maShowTimer.Start();
}

IMPL_LINK_TYPED( HelpTextWindow, TimerHdl, Timer*, pTimer, void)
{
    if ( pTimer == &maShowTimer )
    {
        if ( mnHelpWinStyle == HELPWINSTYLE_QUICK )
        {
            // start auto-hide-timer for non-ShowTip windows
            ImplSVData* pSVData = ImplGetSVData();
            if ( this == pSVData->maHelpData.mpHelpWin )
                maHideTimer.Start();
        }
        ImplShow();
    }
    else
    {
        SAL_WARN_IF( pTimer != &maHideTimer, "vcl", "HelpTextWindow::TimerHdl with bad Timer" );
          ImplDestroyHelpWindow( true );
    }
}

Size HelpTextWindow::CalcOutSize() const
{
    Size aSz = maTextRect.GetSize();
    aSz.Width() += 2*maTextRect.Left();
    aSz.Height() += 2*maTextRect.Top();
    return aSz;
}

void HelpTextWindow::RequestHelp( const HelpEvent& /*rHEvt*/ )
{
    // Just to assure that Window::RequestHelp() is not called by
    // ShowQuickHelp/ShowBalloonHelp in the HelpTextWindow.
}

OUString HelpTextWindow::GetText() const
{
    return maHelpText;
}

void ImplShowHelpWindow( vcl::Window* pParent, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle,
                         const OUString& rHelpText, const OUString& rStatusText,
                         const Point& rScreenPos, const Rectangle& rHelpArea )
{
    if (pParent->ImplGetFrame()->ShowTooltip(rHelpText, rHelpArea))
    {
        //tooltips are handled natively, return early
        return;
    }

    ImplSVData* pSVData = ImplGetSVData();

    if (rHelpText.isEmpty() && !pSVData->maHelpData.mbRequestingHelp)
        return;

    HelpTextWindow* pHelpWin = pSVData->maHelpData.mpHelpWin;
    sal_uInt16 nDelayMode = HELPDELAY_NORMAL;
    if ( pHelpWin )
    {
        SAL_WARN_IF( pHelpWin == pParent, "vcl", "HelpInHelp ?!" );

        if  (   (   ( pHelpWin->GetHelpText() != rHelpText )
                ||  ( pHelpWin->GetWinStyle() != nHelpWinStyle )
                ||  ( pHelpWin->GetHelpArea() != rHelpArea )
                )
            &&  pSVData->maHelpData.mbRequestingHelp
            )
        {
            // remove help window if no HelpText or other HelpText or
            // other help mode. but keep it if we are scrolling, ie not requesting help
            bool bWasVisible = pHelpWin->IsVisible();
            if ( bWasVisible )
                nDelayMode = HELPDELAY_NONE; // display it quickly if we were already in quick help mode
            pHelpWin = nullptr;
            ImplDestroyHelpWindow( bWasVisible );
        }
        else
        {
            bool const bTextChanged = rHelpText != pHelpWin->GetHelpText();
            if (bTextChanged)
            {
                vcl::Window * pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
                Rectangle aInvRect( pHelpWin->GetWindowExtentsRelative( pWindow ) );
                if( pHelpWin->IsVisible() )
                    pWindow->Invalidate( aInvRect );

                pHelpWin->SetHelpText( rHelpText );
                // approach mouse position
                ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, rHelpArea );
                if( pHelpWin->IsVisible() )
                    pHelpWin->Invalidate();
            }
        }
    }

    if (!pHelpWin && !rHelpText.isEmpty())
    {
        sal_uInt64 nCurTime = tools::Time::GetSystemTicks();
        if  (   ( ( nCurTime - pSVData->maHelpData.mnLastHelpHideTime ) < pParent->GetSettings().GetHelpSettings().GetTipDelay() )
            ||  ( nStyle & QuickHelpFlags::NoDelay )
            )
            nDelayMode = HELPDELAY_NONE;

        pHelpWin = VclPtr<HelpTextWindow>::Create( pParent, rHelpText, nHelpWinStyle, nStyle );
        pSVData->maHelpData.mpHelpWin = pHelpWin;
        pHelpWin->SetStatusText( rStatusText );
        pHelpWin->SetHelpArea( rHelpArea );

        //  positioning
        Size aSz = pHelpWin->CalcOutSize();
        pHelpWin->SetOutputSizePixel( aSz );
        ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, rHelpArea );
        // if not called from Window::RequestHelp, then without delay...
        if ( !pSVData->maHelpData.mbRequestingHelp )
            nDelayMode = HELPDELAY_NONE;
        pHelpWin->ShowHelp( nDelayMode );
    }
}

void ImplDestroyHelpWindow( bool bUpdateHideTime )
{
    ImplSVData* pSVData = ImplGetSVData();
    VclPtr<HelpTextWindow> pHelpWin = pSVData->maHelpData.mpHelpWin;
    if ( pHelpWin )
    {
        vcl::Window * pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
        // find out screen area covered by system help window
        Rectangle aInvRect( pHelpWin->GetWindowExtentsRelative( pWindow ) );
        if( pHelpWin->IsVisible() )
            pWindow->Invalidate( aInvRect );
        pSVData->maHelpData.mpHelpWin = nullptr;
        pSVData->maHelpData.mbKeyboardHelp = false;
        pHelpWin->Hide();
        pHelpWin.disposeAndClear();
        if( bUpdateHideTime )
            pSVData->maHelpData.mnLastHelpHideTime = tools::Time::GetSystemTicks();
    }
}

void ImplSetHelpWindowPos( vcl::Window* pHelpWin, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle,
                           const Point& rPos, const Rectangle& rHelpArea )
{
    Point       aPos = rPos;
    Size        aSz = pHelpWin->GetSizePixel();
    Rectangle   aScreenRect = pHelpWin->ImplGetFrameWindow()->GetDesktopRectPixel();
    aPos = pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( aPos );
    // get mouse screen coords
    Point aMousePos( pHelpWin->GetParent()->ImplGetFrameWindow()->GetPointerPosPixel() );
    aMousePos = pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( aMousePos );

    if ( nHelpWinStyle == HELPWINSTYLE_QUICK )
    {
        if ( !(nStyle & QuickHelpFlags::NoAutoPos) )
        {
            long nScreenHeight = aScreenRect.GetHeight();
            aPos.X() -= 4;
            if ( aPos.Y() > aScreenRect.Top()+nScreenHeight-(nScreenHeight/4) )
                aPos.Y() -= aSz.Height()+4;
            else
                aPos.Y() += 21;
        }
    }
    else
    {
        // If it's the mouse position, move the window slightly
        // so the mouse pointer does not cover it
        if ( aPos == aMousePos )
        {
            aPos.X() += 12;
            aPos.Y() += 16;
        }
    }

    if ( nStyle & QuickHelpFlags::NoAutoPos )
    {
        // convert help area to screen coords
        Rectangle devHelpArea(
            pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( rHelpArea.TopLeft() ),
            pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( rHelpArea.BottomRight() ) );

        // Welche Position vom Rechteck?
        aPos = devHelpArea.Center();

        if ( nStyle & QuickHelpFlags::Left )
            aPos.X() = devHelpArea.Left();
        else if ( nStyle & QuickHelpFlags::Right )
            aPos.X() = devHelpArea.Right();

        if ( nStyle & QuickHelpFlags::Top )
            aPos.Y() = devHelpArea.Top();
        else if ( nStyle & QuickHelpFlags::Bottom )
            aPos.Y() = devHelpArea.Bottom();

        // which direction?
        if ( nStyle & QuickHelpFlags::Left )
            ;
        else if ( nStyle & QuickHelpFlags::Right )
            aPos.X() -= aSz.Width();
        else
            aPos.X() -= aSz.Width()/2;

        if ( nStyle & QuickHelpFlags::Top )
            ;
        else if ( nStyle & QuickHelpFlags::Bottom )
            aPos.Y() -= aSz.Height();
        else
            aPos.Y() -= aSz.Height()/2;
    }

    if ( aPos.X() < aScreenRect.Left() )
        aPos.X() = aScreenRect.Left();
    else if ( ( aPos.X() + aSz.Width() ) > aScreenRect.Right() )
        aPos.X() = aScreenRect.Right() - aSz.Width();
    if ( aPos.Y() < aScreenRect.Top() )
        aPos.Y() = aScreenRect.Top();
    else if ( ( aPos.Y() + aSz.Height() ) > aScreenRect.Bottom() )
        aPos.Y() = aScreenRect.Bottom() - aSz.Height();

    if( ! (nStyle & QuickHelpFlags::NoEvadePointer) )
    {
        /* the remark below should be obsolete by now as the helpwindow should
        not be focusable, leaving it as a hint. However it is sensible in most
        conditions to evade the mouse pointer so the content window is fully visible.

        // the popup must not appear under the mouse
        // otherwise it would directly be closed due to a focus change...
        */
        Rectangle aHelpRect( aPos, aSz );
        if( aHelpRect.IsInside( aMousePos ) )
        {
            Point delta(2,2);
            Point aSize( aSz.Width(), aSz.Height() );
            Point aTest( aMousePos - aSize - delta );
            if( aTest.X() > aScreenRect.Left() && aTest.Y() > aScreenRect.Top() )
                aPos = aTest;
            else
                aPos = aMousePos + delta;
        }
    }

    vcl::Window* pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
    aPos = pWindow->AbsoluteScreenToOutputPixel( aPos );
    pHelpWin->SetPosPixel( aPos );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
