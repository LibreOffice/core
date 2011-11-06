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



#include "precompiled_svx.hxx"

#include "delayedevent.hxx"

#include <osl/diagnose.h>
#include <vcl/svapp.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= DelayedEvent
    //====================================================================
    //--------------------------------------------------------------------
    void DelayedEvent::Call( void* _pArg )
    {
        CancelPendingCall();
        OSL_POSTCOND( m_nEventId == 0, "DelayedEvent::Call: CancelPendingCall did not work!" );

        m_nEventId = Application::PostUserEvent( LINK( this, DelayedEvent, OnCall ), _pArg );
    }

    //--------------------------------------------------------------------
    void DelayedEvent::CancelPendingCall()
    {
        if ( m_nEventId )
            Application::RemoveUserEvent( m_nEventId );
        m_nEventId = 0;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( DelayedEvent, OnCall, void*, _pArg )
    {
        m_nEventId = 0;
        return m_aHandler.Call( _pArg );
    }

//........................................................................
} // namespace svxform
//........................................................................
