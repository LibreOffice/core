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

#ifndef INCLUDED_VCL_INC_SALSESSION_HXX
#define INCLUDED_VCL_INC_SALSESSION_HXX

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

typedef void(*SessionProc)(void *pData, SalSessionEvent *pEvent);

class VCL_PLUGIN_PUBLIC SalSession
{
    SessionProc         m_aProc;
    void *              m_pProcData;
public:
    SalSession()
        : m_aProc(0)
        , m_pProcData(NULL)
    {
    }
    virtual ~SalSession();

    void SetCallback( SessionProc aCallback, void * pCallbackData )
    {
        m_aProc = aCallback;
        m_pProcData = pCallbackData;
    }
    void CallCallback( SalSessionEvent* pEvent )
    {
        if( m_aProc )
            m_aProc( m_pProcData, pEvent );
    }

    // query the session manager for a user interaction slot
    virtual void queryInteraction() = 0;
    // signal the session manager that we're done with user interaction
    virtual void interactionDone() = 0;
    // signal that we're done saving
    virtual void saveDone() = 0;
    // try to cancel the shutdown in progress
    virtual bool cancelShutdown() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
