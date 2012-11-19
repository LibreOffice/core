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

#include <rtl/instance.hxx>

#include "internal/rtllifecycle.h"

namespace
{
    struct rtlMemorySingleton
    {
        rtlMemorySingleton()
        {
            rtl_memory_init();
        }
        ~rtlMemorySingleton()
        {
            rtl_memory_fini();
        }
    };
    class theMemorySingleton
        : public rtl::Static<rtlMemorySingleton, theMemorySingleton>{};
}

void ensureMemorySingleton()
{
    theMemorySingleton::get();
}

namespace
{
    struct rtlCacheSingleton
    {
        rtlCacheSingleton()
        {
            rtl_cache_init();
        }
        ~rtlCacheSingleton()
        {
            rtl_cache_fini();
        }
    };
    class theCacheSingleton
        : public rtl::Static<rtlCacheSingleton, theCacheSingleton>{};
}

void ensureCacheSingleton()
{
    theCacheSingleton::get();
}

namespace
{
    struct rtlArenaSingleton
    {
        rtlArenaSingleton()
        {
            rtl_arena_init();
        }
        ~rtlArenaSingleton()
        {
            rtl_arena_fini();
        }
    };
    class theArenaSingleton
        : public rtl::Static<rtlArenaSingleton, theArenaSingleton>{};
}

void ensureArenaSingleton()
{
    theArenaSingleton::get();
}

namespace
{
    struct rtlLocaleSingleton
    {
        rtlLocaleSingleton()
        {
            rtl_locale_init();
        }
        ~rtlLocaleSingleton()
        {
            rtl_locale_fini();
        }
    };
    class theLocaleSingleton
        : public rtl::Static<rtlLocaleSingleton, theLocaleSingleton>{};
}

void ensureLocaleSingleton()
{
    theLocaleSingleton::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
