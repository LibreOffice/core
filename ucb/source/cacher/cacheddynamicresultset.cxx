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

#include <cacheddynamicresultset.hxx>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <cachedcontentresultset.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;

using ::rtl::OUString;

CachedDynamicResultSet::CachedDynamicResultSet(
        Reference< XDynamicResultSet > xOrigin
        , const Reference< XContentIdentifierMapping > & xContentMapping
        , const Reference< XMultiServiceFactory > & xSMgr )
        : DynamicResultSetWrapper( xOrigin, xSMgr )
        , m_xContentIdentifierMapping( xContentMapping )
{
    impl_init();
}

CachedDynamicResultSet::~CachedDynamicResultSet()
{
    impl_deinit();
}

//virtual
void SAL_CALL CachedDynamicResultSet
    ::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetOne( xResultSet );
    OSL_ENSURE( m_xSourceResultOne.is(), "need source resultset" );

    Reference< XResultSet > xCache(
        new CachedContentResultSet( m_xSMgr, m_xSourceResultOne, m_xContentIdentifierMapping ) );

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_xMyResultOne = xCache;
}

//virtual
void SAL_CALL CachedDynamicResultSet
    ::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetTwo( xResultSet );
    OSL_ENSURE( m_xSourceResultTwo.is(), "need source resultset" );

    Reference< XResultSet > xCache(
        new CachedContentResultSet( m_xSMgr, m_xSourceResultTwo, m_xContentIdentifierMapping ) );

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_xMyResultTwo = xCache;
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
XINTERFACE_COMMON_IMPL( CachedDynamicResultSet )

Any SAL_CALL CachedDynamicResultSet
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
XTYPEPROVIDER_IMPL_4( CachedDynamicResultSet
                    , XTypeProvider
                    , XServiceInfo
                    , XDynamicResultSet
                    , XSourceInitialization
                    );

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( CachedDynamicResultSet,
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "com.sun.star.comp.ucb.CachedDynamicResultSet" )),
                            OUString(RTL_CONSTASCII_USTRINGPARAM(
                            CACHED_DRS_SERVICE_NAME )) );

//--------------------------------------------------------------------------
// own methds. ( inherited )
//--------------------------------------------------------------------------
//virtual
void SAL_CALL CachedDynamicResultSet
    ::impl_disposing( const EventObject& Source )
    throw( RuntimeException )
{
    DynamicResultSetWrapper::impl_disposing( Source );
    m_xContentIdentifierMapping.clear();
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class CachedDynamicResultSetFactory
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CachedDynamicResultSetFactory::CachedDynamicResultSetFactory(
        const Reference< XMultiServiceFactory > & rSMgr )
{
    m_xSMgr = rSMgr;
}

CachedDynamicResultSetFactory::~CachedDynamicResultSetFactory()
{
}

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_3( CachedDynamicResultSetFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XCachedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( CachedDynamicResultSetFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XCachedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_IMPL_1( CachedDynamicResultSetFactory,
                         OUString(RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.comp.ucb.CachedDynamicResultSetFactory" )),
                         OUString(RTL_CONSTASCII_USTRINGPARAM(
                         CACHED_DRS_FACTORY_NAME )) );

//--------------------------------------------------------------------------
// Service factory implementation.
//--------------------------------------------------------------------------

ONE_INSTANCE_SERVICE_FACTORY_IMPL( CachedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// CachedDynamicResultSetFactory XCachedDynamicResultSetFactory methods.
//--------------------------------------------------------------------------

//virtual
Reference< XDynamicResultSet > SAL_CALL CachedDynamicResultSetFactory
    ::createCachedDynamicResultSet(
          const Reference< XDynamicResultSet > & SourceStub
        , const Reference< XContentIdentifierMapping > & ContentIdentifierMapping )
        throw( RuntimeException )
{
    Reference< XDynamicResultSet > xRet;
    xRet = new CachedDynamicResultSet( SourceStub, ContentIdentifierMapping, m_xSMgr );
    return xRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
