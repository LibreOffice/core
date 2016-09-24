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

#include <salframe.hxx>
#include <unx/gendisp.hxx>
#include <unx/geninst.h>

SalGenericDisplay::SalGenericDisplay()
 : m_aEventGuard()
{
    m_pCapture = nullptr;
}

SalGenericDisplay::~SalGenericDisplay()
{
}

void SalGenericDisplay::registerFrame( SalFrame* pFrame )
{
    m_aFrames.push_front( pFrame );
}

void SalGenericDisplay::deregisterFrame( SalFrame* pFrame )
{
    {
        osl::MutexGuard g( m_aEventGuard );
        std::list< SalUserEvent >::iterator it = m_aUserEvents.begin();
        while ( it != m_aUserEvents.end() )
        {
            if( it->m_pFrame == pFrame )
            {
                if (it->m_nEvent == SalEvent::UserEvent) {
                    delete static_cast<ImplSVEvent *>(it->m_pData);
                }
                it = m_aUserEvents.erase( it );
            }
            else
                ++it;
        }
    }

    m_aFrames.remove( pFrame );
}

void SalGenericDisplay::emitDisplayChanged()
{
    if( !m_aFrames.empty() )
        m_aFrames.front()->CallCallback( SalEvent::DisplayChanged, nullptr );
}

bool SalGenericDisplay::DispatchInternalEvent()
{
    void* pData = nullptr;
    SalFrame* pFrame = nullptr;
    SalEvent nEvent = SalEvent::NONE;

    {
        osl::MutexGuard g( m_aEventGuard );
        if( !m_aUserEvents.empty() )
        {
            pFrame  = m_aUserEvents.front().m_pFrame;
            pData   = m_aUserEvents.front().m_pData;
            nEvent  = m_aUserEvents.front().m_nEvent;

            m_aUserEvents.pop_front();
        }
    }

    if( pFrame )
        pFrame->CallCallback( nEvent, pData );

    return pFrame != nullptr;
}

void SalGenericDisplay::SendInternalEvent( SalFrame* pFrame, void* pData, SalEvent nEvent )
{
    osl::MutexGuard g( m_aEventGuard );

    m_aUserEvents.push_back( SalUserEvent( pFrame, pData, nEvent ) );

    PostUserEvent(); // wakeup the concrete mainloop
}

void SalGenericDisplay::CancelInternalEvent( SalFrame* pFrame, void* pData, SalEvent nEvent )
{
    osl::MutexGuard g( m_aEventGuard );
    if( ! m_aUserEvents.empty() )
    {
        std::list< SalUserEvent >::iterator it = m_aUserEvents.begin();
        while (it != m_aUserEvents.end())
        {
            if( it->m_pFrame    == pFrame   &&
                it->m_pData     == pData    &&
                it->m_nEvent    == nEvent )
            {
                it = m_aUserEvents.erase( it );
            }
            else
                ++it;
        }
    }
}

bool SalGenericDisplay::HasUserEvents() const
{
    osl::MutexGuard g( m_aEventGuard );
    return !m_aUserEvents.empty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
