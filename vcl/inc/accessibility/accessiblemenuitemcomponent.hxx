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

#include <accessibility/accessiblemenubasecomponent.hxx>




class OAccessibleMenuItemComponent : public OAccessibleMenuBaseComponent
{
    friend class OAccessibleMenuBaseComponent;

protected:
    VclPtr<Menu>            m_pParent;
    sal_uInt16              m_nItemPos;
    OUString                m_sAccessibleName;
    OUString                m_sItemText;

    virtual bool            IsEnabled() override;
    virtual bool            IsVisible() override;
    void                    Select();
    void                    DeSelect();
    virtual void            Click() override;

    void                    SetItemPos( sal_uInt16 nItemPos );
    void                    SetAccessibleName( const OUString& sAccessibleName );
    OUString                GetAccessibleName();
    void                    SetItemText( const OUString& sItemText );
    OUString                GetItemText();

    virtual bool            IsMenuHideDisabledEntries() override;

    virtual void            FillAccessibleStateSet( sal_Int64& rStateSet ) override;

    // OAccessibleComponentHelper
    virtual css::awt::Rectangle implGetBounds(  ) override;

    // XComponent
    virtual void SAL_CALL   disposing() override;

public:
    OAccessibleMenuItemComponent( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu );
    virtual ~OAccessibleMenuItemComponent() override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual OUString SAL_CALL getToolTipText(  ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
