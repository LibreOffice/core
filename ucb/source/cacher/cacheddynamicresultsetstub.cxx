/*************************************************************************
 *
 *  $RCSfile: cacheddynamicresultsetstub.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-01 08:03:25 $
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

#include <cacheddynamicresultsetstub.hxx>

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#ifndef _CACHED_CONTENT_RESULTSET_STUB_HXX
#include <cachedcontentresultsetstub.hxx>
#endif

#ifndef  _COM_SUN_STAR_UCB_CONTENTRESULTSETCAPABILITY_HPP_
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#endif

#ifndef  _COM_SUN_STAR_UCB_XSORTEDDYNAMICRESULTSETFACTORY_HPP_
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

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
                    OUString::createFromAscii( "CachedDynamicResultSetStub" ),
                    OUString::createFromAscii( CACHED_DRS_STUB_SERVICE_NAME ) );

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
                OUString::createFromAscii( "CachedDynamicResultSetStubFactory" ),
                OUString::createFromAscii( CACHED_DRS_STUB_FACTORY_NAME ) );

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
                m_xSMgr->createInstance( OUString::createFromAscii(
                    "com.sun.star.ucb.SortedDynamicResultSetFactory" ) ),
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

