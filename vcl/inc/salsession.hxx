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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
