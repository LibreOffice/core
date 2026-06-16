/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <sal/types.h>
#include <rtl/random.h>
#include <oslrandom.h>


void rtl_random_getBytes (void *Buffer, sal_Size Bytes) noexcept
{
    assert(Buffer != nullptr);

    if (!osl_get_system_random_data(static_cast<char*>(Buffer), Bytes))
    {
        ::std::fprintf(stderr, "rtl_random_getBytes(): cannot read random device, aborting.\n");
        ::std::abort();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
