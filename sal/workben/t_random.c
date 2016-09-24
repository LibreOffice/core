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

    return buf;
}

/*
 * main.
 */
int SAL_CALL main (void)
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
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
