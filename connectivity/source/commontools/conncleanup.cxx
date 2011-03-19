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
#include "precompiled_connectivity.hxx"
#include <connectivity/conncleanup.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/diagnose.h>

//.........................................................................
namespace dbtools
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    static const ::rtl::OUString& getActiveConnectionPropertyName()
    {
        static const ::rtl::OUString s_sActiveConnectionPropertyName( RTL_CONSTASCII_USTRINGPARAM( "ActiveConnection" ));
        return s_sActiveConnectionPropertyName;
    }

    //=====================================================================
    //= OAutoConnectionDisposer
    //=====================================================================
    //---------------------------------------------------------------------
    OAutoConnectionDisposer::OAutoConnectionDisposer(const Reference< XRowSet >& _rxRowSet, const Reference< XConnection >& _rxConnection)
        :m_xRowSet( _rxRowSet )
        ,m_bRSListening( sal_False )
        ,m_bPropertyListening( sal_False )
    {
        Reference< XPropertySet > xProps(_rxRowSet, UNO_QUERY);
        OSL_ENSURE(xProps.is(), "OAutoConnectionDisposer::OAutoConnectionDisposer: invalid rowset (no XPropertySet)!");

        if (!xProps.is())
            return;

        try
        {
            xProps->setPropertyValue( getActiveConnectionPropertyName(), makeAny( _rxConnection ) );
            m_xOriginalConnection = _rxConnection;
            startPropertyListening( xProps );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::OAutoConnectionDisposer: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::startPropertyListening( const Reference< XPropertySet >& _rxRowSet )
    {
        try
        {
            _rxRowSet->addPropertyChangeListener( getActiveConnectionPropertyName(), this );
            m_bPropertyListening = sal_True;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::startPropertyListening: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::stopPropertyListening( const Reference< XPropertySet >& _rxEventSource )
    {
        // prevent deletion of ourself while we're herein
        Reference< XInterface > xKeepAlive(static_cast< XWeak* >(this));

        try
        {   // remove ourself as property change listener
            OSL_ENSURE( _rxEventSource.is(), "OAutoConnectionDisposer::stopPropertyListening: invalid event source (no XPropertySet)!" );
            if ( _rxEventSource.is() )
            {
                _rxEventSource->removePropertyChangeListener( getActiveConnectionPropertyName(), this );
                m_bPropertyListening = sal_False;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::stopPropertyListening: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::startRowSetListening()
    {
        OSL_ENSURE( !m_bRSListening, "OAutoConnectionDisposer::startRowSetListening: already listening!" );
        try
        {
            if ( !m_bRSListening )
                m_xRowSet->addRowSetListener( this );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::startRowSetListening: caught an exception!" );
        }
        m_bRSListening = sal_True;
    }

    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::stopRowSetListening()
    {
        OSL_ENSURE( m_bRSListening, "OAutoConnectionDisposer::stopRowSetListening: not listening!" );
        try
        {
            m_xRowSet->removeRowSetListener( this );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::stopRowSetListening: caught an exception!" );
        }
        m_bRSListening = sal_False;
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
    {
        if ( _rEvent.PropertyName.equals( getActiveConnectionPropertyName() ) )
        {   // somebody set a new ActiveConnection

            Reference< XConnection > xNewConnection;
            _rEvent.NewValue >>= xNewConnection;

            if ( isRowSetListening() )
            {
                // we're listening at the row set, this means that the row set does not have our
                // m_xOriginalConnection as active connection anymore
                // So there are two possibilities
                // a. somebody sets a new connection which is not our original one
                // b. somebody sets a new connection, which is exactly the original one
                // a. we're not interested in a, but in b: In this case, we simply need to move to the state
                // we had originally: listen for property changes, do not listen for row set changes, and
                // do not dispose the connection until the row set does not need it anymore
                if ( xNewConnection.get() == m_xOriginalConnection.get() )
                {
                    stopRowSetListening();
                }
            }
            else
            {
                // start listening at the row set. We're allowed to dispose the old connection as soon
                // as the RowSet changed

                // Unfortunately, the our database form implementations sometimes fire the change of their
                // ActiveConnection twice. This is a error in forms/source/component/DatabaseForm.cxx, but
                // changing this would require incompatible changes we can't do for a while.
                // So for the moment, we have to live with it here.
                //
                // The only scenario where this doubled notification causes problems is when the connection
                // of the form is reset to the one we're responsible for (m_xOriginalConnection), so we
                // check this here.
                //
                // Yes, this is a HACK :(
                if ( xNewConnection.get() != m_xOriginalConnection.get() )
                {
#if OSL_DEBUG_LEVEL > 0
                    Reference< XConnection > xOldConnection;
                    _rEvent.OldValue >>= xOldConnection;
                    OSL_ENSURE( xOldConnection.get() == m_xOriginalConnection.get(), "OAutoConnectionDisposer::propertyChange: unexpected (original) property value!" );
#endif
                    startRowSetListening();
                }
            }
        }
    }

    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        // the rowset is beeing disposed, and nobody has set a new ActiveConnection in the meantime
        if ( isRowSetListening() )
            stopRowSetListening();

        clearConnection();

        if ( isPropertyListening() )
            stopPropertyListening( Reference< XPropertySet >( _rSource.Source, UNO_QUERY ) );
    }
    //---------------------------------------------------------------------
    void OAutoConnectionDisposer::clearConnection()
    {
        try
        {
            // dispose the old connection
            Reference< XComponent > xComp(m_xOriginalConnection, UNO_QUERY);
            if (xComp.is())
                xComp->dispose();
            m_xOriginalConnection.clear();
        }
        catch(Exception&)
        {
            OSL_FAIL("OAutoConnectionDisposer::clearConnection: caught an exception!");
        }
    }
    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::cursorMoved( const ::com::sun::star::lang::EventObject& /*event*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::rowChanged( const ::com::sun::star::lang::EventObject& /*event*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    //---------------------------------------------------------------------
    void SAL_CALL OAutoConnectionDisposer::rowSetChanged( const ::com::sun::star::lang::EventObject& /*event*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
        stopRowSetListening();
        clearConnection();

    }
    //---------------------------------------------------------------------

//.........................................................................
}   // namespace dbtools
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
