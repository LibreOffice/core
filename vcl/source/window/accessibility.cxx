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

#include <config_features.h>

#include <i18nlangtag/mslangid.hxx>

#include <tools/time.hxx>

#include <unotools/fontcfg.hxx>
#include <unotools/confignode.hxx>

#include <vcl/layout.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/help.hxx>
#include <vcl/cursor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/syschild.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/wall.hxx>
#include <vcl/gradient.hxx>
#include <vcl/button.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/dialog.hxx>
#include <vcl/toolkit/unowrap.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/popupmenuwindow.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

// declare system types in sysdata.hxx
#include <vcl/sysdata.hxx>

#include <salframe.hxx>
#include <salobj.hxx>
#include <salinst.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <fontinstance.hxx>
#include <window.h>
#include <toolbox.h>
#include <outdev.h>
#include <PhysicalFontCollection.hxx>
#include <brdwin.hxx>
#include <helpwin.hxx>
#include <sallayout.hxx>
#include <dndlistenercontainer.hxx>
#include <dndeventdispatcher.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceName.hpp>

#include <sal/macros.h>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include <set>
#include <typeinfo>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::com::sun::star;


ImplAccessibleInfos::ImplAccessibleInfos()
{
    nAccessibleRole = 0xFFFF;
    pLabeledByWindow = nullptr;
    pLabelForWindow = nullptr;
    pMemberOfWindow = nullptr;
}

ImplAccessibleInfos::~ImplAccessibleInfos()
{
}

namespace vcl {

css::uno::Reference< css::accessibility::XAccessible > Window::GetAccessible( bool bCreate )
{
    // do not optimize hierarchy for the top level border win (ie, when there is no parent)
    /* // do not optimize accessible hierarchy at all to better reflect real VCL hierarchy
    if ( GetParent() && ( GetType() == WindowType::BORDERWINDOW ) && ( GetChildCount() == 1 ) )
    //if( !ImplIsAccessibleCandidate() )
    {
        vcl::Window* pChild = GetAccessibleChildWindow( 0 );
        if ( pChild )
            return pChild->GetAccessible();
    }
    */
    if ( !mpWindowImpl )
        return css::uno::Reference< css::accessibility::XAccessible >();
    if ( !mpWindowImpl->mxAccessible.is() && bCreate )
        mpWindowImpl->mxAccessible = CreateAccessible();

    return mpWindowImpl->mxAccessible;
}

css::uno::Reference< css::accessibility::XAccessible > Window::CreateAccessible()
{
    css::uno::Reference< css::accessibility::XAccessible > xAcc( GetComponentInterface(), css::uno::UNO_QUERY );
    return xAcc;
}

void Window::SetAccessible( const css::uno::Reference< css::accessibility::XAccessible >& x )
{
    if (!mpWindowImpl)
        return;

    mpWindowImpl->mxAccessible = x;
}

// skip all border windows that are not top level frames
bool Window::ImplIsAccessibleCandidate() const
{
    if( !mpWindowImpl->mbBorderWin )
        return true;
    else
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menus!) are closeable
        if( mpWindowImpl->mbFrame && mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return true;
        else
            return false;
}

bool Window::ImplIsAccessibleNativeFrame() const
{
    if( mpWindowImpl->mbFrame )
        // #101741 do not check for WB_CLOSEABLE because undecorated floaters (like menus!) are closeable
        if( mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) )
            return true;
        else
            return false;
    else
        return false;
}

vcl::Window* Window::GetAccessibleParentWindow() const
{
    if (!mpWindowImpl || ImplIsAccessibleNativeFrame())
        return nullptr;

    vcl::Window* pParent = mpWindowImpl->mpParent;
    if( GetType() == WindowType::MENUBARWINDOW )
    {
        // report the menubar as a child of THE workwindow
        vcl::Window *pWorkWin = GetParent()->mpWindowImpl->mpFirstChild;
        while( pWorkWin && (pWorkWin == this) )
            pWorkWin = pWorkWin->mpWindowImpl->mpNext;
        pParent = pWorkWin;
    }
    // If this is a floating window which has a native border window, then that border should be reported as
    // the accessible parent, unless the floating window is a PopupMenuFloatingWindow

    // The logic here has to match that of AccessibleFactory::createAccessibleContext in
    // accessibility/source/helper/acc_factory.cxx to avoid PopupMenuFloatingWindow
    // becoming a11y parents of themselves
    else if( GetType() == WindowType::FLOATINGWINDOW &&
        mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame &&
        !PopupMenuFloatingWindow::isPopupMenu(this))
    {
        pParent = mpWindowImpl->mpBorderWindow;
    }
    else if( pParent && !pParent->ImplIsAccessibleCandidate() )
    {
        pParent = pParent->mpWindowImpl->mpParent;
    }
    return pParent;
}

sal_uInt16 Window::GetAccessibleChildWindowCount()
{
    if (!mpWindowImpl)
        return 0;

    sal_uInt16 nChildren = 0;
    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
            nChildren++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    // report the menubarwindow as a child of THE workwindow
    if( GetType() == WindowType::BORDERWINDOW )
    {
        ImplBorderWindow *pBorderWindow = static_cast<ImplBorderWindow*>(this);
        if( pBorderWindow->mpMenuBarWindow &&
            pBorderWindow->mpMenuBarWindow->IsVisible()
            )
            --nChildren;
    }
    else if( GetType() == WindowType::WORKWINDOW )
    {
        WorkWindow *pWorkWindow = static_cast<WorkWindow*>(this);
        if( pWorkWindow->GetMenuBar() &&
            pWorkWindow->GetMenuBar()->GetWindow() &&
            pWorkWindow->GetMenuBar()->GetWindow()->IsVisible()
            )
            ++nChildren;
    }

    return nChildren;
}

vcl::Window* Window::GetAccessibleChildWindow( sal_uInt16 n )
{
    // report the menubarwindow as a the first child of THE workwindow
    if( GetType() == WindowType::WORKWINDOW && static_cast<WorkWindow *>(this)->GetMenuBar() )
    {
        if( n == 0 )
        {
            MenuBar *pMenuBar = static_cast<WorkWindow *>(this)->GetMenuBar();
            if( pMenuBar->GetWindow() && pMenuBar->GetWindow()->IsVisible() )
                return pMenuBar->GetWindow();
        }
        else
            --n;
    }

    // transform n to child number including invisible children
    sal_uInt16 nChildren = n;
    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
        {
            if( ! nChildren )
                break;
            nChildren--;
        }
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if( GetType() == WindowType::BORDERWINDOW && pChild && pChild->GetType() == WindowType::MENUBARWINDOW )
    {
        do pChild = pChild->mpWindowImpl->mpNext; while( pChild && ! pChild->IsVisible() );
        SAL_WARN_IF( !pChild, "vcl", "GetAccessibleChildWindow(): wrong index in border window");
    }

    if ( pChild && ( pChild->GetType() == WindowType::BORDERWINDOW ) && ( pChild->GetChildCount() == 1 ) )
    {
        pChild = pChild->GetChild( 0 );
    }
    return pChild;
}

void Window::SetAccessibleRole( sal_uInt16 nRole )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos.reset( new ImplAccessibleInfos );

    SAL_WARN_IF( mpWindowImpl->mpAccessibleInfos->nAccessibleRole != 0xFFFF, "vcl", "AccessibleRole already set!" );
    mpWindowImpl->mpAccessibleInfos->nAccessibleRole = nRole;
}

sal_uInt16 Window::getDefaultAccessibleRole() const
{
    sal_uInt16 nRole = 0xFFFF;
    switch ( GetType() )
    {
        case WindowType::MESSBOX:    // MT: Would be nice to have special roles!
        case WindowType::INFOBOX:
        case WindowType::WARNINGBOX:
        case WindowType::ERRORBOX:
        case WindowType::QUERYBOX: nRole = accessibility::AccessibleRole::ALERT; break;

        case WindowType::MODELESSDIALOG:
        case WindowType::MODALDIALOG:
        case WindowType::TABDIALOG:
        case WindowType::BUTTONDIALOG:
        case WindowType::DIALOG: nRole = accessibility::AccessibleRole::DIALOG; break;

        case WindowType::PUSHBUTTON:
        case WindowType::OKBUTTON:
        case WindowType::CANCELBUTTON:
        case WindowType::HELPBUTTON:
        case WindowType::IMAGEBUTTON:
        case WindowType::MOREBUTTON:
        case WindowType::SPINBUTTON: nRole = accessibility::AccessibleRole::PUSH_BUTTON; break;
        case WindowType::MENUBUTTON: nRole = accessibility::AccessibleRole::BUTTON_MENU; break;

        case WindowType::RADIOBUTTON: nRole = accessibility::AccessibleRole::RADIO_BUTTON; break;
        case WindowType::TRISTATEBOX:
        case WindowType::CHECKBOX: nRole = accessibility::AccessibleRole::CHECK_BOX; break;

        case WindowType::MULTILINEEDIT: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

        case WindowType::PATTERNFIELD:
        case WindowType::CALCINPUTLINE:
        case WindowType::EDIT: nRole = static_cast<Edit const *>(this)->IsPassword() ? accessibility::AccessibleRole::PASSWORD_TEXT : accessibility::AccessibleRole::TEXT; break;

        case WindowType::PATTERNBOX:
        case WindowType::NUMERICBOX:
        case WindowType::METRICBOX:
        case WindowType::CURRENCYBOX:
        case WindowType::LONGCURRENCYBOX:
        case WindowType::COMBOBOX: nRole = accessibility::AccessibleRole::COMBO_BOX; break;

        case WindowType::LISTBOX:
        case WindowType::MULTILISTBOX: nRole = accessibility::AccessibleRole::LIST; break;

        case WindowType::TREELISTBOX: nRole = accessibility::AccessibleRole::TREE; break;

        case WindowType::FIXEDTEXT: nRole = accessibility::AccessibleRole::LABEL; break;
        case WindowType::FIXEDLINE:
            if( !GetText().isEmpty() )
                nRole = accessibility::AccessibleRole::LABEL;
            else
                nRole = accessibility::AccessibleRole::SEPARATOR;
            break;

        case WindowType::FIXEDBITMAP:
        case WindowType::FIXEDIMAGE: nRole = accessibility::AccessibleRole::ICON; break;
        case WindowType::GROUPBOX: nRole = accessibility::AccessibleRole::GROUP_BOX; break;
        case WindowType::SCROLLBAR: nRole = accessibility::AccessibleRole::SCROLL_BAR; break;

        case WindowType::SLIDER:
        case WindowType::SPLITTER:
        case WindowType::SPLITWINDOW: nRole = accessibility::AccessibleRole::SPLIT_PANE; break;

        case WindowType::DATEBOX:
        case WindowType::TIMEBOX:
        case WindowType::DATEFIELD:
        case WindowType::TIMEFIELD: nRole = accessibility::AccessibleRole::DATE_EDITOR; break;

        case WindowType::NUMERICFIELD:
        case WindowType::METRICFIELD:
        case WindowType::CURRENCYFIELD:
        case WindowType::LONGCURRENCYFIELD:
        case WindowType::SPINFIELD: nRole = accessibility::AccessibleRole::SPIN_BOX; break;

        case WindowType::TOOLBOX: nRole = accessibility::AccessibleRole::TOOL_BAR; break;
        case WindowType::STATUSBAR: nRole = accessibility::AccessibleRole::STATUS_BAR; break;

        case WindowType::TABPAGE: nRole = accessibility::AccessibleRole::PANEL; break;
        case WindowType::TABCONTROL: nRole = accessibility::AccessibleRole::PAGE_TAB_LIST; break;

        case WindowType::DOCKINGWINDOW: nRole = (mpWindowImpl->mbFrame) ? accessibility::AccessibleRole::FRAME :
                                                                      accessibility::AccessibleRole::PANEL; break;

        case WindowType::FLOATINGWINDOW: nRole = ( mpWindowImpl->mbFrame ||
                                             (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) ||
                                             (GetStyle() & WB_OWNERDRAWDECORATION) ) ? accessibility::AccessibleRole::FRAME :
                                                                                       accessibility::AccessibleRole::WINDOW; break;

        case WindowType::WORKWINDOW: nRole = accessibility::AccessibleRole::ROOT_PANE; break;

        case WindowType::SCROLLBARBOX: nRole = accessibility::AccessibleRole::FILLER; break;

        case WindowType::HELPTEXTWINDOW: nRole = accessibility::AccessibleRole::TOOL_TIP; break;

        case WindowType::RULER: nRole = accessibility::AccessibleRole::RULER; break;

        case WindowType::SCROLLWINDOW: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

        case WindowType::WINDOW:
        case WindowType::CONTROL:
        case WindowType::BORDERWINDOW:
        case WindowType::SYSTEMCHILDWINDOW:
        default:
            if (ImplIsAccessibleNativeFrame() )
                nRole = accessibility::AccessibleRole::FRAME;
            else if( IsScrollable() )
                nRole = accessibility::AccessibleRole::SCROLL_PANE;
            else if( this->ImplGetWindow()->IsMenuFloatingWindow() )
                nRole = accessibility::AccessibleRole::WINDOW;      // #106002#, contextmenus are windows (i.e. toplevel)
            else
                // #104051# WINDOW seems to be a bad default role, use LAYEREDPANE instead
                // a WINDOW is interpreted as a top-level window, which is typically not the case
                //nRole = accessibility::AccessibleRole::WINDOW;
                nRole = accessibility::AccessibleRole::PANEL;
    }
    return nRole;
}

sal_uInt16 Window::GetAccessibleRole() const
{
    if (!mpWindowImpl)
        return 0;

    sal_uInt16 nRole = mpWindowImpl->mpAccessibleInfos ? mpWindowImpl->mpAccessibleInfos->nAccessibleRole : 0xFFFF;
    if ( nRole == 0xFFFF )
        nRole = getDefaultAccessibleRole();
    return nRole;
}

void Window::SetAccessibleName( const OUString& rName )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos.reset( new ImplAccessibleInfos );

    OUString oldName = GetAccessibleName();

    mpWindowImpl->mpAccessibleInfos->pAccessibleName = rName;

    CallEventListeners( VclEventId::WindowFrameTitleChanged, &oldName );
}

OUString Window::GetAccessibleName() const
{
    if (!mpWindowImpl)
        return OUString();

    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleName)
        return *mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    return getDefaultAccessibleName();
}

OUString Window::getDefaultAccessibleName() const
{
    OUString aAccessibleName;
    switch ( GetType() )
    {
        case WindowType::MULTILINEEDIT:
        case WindowType::PATTERNFIELD:
        case WindowType::NUMERICFIELD:
        case WindowType::METRICFIELD:
        case WindowType::CURRENCYFIELD:
        case WindowType::LONGCURRENCYFIELD:
        case WindowType::CALCINPUTLINE:
        case WindowType::EDIT:

        case WindowType::DATEBOX:
        case WindowType::TIMEBOX:
        case WindowType::CURRENCYBOX:
        case WindowType::LONGCURRENCYBOX:
        case WindowType::DATEFIELD:
        case WindowType::TIMEFIELD:
        case WindowType::SPINFIELD:

        case WindowType::COMBOBOX:
        case WindowType::LISTBOX:
        case WindowType::MULTILISTBOX:
        case WindowType::TREELISTBOX:
        case WindowType::METRICBOX:
        {
            vcl::Window *pLabel = GetAccessibleRelationLabeledBy();
            if ( pLabel && pLabel != this )
                aAccessibleName = pLabel->GetText();
            if (aAccessibleName.isEmpty())
                aAccessibleName = GetQuickHelpText();
        }
        break;

        case WindowType::IMAGEBUTTON:
        case WindowType::PUSHBUTTON:
            aAccessibleName = GetText();
            if (aAccessibleName.isEmpty())
            {
                aAccessibleName = GetQuickHelpText();
                if (aAccessibleName.isEmpty())
                    aAccessibleName = GetHelpText();
            }
        break;

        case WindowType::TOOLBOX:
            aAccessibleName = GetText();
            break;

        case WindowType::MOREBUTTON:
            aAccessibleName = mpWindowImpl->maText;
            break;

        default:
            aAccessibleName = GetText();
            break;
    }

    return GetNonMnemonicString( aAccessibleName );
}

void Window::SetAccessibleDescription( const OUString& rDescription )
{
    if ( ! mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos.reset( new ImplAccessibleInfos );

    SAL_WARN_IF( mpWindowImpl->mpAccessibleInfos->pAccessibleDescription, "vcl", "AccessibleDescription already set!" );
    mpWindowImpl->mpAccessibleInfos->pAccessibleDescription = rDescription;
}

OUString Window::GetAccessibleDescription() const
{
    if (!mpWindowImpl)
        return OUString();

    OUString aAccessibleDescription;
    if ( mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleDescription )
    {
        aAccessibleDescription = *mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    }
    else
    {
        // Special code for help text windows. ZT asks the border window for the
        // description so we have to forward this request to our inner window.
        const vcl::Window* pWin = this->ImplGetWindow();
        if ( pWin->GetType() == WindowType::HELPTEXTWINDOW )
            aAccessibleDescription = pWin->GetHelpText();
        else
            aAccessibleDescription = GetHelpText();
    }

    return aAccessibleDescription;
}

void Window::SetAccessibleRelationLabeledBy( vcl::Window* pLabeledBy )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos.reset( new ImplAccessibleInfos );
    mpWindowImpl->mpAccessibleInfos->pLabeledByWindow = pLabeledBy;
}

void Window::SetAccessibleRelationLabelFor( vcl::Window* pLabelFor )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos.reset( new ImplAccessibleInfos );
    mpWindowImpl->mpAccessibleInfos->pLabelForWindow = pLabelFor;
}

void Window::SetAccessibleRelationMemberOf( vcl::Window* pMemberOfWin )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos.reset( new ImplAccessibleInfos );
    mpWindowImpl->mpAccessibleInfos->pMemberOfWindow = pMemberOfWin;
}

vcl::Window* Window::GetAccessibleRelationMemberOf() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pMemberOfWindow)
        return mpWindowImpl->mpAccessibleInfos->pMemberOfWindow;

    if (!isContainerWindow(this) && !isContainerWindow(GetParent()))
        return getLegacyNonLayoutAccessibleRelationMemberOf();

    return nullptr;
}

vcl::Window* Window::getAccessibleRelationLabelFor() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pLabelForWindow)
        return mpWindowImpl->mpAccessibleInfos->pLabelForWindow;

    return nullptr;
}

vcl::Window* Window::GetAccessibleRelationLabelFor() const
{
    vcl::Window* pWindow = getAccessibleRelationLabelFor();

    if (pWindow)
        return pWindow;

    if (!isContainerWindow(this) && !isContainerWindow(GetParent()))
        return getLegacyNonLayoutAccessibleRelationLabelFor();

    return nullptr;
}

vcl::Window* Window::GetAccessibleRelationLabeledBy() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pLabeledByWindow)
        return mpWindowImpl->mpAccessibleInfos->pLabeledByWindow;

    std::vector<VclPtr<FixedText> > aMnemonicLabels(list_mnemonic_labels());
    if (!aMnemonicLabels.empty())
    {
        //if we have multiple labels, then prefer the first that is visible
        for (auto const & rCandidate : aMnemonicLabels)
        {
            if (rCandidate->IsVisible())
                return rCandidate;
        }
        return aMnemonicLabels[0];
    }

    if (!isContainerWindow(this) && !isContainerWindow(GetParent()))
        return getLegacyNonLayoutAccessibleRelationLabeledBy();

    return nullptr;
}

bool Window::IsAccessibilityEventsSuppressed( bool bTraverseParentPath )
{
    if( !bTraverseParentPath )
        return mpWindowImpl->mbSuppressAccessibilityEvents;
    else
    {
        vcl::Window *pParent = this;
        while ( pParent && pParent->mpWindowImpl)
        {
            if( pParent->mpWindowImpl->mbSuppressAccessibilityEvents )
                return true;
            else
                pParent = pParent->mpWindowImpl->mpParent; // do not use GetParent() to find borderwindows that are frames
        }
        return false;
    }
}

void Window::SetAccessibilityEventsSuppressed(bool bSuppressed)
{
    mpWindowImpl->mbSuppressAccessibilityEvents = bSuppressed;
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
