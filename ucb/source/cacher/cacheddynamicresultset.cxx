/*************************************************************************
 *
 *  $RCSfile: cacheddynamicresultset.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:52:35 $
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

#include <cacheddynamicresultset.hxx>

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif

#ifndef _CACHED_CONTENT_RESULTSET_HXX
#include <cachedcontentresultset.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace rtl;

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
    DBG_ASSERT( m_xSourceResultOne.is(), "need source resultset" )

    Reference< XResultSet > xCache(
        new CachedContentResultSet( m_xSourceResultOne, m_xContentIdentifierMapping ) );

    vos::OGuard aGuard( m_aMutex );
    m_xMyResultOne = xCache;
}

//virtual
void SAL_CALL CachedDynamicResultSet
    ::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    DynamicResultSetWrapper::impl_InitResultSetTwo( xResultSet );
    DBG_ASSERT( m_xSourceResultTwo.is(), "need source resultset" )

    Reference< XResultSet > xCache(
        new CachedContentResultSet( m_xSourceResultTwo, m_xContentIdentifierMapping ) );

    vos::OGuard aGuard( m_aMutex );
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
                    OUString::createFromAscii( "CachedDynamicResultSet" ),
                    OUString::createFromAscii( CACHED_DRS_SERVICE_NAME ) );

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
                OUString::createFromAscii( "CachedDynamicResultSetFactory" ),
                OUString::createFromAscii( CACHED_DRS_FACTORY_NAME ) );

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


