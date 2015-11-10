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

#include "file_image.h"

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

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

    if (image == NULL)
        return EINVAL;

    image->m_base = MAP_FAILED, image->m_size = 0;

    if ((fd = open (filename, O_RDONLY)) == -1)
        return errno;

    if (fstat (fd, &st) == -1)
    {
        result = errno;
        goto cleanup_and_leave;
    }

    p = mmap (NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED)
    {
        result = errno;
        goto cleanup_and_leave;
    }

    image->m_base = p, image->m_size = st.st_size;

cleanup_and_leave:
    close (fd);
    return result;
}

/*
 * file_image_pagein.
 */
int file_image_pagein (file_image * image)
{
    file_image    w;
    long          s;
    size_t        k;
    // force touching of each page despite the optimizer
    volatile char c =0;

    if (image == NULL)
        return EINVAL;

    if ((w.m_base = image->m_base) == NULL)
        return EINVAL;
    if ((w.m_size = image->m_size) == 0)
        return 0;

    if (madvise (w.m_base, w.m_size, MADV_WILLNEED) == -1)
        return errno;

    if ((s = sysconf (_SC_PAGESIZE)) == -1)
        s = 0x1000;

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
    }

    return 0;
}

/*
 * file_image_close
 */
int file_image_close (file_image * image)
{
    if (image == NULL)
        return EINVAL;

    if (munmap (image->m_base, image->m_size) == -1)
        return errno;

    image->m_base = NULL, image->m_size = 0;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
