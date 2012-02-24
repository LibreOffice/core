/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "sal/config.h"

#include <stdexcept>
#include <string>

#include "sal/log.hxx"
#include "salhelper/thread.hxx"

salhelper::Thread::Thread(char const * name): name_(name) {}

void salhelper::Thread::launch() {
    SAL_INFO("salhelper.thread", "launch " << name_);
    // Assumption is that osl::Thread::create returns normally with a true
    // return value iff it causes osl::Thread::run to start executing:
    acquire();
    try {
        if (!create()) {
            throw std::runtime_error("osl::Thread::create failed");
        }
    } catch (...) {
        release();
        throw;
    }
}

salhelper::Thread::~Thread() {}

void salhelper::Thread::run() {
    try {
        setName(name_);
        execute();
    } catch (...) {
        // Work around the problem that onTerminated is not called if run throws
        // an exception:
        onTerminated();
        throw;
    }
}

void salhelper::Thread::onTerminated() { release(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
