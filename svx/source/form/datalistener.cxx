 /*************************************************************************
 *
 *  $RCSfile: datalistener.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 11:21:38 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "datalistener.hxx"
#include "datanavi.hxx"

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::dom::events;

//............................................................................
namespace svxform
{
//............................................................................

    DataListener::DataListener( DataNavigatorWindow* pNaviWin ) :

        m_pNaviWin( pNaviWin )

    {
        DBG_ASSERT( m_pNaviWin, "DataListener::Ctor(): no navigator win" );
    }

    DataListener::~DataListener()
    {
    }

    // XContainerListener
    void SAL_CALL DataListener::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    void SAL_CALL DataListener::elementRemoved( const ContainerEvent& Event ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    void SAL_CALL DataListener::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    // xml::dom::events::XEventListener
    void SAL_CALL DataListener::handleEvent( const Reference< XEvent >& evt ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    // lang::XEventListener
    void SAL_CALL DataListener::disposing( const EventObject& Source ) throw (RuntimeException)
    {
        DBG_ERRORFILE( "disposing" );
    }

//............................................................................
}   // namespace svxform
//............................................................................

