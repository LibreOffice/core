/*************************************************************************
 *
 *  $RCSfile: ProviderCache.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: npower $ $Date: 2003-09-04 07:20:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRAMEWORK_SCRIPT_PROVIDER_PROVIDERCACHE_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_PROVIDERCACHE_HXX_

#include <hash_map>

#include <osl/mutex.hxx>
#include <rtl/ustring>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <com/sun/star/frame/XModel.hpp>
#include <drafts/com/sun/star/script/framework/provider/XScriptProvider.hpp>

#include "ScriptingContext.hxx"

namespace func_provider
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

//Typedefs
//=============================================================================

struct ProviderDetails
{
    //css::uno::Reference< css::lang::XSingleServiceFactory > factory;
    css::uno::Reference< css::lang::XSingleComponentFactory > factory;
    css::uno::Reference< dcsssf::provider::XScriptProvider > provider;
};
typedef ::std::hash_map < ::rtl::OUString, ProviderDetails , ::rtl::OUStringHash,
            ::std::equal_to< ::rtl::OUString > > ProviderDetails_hash;


class ProviderCache
{

public:
     ProviderCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Sequence< css::uno::Any >& scriptContext )
        throw ( css::uno::RuntimeException );
    ~ProviderCache();
     css::uno::Reference< dcsssf::provider::XScriptProvider >
         getProvider( const ::rtl::OUString& providerName );
     css::uno::Sequence < css::uno::Reference< dcsssf::provider::XScriptProvider > >
         getAllProviders() throw ( css::uno::RuntimeException );
private:
    void populateCache()
        throw ( css::uno::RuntimeException );
    //    getFactory( const ::rtl::OUString& serviceName )  throw ( RuntimeException );
    //css::uno::Reference< css::lang::XSingleServiceFactory >
    css::uno::Reference< css::lang::XSingleComponentFactory >
        getFactory( const rtl::OUString& serviceName ) throw ( css::uno::RuntimeException );

   css::uno::Reference< dcsssf::provider::XScriptProvider >
        createProvider( ProviderDetails& details ) throw ( css::uno::RuntimeException );
    ProviderDetails_hash  m_hProviderDetailsCache;
    osl::Mutex m_mutex;
    css::uno::Sequence< css::uno::Any >  m_Sctx;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::lang::XMultiComponentFactory > m_xMgr;


};
} // func_provider
#endif
