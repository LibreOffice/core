/*************************************************************************
 *
 *  $RCSfile: proxyaggregation.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-05-19 12:57:46 $
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

#ifndef COMPHELPER_PROXY_AGGREGATION
#include <comphelper/proxyaggregation.hxx>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif

//.............................................................................
namespace comphelper
{
//.............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::reflection;

    //=========================================================================
    //= OProxyAggregation
    //=========================================================================
    //-------------------------------------------------------------------------
    OProxyAggregation::OProxyAggregation( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
    {
    }

    //-------------------------------------------------------------------------
    void OProxyAggregation::aggregateProxyFor( const Reference< XInterface >& _rxComponent, oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator )
    {
        // first a factory for the proxy
        Reference< XProxyFactory > xFactory(
            m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.reflection.ProxyFactory" ) ) ),
            UNO_QUERY
        );
        OSL_ENSURE( xFactory.is(), "OProxyAggregation::aggregateProxyFor: could not create a proxy factory!" );

        // then the proxy itself
        if ( xFactory.is() )
        {
            m_xProxyAggregate = xFactory->createProxy( _rxComponent );

            // aggregate the proxy
            osl_incrementInterlockedCount( &_rRefCount );
            if ( m_xProxyAggregate.is() )
            {
                // At this point in time, the proxy has a ref count of exactly one - in m_xControlContextProxy.
                // Remember to _not_ reset this member unles the delegator of the proxy has been reset, too!
                m_xProxyAggregate->setDelegator( _rDelegator );
            }
            osl_decrementInterlockedCount( &_rRefCount );
        }
    }

    //-------------------------------------------------------------------------
    Any SAL_CALL OProxyAggregation::queryAggregation( const Type& _rType ) throw (RuntimeException)
    {
        return m_xProxyAggregate.is() ? m_xProxyAggregate->queryAggregation( _rType ) : Any();
    }

    //-------------------------------------------------------------------------
    Sequence< Type > SAL_CALL OProxyAggregation::getTypes(  ) throw (RuntimeException)
    {
        Sequence< Type > aTypes;
        if ( m_xProxyAggregate.is() )
        {
            Reference< XTypeProvider > xTypes;
            m_xProxyAggregate->queryAggregation( ::getCppuType( &xTypes ) ) >>= xTypes;
            if ( xTypes.is() )
                aTypes = xTypes->getTypes();
        }
        return aTypes;
    }

    //-------------------------------------------------------------------------
    OProxyAggregation::~OProxyAggregation()
    {
        if ( m_xProxyAggregate.is() )
            m_xProxyAggregate->setDelegator( NULL );
        m_xProxyAggregate.clear();
            // this should remove the _one_and_only_ "real" reference (means not delegated to
            // ourself) to this proxy, and thus delete it
    }

    //=========================================================================
    //= OComponentProxyAggregationHelper
    //=========================================================================
    //-------------------------------------------------------------------------
    OComponentProxyAggregationHelper::OComponentProxyAggregationHelper( const Reference< XMultiServiceFactory >& _rxORB,
        ::cppu::OBroadcastHelper& _rBHelper )
        :OProxyAggregation( _rxORB )
        ,m_rBHelper( _rBHelper )
    {
        OSL_ENSURE( _rxORB.is(), "OComponentProxyAggregationHelper::OComponentProxyAggregationHelper: invalid arguments!" );
    }

    //-------------------------------------------------------------------------
    void OComponentProxyAggregationHelper::aggregateProxyFor(
        const Reference< XComponent >& _rxComponent, oslInterlockedCount& _rRefCount,
        ::cppu::OWeakObject& _rDelegator )
    {
        OSL_ENSURE( _rxComponent.is(), "OComponentProxyAggregationHelper::aggregateProxyFor: invalid inner component!" );
        m_xInner = _rxComponent;

        // aggregate a proxy for the object
        OProxyAggregation::aggregateProxyFor( m_xInner.get(), _rRefCount, _rDelegator );

        // add as event listener to the inner context, because we want to be notified of disposals
        osl_incrementInterlockedCount( &_rRefCount );
        {
            if ( m_xInner.is() )
                m_xInner->addEventListener( this );
        }
        osl_decrementInterlockedCount( &_rRefCount );
    }

    //-------------------------------------------------------------------------
    Any SAL_CALL OComponentProxyAggregationHelper::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn( BASE::queryInterface( _rType ) );
        if ( !aReturn.hasValue() )
            aReturn = OProxyAggregation::queryAggregation( _rType );
        return aReturn;
    }

    //-------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OComponentProxyAggregationHelper, BASE, OProxyAggregation )

    //-------------------------------------------------------------------------
    OComponentProxyAggregationHelper::~OComponentProxyAggregationHelper( )
    {
        OSL_ENSURE( m_rBHelper.bDisposed, "OComponentProxyAggregationHelper::~OComponentProxyAggregationHelper: you should dispose your derived class in the dtor, if necessary!" );
            // if this asserts, add the following to your derived class dtor:
            //
            // if ( !m_rBHelper.bDisposed )
            // {
            //   acquire(); // to prevent duplicate dtor calls
            //   dispose();
            // }

        m_xInner.clear();
    }

    //-------------------------------------------------------------------------
    void SAL_CALL OComponentProxyAggregationHelper::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        if ( _rSource.Source == m_xInner )
        {   // it's our inner context which is dying -> dispose ourself
            if ( !m_rBHelper.bDisposed && !m_rBHelper.bInDispose )
            {   // (if necessary only, of course)
                dispose();
            }
        }
    }

    //-------------------------------------------------------------------------
    void SAL_CALL OComponentProxyAggregationHelper::dispose() throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard( m_rBHelper.rMutex );

        // dispose our inner context
        // before we do this, remove ourself as listener - else in disposing( EventObject ), we
        // would dispose ourself a second time
        Reference< XComponent > xComp( m_xInner, UNO_QUERY );
        if ( xComp.is() )
        {
            xComp->removeEventListener( this );
            xComp->dispose();
            xComp.clear();
        }
    }

    //=========================================================================
    //= OComponentProxyAggregation
    //=========================================================================
    //-------------------------------------------------------------------------
    OComponentProxyAggregation::OComponentProxyAggregation( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XComponent >& _rxComponent )
        :OComponentProxyAggregation_CBase( m_aMutex )
        ,OComponentProxyAggregationHelper( _rxORB, rBHelper )
    {
        OSL_ENSURE( _rxComponent.is(), "OComponentProxyAggregation::OComponentProxyAggregation: accessible is no XComponent!" );
        if ( _rxComponent.is() )
            aggregateProxyFor( _rxComponent, m_refCount, *this );
    }

    //-------------------------------------------------------------------------
    OComponentProxyAggregation::~OComponentProxyAggregation()
    {
        implEnsureDisposeInDtor( );
    }

    //-------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OComponentProxyAggregation, OComponentProxyAggregation_CBase, OComponentProxyAggregationHelper )

    //-------------------------------------------------------------------------
    IMPLEMENT_GET_IMPLEMENTATION_ID( OComponentProxyAggregation )

    //-------------------------------------------------------------------------
    Sequence< Type > SAL_CALL OComponentProxyAggregation::getTypes(  ) throw (RuntimeException)
    {
        Sequence< Type > aTypes( OComponentProxyAggregationHelper::getTypes() );

        // append XComponent, coming from OComponentProxyAggregation_CBase
        sal_Int32 nLen = aTypes.getLength();
        aTypes.realloc( nLen + 1 );
        aTypes[ nLen ] = ::getCppuType( static_cast< Reference< XComponent >* >( NULL ) );

        return aTypes;
    }

    //-------------------------------------------------------------------------
    void OComponentProxyAggregation::implEnsureDisposeInDtor( )
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();  // to prevent duplicate dtor calls
            dispose();
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OComponentProxyAggregation::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        // simly disambiguate - this is necessary for MSVC to distinguish
        // "disposing( EventObject )" from "disposing()"
        OComponentProxyAggregationHelper::disposing( _rSource );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OComponentProxyAggregation::disposing()  throw (RuntimeException)
    {
        // call the dispose-functionality of the base, which will dispose our aggregated component
        OComponentProxyAggregationHelper::dispose();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OComponentProxyAggregation::dispose() throw( RuntimeException )
    {
        // simply disambiguate
        OComponentProxyAggregation_CBase::dispose();
    }


//.............................................................................
}   // namespace comphelper
//.............................................................................

