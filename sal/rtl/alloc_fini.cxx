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

#include <rtllifecycle.h>

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
    rtlCacheSingleton& theCacheSingleton()
    {
        static rtlCacheSingleton SINGLETON;
        return SINGLETON;
    }
}

void ensureCacheSingleton()
{
    theCacheSingleton();
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
    rtlArenaSingleton& theArenaSingleton()
    {
        static rtlArenaSingleton SINGLETON;
        return SINGLETON;
    }
}

void ensureArenaSingleton()
{
    theArenaSingleton();
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
    rtlLocaleSingleton& theLocaleSingleton()
    {
        static rtlLocaleSingleton SINGLETON;
        return SINGLETON;
    }
}

void ensureLocaleSingleton()
{
    theLocaleSingleton();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
