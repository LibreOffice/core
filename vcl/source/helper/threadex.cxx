/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: threadex.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:50:25 $
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

#define THREADEX_IMPLEMENTATION
#include <threadex.hxx>
#include <svapp.hxx>

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
    m_aFinish = osl_createCondition();
}

SolarThreadExecutor::~SolarThreadExecutor()
{
    osl_destroyCondition( m_aFinish );
}

IMPL_LINK( SolarThreadExecutor, worker, void*, EMPTYARG )
{
    if ( !m_bTimeout )
    {
        m_nReturn = doIt();
        osl_setCondition( m_aFinish );
    }
    return m_nReturn;
}

long SolarThreadExecutor::impl_execute( const TimeValue* _pTimeout )
{
    if( ::vos::OThread::getCurrentIdentifier() == Application::GetMainThreadIdentifier() )
    {
        m_nReturn = doIt();
        osl_setCondition( m_aFinish );
    }
    else
    {
        osl_resetCondition( m_aFinish );
        ULONG nSolarMutexCount = Application::ReleaseSolarMutex();
        Application::PostUserEvent( LINK( this, SolarThreadExecutor, worker ) );
        if ( osl_cond_result_timeout == osl_waitCondition( m_aFinish, _pTimeout ) )
            m_bTimeout = true;
        if( nSolarMutexCount )
            Application::AcquireSolarMutex( nSolarMutexCount );
    }
    return m_nReturn;
}
