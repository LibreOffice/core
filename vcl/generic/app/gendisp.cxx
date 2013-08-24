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
#include <generic/gendisp.hxx>
#include <generic/geninst.h>


SalGenericDisplay::SalGenericDisplay()
{
    m_pCapture = NULL;
    m_aEventGuard = osl_createMutex();
}

SalGenericDisplay::~SalGenericDisplay()
{
    if (m_aEventGuard)
        osl_destroyMutex( m_aEventGuard );
    m_aEventGuard = NULL;
}

void SalGenericDisplay::registerFrame( SalFrame* pFrame )
{
    m_aFrames.push_front( pFrame );
}

void SalGenericDisplay::deregisterFrame( SalFrame* pFrame )
{
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        std::list< SalUserEvent >::iterator it = m_aUserEvents.begin();
        while ( it != m_aUserEvents.end() )
        {
            if( it->m_pFrame == pFrame )
                it = m_aUserEvents.erase( it );
            else
                ++it;
        }
        osl_releaseMutex( m_aEventGuard );
    }
    else
        OSL_FAIL( "SalGenericDisplay::deregisterFrame !acquireMutex\n" );

    m_aFrames.remove( pFrame );
}

void SalGenericDisplay::emitDisplayChanged()
{
    if( !m_aFrames.empty() )
        m_aFrames.front()->CallCallback( SALEVENT_DISPLAYCHANGED, 0 );
}

bool SalGenericDisplay::DispatchInternalEvent()
{
    void* pData = NULL;
    SalFrame* pFrame = NULL;
    sal_uInt16 nEvent = 0;

    if( osl_acquireMutex( m_aEventGuard ) )
    {
        if( m_aUserEvents.begin() != m_aUserEvents.end() )
        {
            pFrame	= m_aUserEvents.front().m_pFrame;
            pData	= m_aUserEvents.front().m_pData;
            nEvent	= m_aUserEvents.front().m_nEvent;

            m_aUserEvents.pop_front();
        }
        osl_releaseMutex( m_aEventGuard );
    }
    else
        OSL_FAIL( "SalGenericDisplay::Yield !acquireMutex\n" );

    if( pFrame )
        pFrame->CallCallback( nEvent, pData );

    return pFrame != NULL;
}

void SalGenericDisplay::SendInternalEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent )
{
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        m_aUserEvents.push_back( SalUserEvent( pFrame, pData, nEvent ) );

        PostUserEvent(); // wakeup the concrete mainloop

        osl_releaseMutex( m_aEventGuard );
    }
    else
        OSL_FAIL( "SalGenericDisplay::SendInternalEvent !acquireMutex\n" );
}

void SalGenericDisplay::CancelInternalEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent )
{
    if( osl_acquireMutex( m_aEventGuard ) )
    {
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

        osl_releaseMutex( m_aEventGuard );
    }
    else
        OSL_FAIL( "SalGenericDisplay::CancelInternalEvent !acquireMutex\n" );
}

bool SalGenericDisplay::HasUserEvents() const
{
    bool bRet = false;
    if( osl_acquireMutex( m_aEventGuard ) )
    {
        if( m_aUserEvents.begin() != m_aUserEvents.end() )
            bRet = true;
        osl_releaseMutex( m_aEventGuard );
    }
    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
