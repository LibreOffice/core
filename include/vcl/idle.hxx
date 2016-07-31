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
#include <vcl/scheduler.hxx>

class VCL_DLLPUBLIC Idle : public Scheduler
{
protected:
    Link<Idle *, void> maIdleHdl;          // Callback Link
    bool               mbAuto;

    virtual void SetDeletionFlags() override;

    virtual bool ReadyForSchedule( const sal_uInt64 nTime, const bool bIdle ) const override;
    virtual void UpdateMinPeriod( const sal_uInt64 nTime, sal_uInt64 &nMinPeriod ) const override;

public:
    Idle( const sal_Char *pDebugName = nullptr );
    Idle( const Idle& rIdle );

    virtual void    Start() override;

    /// Make it possible to associate a callback with this idle handler
    /// of course, you can also sub-class and override 'Invoke'
    void            SetIdleHdl( const Link<Idle *, void>& rLink ) { maIdleHdl = rLink; }
    const Link<Idle *, void>& GetIdleHdl() const { return maIdleHdl; }
    virtual void Invoke() override;
    Idle&           operator=( const Idle& rIdle );
};

/**
 * An auto-idle is long running task processing small chunks of data, which
 * is re-scheduled multiple times.
 *
 * Remember to stop the Idle when finished, as it would otherwise busy loop the CPU!
 *
 * It probably makes sense to re-implement ReadyForSchedule and UpdateMinPeriod,
 * in case there is a quick check and it can otherwise sleep.
 */
class VCL_DLLPUBLIC AutoIdle : public Idle
{
public:
    AutoIdle( const sal_Char *pDebugName = nullptr );
    AutoIdle( const AutoIdle& rIdle );
    AutoIdle& operator=( const AutoIdle& rIdle );
};

#endif // INCLUDED_VCL_IDLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
