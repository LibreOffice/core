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

class WinSalTimer final : public SalTimer, protected VersionedEvent
{
    // for access to Impl* functions
    friend LRESULT CALLBACK SalComWndProc( HWND, UINT nMsg, WPARAM wParam, LPARAM lParam, bool& rDef );
    // for access to GetNextVersionedEvent
    friend void CALLBACK SalTimerProc( PVOID data, BOOLEAN );
    // for access to ImplHandleElapsedTimer
    friend bool ImplSalYield( bool bWait, bool bHandleAllCurrentEvents );

    /**
     * Identifier for our SetTimer based timer
     */
    static constexpr UINT_PTR m_aWmTimerId = 0xdeadbeef;

    HANDLE       m_nTimerId;          ///< Windows timer id
    bool         m_bDirectTimeout;    ///< timeout can be processed directly
    bool         m_bForceRealTimer;   ///< enforce using a real timer for 0ms
    bool         m_bSetTimerRunning;  ///< true, if a SetTimer is running

    void ImplStart( sal_uInt64 nMS );
    void ImplStop();
    void ImplHandleTimerEvent( WPARAM aWPARAM );
    void ImplHandleElapsedTimer();
    void ImplHandle_WM_TIMER( WPARAM aWPARAM );

public:
    WinSalTimer();
    virtual ~WinSalTimer() override;

    virtual void Start(sal_uInt64 nMS) override;
    virtual void Stop() override;

    inline bool IsDirectTimeout() const;
    inline bool HasTimerElapsed() const;

    /**
     * Enforces the usage of a real timer instead of the message queue
     *
     * Needed for Window resize processing, as this starts a modal event loop.
     */
    void SetForceRealTimer( bool bVal );
    inline bool GetForceRealTimer() const;
};

inline bool WinSalTimer::IsDirectTimeout() const
{
    return m_bDirectTimeout;
}

inline bool WinSalTimer::HasTimerElapsed() const
{
    return m_bDirectTimeout || ExistsValidEvent();
}

inline bool WinSalTimer::GetForceRealTimer() const
{
    return m_bForceRealTimer;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
