/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salsession.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:01:41 $
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

#ifndef _VCL_SALSESSION_HXX
#define _VCL_SALSESSION_HXX

#ifndef _VCL_DLLAPI_H
#include "vcl/dllapi.h"
#endif

enum SalSessionEventType
{
    Interaction,
    SaveRequest,
    ShutdownCancel
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

typedef void(*SessionProc)( SalSessionEvent *pEvent);

class VCL_DLLPUBLIC SalSession
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
