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

#include <cppuhelper/supportsservice.hxx>

#include "MasterScriptProviderFactory.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;

namespace func_provider
{

MasterScriptProviderFactory::MasterScriptProviderFactory(
    Reference< XComponentContext > const & xComponentContext )
    : m_xComponentContext( xComponentContext )
{
}

MasterScriptProviderFactory::~MasterScriptProviderFactory()
{
}

Reference< provider::XScriptProvider > SAL_CALL
MasterScriptProviderFactory::createScriptProvider( const Any& context )
{
    Reference< provider::XScriptProvider > xMsp( getActiveMSPList() ->getMSPFromAnyContext( context ), UNO_SET_THROW );
    return xMsp;
}

const rtl::Reference< ActiveMSPList > &
MasterScriptProviderFactory::getActiveMSPList() const
{
    if ( !m_MSPList.is() )
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if ( !m_MSPList.is() )
           m_MSPList = new ActiveMSPList( m_xComponentContext );
    }
    return m_MSPList;
}

OUString SAL_CALL MasterScriptProviderFactory::getImplementationName()
{
    return u"com.sun.star.script.provider.MasterScriptProviderFactory"_ustr;
}

Sequence< OUString > SAL_CALL MasterScriptProviderFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.script.provider.MasterScriptProviderFactory"_ustr };
}

sal_Bool MasterScriptProviderFactory::supportsService(
    OUString const & serviceName )
{
    return cppu::supportsService(this, serviceName);
}

} // namespace func_provider

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
scripting_MasterScriptProviderFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new func_provider::MasterScriptProviderFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
