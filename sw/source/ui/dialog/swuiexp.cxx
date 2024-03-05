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

#include "swdlgfact.hxx"
#include <swuiexp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>

namespace swui
{
SwAbstractDialogFactory& GetFactory()
{
    static SwAbstractDialogFactory_Impl aFactory;
    return aFactory;
}
}

/// anonymous implementation namespace
namespace
{
class DialogFactoryService
    : public ::cppu::WeakImplHelper<css::lang::XServiceInfo, css::lang::XUnoTunnel>
{
public:
    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.text.comp.DialogFactoryService";
    }
    virtual sal_Bool SAL_CALL supportsService(const OUString& serviceName) override
    {
        return cppu::supportsService(this, serviceName);
    }
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { "com.sun.star.text.DialogFactoryService" };
    }

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL
    getSomething(const ::css::uno::Sequence<::sal_Int8>& /*aIdentifier*/) override
    {
        SwAbstractDialogFactory* pFactory = &::swui::GetFactory();
        return reinterpret_cast<sal_Int64>(pFactory);
    }
};

} // closing anonymous implementation namespace

extern "C" {
SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_text_DialogFactoryService_get_implementation(css::uno::XComponentContext*,
                                                          css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new DialogFactoryService);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
