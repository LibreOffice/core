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

#include <accessibility/vclxaccessibletextcomponent.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <cppuhelper/implbase.hxx>
#include <vcl/toolkit/button.hxx>

class VCLXAccessibleRadioButton final
    : public cppu::ImplInheritanceHelper<
          VCLXAccessibleTextComponent, css::accessibility::XAccessible,
          css::accessibility::XAccessibleAction, css::accessibility::XAccessibleValue>
{
    virtual ~VCLXAccessibleRadioButton() override = default;

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet ) override;
    virtual void FillAccessibleStateSet( sal_Int64& rStateSet ) override;

public:
    VCLXAccessibleRadioButton(RadioButton* pRadioButton)
        : ImplInheritanceHelper(pRadioButton) {}

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAccessible
    virtual css::uno::Reference<com::sun::star::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL getCurrentValue(  ) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) override;
    virtual css::uno::Any SAL_CALL getMaximumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumValue(  ) override;
    virtual css::uno::Any SAL_CALL getMinimumIncrement( ) override;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
