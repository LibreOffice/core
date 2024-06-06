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
#include <sal/log.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <tools/time.hxx>

#include <vcl/window.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>

#include <helpwin.hxx>
#include <salframe.hxx>
#include <svdata.hxx>

#define HELPWINSTYLE_QUICK      0
#define HELPWINSTYLE_BALLOON    1

#define HELPTEXTMARGIN_QUICK    3
#define HELPTEXTMARGIN_BALLOON  6

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

bool Help::Start(const OUString&, weld::Widget*)
{
    return false;
}

void Help::SearchKeyword( const OUString& )
{
}

OUString Help::GetHelpText( const OUString& )
{
    return OUString();
}

void Help::EnableContextHelp()
{
    ImplGetSVHelpData().mbContextHelp = true;
}

void Help::DisableContextHelp()
{
    ImplGetSVHelpData().mbContextHelp = false;
}

bool Help::IsContextHelpEnabled()
{
    return ImplGetSVHelpData().mbContextHelp;
}

void Help::EnableExtHelp()
{
    ImplGetSVHelpData().mbExtHelp = true;
}

void Help::DisableExtHelp()
{
    ImplGetSVHelpData().mbExtHelp = false;
}

bool Help::IsExtHelpEnabled()
{
    return ImplGetSVHelpData().mbExtHelp;
}

bool Help::StartExtHelp()
{
    ImplSVData* pSVData = ImplGetSVData();
    ImplSVHelpData& aHelpData = ImplGetSVHelpData();

    if ( aHelpData.mbExtHelp && !aHelpData.mbExtHelpMode )
    {
        aHelpData.mbExtHelpMode = true;
        aHelpData.mbOldBalloonMode = aHelpData.mbBalloonHelp;
        aHelpData.mbBalloonHelp = true;
        if (pSVData->maFrameData.mpAppWin)
            pSVData->maFrameData.mpAppWin->ImplGenerateMouseMove();
        return true;
    }

    return false;
}

bool Help::EndExtHelp()
{
    ImplSVData* pSVData = ImplGetSVData();
    ImplSVHelpData& aHelpData = ImplGetSVHelpData();

    if ( aHelpData.mbExtHelp && aHelpData.mbExtHelpMode )
    {
        aHelpData.mbExtHelpMode = false;
        aHelpData.mbBalloonHelp = aHelpData.mbOldBalloonMode;
        if (pSVData->maFrameData.mpAppWin)
            pSVData->maFrameData.mpAppWin->ImplGenerateMouseMove();
        return true;
    }

    return false;
}

void Help::EnableBalloonHelp()
{
    ImplGetSVHelpData().mbBalloonHelp = true;
}

void Help::DisableBalloonHelp()
{
    ImplGetSVHelpData().mbBalloonHelp = false;
}

bool Help::IsBalloonHelpEnabled()
{
    return ImplGetSVHelpData().mbBalloonHelp;
}

void Help::ShowBalloon( vcl::Window* pParent,
                        const Point& rScreenPos, const tools::Rectangle& rRect,
                        const OUString& rHelpText )
{
    ImplShowHelpWindow( pParent, HELPWINSTYLE_BALLOON, QuickHelpFlags::NONE,
                        rHelpText, rScreenPos, rRect );
}

void Help::EnableQuickHelp()
{
    ImplGetSVHelpData().mbQuickHelp = true;
}

void Help::DisableQuickHelp()
{
    ImplGetSVHelpData().mbQuickHelp = false;
}

bool Help::IsQuickHelpEnabled()
{
    return ImplGetSVHelpData().mbQuickHelp;
}

void Help::ShowQuickHelp( vcl::Window* pParent,
                          const tools::Rectangle& rScreenRect,
                          const OUString& rHelpText,
                          QuickHelpFlags nStyle )
{
    sal_uInt16 nHelpWinStyle = ( nStyle & QuickHelpFlags::TipStyleBalloon ) ? HELPWINSTYLE_BALLOON : HELPWINSTYLE_QUICK;
    Point aScreenPos = nStyle & QuickHelpFlags::NoAutoPos
                           ? Point()
                           : pParent->OutputToScreenPixel(pParent->GetPointerPosPixel());
    ImplShowHelpWindow( pParent, nHelpWinStyle, nStyle,
                        rHelpText, aScreenPos, rScreenRect );
}

void Help::HideBalloonAndQuickHelp()
{
    HelpTextWindow const * pHelpWin = ImplGetSVHelpData().mpHelpWin;
    bool const bIsVisible = ( pHelpWin != nullptr ) && pHelpWin->IsVisible();
    ImplDestroyHelpWindow( bIsVisible );
}

void* Help::ShowPopover(vcl::Window* pParent, const tools::Rectangle& rScreenRect,
                              const OUString& rText, QuickHelpFlags nStyle)
{
    void* nId = pParent->ImplGetFrame()->ShowPopover(rText, pParent, rScreenRect, nStyle);
    if (nId)
    {
        //popovers are handled natively, return early
        return nId;
    }

    sal_uInt16 nHelpWinStyle = ( nStyle & QuickHelpFlags::TipStyleBalloon ) ? HELPWINSTYLE_BALLOON : HELPWINSTYLE_QUICK;
    VclPtrInstance<HelpTextWindow> pHelpWin( pParent, rText, nHelpWinStyle, nStyle );

    nId = pHelpWin.get();
    UpdatePopover(nId, pParent, rScreenRect, rText);

    pHelpWin->ShowHelp(true);
    return nId;
}

void Help::UpdatePopover(void* nId, vcl::Window* pParent, const tools::Rectangle& rScreenRect,
                         const OUString& rText)
{
    if (pParent->ImplGetFrame()->UpdatePopover(nId, rText, pParent, rScreenRect))
    {
        //popovers are handled natively, return early
        return;
    }

    HelpTextWindow* pHelpWin = static_cast< HelpTextWindow* >( nId );
    ENSURE_OR_RETURN_VOID( pHelpWin != nullptr, "Help::UpdatePopover: invalid ID!" );

    Size aSz = pHelpWin->CalcOutSize();
    pHelpWin->SetOutputSizePixel( aSz );
    ImplSetHelpWindowPos( pHelpWin, pHelpWin->GetWinStyle(), pHelpWin->GetStyle(),
        pParent->OutputToScreenPixel( pParent->GetPointerPosPixel() ), rScreenRect );

    pHelpWin->SetHelpText( rText );
    pHelpWin->Invalidate();
}

void Help::HidePopover(vcl::Window const * pParent, void* nId)
{
    if (pParent->ImplGetFrame()->HidePopover(nId))
    {
        //popovers are handled natively, return early
        return;
    }

    VclPtr<HelpTextWindow> pHelpWin = static_cast<HelpTextWindow*>(nId);
    vcl::Window* pFrameWindow = pHelpWin->ImplGetFrameWindow();
    pHelpWin->Hide();
    // trigger update, so that a Paint is instantly triggered since we do not save the background
    pFrameWindow->ImplUpdateAll();
    pHelpWin.disposeAndClear();
    ImplGetSVHelpData().mnLastHelpHideTime = tools::Time::GetSystemTicks();
}

HelpTextWindow::HelpTextWindow( vcl::Window* pParent, const OUString& rText, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle ) :
    FloatingWindow( pParent, WB_SYSTEMWINDOW|WB_TOOLTIPWIN ), // #105827# if we change the parent, mirroring will not work correctly when positioning this window
    maHelpText( rText ),
    maShowTimer( "vcl::HelpTextWindow maShowTimer" ),
    maHideTimer( "vcl::HelpTextWindow maHideTimer" )
{
    SetType( WindowType::HELPTEXTWINDOW );
    ImplSetMouseTransparent( true );
    mnHelpWinStyle = nHelpWinStyle;
    mnStyle = nStyle;

    if( mnStyle & QuickHelpFlags::BiDiRtl )
    {
        vcl::text::ComplexTextLayoutFlags nLayoutMode = GetOutDev()->GetLayoutMode();
        nLayoutMode |= vcl::text::ComplexTextLayoutFlags::BiDiRtl | vcl::text::ComplexTextLayoutFlags::TextOriginLeft;
        GetOutDev()->SetLayoutMode( nLayoutMode );
    }
    SetHelpText( rText );
    Window::SetHelpText( rText );

    if ( ImplGetSVHelpData().mbSetKeyboardHelp )
        ImplGetSVHelpData().mbKeyboardHelp = true;


    maShowTimer.SetInvokeHandler( LINK( this, HelpTextWindow, TimerHdl ) );

    const HelpSettings& rHelpSettings = pParent->GetSettings().GetHelpSettings();
    maHideTimer.SetTimeout( rHelpSettings.GetTipTimeout() );
    maHideTimer.SetInvokeHandler( LINK( this, HelpTextWindow, TimerHdl ) );
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

    if( this == ImplGetSVHelpData().mpHelpWin )
        ImplGetSVHelpData().mpHelpWin = nullptr;
    FloatingWindow::dispose();
}

void HelpTextWindow::SetHelpText( const OUString& rHelpText )
{
    maHelpText = rHelpText;
    ApplySettings(*GetOutDev());
    if ( mnHelpWinStyle == HELPWINSTYLE_QUICK && maHelpText.getLength() < HELPTEXTMAXLEN && maHelpText.indexOf('\n') < 0)
    {
        Size aSize;
        aSize.setHeight( GetTextHeight() );
        if ( mnStyle & QuickHelpFlags::CtrlText )
            aSize.setWidth( GetOutDev()->GetCtrlTextWidth( maHelpText ) );
        else
            aSize.setWidth( GetTextWidth( maHelpText ) );
        maTextRect = tools::Rectangle( Point( HELPTEXTMARGIN_QUICK, HELPTEXTMARGIN_QUICK ), aSize );
    }
    else // HELPWINSTYLE_BALLOON
    {
        sal_Int32 nCharsInLine;
        if (mnHelpWinStyle == HELPWINSTYLE_QUICK)
            nCharsInLine = maHelpText.getLength();
        else
            nCharsInLine = 35 + ((maHelpText.getLength() / 100) * 5);
        // average width to have all windows consistent
        OUStringBuffer aBuf(nCharsInLine);
        comphelper::string::padToLength(aBuf, nCharsInLine, 'x');
        tools::Long nWidth = GetTextWidth( OUString::unacquired(aBuf) );
        Size aTmpSize( nWidth, 0x7FFFFFFF );
        tools::Rectangle aTry1( Point(), aTmpSize );
        DrawTextFlags nDrawFlags = DrawTextFlags::MultiLine | DrawTextFlags::WordBreak |
                            DrawTextFlags::Left | DrawTextFlags::Top;
        if ( mnStyle & QuickHelpFlags::CtrlText )
            nDrawFlags |= DrawTextFlags::Mnemonic;
        tools::Rectangle aTextRect = GetTextRect( aTry1, maHelpText, nDrawFlags );

        // get a better width later...
        maTextRect = aTextRect;

        // safety distance...
        maTextRect.SetPos( Point( HELPTEXTMARGIN_BALLOON, HELPTEXTMARGIN_BALLOON ) );
    }

    Size aSize( CalcOutSize() );
    SetOutputSizePixel( aSize );
    if (IsVisible())
        PaintImmediately();
}

void HelpTextWindow::ImplShow()
{
    VclPtr<HelpTextWindow> xWindow( this );
    Show( true, ShowFlags::NoActivate );
    if( !xWindow->isDisposed() )
        PaintImmediately();
}

void HelpTextWindow::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    // paint native background
    bool bNativeOK = false;
    if (rRenderContext.IsNativeControlSupported(ControlType::Tooltip, ControlPart::Entire))
    {
        tools::Rectangle aCtrlRegion(Point(0, 0), GetOutputSizePixel());
        ImplControlValue aControlValue;
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::Tooltip, ControlPart::Entire, aCtrlRegion,
                                                     ControlState::NONE, aControlValue, OUString());
    }

    // paint text
    if (mnHelpWinStyle == HELPWINSTYLE_QUICK && maHelpText.getLength() < HELPTEXTMAXLEN && maHelpText.indexOf('\n') < 0)
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
    if (bNativeOK)
        return;

    Size aSz = GetOutputSizePixel();
    rRenderContext.DrawRect(tools::Rectangle(Point(), aSz));
    if (mnHelpWinStyle == HELPWINSTYLE_BALLOON)
    {
        aSz.AdjustWidth( -2 );
        aSz.AdjustHeight( -2 );
        Color aColor(rRenderContext.GetLineColor());
        rRenderContext.SetLineColor(COL_GRAY);
        rRenderContext.DrawRect(tools::Rectangle(Point(1, 1), aSz));
        rRenderContext.SetLineColor(aColor);
    }
}

void HelpTextWindow::ShowHelp(bool bNoDelay)
{
    sal_uLong nTimeout = 0;
    if (!bNoDelay)
    {
        // In case of ExtendedHelp display help sooner
        if ( ImplGetSVHelpData().mbExtHelpMode )
            nTimeout = 15;
        else
        {
            if ( mnHelpWinStyle == HELPWINSTYLE_QUICK )
                nTimeout = HelpSettings::GetTipDelay();
            else
                nTimeout = HelpSettings::GetBalloonDelay();
        }
    }

    maShowTimer.SetTimeout( nTimeout );
    maShowTimer.Start();
}

IMPL_LINK( HelpTextWindow, TimerHdl, Timer*, pTimer, void)
{
    if ( pTimer == &maShowTimer )
    {
        ResetHideTimer();
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
    aSz.AdjustWidth(2*maTextRect.Left() );
    aSz.AdjustHeight(2*maTextRect.Top() );
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

void HelpTextWindow::ResetHideTimer()
{
    if (mnHelpWinStyle == HELPWINSTYLE_QUICK)
    {
        // start auto-hide-timer for non-ShowTip windows
        if (this == ImplGetSVHelpData().mpHelpWin)
            maHideTimer.Start();
    }
}

void ImplShowHelpWindow( vcl::Window* pParent, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle,
                         const OUString& rHelpText,
                         const Point& rScreenPos, const tools::Rectangle& rHelpArea )
{
    if (pParent->ImplGetFrame()->ShowTooltip(rHelpText, rHelpArea))
    {
        //tooltips are handled natively, return early
        return;
    }

    ImplSVHelpData& aHelpData = ImplGetSVHelpData();

    if (rHelpText.isEmpty() && !aHelpData.mbRequestingHelp)
        return;

    bool bNoDelay = false;
    if (VclPtr<HelpTextWindow> pHelpWin = aHelpData.mpHelpWin)
    {
        SAL_WARN_IF( pHelpWin == pParent, "vcl", "HelpInHelp ?!" );

        bool bRemoveHelp = (rHelpText.isEmpty() || (pHelpWin->GetWinStyle() != nHelpWinStyle))
                            && aHelpData.mbRequestingHelp;

        if (!bRemoveHelp && pHelpWin->GetParent() == pParent)
        {
            bool const bUpdate = (pHelpWin->GetHelpText() != rHelpText) ||
                ((pHelpWin->GetHelpArea() != rHelpArea) && aHelpData.mbRequestingHelp);
            if (bUpdate)
            {
                pHelpWin->SetHelpText( rHelpText );
                // approach mouse position
                ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, rHelpArea );
                if( pHelpWin->IsVisible() )
                    pHelpWin->Invalidate();
            }
            pHelpWin->ResetHideTimer(); // It is shown anew, so prolongate the hide timeout
            return;
        }

        // remove help window if no HelpText or
        // other help mode. but keep it if we are scrolling, ie not requesting help
        bool bWasVisible = pHelpWin->IsVisible();
        if ( bWasVisible )
            bNoDelay = true; // display it quickly if we were already in quick help mode
        ImplDestroyHelpWindow( bWasVisible );
    }

    if (rHelpText.isEmpty())
        return;

    VclPtr<HelpTextWindow> pHelpWin = VclPtr<HelpTextWindow>::Create( pParent, rHelpText, nHelpWinStyle, nStyle );
    aHelpData.mpHelpWin = pHelpWin;
    pHelpWin->SetHelpArea( rHelpArea );

    //  positioning
    Size aSz = pHelpWin->CalcOutSize();
    pHelpWin->SetOutputSizePixel( aSz );
    ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, rHelpArea );
    // if not called from Window::RequestHelp, then without delay...
    if (!bNoDelay)
    {
        if ( !aHelpData.mbRequestingHelp )
        {
            bNoDelay = true;
        }
        else
        {
            sal_uInt64 nCurTime = tools::Time::GetSystemTicks();
            if ( ( nCurTime - aHelpData.mnLastHelpHideTime ) < o3tl::make_unsigned(HelpSettings::GetTipDelay()) )
                bNoDelay = true;
        }
    }
    pHelpWin->ShowHelp(bNoDelay);
}

void ImplDestroyHelpWindow( bool bUpdateHideTime )
{
    ImplDestroyHelpWindow(ImplGetSVHelpData(), bUpdateHideTime);
}

void ImplDestroyHelpWindow(ImplSVHelpData& rHelpData, bool bUpdateHideTime)
{
    VclPtr<HelpTextWindow> pHelpWin = rHelpData.mpHelpWin;
    if( pHelpWin )
    {
        rHelpData.mpHelpWin = nullptr;
        rHelpData.mbKeyboardHelp = false;
        pHelpWin->Hide();
        pHelpWin.disposeAndClear();
        if( bUpdateHideTime )
            rHelpData.mnLastHelpHideTime = tools::Time::GetSystemTicks();
    }
}

void ImplSetHelpWindowPos( vcl::Window* pHelpWin, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle,
                           const Point& rPos, const tools::Rectangle& rHelpArea )
{
    AbsoluteScreenPixelPoint aPos;
    AbsoluteScreenPixelSize aSz( pHelpWin->GetSizePixel() );
    AbsoluteScreenPixelRectangle   aScreenRect = pHelpWin->ImplGetFrameWindow()->GetDesktopRectPixel();
    vcl::Window* pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
    // get mouse screen coords
    AbsoluteScreenPixelPoint aMousePos(pWindow->OutputToAbsoluteScreenPixel(pWindow->GetPointerPosPixel()));

    if ( nStyle & QuickHelpFlags::NoAutoPos )
    {
        // convert help area to screen coords
        AbsoluteScreenPixelRectangle devHelpArea(
            pWindow->OutputToAbsoluteScreenPixel( rHelpArea.TopLeft() ),
            pWindow->OutputToAbsoluteScreenPixel( rHelpArea.BottomRight() ) );

        // which position of the rectangle?
        aPos = devHelpArea.Center();

        if ( nStyle & QuickHelpFlags::Left )
            aPos.setX( devHelpArea.Left() );
        else if ( nStyle & QuickHelpFlags::Right )
            aPos.setX( devHelpArea.Right() );

        if ( nStyle & QuickHelpFlags::Top )
            aPos.setY( devHelpArea.Top() );
        else if ( nStyle & QuickHelpFlags::Bottom )
            aPos.setY( devHelpArea.Bottom() );

        // which direction?
        if ( nStyle & QuickHelpFlags::Left )
            ;
        else if ( nStyle & QuickHelpFlags::Right )
            aPos.AdjustX( -(aSz.Width()) );
        else
            aPos.AdjustX( -(aSz.Width()/2) );

        if ( nStyle & QuickHelpFlags::Top )
            ;
        else if ( nStyle & QuickHelpFlags::Bottom )
            aPos.AdjustY( -(aSz.Height()) );
        else
            aPos.AdjustY( -(aSz.Height()/2) );
    }
    else
    {
        aPos = pWindow->OutputToAbsoluteScreenPixel(rPos);
        if ( nHelpWinStyle == HELPWINSTYLE_QUICK )
        {
            tools::Long nScreenHeight = aScreenRect.GetHeight();
            aPos.AdjustX( -4 );
            if ( aPos.Y() > aScreenRect.Top()+nScreenHeight-(nScreenHeight/4) )
                aPos.AdjustY( -(aSz.Height()+4) );
            else
                aPos.AdjustY(21 );
        }
        else
        {
            // If it's the mouse position, move the window slightly
            // so the mouse pointer does not cover it
            if ( aPos == aMousePos )
            {
                aPos.AdjustX(12 );
                aPos.AdjustY(16 );
            }
        }
    }

    if ( aPos.X() < aScreenRect.Left() )
        aPos.setX( aScreenRect.Left() );
    else if ( ( aPos.X() + aSz.Width() ) > aScreenRect.Right() )
        aPos.setX( aScreenRect.Right() - aSz.Width() );
    if ( aPos.Y() < aScreenRect.Top() )
        aPos.setY( aScreenRect.Top() );
    else if ( ( aPos.Y() + aSz.Height() ) > aScreenRect.Bottom() )
        aPos.setY( aScreenRect.Bottom() - aSz.Height() );

    if( ! (nStyle & QuickHelpFlags::NoEvadePointer) )
    {
        /* the remark below should be obsolete by now as the helpwindow should
        not be focusable, leaving it as a hint. However it is sensible in most
        conditions to evade the mouse pointer so the content window is fully visible.

        // the popup must not appear under the mouse
        // otherwise it would directly be closed due to a focus change...
        */
        AbsoluteScreenPixelRectangle aHelpRect( aPos, aSz );
        if( aHelpRect.Contains( aMousePos ) )
        {
            AbsoluteScreenPixelPoint delta(2,2);
            AbsoluteScreenPixelPoint aSize( aSz.Width(), aSz.Height() );
            AbsoluteScreenPixelPoint aTest( aMousePos - aSize - delta );
            if( aTest.X() > aScreenRect.Left() && aTest.Y() > aScreenRect.Top() )
                aPos = aTest;
            else
                aPos = aMousePos + delta;
        }
    }

    Point aPosOut = pWindow->AbsoluteScreenToOutputPixel( aPos );
    pHelpWin->SetPosPixel( aPosOut );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
