/*************************************************************************
 *
 *  $RCSfile: provprox.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:48 $
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

#ifndef _PROVPROX_HXX
#define _PROVPROX_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERFACTORY_HPP_
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPARAMETERIZEDCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERSUPPLIER_HPP_
#include <com/sun/star/ucb/XContentProviderSupplier.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

//=========================================================================

#define PROVIDER_FACTORY_SERVICE_NAME \
                            "com.sun.star.ucb.ContentProviderProxyFactory"
#define PROVIDER_PROXY_SERVICE_NAME \
                            "com.sun.star.ucb.ContentProviderProxy"

//============================================================================
//
// class UcbContentProviderProxyFactory.
//
//============================================================================

class UcbContentProviderProxyFactory :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XContentProviderFactory
{
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;

public:
    UcbContentProviderProxyFactory(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );
    virtual ~UcbContentProviderProxyFactory();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_DECL()

    // XContentProviderFactory
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    createContentProvider( const ::rtl::OUString& Service )
        throw( ::com::sun::star::uno::RuntimeException );
};

//============================================================================
//
// class UcbContentProviderProxy.
//
//============================================================================

class UcbContentProviderProxy :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XContentProviderSupplier,
                public com::sun::star::ucb::XContentProvider,
                public com::sun::star::ucb::XParameterizedContentProvider
{
    ::osl::Mutex    m_aMutex;
    ::rtl::OUString m_aService;
    ::rtl::OUString m_aTemplate;
    ::rtl::OUString m_aArguments;
    sal_Bool        m_bReplace;
    sal_Bool        m_bRegister;

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                                m_xSMgr;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
                                m_xProvider;
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
                                m_xTargetProvider;

public:
    UcbContentProviderProxy(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const ::rtl::OUString& Service );
    virtual ~UcbContentProviderProxy();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XContentProviderSupplier
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    getContentProvider()
        throw( ::com::sun::star::uno::RuntimeException );

    // XContentProvider
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContent > SAL_CALL
    queryContent( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::ucb::XContentIdentifier >& Identifier )
        throw( ::com::sun::star::ucb::IllegalIdentifierException,
               ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL
    compareContentIds( const ::com::sun::star::uno::Reference<
                        ::com::sun::star::ucb::XContentIdentifier >& Id1,
                       const ::com::sun::star::uno::Reference<
                           ::com::sun::star::ucb::XContentIdentifier >& Id2 )
        throw( ::com::sun::star::uno::RuntimeException );

    // XParameterizedContentProvider
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    registerInstance( const ::rtl::OUString& Template,
                      const ::rtl::OUString& Arguments,
                      sal_Bool ReplaceExisting )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::ucb::XContentProvider > SAL_CALL
    deregisterInstance( const ::rtl::OUString& Template,
                        const ::rtl::OUString& Arguments )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException );
};

#endif /* !_PROVPROX_HXX */
