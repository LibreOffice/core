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

#include <dbaccess/AsyncronousLink.hxx>
#include <vcl/svapp.hxx>
#include <tools/debug.hxx>


using namespace dbaui;
OAsyncronousLink::OAsyncronousLink( const Link& _rHandler )
    :m_aHandler(_rHandler)
    ,m_aEventSafety()
    ,m_aDestructionSafety()
    ,m_nEventId(0)
{
}

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
        
        
        
        
    }
}

void OAsyncronousLink::Call( void* _pArgument )
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = Application::PostUserEvent( LINK( this, OAsyncronousLink, OnAsyncCall ), _pArgument );
}

void OAsyncronousLink::CancelCall()
{
    ::osl::MutexGuard aEventGuard( m_aEventSafety );
    if ( m_nEventId )
        Application::RemoveUserEvent( m_nEventId );
    m_nEventId = 0;
}

IMPL_LINK(OAsyncronousLink, OnAsyncCall, void*, _pArg)
{
    {
        ::osl::MutexGuard aDestructionGuard( m_aDestructionSafety );
        {
            ::osl::MutexGuard aEventGuard( m_aEventSafety );
            if (!m_nEventId)
                
                
                return 0;
            m_nEventId = 0;
        }
    }
    if (m_aHandler.IsSet())
        return m_aHandler.Call(_pArg);

    return 0L;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
