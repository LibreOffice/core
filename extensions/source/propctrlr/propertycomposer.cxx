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

#include "propertycomposer.hxx"

#include <com/sun/star/lang/NullPointerException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

#include <functional>
#include <algorithm>
#include <map>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::inspection;

    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        //----------------------------------------------------------------
        struct SetPropertyValue : public ::std::unary_function< Reference< XPropertyHandler >, void >
        {
            ::rtl::OUString sPropertyName;
            const Any&      rValue;
            SetPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) : sPropertyName( _rPropertyName ), rValue( _rValue ) { }
            void operator()( const Reference< XPropertyHandler >& _rHandler )
            {
                _rHandler->setPropertyValue( sPropertyName, rValue );
            }
        };

        //----------------------------------------------------------------
        template < class BagType >
        void putIntoBag( const Sequence< typename BagType::value_type >& _rArray, BagType& /* [out] */ _rBag )
        {
            ::std::copy( _rArray.getConstArray(), _rArray.getConstArray() + _rArray.getLength(),
                ::std::insert_iterator< BagType >( _rBag, _rBag.begin() ) );
        }

        //----------------------------------------------------------------
        template < class BagType >
        void copyBagToArray( const BagType& /* [out] */ _rBag, Sequence< typename BagType::value_type >& _rArray )
        {
            _rArray.realloc( _rBag.size() );
            ::std::copy( _rBag.begin(), _rBag.end(), _rArray.getArray() );
        }
    }

    //====================================================================
    //= PropertyComposer
    //====================================================================

    // TODO: there are various places where we determine the first handler in our array which
    // supports a given property id. This is, at the moment, done with searching all handlers,
    // which is O( n * k ) at worst (n being the number of handlers, k being the maximum number
    // of supported properties per handler). Shouldn't we cache this? So that it is O( log k )?

    //--------------------------------------------------------------------
    PropertyComposer::PropertyComposer( const ::std::vector< Reference< XPropertyHandler > >& _rSlaveHandlers )
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
            for (   HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
                    loop != m_aSlaveHandlers.end();
                    ++loop
                )
            {
                if ( !loop->is() )
                    throw NullPointerException();
                (*loop)->addPropertyChangeListener( xMeMyselfAndI );
            }
        }
        osl_atomic_decrement( &m_refCount );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::inspect( const Reference< XInterface >& _rxIntrospectee ) throw (RuntimeException, NullPointerException)
    {
        MethodGuard aGuard( *this );

        for ( HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            (*loop)->inspect( _rxIntrospectee );
        }
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyComposer::getPropertyValue( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->getPropertyValue( _rPropertyName );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::setPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rValue ) throw (UnknownPropertyException, RuntimeException)
    {
        MethodGuard aGuard( *this );
        ::std::for_each( m_aSlaveHandlers.begin(), m_aSlaveHandlers.end(), SetPropertyValue( _rPropertyName, _rValue ) );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyComposer::convertToPropertyValue( const ::rtl::OUString& _rPropertyName, const Any& _rControlValue ) throw (UnknownPropertyException, RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->convertToPropertyValue( _rPropertyName, _rControlValue );
    }

    //--------------------------------------------------------------------
    Any SAL_CALL PropertyComposer::convertToControlValue( const ::rtl::OUString& _rPropertyName, const Any& _rPropertyValue, const Type& _rControlValueType ) throw (UnknownPropertyException, RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->convertToControlValue( _rPropertyName, _rPropertyValue, _rControlValueType );
    }

    //--------------------------------------------------------------------
    PropertyState SAL_CALL PropertyComposer::getPropertyState( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
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
        for ( HandlerArray::const_iterator loop = ( m_aSlaveHandlers.begin() + 1 );
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

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::addPropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aPropertyListeners.addListener( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::removePropertyChangeListener( const Reference< XPropertyChangeListener >& _rxListener ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aPropertyListeners.removeListener( _rxListener );
    }

    //--------------------------------------------------------------------
    Sequence< Property > SAL_CALL PropertyComposer::getSupportedProperties() throw (RuntimeException)
    {
        MethodGuard aGuard( *this );

        if ( !m_bSupportedPropertiesAreKnown )
        {
            // we support a property if and only if all of our slaves support it

            // initially, use all the properties of an arbitrary handler (we take the first one)
            putIntoBag( (*m_aSlaveHandlers.begin())->getSupportedProperties(), m_aSupportedProperties );

            // now intersect with the properties of *all* other handlers
            for ( HandlerArray::const_iterator loop = ( m_aSlaveHandlers.begin() + 1 );
                loop != m_aSlaveHandlers.end();
                ++loop
                )
            {
                // the properties supported by the current handler
                PropertyBag aThisRound;
                putIntoBag( (*loop)->getSupportedProperties(), aThisRound );

                // the intersection of those properties with all we already have
                PropertyBag aIntersection;
                ::std::set_intersection( aThisRound.begin(), aThisRound.end(), m_aSupportedProperties.begin(), m_aSupportedProperties.end(),
                    ::std::insert_iterator< PropertyBag >( aIntersection, aIntersection.begin() ), PropertyLessByName() );

                m_aSupportedProperties.swap( aIntersection );
                if ( m_aSupportedProperties.empty() )
                    break;
            }

            // remove those properties which are not composable
            for (   PropertyBag::iterator check = m_aSupportedProperties.begin();
                    check != m_aSupportedProperties.end();
                )
            {
                sal_Bool bIsComposable = isComposable( check->Name );
                if ( !bIsComposable )
                {
                    PropertyBag::iterator next = check; ++next;
                    m_aSupportedProperties.erase( check );
                    check = next;
                }
                else
                    ++check;
            }

            m_bSupportedPropertiesAreKnown = true;
        }

        Sequence< Property > aSurvived;
        copyBagToArray( m_aSupportedProperties, aSurvived );
        return aSurvived;
    }

    //--------------------------------------------------------------------
    void uniteStringArrays( const PropertyComposer::HandlerArray& _rHandlers, Sequence< ::rtl::OUString > (SAL_CALL XPropertyHandler::*pGetter)( void ),
        Sequence< ::rtl::OUString >& /* [out] */ _rUnion )
    {
        ::std::set< ::rtl::OUString > aUnitedBag;

        Sequence< ::rtl::OUString > aThisRound;
        for ( PropertyComposer::HandlerArray::const_iterator loop = _rHandlers.begin();
              loop != _rHandlers.end();
              ++loop
            )
        {
            aThisRound = (loop->get()->*pGetter)();
            putIntoBag( aThisRound, aUnitedBag );
        }

        copyBagToArray( aUnitedBag, _rUnion );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL PropertyComposer::getSupersededProperties( ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );

        // we supersede those properties which are superseded by at least one of our slaves
        Sequence< ::rtl::OUString > aSuperseded;
        uniteStringArrays( m_aSlaveHandlers, &XPropertyHandler::getSupersededProperties, aSuperseded );
        return aSuperseded;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL PropertyComposer::getActuatingProperties( ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );

        // we're interested in those properties which at least one handler wants to have
        Sequence< ::rtl::OUString > aActuating;
        uniteStringArrays( m_aSlaveHandlers, &XPropertyHandler::getActuatingProperties, aActuating );
        return aActuating;
    }

    //--------------------------------------------------------------------
    LineDescriptor SAL_CALL PropertyComposer::describePropertyLine( const ::rtl::OUString& _rPropertyName,
        const Reference< XPropertyControlFactory >& _rxControlFactory )
        throw (UnknownPropertyException, NullPointerException, RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->describePropertyLine( _rPropertyName, _rxControlFactory );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL PropertyComposer::isComposable( const ::rtl::OUString& _rPropertyName ) throw (UnknownPropertyException, RuntimeException)
    {
        MethodGuard aGuard( *this );
        return m_aSlaveHandlers[0]->isComposable( _rPropertyName );
    }

    //--------------------------------------------------------------------
    InteractiveSelectionResult SAL_CALL PropertyComposer::onInteractivePropertySelection( const ::rtl::OUString& _rPropertyName, sal_Bool _bPrimary, Any& _rData, const Reference< XObjectInspectorUI >& _rxInspectorUI ) throw (UnknownPropertyException, NullPointerException, RuntimeException)
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

    //--------------------------------------------------------------------
    void PropertyComposer::impl_ensureUIRequestComposer( const Reference< XObjectInspectorUI >& _rxInspectorUI )
    {
        OSL_ENSURE( !m_pUIRequestComposer.get() || m_pUIRequestComposer->getDelegatorUI().get() == _rxInspectorUI.get(),
            "PropertyComposer::impl_ensureUIRequestComposer: somebody's changing the horse in the mid of the race!" );

        if ( !m_pUIRequestComposer.get() )
            m_pUIRequestComposer.reset( new ComposedPropertyUIUpdate( _rxInspectorUI, this ) );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::actuatingPropertyChanged( const ::rtl::OUString& _rActuatingPropertyName, const Any& _rNewValue, const Any& _rOldValue, const Reference< XObjectInspectorUI >& _rxInspectorUI, sal_Bool _bFirstTimeInit ) throw (NullPointerException, RuntimeException)
    {
        if ( !_rxInspectorUI.is() )
            throw NullPointerException();

        MethodGuard aGuard( *this );

        impl_ensureUIRequestComposer( _rxInspectorUI );
        ComposedUIAutoFireGuard aAutoFireGuard( *m_pUIRequestComposer );

        // ask all handlers which expressed interest in this particular property, and "compose" their
        // commands for the UIUpdater
        for (   HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
                loop != m_aSlaveHandlers.end();
                ++loop
            )
        {
            // TODO: make this cheaper (cache it?)
            const StlSyntaxSequence< ::rtl::OUString > aThisHandlersActuatingProps = (*loop)->getActuatingProperties();
            for (   StlSyntaxSequence< ::rtl::OUString >::const_iterator loopProps = aThisHandlersActuatingProps.begin();
                    loopProps != aThisHandlersActuatingProps.end();
                    ++loopProps
                )
            {
                if ( *loopProps == _rActuatingPropertyName )
                {
                    (*loop)->actuatingPropertyChanged( _rActuatingPropertyName, _rNewValue, _rOldValue,
                        m_pUIRequestComposer->getUIForPropertyHandler( *loop ),
                        _bFirstTimeInit );
                    break;
                }
            }
        }
    }

    //--------------------------------------------------------------------
    IMPLEMENT_FORWARD_XCOMPONENT( PropertyComposer, PropertyComposer_Base )

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::disposing()
    {
        MethodGuard aGuard( *this );

        // dispose our slave handlers
        for ( PropertyComposer::HandlerArray::const_iterator loop = m_aSlaveHandlers.begin();
              loop != m_aSlaveHandlers.end();
              ++loop
            )
        {
            (*loop)->removePropertyChangeListener( this );
            (*loop)->dispose();
        }

        clearContainer( m_aSlaveHandlers );

        if ( m_pUIRequestComposer.get() )
            m_pUIRequestComposer->dispose();
        m_pUIRequestComposer.reset( NULL );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
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
            DBG_UNHANDLED_EXCEPTION();
        }
        m_aPropertyListeners.notify( aTranslatedEvent, &XPropertyChangeListener::propertyChange );
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyComposer::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        MethodGuard aGuard( *this );
        m_aPropertyListeners.disposing( Source );
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL PropertyComposer::suspend( sal_Bool _bSuspend ) throw (RuntimeException)
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
                        (*loop)->suspend( sal_False );
                    }
                    while ( loop != m_aSlaveHandlers.begin() );
                }
                return false;
            }
        }
        return true;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL PropertyComposer::hasPropertyByName( const ::rtl::OUString& _rName ) throw (RuntimeException)
    {
        return impl_isSupportedProperty_nothrow( _rName );
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
