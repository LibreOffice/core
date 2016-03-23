/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_INC_INTERNAL_SIGNALUNIXIMPL_H
#define INCLUDED_SAL_INC_INTERNAL_SIGNALUNIXIMPL_H

#include "internal/signalimpl.hxx"

class SignalUnixImpl : public SignalImpl
{
public:
    static SignalUnixImpl& instance();

    void signalHandlerFunction(int signal);

protected:
    bool initSignal() override;
    bool deInitSignal() override;

private:
    SignalUnixImpl();

    bool mSetSEGVHandler;
    bool mSetWINCHHandler;
    bool mSetILLHandler;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
