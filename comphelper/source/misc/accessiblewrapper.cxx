/*************************************************************************
 *
 *  $RCSfile: accessiblewrapper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-23 17:24:56 $
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

#ifndef COMPHELPER_ACCESSIBLE_WRAPPER_HXX
#include "comphelper/accessiblewrapper.hxx"
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#include <algorithm>

using namespace ::comphelper;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::reflection;

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
            Reference< XComponent > xComp( _rMapEntry.second, UNO_QUERY );
            if ( xComp.is() )
                xComp->dispose();
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
                m_aChildrenMap.insert( AccessibleMap::value_type( _rxKey, xValue ) );

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
        ::std::for_each( m_aChildrenMap.begin(), m_aChildrenMap.end(), DisposeMappedChild() );
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
            case AccessibleEventId::ACCESSIBLE_CHILD_EVENT:
            case AccessibleEventId::ACCESSIBLE_ACTIVE_DESCENDANT_EVENT:
            case AccessibleEventId::CONTROLLED_BY_EVENT:
            case AccessibleEventId::CONTROLLER_FOR_EVENT:
            case AccessibleEventId::LABEL_FOR_EVENT:
            case AccessibleEventId::LABELED_BY_EVENT:
            case AccessibleEventId::CONTENT_FLOWS_FROM_EVENT:
            case AccessibleEventId::CONTENT_FLOWS_TO_EVENT:
                // these are events where both the old and the new value contain child references
                implTranslateChildEventValue( _rEvent.OldValue, _rTranslatedEvent.OldValue );
                implTranslateChildEventValue( _rEvent.NewValue, _rTranslatedEvent.NewValue );
                break;

            case AccessibleEventId::ACCESSIBLE_ACTION_EVENT:
            case AccessibleEventId::ACCESSIBLE_CARET_EVENT:
            case AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT:
            case AccessibleEventId::ACCESSIBLE_HYPERTEXT_EVENT:
            case AccessibleEventId::ACCESSIBLE_NAME_EVENT:
            case AccessibleEventId::ACCESSIBLE_SELECTION_EVENT:
            case AccessibleEventId::ACCESSIBLE_STATE_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_CAPTION_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_COLUMN_DESCRIPTION_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_COLUMN_HEADER_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_MODEL_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_ROW_DESCRIPTION_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_ROW_HEADER_EVENT:
            case AccessibleEventId::ACCESSIBLE_TABLE_SUMMARY_EVENT:
            case AccessibleEventId::ACCESSIBLE_TEXT_EVENT:
            case AccessibleEventId::ACCESSIBLE_VALUE_EVENT:
            case AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT:
            case AccessibleEventId::MEMBER_OF_EVENT:
            case AccessibleEventId::ACCESSIBLE_ALL_CHILDREN_CHANGED_EVENT:
            case AccessibleEventId::ACCESSIBLE_BOUNDRECT_EVENT:
                // nothing to translate
                break;

            default:
                OSL_ENSURE( sal_False, "OWrappedAccessibleChildrenManager::translateAccessibleEvent: unknown (or unexpected) event id!" );
                break;
        }
    }

    //-------------------------------------------------------------------------
    void OWrappedAccessibleChildrenManager::handleChildNotification( const AccessibleEventObject& _rEvent )
    {
        if ( AccessibleEventId::ACCESSIBLE_ALL_CHILDREN_CHANGED_EVENT == _rEvent.EventId )
        {   // clear our child map
            invalidateAll( );
        }
        else if ( AccessibleEventId::ACCESSIBLE_CHILD_EVENT == _rEvent.EventId )
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
        OSL_ENSURE( m_aChildrenMap.end() != aDisposedPos,
            "OWrappedAccessibleChildrenManager::disposing: where did this come from?" );
        if ( m_aChildrenMap.end() != aDisposedPos )
        {
            m_aChildrenMap.erase( aDisposedPos );
        }
    }

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
    //= OAccessibleWrapper (implementation)
    //=========================================================================
    //-------------------------------------------------------------------------
    OAccessibleWrapper::OAccessibleWrapper( const Reference< XMultiServiceFactory >& _rxORB,
            const Reference< XAccessible >& _rxInnerAccessible, const Reference< XAccessible >& _rxParentAccessible )
        :OAccessibleWrapper_Base( _rxORB, _rxInnerAccessible )
        ,m_xParentAccessible( _rxParentAccessible )
    {
    }

    //--------------------------------------------------------------------
    OAccessibleWrapper::~OAccessibleWrapper( )
    {
        implEnsureDisposeInDtor();
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
            Reference< XAccessibleContext > xInnerContext = m_xInner->getAccessibleContext( );
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
    OAccessibleContextWrapper::OAccessibleContextWrapper(
                const Reference< XMultiServiceFactory >& _rxORB,
                const Reference< XAccessibleContext >& _rxInnerAccessibleContext,
                const Reference< XAccessible >& _rxOwningAccessible,
                const Reference< XAccessible >& _rxParentAccessible )
        :OAccessibleContextWrapper_Base( _rxORB, _rxInnerAccessibleContext )
        ,m_xOwningAccessible( _rxOwningAccessible )
        ,m_xParentAccessible( _rxParentAccessible )
        ,m_nNotifierClient( 0 )
        ,m_pChildMapper( NULL )
    {
        // add as event listener add the inner context, because we want to multiplex the AccessibleEvents
        osl_incrementInterlockedCount( &m_refCount );
        {
            Reference< XAccessibleEventBroadcaster > xBroadcaster( m_xInner, UNO_QUERY );
            if ( xBroadcaster.is() )
                xBroadcaster->addEventListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );

        m_pChildMapper = new OWrappedAccessibleChildrenManager( getORB() );
        m_pChildMapper->acquire();

        // determine if we're allowed to cache children
        Reference< XAccessibleStateSet > xStates( m_xInner->getAccessibleStateSet( ) );
        OSL_ENSURE( xStates.is(), "OAccessibleContextWrapper::OAccessibleContextWrapper: no inner state set!" );
        m_pChildMapper->setTransientChildren( !xStates.is() || xStates->contains( AccessibleStateType::MANAGES_DESCENDANT ) );

        m_pChildMapper->setOwningAccessible( m_xOwningAccessible );
    }

    //--------------------------------------------------------------------
    OAccessibleContextWrapper::~OAccessibleContextWrapper( )
    {
        m_pChildMapper->release();
        m_pChildMapper = NULL;

        implEnsureDisposeInDtor();
    }

    //--------------------------------------------------------------------
    //  IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleContextWrapper, OAccessibleContextWrapper_Base, OAccessibleContextWrapper_MBase )
    IMPLEMENT_FORWARD_REFCOUNT( OAccessibleContextWrapper, OAccessibleContextWrapper_Base )
    ::com::sun::star::uno::Any SAL_CALL OAccessibleContextWrapper::queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Any aReturn = OAccessibleContextWrapper_MBase::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OAccessibleContextWrapper_Base::queryInterface( _rType );
        return aReturn;
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleContextWrapper, OAccessibleContextWrapper_Base, OAccessibleContextWrapper_MBase )

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleContextWrapper::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleChildCount();
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleContextWrapper::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        // get the child of the wrapped component
        Reference< XAccessible > xInnerChild = m_xInner->getAccessibleChild( i );
        return m_pChildMapper->getAccessibleWrapperFor( xInnerChild );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL OAccessibleContextWrapper::getAccessibleParent(  ) throw (RuntimeException)
    {
        return m_xParentAccessible;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL OAccessibleContextWrapper::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleIndexInParent();
    }

    //--------------------------------------------------------------------
    sal_Int16 SAL_CALL OAccessibleContextWrapper::getAccessibleRole(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleRole();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAccessibleContextWrapper::getAccessibleDescription(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleDescription();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAccessibleContextWrapper::getAccessibleName(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleName();
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL OAccessibleContextWrapper::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleRelationSet();
            // TODO: if this relation set would contain relations to siblings, we would normally need
            // to wrap them, too ....
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL OAccessibleContextWrapper::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        return m_xInner->getAccessibleStateSet();
    }

    //--------------------------------------------------------------------
    Locale SAL_CALL OAccessibleContextWrapper::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        return m_xInner->getLocale();
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
    void SAL_CALL OAccessibleContextWrapper::notifyEvent( const AccessibleEventObject& _rEvent ) throw (RuntimeException)
    {
#if OSL_DEBUG_LEVEL > 0
        if ( AccessibleEventId::ACCESSIBLE_STATE_EVENT == _rEvent.EventId )
        {
            sal_Bool bChildTransienceChanged = sal_False;
            sal_Int16 nChangeState;
            if ( _rEvent.OldValue >>= nChangeState )
                bChildTransienceChanged = bChildTransienceChanged || AccessibleStateType::MANAGES_DESCENDANT == nChangeState;
            if ( _rEvent.NewValue >>= nChangeState )
                bChildTransienceChanged = bChildTransienceChanged || AccessibleStateType::MANAGES_DESCENDANT == nChangeState;
            OSL_ENSURE( !bChildTransienceChanged, "OAccessibleContextWrapper::notifyEvent: MANAGES_DESCENDANT is not expected to change during runtime!" );
                // if this asserts, then we would need to update our m_bTransientChildren flag here,
                // as well as (potentially) our child cache
        }
#endif
        AccessibleEventNotifier::TClientId  nClientId( 0 );
        AccessibleEventObject               aTranslatedEvent( _rEvent );

        {
            ::osl::MutexGuard aGuard( m_aMutex );

            // see if any of these notifications affect our child manager
            m_pChildMapper->handleChildNotification( _rEvent );

            // see if we need to multiplex this event
            if ( !m_nNotifierClient )
                // no client id => no listeners => no need to multiplex
                return;

            aTranslatedEvent.Source = *this;
            m_pChildMapper->translateAccessibleEvent( _rEvent, aTranslatedEvent );
            if ( aTranslatedEvent.NewValue == m_xInner )
                aTranslatedEvent.NewValue = makeAny(aTranslatedEvent.Source);
            if ( aTranslatedEvent.OldValue == m_xInner )
                aTranslatedEvent.OldValue = makeAny(aTranslatedEvent.Source);

            // to prevent races:
            nClientId = m_nNotifierClient;
        }

        AccessibleEventNotifier::addEvent( nClientId, aTranslatedEvent );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapper::disposing() throw (::com::sun::star::uno::RuntimeException)
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

            // dispose the child cache/map
            m_pChildMapper->dispose();

            // let the base class dispose the inner object
            // before we do this, remove ourself as listener
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster > xBroadcaster( m_xInner, ::com::sun::star::uno::UNO_QUERY );
            OSL_ENSURE( xBroadcaster.is(), "OAccessibleContextWrapper::disposing(): inner context is no broadcaster!" );
            if ( xBroadcaster.is() )
                xBroadcaster->removeEventListener( this );

            OAccessibleContextWrapper_Base::disposing();
        }
        // --- </mutex lock> -----------------------------------------

        if ( nClientId )
            AccessibleEventNotifier::revokeClientNotifyDisposing( nClientId, *this );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OAccessibleContextWrapper::disposing( const EventObject& _rEvent )  throw (::com::sun::star::uno::RuntimeException)
    {
        // simply disambiguate this
        OAccessibleContextWrapper_Base::disposing( _rEvent );
    }

//.............................................................................
}   // namespace accessibility
//.............................................................................
