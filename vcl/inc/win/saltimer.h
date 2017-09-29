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

#ifndef INCLUDED_VCL_INC_WIN_SALTIMER_H
#define INCLUDED_VCL_INC_WIN_SALTIMER_H

#include <saltimer.hxx>

class WinSalTimer : public VersionedSalTimer
{
    HANDLE       m_nTimerId;          ///< Windows timer id
    bool         m_bPollForMessage;   ///< Run yield until a message is caught (most likely the 0ms timer)

public:
    WinSalTimer();
    virtual ~WinSalTimer() override;

    virtual void Start(sal_uIntPtr nMS) override;
    virtual void Stop() override;

    inline bool IsValidWPARAM( WPARAM wParam ) const;

    inline bool PollForMessage() const;

    // The Impl functions are just public to be called from the static
    // SalComWndProc on main thread redirect! Otherwise they would be private.
    // They must be called from the main application thread only!

    void ImplStart( sal_uIntPtr nMS );
    void ImplStop();
    void ImplEmitTimerCallback();
};

inline bool WinSalTimer::IsValidWPARAM( WPARAM aWPARAM ) const
{
    return GetEventVersion() == static_cast<sal_Int32>( aWPARAM );
}

inline bool WinSalTimer::PollForMessage() const
{
    return m_bPollForMessage;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
