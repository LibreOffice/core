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

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>

#include "ActiveMSPList.hxx"

namespace func_provider
{

class MasterScriptProviderFactory :
    public ::cppu::WeakImplHelper <
        css::script::provider::XScriptProviderFactory,
        css::lang::XServiceInfo >
{
private:

    mutable rtl::Reference< ActiveMSPList > m_MSPList;

    const css::uno::Reference< css::uno::XComponentContext > m_xComponentContext;

    const rtl::Reference< ActiveMSPList > & getActiveMSPList() const;

protected:
    virtual ~MasterScriptProviderFactory() override;

public:
    explicit MasterScriptProviderFactory(
        css::uno::Reference< css::uno::XComponentContext > const & xComponentContext );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL
        supportsService( OUString const & serviceName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // XScriptProviderFactory
    virtual css::uno::Reference< css::script::provider::XScriptProvider >
        SAL_CALL createScriptProvider( const css::uno::Any& context ) override;
};


} // namespace func_provider

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
