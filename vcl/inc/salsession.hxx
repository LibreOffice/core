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



#ifndef _VCL_SALSESSION_HXX
#define _VCL_SALSESSION_HXX

#include "vcl/dllapi.h"

enum SalSessionEventType
{
    Interaction,
    SaveRequest,
    ShutdownCancel,
    Quit
};

struct SalSessionEvent
{
    SalSessionEventType         m_eType;

    SalSessionEvent( SalSessionEventType eType )
            : m_eType( eType )
    {}
};

struct SalSessionInteractionEvent : public SalSessionEvent
{
    bool                        m_bInteractionGranted;

    SalSessionInteractionEvent( bool bGranted )
            : SalSessionEvent( Interaction ),
              m_bInteractionGranted( bGranted )
    {}
};

struct SalSessionSaveRequestEvent : public SalSessionEvent
{
    bool                        m_bShutdown;
    bool                        m_bCancelable;

    SalSessionSaveRequestEvent( bool bShutdown, bool bCancelable )
            : SalSessionEvent( SaveRequest ),
              m_bShutdown( bShutdown ),
              m_bCancelable( bCancelable )
    {}
};

struct SalSessionShutdownCancelEvent : public SalSessionEvent
{
    SalSessionShutdownCancelEvent()
            : SalSessionEvent( ShutdownCancel )
    {}
};

struct SalSessionQuitEvent : public SalSessionEvent
{
    SalSessionQuitEvent()
            : SalSessionEvent( Quit )
    {}
};

typedef void(*SessionProc)( SalSessionEvent *pEvent);

class VCL_PLUGIN_PUBLIC SalSession
{
    SessionProc         m_aProc;
public:
    SalSession()
            : m_aProc( 0 )
    {}
    virtual ~SalSession();

    void SetCallback( SessionProc aCallback )
    {
        m_aProc = aCallback;
    }
    void CallCallback( SalSessionEvent* pEvent )
    {
        if( m_aProc )
            m_aProc( pEvent );
    }

    // query the session manager for a user interaction slot
    virtual void queryInteraction() = 0;
    // signal the session manager that we're done with user interaction
    virtual void interactionDone() = 0;
    // signal that we're done saving
    virtual void saveDone() = 0;
    // try to cancel the sutdown in progress
    virtual bool cancelShutdown() = 0;
};

#endif
