/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include <cassert>

#include <comphelper/proxyaggregation.hxx>
#include <com/sun/star/reflection/ProxyFactory.hpp>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::reflection;

    
    
    
    
    OProxyAggregation::OProxyAggregation( const Reference< XComponentContext >& _rxContext )
        :m_xContext( _rxContext )
    {
    }

    
    void OProxyAggregation::baseAggregateProxyFor( const Reference< XInterface >& _rxComponent, oslInterlockedCount& _rRefCount,
            ::cppu::OWeakObject& _rDelegator )
    {
        
        Reference< XProxyFactory > xFactory = ProxyFactory::create( m_xContext );

        
        { 
            m_xProxyAggregate = xFactory->createProxy( _rxComponent );
        }
        if ( m_xProxyAggregate.is() )
            m_xProxyAggregate->queryAggregation( ::getCppuType( &m_xProxyTypeAccess ) ) >>= m_xProxyTypeAccess;

        
        osl_atomic_increment( &_rRefCount );
        if ( m_xProxyAggregate.is() )
        {
            
            
            
            m_xProxyAggregate->setDelegator( _rDelegator );
        }
        osl_atomic_decrement( &_rRefCount );
    }

    
    Any SAL_CALL OProxyAggregation::queryAggregation( const Type& _rType ) throw (RuntimeException)
    {
        return m_xProxyAggregate.is() ? m_xProxyAggregate->queryAggregation( _rType ) : Any();
    }

    
    Sequence< Type > SAL_CALL OProxyAggregation::getTypes(  ) throw (RuntimeException)
    {
        Sequence< Type > aTypes;
        if ( m_xProxyAggregate.is() )
        {
            if ( m_xProxyTypeAccess.is() )
                aTypes = m_xProxyTypeAccess->getTypes();
        }
        return aTypes;
    }

    
    OProxyAggregation::~OProxyAggregation()
    {
        if ( m_xProxyAggregate.is() )
            m_xProxyAggregate->setDelegator( NULL );
        m_xProxyAggregate.clear();
        m_xProxyTypeAccess.clear();
            
            
    }

    
    
    
    
    OComponentProxyAggregationHelper::OComponentProxyAggregationHelper( const Reference< XComponentContext >& _rxContext,
        ::cppu::OBroadcastHelper& _rBHelper )
        :OProxyAggregation( _rxContext )
        ,m_rBHelper( _rBHelper )
    {
        OSL_ENSURE( _rxContext.is(), "OComponentProxyAggregationHelper::OComponentProxyAggregationHelper: invalid arguments!" );
    }

    
    void OComponentProxyAggregationHelper::componentAggregateProxyFor(
        const Reference< XComponent >& _rxComponent, oslInterlockedCount& _rRefCount,
        ::cppu::OWeakObject& _rDelegator )
    {
        OSL_ENSURE( _rxComponent.is(), "OComponentProxyAggregationHelper::componentAggregateProxyFor: invalid inner component!" );
        m_xInner = _rxComponent;

        
        baseAggregateProxyFor( m_xInner.get(), _rRefCount, _rDelegator );

        
        osl_atomic_increment( &_rRefCount );
        {
            if ( m_xInner.is() )
                m_xInner->addEventListener( this );
        }
        osl_atomic_decrement( &_rRefCount );
    }

    
    Any SAL_CALL OComponentProxyAggregationHelper::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn( BASE::queryInterface( _rType ) );
        if ( !aReturn.hasValue() )
            aReturn = OProxyAggregation::queryAggregation( _rType );
        return aReturn;
    }

    
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OComponentProxyAggregationHelper, BASE, OProxyAggregation )

    
    OComponentProxyAggregationHelper::~OComponentProxyAggregationHelper( )
    {
        OSL_ENSURE( m_rBHelper.bDisposed, "OComponentProxyAggregationHelper::~OComponentProxyAggregationHelper: you should dispose your derived class in the dtor, if necessary!" );
            
            //
            
            
            
            
            

        m_xInner.clear();
    }

    
    void SAL_CALL OComponentProxyAggregationHelper::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        if ( _rSource.Source == m_xInner )
        {   
            if ( !m_rBHelper.bDisposed && !m_rBHelper.bInDispose )
            {   
                dispose();
            }
        }
    }

    
    void SAL_CALL OComponentProxyAggregationHelper::dispose() throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard( m_rBHelper.rMutex );

        
        
        
        Reference< XComponent > xComp( m_xInner, UNO_QUERY );
        if ( xComp.is() )
        {
            xComp->removeEventListener( this );
            xComp->dispose();
            xComp.clear();
        }
    }

    
    
    
    
    OComponentProxyAggregation::OComponentProxyAggregation( const Reference< XComponentContext >& _rxContext,
            const Reference< XComponent >& _rxComponent )
        :WeakComponentImplHelperBase( m_aMutex )
        ,OComponentProxyAggregationHelper( _rxContext, rBHelper )
    {
        OSL_ENSURE( _rxComponent.is(), "OComponentProxyAggregation::OComponentProxyAggregation: accessible is no XComponent!" );
        if ( _rxComponent.is() )
            componentAggregateProxyFor( _rxComponent, m_refCount, *this );
    }

    
    OComponentProxyAggregation::~OComponentProxyAggregation()
    {
        implEnsureDisposeInDtor( );
    }

    
    IMPLEMENT_FORWARD_XINTERFACE2( OComponentProxyAggregation, WeakComponentImplHelperBase, OComponentProxyAggregationHelper )

    
    IMPLEMENT_GET_IMPLEMENTATION_ID( OComponentProxyAggregation )

    
    Sequence< Type > SAL_CALL OComponentProxyAggregation::getTypes(  ) throw (RuntimeException)
    {
        Sequence< Type > aTypes( OComponentProxyAggregationHelper::getTypes() );

        
        sal_Int32 nLen = aTypes.getLength();
        aTypes.realloc( nLen + 1 );
        aTypes[ nLen ] = ::getCppuType( static_cast< Reference< XComponent >* >( NULL ) );

        return aTypes;
    }

    
    void OComponentProxyAggregation::implEnsureDisposeInDtor( )
    {
        if ( !rBHelper.bDisposed )
        {
            acquire();  
            dispose();
        }
    }

    
    void SAL_CALL OComponentProxyAggregation::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        
        
        
        
        assert(_rSource.Source != static_cast< cppu::OWeakObject * >(this));
        OComponentProxyAggregationHelper::disposing( _rSource );
    }

    
    void SAL_CALL OComponentProxyAggregation::disposing()  throw (RuntimeException)
    {
        
        OComponentProxyAggregationHelper::dispose();
    }

    
    void SAL_CALL OComponentProxyAggregation::dispose() throw( RuntimeException )
    {
        
        WeakComponentImplHelperBase::dispose();
    }



}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
