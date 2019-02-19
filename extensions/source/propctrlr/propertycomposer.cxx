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

#include "propertycomposer.hxx"

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <iterator>
#include <map>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::inspection;


    //= helper

    namespace
    {

        struct SetPropertyValue
        {
            OUString sPropertyName;
            const Any&      rValue;
            SetPropertyValue( const OUString& _rPropertyName, const Any& _rValue ) : sPropertyName( _rPropertyName ), rValue( _rValue ) { }
            void operator()( const Reference< XPropertyHandler >& _rHandler )
            {
                _rHandler->setPropertyValue( sPropertyName, rValue );
            }
        };


        template < class BagType >
        void putIntoBag( const Sequence< typename BagType::value_type >& _rArray, BagType& /* [out] */ _rBag )
        {
            std::copy( _rArray.begin(), _rArray.end(),
                         std::insert_iterator< BagType >( _rBag, _rBag.begin() ) );
        }


        template < class BagType >
        void copyBagToArray( const BagType& /* [out] */ _rBag, Sequence< typename BagType::value_type >& _rArray )
        {
            _rArray.realloc( _rBag.size() );
            std::copy( _rBag.begin(), _rBag.end(), _rArray.getArray() );
        }
    }


    //= PropertyComposer


    // TODO: there are various places where we determine the first handler in our array which
    // supports a given property id. This is, at the moment, done with searching all handlers,
    // which is O( n * k ) at worst (n being the number of handlers, k being the maximum number
    // of supported properties per handler). Shouldn't we cache this? So that it is O( log k )?


    PropertyComposer::PropertyComposer( const std::vector< Reference< XPropertyHandler > >& _rSlaveHandlers )
        :PropertyComposer_Base          ( m_aMutex          )
        ,m_aSlaveHandlers               ( _rSlaveHandlers   )
        ,m_aPropertyListeners           ( m_aMutex          )
        ,m_bSupportedPropertiesAreKnown ( false             )
    {
        if ( m_aSlaveHandlers.empty() )
            throw IllegalArgumentException();

        osl_atomic_increment( &m_refCount );
        {
            Reference< XPropertyChangeListener > xMeMyselfAndI( this );
            for (auto const& slaveHandler : m_aSlaveHandlers)
            {
                if ( !slaveHandler.is() )
                    throw NullPointerException();
                slaveHandler->addPropertyChangeListener( xMeMyselfAndI );
            }
        }
        osl_atomic_decrement( &m_refCount );
    }


    void SAL_CALL PropertyComposer::inspect( const Reference< XInterface >& _rxIntrospectee )
    {
        MethodGuard aGuard( *this );

        for (auto const& slaveHandler : m_aSlaveHandlers)
        {
            slaveHandler->inspect( _rxIntrospectee );
        }
    }


    Any SAL_CALL PropertyComposer::getPropertyValue( const OUString& _rPropertyName )
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->getPropertyValue( _rPropertyName );
    }


    void SAL_CALL PropertyComposer::setPropertyValue( const OUString& _rPropertyName, const Any& _rValue )
    {
        MethodGuard aGuard( *this );
        std::for_each( m_aSlaveHandlers.begin(), m_aSlaveHandlers.end(), SetPropertyValue( _rPropertyName, _rValue ) );
    }


    Any SAL_CALL PropertyComposer::convertToPropertyValue( const OUString& _rPropertyName, const Any& _rControlValue )
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->convertToPropertyValue( _rPropertyName, _rControlValue );
    }


    Any SAL_CALL PropertyComposer::convertToControlValue( const OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType )
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->convertToControlValue( _rPropertyName, _rPropertyValue, _rControlValueType );
    }


    PropertyState SAL_CALL PropertyComposer::getPropertyState( const OUString& _rPropertyName )
    {
        MethodGuard aGuard( *this );

        // assume DIRECT for the moment. This will stay this way if *all* slaves
        // tell the property has DIRECT state, and if *all* values equal
        PropertyState eState = PropertyState_DIRECT_VALUE;

        // check the master state
        Reference< XPropertyHandler > xPrimary( *m_aSlaveHandlers.begin() );
        Any aPrimaryValue = xPrimary->getPropertyValue( _rPropertyName );
        eState = xPrimary->getPropertyState( _rPropertyName );

        // loop through the secondary sets
        PropertyState eSecondaryState = PropertyState_DIRECT_VALUE;
        for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin() + 1;
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            // the secondary state
            eSecondaryState = (*loop)->getPropertyState( _rPropertyName );

            // the secondary value
            Any aSecondaryValue( (*loop)->getPropertyValue( _rPropertyName ) );

            if  (   ( PropertyState_AMBIGUOUS_VALUE == eSecondaryState )    // secondary is ambiguous
                ||  ( aPrimaryValue != aSecondaryValue )                    // unequal values
                )
            {
                eState = PropertyState_AMBIGUOUS_VALUE;
                break;
            }
        }

        return eState;
    }


    void SAL_CALL PropertyComposer::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
    {
        MethodGuard aGuard( *this );
        m_aPropertyListeners.addListener( _rxListener );
    }


    void SAL_CALL PropertyComposer::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener )
    {
        MethodGuard aGuard( *this );
        m_aPropertyListeners.removeListener( _rxListener );
    }


    Sequence< Property > SAL_CALL PropertyComposer::getSupportedProperties()
    {
        MethodGuard aGuard( *this );

        if ( !m_bSupportedPropertiesAreKnown )
        {
            // we support a property if and only if all of our slaves support it

            // initially, use all the properties of an arbitrary handler (we take the first one)
            putIntoBag( (*m_aSlaveHandlers.begin())->getSupportedProperties(), m_aSupportedProperties );

            // now intersect with the properties of *all* other handlers
            for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin() + 1;
                loop != m_aSlaveHandlers.end();
                ++loop
                )
            {
                // the properties supported by the current handler
                PropertyBag aThisRound;
                putIntoBag( (*loop)->getSupportedProperties(), aThisRound );

                // the intersection of those properties with all we already have
                PropertyBag aIntersection;
                std::set_intersection( aThisRound.begin(), aThisRound.end(), m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
                    std::insert_iterator< PropertyBag >( aIntersection, aIntersection.begin() ), PropertyLessByName() );

                m_aSupportedProperties.swap( aIntersection );
                if ( m_aSupportedProperties.empty() )
                    break;
            }

            // remove those properties which are not composable
            for (   PropertyBag::iterator check = m_aSupportedProperties.begin();
                    check != m_aSupportedProperties.end();
                )
            {
                bool bIsComposable = isComposable( check->Name );
                if ( !bIsComposable )
                {
                    check = m_aSupportedProperties.erase( check );
                }
                else
                    ++check;
            }

            m_bSupportedPropertiesAreKnown = true;
        }

        return comphelper::containerToSequence( m_aSupportedProperties );
    }


    static void uniteStringArrays( const PropertyComposer::HandlerArray& _rHandlers, Sequence< OUString > (SAL_CALL XPropertyHandler::*pGetter)( ),
        Sequence< OUString >& /* [out] */ _rUnion )
    {
        std::set< OUString > aUnitedBag;

        Sequence< OUString > aThisRound;
        for (auto const& handler : _rHandlers)
        {
            aThisRound = (handler.get()->*pGetter)();
            putIntoBag( aThisRound, aUnitedBag );
        }

        copyBagToArray( aUnitedBag, _rUnion );
    }


    Sequence< OUString > SAL_CALL PropertyComposer::getSupersededProperties( )
    {
        MethodGuard aGuard( *this );

        // we supersede those properties which are superseded by at least one of our slaves
        Sequence< OUString > aSuperseded;
        uniteStringArrays( m_aSlaveHandlers, &XPropertyHandler::getSupersededProperties, aSuperseded );
        return aSuperseded;
    }


    Sequence< OUString > SAL_CALL PropertyComposer::getActuatingProperties( )
    {
        MethodGuard aGuard( *this );

        // we're interested in those properties which at least one handler wants to have
        Sequence< OUString > aActuating;
        uniteStringArrays( m_aSlaveHandlers, &XPropertyHandler::getActuatingProperties, aActuating );
        return aActuating;
    }


    LineDescriptor SAL_CALL PropertyComposer::describePropertyLine( const OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->describePropertyLine( _rPropertyName, _rxControlFactory );
    }


    sal_Bool SAL_CALL PropertyComposer::isComposable( const OUString& _rPropertyName )
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->isComposable( _rPropertyName );
    }


    InteractiveSelectionResult SAL_CALL PropertyComposer::onInteractivePropertySelection( const OUString& _rPropertyName, sal_Bool _bPrimary, Any& _rData, const Reference< XObjectInspectorUI >& _rxInspectorUI )
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        MethodGuard aGuard( *this );

        impl_ensureUIRequestComposer( _rxInspectorUI );
        ComposedUIAutoFireGuard aAutoFireGuard( *m_pUIRequestComposer );

        // ask the first of the handlers
        InteractiveSelectionResult eResult = (*m_aSlaveHandlers.begin())->onInteractivePropertySelection(
                _rPropertyName,
                _bPrimary,
                _rData,
                m_pUIRequestComposer->getUIForPropertyHandler( *m_aSlaveHandlers.begin() )
            );

        switch ( eResult )
        {
        case InteractiveSelectionResult_Cancelled:
            // fine
            break;

        case InteractiveSelectionResult_Success:
        case InteractiveSelectionResult_Pending:
            OSL_FAIL( "PropertyComposer::onInteractivePropertySelection: no chance to forward the new value to the other handlers!" );
            // This means that we cannot know the new property value, which either has already been set
            // at the first component ("Success"), or will be set later on once the asynchronous input
            // is finished ("Pending"). So, we also cannot forward this new property value to the other
            // handlers.
            // We would need to be a listener at the property at the first component, but even this wouldn't
            // be sufficient, since the property handler is free to change *any* property during a dedicated
            // property UI.
            eResult = InteractiveSelectionResult_Cancelled;
            break;

        case InteractiveSelectionResult_ObtainedValue:
            // OK. Our own caller will pass this as setPropertyValue, and we will then pass it to
            // all slave handlers
            break;

        default:
            OSL_FAIL( "OPropertyBrowserController::onInteractivePropertySelection: unknown result value!" );
            break;
        }

        return eResult;
    }


    void PropertyComposer::impl_ensureUIRequestComposer( const Reference< XObjectInspectorUI >& _rxInspectorUI )
    {
        OSL_ENSURE(!m_pUIRequestComposer
                       || m_pUIRequestComposer->getDelegatorUI().get() == _rxInspectorUI.get(),
                   "PropertyComposer::impl_ensureUIRequestComposer: somebody's changing the horse "
                   "in the mid of the race!");

        if (!m_pUIRequestComposer)
            m_pUIRequestComposer.reset( new ComposedPropertyUIUpdate( _rxInspectorUI, this ) );
    }


    void SAL_CALL PropertyComposer::actuatingPropertyChanged( const OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& _rOldValue, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit )
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        MethodGuard aGuard( *this );

        impl_ensureUIRequestComposer( _rxInspectorUI );
        ComposedUIAutoFireGuard aAutoFireGuard( *m_pUIRequestComposer );

        // ask all handlers which expressed interest in this particular property, and "compose" their
        // commands for the UIUpdater
        for (auto const& slaveHandler : m_aSlaveHandlers)
        {
            // TODO: make this cheaper (cache it?)
            const StlSyntaxSequence< OUString > aThisHandlersActuatingProps( slaveHandler->getActuatingProperties() );
            for (const auto & aThisHandlersActuatingProp : aThisHandlersActuatingProps)
            {
                if ( aThisHandlersActuatingProp == _rActuatingPropertyName )
                {
                    slaveHandler->actuatingPropertyChanged( _rActuatingPropertyName, _rNewValue, _rOldValue,
                        m_pUIRequestComposer->getUIForPropertyHandler(slaveHandler),
                        _bFirstTimeInit );
                    break;
                }
            }
        }
    }


    IMPLEMENT_FORWARD_XCOMPONENT( PropertyComposer, PropertyComposer_Base )


    void SAL_CALL PropertyComposer::disposing()
    {
        MethodGuard aGuard( *this );

        // dispose our slave handlers
        for (auto const& slaveHandler : m_aSlaveHandlers)
        {
            slaveHandler->removePropertyChangeListener( this );
            slaveHandler->dispose();
        }

        clearContainer( m_aSlaveHandlers );

        if (m_pUIRequestComposer)
            m_pUIRequestComposer->dispose();
        m_pUIRequestComposer.reset();
    }


    void SAL_CALL PropertyComposer::propertyChange( const PropertyChangeEvent& evt )
    {
        if ( !impl_isSupportedProperty_nothrow( evt.PropertyName ) )
            // A slave handler might fire events for more properties than we support. Ignore those.
            return;

        PropertyChangeEvent aTranslatedEvent( evt );
        try
        {
            aTranslatedEvent.NewValue = getPropertyValue( evt.PropertyName );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("extensions.propctrlr");
        }
        m_aPropertyListeners.notify( aTranslatedEvent, &XPropertyChangeListener::propertyChange );
    }


    void SAL_CALL PropertyComposer::disposing( const EventObject& Source )
    {
        MethodGuard aGuard( *this );
        m_aPropertyListeners.disposing( Source );
    }


    sal_Bool SAL_CALL PropertyComposer::suspend( sal_Bool _bSuspend )
    {
        MethodGuard aGuard( *this );
        for ( PropertyComposer::HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            if ( !(*loop)->suspend( _bSuspend ) )
            {
                if ( _bSuspend && ( loop != m_aSlaveHandlers.begin() ) )
                {
                    // if we tried to suspend, but one of the slave handlers vetoed,
                    // re-activate the handlers which actually did *not* veto
                    // the suspension
                    do
                    {
                        --loop;
                        (*loop)->suspend( false );
                    }
                    while ( loop != m_aSlaveHandlers.begin() );
                }
                return false;
            }
        }
        return true;
    }


    bool PropertyComposer::hasPropertyByName( const OUString& _rName )
    {
        return impl_isSupportedProperty_nothrow( _rName );
    }


} // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
