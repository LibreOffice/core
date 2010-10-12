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
#include "precompiled_dbaccess.hxx"
#include "AsyncronousLink.hxx"
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

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
void OAsyncronousLink::Call( void* _pArgument )
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = Application::PostUserEvent( LINK( this, OAsyncronousLink, OnAsyncCall ), _pArgument );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
