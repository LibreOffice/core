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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
