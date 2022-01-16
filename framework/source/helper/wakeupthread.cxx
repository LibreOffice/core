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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XUpdatable.hpp>

#include <helper/wakeupthread.hxx>
#include <chrono>

using namespace std::chrono_literals;

void framework::WakeUpThread::execute() {
    for (;;) {
        {
            std::unique_lock g(mutex_);
            condition_.wait_for(g, 25ms, [this] { return terminate_; });
            if (terminate_) {
                break;
            }
        }
        css::uno::Reference<css::util::XUpdatable> up(updatable_);
        if (up.is()) {
            up->update();
        }
    }
}

framework::WakeUpThread::WakeUpThread(
    css::uno::Reference<css::util::XUpdatable> const & updatable):
    Thread("WakeUpThread"), updatable_(updatable), terminate_(false)
{}

void framework::WakeUpThread::stop() {
    {
        std::unique_lock g(mutex_);
        terminate_ = true;
    }
    condition_.notify_one();
    join();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
