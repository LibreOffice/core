/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <oslmemory.h>

#include <stdlib.h>

#ifdef __ANDROID__
#include <malloc.h>
#ifndef HAVE_MEMALIGN
#define HAVE_MEMALIGN 1 /* for Android, we are really sure about memalign */
#endif
#endif

void* osl_aligned_alloc( sal_Size align, sal_Size size )
{
    if (size == 0)
    {
        return nullptr;
    }
    else
    {
#if defined(HAVE_POSIX_MEMALIGN)
        void* ptr;
        int err = posix_memalign(&ptr, align, size);
        return err ? nullptr : ptr;
#elif defined(HAVE_MEMALIGN)
        return memalign(align, size);
#else
        void* ptr = malloc(size + (align - 1) + sizeof(void*));
        if (!ptr) return nullptr;
        char* aptr = ((char*)ptr) + sizeof(void*);
        aptr += (align - ((size_t)aptr & (align - 1))) & (align - 1);
        ((void**)aptr)[-1] = ptr;
        return aptr;
#endif
    }
}

void osl_aligned_free( void* p )
{
#if defined(HAVE_POSIX_MEMALIGN) || defined(HAVE_MEMALIGN)
    free(p);
#else
    free(((void**)p)[-1]);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
