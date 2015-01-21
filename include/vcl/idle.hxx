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
    VCL_IDLE_PRIORITY_HIGHEST,
    VCL_IDLE_PRIORITY_HIGH,
    VCL_IDLE_PRIORITY_REPAINT,
    VCL_IDLE_PRIORITY_RESIZE,
    VCL_IDLE_PRIORITY_MEDIUM,
    VCL_IDLE_PRIORITY_LOW,
    VCL_IDLE_PRIORITY_LOWER,
    VCL_IDLE_PRIORITY_LOWEST
};

inline sal_Int32 convertToInt( IdlePriority ePriority )
{
    switch (ePriority)
    {
        case IdlePriority::VCL_IDLE_PRIORITY_HIGHEST:
            return 0;
        case IdlePriority::VCL_IDLE_PRIORITY_HIGH:
            return 1;
        case IdlePriority::VCL_IDLE_PRIORITY_REPAINT:
            return 2;
        case IdlePriority::VCL_IDLE_PRIORITY_RESIZE:
            return 3;
        case IdlePriority::VCL_IDLE_PRIORITY_MEDIUM:
            return 3;
        case IdlePriority::VCL_IDLE_PRIORITY_LOW:
            return 4;
        case IdlePriority::VCL_IDLE_PRIORITY_LOWER:
            return 5;
        case IdlePriority::VCL_IDLE_PRIORITY_LOWEST:
            return 6;
    }

    return 42; // Should not happen
}

class VCL_DLLPUBLIC Idle
{
protected:
    ImplIdleData*   mpIdleData;         // Pointer to element in idle list
    sal_Int32       miPriority;         // Idle priority ( maybe divergent to default)
    IdlePriority    meDefaultPriority;  // Default idle priority
    bool            mbActive;           // Currently in the scheduler
    Link            maIdleHdl;          // Callback Link

    friend struct ImplIdleData;

public:
    Idle();
    Idle( const Idle& rIdle );
    virtual ~Idle();

    void SetPriority( IdlePriority ePriority );
    void SetSchedulingPriority( sal_Int32 iPriority );
    sal_Int32    GetPriority() const { return miPriority; }
    IdlePriority GetDefaultPriority() const { return meDefaultPriority; }

    /// Make it possible to associate a callback with this idle handler
    /// of course, you can also sub-class and override 'DoIdle'
    void            SetIdleHdl( const Link& rLink ) { maIdleHdl = rLink; }
    const Link&     GetIdleHdl() const { return maIdleHdl; }

    // Call idle handler
    virtual void    DoIdle();

    void            Start();
    void            Stop();

    bool            IsActive() const { return mbActive; }

    Idle&          operator=( const Idle& rIdle );
    static void ImplDeInitIdle();

    /// Process all pending idle tasks ahead of time in priority order.
    static void ProcessAllIdleHandlers();
};

#endif // INCLUDED_VCL_IDLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
