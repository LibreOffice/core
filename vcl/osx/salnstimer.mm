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

#include <sal/config.h>

#include "osx/saltimer.h"
#include "osx/salnstimer.h"
#include "osx/salinst.h"
#include "osx/saldata.hxx"
#include "svdata.hxx"

@implementation TimerCallbackCaller
-(void)timerElapsed:(NSTimer*)pTimer
{
    (void)pTimer;
    if( AquaSalTimer::bDispatchTimer )
    {
        SolarMutexGuard aGuard;
        ImplSVData* pSVData = ImplGetSVData();
        if( pSVData->mpSalTimer )
        {
            pSVData->mpSalTimer->CallCallback();

            // NSTimer does not end nextEventMatchingMask of NSApplication
            // so we need to wakeup a waiting Yield to inform it something happened
            GetSalData()->mpFirstInstance->wakeupYield();
        }
    }
}
@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
