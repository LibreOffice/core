/*
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/base/SkMalloc.h"

#include <sal/log.hxx>
#include <rtl/alloc.h>

// Based on SkMemory_malloc.cpp :

static inline void sk_out_of_memory(size_t size)
{
    SAL_WARN("skia", "sk_out_of_memory (asked for " << size << " bytes)");
    abort();
}

static inline void* throw_on_failure(size_t size, void* p)
{
    if (size > 0 && p == nullptr)
    {
        // If we've got a nullptr here, the only reason we should have failed is running out of RAM.
        sk_out_of_memory(size);
    }
    return p;
}

void sk_abort_no_print()
{
    SAL_WARN("skia", "sk_abort_no_print");
    abort();
}

void sk_out_of_memory(void)
{
    SAL_WARN("skia", "sk_out_of_memory");
    abort();
}

void* sk_realloc_throw(void* addr, size_t size)
{
    return throw_on_failure(size, rtl_reallocateMemory(addr, size));
}

void sk_free(void* p) { rtl_freeMemory(p); }

void* sk_malloc_flags(size_t size, unsigned flags)
{
    void* p;
    if (flags & SK_MALLOC_ZERO_INITIALIZE)
    {
        p = rtl_allocateZeroMemory(size);
    }
    else
    {
        p = rtl_allocateMemory(size);
    }
    if (flags & SK_MALLOC_THROW)
    {
        return throw_on_failure(size, p);
    }
    else
    {
        return p;
    }
}
