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

#include <assert.h>
#include <stdlib.h>
#include <rtl/alloc.h>
#include <rtl/ustring.h>
#include <rtllifecycle.h>

#include "strimp.hxx"
#include "alloc_impl.hxx"
#include "alloc_arena.hxx"

/*
 * TODO: add a slower, more awful, but more space efficient
 * custom allocator for the pre-init phase. Existing slab
 * allocator's minimum alloc size is 24bytes, and by default
 * is 32 bytes.
 */
static rtl_arena_type *pre_arena = nullptr;

rtl_allocateStringFn rtl_allocateString = malloc;
rtl_freeStringFn rtl_freeString = free;

extern "C" {
static void *pre_allocateStringFn(size_t n)
{
    sal_Size size = RTL_MEMORY_ALIGN(n + 4, 4);
    char *addr = static_cast<char*>(rtl_arena_alloc(pre_arena, &size));
    assert(size>= 12);
    reinterpret_cast<sal_uInt32*>(addr)[0] = size - 12;
    return addr + 4;
}

static void pre_freeStringFn(void *data)
{
    char    *addr = static_cast<char*>(data) - 4;
    sal_uInt32 size = reinterpret_cast<sal_uInt32*>(addr)[0] + 12;

    rtl_arena_free(pre_arena, addr, size);
}
} // extern "C"

static void mark_static(void *addr, sal_Size /* size */)
{
    char *inner = static_cast<char*>(addr) + 4;
    rtl_uString *str = reinterpret_cast<rtl_uString *>(inner);
    str->refCount |= SAL_STRING_STATIC_FLAG;
}

void alloc_preInit (bool start) noexcept
{
    if (start)
    {
        rtl_allocateString = pre_allocateStringFn;
        rtl_freeString = pre_freeStringFn;
        pre_arena = rtl_arena_create("pre-init strings", 4, 0,
                                     nullptr, rtl_arena_alloc,
                                     rtl_arena_free, 0);

        // To be consistent (and to ensure the rtl_cache threads are started).
        ensureCacheSingleton();
    }
    else
    {
        rtl_arena_foreach(pre_arena, mark_static);
        rtl_allocateString = malloc;
        rtl_freeString = free;

        // TODO: also re-initialize main allocator as well.
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
