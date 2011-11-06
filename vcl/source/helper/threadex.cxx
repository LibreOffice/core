/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
