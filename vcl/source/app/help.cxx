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

#include <tools/diagnose_ex.h>
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

OUString Help::GetHelpText( const OUString&, const vcl::Window* )
{
    return OUString();
}

OUString Help::GetHelpText( const OUString&, const weld::Widget* )
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

void Help::ShowBalloon( vcl::Window* pParent,
                        const Point& rScreenPos, const tools::Rectangle& rRect,
                        const OUString& rHelpText )
{
    ImplShowHelpWindow( pParent, HELPWINSTYLE_BALLOON, QuickHelpFlags::NONE,
                        rHelpText, rScreenPos, rRect );
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

void Help::ShowQuickHelp( vcl::Window* pParent,
                          const tools::Rectangle& rScreenRect,
                          const OUString& rHelpText,
                          QuickHelpFlags nStyle )
{
    sal_uInt16 nHelpWinStyle = ( nStyle & QuickHelpFlags::TipStyleBalloon ) ? HELPWINSTYLE_BALLOON : HELPWINSTYLE_QUICK;
    ImplShowHelpWindow( pParent, nHelpWinStyle, nStyle,
                        rHelpText,
                        pParent->OutputToScreenPixel( pParent->GetPointerPosPixel() ), rScreenRect );
}

void Help::HideBalloonAndQuickHelp()
{
    HelpTextWindow const * pHelpWin = ImplGetSVData()->maHelpData.mpHelpWin;
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
    ImplGetSVData()->maHelpData.mnLastHelpHideTime = tools::Time::GetSystemTicks();
}

HelpTextWindow::HelpTextWindow( vcl::Window* pParent, const OUString& rText, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle ) :
    FloatingWindow( pParent, WB_SYSTEMWINDOW|WB_TOOLTIPWIN ), // #105827# if we change the parent, mirroring will not work correctly when positioning this window
    maHelpText( rText )
{
    SetType( WindowType::HELPTEXTWINDOW );
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


    maShowTimer.SetInvokeHandler( LINK( this, HelpTextWindow, TimerHdl ) );
    maShowTimer.SetDebugName( "vcl::HelpTextWindow maShowTimer" );

    const HelpSettings& rHelpSettings = pParent->GetSettings().GetHelpSettings();
    maHideTimer.SetTimeout( rHelpSettings.GetTipTimeout() );
    maHideTimer.SetInvokeHandler( LINK( this, HelpTextWindow, TimerHdl ) );
    maHideTimer.SetDebugName( "vcl::HelpTextWindow maHideTimer" );
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
    ApplySettings(*this);
    if ( mnHelpWinStyle == HELPWINSTYLE_QUICK && maHelpText.getLength() < HELPTEXTMAXLEN)
    {
        Size aSize;
        aSize.setHeight( GetTextHeight() );
        if ( mnStyle & QuickHelpFlags::CtrlText )
            aSize.setWidth( GetCtrlTextWidth( maHelpText ) );
        else
            aSize.setWidth( GetTextWidth( maHelpText ) );
        maTextRect = tools::Rectangle( Point( HELPTEXTMARGIN_QUICK, HELPTEXTMARGIN_QUICK ), aSize );
    }
    else // HELPWINSTYLE_BALLOON
    {
        sal_Int32 nCharsInLine = 35 + ((maHelpText.getLength()/100)*5);
        // average width to have all windows consistent
        OUStringBuffer aBuf;
        comphelper::string::padToLength(aBuf, nCharsInLine, 'x');
        OUString aXXX = aBuf.makeStringAndClear();
        long nWidth = GetTextWidth( aXXX );
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
}

void HelpTextWindow::ImplShow()
{
    VclPtr<HelpTextWindow> xWindow( this );
    Show( true, ShowFlags::NoActivate );
    if( !xWindow->IsDisposed() )
    Update();
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
}

void HelpTextWindow::ShowHelp(bool bNoDelay)
{
    sal_uLong nTimeout = 0;
    if (!bNoDelay)
    {
        // In case of ExtendedHelp display help sooner
        if ( ImplGetSVData()->maHelpData.mbExtHelpMode )
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

void ImplShowHelpWindow( vcl::Window* pParent, sal_uInt16 nHelpWinStyle, QuickHelpFlags nStyle,
                         const OUString& rHelpText,
                         const Point& rScreenPos, const tools::Rectangle& rHelpArea )
{
    if (pParent->ImplGetFrame()->ShowTooltip(rHelpText, rHelpArea))
    {
        //tooltips are handled natively, return early
        return;
    }

    ImplSVData* pSVData = ImplGetSVData();

    if (rHelpText.isEmpty() && !pSVData->maHelpData.mbRequestingHelp)
        return;

    VclPtr<HelpTextWindow> pHelpWin = pSVData->maHelpData.mpHelpWin;
    bool bNoDelay = false;
    if ( pHelpWin )
    {
        SAL_WARN_IF( pHelpWin == pParent, "vcl", "HelpInHelp ?!" );

        if  (   (   rHelpText.isEmpty()
                ||  ( pHelpWin->GetWinStyle() != nHelpWinStyle )
                )
            &&  pSVData->maHelpData.mbRequestingHelp
            )
        {
            // remove help window if no HelpText or
            // other help mode. but keep it if we are scrolling, ie not requesting help
            bool bWasVisible = pHelpWin->IsVisible();
            if ( bWasVisible )
                bNoDelay = true; // display it quickly if we were already in quick help mode
            pHelpWin = nullptr;
            ImplDestroyHelpWindow( bWasVisible );
        }
        else
        {
            bool const bUpdate = (pHelpWin->GetHelpText() != rHelpText) ||
                ((pHelpWin->GetHelpArea() != rHelpArea) && pSVData->maHelpData.mbRequestingHelp);
            if (bUpdate)
            {
                vcl::Window * pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
                tools::Rectangle aInvRect( pHelpWin->GetWindowExtentsRelative( pWindow ) );
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

    if (pHelpWin || rHelpText.isEmpty())
        return;

    sal_uInt64 nCurTime = tools::Time::GetSystemTicks();
    if ( ( nCurTime - pSVData->maHelpData.mnLastHelpHideTime ) < HelpSettings::GetTipDelay() )
        bNoDelay = true;

    pHelpWin = VclPtr<HelpTextWindow>::Create( pParent, rHelpText, nHelpWinStyle, nStyle );
    pSVData->maHelpData.mpHelpWin = pHelpWin;
    pHelpWin->SetHelpArea( rHelpArea );

    //  positioning
    Size aSz = pHelpWin->CalcOutSize();
    pHelpWin->SetOutputSizePixel( aSz );
    ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, rHelpArea );
    // if not called from Window::RequestHelp, then without delay...
    if ( !pSVData->maHelpData.mbRequestingHelp )
        bNoDelay = true;
    pHelpWin->ShowHelp(bNoDelay);

}

void ImplDestroyHelpWindow( bool bUpdateHideTime )
{
    ImplSVData* pSVData = ImplGetSVData();
    VclPtr<HelpTextWindow> pHelpWin = pSVData->maHelpData.mpHelpWin;
    if ( pHelpWin )
    {
        vcl::Window * pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
        // find out screen area covered by system help window
        tools::Rectangle aInvRect( pHelpWin->GetWindowExtentsRelative( pWindow ) );
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
                           const Point& rPos, const tools::Rectangle& rHelpArea )
{
    Point       aPos = rPos;
    Size        aSz = pHelpWin->GetSizePixel();
    tools::Rectangle   aScreenRect = pHelpWin->ImplGetFrameWindow()->GetDesktopRectPixel();
    aPos = pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( aPos );
    // get mouse screen coords
    Point aMousePos( pHelpWin->GetParent()->ImplGetFrameWindow()->GetPointerPosPixel() );
    aMousePos = pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( aMousePos );

    if ( nHelpWinStyle == HELPWINSTYLE_QUICK )
    {
        if ( !(nStyle & QuickHelpFlags::NoAutoPos) )
        {
            long nScreenHeight = aScreenRect.GetHeight();
            aPos.AdjustX( -4 );
            if ( aPos.Y() > aScreenRect.Top()+nScreenHeight-(nScreenHeight/4) )
                aPos.AdjustY( -(aSz.Height()+4) );
            else
                aPos.AdjustY(21 );
        }
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

    if ( nStyle & QuickHelpFlags::NoAutoPos )
    {
        // convert help area to screen coords
        tools::Rectangle devHelpArea(
            pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( rHelpArea.TopLeft() ),
            pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( rHelpArea.BottomRight() ) );

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
        tools::Rectangle aHelpRect( aPos, aSz );
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
