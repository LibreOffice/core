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
#include <vcl/accessiblefactory.hxx>
#include <standard/vclxaccessiblebutton.hxx>
#include <standard/vclxaccessiblecheckbox.hxx>
#include <standard/vclxaccessibledropdowncombobox.hxx>
#include <standard/vclxaccessiblecombobox.hxx>
#include <standard/vclxaccessibledropdownlistbox.hxx>
#include <standard/vclxaccessibleedit.hxx>
#include <standard/vclxaccessiblefixedhyperlink.hxx>
#include <standard/vclxaccessiblefixedtext.hxx>
#include <standard/vclxaccessibleheaderbar.hxx>
#include <standard/vclxaccessiblelistbox.hxx>
#include <standard/vclxaccessiblemenu.hxx>
#include <standard/vclxaccessibleradiobutton.hxx>
#include <standard/vclxaccessiblescrollbar.hxx>
#include <standard/vclxaccessibletextcomponent.hxx>
#include <standard/vclxaccessibletoolbox.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <standard/vclxaccessiblestatusbar.hxx>
#include <standard/vclxaccessibletabcontrol.hxx>
#include <standard/vclxaccessibletabpagewindow.hxx>
#include <standard/vclxaccessiblemenubar.hxx>
#include <standard/vclxaccessiblepopupmenu.hxx>
#include <extended/accessibletablistbox.hxx>
#include <extended/AccessibleBrowseBox.hxx>
#include <extended/accessibleiconchoicectrl.hxx>
#include <extended/accessibletabbar.hxx>
#include <extended/accessiblelistbox.hxx>
#include <extended/AccessibleBrowseBoxHeaderBar.hxx>
#include <extended/textwindowaccessibility.hxx>
#include <extended/AccessibleBrowseBoxTableCell.hxx>
#include <extended/AccessibleBrowseBoxHeaderCell.hxx>
#include <extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <extended/accessibleeditbrowseboxcell.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <extended/AccessibleGridControl.hxx>
#include <vcl/accessibletable.hxx>
#include <vcl/popupmenuwindow.hxx>

#include <floatingwindowaccessible.hxx>

using namespace ::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;
using namespace ::vcl::table;

namespace {

bool hasFloatingChild(vcl::Window *pWindow)
{
    vcl::Window * pChild = pWindow->GetAccessibleChildWindow(0);
    return pChild && pChild->GetType() == WindowType::FLOATINGWINDOW;
}

// IAccessibleFactory
class AccessibleFactory :public ::toolkit::IAccessibleFactory
                        ,public ::vcl::IAccessibleFactory
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
        createAccessibleContext( VCLXHeaderBar* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        createAccessibleContext( VCLXWindow* _pXWindow ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessible( Menu* _pMenu, bool _bIsMenuBar ) override;

    // ::vcl::IAccessibleFactory
    virtual vcl::IAccessibleTabListBox*
        createAccessibleTabListBox(
            const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
            SvHeaderTabListBox& rBox
        ) const override;

    virtual vcl::IAccessibleBrowseBox*
        createAccessibleBrowseBox(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            vcl::IAccessibleTableProvider& _rBrowseBox
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
            vcl::IAccessibleTableProvider& _rOwningTable,
            vcl::AccessibleBrowseBoxObjType _eObjType
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleBrowseBoxTableCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            vcl::IAccessibleTableProvider& _rBrowseBox,
            const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
            sal_Int32 _nRowId,
            sal_uInt16 _nColId,
            sal_Int32 _nOffset
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleBrowseBoxHeaderCell(
            sal_Int32 _nColumnRowId,
            const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
            vcl::IAccessibleTableProvider& _rBrowseBox,
            const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
            vcl::AccessibleBrowseBoxObjType  _eObjType
        ) const override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
        createAccessibleCheckBoxCell(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
            vcl::IAccessibleTableProvider& _rBrowseBox,
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
            vcl::IAccessibleTableProvider& _rBrowseBox,
            sal_Int32 _nRowPos,
            sal_uInt16 _nColPos
        ) const override;

protected:
    virtual ~AccessibleFactory() override;
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

    VclPtr<vcl::Window> pWindow = _pXWindow->GetWindow();
    if ( pWindow )
    {
        WindowType nType = pWindow->GetType();

        if ( nType == WindowType::MENUBARWINDOW || pWindow->IsMenuFloatingWindow() || pWindow->IsToolbarFloatingWindow() )
        {
            Reference< XAccessible > xAcc( pWindow->GetAccessible() );
            if ( xAcc.is() )
            {
                Reference< XAccessibleContext > xCont( xAcc->getAccessibleContext() );
                if ( pWindow->GetType() == WindowType::MENUBARWINDOW ||
                    ( xCont.is() && xCont->getAccessibleRole() == AccessibleRole::POPUP_MENU ) )
                {
                    xContext = xCont;
                }
            }
        }

        else if ( nType == WindowType::STATUSBAR )
        {
            xContext = new VCLXAccessibleStatusBar(_pXWindow);
        }

        else if ( nType == WindowType::TABCONTROL )
        {
            xContext = new VCLXAccessibleTabControl(_pXWindow);
        }

        else if ( nType == WindowType::TABPAGE && pWindow->GetAccessibleParentWindow() && pWindow->GetAccessibleParentWindow()->GetType() == WindowType::TABCONTROL )
        {
            xContext = new VCLXAccessibleTabPageWindow( _pXWindow );
        }

        else if ( nType == WindowType::FLOATINGWINDOW )
        {
            xContext = new FloatingWindowAccessible( _pXWindow );
        }

        else if ( nType == WindowType::BORDERWINDOW && hasFloatingChild( pWindow ) )
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

        else if ( ( nType == WindowType::HELPTEXTWINDOW ) || ( nType == WindowType::FIXEDLINE ) )
        {
            xContext = new VCLXAccessibleFixedText(_pXWindow);
        }
        else
        {
            xContext = new VCLXAccessibleComponent(_pXWindow);
        }
    }
    return xContext;
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXToolBox* _pXWindow )
{
    return new VCLXAccessibleToolBox( _pXWindow );
}

Reference< XAccessibleContext > AccessibleFactory::createAccessibleContext( VCLXHeaderBar* _pXWindow )
{
    return new VCLXAccessibleHeaderBar(_pXWindow);
}

vcl::IAccessibleTabListBox* AccessibleFactory::createAccessibleTabListBox(
    const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox ) const
{
    return new AccessibleTabListBox( rxParent, rBox );
}

vcl::IAccessibleBrowseBox* AccessibleFactory::createAccessibleBrowseBox(
    const Reference< XAccessible >& _rxParent, vcl::IAccessibleTableProvider& _rBrowseBox ) const
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
    const Reference< XAccessible >& rxParent, vcl::IAccessibleTableProvider& _rOwningTable,
    vcl::AccessibleBrowseBoxObjType _eObjType ) const
{
    return new AccessibleBrowseBoxHeaderBar( rxParent, _rOwningTable, _eObjType );
}

Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxTableCell(
    const Reference< XAccessible >& _rxParent, vcl::IAccessibleTableProvider& _rBrowseBox,
    const Reference< XWindow >& _xFocusWindow, sal_Int32 _nRowId, sal_uInt16 _nColId, sal_Int32 _nOffset ) const
{
    return new AccessibleBrowseBoxTableCell( _rxParent, _rBrowseBox, _xFocusWindow,
        _nRowId, _nColId, _nOffset );
}

Reference< XAccessible > AccessibleFactory::createAccessibleBrowseBoxHeaderCell(
    sal_Int32 _nColumnRowId, const Reference< XAccessible >& rxParent, vcl::IAccessibleTableProvider& _rBrowseBox,
    const Reference< XWindow >& _xFocusWindow, vcl::AccessibleBrowseBoxObjType  _eObjType ) const
{
    return new AccessibleBrowseBoxHeaderCell( _nColumnRowId, rxParent, _rBrowseBox,
        _xFocusWindow, _eObjType );
}

Reference< XAccessible > AccessibleFactory::createAccessibleCheckBoxCell(
    const Reference< XAccessible >& _rxParent, vcl::IAccessibleTableProvider& _rBrowseBox,
    const Reference< XWindow >& _xFocusWindow, sal_Int32 _nRowPos, sal_uInt16 _nColPos,
    const TriState& _eState, bool _bIsTriState ) const
{
    return new AccessibleCheckBoxCell( _rxParent, _rBrowseBox, _xFocusWindow,
        _nRowPos, _nColPos, _eState, _bIsTriState );
}

Reference< XAccessible > AccessibleFactory::createEditBrowseBoxTableCellAccess(
    const Reference< XAccessible >& _rxParent, const Reference< XAccessible >& _rxControlAccessible,
    const Reference< XWindow >& _rxFocusWindow, vcl::IAccessibleTableProvider& _rBrowseBox,
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
    SAL_DLLPUBLIC_EXPORT void* getStandardAccessibleFactory()
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
    SAL_DLLPUBLIC_EXPORT void* getSvtAccessibilityComponentFactory()
    {
        ::vcl::IAccessibleFactory* pFactory = new AccessibleFactory;
        pFactory->acquire();
        return pFactory;
    }
}
#endif // HAVE_FEATURE_DESKTOP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
