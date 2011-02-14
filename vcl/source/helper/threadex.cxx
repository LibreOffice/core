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
#include "precompiled_vcl.hxx"

#define THREADEX_IMPLEMENTATION
#include <vcl/threadex.hxx>
#include <vcl/svapp.hxx>

using namespace vcl;

ThreadExecutor::ThreadExecutor()
{
    m_aFinish = osl_createCondition();
    m_aThread = NULL;
}

ThreadExecutor::~ThreadExecutor()
{
    osl_destroyCondition( m_aFinish );
    if( m_aThread )
        osl_destroyThread( m_aThread );
}

extern "C"
{
    static void call_worker( void* pInstance )
    {
        ThreadExecutor::worker( pInstance );
    }
}

void ThreadExecutor::worker( void* pInstance )
{
    ThreadExecutor* pThis = ((ThreadExecutor*)pInstance);
    pThis->m_nReturn = pThis->doIt();
    osl_setCondition( pThis->m_aFinish );
}

long ThreadExecutor::execute()
{
    osl_resetCondition( m_aFinish );
    if( m_aThread )
        osl_destroyThread( m_aThread ), m_aThread = NULL;
    m_aThread = osl_createThread( call_worker, this );
    while( ! osl_checkCondition( m_aFinish ) )
        Application::Reschedule();
    return m_nReturn;
}


SolarThreadExecutor::SolarThreadExecutor()
    :m_nReturn( 0 )
    ,m_bTimeout( false )
{
    m_aStart = osl_createCondition();
    m_aFinish = osl_createCondition();
}

SolarThreadExecutor::~SolarThreadExecutor()
{
    osl_destroyCondition( m_aStart );
    osl_destroyCondition( m_aFinish );
}

IMPL_LINK( SolarThreadExecutor, worker, void*, EMPTYARG )
{
    if ( !m_bTimeout )
    {
        osl_setCondition( m_aStart );
        m_nReturn = doIt();
        osl_setCondition( m_aFinish );
    }
    return m_nReturn;
}

long SolarThreadExecutor::impl_execute( const TimeValue* _pTimeout )
{
    if( ::vos::OThread::getCurrentIdentifier() == Application::GetMainThreadIdentifier() )
    {
        osl_setCondition( m_aStart );
        m_nReturn = doIt();
        osl_setCondition( m_aFinish );
    }
    else
    {
        osl_resetCondition( m_aStart );
        osl_resetCondition( m_aFinish );
        sal_uLong nSolarMutexCount = Application::ReleaseSolarMutex();
        sal_uLong nEvent = Application::PostUserEvent( LINK( this, SolarThreadExecutor, worker ) );
        if ( osl_cond_result_timeout == osl_waitCondition( m_aStart, _pTimeout ) )
        {
            m_bTimeout = true;
            Application::RemoveUserEvent( nEvent );
        }
        else
            osl_waitCondition( m_aFinish, NULL );
        if( nSolarMutexCount )
            Application::AcquireSolarMutex( nSolarMutexCount );
    }
    return m_nReturn;
}
