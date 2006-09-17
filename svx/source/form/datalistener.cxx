 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datalistener.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:03:33 $
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
#include "precompiled_svx.hxx"

#include "datalistener.hxx"
#include "datanavi.hxx"

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
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
    void SAL_CALL DataListener::elementInserted( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    void SAL_CALL DataListener::elementRemoved( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    void SAL_CALL DataListener::elementReplaced( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    // XFrameActionListener
    void SAL_CALL DataListener::frameAction( const FrameActionEvent& rActionEvt ) throw (RuntimeException)
    {
        if ( FrameAction_COMPONENT_ATTACHED == rActionEvt.Action ||
             FrameAction_COMPONENT_REATTACHED == rActionEvt.Action )
        {
            m_pNaviWin->NotifyChanges( FrameAction_COMPONENT_REATTACHED == rActionEvt.Action );
        }
    }

    // xml::dom::events::XEventListener
    void SAL_CALL DataListener::handleEvent( const Reference< XEvent >& /*evt*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    // lang::XEventListener
    void SAL_CALL DataListener::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
    {
        DBG_ERRORFILE( "disposing" );
    }

//............................................................................
}   // namespace svxform
//............................................................................

