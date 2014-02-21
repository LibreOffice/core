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

#ifndef INCLUDED_VCL_TIMER_HXX
#define INCLUDED_VCL_TIMER_HXX

#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>

struct ImplTimerData;
struct ImplSVData;

class VCL_DLLPUBLIC Timer
{
protected:
    ImplTimerData*  mpTimerData;
    sal_uLong       mnTimeout;
    bool            mbActive;
    bool            mbAuto;
    Link            maTimeoutHdl;

public:
                    Timer();
                    Timer( const Timer& rTimer );
    virtual         ~Timer();

    virtual void    Timeout();

    void            Start();
    void            Stop();

    /// set the timeout in milliseconds
    void            SetTimeout( sal_uLong nTimeoutMs );
    sal_uLong       GetTimeout() const { return mnTimeout; }
    bool        IsActive() const { return mbActive ? sal_True : sal_False; }

    void            SetTimeoutHdl( const Link& rLink ) { maTimeoutHdl = rLink; }
    const Link&     GetTimeoutHdl() const { return maTimeoutHdl; }

    Timer&          operator=( const Timer& rTimer );

    static void ImplDeInitTimer();
    static void ImplTimerCallbackProc();
};


class VCL_DLLPUBLIC AutoTimer : public Timer
{
public:
                    AutoTimer();
                    AutoTimer( const AutoTimer& rTimer );

    AutoTimer&      operator=( const AutoTimer& rTimer );
};

#endif // INCLUDED_VCL_TIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
