/*************************************************************************
 *
 *  $RCSfile: AsyncronousLink.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:15:07 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_ASYNCRONOUSLINK_HXX
#include "AsyncronousLink.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//==================================================================
//= OAsyncronousLink
//==================================================================
using namespace dbaui;
//------------------------------------------------------------------
OAsyncronousLink::OAsyncronousLink(const Link& _rHandler, ::vos::OMutex* _pEventSafety, ::vos::OMutex* _pDestructionSafety)
    :m_aHandler(_rHandler)
    ,m_pEventSafety(NULL)
    ,m_pDestructionSafety(NULL)
    ,m_bOwnMutexes(FALSE)
    ,m_nEventId(0)
{
    if (_pEventSafety && _pDestructionSafety)
    {
        m_pEventSafety = _pEventSafety;
        m_pDestructionSafety = _pDestructionSafety;
        m_bOwnMutexes = FALSE;
    }
    else
    {
        m_pEventSafety = new ::vos::OMutex;
        m_pDestructionSafety = new ::vos::OMutex;
        m_bOwnMutexes = TRUE;
    }
}

//------------------------------------------------------------------
OAsyncronousLink::~OAsyncronousLink()
{
    {
        ::vos::OGuard aEventGuard(*m_pEventSafety);
        if (m_nEventId)
            Application::RemoveUserEvent(m_nEventId);
        m_nEventId = 0;
    }

    {
        ::vos::OGuard aDestructionGuard(*m_pDestructionSafety);
        // this is just for the case we're deleted while another thread just handled the event :
        // if this other thread called our link while we were deleting the event here, the
        // link handler blocked. With leaving the above block it continued, but now we are prevented
        // to leave this destructor 'til the link handler recognizes that nEvent == 0 and leaves.
    }
    if (m_bOwnMutexes)
    {
        delete m_pEventSafety;
        delete m_pDestructionSafety;
    }
    m_pEventSafety = NULL;
    m_pDestructionSafety = NULL;
}


//------------------------------------------------------------------
void OAsyncronousLink::Call(void* _pArgument)
{
    ::vos::OGuard aEventGuard(*m_pEventSafety);
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = Application::PostUserEvent(LINK(this, OAsyncronousLink, OnAsyncCall));
}

//------------------------------------------------------------------
void OAsyncronousLink::CancelCall()
{
    ::vos::OGuard aEventGuard(*m_pEventSafety);
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = 0;
}

//------------------------------------------------------------------
IMPL_LINK(OAsyncronousLink, OnAsyncCall, void*, _pArg)
{
    {
        ::vos::OGuard aDestructionGuard(*m_pDestructionSafety);
        {
            ::vos::OGuard aEventGuard(*m_pEventSafety);
            if (!m_nEventId)
                // our destructor deleted the event just while we we're waiting for m_pEventSafety
                // -> get outta here
                return 0;
            m_nEventId = 0;
        }
    }
    if (m_aHandler.IsSet())
        return m_aHandler.Call(_pArg);

    return 0L;
}
