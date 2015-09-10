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

#include <dbaccess/AsynchronousLink.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>

// OAsynchronousLink
using namespace dbaui;
OAsynchronousLink::OAsynchronousLink( const Link<void*,void>& _rHandler )
    :m_aHandler(_rHandler)
    ,m_aEventSafety()
    ,m_aDestructionSafety()
    ,m_nEventId(0)
{
}

OAsynchronousLink::~OAsynchronousLink()
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
}

void OAsynchronousLink::Call( void* _pArgument )
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = Application::PostUserEvent( LINK( this, OAsynchronousLink, OnAsyncCall ), _pArgument );
}

void OAsynchronousLink::CancelCall()
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if ( m_nEventId )
        Application::RemoveUserEvent( m_nEventId );
    m_nEventId = 0;
}

IMPL_LINK_TYPED(OAsynchronousLink, OnAsyncCall, void*, _pArg, void)
{
    {
        ::osl::MutexGuard aDestructionGuard( m_aDestructionSafety );
        {
            ::osl::MutexGuard aEventGuard( m_aEventSafety );
            if (!m_nEventId)
                // our destructor deleted the event just while we are waiting for m_aEventSafety
                // -> get outta here
                return;
            m_nEventId = 0;
        }
    }
    if (m_aHandler.IsSet())
        m_aHandler.Call(_pArg);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
