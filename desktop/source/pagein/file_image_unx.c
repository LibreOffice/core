/*************************************************************************
 *
 *  $RCSfile: file_image_unx.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:41:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "file_image.h"

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#if defined(LINUX)
#define __USE_BSD /* madvise, MADV_WILLNEED */
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
