/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configregistry.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _CONFIGMGR_REGISTRY_CONFIGREGISTRY_HXX_
#define _CONFIGMGR_REGISTRY_CONFIGREGISTRY_HXX_

#include <cppuhelper/implbase2.hxx>
#include "confsvccomponent.hxx"
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

//..........................................................................
namespace configmgr
{
//..........................................................................

//==========================================================================
//= OConfigurationRegistry
//==========================================================================
/** an object implmenting the <service scope="com.sun.star.configuration">ConfigurationRegistry</service>
    service.
*/
class OConfigurationRegistry
        :public ServiceComponentImpl
        ,public cppu::ImplHelper2< com::sun::star::registry::XSimpleRegistry,   com::sun::star::util::XFlushable >
{
public:
    static const ServiceImplementationInfo s_aServiceInfo;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xORB;         /// the service provider used for creating the instance
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        m_xConfigurationProvider;   /// the configuration provider used for creating configuration accesses

    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >
                        m_xRootKey;     /// the root key for the registry-like configuration access
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        m_xSubtreeRoot; /// the root of the sub tree the object wraps
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesBatch >
                        m_xUpdateRoot;  /// the update access to the root of the sub tree, valid if opened for writing
    ::rtl::OUString     m_sLocation;    /// URL of the configuration node we're representing, if any


public:
    OConfigurationRegistry(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rORB)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw() { ServiceComponentImpl::acquire(); }
    virtual void SAL_CALL release(  ) throw() { ServiceComponentImpl::release(); }

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XSimpleRegistry
    virtual ::rtl::OUString SAL_CALL getURL() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL open( const ::rtl::OUString& rURL, sal_Bool bReadOnly, sal_Bool bCreate ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL close(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL destroy(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey > SAL_CALL getRootKey(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mergeKey( const ::rtl::OUString& aKeyName, const ::rtl::OUString& aUrl ) throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::registry::MergeConflictException, ::com::sun::star::uno::RuntimeException);

    // XFlushable
    virtual void SAL_CALL flush(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual void SAL_CALL disposing();
    /// translates the given URL into a nodepath which may be used with the configuration provider
    ::rtl::OUString getNodePathFromURL(const ::rtl::OUString& _rURL);

    void implCheckOpen() throw(::com::sun::star::registry::InvalidRegistryException, ::com::sun::star::uno::RuntimeException);

    sal_Bool implIsOpen()
        throw (::com::sun::star::uno::RuntimeException);
};


//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // _CONFIGMGR_REGISTRY_CONFIGREGISTRY_HXX_


