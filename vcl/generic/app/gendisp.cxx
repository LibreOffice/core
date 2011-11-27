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


#include <salframe.hxx>
#include <generic/gendisp.hxx>
#include <generic/geninst.h>

using ::rtl::OUString;

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
            std::list< SalUserEvent >::iterator it, next;
            next = m_aUserEvents.begin();
            do
            {
                it = next++;
                if( it->m_pFrame    == pFrame   &&
                    it->m_pData     == pData    &&
                    it->m_nEvent    == nEvent )
                {
                    m_aUserEvents.erase( it );
                }
            } while( next != m_aUserEvents.end() );
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
