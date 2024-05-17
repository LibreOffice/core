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

#include "sddlgfact.hxx"
#include <sal/types.h>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

class SdAbstractDialogFactory;

/// anonymous implementation namespace
namespace
{
class CreateDialogFactoryService
    : public ::cppu::WeakImplHelper<css::lang::XServiceInfo, css::lang::XUnoTunnel>
{
public:
    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.presentation.comp.CreateDialogFactoryService"_ustr;
    }
    virtual sal_Bool SAL_CALL supportsService(const OUString& serviceName) override
    {
        return cppu::supportsService(this, serviceName);
    }
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.presentation.CreateDialogFactoryService"_ustr };
    }

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL
    getSomething(const ::css::uno::Sequence<::sal_Int8>& /*aIdentifier*/) override
    {
        static SdAbstractDialogFactory_Impl aFactory;
        return reinterpret_cast<sal_Int64>(static_cast<SdAbstractDialogFactory*>(&aFactory));
    }
};

} // closing anonymous implementation namespace

extern "C" {
SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_presentation_CreateDialogFactoryService_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new CreateDialogFactoryService);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
