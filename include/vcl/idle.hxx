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

#include <vcl/timer.hxx>

/**
 * An idle is a timer to be scheduled immediately.
 *
 * It's - more or less - just a convenience class.
 */
class VCL_DLLPUBLIC Idle : public Timer
{
private:
    // Delete all timeout specific functions, we don't want in an Idle
    void          SetTimeout( sal_uInt64 nTimeoutMs ) = delete;
    sal_uInt64    GetTimeout() const = delete;

protected:
    virtual sal_uInt64 UpdateMinPeriod(
        sal_uInt64 nMinPeriod, sal_uInt64 nTimeNow ) const override;

    Idle( bool bAuto, const sal_Char *pDebugName = nullptr );

public:
    Idle( const sal_Char *pDebugName = nullptr );

    virtual void  Start() override;
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
};


#endif // INCLUDED_VCL_IDLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
