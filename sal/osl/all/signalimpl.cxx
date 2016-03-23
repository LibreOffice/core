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

#include "internal/signalimpl.hxx"

#include <sal/config.h>
#include <config_features.h>
#include <osl/diagnose.h>
#include <sal/macros.h>


SignalImpl::SignalImpl()
  : mErrorReportingEnabled(true),
    mInitSignal(false),
    mSignalList(nullptr)
{
}

SignalImpl::~SignalImpl()
{

}

oslSignalAction SignalImpl::callSignalHandler(oslSignalInfo* pInfo)
{
    oslSignalHandlerImpl* pHandler = mSignalList;
    oslSignalAction action = osl_Signal_ActCallNextHdl;

    while (pHandler != nullptr)
    {
        if ((action = pHandler->Handler(pHandler->pData, pInfo))
                != osl_Signal_ActCallNextHdl)
            break;

        pHandler = pHandler->pNext;
    }

    return action;
}

oslSignalHandler SignalImpl::addSignalHandler(oslSignalHandlerFunction handler, void* pData)
{
    OSL_ASSERT(handler != nullptr);
    if (handler == nullptr )
    {
        return nullptr;
    }

    if (! mInitSignal)
        mInitSignal = initSignal();

    oslSignalHandlerImpl* pHandler = static_cast<oslSignalHandlerImpl*>(calloc(1, sizeof(oslSignalHandlerImpl)));

    if (pHandler != nullptr)
    {
        pHandler->Handler = handler;
        pHandler->pData   = pData;

        osl_acquireMutex(mSignalListMutex);

        pHandler->pNext  = mSignalList;
        mSignalList      = pHandler;

        osl_releaseMutex(mSignalListMutex);

        return pHandler;
    }

    return nullptr;
}

bool SignalImpl::removeSignalHandler(oslSignalHandler handler)
{
    OSL_ASSERT(handler != nullptr);

    if (! mInitSignal)
        mInitSignal = initSignal();

    osl_acquireMutex(mSignalListMutex);

    oslSignalHandlerImpl* pHandler = mSignalList;
    oslSignalHandlerImpl* pPrevious = nullptr;

    while (pHandler != nullptr)
    {
        if (pHandler == handler)
        {
            if (pPrevious)
                pPrevious->pNext = pHandler->pNext;
            else
                mSignalList = pHandler->pNext;

            osl_releaseMutex(mSignalListMutex);

            if (mSignalList == nullptr)
                mInitSignal = deInitSignal();

            free(pHandler);

            return true;
        }

        pPrevious = pHandler;
        pHandler  = pHandler->pNext;
    }

    osl_releaseMutex(mSignalListMutex);

    return false;
}

oslSignalAction SignalImpl::raiseSignal(sal_Int32 userSignal, void* userData)
{
    if (! mInitSignal)
        mInitSignal = initSignal();

    osl_acquireMutex(mSignalListMutex);

    oslSignalInfo info;
    info.Signal     = osl_Signal_User;
    info.UserSignal = userSignal;
    info.UserData   = userData;

    oslSignalAction action = callSignalHandler(&info);

    osl_releaseMutex(mSignalListMutex);

    return action;
}

bool SignalImpl::setErrorReporting(bool bEnable)
{
    bool bOld = mErrorReportingEnabled;
    mErrorReportingEnabled = bEnable;

    return bOld;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
