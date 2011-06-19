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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <cacheddynamicresultsetstub.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cachedcontentresultsetstub.hxx>
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <osl/diagnose.h>

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

using ::rtl::OUString;

CachedDynamicResultSetStub::CachedDynamicResultSetStub(
        Reference< XDynamicResultSet > xOrigin
        , const Reference< XMultiServiceFactory > & xSMgr )
        : DynamicResultSetWrapper( xOrigin, xSMgr )
{
    OSL_ENSURE( m_xSMgr.is(), "need Multiservicefactory to create stub" );
    impl_init();
}

CachedDynamicResultSetStub::~CachedDynamicResultSetStub()
{
    impl_deinit();
}

//virtual
void SAL_CALL CachedDynamicResultSetStub
    ::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetOne( xResultSet );
    OSL_ENSURE( m_xSourceResultOne.is(), "need source resultset" );

    Reference< XResultSet > xStub(
        new CachedContentResultSetStub( m_xSourceResultOne ) );

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_xMyResultOne = xStub;
}

//virtual
void SAL_CALL CachedDynamicResultSetStub
    ::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetTwo( xResultSet );
    OSL_ENSURE( m_xSourceResultTwo.is(), "need source resultset" );

    Reference< XResultSet > xStub(
        new CachedContentResultSetStub( m_xSourceResultTwo ) );

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_xMyResultTwo = xStub;
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
XINTERFACE_COMMON_IMPL( CachedDynamicResultSetStub )

Any SAL_CALL CachedDynamicResultSetStub
    ::queryInterface( const Type&  rType )
    throw ( RuntimeException )
{
    //list all interfaces inclusive baseclasses of interfaces

    Any aRet = DynamicResultSetWrapper::queryInterface( rType );
    if( aRet.hasValue() )
        return aRet;

    aRet = cppu::queryInterface( rType,
                static_cast< XTypeProvider* >( this )
                , static_cast< XServiceInfo* >( this )
                );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------
//list all interfaces exclusive baseclasses
XTYPEPROVIDER_IMPL_5( CachedDynamicResultSetStub
                    , XTypeProvider
                    , XServiceInfo
                    , XDynamicResultSet
                    , XDynamicResultSetListener
                    , XSourceInitialization
                    );

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( CachedDynamicResultSetStub,
                        OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.comp.ucb.CachedDynamicResultSetStub" )),
                        OUString(RTL_CONSTASCII_USTRINGPARAM(
                        CACHED_DRS_STUB_SERVICE_NAME )) );

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CachedDynamicResultSetStubFactory
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CachedDynamicResultSetStubFactory::CachedDynamicResultSetStubFactory(
        const Reference< XMultiServiceFactory > & rSMgr )
{
    m_xSMgr = rSMgr;
}

CachedDynamicResultSetStubFactory::~CachedDynamicResultSetStubFactory()
{
}

//--------------------------------------------------------------------------
// CachedDynamicResultSetStubFactory XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_3( CachedDynamicResultSetStubFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XCachedDynamicResultSetStubFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetStubFactory XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( CachedDynamicResultSetStubFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedDynamicResultSetStubFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetStubFactory XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_IMPL_1( CachedDynamicResultSetStubFactory,
                     OUString(RTL_CONSTASCII_USTRINGPARAM(
                     "com.sun.star.comp.ucb.CachedDynamicResultSetStubFactory" )),
                     OUString(RTL_CONSTASCII_USTRINGPARAM(
                     CACHED_DRS_STUB_FACTORY_NAME )) );

//--------------------------------------------------------------------------
// Service factory implementation.
//--------------------------------------------------------------------------

ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedDynamicResultSetStubFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetStubFactory XCachedDynamicResultSetStubFactory methods.
//--------------------------------------------------------------------------

//virtual
Reference< XDynamicResultSet > SAL_CALL CachedDynamicResultSetStubFactory
    ::createCachedDynamicResultSetStub(
            const Reference< XDynamicResultSet > & Source )
            throw( RuntimeException )
{
    Reference< XDynamicResultSet > xRet;
    xRet = new CachedDynamicResultSetStub( Source, m_xSMgr );
    return xRet;
}

//virtual
void SAL_CALL CachedDynamicResultSetStubFactory
    ::connectToCache(
              const Reference< XDynamicResultSet > & Source
            , const Reference< XDynamicResultSet > & TargetCache
            , const Sequence< NumberedSortingInfo > & SortingInfo
            , const Reference< XAnyCompareFactory > & CompareFactory
            )
            throw (  ListenerAlreadySetException
            , AlreadyInitializedException
            , RuntimeException )
{
    OSL_ENSURE( Source.is(), "a Source is needed" );
    OSL_ENSURE( TargetCache.is(), "a TargetCache is needed" );

    Reference< XDynamicResultSet > xSource( Source );
    if( SortingInfo.getLength() &&
        !( xSource->getCapabilities() & ContentResultSetCapability::SORTED )
        )
    {
        Reference< XSortedDynamicResultSetFactory > xSortFactory;
        try
        {
            xSortFactory = Reference< XSortedDynamicResultSetFactory >(
                m_xSMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.ucb.SortedDynamicResultSetFactory" )) ),
                UNO_QUERY );
        }
        catch ( Exception const & )
        {
        }

        if( xSortFactory.is() )
        {
            Reference< XDynamicResultSet > xSorted(
                xSortFactory->createSortedDynamicResultSet(
                    Source, SortingInfo, CompareFactory ) );
            if( xSorted.is() )
                xSource = xSorted;
        }
    }

    Reference< XDynamicResultSet > xStub(
        new CachedDynamicResultSetStub( xSource, m_xSMgr ) );

    Reference< XSourceInitialization > xTarget( TargetCache, UNO_QUERY );
    OSL_ENSURE( xTarget.is(), "Target must have interface XSourceInitialization" );

    xTarget->setSource( xStub );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
