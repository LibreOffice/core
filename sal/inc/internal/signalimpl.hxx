/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_INC_INTERNAL_SIGNALIMPL_H
#define INCLUDED_SAL_INC_INTERNAL_SIGNALIMPL_H

#include <osl/mutex.h>
#include <osl/signal.h>

class SignalImpl
{
    // disable copying
    SignalImpl(const SignalImpl &other) = delete;
    SignalImpl &operator=(const SignalImpl &other) = delete;

public:
    virtual oslSignalHandler addSignalHandler(oslSignalHandlerFunction handler, void* pData);
    virtual bool removeSignalHandler(oslSignalHandler handler);
    virtual oslSignalAction raiseSignal(sal_Int32 userSignal, void* userData);

    virtual bool setErrorReporting(bool bEnable);

protected:
    struct oslSignalHandlerImpl
    {
        oslSignalHandlerFunction    Handler;
        void*                       pData;
        oslSignalHandlerImpl*       pNext;
    };

    SignalImpl();
    virtual ~SignalImpl();

    virtual bool initSignal() = 0;
    virtual bool deInitSignal() = 0;

    virtual oslSignalAction callSignalHandler(oslSignalInfo* pInfo);

    bool mErrorReportingEnabled;
    bool mInitSignal;

    oslMutex mSignalListMutex;
    oslSignalHandlerImpl* mSignalList;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
