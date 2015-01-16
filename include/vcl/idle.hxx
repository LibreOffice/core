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

#ifndef INCLUDED_VCL_IDLE_HXX
#define INCLUDED_VCL_IDLE_HXX

#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>

struct ImplIdleData;
struct ImplSVData;

enum class IdlePriority {
    VCL_IDLE_PRIORITY_STARVATIONPROTECTION  = -1, // Do not use this for normal prioritizing
    VCL_IDLE_PRIORITY_HIGHEST               = 0, // -> 0ms
    VCL_IDLE_PRIORITY_HIGH                  = 1,    // -> 1ms
    VCL_IDLE_PRIORITY_DEFAULT               = 1,    // -> 1ms
    VCL_IDLE_PRIORITY_REPAINT               = 2, // -> 30ms
    VCL_IDLE_PRIORITY_RESIZE                = 3,  // -> 50ms
    VCL_IDLE_PRIORITY_MEDIUM                = 3,  // -> 50ms
    VCL_IDLE_PRIORITY_LOW                   = 5,     // -> 100ms
    VCL_IDLE_PRIORITY_LOWER                 = 6,   // -> 200ms
    VCL_IDLE_PRIORITY_LOWEST                = 7   // -> 400ms
};


// To port from Timer -> Idle switch class name,
// s/Timeout/DoIdle/ etc. and select priority
class VCL_DLLPUBLIC Idle
{
protected:
    ImplIdleData*   mpIdleData;
    IdlePriority    mePriority;
    IdlePriority    meDefaultPriority;
    bool            mbActive;
    Link            maIdleHdl;

    friend struct ImplIdleData;

public:
    Idle();
    Idle( const Idle& rIdle );
    virtual ~Idle();

    void SetPriority( IdlePriority ePriority );
    IdlePriority GetPriority() const { return mePriority; }
    IdlePriority GetDefaultPriority() const { return meDefaultPriority; }

    /// Make it possible to associate a callback with this idle handler
    /// of course, you can also sub-class and override 'DoIdle'
    void            SetIdleHdl( const Link& rLink ) { maIdleHdl = rLink; }
    const Link&     GetIdleHdl() const              { return maIdleHdl; }

    virtual void    DoIdle();

    void            Start();
    void            Stop();

    bool            IsActive() const { return mbActive; }

    Idle&          operator=( const Idle& rIdle );
    static void ImplDeInitIdle();
    static void ImplIdleCallbackProc();

    /// Process all pending idle tasks ahead of time in priority order.
    static void ProcessAllIdleHandlers();
};

#endif // INCLUDED_VCL_IDLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
