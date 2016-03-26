/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SAL_INC_INTERNAL_SIGNALSHARED_H
#define INCLUDED_SAL_INC_INTERNAL_SIGNALSHARED_H

#include <osl/signal.h>
#include <osl/mutex.h>

struct oslSignalHandlerImpl
{
    oslSignalHandlerFunction    Handler;
    void*                       pData;
    oslSignalHandlerImpl*       pNext;
};

typedef void (*ErrorReportingChangedHandler)(bool);

extern bool                     bErrorReportingEnabled;
extern bool                     bInitSignal;

void setErrorReportingChangedHandler(ErrorReportingChangedHandler handler);

oslSignalAction callSignalHandler(oslSignalInfo* pInfo);

// platform-specific functions that need to be implemented
bool onInitSignal();
bool onDeInitSignal();

#endif

