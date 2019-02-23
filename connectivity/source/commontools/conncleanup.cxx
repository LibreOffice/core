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

#include <connectivity/conncleanup.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <osl/diagnose.h>


namespace dbtools
{


    using namespace css::uno;
    using namespace css::beans;
    using namespace css::sdbc;
    using namespace css::lang;

    static const char ACTIVE_CONNECTION_PROPERTY_NAME[] = "ActiveConnection";

    OAutoConnectionDisposer::OAutoConnectionDisposer(const Reference< XRowSet >& _rxRowSet, const Reference< XConnection >& _rxConnection)
        :m_xRowSet( _rxRowSet )
        ,m_bRSListening( false )
        ,m_bPropertyListening( false )
    {
        Reference< XPropertySet > xProps(_rxRowSet, UNO_QUERY);
        OSL_ENSURE(xProps.is(), "OAutoConnectionDisposer::OAutoConnectionDisposer: invalid rowset (no XPropertySet)!");

        if (!xProps.is())
            return;

        try
        {
            xProps->setPropertyValue( ACTIVE_CONNECTION_PROPERTY_NAME, makeAny( _rxConnection ) );
            m_xOriginalConnection = _rxConnection;
            startPropertyListening( xProps );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::OAutoConnectionDisposer: caught an exception!" );
        }
    }


    void OAutoConnectionDisposer::startPropertyListening( const Reference< XPropertySet >& _rxRowSet )
    {
        try
        {
            _rxRowSet->addPropertyChangeListener( ACTIVE_CONNECTION_PROPERTY_NAME, this );
            m_bPropertyListening = true;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::startPropertyListening: caught an exception!" );
        }
    }


    void OAutoConnectionDisposer::stopPropertyListening( const Reference< XPropertySet >& _rxEventSource )
    {
        // prevent deletion of ourself while we're herein
        Reference< XInterface > xKeepAlive(static_cast< XWeak* >(this));

        try
        {   // remove ourself as property change listener
            OSL_ENSURE( _rxEventSource.is(), "OAutoConnectionDisposer::stopPropertyListening: invalid event source (no XPropertySet)!" );
            if ( _rxEventSource.is() )
            {
                _rxEventSource->removePropertyChangeListener( ACTIVE_CONNECTION_PROPERTY_NAME, this );
                m_bPropertyListening = false;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAutoConnectionDisposer::stopPropertyListening: caught an exception!" );
        }
    }


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
        m_bRSListening = true;
    }


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
        m_bRSListening = false;
    }


    void SAL_CALL OAutoConnectionDisposer::propertyChange( const PropertyChangeEvent& _rEvent )
    {
        if ( _rEvent.PropertyName == ACTIVE_CONNECTION_PROPERTY_NAME )
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


    void SAL_CALL OAutoConnectionDisposer::disposing( const EventObject& _rSource )
    {
        // the rowset is being disposed, and nobody has set a new ActiveConnection in the meantime
        if ( isRowSetListening() )
            stopRowSetListening();

        clearConnection();

        if ( m_bPropertyListening )
            stopPropertyListening( Reference< XPropertySet >( _rSource.Source, UNO_QUERY ) );
    }

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

    void SAL_CALL OAutoConnectionDisposer::cursorMoved( const css::lang::EventObject& /*event*/ )
    {
    }

    void SAL_CALL OAutoConnectionDisposer::rowChanged( const css::lang::EventObject& /*event*/ )
    {
    }

    void SAL_CALL OAutoConnectionDisposer::rowSetChanged( const css::lang::EventObject& /*event*/ )
    {
        stopRowSetListening();
        clearConnection();

    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
