/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conncleanup.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:05:14 $
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

#ifndef _CONNECTIVITY_CONNCLEANUP_HXX_
#include <connectivity/conncleanup.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
        static const ::rtl::OUString s_sActiveConnectionPropertyName = ::rtl::OUString::createFromAscii("ActiveConnection");
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
            OSL_ENSURE( sal_False, "OAutoConnectionDisposer::OAutoConnectionDisposer: caught an exception!" );
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
            OSL_ENSURE( sal_False, "OAutoConnectionDisposer::startPropertyListening: caught an exception!" );
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
            OSL_ENSURE( sal_False, "OAutoConnectionDisposer::stopPropertyListening: caught an exception!" );
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
            OSL_ENSURE( sal_False, "OAutoConnectionDisposer::startRowSetListening: caught an exception!" );
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
            OSL_ENSURE( sal_False, "OAutoConnectionDisposer::stopRowSetListening: caught an exception!" );
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
                //
                // 94407 - 08.11.2001 - fs@openoffice.org
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
            OSL_ENSURE(sal_False, "OAutoConnectionDisposer::clearConnection: caught an exception!");
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

