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

#include "propeventtranslation.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/NullPointerException.hpp>

//........................................................................
namespace pcr
{
//........................................................................

    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::NullPointerException;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
