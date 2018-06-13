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

#ifndef INCLUDED_COMPHELPER_DOUBLECHECKEDINIT_HXX
#define INCLUDED_COMPHELPER_DOUBLECHECKEDINIT_HXX

#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>

#include <functional>
#include <memory>

namespace comphelper
{
/**
 * Thread-safe singleton creation.
 *
 * This function acts like rtl_Instance::create(), but it uses an external pointer for storing
 * the object.
 */
template <typename Type, typename Function = std::function<Type*()>,
          typename Guard = osl::MutexGuard, typename GuardCtor = osl::GetGlobalMutex>
static inline Type* doubleCheckedInit(Type*& pointer, Function function,
                                      GuardCtor guardCtor = osl::GetGlobalMutex())
{
    Type* p = pointer;
    if (!p)
    {
        Guard guard(guardCtor());
        p = pointer;
        if (!p)
        {
            p = function();
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            pointer = p;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return p;
}

/**
 * Thread-safe singleton creation.
 *
 * This function acts like rtl_Instance::create(), but it uses an external std::unique_ptr for storing
 * the object.
 */
template <typename Type, typename Function = std::function<Type*()>,
          typename Guard = osl::MutexGuard, typename GuardCtor = osl::GetGlobalMutex>
static inline Type* doubleCheckedInit(std::unique_ptr<Type>& pointer, Function function,
                                      GuardCtor guardCtor = osl::GetGlobalMutex())
{
    Type* p = pointer.get();
    if (!p)
    {
        Guard guard(guardCtor());
        p = pointer.get();
        if (!p)
        {
            p = function();
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            pointer.reset(p);
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return p;
}

} // namespace

#endif // INCLUDED_COMPHELPER_DOUBLECHECKEDINIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
