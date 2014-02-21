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

#ifndef INCLUDED_VCL_INC_GENERIC_GENDISP_HXX
#define INCLUDED_VCL_INC_GENERIC_GENDISP_HXX

#include <sal/types.h>
#include <osl/mutex.h>
#include <osl/conditn.hxx>
#include <salwtype.hxx>
#include <vcl/dllapi.h>
#include <list>
#include <vector>

class SalFrame;
class VCL_DLLPUBLIC SalGenericDisplay
{
    oslMutex m_aEventGuard;
    struct SalUserEvent
    {
        SalFrame*  m_pFrame;
        void*      m_pData;
        sal_uInt16 m_nEvent;

        SalUserEvent( SalFrame* pFrame, void* pData,
                      sal_uInt16 nEvent )
                : m_pFrame( pFrame ),
                  m_pData( pData ),
                  m_nEvent( nEvent )
        {}
    };
    std::list< SalUserEvent > m_aUserEvents;
protected:
    SalFrame* m_pCapture;
    std::list<SalFrame*> m_aFrames;
public:
                 SalGenericDisplay();
    virtual      ~SalGenericDisplay();

    inline void  EventGuardAcquire() { osl_acquireMutex( m_aEventGuard ); }
    inline void  EventGuardRelease() { osl_releaseMutex( m_aEventGuard ); }

    virtual void registerFrame( SalFrame* pFrame );
    virtual void deregisterFrame( SalFrame* pFrame );
            void emitDisplayChanged();

    // Event handling
    virtual void PostUserEvent() = 0;

    virtual void SendInternalEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent = SALEVENT_USEREVENT );
    virtual void CancelInternalEvent( SalFrame* pFrame, void* pData, sal_uInt16 nEvent );
    virtual bool DispatchInternalEvent();
    bool         HasUserEvents() const;

    bool     MouseCaptured( const SalFrame *pFrameData ) const
                        { return m_pCapture == pFrameData; }
    SalFrame*    GetCaptureFrame() const
                        { return m_pCapture; }
};


#endif // INCLUDED_VCL_INC_GENERIC_GENDISP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
