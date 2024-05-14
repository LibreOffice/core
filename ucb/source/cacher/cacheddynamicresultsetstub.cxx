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


#include "cacheddynamicresultsetstub.hxx"
#include "cachedcontentresultsetstub.hxx"
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#include <com/sun/star/ucb/SortedDynamicResultSetFactory.hpp>
#include <osl/diagnose.h>
#include <ucbhelper/macros.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;


CachedDynamicResultSetStub::CachedDynamicResultSetStub(
        Reference< XDynamicResultSet > const & xOrigin
        , const Reference< XComponentContext > & rxContext )
        : DynamicResultSetWrapper( xOrigin, rxContext )
{
    OSL_ENSURE( m_xContext.is(), "need Multiservicefactory to create stub" );
    impl_init();
}

CachedDynamicResultSetStub::~CachedDynamicResultSetStub()
{
    impl_deinit();
}

//virtual
void CachedDynamicResultSetStub
    ::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetOne( xResultSet );
    OSL_ENSURE( m_xSourceResultOne.is(), "need source resultset" );

    Reference< XResultSet > xStub(
        new CachedContentResultSetStub( m_xSourceResultOne ) );

    std::unique_lock aGuard( m_aMutex );
    m_xMyResultOne = std::move(xStub);
}

//virtual
void CachedDynamicResultSetStub
    ::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetTwo( xResultSet );
    OSL_ENSURE( m_xSourceResultTwo.is(), "need source resultset" );

    Reference< XResultSet > xStub(
        new CachedContentResultSetStub( m_xSourceResultTwo ) );

    std::unique_lock aGuard( m_aMutex );
    m_xMyResultTwo = std::move(xStub);
}

// XInterface methods.
void SAL_CALL CachedDynamicResultSetStub::acquire()
    noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL CachedDynamicResultSetStub::release()
    noexcept
{
    OWeakObject::release();
}

Any SAL_CALL CachedDynamicResultSetStub
    ::queryInterface( const Type&  rType )
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


// XTypeProvider methods.

//list all interfaces exclusive baseclasses
XTYPEPROVIDER_IMPL_5( CachedDynamicResultSetStub
                    , XTypeProvider
                    , XServiceInfo
                    , XDynamicResultSet
                    , XDynamicResultSetListener
                    , XSourceInitialization
                    );


// XServiceInfo methods.

OUString SAL_CALL CachedDynamicResultSetStub::getImplementationName()
{
    return u"com.sun.star.comp.ucb.CachedDynamicResultSetStub"_ustr;
}

sal_Bool SAL_CALL CachedDynamicResultSetStub::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL CachedDynamicResultSetStub::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.CachedDynamicResultSetStub"_ustr };
}





CachedDynamicResultSetStubFactory::CachedDynamicResultSetStubFactory(
        const Reference< XComponentContext > & rxContext )
{
    m_xContext = rxContext;
}

CachedDynamicResultSetStubFactory::~CachedDynamicResultSetStubFactory()
{
}

// CachedDynamicResultSetStubFactory XServiceInfo methods.

OUString SAL_CALL CachedDynamicResultSetStubFactory::getImplementationName()
{
    return u"com.sun.star.comp.ucb.CachedDynamicResultSetStubFactory"_ustr;
}
sal_Bool SAL_CALL CachedDynamicResultSetStubFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}
css::uno::Sequence< OUString > SAL_CALL CachedDynamicResultSetStubFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.ucb.CachedDynamicResultSetStubFactory"_ustr };
}

// Service factory implementation.



extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_CachedDynamicResultSetStubFactory_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new CachedDynamicResultSetStubFactory(context));
}


// CachedDynamicResultSetStubFactory XCachedDynamicResultSetStubFactory methods.


//virtual
Reference< XDynamicResultSet > SAL_CALL CachedDynamicResultSetStubFactory
    ::createCachedDynamicResultSetStub(
            const Reference< XDynamicResultSet > & Source )
{
    Reference< XDynamicResultSet > xRet = new CachedDynamicResultSetStub( Source, m_xContext );
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
{
    OSL_ENSURE( Source.is(), "a Source is needed" );
    OSL_ENSURE( TargetCache.is(), "a TargetCache is needed" );

    Reference< XDynamicResultSet > xSource( Source );
    if( SortingInfo.hasElements() &&
        !( xSource->getCapabilities() & ContentResultSetCapability::SORTED )
        )
    {
        Reference< XSortedDynamicResultSetFactory > xSortFactory;
        try
        {
            xSortFactory = SortedDynamicResultSetFactory::create( m_xContext );
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
        new CachedDynamicResultSetStub( xSource, m_xContext ) );

    Reference< XSourceInitialization > xTarget( TargetCache, UNO_QUERY );
    OSL_ENSURE( xTarget.is(), "Target must have interface XSourceInitialization" );

    xTarget->setSource( xStub );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
