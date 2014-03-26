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

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUITEMCOMPONENT_HXX
#define ACCESSIBILITY_STANDARD_ACCESSIBLEMENUITEMCOMPONENT_HXX

#include <accessibility/standard/accessiblemenubasecomponent.hxx>



//  class OAccessibleMenuItemComponent


class OAccessibleMenuItemComponent : public OAccessibleMenuBaseComponent
{
    friend class OAccessibleMenuBaseComponent;

protected:
    Menu*                   m_pParent;
    sal_uInt16              m_nItemPos;
    OUString                m_sAccessibleName;
    OUString                m_sItemText;

    virtual sal_Bool        IsEnabled() SAL_OVERRIDE;
    virtual sal_Bool        IsVisible() SAL_OVERRIDE;
    virtual void            Select() SAL_OVERRIDE;
    virtual void            DeSelect() SAL_OVERRIDE;
    virtual void            Click() SAL_OVERRIDE;

    void                    SetItemPos( sal_uInt16 nItemPos );
    void                    SetAccessibleName( const OUString& sAccessibleName );
    OUString                GetAccessibleName();
    void                    SetItemText( const OUString& sItemText );
    OUString                GetItemText();

    virtual sal_Bool        IsMenuHideDisabledEntries() SAL_OVERRIDE;

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) SAL_OVERRIDE;

    // OCommonAccessibleComponent
    virtual ::com::sun::star::awt::Rectangle implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // XComponent
    virtual void SAL_CALL   disposing() SAL_OVERRIDE;

public:
    OAccessibleMenuItemComponent( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu );
    virtual ~OAccessibleMenuItemComponent();

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif // ACCESSIBILITY_STANDARD_ACCESSIBLEMENUITEMCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
