/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: file_image_unx.c,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "file_image.h"

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#if defined(LINUX)
#  ifndef __USE_BSD
#    define __USE_BSD /* madvise, MADV_WILLNEED */
#  endif
#endif /* Linux */

#include <sys/mman.h>
#include <sys/stat.h>

#include <string.h>

/*
 * file_image_open
 */
int file_image_open (file_image * image, const char * filename)
{
    int         result = 0;
    int         fd;
    struct stat st;
    void *      p;

    if (image == 0)
        return (EINVAL);

    image->m_base = MAP_FAILED, image->m_size = 0;

    if ((fd = open (filename, O_RDONLY)) == -1)
        return (errno);

    if (fstat (fd, &st) == -1)
    {
        result = errno;
        goto cleanup_and_leave;
    }

    p = mmap (0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED)
    {
        result = errno;
        goto cleanup_and_leave;
    }

    image->m_base = p, image->m_size = st.st_size;

cleanup_and_leave:
    close (fd);
    return (result);
}

/*
 * file_image_pagein.
 */
int file_image_pagein (file_image * image)
{
    file_image    w;
    long          s;
    size_t        k;
    volatile char c = 0;

    if (image == 0)
        return (EINVAL);

    if ((w.m_base = image->m_base) == 0)
        return (EINVAL);
    if ((w.m_size = image->m_size) == 0)
        return (0);

    if (madvise (w.m_base, w.m_size, MADV_WILLNEED) == -1)
    {
#ifndef MACOSX
        return (errno);
#else
        /* madvise MADV_WILLNEED need not succeed here */
        /* but that is fine */
#endif
    }


#ifndef MACOSX
    if ((s = sysconf (_SC_PAGESIZE)) == -1)
        s = 0x1000;
#else
    s = getpagesize();
#endif

    k = (size_t)(s);
    while (w.m_size > k)
    {
        c ^= ((char*)(w.m_base))[0];
        w.m_base  = (char*)(w.m_base) + k;
        w.m_size -= k;
    }
    if (w.m_size > 0)
    {
        c ^= ((char*)(w.m_base))[0];
        w.m_base  = (char*)(w.m_base) + w.m_size;
        w.m_size -= w.m_size;
    }

    return (0);
}

/*
 * file_image_close
 */
int file_image_close (file_image * image)
{
    if (image == 0)
        return (EINVAL);

    if (munmap (image->m_base, image->m_size) == -1)
        return (errno);

    image->m_base = 0, image->m_size = 0;
    return (0);
}
