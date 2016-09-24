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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_UNO_ADMINDLG_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_UNO_ADMINDLG_HXX

#include "unoadmin.hxx"

namespace dbaui
{
// ODataSourcePropertyDialog
class ODataSourcePropertyDialog
        :public ODatabaseAdministrationDialog
        ,public ::comphelper::OPropertyArrayUsageHelper< ODataSourcePropertyDialog >
{

protected:
    explicit ODataSourcePropertyDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

public:
    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo - static methods
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
    static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
    static css::uno::Reference< css::uno::XInterface >
            SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
protected:
// OGenericUnoDialog overridables
    virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override;
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_UNO_ADMINDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
