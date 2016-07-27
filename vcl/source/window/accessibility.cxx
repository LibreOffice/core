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

#include "tools/time.hxx"
#include "tools/debug.hxx"
#include "tools/rc.h"

#include "unotools/fontcfg.hxx"
#include "unotools/confignode.hxx"

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
#include <vcl/unowrap.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/popupmenuwindow.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

// declare system types in sysdata.hxx
#include <vcl/sysdata.hxx>

#include "salframe.hxx"
#include "salobj.hxx"
#include "salinst.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "fontinstance.hxx"
#include "window.h"
#include "toolbox.h"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "brdwin.hxx"
#include "helpwin.hxx"
#include "sallayout.hxx"
#include "dndlistenercontainer.hxx"
#include "dndeventdispatcher.hxx"

#include "com/sun/star/accessibility/XAccessible.hpp"
#include "com/sun/star/accessibility/AccessibleRole.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/SystemClipboard.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "comphelper/processfactory.hxx"

#include <sal/macros.h>
#include <rtl/strbuf.hxx>

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
    pAccessibleName = nullptr;
    pAccessibleDescription = nullptr;
    pLabeledByWindow = nullptr;
    pLabelForWindow = nullptr;
    pMemberOfWindow = nullptr;
}

ImplAccessibleInfos::~ImplAccessibleInfos()
{
    delete pAccessibleName;
    delete pAccessibleDescription;
}

namespace vcl {

css::uno::Reference< css::accessibility::XAccessible > Window::GetAccessible( bool bCreate )
{
    // do not optimize hierarchy for the top level border win (ie, when there is no parent)
    /* // do not optimize accessible hierarchy at all to better reflect real VCL hierarchy
    if ( GetParent() && ( GetType() == WINDOW_BORDERWINDOW ) && ( GetChildCount() == 1 ) )
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
    assert(mpWindowImpl);
    mpWindowImpl->mxAccessible = x;
}

// skip all border windows that are no top level frames
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
        if( (mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE)) )
            return true;
        else
            return false;
    else
        return false;
}

vcl::Window* Window::GetAccessibleParentWindow() const
{
    if ( ImplIsAccessibleNativeFrame() )
        return nullptr;

    vcl::Window* pParent = mpWindowImpl->mpParent;
    if( GetType() == WINDOW_MENUBARWINDOW )
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
    else if( GetType() == WINDOW_FLOATINGWINDOW &&
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
    sal_uInt16 nChildren = 0;
    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->IsVisible() )
            nChildren++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    // report the menubarwindow as a child of THE workwindow
    if( GetType() == WINDOW_BORDERWINDOW )
    {
        ImplBorderWindow *pBorderWindow = static_cast<ImplBorderWindow*>(this);
        if( pBorderWindow->mpMenuBarWindow &&
            pBorderWindow->mpMenuBarWindow->IsVisible()
            )
            --nChildren;
    }
    else if( GetType() == WINDOW_WORKWINDOW )
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
    if( GetType() == WINDOW_WORKWINDOW && static_cast<WorkWindow *>(this)->GetMenuBar() )
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

    if( GetType() == WINDOW_BORDERWINDOW && pChild && pChild->GetType() == WINDOW_MENUBARWINDOW )
    {
        do pChild = pChild->mpWindowImpl->mpNext; while( pChild && ! pChild->IsVisible() );
        SAL_WARN_IF( !pChild, "vcl", "GetAccessibleChildWindow(): wrong index in border window");
    }

    if ( pChild && ( pChild->GetType() == WINDOW_BORDERWINDOW ) && ( pChild->GetChildCount() == 1 ) )
    {
        pChild = pChild->GetChild( 0 );
    }
    return pChild;
}

void Window::SetAccessibleRole( sal_uInt16 nRole )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    SAL_WARN_IF( mpWindowImpl->mpAccessibleInfos->nAccessibleRole != 0xFFFF, "vcl", "AccessibleRole already set!" );
    mpWindowImpl->mpAccessibleInfos->nAccessibleRole = nRole;
}

sal_uInt16 Window::getDefaultAccessibleRole() const
{
    sal_uInt16 nRole = 0xFFFF;
    switch ( GetType() )
    {
        case WINDOW_MESSBOX:    // MT: Would be nice to have special roles!
        case WINDOW_INFOBOX:
        case WINDOW_WARNINGBOX:
        case WINDOW_ERRORBOX:
        case WINDOW_QUERYBOX: nRole = accessibility::AccessibleRole::ALERT; break;

        case WINDOW_MODELESSDIALOG:
        case WINDOW_MODALDIALOG:
        case WINDOW_SYSTEMDIALOG:
        case WINDOW_PRINTERSETUPDIALOG:
        case WINDOW_PRINTDIALOG:
        case WINDOW_TABDIALOG:
        case WINDOW_BUTTONDIALOG:
        case WINDOW_DIALOG: nRole = accessibility::AccessibleRole::DIALOG; break;

        case WINDOW_PUSHBUTTON:
        case WINDOW_OKBUTTON:
        case WINDOW_CANCELBUTTON:
        case WINDOW_HELPBUTTON:
        case WINDOW_IMAGEBUTTON:
        case WINDOW_MOREBUTTON:
        case WINDOW_SPINBUTTON:
        case WINDOW_BUTTON: nRole = accessibility::AccessibleRole::PUSH_BUTTON; break;
        case WINDOW_MENUBUTTON: nRole = accessibility::AccessibleRole::BUTTON_MENU; break;

        case WINDOW_PATHDIALOG: nRole = accessibility::AccessibleRole::DIRECTORY_PANE; break;
        case WINDOW_FILEDIALOG: nRole = accessibility::AccessibleRole::FILE_CHOOSER; break;
        case WINDOW_COLORDIALOG: nRole = accessibility::AccessibleRole::COLOR_CHOOSER; break;
        case WINDOW_FONTDIALOG: nRole = accessibility::AccessibleRole::FONT_CHOOSER; break;

        case WINDOW_RADIOBUTTON: nRole = accessibility::AccessibleRole::RADIO_BUTTON; break;
        case WINDOW_TRISTATEBOX:
        case WINDOW_CHECKBOX: nRole = accessibility::AccessibleRole::CHECK_BOX; break;

        case WINDOW_MULTILINEEDIT: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

        case WINDOW_PATTERNFIELD:
        case WINDOW_CALCINPUTLINE:
        case WINDOW_EDIT: nRole = ( GetStyle() & WB_PASSWORD ) ? (accessibility::AccessibleRole::PASSWORD_TEXT) : (accessibility::AccessibleRole::TEXT); break;

        case WINDOW_PATTERNBOX:
        case WINDOW_NUMERICBOX:
        case WINDOW_METRICBOX:
        case WINDOW_CURRENCYBOX:
        case WINDOW_LONGCURRENCYBOX:
        case WINDOW_COMBOBOX: nRole = accessibility::AccessibleRole::COMBO_BOX; break;

        case WINDOW_LISTBOX:
        case WINDOW_MULTILISTBOX: nRole = accessibility::AccessibleRole::LIST; break;

        case WINDOW_TREELISTBOX: nRole = accessibility::AccessibleRole::TREE; break;

        case WINDOW_FIXEDTEXT: nRole = accessibility::AccessibleRole::LABEL; break;
        case WINDOW_FIXEDLINE:
            if( !GetText().isEmpty() )
                nRole = accessibility::AccessibleRole::LABEL;
            else
                nRole = accessibility::AccessibleRole::SEPARATOR;
            break;

        case WINDOW_FIXEDBITMAP:
        case WINDOW_FIXEDIMAGE: nRole = accessibility::AccessibleRole::ICON; break;
        case WINDOW_GROUPBOX: nRole = accessibility::AccessibleRole::GROUP_BOX; break;
        case WINDOW_SCROLLBAR: nRole = accessibility::AccessibleRole::SCROLL_BAR; break;

        case WINDOW_SLIDER:
        case WINDOW_SPLITTER:
        case WINDOW_SPLITWINDOW: nRole = accessibility::AccessibleRole::SPLIT_PANE; break;

        case WINDOW_DATEBOX:
        case WINDOW_TIMEBOX:
        case WINDOW_DATEFIELD:
        case WINDOW_TIMEFIELD: nRole = accessibility::AccessibleRole::DATE_EDITOR; break;

        case WINDOW_NUMERICFIELD:
        case WINDOW_METRICFIELD:
        case WINDOW_CURRENCYFIELD:
        case WINDOW_LONGCURRENCYFIELD:
        case WINDOW_SPINFIELD: nRole = accessibility::AccessibleRole::SPIN_BOX; break;

        case WINDOW_TOOLBOX: nRole = accessibility::AccessibleRole::TOOL_BAR; break;
        case WINDOW_STATUSBAR: nRole = accessibility::AccessibleRole::STATUS_BAR; break;

        case WINDOW_TABPAGE: nRole = accessibility::AccessibleRole::PANEL; break;
        case WINDOW_TABCONTROL: nRole = accessibility::AccessibleRole::PAGE_TAB_LIST; break;

        case WINDOW_DOCKINGWINDOW:
        case WINDOW_SYSWINDOW:      nRole = (mpWindowImpl->mbFrame) ? accessibility::AccessibleRole::FRAME :
                                                                      accessibility::AccessibleRole::PANEL; break;

        case WINDOW_FLOATINGWINDOW: nRole = ( mpWindowImpl->mbFrame ||
                                             (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) ||
                                             (GetStyle() & WB_OWNERDRAWDECORATION) ) ? accessibility::AccessibleRole::FRAME :
                                                                                       accessibility::AccessibleRole::WINDOW; break;

        case WINDOW_WORKWINDOW: nRole = accessibility::AccessibleRole::ROOT_PANE; break;

        case WINDOW_SCROLLBARBOX: nRole = accessibility::AccessibleRole::FILLER; break;

        case WINDOW_HELPTEXTWINDOW: nRole = accessibility::AccessibleRole::TOOL_TIP; break;

        case WINDOW_RULER: nRole = accessibility::AccessibleRole::RULER; break;

        case WINDOW_SCROLLWINDOW: nRole = accessibility::AccessibleRole::SCROLL_PANE; break;

        case WINDOW_WINDOW:
        case WINDOW_CONTROL:
        case WINDOW_BORDERWINDOW:
        case WINDOW_SYSTEMCHILDWINDOW:
        default:
            if (ImplIsAccessibleNativeFrame() )
                nRole = accessibility::AccessibleRole::FRAME;
            else if( IsScrollable() )
                nRole = accessibility::AccessibleRole::SCROLL_PANE;
            else if( const_cast<vcl::Window*>(this)->ImplGetWindow()->IsMenuFloatingWindow() )
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
    using namespace ::com::sun::star;

    sal_uInt16 nRole = mpWindowImpl->mpAccessibleInfos ? mpWindowImpl->mpAccessibleInfos->nAccessibleRole : 0xFFFF;
    if ( nRole == 0xFFFF )
        nRole = getDefaultAccessibleRole();
    return nRole;
}

void Window::SetAccessibleName( const OUString& rName )
{
   if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    OUString oldName = GetAccessibleName();

    delete mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    mpWindowImpl->mpAccessibleInfos->pAccessibleName = new OUString( rName );

    CallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldName );
}

OUString Window::GetAccessibleName() const
{
    if (mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleName)
        return *mpWindowImpl->mpAccessibleInfos->pAccessibleName;
    return getDefaultAccessibleName();
}

OUString Window::getDefaultAccessibleName() const
{
    OUString aAccessibleName;
    switch ( GetType() )
    {
        case WINDOW_MULTILINEEDIT:
        case WINDOW_PATTERNFIELD:
        case WINDOW_NUMERICFIELD:
        case WINDOW_METRICFIELD:
        case WINDOW_CURRENCYFIELD:
        case WINDOW_LONGCURRENCYFIELD:
        case WINDOW_CALCINPUTLINE:
        case WINDOW_EDIT:

        case WINDOW_DATEBOX:
        case WINDOW_TIMEBOX:
        case WINDOW_CURRENCYBOX:
        case WINDOW_LONGCURRENCYBOX:
        case WINDOW_DATEFIELD:
        case WINDOW_TIMEFIELD:
        case WINDOW_SPINFIELD:

        case WINDOW_COMBOBOX:
        case WINDOW_LISTBOX:
        case WINDOW_MULTILISTBOX:
        case WINDOW_TREELISTBOX:
        case WINDOW_METRICBOX:
        {
            vcl::Window *pLabel = GetAccessibleRelationLabeledBy();
            if ( pLabel && pLabel != this )
                aAccessibleName = pLabel->GetText();
            if (aAccessibleName.isEmpty())
                aAccessibleName = GetQuickHelpText();
        }
        break;

        case WINDOW_IMAGEBUTTON:
        case WINDOW_PUSHBUTTON:
            aAccessibleName = GetText();
            if (aAccessibleName.isEmpty())
            {
                aAccessibleName = GetQuickHelpText();
                if (aAccessibleName.isEmpty())
                    aAccessibleName = GetHelpText();
            }
        break;

        case WINDOW_TOOLBOX:
            aAccessibleName = GetText();
            break;

        case WINDOW_MOREBUTTON:
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
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;

    SAL_WARN_IF( mpWindowImpl->mpAccessibleInfos->pAccessibleDescription, "vcl", "AccessibleDescription already set!" );
    delete mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    mpWindowImpl->mpAccessibleInfos->pAccessibleDescription = new OUString( rDescription );
}

OUString Window::GetAccessibleDescription() const
{
    OUString aAccessibleDescription;
    if ( mpWindowImpl->mpAccessibleInfos && mpWindowImpl->mpAccessibleInfos->pAccessibleDescription )
    {
        aAccessibleDescription = *mpWindowImpl->mpAccessibleInfos->pAccessibleDescription;
    }
    else
    {
        // Special code for help text windows. ZT asks the border window for the
        // description so we have to forward this request to our inner window.
        const vcl::Window* pWin = const_cast<vcl::Window *>(this)->ImplGetWindow();
        if ( pWin->GetType() == WINDOW_HELPTEXTWINDOW )
            aAccessibleDescription = pWin->GetHelpText();
        else
            aAccessibleDescription = GetHelpText();
    }

    return aAccessibleDescription;
}

void Window::SetAccessibleRelationLabeledBy( vcl::Window* pLabeledBy )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
    mpWindowImpl->mpAccessibleInfos->pLabeledByWindow = pLabeledBy;
}

void Window::SetAccessibleRelationLabelFor( vcl::Window* pLabelFor )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
    mpWindowImpl->mpAccessibleInfos->pLabelForWindow = pLabelFor;
}

void Window::SetAccessibleRelationMemberOf( vcl::Window* pMemberOfWin )
{
    if ( !mpWindowImpl->mpAccessibleInfos )
        mpWindowImpl->mpAccessibleInfos = new ImplAccessibleInfos;
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
        for (auto aI = aMnemonicLabels.begin(), aEnd = aMnemonicLabels.end(); aI != aEnd; ++aI)
        {
            vcl::Window *pCandidate = *aI;
            if (pCandidate->IsVisible())
                return pCandidate;
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
