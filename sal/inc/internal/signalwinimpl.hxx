/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_INC_INTERNAL_SIGNALWINIMPL_H
#define INCLUDED_SAL_INC_INTERNAL_SIGNALWINIMPL_H

#include "internal/signalimpl.hxx"
#include "system.h"

class SignalWinImpl : public SignalImpl
{
public:
    static SignalWinImpl& instance();

    bool setErrorReporting(bool bEnable) override;

    long signalHandlerFunction(LPEXCEPTION_POINTERS lpEP);

protected:
    bool initSignal() override;
    bool deInitSignal() override;

private:
    SignalWinImpl();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
