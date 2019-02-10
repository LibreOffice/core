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


#include <svdata.hxx>

#include "dlgctrl.hxx"
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
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/i18n/XCharacterClassification.hpp>

using namespace ::com::sun::star;

static bool ImplHasIndirectTabParent( vcl::Window* pWindow )
{
    // The window has indirect tab parent if it is included in tab hierarchy
    // of the indirect parent window

    vcl::Window* pNonLayoutParent = getNonLayoutParent(pWindow);
    return ( pNonLayoutParent
          && ( pNonLayoutParent->ImplGetWindow()->GetStyle() & WB_CHILDDLGCTRL ) );
}

static vcl::Window* ImplGetTopParentOfTabHierarchy( vcl::Window* pParent )
{
    // The method allows to find the most close parent containing all the
    // window from the current tab-hierarchy
    // The direct parent should be provided as a parameter here

    vcl::Window* pResult = pParent;

    if ( pResult )
    {
        vcl::Window* pNonLayoutParent = getNonLayoutParent(pResult);
        while ( pNonLayoutParent && ( pResult->ImplGetWindow()->GetStyle() & WB_CHILDDLGCTRL ) )
        {
            pResult = pNonLayoutParent;
            pNonLayoutParent = getNonLayoutParent(pResult);
        }
    }

    return pResult;
}

static vcl::Window* ImplGetCurTabWindow(const vcl::Window* pWindow)
{
    assert(pWindow->GetType() == WindowType::TABCONTROL);
    const TabControl* pTabControl = static_cast<const TabControl*>(pWindow);
    // Check if the TabPage is a Child of the TabControl and still exists (by
    // walking all child windows); because it could be that the TabPage has been
    // destroyed already by a Dialog-Dtor, event that the TabControl still exists.
    const TabPage* pTempTabPage = pTabControl->GetTabPage(pTabControl->GetCurPageId());
    if (pTempTabPage)
    {
        vcl::Window* pTempWindow = pTabControl->GetWindow(GetWindowType::FirstChild);
        while (pTempWindow)
        {
            if (pTempWindow->ImplGetWindow() == pTempTabPage)
            {
                return const_cast<TabPage*>(pTempTabPage);
            }
            pTempWindow = nextLogicalChildOfParent(pTabControl, pTempWindow);
        }
    }

    return nullptr;
}

static vcl::Window* ImplGetSubChildWindow( vcl::Window* pParent, sal_uInt16 n, sal_uInt16& nIndex )
{
    // ignore all windows with mpClientWindow set
    for (vcl::Window *pNewParent = pParent->ImplGetWindow();
         pParent != pNewParent; pParent = pNewParent);

    vcl::Window* pFoundWindow = nullptr;
    vcl::Window* pWindow = firstLogicalChildOfParent(pParent);
    vcl::Window* pNextWindow = pWindow;

    // process just the current page of a tab control
    if (pWindow && pParent->GetType() == WindowType::TABCONTROL)
    {
        pWindow = ImplGetCurTabWindow(pParent);
        pNextWindow = lastLogicalChildOfParent(pParent);
    }

    while (pWindow)
    {
        pWindow = pWindow->ImplGetWindow();

        // skip invisible and disabled windows
        if (isVisibleInLayout(pWindow))
        {
            // return the TabControl itself, before handling its page
            if (pWindow->GetType() == WindowType::TABCONTROL)
            {
                if (n == nIndex)
                    return pWindow;
                ++nIndex;
            }
            if (pWindow->GetStyle() & (WB_DIALOGCONTROL | WB_CHILDDLGCTRL))
                pFoundWindow = ImplGetSubChildWindow(pWindow, n, nIndex);
            else
                pFoundWindow = pWindow;

            if (n == nIndex)
                return pFoundWindow;
            ++nIndex;
        }

        pWindow = nextLogicalChildOfParent(pParent, pNextWindow);
        pNextWindow = pWindow;
    }

    --nIndex;
    assert(!pFoundWindow || (pFoundWindow == pFoundWindow->ImplGetWindow()));
    return pFoundWindow;
}

vcl::Window* ImplGetChildWindow( vcl::Window* pParent, sal_uInt16 n, sal_uInt16& nIndex, bool bTestEnable )
{
    pParent = ImplGetTopParentOfTabHierarchy( pParent );

    nIndex = 0;
    vcl::Window* pWindow = ImplGetSubChildWindow( pParent, n, nIndex );
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

static vcl::Window* ImplGetNextWindow( vcl::Window* pParent, sal_uInt16 n, sal_uInt16& nIndex, bool bTestEnable )
{
    vcl::Window* pWindow = ImplGetChildWindow( pParent, n+1, nIndex, bTestEnable );
    if ( n == nIndex )
    {
        n = 0;
        pWindow = ImplGetChildWindow( pParent, n, nIndex, bTestEnable );
    }
    return pWindow;
}

namespace vcl {

static bool lcl_ToolBoxTabStop( Window* pWindow )
{
    ToolBox* pToolBoxWindow = static_cast<ToolBox*>( pWindow );

    sal_uInt16 nId;
    for ( ToolBox::ImplToolItems::size_type nPos = 0; nPos < pToolBoxWindow->GetItemCount(); nPos++ )
    {
        nId = pToolBoxWindow->GetItemId( nPos );
        if ( pToolBoxWindow->IsItemVisible( nId ) && pToolBoxWindow->IsItemEnabled( nId ) )
            return true;
    }

    return false;
}

vcl::Window* Window::ImplGetDlgWindow( sal_uInt16 nIndex, GetDlgWindowType nType,
                                  sal_uInt16 nFormStart, sal_uInt16 nFormEnd,
                                  sal_uInt16* pIndex )
{
    SAL_WARN_IF( (nIndex < nFormStart) || (nIndex > nFormEnd), "vcl",
                "Window::ImplGetDlgWindow() - nIndex not in Form" );

    vcl::Window* pWindow = nullptr;
    sal_uInt16  i;
    sal_uInt16  nTemp;
    sal_uInt16  nStartIndex;

    if ( nType == GetDlgWindowType::Prev )
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
            {
                if ( WindowType::TOOLBOX == pWindow->GetType() )
                {
                    if ( lcl_ToolBoxTabStop( pWindow ) )
                        break;
                }
                else
                    break;
            }
        }
        while ( i != nIndex );
    }
    else
    {
        i = nIndex;
        pWindow = ImplGetChildWindow( this, i, i, (nType == GetDlgWindowType::First) );
        if ( pWindow )
        {
            nStartIndex = i;

            if ( nType == GetDlgWindowType::Next )
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

            if (i <= nFormEnd && pWindow)
            {
                // carry the 2nd index, in case all controls are disabled
                sal_uInt16 nStartIndex2 = i;
                sal_uInt16 nOldIndex = i+1;

                do
                {
                    if ( pWindow->GetStyle() & WB_TABSTOP )
                    {
                        if ( WindowType::TOOLBOX == pWindow->GetType() )
                        {
                            if ( lcl_ToolBoxTabStop( pWindow ) )
                                break;
                        }
                        else
                            break;
                    }
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
                while (i != nStartIndex && i != nStartIndex2 && pWindow);

                if ( (i == nStartIndex2) && pWindow &&
                     (!(pWindow->GetStyle() & WB_TABSTOP) || !isEnabledInLayout(pWindow)) )
                    i = nStartIndex;
            }
        }

        if ( nType == GetDlgWindowType::First )
        {
            if ( pWindow )
            {
                if ( pWindow->GetType() == WindowType::TABCONTROL )
                {
                    vcl::Window* pNextWindow = ImplGetDlgWindow( i, GetDlgWindowType::Next );
                    if ( pNextWindow )
                    {
                        if ( pWindow->IsChild( pNextWindow ) )
                            pWindow = pNextWindow;
                    }
                }

                if ( !(pWindow->GetStyle() & WB_TABSTOP) )
                    pWindow = nullptr;
            }
        }
    }

    if ( pIndex )
        *pIndex = i;

    return pWindow;
}

} /* namespace vcl */

vcl::Window* ImplFindDlgCtrlWindow( vcl::Window* pParent, vcl::Window* pWindow, sal_uInt16& rIndex,
                               sal_uInt16& rFormStart, sal_uInt16& rFormEnd )
{
    vcl::Window* pSWindow;
    vcl::Window* pSecondWindow = nullptr;
    vcl::Window* pTempWindow = nullptr;
    sal_uInt16  i;
    sal_uInt16  nSecond_i = 0;
    sal_uInt16  nFormStart = 0;
    sal_uInt16  nSecondFormStart = 0;
    sal_uInt16  nFormEnd;

    // find focus window in the child list
    vcl::Window* pFirstChildWindow = pSWindow = ImplGetChildWindow( pParent, 0, i, false );

    if( pWindow == nullptr )
        pWindow = pSWindow;

    while ( pSWindow )
    {
        // the DialogControlStart mark is only accepted for the direct children
        if ( !ImplHasIndirectTabParent( pSWindow )
          && pSWindow->ImplGetWindow()->IsDialogControlStart() )
            nFormStart = i;

        // SecondWindow for composite controls like ComboBoxes and arrays
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
            pSWindow = nullptr;
    }

    if ( !pSWindow )
    {
        // Window not found; we cannot handle it
        if ( !pSecondWindow )
            return nullptr;
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
                SAL_WARN( "vcl", "It seems to be an endless loop!" );
                rFormStart = 0;
                break;
            }
        }
    }
    while ( pTempWindow );
    rFormEnd = nFormEnd;

    return pSWindow;
}

vcl::Window* ImplFindAccelWindow( vcl::Window* pParent, sal_uInt16& rIndex, sal_Unicode cCharCode,
                             sal_uInt16 nFormStart, sal_uInt16 nFormEnd, bool bCheckEnable )
{
    SAL_WARN_IF( (rIndex < nFormStart) || (rIndex > nFormEnd), "vcl",
                "Window::ImplFindAccelWindow() - rIndex not in Form" );

    sal_Unicode cCompareChar;
    sal_uInt16  nStart = rIndex;
    sal_uInt16  i = rIndex;
    vcl::Window* pWindow;

    uno::Reference<i18n::XCharacterClassification> const& xCharClass(ImplGetCharClass());

    const css::lang::Locale& rLocale = Application::GetSettings().GetUILanguageTag().getLocale();
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
                if (pWindow->GetType() == WindowType::FIXEDTEXT)
                {
                    FixedText *pFixedText = static_cast<FixedText*>(pWindow);
                    vcl::Window *pMnemonicWidget = pFixedText->get_mnemonic_widget();
                    SAL_WARN_IF(isContainerWindow(pFixedText->GetParent()) && !pMnemonicWidget,
                        "vcl.a11y", "label missing mnemonic_widget?");
                    if (pMnemonicWidget)
                        return pMnemonicWidget;
                }

                // skip Static-Controls
                if ( (pWindow->GetType() == WindowType::FIXEDTEXT)   ||
                     (pWindow->GetType() == WindowType::FIXEDLINE)   ||
                     (pWindow->GetType() == WindowType::GROUPBOX) )
                    pWindow = pParent->ImplGetDlgWindow( i, GetDlgWindowType::Next );
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
            ::ImplFindDlgCtrlWindow( pWindow, nullptr,
                                     nChildIndex, nChildFormStart, nChildFormEnd );
            vcl::Window* pAccelWin = ImplFindAccelWindow( pWindow, nChildIndex, cCharCode,
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

    return nullptr;
}

namespace vcl {

void Window::ImplControlFocus( GetFocusFlags nFlags )
{
    if ( nFlags & GetFocusFlags::Mnemonic )
    {
        if ( GetType() == WindowType::RADIOBUTTON )
        {
            if ( !static_cast<RadioButton*>(this)->IsChecked() )
                static_cast<RadioButton*>(this)->ImplCallClick( true, nFlags );
            else
                ImplGrabFocus( nFlags );
        }
        else
        {
            ImplGrabFocus( nFlags );
            if ( nFlags & GetFocusFlags::UniqueMnemonic )
            {
                if ( GetType() == WindowType::CHECKBOX )
                    static_cast<CheckBox*>(this)->ImplCheck();
                else if ( mpWindowImpl->mbPushButton )
                {
                    static_cast<PushButton*>(this)->SetPressed( true );
                    static_cast<PushButton*>(this)->SetPressed( false );
                    static_cast<PushButton*>(this)->Click();
                }
            }
        }
    }
    else
    {
        if ( GetType() == WindowType::RADIOBUTTON )
        {
            if ( !static_cast<RadioButton*>(this)->IsChecked() )
                static_cast<RadioButton*>(this)->ImplCallClick( true, nFlags );
            else
                ImplGrabFocus( nFlags );
        }
        else
            ImplGrabFocus( nFlags );
    }
}

} /* namespace vcl */

namespace
{
    bool isSuitableDestination(vcl::Window const *pWindow)
    {
        return (pWindow && isVisibleInLayout(pWindow) &&
                isEnabledInLayout(pWindow) && pWindow->IsInputEnabled() &&
                //Pure window shouldn't get window after controls such as
                //buttons.
                (pWindow->GetType() != WindowType::WINDOW &&
                  pWindow->GetType() != WindowType::WORKWINDOW && pWindow->GetType() != WindowType::CONTROL)
               );
    }

    bool focusNextInGroup(const std::vector<VclPtr<RadioButton> >::iterator& aStart, std::vector<VclPtr<RadioButton> > &rGroup)
    {
        std::vector<VclPtr<RadioButton> >::iterator aI(aStart);

        if (aStart != rGroup.end())
            ++aI;

        aI = std::find_if(aI, rGroup.end(), isSuitableDestination);
        if (aI != rGroup.end())
        {
            vcl::Window *pWindow = *aI;
            pWindow->ImplControlFocus( GetFocusFlags::CURSOR | GetFocusFlags::Forward );
            return true;
        }
        aI = std::find_if(rGroup.begin(), aStart, isSuitableDestination);
        if (aI != aStart)
        {
            vcl::Window *pWindow = *aI;
            pWindow->ImplControlFocus( GetFocusFlags::CURSOR | GetFocusFlags::Forward );
            return true;
        }
        return false;
    }

    bool nextInGroup(RadioButton *pSourceWindow, bool bBackward)
    {
        std::vector<VclPtr<RadioButton> > aGroup(pSourceWindow->GetRadioButtonGroup());

        if (aGroup.size() == 1) //only one button in group
            return false;

        if (bBackward)
            std::reverse(aGroup.begin(), aGroup.end());

        auto aStart(std::find(aGroup.begin(), aGroup.end(), VclPtr<RadioButton>(pSourceWindow)));

        assert(aStart != aGroup.end());

        return focusNextInGroup(aStart, aGroup);
    }
}

namespace vcl {

bool Window::ImplDlgCtrl( const KeyEvent& rKEvt, bool bKeyInput )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16  nKeyCode = aKeyCode.GetCode();
    vcl::Window* pSWindow;
    vcl::Window* pTempWindow;
    vcl::Window* pButtonWindow;
    sal_uInt16  i;
    sal_uInt16  iButton;
    sal_uInt16  iButtonStart;
    sal_uInt16  iTemp;
    sal_uInt16  nIndex;
    sal_uInt16  nFormStart;
    sal_uInt16  nFormEnd;
    DialogControlFlags nDlgCtrlFlags;

    // we cannot take over control without Focus-window
    vcl::Window* pFocusWindow = Application::GetFocusWindow();
    if ( !pFocusWindow || !ImplIsWindowOrChild( pFocusWindow ) )
        return false;

    // find Focus-Window in the child list
    pSWindow = ::ImplFindDlgCtrlWindow( this, pFocusWindow,
                                        nIndex, nFormStart, nFormEnd );
    if ( !pSWindow )
        return false;
    i = nIndex;

    nDlgCtrlFlags = DialogControlFlags::NONE;
    pTempWindow = pSWindow;
    do
    {
        nDlgCtrlFlags |= pTempWindow->GetDialogControlFlags();
        if ( pTempWindow == this )
            break;
        pTempWindow = pTempWindow->ImplGetParent();
    }
    while ( pTempWindow );

    pButtonWindow = nullptr;

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
                pButtonWindow = nullptr;
        }

        if ( bKeyInput && !pButtonWindow && (nDlgCtrlFlags & DialogControlFlags::Return) )
        {
            GetDlgWindowType nType;
            GetFocusFlags    nGetFocusFlags = GetFocusFlags::Tab;
            sal_uInt16  nNewIndex;
            sal_uInt16  iStart;
            if ( aKeyCode.IsShift() )
            {
                nType = GetDlgWindowType::Prev;
                nGetFocusFlags |= GetFocusFlags::Backward;
            }
            else
            {
                nType = GetDlgWindowType::Next;
                nGetFocusFlags |= GetFocusFlags::Forward;
            }
            iStart = i;
            pTempWindow = ImplGetDlgWindow( i, nType, nFormStart, nFormEnd, &nNewIndex );
            while ( pTempWindow && (pTempWindow != pSWindow) )
            {
                if ( !pTempWindow->mpWindowImpl->mbPushButton )
                {
                    // get Around-Flag
                    if ( nType == GetDlgWindowType::Prev )
                    {
                        if ( nNewIndex > iStart )
                            nGetFocusFlags |= GetFocusFlags::Around;
                    }
                    else
                    {
                        if ( nNewIndex < iStart )
                            nGetFocusFlags |= GetFocusFlags::Around;
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
                    pTempWindow = nullptr;
            }
            // if this is the same window, simulate a Get/LoseFocus,
            // in case AROUND is being processed
            if ( pTempWindow && (pTempWindow == pSWindow) )
            {
                NotifyEvent aNEvt1( MouseNotifyEvent::LOSEFOCUS, pSWindow );
                if ( !ImplCallPreNotify( aNEvt1 ) )
                    pSWindow->CompatLoseFocus();
                pSWindow->mpWindowImpl->mnGetFocusFlags = nGetFocusFlags | GetFocusFlags::Around;
                NotifyEvent aNEvt2( MouseNotifyEvent::GETFOCUS, pSWindow );
                if ( !ImplCallPreNotify( aNEvt2 ) )
                    pSWindow->CompatGetFocus();
                pSWindow->mpWindowImpl->mnGetFocusFlags = GetFocusFlags::NONE;
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
            if ( pButtonWindow->GetType() == WindowType::CANCELBUTTON )
                break;

            pButtonWindow = ImplGetNextWindow( this, iButton, iButton, true );
            if ( (iButton <= iButtonStart) || (iButton > nFormEnd) )
                pButtonWindow = nullptr;
        }

        if ( bKeyInput && mpWindowImpl->mpDlgCtrlDownWindow )
        {
            if ( mpWindowImpl->mpDlgCtrlDownWindow.get() != pButtonWindow )
            {
                static_cast<PushButton*>(mpWindowImpl->mpDlgCtrlDownWindow.get())->SetPressed( false );
                mpWindowImpl->mpDlgCtrlDownWindow = nullptr;
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
                GetDlgWindowType nType;
                GetFocusFlags    nGetFocusFlags = GetFocusFlags::Tab;
                sal_uInt16  nNewIndex;
                bool        bFormular = false;

                // for Ctrl-Tab check if we want to jump to next template
                if ( aKeyCode.IsMod1() )
                {
                    // search group
                    vcl::Window* pFormularFirstWindow = nullptr;
                    vcl::Window* pLastFormularFirstWindow = nullptr;
                    pTempWindow = ImplGetChildWindow( this, 0, iTemp, false );
                    vcl::Window* pPrevFirstFormularFirstWindow = nullptr;
                    vcl::Window* pFirstFormularFirstWindow = pTempWindow;
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
                            pTempWindow = nullptr;
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
                            pFormularFirstWindow = ImplGetDlgWindow( nTempIndex, GetDlgWindowType::First, nFoundFormStart, nFoundFormEnd );
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
                    if (!aKeyCode.IsMod1() || (pSWindow->GetStyle() & WB_NODIALOGCONTROL))
                    {
                        if ( aKeyCode.IsShift() )
                        {
                            nType = GetDlgWindowType::Prev;
                            nGetFocusFlags |= GetFocusFlags::Backward;
                        }
                        else
                        {
                            nType = GetDlgWindowType::Next;
                            nGetFocusFlags |= GetFocusFlags::Forward;
                        }
                        vcl::Window* pWindow = ImplGetDlgWindow( i, nType, nFormStart, nFormEnd, &nNewIndex );
                        // if this is the same window, simulate a Get/LoseFocus,
                        // in case AROUND is being processed
                        if ( pWindow == pSWindow )
                        {
                            NotifyEvent aNEvt1( MouseNotifyEvent::LOSEFOCUS, pSWindow );
                            if ( !ImplCallPreNotify( aNEvt1 ) )
                                pSWindow->CompatLoseFocus();
                            pSWindow->mpWindowImpl->mnGetFocusFlags = nGetFocusFlags | GetFocusFlags::Around;
                            NotifyEvent aNEvt2( MouseNotifyEvent::GETFOCUS, pSWindow );
                            if ( !ImplCallPreNotify( aNEvt2 ) )
                                pSWindow->CompatGetFocus();
                            pSWindow->mpWindowImpl->mnGetFocusFlags = GetFocusFlags::NONE;
                            return true;
                        }
                        else if ( pWindow )
                        {
                            // get Around-Flag
                            if ( nType == GetDlgWindowType::Prev )
                            {
                                if ( nNewIndex > i )
                                    nGetFocusFlags |= GetFocusFlags::Around;
                            }
                            else
                            {
                                if ( nNewIndex < i )
                                    nGetFocusFlags |= GetFocusFlags::Around;
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
            if (pSWindow->GetType() == WindowType::RADIOBUTTON)
                return nextInGroup(static_cast<RadioButton*>(pSWindow), true);
            else
            {
                WinBits nStyle = pSWindow->GetStyle();
                if ( !(nStyle & WB_GROUP) )
                {
                    vcl::Window* pWindow = prevLogicalChildOfParent(this, pSWindow);
                    while ( pWindow )
                    {
                        pWindow = pWindow->ImplGetWindow();

                        nStyle = pWindow->GetStyle();

                        if (isSuitableDestination(pWindow))
                        {
                            if ( pWindow != pSWindow )
                                pWindow->ImplControlFocus( GetFocusFlags::CURSOR | GetFocusFlags::Backward );
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
            if (pSWindow->GetType() == WindowType::RADIOBUTTON)
                return nextInGroup(static_cast<RadioButton*>(pSWindow), false);
            else
            {
                vcl::Window* pWindow = nextLogicalChildOfParent(this, pSWindow);
                while ( pWindow )
                {
                    pWindow = pWindow->ImplGetWindow();

                    WinBits nStyle = pWindow->GetStyle();

                    if ( nStyle & WB_GROUP )
                        break;

                    if (isSuitableDestination(pWindow))
                    {
                        pWindow->ImplControlFocus( GetFocusFlags::CURSOR | GetFocusFlags::Backward );
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
                    GetFocusFlags nGetFocusFlags = GetFocusFlags::Mnemonic;
                    if ( pSWindow == ::ImplFindAccelWindow( this, i, c, nFormStart, nFormEnd ) )
                        nGetFocusFlags |= GetFocusFlags::UniqueMnemonic;
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
            if ( mpWindowImpl->mpDlgCtrlDownWindow && (mpWindowImpl->mpDlgCtrlDownWindow.get() != pButtonWindow) )
            {
                static_cast<PushButton*>(mpWindowImpl->mpDlgCtrlDownWindow.get())->SetPressed( false );
                mpWindowImpl->mpDlgCtrlDownWindow = nullptr;
            }

            static_cast<PushButton*>(pButtonWindow)->SetPressed( true );
            mpWindowImpl->mpDlgCtrlDownWindow = pButtonWindow;
        }
        else if ( mpWindowImpl->mpDlgCtrlDownWindow.get() == pButtonWindow )
        {
            mpWindowImpl->mpDlgCtrlDownWindow = nullptr;
            static_cast<PushButton*>(pButtonWindow)->SetPressed( false );
            static_cast<PushButton*>(pButtonWindow)->Click();
        }

        return true;
    }

    return false;
}

// checks if this window has dialog control
bool Window::ImplHasDlgCtrl() const
{
    vcl::Window* pDlgCtrlParent;

    // lookup window for dialog control
    pDlgCtrlParent = ImplGetParent();
    while ( pDlgCtrlParent &&
            !pDlgCtrlParent->ImplIsOverlapWindow() &&
            ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
        pDlgCtrlParent = pDlgCtrlParent->ImplGetParent();

    return pDlgCtrlParent && ((pDlgCtrlParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL);
}

void Window::ImplDlgCtrlNextWindow()
{
    vcl::Window* pDlgCtrlParent;
    vcl::Window* pDlgCtrl;
    vcl::Window* pSWindow;
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

    vcl::Window* pWindow = pDlgCtrlParent->ImplGetDlgWindow( nIndex, GetDlgWindowType::Next, nFormStart, nFormEnd );
    if ( pWindow && (pWindow != pSWindow) )
        pWindow->ImplControlFocus();
}

static void ImplDlgCtrlUpdateDefButton( vcl::Window* pParent, vcl::Window* pFocusWindow,
                                        bool bGetFocus )
{
    PushButton* pOldDefButton   = nullptr;
    PushButton* pNewDefButton   = nullptr;
    vcl::Window*     pSWindow;
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
            PushButton* pPushButton = static_cast<PushButton*>(pSWindow);
            if ( pPushButton->ImplIsDefButton() )
                pOldDefButton = pPushButton;
            if ( pPushButton->HasChildPathFocus() )
                pNewDefButton = pPushButton;
            else if ( !pNewDefButton && (pPushButton->GetStyle() & WB_DEFBUTTON) )
                pNewDefButton = pPushButton;
        }

        pSWindow = ImplGetNextWindow( pParent, i, i, false );
        if ( !i || (i > nFormEnd) )
            pSWindow = nullptr;
    }

    if ( !bGetFocus )
    {
        sal_uInt16 nDummy;
        vcl::Window* pNewFocusWindow = Application::GetFocusWindow();
        if ( !pNewFocusWindow || !pParent->ImplIsWindowOrChild( pNewFocusWindow ) )
            pNewDefButton = nullptr;
        else if ( !::ImplFindDlgCtrlWindow( pParent, pNewFocusWindow, i, nDummy, nDummy ) ||
                  (i < nFormStart) || (i > nFormEnd) )
            pNewDefButton = nullptr;
    }

    if ( pOldDefButton != pNewDefButton )
    {
        if ( pOldDefButton )
            pOldDefButton->ImplSetDefButton( false );
        if ( pNewDefButton )
            pNewDefButton->ImplSetDefButton( true );
    }
}

void Window::ImplDlgCtrlFocusChanged( vcl::Window* pWindow, bool bGetFocus )
{
    if ( mpWindowImpl->mpDlgCtrlDownWindow && !bGetFocus )
    {
        static_cast<PushButton*>(mpWindowImpl->mpDlgCtrlDownWindow.get())->SetPressed( false );
        mpWindowImpl->mpDlgCtrlDownWindow = nullptr;
    }

    ImplDlgCtrlUpdateDefButton( this, pWindow, bGetFocus );
}

vcl::Window* Window::ImplFindDlgCtrlWindow( vcl::Window* pWindow )
{
    sal_uInt16  nIndex;
    sal_uInt16  nFormStart;
    sal_uInt16  nFormEnd;

    // find Focus-Window in the Child-List and return
    return ::ImplFindDlgCtrlWindow( this, pWindow, nIndex, nFormStart, nFormEnd );
}

vcl::Window* Window::GetParentLabelFor( const vcl::Window* ) const
{
    return nullptr;
}

vcl::Window* Window::GetParentLabeledBy( const vcl::Window* ) const
{
    return nullptr;
}

KeyEvent Window::GetActivationKey() const
{
    KeyEvent aKeyEvent;

    sal_Unicode nAccel = getAccel( GetText() );
    if( ! nAccel )
    {
        vcl::Window* pWindow = GetAccessibleRelationLabeledBy();
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
        vcl::KeyCode aKeyCode( nCode, false, false, true, false );
        aKeyEvent = KeyEvent( nAccel, aKeyCode );
    }
    return aKeyEvent;
}

} /* namespace vcl */

sal_Unicode getAccel( const OUString& rStr )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
