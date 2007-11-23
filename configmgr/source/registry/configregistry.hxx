/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configregistry.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:30:26 $
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

#ifndef _CONFIGMGR_REGISTRY_CONFIGREGISTRY_HXX_
#define _CONFIGMGR_REGISTRY_CONFIGREGISTRY_HXX_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#include "confsvccomponent.hxx"
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

//..........................................................................
namespace configmgr
{
//..........................................................................

//==========================================================================
//= OConfigurationRegistry
//==========================================================================
typedef ::cppu::ImplHelper2 <   ::com::sun::star::registry::XSimpleRegistry
                            ,   ::com::sun::star::util::XFlushable
                            >   OConfigurationRegistry_Base;

/** an object implmenting the <service scope="com.sun.star.configuration">ConfigurationRegistry</service>
    service.
*/
class OConfigurationRegistry
        :public ServiceComponentImpl
        ,public OConfigurationRegistry_Base
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


