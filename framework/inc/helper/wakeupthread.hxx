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

#ifndef INCLUDED_FRAMEWORK_INC_HELPER_WAKEUPTHREAD_HXX
#define INCLUDED_FRAMEWORK_INC_HELPER_WAKEUPTHREAD_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <sal/types.h>
#include <salhelper/thread.hxx>

namespace com { namespace sun { namespace star { namespace util {
    class XUpdatable;
} } } }

namespace framework{

class WakeUpThread: public salhelper::Thread {
    css::uno::WeakReference<css::util::XUpdatable> updatable_;
    osl::Condition condition_;

    osl::Mutex mutex_;
    bool terminate_;

    void execute() override;

public:
    WakeUpThread(css::uno::Reference<css::util::XUpdatable> const & updatable);

    void stop();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
