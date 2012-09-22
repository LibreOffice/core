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

#include "comphelper/accessiblewrapper.hxx"
#include <com/sun/star/reflection/XProxyFactory.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <algorithm>

using namespace ::comphelper;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//.............................................................................
namespace comphelper
{
//.............................................................................

    //=========================================================================
    //= OWrappedAccessibleChildrenManager
    //=========================================================================
    //--------------------------------------------------------------------
    struct RemoveEventListener
            : public ::std::unary_function< AccessibleMap::value_type, void >
    {
    private:
        Reference< XEventListener > m_xListener;

    public:
        RemoveEventListener( const Reference< XEventListener >& _rxListener )
            :m_xListener( _rxListener  )
        {
        }

        void operator()( const AccessibleMap::value_type& _rMapEntry ) const
        {
            Reference< XComponent > xComp( _rMapEntry.first, UNO_QUERY );
            if ( xComp.is() )
                xComp->removeEventListener( m_xListener );
        }
    };

    //--------------------------------------------------------------------
    struct DisposeMappedChild
            : public ::std::unary_function< AccessibleMap::value_type, void >
    {
        void operator()( const AccessibleMap::value_type& _rMapEntry ) const
        {
            Reference< XComponent > xContextComponent;
            if ( _rMapEntry.second.is() )
                xContextComponent = xContextComponent.query( _rMapEntry.second->getAccessibleContext() );
            if ( xContextComponent.is() )
                xContextComponent->dispose();
        }
    };

    //-------------------------------------------------------------------------
    OWrappedAccessibleChildrenManager::OWrappedAccessibleChildrenManager( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
        ,m_bTransientChildren( sal_True )
    {
    }

    //-------------------------------------------------------------------------
    OWrappedAccessibleChildrenManager::~OWrappedAccessibleChildrenManager( )
    {
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::setTransientChildren( sal_Bool _bSet )
    {
        m_bTransientChildren = _bSet;
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::setOwningAccessible( const Reference< XAccessible >& _rxAcc )
    {
        OSL_ENSURE( !m_aOwningAccessible.get().is(), "OWrappedAccessibleChildrenManager::setOwningAccessible: to be called only once!" );
        m_aOwningAccessible = WeakReference< XAccessible >( _rxAcc );
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::removeFromCache( const Reference< XAccessible >& _rxKey )
    {
        AccessibleMap::iterator aRemovedPos = m_aChildrenMap.find( _rxKey );
        if ( m_aChildrenMap.end() != aRemovedPos )
        {   // it was cached
            // remove ourself as event listener
            RemoveEventListener aOperator( this );
            aOperator( *aRemovedPos );
            // and remove the entry from the map
            m_aChildrenMap.erase( aRemovedPos );
        }
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::invalidateAll( )
    {
        // remove as event listener from the map elements
        ::std::for_each( m_aChildrenMap.begin(), m_aChildrenMap.end(), RemoveEventListener( this ) );
        // clear the map
        AccessibleMap aMap;
        m_aChildrenMap.swap( aMap );
    }

    //-------------------------------------------------------------------------
    Reference< XAccessible > OWrappedAccessibleChildrenManager::getAccessibleWrapperFor(
        const Reference< XAccessible >& _rxKey, sal_Bool _bCreate )
    {
        Reference< XAccessible > xValue;

        if( !_rxKey.is() )
        {
            // fprintf( stderr, "It was this path that was crashing stuff\n" );
            return xValue;
        }

        // do we have this child in the cahce?
        AccessibleMap::const_iterator aPos = m_aChildrenMap.find( _rxKey );
        if ( m_aChildrenMap.end() != aPos )
        {
            xValue = aPos->second;
        }
        else if ( _bCreate )
        {   // not found in the cache, and allowed to create
            // -> new wrapper
            xValue = new OAccessibleWrapper( m_xORB, _rxKey, (Reference< XAccessible >)m_aOwningAccessible );

            // see if we do cache children
            if ( !m_bTransientChildren )
            {
                if (!m_aChildrenMap.insert(
                        AccessibleMap::value_type( _rxKey, xValue ) ).second)
                {
                    OSL_FAIL(
                        "OWrappedAccessibleChildrenManager::"
                            "getAccessibleWrapperFor: element was already"
                            " inserted!" );
                }

                // listen for disposals of inner children - this may happen when the inner context
                // is the owner for the inner children (it will dispose these children, and of course
                // not our wrapper for these children)
                Reference< XComponent > xComp( _rxKey, UNO_QUERY );
                if ( xComp.is() )
                    xComp->addEventListener( this );
            }
        }

        return xValue;
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::dispose()
    {
        // dispose our children
        ::std::for_each( m_aChildrenMap.begin(), m_aChildrenMap.end(), RemoveEventListener( this ) );
        ::std::for_each( m_aChildrenMap.begin(), m_aChildrenMap.end(), DisposeMappedChild( ) );
        // clear our children
        AccessibleMap aMap;
        m_aChildrenMap.swap( aMap );
    }

    //--------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::implTranslateChildEventValue( const Any& _rInValue, Any& _rOutValue )
    {
        _rOutValue.clear();
        Reference< XAccessible > xChild;
        if ( _rInValue >>= xChild )
            _rOutValue <<= getAccessibleWrapperFor( xChild, sal_True );
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::translateAccessibleEvent( const AccessibleEventObject& _rEvent, AccessibleEventObject& _rTranslatedEvent )
    {
        // just in case we can't translate some of the values:
        _rTranslatedEvent.NewValue = _rEvent.NewValue;
        _rTranslatedEvent.OldValue = _rEvent.OldValue;

        switch ( _rEvent.EventId )
        {
            case AccessibleEventId::CHILD:
            case AccessibleEventId::ACTIVE_DESCENDANT_CHANGED:
            case AccessibleEventId::CONTROLLED_BY_RELATION_CHANGED:
            case AccessibleEventId::CONTROLLER_FOR_RELATION_CHANGED:
            case AccessibleEventId::LABEL_FOR_RELATION_CHANGED:
            case AccessibleEventId::LABELED_BY_RELATION_CHANGED:
            case AccessibleEventId::CONTENT_FLOWS_FROM_RELATION_CHANGED:
            case AccessibleEventId::CONTENT_FLOWS_TO_RELATION_CHANGED:
                // these are events where both the old and the new value contain child references
                implTranslateChildEventValue( _rEvent.OldValue, _rTranslatedEvent.OldValue );
                implTranslateChildEventValue( _rEvent.NewValue, _rTranslatedEvent.NewValue );
                break;

            case AccessibleEventId::NAME_CHANGED:
            case AccessibleEventId::DESCRIPTION_CHANGED:
            case AccessibleEventId::ACTION_CHANGED:
            case AccessibleEventId::STATE_CHANGED:
            case AccessibleEventId::BOUNDRECT_CHANGED:
            case AccessibleEventId::INVALIDATE_ALL_CHILDREN:
            case AccessibleEventId::SELECTION_CHANGED:
            case AccessibleEventId::VISIBLE_DATA_CHANGED:
            case AccessibleEventId::VALUE_CHANGED:
            case AccessibleEventId::MEMBER_OF_RELATION_CHANGED:
            case AccessibleEventId::CARET_CHANGED:
            case AccessibleEventId::TEXT_CHANGED:
            case AccessibleEventId::HYPERTEXT_CHANGED:
            case AccessibleEventId::TABLE_CAPTION_CHANGED:
            case AccessibleEventId::TABLE_COLUMN_DESCRIPTION_CHANGED:
            case AccessibleEventId::TABLE_COLUMN_HEADER_CHANGED:
            case AccessibleEventId::TABLE_MODEL_CHANGED:
            case AccessibleEventId::TABLE_ROW_DESCRIPTION_CHANGED:
            case AccessibleEventId::TABLE_ROW_HEADER_CHANGED:
            case AccessibleEventId::TABLE_SUMMARY_CHANGED:
            // #130798#
            // these Ids are also missed: SUB_WINDOW_OF_RELATION_CHANGED & TEXT_ATTRIBUTE_CHANGED
            case AccessibleEventId::TEXT_SELECTION_CHANGED:
                // nothing to translate
                break;

            default:
                OSL_FAIL( "OWrappedAccessibleChildrenManager::translateAccessibleEvent: unknown (or unexpected) event id!" );
                break;
        }
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::handleChildNotification( const AccessibleEventObject& _rEvent )
    {
        if ( AccessibleEventId::INVALIDATE_ALL_CHILDREN == _rEvent.EventId )
        {   // clear our child map
            invalidateAll( );
        }
        else if ( AccessibleEventId::CHILD == _rEvent.EventId )
        {
            // check if the removed or replaced element is cached
            Reference< XAccessible > xRemoved;
            if ( _rEvent.OldValue >>= xRemoved )
                removeFromCache( xRemoved );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OWrappedAccessibleChildrenManager::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        // this should come from one of the inner XAccessible's of our children
        Reference< XAccessible > xSource( _rSource.Source, UNO_QUERY );
        AccessibleMap::iterator aDisposedPos = m_aChildrenMap.find( xSource );
#if OSL_DEBUG_LEVEL > 0
        if ( m_aChildrenMap.end() == aDisposedPos )
        {
               OSL_FAIL( "OWrappedAccessibleChildrenManager::disposing: where did this come from?" );
            // helper for dignostics
            Reference< XAccessible > xOwningAccessible( m_aOwningAccessible );
            Reference< XAccessibleContext > xContext;
            try
            {
                if ( xOwningAccessible.is() )
                    xContext = xOwningAccessible->getAccessibleContext();
                if ( xContext.is() )
                {
                    ::rtl::OUString sName = xContext->getAccessibleName();
                    ::rtl::OUString sDescription = xContext->getAccessibleDescription();
//                  sal_Int32 nPlaceYourBreakpointHere = 0;
                }
            }
            catch( const Exception& /*e*/ )
            {
                // silent this, it's only diagnostics which failed
            }
        }
#endif
        if ( m_aChildrenMap.end() != aDisposedPos )
        {
            m_aChildrenMap.erase( aDisposedPos );
        }
    }

    //=========================================================================
    //= OAccessibleWrapper (implementation)
    //=========================================================================
    //-------------------------------------------------------------------------
    OAccessibleWrapper::OAccessibleWrapper( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XAccessible >& _rxInnerAccessible, const Reference< XAccessible >& _rxParentAccessible )
        :OAccessibleWrapper_Base( )
        ,OComponentProxyAggregation( _rxORB, Reference< XComponent >( _rxInnerAccessible, UNO_QUERY ) )
        ,m_xParentAccessible( _rxParentAccessible )
        ,m_xInnerAccessible( _rxInnerAccessible )
    {
    }

    //--------------------------------------------------------------------
    OAccessibleWrapper::~OAccessibleWrapper( )
    {
        if ( !m_rBHelper.bDisposed )
        {
            acquire();  // to prevent duplicate dtor calls
            dispose();
        }
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleWrapper, OComponentProxyAggregation, OAccessibleWrapper_Base )
    IMPLEMENT_FORWARD_REFCOUNT( OAccessibleWrapper, OComponentProxyAggregation )

    //--------------------------------------------------------------------
    Any OAccessibleWrapper::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        // #111089# instead of the inner XAccessible the proxy XAccessible must be returned
        Any aReturn = OAccessibleWrapper_Base::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OComponentProxyAggregation::queryInterface( _rType );

        return aReturn;
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > OAccessibleWrapper::getContextNoCreate( ) const
    {
        return (Reference< XAccessibleContext >)m_aContext;
    }

    //--------------------------------------------------------------------
    OAccessibleContextWrapper* OAccessibleWrapper::createAccessibleContext( const Reference< XAccessibleContext >& _rxInnerContext )
    {
        return new OAccessibleContextWrapper( getORB(), _rxInnerContext, this, m_xParentAccessible );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL OAccessibleWrapper::getAccessibleContext(  ) throw (RuntimeException)
    {
        // see if the context is still alive (we cache it)
        Reference< XAccessibleContext > xContext = (Reference< XAccessibleContext >)m_aContext;
        if ( !xContext.is() )
        {
            // create a new context
            Reference< XAccessibleContext > xInnerContext = m_xInnerAccessible->getAccessibleContext( );
            if ( xInnerContext.is() )
            {
                xContext = createAccessibleContext( xInnerContext );
                // cache it
                m_aContext = WeakReference< XAccessibleContext >( xContext );
            }
        }

        return xContext;
    }

    //=========================================================================
    //= OAccessibleWrapper (implementation)
    //=========================================================================
    //-------------------------------------------------------------------------
    OAccessibleContextWrapperHelper::OAccessibleContextWrapperHelper(
                const Reference< XMultiServiceFactory >& _rxORB,
                ::cppu::OBroadcastHelper& _rBHelper,
                const Reference< XAccessibleContext >& _rxInnerAccessibleContext,
                const Reference< XAccessible >& _rxOwningAccessible,
                const Reference< XAccessible >& _rxParentAccessible )
        :OComponentProxyAggregationHelper( _rxORB, _rBHelper )
        ,m_xInnerContext( _rxInnerAccessibleContext )
        ,m_xOwningAccessible( _rxOwningAccessible )
        ,m_xParentAccessible( _rxParentAccessible )
        ,m_pChildMapper( NULL )
    {
        // initialize the mapper for our children
        m_pChildMapper = new OWrappedAccessibleChildrenManager( getORB() );
        m_pChildMapper->acquire();

        // determine if we're allowed to cache children
        Reference< XAccessibleStateSet > xStates( m_xInnerContext->getAccessibleStateSet( ) );
        OSL_ENSURE( xStates.is(), "OAccessibleContextWrapperHelper::OAccessibleContextWrapperHelper: no inner state set!" );
        m_pChildMapper->setTransientChildren( !xStates.is() || xStates->contains( AccessibleStateType::MANAGES_DESCENDANTS) );

        m_pChildMapper->setOwningAccessible( m_xOwningAccessible );
    }

    //--------------------------------------------------------------------
    void OAccessibleContextWrapperHelper::aggregateProxy( oslInterlockedCount& _rRefCount, ::cppu::OWeakObject& _rDelegator )
    {
        Reference< XComponent > xInnerComponent( m_xInnerContext, UNO_QUERY );
        OSL_ENSURE( xInnerComponent.is(), "OComponentProxyAggregation::aggregateProxy: accessible is no XComponent!" );
        if ( xInnerComponent.is() )
            componentAggregateProxyFor( xInnerComponent, _rRefCount, _rDelegator );

        // add as event listener to the inner context, because we want to multiplex the AccessibleEvents
        osl_atomic_increment( &_rRefCount );
        {
            Reference< XAccessibleEventBroadcaster > xBroadcaster( m_xInner, UNO_QUERY );
            if ( xBroadcaster.is() )
                xBroadcaster->addEventListener( this );
        }
        osl_atomic_decrement( &_rRefCount );
    }

    //--------------------------------------------------------------------
    OAccessibleContextWrapperHelper::~OAccessibleContextWrapperHelper( )
    {
        OSL_ENSURE( m_rBHelper.bDisposed, "OAccessibleContextWrapperHelper::~OAccessibleContextWrapperHelper: you should ensure (in your dtor) that the object is disposed!" );

        m_pChildMapper->release();
        m_pChildMapper = NULL;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL OAccessibleContextWrapperHelper::queryInterface( const Type& _rType ) throw (RuntimeException)
    {
        Any aReturn = OComponentProxyAggregationHelper::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OAccessibleContextWrapperHelper_Base::queryInterface( _rType );
        return aReturn;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleContextWrapperHelper, OComponentProxyAggregationHelper, OAccessibleContextWrapperHelper_Base )

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleContextWrapperHelper::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleChildCount();
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleContextWrapperHelper::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        // get the child of the wrapped component
        Reference< XAccessible > xInnerChild = m_xInnerContext->getAccessibleChild( i );
        return m_pChildMapper->getAccessibleWrapperFor( xInnerChild );
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL OAccessibleContextWrapperHelper::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleRelationSet();
            // TODO: if this relation set would contain relations to siblings, we would normally need
            // to wrap them, too ....
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapperHelper::notifyEvent( const AccessibleEventObject& _rEvent ) throw (RuntimeException)
    {
#if OSL_DEBUG_LEVEL > 0
        if ( AccessibleEventId::STATE_CHANGED == _rEvent.EventId )
        {
            sal_Bool bChildTransienceChanged = sal_False;
            sal_Int16 nChangeState = 0;
            if ( _rEvent.OldValue >>= nChangeState )
                bChildTransienceChanged = bChildTransienceChanged || AccessibleStateType::MANAGES_DESCENDANTS == nChangeState;
            if ( _rEvent.NewValue >>= nChangeState )
                bChildTransienceChanged = bChildTransienceChanged || AccessibleStateType::MANAGES_DESCENDANTS == nChangeState;
            OSL_ENSURE( !bChildTransienceChanged, "OAccessibleContextWrapperHelper::notifyEvent: MANAGES_DESCENDANTS is not expected to change during runtime!" );
                // if this asserts, then we would need to update our m_bTransientChildren flag here,
                // as well as (potentially) our child cache
        }
#endif
        AccessibleEventObject aTranslatedEvent( _rEvent );

        {
            ::osl::MutexGuard aGuard( m_rBHelper.rMutex );

            // translate the event
            queryInterface( ::getCppuType( static_cast< Reference< XInterface >* >( NULL ) ) ) >>= aTranslatedEvent.Source;
            m_pChildMapper->translateAccessibleEvent( _rEvent, aTranslatedEvent );

            // see if any of these notifications affect our child manager
            m_pChildMapper->handleChildNotification( _rEvent );

            if ( aTranslatedEvent.NewValue == m_xInner )
                aTranslatedEvent.NewValue = makeAny(aTranslatedEvent.Source);
            if ( aTranslatedEvent.OldValue == m_xInner )
                aTranslatedEvent.OldValue = makeAny(aTranslatedEvent.Source);
        }

        notifyTranslatedEvent( aTranslatedEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapperHelper::dispose() throw( RuntimeException )
    {
        ::osl::MutexGuard aGuard( m_rBHelper.rMutex );

        // stop multiplexing events
        Reference< XAccessibleEventBroadcaster > xBroadcaster( m_xInner, UNO_QUERY );
        OSL_ENSURE( xBroadcaster.is(), "OAccessibleContextWrapperHelper::disposing(): inner context is no broadcaster!" );
        if ( xBroadcaster.is() )
            xBroadcaster->removeEventListener( this );

        // dispose the child cache/map
        m_pChildMapper->dispose();

        // let the base class dispose the inner component
        OComponentProxyAggregationHelper::dispose();
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapperHelper::disposing( const EventObject& _rEvent )  throw (RuntimeException)
    {
        // simply disambiguate this
        OComponentProxyAggregationHelper::disposing( _rEvent );
    }

    //====================================================================
    //= OAccessibleContextWrapper
    //====================================================================
    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleContextWrapper, OAccessibleContextWrapper_CBase, OAccessibleContextWrapperHelper )

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleContextWrapper, OAccessibleContextWrapper_CBase, OAccessibleContextWrapperHelper )

    //--------------------------------------------------------------------
    OAccessibleContextWrapper::OAccessibleContextWrapper( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XAccessibleContext >& _rxInnerAccessibleContext, const Reference< XAccessible >& _rxOwningAccessible,
            const Reference< XAccessible >& _rxParentAccessible )
        :OAccessibleContextWrapper_CBase( m_aMutex )
        ,OAccessibleContextWrapperHelper( _rxORB, rBHelper, _rxInnerAccessibleContext, _rxOwningAccessible, _rxParentAccessible )
        ,m_nNotifierClient( 0 )
    {
        aggregateProxy( m_refCount, *this );
    }

    //--------------------------------------------------------------------
    OAccessibleContextWrapper::~OAccessibleContextWrapper()
    {
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleContextWrapper::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return OAccessibleContextWrapperHelper::getAccessibleChildCount();
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleContextWrapper::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        return OAccessibleContextWrapperHelper::getAccessibleChild( i );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleContextWrapper::getAccessibleParent(  ) throw (RuntimeException)
    {
        return m_xParentAccessible;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleContextWrapper::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleIndexInParent();
    }

    //--------------------------------------------------------------------
    sal_Int16 SAL_CALL OAccessibleContextWrapper::getAccessibleRole(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleRole();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAccessibleContextWrapper::getAccessibleDescription(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleDescription();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAccessibleContextWrapper::getAccessibleName(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleName();
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL OAccessibleContextWrapper::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        return OAccessibleContextWrapperHelper::getAccessibleRelationSet();
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL OAccessibleContextWrapper::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        return m_xInnerContext->getAccessibleStateSet();
    }

    //--------------------------------------------------------------------
    Locale SAL_CALL OAccessibleContextWrapper::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        return m_xInnerContext->getLocale();
    }

    //--------------------------------------------------------------------
    void OAccessibleContextWrapper::notifyTranslatedEvent( const AccessibleEventObject& _rEvent ) throw (RuntimeException)
    {
        if ( m_nNotifierClient )
            AccessibleEventNotifier::addEvent( m_nNotifierClient, _rEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapper::addEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_nNotifierClient )
            m_nNotifierClient = AccessibleEventNotifier::registerClient( );
        AccessibleEventNotifier::addEventListener( m_nNotifierClient, _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapper::removeEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_nNotifierClient )
        {
            if ( 0 == AccessibleEventNotifier::removeEventListener( m_nNotifierClient, _rxListener ) )
            {
                AccessibleEventNotifier::TClientId nId( m_nNotifierClient );
                m_nNotifierClient = 0;
                AccessibleEventNotifier::revokeClient( nId );
            }
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapper::disposing()  throw (RuntimeException)
    {
        AccessibleEventNotifier::TClientId nClientId( 0 );

        // --- <mutex lock> -----------------------------------------
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // prepare notifying our AccessibleListeners
            if ( m_nNotifierClient )
            {
                nClientId = m_nNotifierClient;
                m_nNotifierClient = 0;
            }
        }
        // --- </mutex lock> -----------------------------------------

        // let the base class do
        OAccessibleContextWrapperHelper::dispose();

        // notify the disposal
        if ( nClientId )
            AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, *this );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapper::dispose() throw( RuntimeException )
    {
        // simply disambiguate
        OComponentProxyAggregation_CBase::dispose();
    }

//.............................................................................
}   // namespace accessibility
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
