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
#include <vcl/task.hxx>

class VCL_DLLPUBLIC Timer : public Task
{
    Link<Timer *, void> maInvokeHandler;   ///< Callback Link
    sal_uInt64          mnTimeout;
    const bool          mbAuto;

protected:
    virtual void SetDeletionFlags() override;
    virtual sal_uInt64 UpdateMinPeriod( sal_uInt64 nTimeNow ) const override;

    Timer( bool bAuto, const char *pDebugName );

public:
    Timer( const char *pDebugName = nullptr );
    Timer( const Timer& rTimer );
    virtual ~Timer() override;
    Timer& operator=( const Timer& rTimer );

    /**
     * Calls the maInvokeHandler with the parameter this.
     */
    virtual void    Invoke() override;
    /**
     * Calls the maInvokeHandler with the parameter.
     *
     * Convenience Invoke function, mainly used to call with nullptr.
     *
     * @param arg parameter for the Link::Call function
     */
    void            Invoke( Timer *arg );
    void            SetInvokeHandler( const Link<Timer *, void>& rLink ) { maInvokeHandler = rLink; }
    void            ClearInvokeHandler() { SetInvokeHandler( Link<Timer *, void>() ); }

    void            SetTimeout( sal_uInt64 nTimeoutMs );
    sal_uInt64      GetTimeout() const { return mnTimeout; }
    /**
     * Activates the timer task
     *
     * If the timer is already active, it's reset!
     * Check with Task::IsActive() to prevent reset.
     */
    virtual void    Start() override;
};

/// An auto-timer is a multi-shot timer re-emitting itself at
/// interval until destroyed or stopped.
class VCL_DLLPUBLIC AutoTimer : public Timer
{
public:
    AutoTimer( const char *pDebugName = nullptr );
};

/// Value suitable as a timeout user input into an EditBox to an expensive update
#define EDIT_UPDATEDATA_TIMEOUT     350

#endif // INCLUDED_VCL_TIMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
