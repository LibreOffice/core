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

#ifndef INCLUDED_ACCESSIBILITY_INC_STANDARD_ACCESSIBLEMENUCOMPONENT_HXX
#define INCLUDED_ACCESSIBILITY_INC_STANDARD_ACCESSIBLEMENUCOMPONENT_HXX

#include <standard/accessiblemenubasecomponent.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <cppuhelper/implbase1.hxx>


//  class OAccessibleMenuComponent

typedef ::cppu::ImplHelper1<
    css::accessibility::XAccessibleSelection > OAccessibleMenuComponent_BASE;

class OAccessibleMenuComponent : public OAccessibleMenuBaseComponent,
                                 public OAccessibleMenuComponent_BASE
{
protected:
    virtual bool            IsEnabled() override;
    virtual bool            IsVisible() override;

    virtual void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds(  ) override;

public:
    OAccessibleMenuComponent( Menu* pMenu );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen(  ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) override;
    virtual OUString SAL_CALL getToolTipText(  ) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection(  ) override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nChildIndex ) override;
};

#endif // INCLUDED_ACCESSIBILITY_INC_STANDARD_ACCESSIBLEMENUCOMPONENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
