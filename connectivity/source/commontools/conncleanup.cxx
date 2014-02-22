/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <connectivity/conncleanup.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/diagnose.h>


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;

    
    static const OUString& getActiveConnectionPropertyName()
    {
        static const OUString s_sActiveConnectionPropertyName( "ActiveConnection" );
        return s_sActiveConnectionPropertyName;
    }

    
    
    
    
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

    
    void OAutoConnectionDisposer::stopPropertyListening( const Reference< XPropertySet >& _rxEventSource )
    {
        
        Reference< XInterface > xKeepAlive(static_cast< XWeak* >(this));

        try
        {   
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

    
    void SAL_CALL OAutoConnectionDisposer::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
    {
        if ( _rEvent.PropertyName.equals( getActiveConnectionPropertyName() ) )
        {   

            Reference< XConnection > xNewConnection;
            _rEvent.NewValue >>= xNewConnection;

            if ( isRowSetListening() )
            {
                
                
                
                
                
                
                
                
                if ( xNewConnection.get() == m_xOriginalConnection.get() )
                {
                    stopRowSetListening();
                }
            }
            else
            {
                
                

                
                
                
                
                //
                
                
                
                //
                
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

    
    void SAL_CALL OAutoConnectionDisposer::disposing( const EventObject& _rSource ) throw (RuntimeException)
    {
        
        if ( isRowSetListening() )
            stopRowSetListening();

        clearConnection();

        if ( isPropertyListening() )
            stopPropertyListening( Reference< XPropertySet >( _rSource.Source, UNO_QUERY ) );
    }
    
    void OAutoConnectionDisposer::clearConnection()
    {
        try
        {
            
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
    
    void SAL_CALL OAutoConnectionDisposer::cursorMoved( const ::com::sun::star::lang::EventObject& /*event*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    
    void SAL_CALL OAutoConnectionDisposer::rowChanged( const ::com::sun::star::lang::EventObject& /*event*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
    }
    
    void SAL_CALL OAutoConnectionDisposer::rowSetChanged( const ::com::sun::star::lang::EventObject& /*event*/ ) throw (::com::sun::star::uno::RuntimeException)
    {
        stopRowSetListening();
        clearConnection();

    }
    


}   


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
