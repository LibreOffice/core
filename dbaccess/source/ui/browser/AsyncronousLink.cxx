/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AsyncronousLink.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:12:40 $
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
#include "precompiled_dbaccess.hxx"
#ifndef DBAUI_ASYNCRONOUSLINK_HXX
#include "AsyncronousLink.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//==================================================================
//= OAsyncronousLink
//==================================================================
using namespace dbaui;
DBG_NAME(OAsyncronousLink)
//------------------------------------------------------------------
OAsyncronousLink::OAsyncronousLink( const Link& _rHandler )
    :m_aHandler(_rHandler)
    ,m_aEventSafety()
    ,m_aDestructionSafety()
    ,m_nEventId(0)
{
    DBG_CTOR(OAsyncronousLink,NULL);
}

//------------------------------------------------------------------
OAsyncronousLink::~OAsyncronousLink()
{
    {
        ::osl::MutexGuard aEventGuard( m_aEventSafety );
        if ( m_nEventId )
            Application::RemoveUserEvent(m_nEventId);
        m_nEventId = 0;
    }

    {
        ::osl::MutexGuard aDestructionGuard( m_aDestructionSafety );
        // this is just for the case we're deleted while another thread just handled the event :
        // if this other thread called our link while we were deleting the event here, the
        // link handler blocked. With leaving the above block it continued, but now we are prevented
        // to leave this destructor 'til the link handler recognizes that nEvent == 0 and leaves.
    }
    DBG_DTOR(OAsyncronousLink,NULL);
}


//------------------------------------------------------------------
void OAsyncronousLink::Call(void* /*_pArgument*/)
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = Application::PostUserEvent(LINK(this, OAsyncronousLink, OnAsyncCall));
}

//------------------------------------------------------------------
void OAsyncronousLink::CancelCall()
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if ( m_nEventId )
        Application::RemoveUserEvent( m_nEventId );
    m_nEventId = 0;
}

//------------------------------------------------------------------
IMPL_LINK(OAsyncronousLink, OnAsyncCall, void*, _pArg)
{
    {
        ::osl::MutexGuard aDestructionGuard( m_aDestructionSafety );
        {
            ::osl::MutexGuard aEventGuard( m_aEventSafety );
            if (!m_nEventId)
                // our destructor deleted the event just while we we're waiting for m_aEventSafety
                // -> get outta here
                return 0;
            m_nEventId = 0;
        }
    }
    if (m_aHandler.IsSet())
        return m_aHandler.Call(_pArg);

    return 0L;
}
