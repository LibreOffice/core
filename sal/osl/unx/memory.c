/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <internal/oslmemory.h>

#include <stdlib.h>
#ifdef __ANDROID__
#include <malloc.h>
#endif

void* osl_aligned_alloc( sal_Size align, sal_Size size )
{
    if (size == 0)
    {
        return NULL;
    }
    else
    {
#if defined __ANDROID__
        return memalign(align, size);
#elif defined MAC_OS_X_VERSION_MAX_ALLOWED && MAC_OS_X_VERSION_MAX_ALLOWED < 1060
        void* ptr = malloc(size + (align - 1) + sizeof(void*));
        if (!ptr) return NULL;
        char* aptr = ((char*)ptr) + sizeof(void*);
        aptr += (align - ((size_t)aptr & (align - 1))) & (align - 1);
        ((void**)aptr)[-1] = ptr;
        return aptr;
#else
        void* ptr;
        int err = posix_memalign(&ptr, align, size);
        return err ? NULL : ptr;
#endif
    }
}

void osl_aligned_free( void* p )
{
#if defined __APPLE__ && MAC_OS_X_VERSION_MAX_ALLOWED < 1060
    free(((void**)p)[-1]);
#else
    free(p);
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
