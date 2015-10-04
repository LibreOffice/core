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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLECHECKBOX_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLECHECKBOX_HXX

#include <accessibility/standard/vclxaccessibletextcomponent.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <cppuhelper/implbase.hxx>



//  class VCLXAccessibleCheckBox


typedef ::cppu::ImplHelper<
    css::accessibility::XAccessibleAction,
    css::accessibility::XAccessibleValue > VCLXAccessibleCheckBox_BASE;

class VCLXAccessibleCheckBox : public VCLXAccessibleTextComponent,
                               public VCLXAccessibleCheckBox_BASE
{
private:
    bool    m_bChecked;
    bool    m_bIndeterminate;

protected:
    virtual ~VCLXAccessibleCheckBox();

    bool    IsChecked();
    bool    IsIndeterminate();

    void    SetChecked( bool bChecked );
    void    SetIndeterminate( bool bIndeterminate );

    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
    virtual void FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet ) override;

public:
    VCLXAccessibleCheckBox( VCLXWindow* pVCLXindow );

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL getCurrentValue(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getMaximumValue(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getMinimumValue(  ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_STANDARD_VCLXACCESSIBLECHECKBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
