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
#include "precompiled_chart2.hxx"

#include "TimerTriggeredControllerLock.hxx"

// header for define EDIT_UPDATEDATA_TIMEOUT
#include <vcl/edit.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;

TimerTriggeredControllerLock::TimerTriggeredControllerLock( const uno::Reference< frame::XModel >& xModel )
    : m_xModel( xModel )
    , m_apControllerLockGuard()
    , m_aTimer()
{
    m_aTimer.SetTimeout( 4*EDIT_UPDATEDATA_TIMEOUT );
    m_aTimer.SetTimeoutHdl( LINK( this, TimerTriggeredControllerLock, TimerTimeout ) );
}
TimerTriggeredControllerLock::~TimerTriggeredControllerLock()
{
    m_aTimer.Stop();
}

void TimerTriggeredControllerLock::startTimer()
{
    if(!m_apControllerLockGuard.get())
        m_apControllerLockGuard = std::auto_ptr< ControllerLockGuard >( new  ControllerLockGuard(m_xModel) );
    m_aTimer.Start();
}
IMPL_LINK( TimerTriggeredControllerLock, TimerTimeout, void*, EMPTYARG )
{
    m_apControllerLockGuard.reset();
    return 0;
}

//.............................................................................
} //namespace chart
//.............................................................................
