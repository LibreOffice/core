/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_PROVIDERCACHE_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_PROVIDERCACHE_HXX_

#include <boost/unordered_map.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/script/provider/XScriptProvider.hpp>

#include "ScriptingContext.hxx"

namespace func_provider
{
// for simplification
#define css ::com::sun::star

//Typedefs
//=============================================================================

struct ProviderDetails
{
    //css::uno::Reference< css::lang::XSingleServiceFactory > factory;
    css::uno::Reference< css::lang::XSingleComponentFactory > factory;
    css::uno::Reference< css::script::provider::XScriptProvider > provider;
};
typedef ::boost::unordered_map < ::rtl::OUString, ProviderDetails , ::rtl::OUStringHash,
            ::std::equal_to< ::rtl::OUString > > ProviderDetails_hash;


class ProviderCache
{

public:
     ProviderCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Sequence< css::uno::Any >& scriptContext )
        throw ( css::uno::RuntimeException );
     ProviderCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Sequence< css::uno::Any >& scriptContext,
        const css::uno::Sequence< ::rtl::OUString >& blackList )
        throw ( css::uno::RuntimeException );
    ~ProviderCache();
     css::uno::Reference< css::script::provider::XScriptProvider >
         getProvider( const ::rtl::OUString& providerName );
     css::uno::Sequence < css::uno::Reference< css::script::provider::XScriptProvider > >
         getAllProviders() throw ( css::uno::RuntimeException );
private:
    void populateCache()
        throw ( css::uno::RuntimeException );

   css::uno::Reference< css::script::provider::XScriptProvider >
        createProvider( ProviderDetails& details ) throw ( css::uno::RuntimeException );
    bool isInBlackList( const ::rtl::OUString& serviceName )
    {
        if ( m_sBlackList.getLength() > 0 )
        {
            for ( sal_Int32 index = 0; index < m_sBlackList.getLength(); index++ )
            {
                if ( m_sBlackList[ index ].equals( serviceName ) )
                {
                    return true;
                }
            }
        }
        return false;
    }
    css::uno::Sequence< ::rtl::OUString >  m_sBlackList;
    ProviderDetails_hash  m_hProviderDetailsCache;
    osl::Mutex m_mutex;
    css::uno::Sequence< css::uno::Any >  m_Sctx;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;


};
} // func_provider
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
