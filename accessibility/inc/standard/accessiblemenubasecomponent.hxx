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

#pragma once

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

#include <vector>

class Menu;
class VclSimpleEvent;
class VclMenuEvent;

class OAccessibleMenuItemComponent;

class OAccessibleMenuBaseComponent : public cppu::ImplInheritanceHelper<
                                         comphelper::OAccessibleExtendedComponentHelper,
                                         css::accessibility::XAccessible,
                                         css::lang::XServiceInfo>
{
    friend class OAccessibleMenuItemComponent;
    friend class VCLXAccessibleMenuItem;
    friend class VCLXAccessibleMenu;

protected:
    std::vector<rtl::Reference<OAccessibleMenuItemComponent>> m_aAccessibleChildren;
    VclPtr<Menu>            m_pMenu;

    bool                    m_bEnabled;
    bool                    m_bFocused;
    bool                    m_bVisible;
    bool                    m_bSelected;
    bool                    m_bChecked;

    virtual bool            IsEnabled();
    virtual bool            IsFocused();
    virtual bool            IsVisible();
    virtual bool            IsSelected();
    virtual bool            IsChecked();

    void                    SetEnabled( bool bEnabled );
    void                    SetFocused( bool bFocused );
    void                    SetVisible( bool bVisible );
    void                    SetSelected( bool bSelected );
    void                    SetChecked( bool bChecked );

    void                    UpdateEnabled( sal_Int32 i, bool bEnabled );
    void                    UpdateFocused( sal_Int32 i, bool bFocused );
    void                    UpdateVisible();
    void                    UpdateSelected( sal_Int32 i, bool bSelected );
    void                    UpdateChecked( sal_Int32 i, bool bChecked );
    void                    UpdateAccessibleName( sal_Int32 i );
    void                    UpdateItemRole(sal_Int32 i);
    void                    UpdateItemText( sal_Int32 i );

    sal_Int64               GetChildCount() const;

    css::uno::Reference< css::accessibility::XAccessible > GetChild( sal_Int64 i );
    css::uno::Reference< css::accessibility::XAccessible > GetChildAt( const css::awt::Point& rPoint );

    void                    InsertChild( sal_Int32 i );
    void                    RemoveChild( sal_Int32 i );

    virtual bool        IsHighlighted();
    bool                IsChildHighlighted();

    virtual bool        IsMenuHideDisabledEntries();

    void                    SelectChild( sal_Int32 i );
    void                    DeSelectAll();
    bool                IsChildSelected( sal_Int32 i );

    virtual void            Click();
    virtual bool            IsPopupMenuOpen();

    DECL_LINK( MenuEventListener, VclMenuEvent&, void );

    void                    ProcessMenuEvent( const VclMenuEvent& rVclMenuEvent );

    virtual void            FillAccessibleStateSet( sal_Int64& rStateSet ) = 0;

    // XComponent
    virtual void SAL_CALL   disposing() override;

public:
    OAccessibleMenuBaseComponent( Menu* pMenu );
    virtual ~OAccessibleMenuBaseComponent() override;

    void                    SetStates();

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
