/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "propeventtranslation.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::NullPointerException;
    /** === end UNO using === **/

    //====================================================================
    //= PropertyEventTranslation
    //====================================================================
    //--------------------------------------------------------------------
    PropertyEventTranslation::PropertyEventTranslation( const Reference< XPropertyChangeListener >& _rxDelegator,
        const Reference< XInterface >& _rxTranslatedEventSource )
        :m_xDelegator( _rxDelegator )
        ,m_xTranslatedEventSource( _rxTranslatedEventSource )
    {
        if ( !m_xDelegator.is() )
            throw NullPointerException();
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyEventTranslation::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
    {
        if ( !m_xDelegator.is() )
            throw DisposedException();

        if ( !m_xTranslatedEventSource.is() )
            m_xDelegator->propertyChange( evt );
        else
        {
            PropertyChangeEvent aTranslatedEvent( evt );
            aTranslatedEvent.Source = m_xTranslatedEventSource;
            m_xDelegator->propertyChange( aTranslatedEvent );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL PropertyEventTranslation::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        if ( !m_xDelegator.is() )
            throw DisposedException();

        if ( !m_xTranslatedEventSource.is() )
            m_xDelegator->disposing( Source );
        else
        {
            EventObject aTranslatedEvent( Source );
            aTranslatedEvent.Source = m_xTranslatedEventSource;
            m_xDelegator->disposing( aTranslatedEvent );
        }

        m_xDelegator.clear();
        m_xTranslatedEventSource.clear();
    }

//........................................................................
} // namespace pcr
//........................................................................

