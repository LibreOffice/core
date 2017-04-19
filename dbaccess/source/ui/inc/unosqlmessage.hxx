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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_UNOSQLMESSAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_UNOSQLMESSAGE_HXX

#include <svtools/genericunodialog.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "core_resource.hxx"

namespace dbaui
{

typedef ::svt::OGenericUnoDialog OSQLMessageDialogBase;
class OSQLMessageDialog
        :public OSQLMessageDialogBase
        ,public ::comphelper::OPropertyArrayUsageHelper< OSQLMessageDialog >
{
    dbaccess::OModuleClient m_aModuleClient;
protected:
    // <properties>
    css::uno::Any        m_aException;
    OUString             m_sHelpURL;
    // </properties>

protected:
    OSQLMessageDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

public:
    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XServiceInfo - static methods
    /// @throws css::uno::RuntimeException
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
    /// @throws css::uno::RuntimeException
    static OUString getImplementationName_Static();
    static css::uno::Reference< css::uno::XInterface >
            SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

protected:
    virtual void SAL_CALL initialize(css::uno::Sequence< css::uno::Any > const & args) override;

// OPropertySetHelper overridables
    // (overwriting these three, because we have some special handling for our property)
    virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue) override;

// OGenericUnoDialog overridables
    virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override;
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_UNOSQLMESSAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
