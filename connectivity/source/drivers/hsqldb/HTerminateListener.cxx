/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HTerminateListener.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:30:54 $
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

#ifndef CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX
#include "HTerminateListener.hxx"
#endif
#ifndef CONNECTIVITY_HSQLDB_DRIVER_HXX
#include "hsqldb/HDriver.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................
    using namespace hsqldb;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::lang;
    using namespace ::rtl;

// XEventListener
void SAL_CALL OConnectionController::disposing( const EventObject& /*Source*/ )
throw( RuntimeException )
{
}

// XTerminateListener
void SAL_CALL OConnectionController::queryTermination( const EventObject& /*aEvent*/ )
throw( TerminationVetoException, RuntimeException )
{
}

void SAL_CALL OConnectionController::notifyTermination( const EventObject& /*aEvent*/ )
throw( RuntimeException )
{
    m_pDriver->shutdownConnections();
}

//........................................................................
}   // namespace connectivity
//........................................................................
