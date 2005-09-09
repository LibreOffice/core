/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheddynamicresultsetstub.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:12:26 $
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

#ifndef _CACHED_DYNAMIC_RESULTSET_STUB_HXX
#define _CACHED_DYNAMIC_RESULTSET_STUB_HXX

#ifndef _DYNAMIC_RESULTSET_WRAPPER_HXX
#include <dynamicresultsetwrapper.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XCACHEDDYNAMICRESULTSETSTUBFACTORY_HPP_
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>
#endif

#define CACHED_DRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSetStub"
#define CACHED_DRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetStubFactory"

//=========================================================================

class CachedDynamicResultSetStub
                : public DynamicResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
{
protected:
    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );

public:
    CachedDynamicResultSetStub( com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory > & xSMgr );

    virtual ~CachedDynamicResultSetStub();


    //-----------------------------------------------------------------
    // XInterface inherited
    //-----------------------------------------------------------------
    XINTERFACE_DECL()
    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()
    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()
};

//=========================================================================

class CachedDynamicResultSetStubFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedDynamicResultSetStubFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;

public:

    CachedDynamicResultSetStubFactory(
        const com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory > & rSMgr);

    virtual ~CachedDynamicResultSetStubFactory();

    //-----------------------------------------------------------------
    // XInterface
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    XSERVICEINFO_DECL()

    //-----------------------------------------------------------------
    // XCachedDynamicResultSetStubFactory

    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSetStub(
                const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSet > & Source )
                throw( com::sun::star::uno::RuntimeException );


    virtual void SAL_CALL connectToCache(
                  const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > & Source
                , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > & TargetCache
                , const com::sun::star::uno::Sequence<
                        com::sun::star::ucb::NumberedSortingInfo > & SortingInfo
                , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XAnyCompareFactory > & CompareFactory
                )
                throw (
                  com::sun::star::ucb::ListenerAlreadySetException
                , com::sun::star::ucb::AlreadyInitializedException
                , com::sun::star::uno::RuntimeException
                 );
};

#endif

