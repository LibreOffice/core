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

#include <tools/debug.hxx>

#include <svdata.hxx>
#include <window.h>

#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/button.hxx>
#include <vcl/settings.hxx>
#include <vcl/unohelp.hxx>

#include <com/sun/star/i18n/XCharacterClassification.hpp>

using namespace ::com::sun::star;

static bool ImplHasIndirectTabParent( Window* pWindow )
{
    // The window has inderect tab parent if it is included in tab hierarchy
    // of the indirect parent window

    Window* pNonLayoutParent = getNonLayoutParent(pWindow);
    return ( pNonLayoutParent
          && ( pNonLayoutParent->ImplGetWindow()->GetStyle() & WB_CHILDDLGCTRL ) );
}

static Window* ImplGetTopParentOfTabHierarchy( Window* pParent )
{
    // The method allows to find the most close parent containing all the
    // window from the current tab-hierarchy
    // The direct parent should be provided as a parameter here

    Window* pResult = pParent;

    if ( pResult )
    {
        Window* pNonLayoutParent = getNonLayoutParent(pResult);
        while ( pNonLayoutParent && ( pResult->ImplGetWindow()->GetStyle() & WB_CHILDDLGCTRL ) )
        {
            pResult = pNonLayoutParent;
            pNonLayoutParent = getNonLayoutParent(pResult);
        }
    }

    return pResult;
}

static Window* ImplGetSubChildWindow( Window* pParent, sal_uInt16 n, sal_uInt16& nIndex )
{
    Window*     pTabPage = NULL;
    Window*     pFoundWindow = NULL;

    Window*     pWindow = firstLogicalChildOfParent(pParent);
    Window*     pNextWindow = pWindow;
    while ( pWindow )
    {
        pWindow = pWindow->ImplGetWindow();

        // skip invisible and disabled windows
        if ( pTabPage || isVisibleInLayout(pWindow) )
        {
            // if the last control was a TabControl, take its TabPage
            if ( pTabPage )
            {
                pFoundWindow = ImplGetSubChildWindow( pTabPage, n, nIndex );
                pTabPage = NULL;
            }
            else
            {
                pFoundWindow = pWindow;

                // for a TabControl, remember the current TabPage for later use
                if ( pWindow->GetType() == WINDOW_TABCONTROL )
                {
                    TabControl* pTabControl = ((TabControl*)pWindow);
                    // Check if the TabPage is a Child of the TabControl and still exists (by
                    // walking all child windows); because it could be that the TabPage has been
                    // destroyed already by a Dialog-Dtor, event that the TabControl still exists.
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
                            pTempWindow = nextLogicalChildOfParent(pTabControl, pTempWindow);
                        }
                    }
                }
                else if ( ( pWindow->GetStyle() & WB_DIALOGCONTROL )
                       || ( pWindow->GetStyle() & WB_CHILDDLGCTRL ) )
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
            pWindow = nextLogicalChildOfParent(pParent, pNextWindow);
            pNextWindow = pWindow;
        }
    }

    nIndex--;
    return pFoundWindow;
}

static Window* ImplGetChildWindow( Window* pParent, sal_uInt16 n, sal_uInt16& nIndex, bool bTestEnable )
{
    pParent = ImplGetTopParentOfTabHierarchy( pParent );

    nIndex = 0;
    Window* pWindow = ImplGetSubChildWindow( pParent, n, nIndex );
    if ( bTestEnable )
    {
        sal_uInt16 n2 = nIndex;
        while ( pWindow && (!isEnabledInLayout(pWindow) || !pWindow->IsInputEnabled()) )
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
            while ( pWindow && n && (!isEnabledInLayout(pWindow) || !pWindow->IsInputEnabled()) );
        }
    }
    return pWindow;
}

static Window* ImplGetNextWindow( Window* pParent, sal_uInt16 n, sal_uInt16& nIndex, bool bTestEnable )
{
    Window* pWindow = ImplGetChildWindow( pParent, n+1, nIndex, bTestEnable );
    if ( n == nIndex )
    {
        n = 0;
        pWindow = ImplGetChildWindow( pParent, n, nIndex, bTestEnable );
    }
    return pWindow;
}

Window* Window::ImplGetDlgWindow( sal_uInt16 nIndex, sal_uInt16 nType,
                                  sal_uInt16 nFormStart, sal_uInt16 nFormEnd,
                                  sal_uInt16* pIndex )
{
    DBG_ASSERT( (nIndex >= nFormStart) && (nIndex <= nFormEnd),
                "Window::ImplGetDlgWindow() - nIndex not in Form" );

    Window* pWindow = NULL;
    sal_uInt16  i;
    sal_uInt16  nTemp;
    sal_uInt16  nStartIndex;

    if ( nType == DLGWINDOW_PREV )
    {
        i = nIndex;
        do
        {
            if ( i > nFormStart )
                i--;
            else
                i = nFormEnd;
            pWindow = ImplGetChildWindow( this, i, nTemp, true );
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
                    pWindow = ImplGetNextWindow( this, i, i, true );
                    if ( (i > nFormEnd) || (i < nFormStart) )
                        pWindow = ImplGetChildWindow( this, nFormStart, i, true );
                }
                else
                    pWindow = ImplGetChildWindow( this, nFormStart, i, true );
            }

            if ( i <= nFormEnd )
            {
                // carry the 2nd index, in case all controls are disabled
                sal_uInt16 nStartIndex2 = i;
                sal_uInt16 nOldIndex = i+1;

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
                        pWindow = ImplGetChildWindow( this, nFormStart, i, true );
                    else
                        pWindow = ImplGetNextWindow( this, i, i, true );
                }
                while ( (i != nStartIndex) && (i != nStartIndex2) );

                if ( (i == nStartIndex2) &&
                     (!(pWindow->GetStyle() & WB_TABSTOP) || !isEnabledInLayout(pWindow)) )
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

static Window* ImplFindDlgCtrlWindow( Window* pParent, Window* pWindow, sal_uInt16& rIndex,
                                      sal_uInt16& rFormStart, sal_uInt16& rFormEnd )
{
    Window* pSWindow;
    Window* pSecondWindow = NULL;
    Window* pTempWindow = NULL;
    sal_uInt16  i;
    sal_uInt16  nSecond_i = 0;
    sal_uInt16  nFormStart = 0;
    sal_uInt16  nSecondFormStart = 0;
    sal_uInt16  nFormEnd;

    // find focus window in the child list
    Window* pFirstChildWindow = pSWindow = ImplGetChildWindow( pParent, 0, i, false );

    if( pWindow == NULL )
        pWindow = pSWindow;

    while ( pSWindow )
    {
        // the DialogControlStart mark is only accepted for the direct children
        if ( !ImplHasIndirectTabParent( pSWindow )
          && pSWindow->ImplGetWindow()->IsDialogControlStart() )
            nFormStart = i;

        // SecondWindow for composit controls like ComboBoxes and arrays
        if ( pSWindow->ImplIsWindowOrChild( pWindow ) )
        {
            pSecondWindow = pSWindow;
            nSecond_i = i;
            nSecondFormStart = nFormStart;
            if ( pSWindow == pWindow )
                break;
        }

        pSWindow = ImplGetNextWindow( pParent, i, i, false );
        if ( !i )
            pSWindow = NULL;
    }

    if ( !pSWindow )
    {
        // Window not found; we cannot handle it
        if ( !pSecondWindow )
            return NULL;
        else
        {
            pSWindow = pSecondWindow;
            i = nSecond_i;
            nFormStart = nSecondFormStart;
        }
    }

    // initialize
    rIndex = i;
    rFormStart = nFormStart;

    // find end of template
    nFormEnd = nFormStart;
    pTempWindow = pSWindow;
    sal_Int32 nIteration = 0;
    do
    {
        nFormEnd = i;
        pTempWindow = ImplGetNextWindow( pParent, i, i, false );

        // the DialogControlStart mark is only accepted for the direct children
        if ( !i
          || ( pTempWindow && !ImplHasIndirectTabParent( pTempWindow )
               && pTempWindow->ImplGetWindow()->IsDialogControlStart() ) )
            break;

        if ( pTempWindow && pTempWindow == pFirstChildWindow )
        {
            // It is possible to go through the begin of hierarchy once
            // while looking for DialogControlStart mark.
            // If it happens second time, it looks like an endless loop,
            // that should be impossible, but just for the case...
            nIteration++;
            if ( nIteration >= 2 )
            {
                // this is an unexpected scenario
                DBG_ASSERT( false, "It seems to be an endless loop!" );
                rFormStart = 0;
                break;
            }
        }
    }
    while ( pTempWindow );
    rFormEnd = nFormEnd;

    return pSWindow;
}

static Window* ImplFindAccelWindow( Window* pParent, sal_uInt16& rIndex, sal_Unicode cCharCode,
                                    sal_uInt16 nFormStart, sal_uInt16 nFormEnd, bool bCheckEnable = true )
{
    DBG_ASSERT( (rIndex >= nFormStart) && (rIndex <= nFormEnd),
                "Window::ImplFindAccelWindow() - rIndex not in Form" );

    sal_Unicode cCompareChar;
    sal_uInt16  nStart = rIndex;
    sal_uInt16  i = rIndex;
    Window* pWindow;

    // MT: Where can we keep the CharClass?!
    static uno::Reference< i18n::XCharacterClassification > xCharClass;
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();

    const ::com::sun::star::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    cCharCode = xCharClass->toUpper( OUString(cCharCode), 0, 1, rLocale )[0];

    if ( i < nFormEnd )
        pWindow = ImplGetNextWindow( pParent, i, i, true );
    else
        pWindow = ImplGetChildWindow( pParent, nFormStart, i, true );
    while( pWindow )
    {
        const OUString aStr = pWindow->GetText();
        sal_Int32 nPos = aStr.indexOf( '~' );
        while (nPos != -1)
        {
            cCompareChar = aStr[nPos+1];
            cCompareChar = xCharClass->toUpper( OUString(cCompareChar), 0, 1, rLocale )[0];
            if ( cCompareChar == cCharCode )
            {
                if (pWindow->GetType() == WINDOW_FIXEDTEXT)
                {
                    FixedText *pFixedText = static_cast<FixedText*>(pWindow);
                    Window *pMnemonicWidget = pFixedText->get_mnemonic_widget();
                    SAL_WARN_IF(isContainerWindow(pFixedText->GetParent()) && !pMnemonicWidget,
                        "vcl.a11y", "label missing mnemonic_widget?");
                    if (pMnemonicWidget)
                        return pMnemonicWidget;
                }

                // skip Static-Controls
                if ( (pWindow->GetType() == WINDOW_FIXEDTEXT)   ||
                     (pWindow->GetType() == WINDOW_FIXEDLINE)   ||
                     (pWindow->GetType() == WINDOW_GROUPBOX) )
                    pWindow = pParent->ImplGetDlgWindow( i, DLGWINDOW_NEXT );
                rIndex = i;
                return pWindow;
            }
            nPos = aStr.indexOf( '~', nPos+1 );
        }

        // #i93011# it would have made sense to have this really recursive
        // right from the start. However this would cause unpredictable side effects now
        // so instead we have a style bit for some child windows, that want their
        // children checked for accelerators
        if( (pWindow->GetStyle() & WB_CHILDDLGCTRL) != 0 )
        {
            sal_uInt16  nChildIndex;
            sal_uInt16  nChildFormStart;
            sal_uInt16  nChildFormEnd;

            // get form start and end
            ::ImplFindDlgCtrlWindow( pWindow, NULL,
                                     nChildIndex, nChildFormStart, nChildFormEnd );
            Window* pAccelWin = ImplFindAccelWindow( pWindow, nChildIndex, cCharCode,
                                                     nChildFormStart, nChildFormEnd,
                                                     bCheckEnable );
            if( pAccelWin )
                return pAccelWin;
        }

        if ( i == nStart )
            break;

        if ( i < nFormEnd )
        {
            pWindow = ImplGetNextWindow( pParent, i, i, bCheckEnable );
            if( ! pWindow )
                pWindow = ImplGetChildWindow( pParent, nFormStart, i, bCheckEnable );
        }
        else
            pWindow = ImplGetChildWindow( pParent, nFormStart, i, bCheckEnable );
    }

    return NULL;
}

void Window::ImplControlFocus( sal_uInt16 nFlags )
{
    if ( nFlags & GETFOCUS_MNEMONIC )
    {
        if ( GetType() == WINDOW_RADIOBUTTON )
        {
            if ( !((RadioButton*)this)->IsChecked() )
                ((RadioButton*)this)->ImplCallClick( true, nFlags );
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
                    ((PushButton*)this)->SetPressed( true );
                    ((PushButton*)this)->SetPressed( false );
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
                ((RadioButton*)this)->ImplCallClick( true, nFlags );
            else
                ImplGrabFocus( nFlags );
        }
        else
            ImplGrabFocus( nFlags );
    }
}

namespace
{
    bool isSuitableDestination(Window *pWindow)
    {
        return (pWindow && isVisibleInLayout(pWindow) &&
                isEnabledInLayout(pWindow) && pWindow->IsInputEnabled() &&
                //Pure window shouldn't get window after controls such as
                //buttons.
                (pWindow->GetType() != WINDOW_WINDOW && pWindow->GetType() != WINDOW_SYSWINDOW &&
                  pWindow->GetType() != WINDOW_WORKWINDOW && pWindow->GetType() != WINDOW_CONTROL)
               );
    }

    bool focusNextInGroup(std::vector<RadioButton*>::iterator aStart, std::vector<RadioButton*> &rGroup)
    {
        std::vector<RadioButton*>::iterator aI(aStart);

        if (aStart != rGroup.end())
            ++aI;

        for (; aI != rGroup.end(); ++aI)
        {
            Window *pWindow = *aI;

            if (isSuitableDestination(pWindow))
            {
                pWindow->ImplControlFocus( GETFOCUS_CURSOR | GETFOCUS_FORWARD );
                return true;
            }
        }

        for (aI = rGroup.begin(); aI != aStart; ++aI)
        {
            Window *pWindow = *aI;

            if (isSuitableDestination(pWindow))
            {
                pWindow->ImplControlFocus( GETFOCUS_CURSOR | GETFOCUS_FORWARD );
                return true;
            }
        }

        return false;
    }

    bool nextInGroup(RadioButton *pSourceWindow, bool bBackward)
    {
        std::vector<RadioButton*> aGroup(pSourceWindow->GetRadioButtonGroup(true));

        if (aGroup.size() == 1) //only one button in group
            return false;

        if (bBackward)
            std::reverse(aGroup.begin(), aGroup.end());

        std::vector<RadioButton*>::iterator aStart(std::find(aGroup.begin(), aGroup.end(), pSourceWindow));

        assert(aStart != aGroup.end());

        return focusNextInGroup(aStart, aGroup);
    }
}

bool Window::ImplDlgCtrl( const KeyEvent& rKEvt, bool bKeyInput )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16  nKeyCode = aKeyCode.GetCode();
    Window* pSWindow;
    Window* pTempWindow;
    Window* pButtonWindow;
    sal_uInt16  i;
    sal_uInt16  iButton;
    sal_uInt16  iButtonStart;
    sal_uInt16  iTemp;
    sal_uInt16  nIndex;
    sal_uInt16  nFormStart;
    sal_uInt16  nFormEnd;
    sal_uInt16  nDlgCtrlFlags;

    // Ohne Focus-Window koennen wir auch keine Steuerung uebernehmen
    // w/o a Focus-Window we cannot take control
    Window* pFocusWindow = Application::GetFocusWindow();
    if ( !pFocusWindow || !ImplIsWindowOrChild( pFocusWindow ) )
        return false;

    // find Focus-Window in the child list
    pSWindow = ::ImplFindDlgCtrlWindow( this, pFocusWindow,
                                        nIndex, nFormStart, nFormEnd );
    if ( !pSWindow )
        return false;
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
        // search first for a DefPushButton/CancelButton
        pButtonWindow = ImplGetChildWindow( this, nFormStart, iButton, true );
        iButtonStart = iButton;
        while ( pButtonWindow )
        {
            if ( (pButtonWindow->GetStyle() & WB_DEFBUTTON) &&
                 pButtonWindow->mpWindowImpl->mbPushButton )
                break;

            pButtonWindow = ImplGetNextWindow( this, iButton, iButton, true );
            if ( (iButton <= iButtonStart) || (iButton > nFormEnd) )
                pButtonWindow = NULL;
        }

        if ( bKeyInput && !pButtonWindow && (nDlgCtrlFlags & WINDOW_DLGCTRL_RETURN) )
        {
            sal_uInt16  nType;
            sal_uInt16  nGetFocusFlags = GETFOCUS_TAB;
            sal_uInt16  nNewIndex;
            sal_uInt16  iStart;
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
                    // get Around-Flag
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
                    return true;
                }
                else
                {
                    i = nNewIndex;
                    pTempWindow = ImplGetDlgWindow( i, nType, nFormStart, nFormEnd, &nNewIndex );
                }
                if ( (i <= iStart) || (i > nFormEnd) )
                    pTempWindow = NULL;
            }
            // if this is the same window, simulate a Get/LoseFocus,
            // in case AROUND is being processed
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
                return true;
            }
        }
    }
    else if ( nKeyCode == KEY_ESCAPE )
    {
        // search first for a DefPushButton/CancelButton
        pButtonWindow = ImplGetChildWindow( this, nFormStart, iButton, true );
        iButtonStart = iButton;
        while ( pButtonWindow )
        {
            if ( pButtonWindow->GetType() == WINDOW_CANCELBUTTON )
                break;

            pButtonWindow = ImplGetNextWindow( this, iButton, iButton, true );
            if ( (iButton <= iButtonStart) || (iButton > nFormEnd) )
                pButtonWindow = NULL;
        }

        if ( bKeyInput && mpWindowImpl->mpDlgCtrlDownWindow )
        {
            if ( mpWindowImpl->mpDlgCtrlDownWindow != pButtonWindow )
            {
                ((PushButton*)mpWindowImpl->mpDlgCtrlDownWindow)->SetPressed( false );
                mpWindowImpl->mpDlgCtrlDownWindow = NULL;
                return true;
            }
        }
    }
    else if ( bKeyInput )
    {
        if ( nKeyCode == KEY_TAB )
        {
            // do not skip Alt key, for MS Windows
            if ( !aKeyCode.IsMod2() )
            {
                sal_uInt16  nType;
                sal_uInt16  nGetFocusFlags = GETFOCUS_TAB;
                sal_uInt16  nNewIndex;
                bool        bFormular = false;

                // for Ctrl-Tab check if we want to jump to next template
                if ( aKeyCode.IsMod1() )
                {
                    // search group
                    Window* pFormularFirstWindow = NULL;
                    Window* pLastFormularFirstWindow = NULL;
                    pTempWindow = ImplGetChildWindow( this, 0, iTemp, false );
                    Window* pPrevFirstFormularFirstWindow = NULL;
                    Window* pFirstFormularFirstWindow = pTempWindow;
                    while ( pTempWindow )
                    {
                        if ( pTempWindow->ImplGetWindow()->IsDialogControlStart() )
                        {
                            if ( iTemp != 0 )
                                bFormular = true;
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

                        pTempWindow = ImplGetNextWindow( this, iTemp, iTemp, false );
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

                        sal_uInt16 nFoundFormStart = 0;
                        sal_uInt16 nFoundFormEnd = 0;
                        sal_uInt16 nTempIndex = 0;
                        if ( ::ImplFindDlgCtrlWindow( this, pFormularFirstWindow, nTempIndex,
                                                      nFoundFormStart, nFoundFormEnd ) )
                        {
                            nTempIndex = nFoundFormStart;
                            pFormularFirstWindow = ImplGetDlgWindow( nTempIndex, DLGWINDOW_FIRST, nFoundFormStart, nFoundFormEnd );
                            if ( pFormularFirstWindow )
                            {
                                pFormularFirstWindow->ImplControlFocus();
                                return true;
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
                        // if this is the same window, simulate a Get/LoseFocus,
                        // in case AROUND is being processed
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
                            return true;
                        }
                        else if ( pWindow )
                        {
                            // get Around-Flag
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
                            return true;
                        }
                    }
                }
            }
        }
        else if ( (nKeyCode == KEY_LEFT) || (nKeyCode == KEY_UP) )
        {
            if (pSWindow->GetType() == WINDOW_RADIOBUTTON)
                return nextInGroup(static_cast<RadioButton*>(pSWindow), true);
            else
            {
                WinBits nStyle = pSWindow->GetStyle();
                if ( !(nStyle & WB_GROUP) )
                {
                    Window* pWindow = prevLogicalChildOfParent(this, pSWindow);
                    while ( pWindow )
                    {
                        pWindow = pWindow->ImplGetWindow();

                        nStyle = pWindow->GetStyle();

                        if (isSuitableDestination(pWindow))
                        {
                            if ( pWindow != pSWindow )
                                pWindow->ImplControlFocus( GETFOCUS_CURSOR | GETFOCUS_BACKWARD );
                            return true;
                        }

                        if ( nStyle & WB_GROUP )
                            break;

                        pWindow = prevLogicalChildOfParent(this, pWindow);
                    }
                }
            }
        }
        else if ( (nKeyCode == KEY_RIGHT) || (nKeyCode == KEY_DOWN) )
        {
            if (pSWindow->GetType() == WINDOW_RADIOBUTTON)
                return nextInGroup(static_cast<RadioButton*>(pSWindow), false);
            else
            {
                Window* pWindow = nextLogicalChildOfParent(this, pSWindow);
                while ( pWindow )
                {
                    pWindow = pWindow->ImplGetWindow();

                    WinBits nStyle = pWindow->GetStyle();

                    if ( nStyle & WB_GROUP )
                        break;

                    if (isSuitableDestination(pWindow))
                    {
                        pWindow->ImplControlFocus( GETFOCUS_CURSOR | GETFOCUS_BACKWARD );
                        return true;
                    }

                    pWindow = nextLogicalChildOfParent(this, pWindow);
                }
            }
        }
        else
        {
            sal_Unicode c = rKEvt.GetCharCode();
            if ( c )
            {
                pSWindow = ::ImplFindAccelWindow( this, i, c, nFormStart, nFormEnd );
                if ( pSWindow )
                {
                    sal_uInt16 nGetFocusFlags = GETFOCUS_MNEMONIC;
                    if ( pSWindow == ::ImplFindAccelWindow( this, i, c, nFormStart, nFormEnd ) )
                        nGetFocusFlags |= GETFOCUS_UNIQUEMNEMONIC;
                    pSWindow->ImplControlFocus( nGetFocusFlags );
                    return true;
                }
            }
        }
    }

    if (isSuitableDestination(pButtonWindow))
    {
        if ( bKeyInput )
        {
            if ( mpWindowImpl->mpDlgCtrlDownWindow && (mpWindowImpl->mpDlgCtrlDownWindow != pButtonWindow) )
            {
                ((PushButton*)mpWindowImpl->mpDlgCtrlDownWindow)->SetPressed( false );
                mpWindowImpl->mpDlgCtrlDownWindow = NULL;
            }

            ((PushButton*)pButtonWindow)->SetPressed( true );
            mpWindowImpl->mpDlgCtrlDownWindow = pButtonWindow;
        }
        else if ( mpWindowImpl->mpDlgCtrlDownWindow == pButtonWindow )
        {
            mpWindowImpl->mpDlgCtrlDownWindow = NULL;
            ((PushButton*)pButtonWindow)->SetPressed( false );
            ((PushButton*)pButtonWindow)->Click();
        }

        return true;
    }

    return false;
}

// checks if this window has dialog control
bool Window::ImplHasDlgCtrl()
{
    Window* pDlgCtrlParent;

    // lookup window for dialog control
    pDlgCtrlParent = ImplGetParent();
    while ( pDlgCtrlParent &&
            !pDlgCtrlParent->ImplIsOverlapWindow() &&
            ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        pDlgCtrlParent = pDlgCtrlParent->ImplGetParent();

    if ( !pDlgCtrlParent || ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        return false;
    else
        return true;
}

void Window::ImplDlgCtrlNextWindow()
{
    Window* pDlgCtrlParent;
    Window* pDlgCtrl;
    Window* pSWindow;
    sal_uInt16  nIndex;
    sal_uInt16  nFormStart;
    sal_uInt16  nFormEnd;

    // lookup window for dialog control
    pDlgCtrl = this;
    pDlgCtrlParent = ImplGetParent();
    while ( pDlgCtrlParent &&
            !pDlgCtrlParent->ImplIsOverlapWindow() &&
            ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        pDlgCtrlParent = pDlgCtrlParent->ImplGetParent();

if ( !pDlgCtrlParent || (GetStyle() & WB_NODIALOGCONTROL) || ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
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

static void ImplDlgCtrlUpdateDefButton( Window* pParent, Window* pFocusWindow,
                                        bool bGetFocus )
{
    PushButton* pOldDefButton   = NULL;
    PushButton* pNewDefButton   = NULL;
    Window*     pSWindow;
    sal_uInt16      i;
    sal_uInt16      nFormStart;
    sal_uInt16      nFormEnd;

    // find template
    pSWindow = ::ImplFindDlgCtrlWindow( pParent, pFocusWindow, i, nFormStart, nFormEnd );
    if ( !pSWindow )
    {
        nFormStart = 0;
        nFormEnd = 0xFFFF;
    }

    pSWindow = ImplGetChildWindow( pParent, nFormStart, i, false );
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

        pSWindow = ImplGetNextWindow( pParent, i, i, false );
        if ( !i || (i > nFormEnd) )
            pSWindow = NULL;
    }

    if ( !bGetFocus )
    {
        sal_uInt16 nDummy;
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
            pOldDefButton->ImplSetDefButton( false );
        if ( pNewDefButton )
            pNewDefButton->ImplSetDefButton( true );
    }
}

void Window::ImplDlgCtrlFocusChanged( Window* pWindow, bool bGetFocus )
{
    if ( mpWindowImpl->mpDlgCtrlDownWindow && !bGetFocus )
    {
        ((PushButton*)mpWindowImpl->mpDlgCtrlDownWindow)->SetPressed( false );
        mpWindowImpl->mpDlgCtrlDownWindow = NULL;
    }

    ImplDlgCtrlUpdateDefButton( this, pWindow, bGetFocus );
}

Window* Window::ImplFindDlgCtrlWindow( Window* pWindow )
{
    sal_uInt16  nIndex;
    sal_uInt16  nFormStart;
    sal_uInt16  nFormEnd;

    // find Focus-Window in the Child-List and return
    return ::ImplFindDlgCtrlWindow( this, pWindow, nIndex, nFormStart, nFormEnd );
}

Window* Window::GetParentLabelFor( const Window* ) const
{
    return NULL;
}

Window* Window::GetParentLabeledBy( const Window* ) const
{
    return NULL;
}

static sal_Unicode getAccel( const OUString& rStr )
{
    sal_Unicode nChar = 0;
    sal_Int32 nPos = 0;
    do
    {
        nPos = rStr.indexOf( '~', nPos );
        if( nPos != -1 && nPos < rStr.getLength() )
            nChar = rStr[ ++nPos ];
        else
            nChar = 0;
    } while( nChar == '~' );
    return nChar;
}

static Window* ImplGetLabelFor( Window* pFrameWindow, WindowType nMyType, Window* pLabel, sal_Unicode nAccel )
{
    Window* pWindow = NULL;

    if( nMyType == WINDOW_FIXEDTEXT     ||
        nMyType == WINDOW_FIXEDLINE     ||
        nMyType == WINDOW_GROUPBOX )
    {
        // #i100833# MT 2010/02: Group box and fixed lines can also lable a fixed text.
        // See tools/options/print for example.
        bool bThisIsAGroupControl = (nMyType == WINDOW_GROUPBOX) || (nMyType == WINDOW_FIXEDLINE);
        // get index, form start and form end
        sal_uInt16 nIndex=0, nFormStart=0, nFormEnd=0;
        ::ImplFindDlgCtrlWindow( pFrameWindow,
                                           pLabel,
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
                                             false );
        }
        else
        {
            // find the next control; if that is a fixed text
            // fixed line or group box, then return NULL
            while( nIndex < nFormEnd )
            {
                nIndex++;
                Window* pSWindow = ::ImplGetChildWindow( pFrameWindow,
                                                 nIndex,
                                                 nIndex,
                                                 false );
                if( pSWindow && isVisibleInLayout(pSWindow) && ! (pSWindow->GetStyle() & WB_NOLABEL) )
                {
                    WindowType nType = pSWindow->GetType();
                    if( nType != WINDOW_FIXEDTEXT   &&
                        nType != WINDOW_FIXEDLINE   &&
                        nType != WINDOW_GROUPBOX )
                    {
                        pWindow = pSWindow;
                    }
                    else if( bThisIsAGroupControl && ( nType == WINDOW_FIXEDTEXT ) )
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

Window* Window::getLegacyNonLayoutAccessibleRelationLabelFor() const
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

    pWindow = ImplGetLabelFor( pFrameWindow, GetType(), const_cast<Window*>(this), nAccel );
    if( ! pWindow && mpWindowImpl->mpRealParent )
        pWindow = ImplGetLabelFor( mpWindowImpl->mpRealParent, GetType(), const_cast<Window*>(this), nAccel );
    return pWindow;
}

static Window* ImplGetLabeledBy( Window* pFrameWindow, WindowType nMyType, Window* pLabeled )
{
    Window* pWindow = NULL;
    if ( (nMyType != WINDOW_GROUPBOX) && (nMyType != WINDOW_FIXEDLINE) )
    {
        // search for a control that labels this window
        // a label is considered the last fixed text, fixed line or group box
        // that comes before this control; with the exception of push buttons
        // which are labeled only if the fixed text, fixed line or group box
        // is directly before the control

        // get form start and form end and index of this control
        sal_uInt16 nIndex, nFormStart, nFormEnd;
        Window* pSWindow = ::ImplFindDlgCtrlWindow( pFrameWindow,
                                                    pLabeled,
                                                    nIndex,
                                                    nFormStart,
                                                    nFormEnd );
        if( pSWindow && nIndex != nFormStart )
        {
            if( nMyType == WINDOW_PUSHBUTTON        ||
                nMyType == WINDOW_HELPBUTTON        ||
                nMyType == WINDOW_OKBUTTON      ||
                nMyType == WINDOW_CANCELBUTTON )
            {
                nFormStart = nIndex-1;
            }
            for( sal_uInt16 nSearchIndex = nIndex-1; nSearchIndex >= nFormStart; nSearchIndex-- )
            {
                sal_uInt16 nFoundIndex = 0;
                pSWindow = ::ImplGetChildWindow( pFrameWindow,
                                                 nSearchIndex,
                                                 nFoundIndex,
                                                 false );
                if( pSWindow && isVisibleInLayout(pSWindow) && !(pSWindow->GetStyle() & WB_NOLABEL) )
                {
                    WindowType nType = pSWindow->GetType();
                    if ( ( nType == WINDOW_FIXEDTEXT    ||
                          nType == WINDOW_FIXEDLINE ||
                          nType == WINDOW_GROUPBOX ) )
                    {
                        // a fixed text can't be labeld by a fixed text.
                        if ( ( nMyType != WINDOW_FIXEDTEXT ) || ( nType != WINDOW_FIXEDTEXT ) )
                            pWindow = pSWindow;
                        break;
                    }
                }
                if( nFoundIndex > nSearchIndex || nSearchIndex == 0 )
                    break;
            }
        }
    }
    return pWindow;
}

Window* Window::getLegacyNonLayoutAccessibleRelationLabeledBy() const
{
    Window* pWindow = NULL;
    Window* pFrameWindow = ImplGetFrameWindow();

    if ( mpWindowImpl->mpRealParent )
    {
        pWindow = mpWindowImpl->mpRealParent->GetParentLabeledBy( this );

        if( pWindow )
            return pWindow;
    }

    // #i62723#, #104191# checkboxes and radiobuttons are not supposed to have labels
    if( GetType() == WINDOW_CHECKBOX || GetType() == WINDOW_RADIOBUTTON )
        return NULL;

//    if( ! ( GetType() == WINDOW_FIXEDTEXT     ||
//            GetType() == WINDOW_FIXEDLINE     ||
//            GetType() == WINDOW_GROUPBOX ) )
    // #i100833# MT 2010/02: Group box and fixed lines can also lable a fixed text.
    // See tools/options/print for example.

    pWindow = ImplGetLabeledBy( pFrameWindow, GetType(), const_cast<Window*>(this) );
    if( ! pWindow && mpWindowImpl->mpRealParent )
        pWindow = ImplGetLabeledBy( mpWindowImpl->mpRealParent, GetType(), const_cast<Window*>(this) );

    return pWindow;
}

Window* Window::getLegacyNonLayoutAccessibleRelationMemberOf() const
{
    Window* pWindow = NULL;
    Window* pFrameWindow = GetParent();
    if ( !pFrameWindow )
    {
        pFrameWindow = ImplGetFrameWindow();
    }
    // if( ! ( GetType() == WINDOW_FIXEDTEXT        ||
    if( !( GetType() == WINDOW_FIXEDLINE ||
        GetType() == WINDOW_GROUPBOX ) )
    {
        // search for a control that makes member of this window
        // it is considered the last fixed line or group box
        // that comes before this control; with the exception of push buttons
        // which are labeled only if the fixed line or group box
        // is directly before the control
        // get form start and form end and index of this control
        sal_uInt16 nIndex, nFormStart, nFormEnd;
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
            for( sal_uInt16 nSearchIndex = nIndex-1; nSearchIndex >= nFormStart; nSearchIndex-- )
            {
                sal_uInt16 nFoundIndex = 0;
                pSWindow = ::ImplGetChildWindow( pFrameWindow,
                    nSearchIndex,
                    nFoundIndex,
                    false );
                if( pSWindow && pSWindow->IsVisible() &&
                    ( pSWindow->GetType() == WINDOW_FIXEDLINE   ||
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

KeyEvent Window::GetActivationKey() const
{
    KeyEvent aKeyEvent;

    sal_Unicode nAccel = getAccel( GetText() );
    if( ! nAccel )
    {
        Window* pWindow = GetAccessibleRelationLabeledBy();
        if( pWindow )
            nAccel = getAccel( pWindow->GetText() );
    }
    if( nAccel )
    {
        sal_uInt16 nCode = 0;
        if( nAccel >= 'a' && nAccel <= 'z' )
            nCode = KEY_A + (nAccel-'a');
        else if( nAccel >= 'A' && nAccel <= 'Z' )
            nCode = KEY_A + (nAccel-'A');
        else if( nAccel >= '0' && nAccel <= '9' )
            nCode = KEY_0 + (nAccel-'0');
        else if( nAccel == '.' )
            nCode = KEY_POINT;
        else if( nAccel == '-' )
            nCode = KEY_SUBTRACT;
        KeyCode aKeyCode( nCode, false, false, true, false );
        aKeyEvent = KeyEvent( nAccel, aKeyCode );
    }
    return aKeyEvent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
