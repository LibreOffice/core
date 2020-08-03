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

#pragma once

#include <osl/getglobalmutex.hxx>

#include <atomic>
// HACK: <atomic> includes <stdbool.h>, which in some Clang versions does '#define bool bool',
// which confuses clang plugins.
#undef bool
#include <functional>

namespace comphelper
{
/**
 * Thread-safe singleton creation.
 *
 * It is normally sufficient to create singletons using static variables in a function.
 * This function is only for use cases that have a more complex lifetime of the object,
 * such as when the object may require specific cleanup or may be created more times
 * (e.g. when there is a "singleton" per each instance of another object).
 */
template <typename Type, typename Function = std::function<Type*()>,
          typename Guard = osl::MutexGuard, typename GuardCtor = osl::GetGlobalMutex>
static inline Type* doubleCheckedInit(std::atomic<Type*>& pointer, Function function,
                                      GuardCtor guardCtor = osl::GetGlobalMutex())
{
    Type* p = pointer.load(std::memory_order_acquire);
    if (!p)
    {
        Guard guard(guardCtor());
        p = pointer.load(std::memory_order_relaxed);
        if (!p)
        {
            p = function();
            pointer.store(p, std::memory_order_release);
        }
    }
    return p;
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
