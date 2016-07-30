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

#ifndef INCLUDED_VCL_INC_SALTIMER_HXX
#define INCLUDED_VCL_INC_SALTIMER_HXX

#include <sal/config.h>

#include <vcl/dllapi.h>

#include <salwtype.hxx>

/*
 * note: there will be only a single instance of SalTimer
 * SalTimer originally had only static methods, but
 * this needed to be virtualized for the sal plugin migration
 */

class VCL_PLUGIN_PUBLIC SalTimer
{
    SALTIMERPROC        m_pProc;
public:
    SalTimer() : m_pProc( nullptr ) {}
    virtual ~SalTimer();

    // AutoRepeat and Restart
    virtual void            Start( sal_uLong nMS ) = 0;
    virtual void            Stop() = 0;

    // Callbacks (indepen in \sv\source\app\timer.cxx)
    void            SetCallback( SALTIMERPROC pProc )
    {
        m_pProc = pProc;
    }

    void            CallCallback( bool idle )
    {
        if( m_pProc )
            m_pProc( idle );
    }
};

class Scheduler;

// Internal scheduler record holding intrusive linked list pieces
struct ImplSchedulerData
{
    ImplSchedulerData*  mpNext;        // Pointer to the next element in list
    Scheduler*          mpScheduler;   // Pointer to VCL Scheduler instance
    bool                mbInScheduler; // Scheduler currently processed?
    sal_uInt64          mnUpdateTime;  // Last Update Time

    void Invoke();

    const char *GetDebugName() const;
    static ImplSchedulerData *GetMostImportantTask( bool bTimer );
};

#endif // INCLUDED_VCL_INC_SALTIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
