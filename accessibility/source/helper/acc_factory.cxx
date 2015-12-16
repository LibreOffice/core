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

#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/accessiblefactory.hxx>
#include <svtools/accessiblefactory.hxx>
#include <accessibility/standard/vclxaccessiblebutton.hxx>
#include <accessibility/standard/vclxaccessiblecheckbox.hxx>
#include <accessibility/standard/vclxaccessibledropdowncombobox.hxx>
#include <accessibility/standard/vclxaccessiblecombobox.hxx>
#include <accessibility/standard/vclxaccessibledropdownlistbox.hxx>
#include <accessibility/standard/vclxaccessibleedit.hxx>
#include <accessibility/standard/vclxaccessiblefixedhyperlink.hxx>
#include <accessibility/standard/vclxaccessiblefixedtext.hxx>
#include <accessibility/standard/vclxaccessiblelistbox.hxx>
#include <accessibility/standard/vclxaccessiblemenu.hxx>
#include <accessibility/standard/vclxaccessibleradiobutton.hxx>
#include <accessibility/standard/vclxaccessiblescrollbar.hxx>
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#include <accessibility/standard/vclxaccessibletoolbox.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <accessibility/standard/vclxaccessiblestatusbar.hxx>
#include <accessibility/standard/vclxaccessibletabcontrol.hxx>
#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#include <accessibility/standard/vclxaccessiblemenubar.hxx>
#include <accessibility/standard/vclxaccessiblepopupmenu.hxx>
#include <accessibility/extended/accessibletablistbox.hxx>
#include <accessibility/extended/AccessibleBrowseBox.hxx>
#include <accessibility/extended/accessibleiconchoicectrl.hxx>
#include <accessibility/extended/accessibletabbar.hxx>
#include <accessibility/extended/accessiblelistbox.hxx>
#include <accessibility/extended/AccessibleBrowseBoxHeaderBar.hxx>
#include <accessibility/extended/textwindowaccessibility.hxx>
#include <accessibility/extended/AccessibleBrowseBoxTableCell.hxx>
#include <accessibility/extended/AccessibleBrowseBoxHeaderCell.hxx>
#include <accessibility/extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <accessibility/extended/accessibleeditbrowseboxcell.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <accessibility/extended/AccessibleGridControl.hxx>
#include <svtools/accessibletable.hxx>
#include <vcl/popupmenuwindow.hxx>

#include "floatingwindowaccessible.hxx"

using namespace ::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;

namespace {

inline bool hasFloatingChild(vcl::Window *pWindow)
{
    vcl::Window * pChild = pWindow->GetAccessibleChildWindow(0);
    if( pChild && WINDOW_FLOATINGWINDOW == pChild->GetType() )
        return true;

    return false;
}

// IAccessibleFactory
class AccessibleFactory :public ::toolkit::IAccessibleFactory
                        ,public ::svt::IAccessibleFactory
{
public:
    AccessibleFactory();

    // ::toolkit::IAccessibleFactory
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXButton* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXCheckBox* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXRadioButton* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXListBox* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXFixedText* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXFixedHyperlink* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXScrollBar* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXEdit* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXComboBox* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXToolBox* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXWindow* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessible( Menu* _pMenu, bool _bIsMenuBar ) override;

    // ::svt::IAccessibleFactory
    virtual IAccessibleTabListBox*
        createAccessibleTabListBox(
            const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
            SvHeaderTabListBox& rBox
        ) const override;

    virtual IAccessibleBrowseBox*
        createAccessibleBrowseBox(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            IAccessibleTableProvider& _rBrowseBox
        ) const override;

    virtual IAccessibleTableControl*
        createAccessibleTableControl(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            IAccessibleTable& _rTable
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleIconChoiceCtrl(
            SvtIconChoiceCtrl& _rIconCtrl,
            const css::uno::Reference< css::accessibility::XAccessible >& _xParent
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleTabBar(
            TabBar& _rTabBar
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleTextWindowContext(
            VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleTreeListBox(
            SvTreeListBox& _rListBox,
            const css::uno::Reference< css::accessibility::XAccessible >& _xParent
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleBrowseBoxHeaderBar(
            const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
            IAccessibleTableProvider& _rOwningTable,
            AccessibleBrowseBoxObjType _eObjType
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleBrowseBoxTableCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            IAccessibleTableProvider& _rBrowseBox,
            const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowId,
            sal_uInt16 _nColId,
            sal_Int32 _nOffset
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleBrowseBoxHeaderCell(
            sal_Int32 _nColumnRowId,
            const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
            IAccessibleTableProvider& _rBrowseBox,
            const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
            AccessibleBrowseBoxObjType  _eObjType
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleCheckBoxCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            IAccessibleTableProvider& _rBrowseBox,
            const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos,
            const TriState& _eState,
            bool _bIsTriState
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createEditBrowseBoxTableCellAccess(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            const css::uno::Reference< css::accessibility::XAccessible >& _rxControlAccessible,
            const css::uno::Reference< css::awt::XWindow >& _rxFocusWindow,
            IAccessibleTableProvider& _rBrowseBox,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        ) const override;

protected:
    virtual ~AccessibleFactory();
};

AccessibleFactory::AccessibleFactory()
{
}

AccessibleFactory::~AccessibleFactory()
{
}

Reference< XAccessible > AccessibleFactory::createAccessible( Menu* _pMenu, bool _bIsMenuBar )
{
    OAccessibleMenuBaseComponent* pAccessible;
    if ( _bIsMenuBar )
        pAccessible = new VCLXAccessibleMenuBar( _pMenu );
    else
        pAccessible = new VCLXAccessiblePopupMenu( _pMenu );
    pAccessible->SetStates();
    return pAccessible;
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXButton* _pXWindow )
{
    return new VCLXAccessibleButton( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXCheckBox* _pXWindow )
{
    return new VCLXAccessibleCheckBox( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXRadioButton* _pXWindow )
{
    return new VCLXAccessibleRadioButton( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXListBox* _pXWindow )
{
    bool bIsDropDownBox = false;
    VclPtr< ListBox > pBox = _pXWindow->GetAs< ListBox >();
    if ( pBox )
        bIsDropDownBox = ( ( pBox->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN );

    if ( bIsDropDownBox )
        return new VCLXAccessibleDropDownListBox( _pXWindow );
    else
        return new VCLXAccessibleListBox( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXFixedText* _pXWindow )
{
    return new VCLXAccessibleFixedText( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXFixedHyperlink* _pXWindow )
{
    return new VCLXAccessibleFixedHyperlink( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXScrollBar* _pXWindow )
{
    return new VCLXAccessibleScrollBar( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXEdit* _pXWindow )
{
    return new VCLXAccessibleEdit( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXComboBox* _pXWindow )
{
    bool bIsDropDownBox = false;
    VclPtr< ComboBox > pBox = _pXWindow->GetAs< ComboBox >();
    if ( pBox )
        bIsDropDownBox = ( ( pBox->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN );

    if ( bIsDropDownBox )
        return new VCLXAccessibleDropDownComboBox( _pXWindow );
    else
        return new VCLXAccessibleComboBox( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXWindow* _pXWindow )
{
    Reference< XAccessibleContext > xContext;

    vcl::Window* pWindow = _pXWindow->GetWindow();
    if ( pWindow )
    {
        WindowType nType = pWindow->GetType();

        if ( nType == WINDOW_MENUBARWINDOW || pWindow->IsMenuFloatingWindow() || pWindow->IsToolbarFloatingWindow() )
        {
            Reference< XAccessible > xAcc( pWindow->GetAccessible() );
            if ( xAcc.is() )
            {
                Reference< XAccessibleContext > xCont( xAcc->getAccessibleContext() );
                if ( pWindow->GetType() == WINDOW_MENUBARWINDOW ||
                    ( xCont.is() && xCont->getAccessibleRole() == AccessibleRole::POPUP_MENU ) )
                {
                    xContext = xCont;
                }
            }
        }

        else if ( nType == WINDOW_STATUSBAR )
        {
            xContext = static_cast<XAccessibleContext*>(new VCLXAccessibleStatusBar( _pXWindow ));
        }

        else if ( nType == WINDOW_TABCONTROL )
        {
            xContext = static_cast<XAccessibleContext*>(new VCLXAccessibleTabControl( _pXWindow ));
        }

        else if ( nType == WINDOW_TABPAGE && pWindow->GetAccessibleParentWindow() && pWindow->GetAccessibleParentWindow()->GetType() == WINDOW_TABCONTROL )
        {
            xContext = new VCLXAccessibleTabPageWindow( _pXWindow );
        }

        else if ( nType == WINDOW_FLOATINGWINDOW )
        {
            xContext = new FloatingWindowAccessible( _pXWindow );
        }

        else if ( nType == WINDOW_BORDERWINDOW && hasFloatingChild( pWindow ) )
        {
            // The logic here has to match that of Window::GetAccessibleParentWindow in
            // vcl/source/window/window.cxx to avoid PopupMenuFloatingWindow
            // becoming a11y parents of themselves
            vcl::Window* pChild = pWindow->GetAccessibleChildWindow(0);
            if (PopupMenuFloatingWindow::isPopupMenu(pChild))
            {
                // Get the accessible context from the child window.
                Reference<XAccessible> xAccessible = pChild->CreateAccessible();
                if (xAccessible.is())
                    xContext = xAccessible->getAccessibleContext();
            }
            else
                xContext = new FloatingWindowAccessible( _pXWindow );
        }

        else if ( ( nType == WINDOW_HELPTEXTWINDOW ) || ( nType == WINDOW_FIXEDLINE ) )
        {
           xContext = static_cast<XAccessibleContext*>(new VCLXAccessibleFixedText( _pXWindow ));
        }
        else
        {
           xContext = static_cast<XAccessibleContext*>(new VCLXAccessibleComponent( _pXWindow ));
        }
    }
    return xContext;
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXToolBox* _pXWindow )
{
    return new VCLXAccessibleToolBox( _pXWindow );
}

IAccessibleTabListBox* AccessibleFactory::createAccessibleTabListBox(
    const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox ) const
{
    return new AccessibleTabListBox( rxParent, rBox );
}

IAccessibleBrowseBox* AccessibleFactory::createAccessibleBrowseBox(
    const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox ) const
{
    return new AccessibleBrowseBoxAccess( _rxParent, _rBrowseBox );
}

IAccessibleTableControl* AccessibleFactory::createAccessibleTableControl(
    const Reference< XAccessible >& _rxParent, IAccessibleTable& _rTable ) const
{
    return new AccessibleGridControlAccess( _rxParent, _rTable );
}

Reference< XAccessible > AccessibleFactory::createAccessibleIconChoiceCtrl(
    SvtIconChoiceCtrl& _rIconCtrl, const Reference< XAccessible >& _xParent ) const
{
    return new AccessibleIconChoiceCtrl( _rIconCtrl, _xParent );
}

Reference< XAccessible > AccessibleFactory::createAccessibleTabBar( TabBar& _rTabBar ) const
{
    return new AccessibleTabBar( &_rTabBar );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleTextWindowContext(
    VCLXWindow* pVclXWindow, TextEngine& rEngine, TextView& rView ) const
{
    return new Document( pVclXWindow, rEngine, rView );
}

Reference< XAccessible > AccessibleFactory::createAccessibleTreeListBox(
    SvTreeListBox& _rListBox, const Reference< XAccessible >& _xParent ) const
{
    return new AccessibleListBox( _rListBox, _xParent );
}

Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxHeaderBar(
    const Reference< XAccessible >& rxParent, IAccessibleTableProvider& _rOwningTable,
    AccessibleBrowseBoxObjType _eObjType ) const
{
    return new AccessibleBrowseBoxHeaderBar( rxParent, _rOwningTable, _eObjType );
}

Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxTableCell(
    const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox,
    const Reference< XWindow >& _xFocusWindow, sal_Int32 _nRowId, sal_uInt16 _nColId, sal_Int32 _nOffset ) const
{
    return new AccessibleBrowseBoxTableCell( _rxParent, _rBrowseBox, _xFocusWindow,
        _nRowId, _nColId, _nOffset );
}

Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxHeaderCell(
    sal_Int32 _nColumnRowId, const Reference< XAccessible >& rxParent, IAccessibleTableProvider& _rBrowseBox,
    const Reference< XWindow >& _xFocusWindow, AccessibleBrowseBoxObjType  _eObjType ) const
{
    return new AccessibleBrowseBoxHeaderCell( _nColumnRowId, rxParent, _rBrowseBox,
        _xFocusWindow, _eObjType );
}

Reference< XAccessible > AccessibleFactory::createAccessibleCheckBoxCell(
    const Reference< XAccessible >& _rxParent, IAccessibleTableProvider& _rBrowseBox,
    const Reference< XWindow >& _xFocusWindow, sal_Int32 _nRowPos, sal_uInt16 _nColPos,
    const TriState& _eState, bool _bIsTriState ) const
{
    return new AccessibleCheckBoxCell( _rxParent, _rBrowseBox, _xFocusWindow,
        _nRowPos, _nColPos, _eState, _bIsTriState );
}

Reference< XAccessible > AccessibleFactory::createEditBrowseBoxTableCellAccess(
    const Reference< XAccessible >& _rxParent, const Reference< XAccessible >& _rxControlAccessible,
    const Reference< XWindow >& _rxFocusWindow, IAccessibleTableProvider& _rBrowseBox,
    sal_Int32 _nRowPos, sal_uInt16 _nColPos ) const
{
    return new EditBrowseBoxTableCellAccess( _rxParent, _rxControlAccessible,
        _rxFocusWindow, _rBrowseBox, _nRowPos, _nColPos );
}

} // anonymous namespace

#if HAVE_FEATURE_DESKTOP
/* this is the entry point to retrieve a factory for the toolkit-level Accessible/Contexts supplied
    by this library

    This function implements the factory function needed in toolkit
    (of type GetStandardAccComponentFactory).
*/
extern "C"
{
    SAL_DLLPUBLIC_EXPORT void* SAL_CALL getStandardAccessibleFactory()
    {
        ::toolkit::IAccessibleFactory* pFactory = new AccessibleFactory;
        pFactory->acquire();
        return pFactory;
    }
}

/** this is the entry point to retrieve a factory for the svtools-level Accessible/Contexts supplied
    by this library

    This function implements the factory function needed in svtools
    (of type GetSvtAccessibilityComponentFactory).
*/
extern "C"
{
    SAL_DLLPUBLIC_EXPORT void* SAL_CALL getSvtAccessibilityComponentFactory()
    {
        ::svt::IAccessibleFactory* pFactory = new AccessibleFactory;
        pFactory->acquire();
        return pFactory;
    }
}
#endif // HAVE_FEATURE_DESKTOP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
