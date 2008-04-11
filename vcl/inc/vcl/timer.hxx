/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: timer.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_TIMER_HXX
#define _SV_TIMER_HXX

#include <tools/link.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

struct ImplTimerData;
struct ImplSVData;

// ---------
// - Timer -
// ---------

class VCL_DLLPUBLIC Timer
{
protected:
    ImplTimerData*  mpTimerData;
    ULONG           mnTimeout;
    BOOL            mbActive;
    BOOL            mbAuto;
    Link            maTimeoutHdl;

public:
                    Timer();
                    Timer( const Timer& rTimer );
    virtual         ~Timer();

    virtual void    Timeout();

    void            Start();
    void            Stop();

    void            SetTimeout( ULONG nTimeout );
    ULONG           GetTimeout() const { return mnTimeout; }
    BOOL            IsActive() const { return mbActive; }

    void            SetTimeoutHdl( const Link& rLink ) { maTimeoutHdl = rLink; }
    const Link&     GetTimeoutHdl() const { return maTimeoutHdl; }

    Timer&          operator=( const Timer& rTimer );

//    #ifdef _SOLAR__PRIVATE
    static void ImplDeInitTimer();
    static void ImplTimerCallbackProc();
//    #endif
};

// -------------
// - AutoTimer -
// -------------

class VCL_DLLPUBLIC AutoTimer : public Timer
{
public:
                    AutoTimer();
                    AutoTimer( const AutoTimer& rTimer );

    AutoTimer&      operator=( const AutoTimer& rTimer );
};

#endif // _SV_TIMER_HXX
