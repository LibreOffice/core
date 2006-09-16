/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propeventtranslation.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:22:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPEVENTTRANSLATION_HXX
#include "propeventtranslation.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
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

