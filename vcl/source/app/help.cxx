/*************************************************************************
 *
 *  $RCSfile: help.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_HELP_CXX

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_HELPWIN_HXX
#include <helpwin.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#pragma hdrstop

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

// -----------------------------------------------------------------------

BOOL Help::Start( ULONG )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Help::Start( const XubString& )
{
    return FALSE;
}

// -----------------------------------------------------------------------

XubString Help::GetHelpText( ULONG nHelpId )
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
}

// =======================================================================

HelpTextWindow::HelpTextWindow( Window* pParent, const XubString& rText, USHORT nHelpWinStyle, USHORT nStyle ) :
    FloatingWindow( pParent->ImplGetFrameWindow(), 0 ),
    maHelpText( rText )
{
    ImplSetMouseTransparent( TRUE );
    mnHelpWinStyle = nHelpWinStyle;
    mnStyle = nStyle;

    EnableAlwaysOnTop();
    EnableSaveBackground();

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetPointFont( rStyleSettings.GetHelpFont() );
    SetTextColor( rStyleSettings.GetHelpTextColor() );
    SetTextAlign( ALIGN_TOP );
    SetBackground( Wallpaper( rStyleSettings.GetHelpColor() ) );
    SetLineColor( COL_BLACK );
    SetFillColor();

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

    const HelpSettings& rHelpSettings = GetSettings().GetHelpSettings();
    maShowTimer.SetTimeoutHdl( LINK( this, HelpTextWindow, TimerHdl ) );
    maHideTimer.SetTimeoutHdl( LINK( this, HelpTextWindow, TimerHdl ) );
    maHideTimer.SetTimeout( rHelpSettings.GetTipTimeout() );
}

// -----------------------------------------------------------------------

HelpTextWindow::~HelpTextWindow()
{
    maShowTimer.Stop();
    maHideTimer.Stop();

    if ( maStatusText.Len() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        pSVData->mpApp->HideHelpStatusText();
    }
}

// -----------------------------------------------------------------------

void HelpTextWindow::ImplShow()
{
    if ( maStatusText.Len() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        pSVData->mpApp->ShowHelpStatusText( maStatusText );
    }
    Show();
    Update();
}

// -----------------------------------------------------------------------

void HelpTextWindow::Paint( const Rectangle& )
{
    // Border zeichen
    // .....

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

    // Umrandung
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
        ImplShow();
        if ( mnHelpWinStyle == HELPWINSTYLE_QUICK )
        {
            // Auto-Hide nicht bei einem Tip-Fenster (ShowTip)
            ImplSVData* pSVData = ImplGetSVData();
            if ( this == pSVData->maHelpData.mpHelpWin )
                maHideTimer.Start();
        }
    }
    else
    {
        Hide();
        ImplDestroyHelpWindow();
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

void HelpTextWindow::RequestHelp( const HelpEvent& rHEvt )
{
    // Nur damit nicht von Window::RequestHelp() ein
    // ShowQuickHelp/ShowBalloonHelp am HelpTextWindow aufgerufen wird.
}

// =======================================================================


void ImplShowHelpWindow( Window* pParent, USHORT nHelpWinStyle, USHORT nStyle,
                         const XubString& rHelpText, const XubString& rStatusText,
                         const Point& rScreenPos, const Rectangle* pHelpArea )
{
    ImplSVData* pSVData = ImplGetSVData();
    HelpTextWindow* pHelpWin = pSVData->maHelpData.mpHelpWin;
    USHORT nDelayMode = HELPDELAY_NORMAL;
    if ( pHelpWin )
    {
        DBG_ASSERT( pHelpWin != pParent, "HelpInHelp ?!" );

        if ( ( pHelpWin->GetHelpText() != rHelpText ) ||
             ( pHelpWin->GetWinStyle() != nHelpWinStyle ) ||
             ( pHelpArea && ( pHelpWin->GetHelpArea() != *pHelpArea ) ) )
        {
            // Fenster wegnehmen wenn kein HelpText oder anderer HelpText oder
            // anderer Modus.
            if ( pHelpWin->IsVisible() )
                nDelayMode = HELPDELAY_SHORT;   // Wenn schon vorher Quick-Hilfe, dann jetzt auch schnell
            pHelpWin = NULL;
            ImplDestroyHelpWindow();
        }
        else if ( !pHelpWin->IsVisible() )
        {
            // Dann die Position der Maus annaehren...
            ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, pHelpArea );
        }
    }

    if ( !pHelpWin && rHelpText.Len() )
    {
        DBG_ASSERT( !pHelpWin, "Noch ein HelpWin ?!" );
        pHelpWin = new HelpTextWindow( pParent, rHelpText, nHelpWinStyle, nStyle );
        pSVData->maHelpData.mpHelpWin = pHelpWin;
        pHelpWin->SetStatusText( rStatusText );
        if ( pHelpArea )
            pHelpWin->SetHelpArea( *pHelpArea );

//      Positionieren...
        Size aSz = pHelpWin->CalcOutSize();
        pHelpWin->SetOutputSizePixel( aSz );
        ImplSetHelpWindowPos( pHelpWin, nHelpWinStyle, nStyle, rScreenPos, pHelpArea );
        // Wenn nicht aus Window::RequestHelp, dann ohne Delay...
        if ( !pSVData->maHelpData.mbRequestingHelp )
            nDelayMode = HELPDELAY_NONE;
        pHelpWin->ShowHelp( nDelayMode );
    }
}

// -----------------------------------------------------------------------

void ImplDestroyHelpWindow( BOOL bUpdate )
{
    ImplSVData* pSVData = ImplGetSVData();
    HelpTextWindow* pHelpWin = pSVData->maHelpData.mpHelpWin;
    if ( pHelpWin )
    {
        pSVData->maHelpData.mpHelpWin = NULL;
        pHelpWin->Hide();
        if ( bUpdate )
           {
            // Update ausloesen, damit ein Paint sofort ausgeloest wird, da
            // wir den Hintergrund nicht sichern
            Window* pFrameWindow = pHelpWin->ImplGetFrameWindow();
            pFrameWindow->ImplUpdateAll();
        }
        delete pHelpWin;
    }
}

// -----------------------------------------------------------------------

void ImplSetHelpWindowPos( Window* pHelpWin, USHORT nHelpWinStyle, USHORT nStyle,
                           const Point& rPos, const Rectangle* pHelpArea )
{
    Point       aPos = rPos;
    Size        aSz = pHelpWin->GetSizePixel();
    Rectangle   aScreenRect = pHelpWin->ImplGetFrameWindow()->GetDesktopRectPixel();
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
        // Wenn es die Maus-Position ist, dann Fenster leicht versetzt
        // anzeigen, damit MousePointer nicht das Hilfe-Fenster verdeckt
        if ( aPos == pHelpWin->OutputToScreenPixel( pHelpWin->GetPointerPosPixel() ) )
        {
            aPos.X() += 12;
            aPos.Y() += 16;
        }
    }

    if ( nStyle & QUICKHELP_NOAUTOPOS )
    {
        if ( pHelpArea )
        {
            // Welche Position vom Rechteck?
            aPos = pHelpArea->Center();

            if ( nStyle & QUICKHELP_LEFT )
                aPos.X() = pHelpArea->Left();
            else if ( nStyle & QUICKHELP_RIGHT )
                aPos.X() = pHelpArea->Right();

            if ( nStyle & QUICKHELP_TOP )
                aPos.Y() = pHelpArea->Top();
            else if ( nStyle & QUICKHELP_BOTTOM )
                aPos.Y() = pHelpArea->Bottom();
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

    pHelpWin->SetPosPixel( aPos );
}
