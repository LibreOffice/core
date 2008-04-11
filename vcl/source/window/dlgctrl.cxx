/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlgctrl.cxx,v $
 * $Revision: 1.26 $
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
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/window.h>

#include <vcl/unohelp.hxx>
#include <com/sun/star/i18n/XCharacterClassification.hpp>

using namespace ::com::sun::star;




// =======================================================================

static Window* ImplGetSubChildWindow( Window* pParent, USHORT n, USHORT& nIndex )
{
    Window*     pTabPage = NULL;
    Window*     pFoundWindow = NULL;
    Window*     pWindow = pParent->GetWindow( WINDOW_FIRSTCHILD );
    Window*     pNextWindow = pWindow;
    while ( pWindow )
    {
        pWindow = pWindow->ImplGetWindow();

        // Unsichtbare und disablte Fenster werden uebersprungen
        if ( pTabPage || pWindow->IsVisible() )
        {
            // Wenn das letzte Control ein TabControl war, wird von
            // diesem die TabPage genommen
            if ( pTabPage )
            {
                pFoundWindow = ImplGetSubChildWindow( pTabPage, n, nIndex );
                pTabPage = NULL;
            }
            else
            {
                pFoundWindow = pWindow;

                // Bei einem TabControl sich die aktuelle TabPage merken,
                // damit diese dann genommen wird
                if ( pWindow->GetType() == WINDOW_TABCONTROL )
                {
                    TabControl* pTabControl = ((TabControl*)pWindow);
                    // Feststellen, ob TabPage Child vom TabControl ist
                    // und auch noch existiert (deshalb durch Vergleich,
                    // indem alle ChildFenster getestet werden). Denn es
                    // kann sein, das TabPages schon in einem Dialog-Dtor
                    // zerstoert wurden, obwohl das TabControl noch
                    // existiert.
                    TabPage* pTempTabPage = pTabControl->GetTabPage( pTabControl->GetCurPageId() );
                    if ( pTempTabPage )
                    {
                        Window* pTempWindow = pTabControl->GetWindow( WINDOW_FIRSTCHILD );
                        while ( pTempWindow )
                        {
                            if ( pTempWindow->ImplGetWindow() == pTempTabPage )
                            {
                                pTabPage = pTempTabPage;
                                break;
                            }
                            pTempWindow = pTempWindow->GetWindow( WINDOW_NEXT );
                        }
                    }
                }
                else if ( pWindow->GetStyle() & WB_DIALOGCONTROL )
                    pFoundWindow = ImplGetSubChildWindow( pWindow, n, nIndex );
            }

            if ( n == nIndex )
                return pFoundWindow;
            nIndex++;
        }

        if ( pTabPage )
            pWindow = pTabPage;
        else
        {
            pWindow = pNextWindow->GetWindow( WINDOW_NEXT );
            pNextWindow = pWindow;
        }
    }

    nIndex--;
    return pFoundWindow;
}

// -----------------------------------------------------------------------

static Window* ImplGetChildWindow( Window* pParent, USHORT n, USHORT& nIndex, BOOL bTestEnable )
{
    nIndex = 0;
    Window* pWindow = ImplGetSubChildWindow( pParent, n, nIndex );
    if ( bTestEnable )
    {
        USHORT n2 = nIndex;
        while ( pWindow && (!pWindow->IsEnabled() || !pWindow->IsInputEnabled()) )
        {
            n2 = nIndex+1;
            nIndex = 0;
            pWindow = ImplGetSubChildWindow( pParent, n2, nIndex );
            if ( nIndex < n2 )
                break;
        }

        if ( (nIndex < n2) && n )
        {
            do
            {
                n--;
                nIndex = 0;
                pWindow = ImplGetSubChildWindow( pParent, n, nIndex );
            }
            while ( pWindow && n && (!pWindow->IsEnabled() || !pWindow->IsInputEnabled()) );
        }
    }
    return pWindow;
}

// -----------------------------------------------------------------------

static Window* ImplGetNextWindow( Window* pParent, USHORT n, USHORT& nIndex, BOOL bTestEnable )
{
    Window* pWindow = ImplGetChildWindow( pParent, n+1, nIndex, bTestEnable );
    if ( n == nIndex )
    {
        n = 0;
        pWindow = ImplGetChildWindow( pParent, n, nIndex, bTestEnable );
    }
    return pWindow;
}

// -----------------------------------------------------------------------

Window* Window::ImplGetDlgWindow( USHORT nIndex, USHORT nType,
                                  USHORT nFormStart, USHORT nFormEnd,
                                  USHORT* pIndex )
{
    DBG_ASSERT( (nIndex >= nFormStart) && (nIndex <= nFormEnd),
                "Window::ImplGetDlgWindow() - nIndex not in Form" );

    Window* pWindow = NULL;
    USHORT  i;
    USHORT  nTemp;
    USHORT  nStartIndex;

    if ( nType == DLGWINDOW_PREV )
    {
        i = nIndex;
        do
        {
            if ( i > nFormStart )
                i--;
            else
                i = nFormEnd;
            pWindow = ImplGetChildWindow( this, i, nTemp, TRUE );
            if ( !pWindow )
                break;
            if ( (i == nTemp) && (pWindow->GetStyle() & WB_TABSTOP) )
                break;
        }
        while ( i != nIndex );
    }
    else
    {
        i = nIndex;
        pWindow = ImplGetChildWindow( this, i, i, (nType == DLGWINDOW_FIRST) );
        if ( pWindow )
        {
            nStartIndex = i;

            if ( nType == DLGWINDOW_NEXT )
            {
                if ( i < nFormEnd )
                {
                    pWindow = ImplGetNextWindow( this, i, i, TRUE );
                    if ( (i > nFormEnd) || (i < nFormStart) )
                        pWindow = ImplGetChildWindow( this, nFormStart, i, TRUE );
                }
                else
                    pWindow = ImplGetChildWindow( this, nFormStart, i, TRUE );
            }

            if ( i <= nFormEnd )
            {
                // 2ten Index mitfuehren, falls alle Controls disablte
                USHORT nStartIndex2 = i;
                USHORT nOldIndex = i+1;

                do
                {
                    if ( pWindow->GetStyle() & WB_TABSTOP )
                        break;
                    if( i == nOldIndex ) // only disabled controls ?
                    {
                        i = nStartIndex2;
                        break;
                    }
                    nOldIndex = i;
                    if ( (i > nFormEnd) || (i < nFormStart) )
                        pWindow = ImplGetChildWindow( this, nFormStart, i, TRUE );
                    else
                        pWindow = ImplGetNextWindow( this, i, i, TRUE );
                }
                while ( (i != nStartIndex) && (i != nStartIndex2) );

                if ( (i == nStartIndex2) &&
                     (!(pWindow->GetStyle() & WB_TABSTOP) || !pWindow->IsEnabled()) )
                    i = nStartIndex;
            }
        }

        if ( nType == DLGWINDOW_FIRST )
        {
            if ( pWindow )
            {
                if ( pWindow->GetType() == WINDOW_TABCONTROL )
                {
                    Window* pNextWindow = ImplGetDlgWindow( i, DLGWINDOW_NEXT );
                    if ( pNextWindow )
                    {
                        if ( pWindow->IsChild( pNextWindow ) )
                            pWindow = pNextWindow;
                    }
                }

                if ( !(pWindow->GetStyle() & WB_TABSTOP) )
                    pWindow = NULL;
            }
        }
    }

    if ( pIndex )
        *pIndex = i;

    return pWindow;
}

// -----------------------------------------------------------------------

static Window* ImplFindAccelWindow( Window* pParent, USHORT& rIndex, xub_Unicode cCharCode,
                                    USHORT nFormStart, USHORT nFormEnd, BOOL bCheckEnable = TRUE )
{
    DBG_ASSERT( (rIndex >= nFormStart) && (rIndex <= nFormEnd),
                "Window::ImplFindAccelWindow() - rIndex not in Form" );

    xub_Unicode cCompareChar;
    USHORT  nStart = rIndex;
    USHORT  i = rIndex;
    int     bSearch = TRUE;
    Window* pWindow;

    // MT: Where can we keep the CharClass?!
    static uno::Reference< i18n::XCharacterClassification > xCharClass;
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILocale();
    cCharCode = xCharClass->toUpper( String(cCharCode), 0, 1, rLocale )[0];

    if ( i < nFormEnd )
        pWindow = ImplGetNextWindow( pParent, i, i, TRUE );
    else
        pWindow = ImplGetChildWindow( pParent, nFormStart, i, TRUE );
    while ( bSearch )
    {
        const XubString aStr = pWindow->GetText();
        USHORT nPos = aStr.Search( '~' );
        while ( nPos != STRING_NOTFOUND )
        {
            cCompareChar = aStr.GetChar( nPos+1 );
            cCompareChar = xCharClass->toUpper( String(cCompareChar), 0, 1, rLocale )[0];
            if ( cCompareChar == cCharCode )
            {
                // Bei Static-Controls auf das naechste Controlm weiterschalten
                if ( (pWindow->GetType() == WINDOW_FIXEDTEXT)   ||
                     (pWindow->GetType() == WINDOW_FIXEDLINE)   ||
                     (pWindow->GetType() == WINDOW_GROUPBOX) )
                    pWindow = pParent->ImplGetDlgWindow( i, DLGWINDOW_NEXT );
                rIndex = i;
                return pWindow;
            }
            nPos = aStr.Search( '~', nPos+1 );
        }

        if ( i == nStart )
            break;

        if ( i < nFormEnd )
            pWindow = ImplGetNextWindow( pParent, i, i, bCheckEnable );
        else
            pWindow = ImplGetChildWindow( pParent, nFormStart, i, bCheckEnable );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static Window* ImplFindDlgCtrlWindow( Window* pParent, Window* pWindow, USHORT& rIndex,
                                      USHORT& rFormStart, USHORT& rFormEnd )
{
    Window* pSWindow;
    Window* pSecondWindow = NULL;
    Window* pTempWindow = NULL;
    USHORT  i;
    USHORT  nSecond_i = 0;
    USHORT  nFormStart = 0;
    USHORT  nSecondFormStart = 0;
    USHORT  nFormEnd;

    // Focus-Fenster in der Child-Liste suchen
    pSWindow = ImplGetChildWindow( pParent, 0, i, FALSE );
    while ( pSWindow )
    {
        if ( pSWindow->ImplGetWindow()->IsDialogControlStart() )
            nFormStart = i;

        // SecondWindow wegen zusammengesetzten Controls wie
        // ComboBoxen und Feldern
        if ( pSWindow->ImplIsWindowOrChild( pWindow ) )
        {
            pSecondWindow = pSWindow;
            nSecond_i = i;
            nSecondFormStart = nFormStart;
            if ( pSWindow == pWindow )
                break;
        }

        pSWindow = ImplGetNextWindow( pParent, i, i, FALSE );
        if ( !i )
            pSWindow = NULL;
    }

    if ( !pSWindow )
    {
        // Fenster nicht gefunden, dann koennen wir auch keine
        // Steuerung uebernehmen
        if ( !pSecondWindow )
            return NULL;
        else
        {
            pSWindow = pSecondWindow;
            i = nSecond_i;
            nFormStart = nSecondFormStart;
        }
    }

    // Start-Daten setzen
    rIndex = i;
    rFormStart = nFormStart;

    // Formularende suchen
    nFormEnd = nFormStart;
    pTempWindow = pSWindow;
    do
    {
        nFormEnd = i;
        pTempWindow = ImplGetNextWindow( pParent, i, i, FALSE );
        if ( !i || (pTempWindow && pTempWindow->ImplGetWindow()->IsDialogControlStart()) )
            break;
    }
    while ( pTempWindow );
    rFormEnd = nFormEnd;

    return pSWindow;
}

// -----------------------------------------------------------------------

void Window::ImplControlFocus( USHORT nFlags )
{
    if ( nFlags & GETFOCUS_MNEMONIC )
    {
        if ( GetType() == WINDOW_RADIOBUTTON )
        {
            if ( !((RadioButton*)this)->IsChecked() )
                ((RadioButton*)this)->ImplCallClick( TRUE, nFlags );
            else
                ImplGrabFocus( nFlags );
        }
        else
        {
            ImplGrabFocus( nFlags );
            if ( nFlags & GETFOCUS_UNIQUEMNEMONIC )
            {
                if ( GetType() == WINDOW_CHECKBOX )
                    ((CheckBox*)this)->ImplCheck();
                else if ( mpWindowImpl->mbPushButton )
                {
                    ((PushButton*)this)->SetPressed( TRUE );
                    ((PushButton*)this)->SetPressed( FALSE );
                    ((PushButton*)this)->Click();
                }
            }
        }
    }
    else
    {
        if ( GetType() == WINDOW_RADIOBUTTON )
        {
            if ( !((RadioButton*)this)->IsChecked() )
                ((RadioButton*)this)->ImplCallClick( TRUE, nFlags );
            else
                ImplGrabFocus( nFlags );
        }
        else
            ImplGrabFocus( nFlags );
    }
}

// -----------------------------------------------------------------------

BOOL Window::ImplDlgCtrl( const KeyEvent& rKEvt, BOOL bKeyInput )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    USHORT  nKeyCode = aKeyCode.GetCode();
    Window* pSWindow;
    Window* pTempWindow;
    Window* pButtonWindow;
    USHORT  i;
    USHORT  iButton;
    USHORT  iButtonStart;
    USHORT  iTemp;
    USHORT  nIndex;
    USHORT  nFormStart;
    USHORT  nFormEnd;
    USHORT  nDlgCtrlFlags;

    // Ohne Focus-Window koennen wir auch keine Steuerung uebernehmen
    Window* pFocusWindow = Application::GetFocusWindow();
    if ( !pFocusWindow || !ImplIsWindowOrChild( pFocusWindow ) )
        return FALSE;

    // Focus-Fenster in der Child-Liste suchen
    pSWindow = ::ImplFindDlgCtrlWindow( this, pFocusWindow,
                                        nIndex, nFormStart, nFormEnd );
    if ( !pSWindow )
        return FALSE;
    i = nIndex;

    nDlgCtrlFlags = 0;
    pTempWindow = pSWindow;
    do
    {
        nDlgCtrlFlags |= pTempWindow->GetDialogControlFlags();
        if ( pTempWindow == this )
            break;
        pTempWindow = pTempWindow->ImplGetParent();
    }
    while ( pTempWindow );

    pButtonWindow = NULL;

    if ( nKeyCode == KEY_RETURN )
    {
        // Wir suchen zuerst nach einem DefPushButton/CancelButton
        pButtonWindow = ImplGetChildWindow( this, nFormStart, iButton, TRUE );
        iButtonStart = iButton;
        while ( pButtonWindow )
        {
            if ( (pButtonWindow->GetStyle() & WB_DEFBUTTON) &&
                 pButtonWindow->mpWindowImpl->mbPushButton )
                break;

            pButtonWindow = ImplGetNextWindow( this, iButton, iButton, TRUE );
            if ( (iButton <= iButtonStart) || (iButton > nFormEnd) )
                pButtonWindow = NULL;
        }

        if ( bKeyInput && !pButtonWindow && (nDlgCtrlFlags & WINDOW_DLGCTRL_RETURN) )
        {
            USHORT  nType;
            USHORT  nGetFocusFlags = GETFOCUS_TAB;
            USHORT  nNewIndex;
            USHORT  iStart;
            if ( aKeyCode.IsShift() )
            {
                nType = DLGWINDOW_PREV;
                nGetFocusFlags |= GETFOCUS_BACKWARD;
            }
            else
            {
                nType = DLGWINDOW_NEXT;
                nGetFocusFlags |= GETFOCUS_FORWARD;
            }
            iStart = i;
            pTempWindow = ImplGetDlgWindow( i, nType, nFormStart, nFormEnd, &nNewIndex );
            while ( pTempWindow && (pTempWindow != pSWindow) )
            {
                if ( !pTempWindow->mpWindowImpl->mbPushButton )
                {
                    // Around-Flag ermitteln
                    if ( nType == DLGWINDOW_PREV )
                    {
                        if ( nNewIndex > iStart )
                            nGetFocusFlags |= GETFOCUS_AROUND;
                    }
                    else
                    {
                        if ( nNewIndex < iStart )
                            nGetFocusFlags |= GETFOCUS_AROUND;
                    }
                    pTempWindow->ImplControlFocus( nGetFocusFlags );
                    return TRUE;
                }
                else
                {
                    i = nNewIndex;
                    pTempWindow = ImplGetDlgWindow( i, nType, nFormStart, nFormEnd, &nNewIndex );
                }
                if ( (i <= iStart) || (i > nFormEnd) )
                    pTempWindow = NULL;
            }
            // Wenn es das gleiche Fenster ist, ein Get/LoseFocus
            // simulieren, falls AROUND ausgewertet wird
            if ( pTempWindow && (pTempWindow == pSWindow) )
            {
                NotifyEvent aNEvt1( EVENT_LOSEFOCUS, pSWindow );
                if ( !ImplCallPreNotify( aNEvt1 ) )
                    pSWindow->LoseFocus();
                pSWindow->mpWindowImpl->mnGetFocusFlags = nGetFocusFlags | GETFOCUS_AROUND;
                NotifyEvent aNEvt2( EVENT_GETFOCUS, pSWindow );
                if ( !ImplCallPreNotify( aNEvt2 ) )
                    pSWindow->GetFocus();
                pSWindow->mpWindowImpl->mnGetFocusFlags = 0;
                return TRUE;
            }
        }
    }
    else if ( nKeyCode == KEY_ESCAPE )
    {
        // Wir suchen zuerst nach einem DefPushButton/CancelButton
        pButtonWindow = ImplGetChildWindow( this, nFormStart, iButton, TRUE );
        iButtonStart = iButton;
        while ( pButtonWindow )
        {
            if ( pButtonWindow->GetType() == WINDOW_CANCELBUTTON )
                break;

            pButtonWindow = ImplGetNextWindow( this, iButton, iButton, TRUE );
            if ( (iButton <= iButtonStart) || (iButton > nFormEnd) )
                pButtonWindow = NULL;
        }

        if ( bKeyInput && mpWindowImpl->mpDlgCtrlDownWindow )
        {
            if ( mpWindowImpl->mpDlgCtrlDownWindow != pButtonWindow )
            {
                ((PushButton*)mpWindowImpl->mpDlgCtrlDownWindow)->SetPressed( FALSE );
                mpWindowImpl->mpDlgCtrlDownWindow = NULL;
                return TRUE;
            }
        }
    }
    else if ( bKeyInput )
    {
        if ( nKeyCode == KEY_TAB )
        {
            // keine Alt-Taste abfangen, wegen Windows
            if ( !aKeyCode.IsMod2() )
            {
                USHORT  nType;
                USHORT  nGetFocusFlags = GETFOCUS_TAB;
                USHORT  nNewIndex;
                BOOL    bFormular = FALSE;

                // Bei Ctrl-Tab erstmal testen, ob zwischen Formularen
                // gesprungen werden soll
                if ( aKeyCode.IsMod1() )
                {
                    // Gruppe suchen
                    Window* pFormularFirstWindow = NULL;
                    Window* pLastFormularFirstWindow = NULL;
                    pTempWindow = ImplGetChildWindow( this, 0, iTemp, FALSE );
                    Window* pPrevFirstFormularFirstWindow = NULL;
                    Window* pFirstFormularFirstWindow = pTempWindow;
                    while ( pTempWindow )
                    {
                        if ( pTempWindow->ImplGetWindow()->IsDialogControlStart() )
                        {
                            if ( iTemp != 0 )
                                bFormular = TRUE;
                            if ( aKeyCode.IsShift() )
                            {
                                if ( iTemp <= nIndex )
                                    pFormularFirstWindow = pPrevFirstFormularFirstWindow;
                                pPrevFirstFormularFirstWindow = pTempWindow;
                            }
                            else
                            {
                                if ( (iTemp > nIndex) && !pFormularFirstWindow )
                                    pFormularFirstWindow = pTempWindow;
                            }
                            pLastFormularFirstWindow = pTempWindow;
                        }

                        pTempWindow = ImplGetNextWindow( this, iTemp, iTemp, FALSE );
                        if ( !iTemp )
                            pTempWindow = NULL;
                    }

                    if ( bFormular )
                    {
                        if ( !pFormularFirstWindow )
                        {
                            if ( aKeyCode.IsShift() )
                                pFormularFirstWindow = pLastFormularFirstWindow;
                            else
                                pFormularFirstWindow = pFirstFormularFirstWindow;
                        }

                        USHORT nFoundFormStart = 0;
                        USHORT nFoundFormEnd = 0;
                        USHORT nTempIndex = 0;
                        if ( ::ImplFindDlgCtrlWindow( this, pFormularFirstWindow, nTempIndex,
                                                      nFoundFormStart, nFoundFormEnd ) )
                        {
                            nTempIndex = nFoundFormStart;
                            pFormularFirstWindow = ImplGetDlgWindow( nTempIndex, DLGWINDOW_FIRST, nFoundFormStart, nFoundFormEnd );
                            if ( pFormularFirstWindow )
                            {
                                pFormularFirstWindow->ImplControlFocus();
                                return TRUE;
                            }
                        }
                    }
                }

                if ( !bFormular )
                {
                    // Only use Ctrl-TAB if it was allowed for the whole
                    // dialog or for the current control (#103667#)
                    if ( !aKeyCode.IsMod1() || (nDlgCtrlFlags & WINDOW_DLGCTRL_MOD1TAB) ||
                        ( pSWindow->GetStyle() & WINDOW_DLGCTRL_MOD1TAB) )
                    {
                        if ( aKeyCode.IsShift() )
                        {
                            nType = DLGWINDOW_PREV;
                            nGetFocusFlags |= GETFOCUS_BACKWARD;
                        }
                        else
                        {
                            nType = DLGWINDOW_NEXT;
                            nGetFocusFlags |= GETFOCUS_FORWARD;
                        }
                        Window* pWindow = ImplGetDlgWindow( i, nType, nFormStart, nFormEnd, &nNewIndex );
                        // Wenn es das gleiche Fenster ist, ein Get/LoseFocus
                        // simulieren, falls AROUND ausgewertet wird
                        if ( pWindow == pSWindow )
                        {
                            NotifyEvent aNEvt1( EVENT_LOSEFOCUS, pSWindow );
                            if ( !ImplCallPreNotify( aNEvt1 ) )
                                pSWindow->LoseFocus();
                            pSWindow->mpWindowImpl->mnGetFocusFlags = nGetFocusFlags | GETFOCUS_AROUND;
                            NotifyEvent aNEvt2( EVENT_GETFOCUS, pSWindow );
                            if ( !ImplCallPreNotify( aNEvt2 ) )
                                pSWindow->GetFocus();
                            pSWindow->mpWindowImpl->mnGetFocusFlags = 0;
                            return TRUE;
                        }
                        else if ( pWindow )
                        {
                            // Around-Flag ermitteln
                            if ( nType == DLGWINDOW_PREV )
                            {
                                if ( nNewIndex > i )
                                    nGetFocusFlags |= GETFOCUS_AROUND;
                            }
                            else
                            {
                                if ( nNewIndex < i )
                                    nGetFocusFlags |= GETFOCUS_AROUND;
                            }
                            pWindow->ImplControlFocus( nGetFocusFlags );
                            return TRUE;
                        }
                    }
                }
            }
        }
        else if ( (nKeyCode == KEY_LEFT) || (nKeyCode == KEY_UP) )
        {
            Window* pWindow = pSWindow;
            WinBits nStyle = pSWindow->GetStyle();
            if ( !(nStyle & WB_GROUP) )
            {
                pWindow = pWindow->GetWindow( WINDOW_PREV );
                while ( pWindow )
                {
                    pWindow = pWindow->ImplGetWindow();

                    nStyle = pWindow->GetStyle();

                    if ( pWindow->IsVisible() && pWindow->IsEnabled() && pWindow->IsInputEnabled() )
                    {
                        if ( pWindow != pSWindow )
                            pWindow->ImplControlFocus( GETFOCUS_CURSOR | GETFOCUS_BACKWARD );
                        return TRUE;
                    }

                    if ( nStyle & WB_GROUP )
                        break;

                    pWindow = pWindow->GetWindow( WINDOW_PREV );
                }
            }
        }
        else if ( (nKeyCode == KEY_RIGHT) || (nKeyCode == KEY_DOWN) )
        {
            Window* pWindow;
            WinBits nStyle;
            pWindow = pSWindow->GetWindow( WINDOW_NEXT );
            while ( pWindow )
            {
                pWindow = pWindow->ImplGetWindow();

                nStyle = pWindow->GetStyle();

                if ( nStyle & WB_GROUP )
                    break;

                if ( pWindow->IsVisible() && pWindow->IsEnabled() && pWindow->IsInputEnabled() )
                {
                    pWindow->ImplControlFocus( GETFOCUS_CURSOR | GETFOCUS_BACKWARD );
                    return TRUE;
                }

                pWindow = pWindow->GetWindow( WINDOW_NEXT );
            }
        }
        else
        {
            xub_Unicode c = rKEvt.GetCharCode();
            if ( c )
            {
                pSWindow = ::ImplFindAccelWindow( this, i, c, nFormStart, nFormEnd );
                if ( pSWindow )
                {
                    USHORT nGetFocusFlags = GETFOCUS_MNEMONIC;
                    if ( pSWindow == ::ImplFindAccelWindow( this, i, c, nFormStart, nFormEnd ) )
                        nGetFocusFlags |= GETFOCUS_UNIQUEMNEMONIC;
                    pSWindow->ImplControlFocus( nGetFocusFlags );
                    return TRUE;
                }
            }
        }
    }

    if ( pButtonWindow && pButtonWindow->IsVisible() && pButtonWindow->IsEnabled() && pButtonWindow->IsInputEnabled() )
    {
        if ( bKeyInput )
        {
            if ( mpWindowImpl->mpDlgCtrlDownWindow && (mpWindowImpl->mpDlgCtrlDownWindow != pButtonWindow) )
            {
                ((PushButton*)mpWindowImpl->mpDlgCtrlDownWindow)->SetPressed( FALSE );
                mpWindowImpl->mpDlgCtrlDownWindow = NULL;
            }

            ((PushButton*)pButtonWindow)->SetPressed( TRUE );
            mpWindowImpl->mpDlgCtrlDownWindow = pButtonWindow;
        }
        else if ( mpWindowImpl->mpDlgCtrlDownWindow == pButtonWindow )
        {
            mpWindowImpl->mpDlgCtrlDownWindow = NULL;
            ((PushButton*)pButtonWindow)->SetPressed( FALSE );
            ((PushButton*)pButtonWindow)->Click();
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

// checks if this window has dialog control
BOOL Window::ImplHasDlgCtrl()
{
    Window* pDlgCtrlParent;
    Window* pDlgCtrl;

    // lookup window for dialog control
    pDlgCtrl = this;
    pDlgCtrlParent = ImplGetParent();
    while ( pDlgCtrlParent &&
            !pDlgCtrlParent->ImplIsOverlapWindow() &&
            ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        pDlgCtrlParent = pDlgCtrlParent->ImplGetParent();

    if ( !pDlgCtrlParent || ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        return FALSE;
    else
        return TRUE;
}

void Window::ImplDlgCtrlNextWindow()
{
    Window* pDlgCtrlParent;
    Window* pDlgCtrl;
    Window* pSWindow;
    USHORT  nIndex;
    USHORT  nFormStart;
    USHORT  nFormEnd;

    // lookup window for dialog control
    pDlgCtrl = this;
    pDlgCtrlParent = ImplGetParent();
    while ( pDlgCtrlParent &&
            !pDlgCtrlParent->ImplIsOverlapWindow() &&
            ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        pDlgCtrlParent = pDlgCtrlParent->ImplGetParent();

    if ( !pDlgCtrlParent || ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        return;

    // lookup window in child list
    pSWindow = ::ImplFindDlgCtrlWindow( pDlgCtrlParent, pDlgCtrl,
                                        nIndex, nFormStart, nFormEnd );
    if ( !pSWindow )
        return;

    Window* pWindow = pDlgCtrlParent->ImplGetDlgWindow( nIndex, DLGWINDOW_NEXT, nFormStart, nFormEnd );
    if ( pWindow && (pWindow != pSWindow) )
        pWindow->ImplControlFocus();
}

// -----------------------------------------------------------------------

static void ImplDlgCtrlUpdateDefButton( Window* pParent, Window* pFocusWindow,
                                        BOOL bGetFocus )
{
    PushButton* pOldDefButton   = NULL;
    PushButton* pNewDefButton   = NULL;
    Window*     pSWindow;
    USHORT      i;
    USHORT      nFormStart;
    USHORT      nFormEnd;

    // Formular suchen
    pSWindow = ::ImplFindDlgCtrlWindow( pParent, pFocusWindow, i, nFormStart, nFormEnd );
    if ( !pSWindow )
    {
        nFormStart = 0;
        nFormEnd = 0xFFFF;
    }

    pSWindow = ImplGetChildWindow( pParent, nFormStart, i, FALSE );
    while ( pSWindow )
    {
        if ( pSWindow->ImplIsPushButton() )
        {
            PushButton* pPushButton = (PushButton*)pSWindow;
            if ( pPushButton->ImplIsDefButton() )
                pOldDefButton = pPushButton;
            if ( pPushButton->HasChildPathFocus() )
                pNewDefButton = pPushButton;
            else if ( !pNewDefButton && (pPushButton->GetStyle() & WB_DEFBUTTON) )
                pNewDefButton = pPushButton;
        }

        pSWindow = ImplGetNextWindow( pParent, i, i, FALSE );
        if ( !i || (i > nFormEnd) )
            pSWindow = NULL;
    }

    if ( !bGetFocus )
    {
        USHORT nDummy;
        Window* pNewFocusWindow = Application::GetFocusWindow();
        if ( !pNewFocusWindow || !pParent->ImplIsWindowOrChild( pNewFocusWindow ) )
            pNewDefButton = NULL;
        else if ( !::ImplFindDlgCtrlWindow( pParent, pNewFocusWindow, i, nDummy, nDummy ) ||
                  (i < nFormStart) || (i > nFormEnd) )
            pNewDefButton = NULL;
    }

    if ( pOldDefButton != pNewDefButton )
    {
        if ( pOldDefButton )
            pOldDefButton->ImplSetDefButton( FALSE );
        if ( pNewDefButton )
            pNewDefButton->ImplSetDefButton( TRUE );
    }
}

// -----------------------------------------------------------------------

void Window::ImplDlgCtrlFocusChanged( Window* pWindow, BOOL bGetFocus )
{
    if ( mpWindowImpl->mpDlgCtrlDownWindow && !bGetFocus )
    {
        ((PushButton*)mpWindowImpl->mpDlgCtrlDownWindow)->SetPressed( FALSE );
        mpWindowImpl->mpDlgCtrlDownWindow = NULL;
    }

    ImplDlgCtrlUpdateDefButton( this, pWindow, bGetFocus );
}

// -----------------------------------------------------------------------

Window* Window::ImplFindDlgCtrlWindow( Window* pWindow )
{
    USHORT  nIndex;
    USHORT  nFormStart;
    USHORT  nFormEnd;

    // Focus-Fenster in der Child-Liste suchen und zurueckgeben
    return ::ImplFindDlgCtrlWindow( this, pWindow, nIndex, nFormStart, nFormEnd );
}


// -----------------------------------------------------------------------

Window* Window::GetParentLabelFor( const Window* ) const
{
    return NULL;
}

// -----------------------------------------------------------------------

Window* Window::GetParentLabeledBy( const Window* ) const
{
    return NULL;
}

// -----------------------------------------------------------------------

static sal_Unicode getAccel( const String& rStr )
{
    sal_Unicode nChar = 0;
    USHORT nPos = 0;
    do
    {
        nPos = rStr.Search( '~', nPos );
        if( nPos != STRING_NOTFOUND && nPos < rStr.Len() )
            nChar = rStr.GetChar( ++nPos );
        else
            nChar = 0;
    } while( nChar == '~' );
    return nChar;
}

Window* Window::GetLabelFor() const
{
    Window* pWindow = NULL;
    Window* pFrameWindow = ImplGetFrameWindow();

    WinBits nFrameStyle = pFrameWindow->GetStyle();
    if( ! ( nFrameStyle & WB_DIALOGCONTROL )
        || ( nFrameStyle & WB_NODIALOGCONTROL )
        )
        return NULL;

    if ( mpWindowImpl->mpRealParent )
        pWindow = mpWindowImpl->mpRealParent->GetParentLabelFor( this );

    if( pWindow )
        return pWindow;

    sal_Unicode nAccel = getAccel( GetText() );
    if( GetType() == WINDOW_FIXEDTEXT       ||
        GetType() == WINDOW_FIXEDLINE       ||
        GetType() == WINDOW_GROUPBOX )
    {
        Window* pSWindow = NULL;
        // get index, form start and form end
        USHORT nIndex, nFormStart, nFormEnd;
        pSWindow = ::ImplFindDlgCtrlWindow( pFrameWindow,
                                           const_cast<Window*>(this),
                                           nIndex,
                                           nFormStart,
                                           nFormEnd );
        if( nAccel )
        {
            // find the accelerated window
            pWindow = ::ImplFindAccelWindow( pFrameWindow,
                                             nIndex,
                                             nAccel,
                                             nFormStart,
                                             nFormEnd,
                                             FALSE );
        }
        else
        {
            // find the next control; if that is a fixed text
            // fixed line or group box, then return NULL
            while( nIndex < nFormEnd )
            {
                nIndex++;
                pSWindow = ::ImplGetChildWindow( pFrameWindow,
                                                 nIndex,
                                                 nIndex,
                                                 FALSE );
                if( pSWindow && pSWindow->IsVisible() && ! (pSWindow->GetStyle() & WB_NOLABEL) )
                {
                    if( pSWindow->GetType() != WINDOW_FIXEDTEXT     &&
                        pSWindow->GetType() != WINDOW_FIXEDLINE     &&
                        pSWindow->GetType() != WINDOW_GROUPBOX )
                    {
                        pWindow = pSWindow;
                    }
                    break;
                }
            }
        }
    }

    return pWindow;
}

// -----------------------------------------------------------------------

Window* Window::GetLabeledBy() const
{
    Window* pWindow = NULL;
    Window* pFrameWindow = ImplGetFrameWindow();

    if ( mpWindowImpl->mpRealParent )
        pWindow = mpWindowImpl->mpRealParent->GetParentLabeledBy( this );

    if( pWindow )
        return pWindow;

    // #i62723#, #104191# checkboxes and radiobuttons are not supposed to have labels
    if( GetType() == WINDOW_CHECKBOX || GetType() == WINDOW_RADIOBUTTON )
        return NULL;

    if( ! ( GetType() == WINDOW_FIXEDTEXT       ||
            GetType() == WINDOW_FIXEDLINE       ||
            GetType() == WINDOW_GROUPBOX ) )
    {
        // search for a control that labels this window
        // a label is considered the last fixed text, fixed line or group box
        // that comes before this control; with the exception of push buttons
        // which are labeled only if the fixed text, fixed line or group box
        // is directly before the control

        // get form start and form end and index of this control
        USHORT nIndex, nFormStart, nFormEnd;
        Window* pSWindow = ::ImplFindDlgCtrlWindow( pFrameWindow,
                                                    const_cast<Window*>(this),
                                                    nIndex,
                                                    nFormStart,
                                                    nFormEnd );
        if( pSWindow && nIndex != nFormStart )
        {
            if( GetType() == WINDOW_PUSHBUTTON      ||
                GetType() == WINDOW_HELPBUTTON      ||
                GetType() == WINDOW_OKBUTTON        ||
                GetType() == WINDOW_CANCELBUTTON )
            {
                nFormStart = nIndex-1;
            }
            for( USHORT nSearchIndex = nIndex-1; nSearchIndex >= nFormStart; nSearchIndex-- )
            {
                USHORT nFoundIndex = 0;
                pSWindow = ::ImplGetChildWindow( pFrameWindow,
                                                 nSearchIndex,
                                                 nFoundIndex,
                                                 FALSE );
                if( pSWindow && pSWindow->IsVisible() &&
                    ! (pSWindow->GetStyle() & WB_NOLABEL) &&
                    ( pSWindow->GetType() == WINDOW_FIXEDTEXT   ||
                      pSWindow->GetType() == WINDOW_FIXEDLINE   ||
                      pSWindow->GetType() == WINDOW_GROUPBOX ) )
                {
                    pWindow = pSWindow;
                    break;
                }
                if( nFoundIndex > nSearchIndex || nSearchIndex == 0 )
                    break;
            }
        }
    }
    return pWindow;
}

// -----------------------------------------------------------------------

KeyEvent Window::GetActivationKey() const
{
    KeyEvent aKeyEvent;

    sal_Unicode nAccel = getAccel( GetText() );
    if( ! nAccel )
    {
        Window* pWindow = GetLabeledBy();
        if( pWindow )
            nAccel = getAccel( pWindow->GetText() );
    }
    if( nAccel )
    {
        USHORT nCode = 0;
        if( nAccel >= 'a' && nAccel <= 'z' )
            nCode = KEY_A + (nAccel-'a');
        else if( nAccel >= 'A' && nAccel <= 'Z' )
            nCode = KEY_A + (nAccel-'A');
        else if( nAccel >= '0' && nAccel <= '9' )
            nCode = KEY_0 + (nAccel-'0');
        KeyCode aKeyCode( nCode, FALSE, FALSE, TRUE );
        aKeyEvent = KeyEvent( nAccel, aKeyCode );
    }
    return aKeyEvent;
}
