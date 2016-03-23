/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_features.h>

#include <osl/signal.h>

#include "internal/signalimpl.hxx"
#ifdef SAL_W32
#include "internal/signalwinimpl.hxx"
#else
#include "internal/signaluniximpl.hxx"
#endif

namespace
{
SignalImpl* CreateSignalImpl()
{
#ifdef SAL_W32
    return &SignalWinImpl::instance();
#else
    return &SignalUnixImpl::instance();
#endif
}
}

SignalImpl* signalImpl = CreateSignalImpl();


oslSignalHandler SAL_CALL osl_addSignalHandler(oslSignalHandlerFunction Handler, void* pData)
{
    return signalImpl->addSignalHandler(Handler, pData);
}

sal_Bool SAL_CALL osl_removeSignalHandler(oslSignalHandler Handler)
{
    return signalImpl->removeSignalHandler(Handler);
}

oslSignalAction SAL_CALL osl_raiseSignal(sal_Int32 UserSignal, void* UserData)
{
    return signalImpl->raiseSignal(UserSignal, UserData);
}

sal_Bool SAL_CALL osl_setErrorReporting(sal_Bool bEnable)
{
    return signalImpl->setErrorReporting(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
