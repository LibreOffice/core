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

#include <svtools/genericunodialog.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/proparrhlp.hxx>

namespace dbaui
{

typedef ::svt::OGenericUnoDialog OSQLMessageDialogBase;
class OSQLMessageDialog final
        :public OSQLMessageDialogBase
        ,public ::comphelper::OPropertyArrayUsageHelper< OSQLMessageDialog >
{
    // <properties>
    css::uno::Any        m_aException;
    OUString             m_sHelpURL;
    // </properties>

public:
    OSQLMessageDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

private:
    virtual void SAL_CALL initialize(css::uno::Sequence< css::uno::Any > const & args) override;

// OPropertySetHelper overridables
    // (overwriting these three, because we have some special handling for our property)
    virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue) override;

    // OGenericUnoDialog overridables
    virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
};

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
