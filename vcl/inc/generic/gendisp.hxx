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

#ifndef _VCL_GEN_DISP_HXX
#define _VCL_GEN_DISP_HXX

#include <sal/types.h>
#include <osl/mutex.h>
#include <osl/conditn.hxx>
#include <salwtype.hxx>
#include <vcl/dllapi.h>
#include <tools/gen.hxx>
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

    sal_Bool     MouseCaptured( const SalFrame *pFrameData ) const
                        { return m_pCapture == pFrameData; }
    SalFrame*    GetCaptureFrame() const
                        { return m_pCapture; }
};


#endif // _VCL_GEN_DISP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
