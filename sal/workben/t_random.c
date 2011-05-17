/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rtl/random.h>

static char *pt (unsigned char *md, int length)
{
    int i;
    static char buf[80];

    for (i=0; i<length; i++)
        sprintf(&(buf[i*2]),"%02x",md[i]);

    return(buf);
}

/*
 * main.
 */
#ifdef WIN32
int __cdecl main (void)
#else
int main (void)
#endif
{
    rtlRandomPool pool;
    pool = rtl_random_createPool();
    if (pool)
    {
        unsigned char buffer[1000];

        rtl_random_getBytes (pool, buffer, 8);
        printf ("random: %s\n", pt (buffer, 8));
    }
    rtl_random_destroyPool (pool);
    return(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
