/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProviderCache.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:21:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_PROVIDERCACHE_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_PROVIDERCACHE_HXX_

#include <hash_map>
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
typedef ::std::hash_map < ::rtl::OUString, ProviderDetails , ::rtl::OUStringHash,
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
