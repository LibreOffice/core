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

#include <stdlib.h>

#include <signalshared.hxx>

#include <mutex>

namespace
{
oslSignalHandlerImpl* SignalList;
bool bInitSignal = false;

std::mutex& getSignalMutex()
{
    static std::mutex aMutex;
    return aMutex;
}
}

oslSignalAction callSignalHandler(oslSignalInfo* pInfo)
{
    oslSignalHandlerImpl* pHandler = SignalList;
    oslSignalAction Action = osl_Signal_ActCallNextHdl;

    while (pHandler)
    {
        if ((Action = pHandler->Handler(pHandler->pData, pInfo)) != osl_Signal_ActCallNextHdl)
            break;

        pHandler = pHandler->pNext;
    }

    return Action;
}

oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction handler, void* pData)
{
    if (!handler)
        return nullptr;

    oslSignalHandlerImpl* pHandler
        = static_cast<oslSignalHandlerImpl*>(calloc(1, sizeof(oslSignalHandlerImpl)));

    std::scoped_lock aGuard(getSignalMutex());

    if (!bInitSignal)
        bInitSignal = onInitSignal();

    if (pHandler)
    {
        pHandler->Handler = handler;
        pHandler->pData = pData;

        pHandler->pNext = SignalList;
        SignalList = pHandler;

        return pHandler;
    }

    return nullptr;
}

sal_Bool SAL_CALL osl_removeSignalHandler(oslSignalHandler handler)
{
    std::scoped_lock aGuard(getSignalMutex());

    if (!bInitSignal)
        bInitSignal = onInitSignal();

    oslSignalHandlerImpl* pHandler = SignalList;
    oslSignalHandlerImpl* pPrevious = nullptr;

    while (pHandler)
    {
        if (pHandler == handler)
        {
            if (pPrevious)
                pPrevious->pNext = pHandler->pNext;
            else
                SignalList = pHandler->pNext;

            if (SignalList == nullptr)
                bInitSignal = onDeInitSignal();

            free(pHandler);

            return true;
        }

        pPrevious = pHandler;
        pHandler = pHandler->pNext;
    }

    return false;
}

oslSignalAction SAL_CALL osl_raiseSignal(sal_Int32 userSignal, void* userData)
{
    std::scoped_lock aGuard(getSignalMutex());

    if (!bInitSignal)
        bInitSignal = onInitSignal();

    oslSignalInfo info;
    info.Signal = osl_Signal_User;
    info.UserSignal = userSignal;
    info.UserData = userData;

    oslSignalAction action = callSignalHandler(&info);

    return action;
}

sal_Bool SAL_CALL osl_setErrorReporting(sal_Bool /*bEnable*/)
{
    // this is part of the stable API
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
