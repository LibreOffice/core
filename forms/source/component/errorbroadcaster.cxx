/*************************************************************************
 *
 *  $RCSfile: errorbroadcaster.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:01:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef FORMS_ERRORBROADCASTER_HXX
#include "errorbroadcaster.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdb;
    using namespace ::dbtools;

    //=====================================================================
    //= OErrorBroadcaster
    //=====================================================================
    //---------------------------------------------------------------------
    OErrorBroadcaster::OErrorBroadcaster( ::cppu::OBroadcastHelper& _rBHelper )
        :m_rBHelper( _rBHelper )
        ,m_aErrorListeners( _rBHelper.rMutex )
    {
    }

    //---------------------------------------------------------------------
    OErrorBroadcaster::~OErrorBroadcaster( )
    {
        OSL_ENSURE( m_rBHelper.bDisposed || m_rBHelper.bInDispose,
            "OErrorBroadcaster::~OErrorBroadcaster: not disposed!" );
        // herein, we don't have a chance to do the dispose ourself ....

        OSL_ENSURE( 0 == m_aErrorListeners.getLength(),
            "OErrorBroadcaster::~OErrorBroadcaster: still have listeners!" );
        // either we're not disposed, or the derived class did not call our dispose from within their dispose
    }

    //---------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::disposing()
    {
        EventObject aDisposeEvent( static_cast< XSQLErrorBroadcaster* >( this ) );
        m_aErrorListeners.disposeAndClear( aDisposeEvent );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::onError( const SQLException& _rException, const ::rtl::OUString& _rContextDescription )
    {
        Any aError;
        if ( _rContextDescription.getLength() )
            aError = makeAny( prependContextInfo( _rException, static_cast< XSQLErrorBroadcaster* >( this ), _rContextDescription ) );
        else
            aError = makeAny( _rException );

        onError( SQLErrorEvent( static_cast< XSQLErrorBroadcaster* >( this ), aError ) );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::onError( const ::com::sun::star::sdb::SQLErrorEvent& _rError )
    {
        if ( m_aErrorListeners.getLength() )
        {

            ::cppu::OInterfaceIteratorHelper aIter( m_aErrorListeners );
            while ( aIter.hasMoreElements() )
                static_cast< XSQLErrorListener* >( aIter.next() )->errorOccured( _rError );
        }
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::addSQLErrorListener( const Reference< XSQLErrorListener >& _rxListener ) throw( RuntimeException )
    {
        m_aErrorListeners.addInterface( _rxListener );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OErrorBroadcaster::removeSQLErrorListener( const Reference< XSQLErrorListener >& _rxListener ) throw( RuntimeException )
    {
        m_aErrorListeners.removeInterface( _rxListener );
    }

//.........................................................................
}   // namespace frm
//.........................................................................

