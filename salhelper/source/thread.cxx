/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <stdexcept>
#include <string>

#include <comphelper/scopeguard.hxx>
#include <sal/log.hxx>
#include <salhelper/thread.hxx>

salhelper::Thread::Thread(char const * name): name_(name) {}

void salhelper::Thread::launch() {
    SAL_INFO("salhelper.thread", "launch " << name_);
    // Assumption is that osl::Thread::create returns normally with a true
    // return value iff it causes osl::Thread::run to start executing:
    acquire();
    comphelper::ScopeGuard g([this] { release(); });
    if (!create()) {
        throw std::runtime_error("osl::Thread::create failed");
    }
    g.dismiss();
}

salhelper::Thread::~Thread() {}

void salhelper::Thread::run() {
    // Work around the problem that onTerminated is not called if run throws an exception:
    comphelper::ScopeGuard g([this] { onTerminated(); });
    setName(name_);
    execute();
    g.dismiss();
}

void salhelper::Thread::onTerminated() { release(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
