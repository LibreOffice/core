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
#include <rtl/alloc.h>
#include <rtl/ustring.h>

#include "strimp.hxx"
#include "alloc_impl.hxx"
#include "alloc_arena.hxx"

sal_Int16 rtl_ImplGetDigit( sal_Unicode ch, sal_Int16 nRadix )
{
    sal_Int16 n = -1;
    if ( (ch >= '0') && (ch <= '9') )
        n = ch-'0';
    else if ( (ch >= 'a') && (ch <= 'z') )
        n = ch-'a'+10;
    else if ( (ch >= 'A') && (ch <= 'Z') )
        n = ch-'A'+10;
    return (n < nRadix) ? n : -1;
}

bool rtl_ImplIsWhitespace( sal_Unicode c )
{
    /* Space or Control character? */
    if ( (c <= 32) && c )
        return true;

    /* Only in the General Punctuation area Space or Control characters are included? */
    if ( (c < 0x2000) || (c > 0x206F) )
        return false;

    if ( ((c >= 0x2000) && (c <= 0x200B)) ||    /* All Spaces           */
         (c == 0x2028) ||                       /* LINE SEPARATOR       */
         (c == 0x2029) )                        /* PARAGRAPH SEPARATOR  */
        return true;

    return false;
}

/*
 * TODO: add a slower, more awful, but more space efficient
 * custom allocator for the pre-init phase. Existing slab
 * allocator's minimum alloc size is 24bytes, and by default
 * is 32 bytes.
 */
static rtl_arena_type *pre_arena = nullptr;

rtl_allocateStringFn rtl_allocateString = rtl_allocateMemory;
rtl_freeStringFn rtl_freeString = rtl_freeMemory;

extern "C" {
static void *pre_allocateStringFn(sal_Size n)
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
    sal_Size size = reinterpret_cast<sal_uInt32*>(addr)[0] + 12;

    rtl_arena_free(pre_arena, addr, size);
}
} // extern "C"

static void mark_static(void *addr, sal_Size /* size */, void *)
{
    char *inner = static_cast<char*>(addr) + 4;
    rtl_uString *str = reinterpret_cast<rtl_uString *>(inner);
    str->refCount |= SAL_STRING_STATIC_FLAG;
}

void SAL_CALL rtl_alloc_preInit (sal_Bool start) SAL_THROW_EXTERN_C()
{
    if (start)
    {
        rtl_allocateString = pre_allocateStringFn;
        rtl_freeString = pre_freeStringFn;
        pre_arena = rtl_arena_create("pre-init strings", 4, 0,
                                     nullptr, rtl_arena_alloc,
                                     rtl_arena_free, 0);
    }
    else // back to normal
    {
        rtl_arena_foreach(pre_arena, mark_static, nullptr);
        rtl_allocateString = rtl_allocateMemory;
        rtl_freeString = rtl_freeMemory;

        // TODO: also re-intialize main allocator as well.
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
