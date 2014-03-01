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
#ifdef __ANDROID__
    return memalign(align, size);
#else
    if (size == 0)
    {
        return NULL;
    }
    else
    {
        void* ptr;
        int err;
#ifdef __APPLE__
#ifdef TARGET_OS_MAC
        /* Darwin/OSX lacks posix_memalign */
        ptr = malloc(size);
        err = (ptr != NULL);
#endif
#else
        err = posix_memalign(&ptr, align, size);
#endif
        return err ? NULL : ptr;
    }
#endif
}

void osl_aligned_free( void* p )
{
    free(p);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
