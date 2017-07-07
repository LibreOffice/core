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

#ifndef INCLUDED_SCRIPTING_SOURCE_PROVIDER_PROVIDERCACHE_HXX
#define INCLUDED_SCRIPTING_SOURCE_PROVIDER_PROVIDERCACHE_HXX

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <com/sun/star/script/provider/XScriptProvider.hpp>

#include <unordered_map>

namespace func_provider
{

//Typedefs


struct ProviderDetails
{
    //css::uno::Reference< css::lang::XSingleServiceFactory > factory;
    css::uno::Reference< css::lang::XSingleComponentFactory > factory;
    css::uno::Reference< css::script::provider::XScriptProvider > provider;
};
typedef std::unordered_map < OUString, ProviderDetails , OUStringHash > ProviderDetails_hash;


class ProviderCache
{

public:
     /// @throws css::uno::RuntimeException
     ProviderCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Sequence< css::uno::Any >& scriptContext );
     /// @throws css::uno::RuntimeException
     ProviderCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Sequence< css::uno::Any >& scriptContext,
        const css::uno::Sequence< OUString >& blackList );
    ~ProviderCache();
     css::uno::Reference< css::script::provider::XScriptProvider >
         getProvider( const OUString& providerName );
     /// @throws css::uno::RuntimeException
     css::uno::Sequence < css::uno::Reference< css::script::provider::XScriptProvider > >
         getAllProviders();
private:
    /// @throws css::uno::RuntimeException
    void populateCache();

    /// @throws css::uno::RuntimeException
   css::uno::Reference< css::script::provider::XScriptProvider >
        createProvider( ProviderDetails& details );
    bool isInBlackList( const OUString& serviceName )
    {
        for ( sal_Int32 index = 0; index < m_sBlackList.getLength(); index++ )
        {
            if ( m_sBlackList[ index ] == serviceName )
            {
                return true;
            }
        }
        return false;
    }
    css::uno::Sequence< OUString >  m_sBlackList;
    ProviderDetails_hash  m_hProviderDetailsCache;
    osl::Mutex m_mutex;
    css::uno::Sequence< css::uno::Any >  m_Sctx;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;


};
} // func_provider
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
