/*************************************************************************
 *
 *  $RCSfile: dlgctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
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

#define _SV_DLGCTRL_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <tabpage.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <tabctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <button.hxx>
#endif

#include <unohelp.hxx>

#ifndef _COM_SUN_STAR_LANG_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/lang/XCharacterClassification.hpp>
#endif

using namespace ::com::sun::star;


#pragma hdrstop

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

                do
                {
                    if ( pWindow->GetStyle() & WB_TABSTOP )
                        break;
                    if ( (i >= nFormEnd) || (i < nFormStart) )
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
                                    USHORT nFormStart, USHORT nFormEnd )
{
    DBG_ASSERT( (rIndex >= nFormStart) && (rIndex <= nFormEnd),
                "Window::ImplFindAccelWindow() - rIndex not in Form" );

    xub_Unicode cCompareChar;
    USHORT  nStart = rIndex;
    USHORT  i = rIndex;
    int     bSearch = TRUE;
    Window* pWindow;

    // MT: Where can we keep the CharClass?!
    static uno::Reference< lang::XCharacterClassification > xCharClass;
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetLocale();

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
            pWindow = ImplGetNextWindow( pParent, i, i, TRUE );
        else
            pWindow = ImplGetChildWindow( pParent, nFormStart, i, TRUE );
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
    USHORT  nSecond_i;
    USHORT  nFormStart = 0;
    USHORT  nSecondFormStart;
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
                else if ( mbPushButton )
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
                 pButtonWindow->mbPushButton )
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
                if ( !pTempWindow->mbPushButton )
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
                pSWindow->mnGetFocusFlags = nGetFocusFlags | GETFOCUS_AROUND;
                NotifyEvent aNEvt2( EVENT_GETFOCUS, pSWindow );
                if ( !ImplCallPreNotify( aNEvt2 ) )
                    pSWindow->GetFocus();
                pSWindow->mnGetFocusFlags = 0;
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

        if ( bKeyInput && mpDlgCtrlDownWindow )
        {
            if ( mpDlgCtrlDownWindow != pButtonWindow )
            {
                ((PushButton*)mpDlgCtrlDownWindow)->SetPressed( FALSE );
                mpDlgCtrlDownWindow = NULL;
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
                    // Nur mit Ctrl+Tab zwischen Controls springen, wenn
                    // es erlaubt wurde
                    if ( !aKeyCode.IsMod1() || (nDlgCtrlFlags & WINDOW_DLGCTRL_MOD1TAB) )
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
                            pSWindow->mnGetFocusFlags = nGetFocusFlags | GETFOCUS_AROUND;
                            NotifyEvent aNEvt2( EVENT_GETFOCUS, pSWindow );
                            if ( !ImplCallPreNotify( aNEvt2 ) )
                                pSWindow->GetFocus();
                            pSWindow->mnGetFocusFlags = 0;
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
            if ( mpDlgCtrlDownWindow && (mpDlgCtrlDownWindow != pButtonWindow) )
            {
                ((PushButton*)mpDlgCtrlDownWindow)->SetPressed( FALSE );
                mpDlgCtrlDownWindow = NULL;
            }

            ((PushButton*)pButtonWindow)->SetPressed( TRUE );
            mpDlgCtrlDownWindow = pButtonWindow;
        }
        else if ( mpDlgCtrlDownWindow == pButtonWindow )
        {
            mpDlgCtrlDownWindow = NULL;
            ((PushButton*)pButtonWindow)->SetPressed( FALSE );
            ((PushButton*)pButtonWindow)->Click();
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Window::ImplDlgCtrlNextWindow()
{
    Window* pDlgCtrlParent;
    Window* pDlgCtrl;
    Window* pSWindow;
    USHORT  nIndex;
    USHORT  nFormStart;
    USHORT  nFormEnd;

    // Fenster fuer Dialog-Steuerung suchen
    pDlgCtrl = this;
    pDlgCtrlParent = ImplGetParent();
    while ( pDlgCtrlParent &&
            !pDlgCtrlParent->ImplIsOverlapWindow() &&
            ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        pDlgCtrlParent = pDlgCtrlParent->ImplGetParent();

    if ( !pDlgCtrlParent || ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        return;

    // Fenster in der Child-Liste suchen
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
    if ( mpDlgCtrlDownWindow && !bGetFocus )
    {
        ((PushButton*)mpDlgCtrlDownWindow)->SetPressed( FALSE );
        mpDlgCtrlDownWindow = NULL;
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
