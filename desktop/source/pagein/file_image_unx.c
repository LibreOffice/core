/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
