/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: help.cxx,v $
 * $Revision: 1.40 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vcl/svdata.hxx"
#include "vcl/window.hxx"
#include "vcl/event.hxx"
#include "vcl/svapp.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/help.hxx"
#include "vcl/helpwin.hxx"
#include "tools/debug.hxx"
#include "tools/time.hxx"

// =======================================================================

#define HELPWINSTYLE_QUICK      0
#define HELPWINSTYLE_BALLOON    1

#define HELPTEXTMARGIN_QUICK    3
#define HELPTEXTMARGIN_BALLOON  6

#define HELPDELAY_NORMAL        1
#define HELPDELAY_SHORT         2
#define HELPDELAY_NONE          3

// =======================================================================

Help::Help()
{
}

Help::~Help()
{
}

// -----------------------------------------------------------------------

BOOL Help::Start( ULONG, const Window* )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::Start( const XubString&, const Window* )
{
    return FALSE;
}

// -----------------------------------------------------------------------

XubString Help::GetHelpText( ULONG, const Window* )
{
    return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

XubString Help::GetHelpText( const String&, const Window* )
{
    return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void Help::EnableContextHelp()
{
    ImplGetSVData()->maHelpData.mbContextHelp = TRUE;
}

// -----------------------------------------------------------------------

void Help::DisableContextHelp()
{
    ImplGetSVData()->maHelpData.mbContextHelp = FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::IsContextHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbContextHelp;
}

// -----------------------------------------------------------------------

BOOL Help::StartContextHelp()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mbContextHelp )
    {
        Window* pWindow = pSVData->maWinData.mpFocusWin;
        if ( pWindow )
        {
            Point       aMousePos = pWindow->OutputToScreenPixel( pWindow->GetPointerPosPixel() );
            HelpEvent   aHelpEvent( aMousePos, HELPMODE_CONTEXT );
            pWindow->RequestHelp( aHelpEvent );
            return TRUE;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Help::EnableExtHelp()
{
    ImplGetSVData()->maHelpData.mbExtHelp = TRUE;
}

// -----------------------------------------------------------------------

void Help::DisableExtHelp()
{
    ImplGetSVData()->maHelpData.mbExtHelp = FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::IsExtHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbExtHelp;
}

// -----------------------------------------------------------------------

BOOL Help::StartExtHelp()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mbExtHelp && !pSVData->maHelpData.mbExtHelpMode )
    {
        pSVData->maHelpData.mbExtHelpMode = TRUE;
        pSVData->maHelpData.mbOldBalloonMode = pSVData->maHelpData.mbBalloonHelp;
        pSVData->maHelpData.mbBalloonHelp = TRUE;
        if ( pSVData->maWinData.mpAppWin )
            pSVData->maWinData.mpAppWin->ImplGenerateMouseMove();
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::EndExtHelp()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mbExtHelp && pSVData->maHelpData.mbExtHelpMode )
    {
        pSVData->maHelpData.mbExtHelpMode = FALSE;
        pSVData->maHelpData.mbBalloonHelp = pSVData->maHelpData.mbOldBalloonMode;
        if ( pSVData->maWinData.mpAppWin )
            pSVData->maWinData.mpAppWin->ImplGenerateMouseMove();
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::IsExtHelpActive()
{
    return ImplGetSVData()->maHelpData.mbExtHelpMode;
}

// -----------------------------------------------------------------------

void Help::EnableBalloonHelp()
{
    ImplGetSVData()->maHelpData.mbBalloonHelp = TRUE;
}

// -----------------------------------------------------------------------

void Help::DisableBalloonHelp()
{
    ImplGetSVData()->maHelpData.mbBalloonHelp = FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::IsBalloonHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbBalloonHelp;
}

// -----------------------------------------------------------------------

BOOL Help::ShowBalloon( Window* pParent,
                        const Point& rScreenPos,
                        const XubString& rHelpText )
{
    ImplShowHelpWindow( pParent, HELPWINSTYLE_BALLOON, 0,
                        rHelpText, ImplGetSVEmptyStr(), rScreenPos );

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Help::ShowBalloon( Window* pParent,
                        const Point& rScreenPos, const Rectangle& rRect,
                        const XubString& rHelpText )
{
    ImplShowHelpWindow( pParent, HELPWINSTYLE_BALLOON, 0,
                        rHelpText, ImplGetSVEmptyStr(), rScreenPos, &rRect );

    return TRUE;
}

// -----------------------------------------------------------------------

void Help::EnableQuickHelp()
{
    ImplGetSVData()->maHelpData.mbQuickHelp = TRUE;
}

// -----------------------------------------------------------------------

void Help::DisableQuickHelp()
{
    ImplGetSVData()->maHelpData.mbQuickHelp = FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::IsQuickHelpEnabled()
{
    return ImplGetSVData()->maHelpData.mbQuickHelp;
}

// -----------------------------------------------------------------------

BOOL Help::ShowQuickHelp( Window* pParent,
                          const Rectangle& rScreenRect,
                          const XubString& rHelpText,
                          const XubString& rLongHelpText,
                          USHORT nStyle )
{
    ImplShowHelpWindow( pParent, HELPWINSTYLE_QUICK, nStyle,
                        rHelpText, rLongHelpText,
                        pParent->OutputToScreenPixel( pParent->GetPointerPosPixel() ), &rScreenRect );
    return TRUE;
}

// -----------------------------------------------------------------------

ULONG Help::ShowTip( Window* pParent, const Rectangle& rRect,
                     const XubString& rText, USHORT nStyle )
{
    USHORT nHelpWinStyle = HELPWINSTYLE_QUICK;
    HelpTextWindow* pHelpWin = new HelpTextWindow( pParent, rText, nHelpWinStyle, nStyle );

    Size aSz = pHelpWin->CalcOutSize();
    pHelpWin->SetOutputSizePixel( aSz );
    ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle,
        pParent->OutputToScreenPixel( pParent->GetPointerPosPixel() ), &rRect );
    pHelpWin->ShowHelp( HELPDELAY_NONE );
    return (ULONG)pHelpWin;
}

// -----------------------------------------------------------------------

void Help::HideTip( ULONG nId )
{
    HelpTextWindow* pHelpWin = (HelpTextWindow*)nId;
    Window* pFrameWindow = pHelpWin->ImplGetFrameWindow();
    pHelpWin->Hide();
    // Update ausloesen, damit ein Paint sofort ausgeloest wird, da
    // wir den Hintergrund nicht sichern
    pFrameWindow->ImplUpdateAll();
    delete pHelpWin;
    ImplGetSVData()->maHelpData.mnLastHelpHideTime = Time::GetSystemTicks();
}

// =======================================================================

HelpTextWindow::HelpTextWindow( Window* pParent, const XubString& rText, USHORT nHelpWinStyle, USHORT nStyle ) :
    //FloatingWindow( pParent->ImplGetFrameWindow(), WB_SYSTEMWINDOW ),
    FloatingWindow( pParent, WB_SYSTEMWINDOW|WB_TOOLTIPWIN ), // #105827# if we change the parent, mirroring will not work correctly when positioning this window
    maHelpText( rText )
{
    SetType( WINDOW_HELPTEXTWINDOW );
    ImplSetMouseTransparent( TRUE );
    mnHelpWinStyle = nHelpWinStyle;
    mnStyle = nStyle;
//  on windows this will raise the application window, because help windows are system windows now
//  EnableAlwaysOnTop();
    EnableSaveBackground();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetPointFont( rStyleSettings.GetHelpFont() );
    SetTextColor( rStyleSettings.GetHelpTextColor() );
    SetTextAlign( ALIGN_TOP );
    if ( IsNativeControlSupported( CTRL_TOOLTIP, PART_ENTIRE_CONTROL ) )
    {
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
        SetBackground( Wallpaper( rStyleSettings.GetHelpColor() ) );
    if( rStyleSettings.GetHelpColor().IsDark() )
        SetLineColor( COL_WHITE );
    else
        SetLineColor( COL_BLACK );
    SetFillColor();

    if( mnStyle & QUICKHELP_BIDI_RTL )
    {
        ULONG nLayoutMode = GetLayoutMode();
        nLayoutMode |= TEXT_LAYOUT_BIDI_RTL | TEXT_LAYOUT_TEXTORIGIN_LEFT;
        SetLayoutMode( nLayoutMode );
    }
    SetHelpText( rText );
    Window::SetHelpText( rText );

    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maHelpData.mbSetKeyboardHelp )
        pSVData->maHelpData.mbKeyboardHelp = TRUE;

    const HelpSettings& rHelpSettings = pParent->GetSettings().GetHelpSettings();
    maShowTimer.SetTimeoutHdl( LINK( this, HelpTextWindow, TimerHdl ) );
    maHideTimer.SetTimeoutHdl( LINK( this, HelpTextWindow, TimerHdl ) );
    maHideTimer.SetTimeout( rHelpSettings.GetTipTimeout() );
}

// -----------------------------------------------------------------------

HelpTextWindow::~HelpTextWindow()
{
    maShowTimer.Stop();
    maHideTimer.Stop();

    if( this == ImplGetSVData()->maHelpData.mpHelpWin )
        ImplGetSVData()->maHelpData.mpHelpWin = NULL;

    if ( maStatusText.Len() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        pSVData->mpApp->HideHelpStatusText();
    }
}

// -----------------------------------------------------------------------

void HelpTextWindow::SetHelpText( const String& rHelpText )
{
    maHelpText = rHelpText;
    if ( mnHelpWinStyle == HELPWINSTYLE_QUICK )
    {
        Size aSize;
        aSize.Height() = GetTextHeight();
        if ( mnStyle & QUICKHELP_CTRLTEXT )
            aSize.Width() = GetCtrlTextWidth( maHelpText );
        else
            aSize.Width() = GetTextWidth( maHelpText );
        maTextRect = Rectangle( Point( HELPTEXTMARGIN_QUICK, HELPTEXTMARGIN_QUICK ), aSize );
    }
    else // HELPWINSTYLE_BALLOON
    {
        Point       aTmpPoint;
        USHORT      nCharsInLine = 35 + ((maHelpText.Len()/100)*5);
        XubString   aXXX;
        aXXX.Fill( nCharsInLine, 'x' );   // Durchschnittliche Breite, damit nicht jedes Fenster anders.
        long nWidth = GetTextWidth( aXXX );
        Size aTmpSize( nWidth, 0x7FFFFFFF );
        Rectangle aTry1( aTmpPoint, aTmpSize );
        USHORT nDrawFlags = TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK |
                            TEXT_DRAW_LEFT | TEXT_DRAW_TOP;
        if ( mnStyle & QUICKHELP_CTRLTEXT )
            nDrawFlags |= TEXT_DRAW_MNEMONIC;
        Rectangle aTextRect = GetTextRect( aTry1, maHelpText, nDrawFlags );

        // Spaeter mal eine geeignete Breite ermitteln...
        maTextRect = aTextRect;

        // Sicherheitsabstand...
        maTextRect.SetPos( Point( HELPTEXTMARGIN_BALLOON, HELPTEXTMARGIN_BALLOON ) );
    }

    Size aSize( CalcOutSize() );
    SetOutputSizePixel( aSize );
}

// -----------------------------------------------------------------------

void HelpTextWindow::ImplShow()
{
    ImplDelData aDogTag( this );
    if ( maStatusText.Len() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        pSVData->mpApp->ShowHelpStatusText( maStatusText );
    }
    Show( TRUE, SHOW_NOACTIVATE );
    if( !aDogTag.IsDelete() )
    Update();
}

// -----------------------------------------------------------------------

void HelpTextWindow::Paint( const Rectangle& )
{
    // paint native background
    bool bNativeOK = false;
    if ( IsNativeControlSupported( CTRL_TOOLTIP, PART_ENTIRE_CONTROL ) )
    {
        // #i46472# workaround gcc3.3 temporary problem
        Region aCtrlRegion = Region( Rectangle( Point( 0, 0 ), GetOutputSizePixel() ) );
        ImplControlValue    aControlValue;
        bNativeOK = DrawNativeControl( CTRL_TOOLTIP, PART_ENTIRE_CONTROL, aCtrlRegion,
                                       0, aControlValue, rtl::OUString() );
    }

    // paint text
    if ( mnHelpWinStyle == HELPWINSTYLE_QUICK )
    {
        if ( mnStyle & QUICKHELP_CTRLTEXT )
            DrawCtrlText( maTextRect.TopLeft(), maHelpText );
        else
            DrawText( maTextRect.TopLeft(), maHelpText );
    }
    else // HELPWINSTYLE_BALLOON
    {
        USHORT nDrawFlags = TEXT_DRAW_MULTILINE|TEXT_DRAW_WORDBREAK|
                                TEXT_DRAW_LEFT|TEXT_DRAW_TOP;
        if ( mnStyle & QUICKHELP_CTRLTEXT )
            nDrawFlags |= TEXT_DRAW_MNEMONIC;
        DrawText( maTextRect, maHelpText, nDrawFlags );
    }

    // border
    if( ! bNativeOK )
    {
        Size aSz = GetOutputSizePixel();
        DrawRect( Rectangle( Point(), aSz ) );
        if ( mnHelpWinStyle == HELPWINSTYLE_BALLOON )
        {
            aSz.Width() -= 2;
            aSz.Height() -= 2;
            Color aColor( GetLineColor() );
            SetLineColor( ( COL_GRAY ) );
            DrawRect( Rectangle( Point( 1, 1 ), aSz ) );
            SetLineColor( aColor );
        }
    }
}

// -----------------------------------------------------------------------

void HelpTextWindow::ShowHelp( USHORT nDelayMode )
{
    ULONG nTimeout = 0;
    if ( nDelayMode != HELPDELAY_NONE )
    {
        // Im ExtendedHelp-Fall die Hilfe schneller anzeigen
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

// -----------------------------------------------------------------------

IMPL_LINK( HelpTextWindow, TimerHdl, Timer*, pTimer)
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
        DBG_ASSERT( pTimer == &maHideTimer, "HelpTextWindow::TimerHdl with bad Timer" );
          ImplDestroyHelpWindow( true );
    }

    return 1;
}

// -----------------------------------------------------------------------

Size HelpTextWindow::CalcOutSize() const
{
    Size aSz = maTextRect.GetSize();
    aSz.Width() += 2*maTextRect.Left();
    aSz.Height() += 2*maTextRect.Top();
    return aSz;
}

// -----------------------------------------------------------------------

void HelpTextWindow::RequestHelp( const HelpEvent& /*rHEvt*/ )
{
    // Nur damit nicht von Window::RequestHelp() ein
    // ShowQuickHelp/ShowBalloonHelp am HelpTextWindow aufgerufen wird.
}

// -----------------------------------------------------------------------

String HelpTextWindow::GetText() const
{
    return maHelpText;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > HelpTextWindow::CreateAccessible()
{
    return FloatingWindow::CreateAccessible();
}

// -----------------------------------------------------------------------

BOOL HelpTextWindow::RegisterAccessibleParent()
{
        return FALSE;
}

// -----------------------------------------------------------------------

void HelpTextWindow::RevokeAccessibleParent()
{
}

// =======================================================================

void ImplShowHelpWindow( Window* pParent, USHORT nHelpWinStyle, USHORT nStyle,
                         const XubString& rHelpText, const XubString& rStatusText,
                         const Point& rScreenPos, const Rectangle* pHelpArea )
{
    ImplSVData* pSVData = ImplGetSVData();

    if( !rHelpText.Len() && !pSVData->maHelpData.mbRequestingHelp )
        return;

    HelpTextWindow* pHelpWin = pSVData->maHelpData.mpHelpWin;
    USHORT nDelayMode = HELPDELAY_NORMAL;
    if ( pHelpWin )
    {
        DBG_ASSERT( pHelpWin != pParent, "HelpInHelp ?!" );

            if ( (( pHelpWin->GetHelpText() != rHelpText ) ||
                 ( pHelpWin->GetWinStyle() != nHelpWinStyle ) ||
                 ( pHelpArea && ( pHelpWin->GetHelpArea() != *pHelpArea ) ) )
                 && pSVData->maHelpData.mbRequestingHelp )
        {
            // remove help window if no HelpText or other HelpText or
            // other help mode. but keep it if we are scrolling, ie not requesting help
            bool bWasVisible = pHelpWin->IsVisible();
            if ( bWasVisible )
                nDelayMode = HELPDELAY_NONE; // display it quickly if we were already in quick help mode
            pHelpWin = NULL;
            ImplDestroyHelpWindow( bWasVisible );
        }
        else
        {
            bool bTextChanged = rHelpText != pHelpWin->GetHelpText();
            if( bTextChanged )
            {
                Window * pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
                Rectangle aInvRect( pHelpWin->GetWindowExtentsRelative( pWindow ) );
                if( pHelpWin->IsVisible() )
                    pWindow->Invalidate( aInvRect );

                pHelpWin->SetHelpText( rHelpText );
                // approach mouse position
                ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, pHelpArea );
                if( pHelpWin->IsVisible() )
                    pHelpWin->Invalidate();
            }
        }
    }

    if ( !pHelpWin && rHelpText.Len() )
    {
        ULONG nCurTime = Time::GetSystemTicks();
        if( (nCurTime - pSVData->maHelpData.mnLastHelpHideTime) < pParent->GetSettings().GetHelpSettings().GetTipDelay() )
            nDelayMode = HELPDELAY_NONE;

        DBG_ASSERT( !pHelpWin, "Noch ein HelpWin ?!" );
        pHelpWin = new HelpTextWindow( pParent, rHelpText, nHelpWinStyle, nStyle );
        pSVData->maHelpData.mpHelpWin = pHelpWin;
        pHelpWin->SetStatusText( rStatusText );
        if ( pHelpArea )
            pHelpWin->SetHelpArea( *pHelpArea );

        //  positioning
        Size aSz = pHelpWin->CalcOutSize();
        pHelpWin->SetOutputSizePixel( aSz );
        ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, pHelpArea );
        // if not called from Window::RequestHelp, then without delay...
        if ( !pSVData->maHelpData.mbRequestingHelp )
            nDelayMode = HELPDELAY_NONE;
        pHelpWin->ShowHelp( nDelayMode );
    }
}

// -----------------------------------------------------------------------

void ImplDestroyHelpWindow( bool bUpdateHideTime )
{
    ImplSVData* pSVData = ImplGetSVData();
    HelpTextWindow* pHelpWin = pSVData->maHelpData.mpHelpWin;
    if ( pHelpWin )
    {
        Window * pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
        // find out screen area covered by system help window
        Rectangle aInvRect( pHelpWin->GetWindowExtentsRelative( pWindow ) );
        if( pHelpWin->IsVisible() )
            pWindow->Invalidate( aInvRect );
        pSVData->maHelpData.mpHelpWin = NULL;
        pSVData->maHelpData.mbKeyboardHelp = FALSE;
        pHelpWin->Hide();
        delete pHelpWin;
        if( bUpdateHideTime )
            pSVData->maHelpData.mnLastHelpHideTime = Time::GetSystemTicks();
    }
}

// -----------------------------------------------------------------------

void ImplSetHelpWindowPos( Window* pHelpWin, USHORT nHelpWinStyle, USHORT nStyle,
                           const Point& rPos, const Rectangle* pHelpArea )
{
    Point       aPos = rPos;
    Size        aSz = pHelpWin->GetSizePixel();
    Rectangle   aScreenRect = pHelpWin->ImplGetFrameWindow()->GetDesktopRectPixel();
    aPos = pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( aPos );
    // get mouse screen coords
    Point mPos( pHelpWin->GetParent()->ImplGetFrameWindow()->GetPointerPosPixel() );
    mPos = pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( mPos );

    if ( nHelpWinStyle == HELPWINSTYLE_QUICK )
    {
        if ( !(nStyle & QUICKHELP_NOAUTOPOS) )
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
        if ( aPos == mPos )
        {
            aPos.X() += 12;
            aPos.Y() += 16;
        }
    }

    if ( nStyle & QUICKHELP_NOAUTOPOS )
    {
        if ( pHelpArea )
        {
            // convert help area to screen coords
            Rectangle devHelpArea(
                pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( pHelpArea->TopLeft() ),
                pHelpWin->GetParent()->ImplGetFrameWindow()->OutputToAbsoluteScreenPixel( pHelpArea->BottomRight() ) );

            // Welche Position vom Rechteck?
            aPos = devHelpArea.Center();

            if ( nStyle & QUICKHELP_LEFT )
                aPos.X() = devHelpArea.Left();
            else if ( nStyle & QUICKHELP_RIGHT )
                aPos.X() = devHelpArea.Right();

            if ( nStyle & QUICKHELP_TOP )
                aPos.Y() = devHelpArea.Top();
            else if ( nStyle & QUICKHELP_BOTTOM )
                aPos.Y() = devHelpArea.Bottom();
        }

        // Welche Richtung?
        if ( nStyle & QUICKHELP_LEFT )
            ;
        else if ( nStyle & QUICKHELP_RIGHT )
            aPos.X() -= aSz.Width();
        else
            aPos.X() -= aSz.Width()/2;

        if ( nStyle & QUICKHELP_TOP )
            ;
        else if ( nStyle & QUICKHELP_BOTTOM )
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

    // the popup must not appear under the mouse
    // otherwise it would directly be closed due to a focus change...
    Rectangle aHelpRect( aPos, aSz );
    if( aHelpRect.IsInside( mPos ) )
    {
        Point delta(2,2);
        Point pSize( aSz.Width(), aSz.Height() );
        Point pTest( mPos - pSize - delta );
        if( pTest.X() > aScreenRect.Left() &&  pTest.Y() > aScreenRect.Top() )
            aPos = pTest;
        else
            aPos = mPos + delta;
    }

    Window* pWindow = pHelpWin->GetParent()->ImplGetFrameWindow();
    aPos = pWindow->AbsoluteScreenToOutputPixel( aPos );
    pHelpWin->SetPosPixel( aPos );
}
