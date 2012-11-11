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

#include "sal/main.h"

#include "sofficemain.h"

#ifdef DBG_UTIL
#ifdef __gnu_linux__
#include <stdio.h>
#include <stdlib.h>

static int g_Exiting = 0;

/* HACK: detect calls to xmlCleanupParser, which causes hard to debug crashes */
__attribute__ ((visibility("default"))) void xmlCleanupParser(void)
{
    /* there are libraries that register xmlCleanupParser as an atexit handler,
       which is not entirely sound (another atexit handler could want to
       use libxml), but not enough of a problem to complain.
       (example found by llunak: KDE's Strigi library) */
    if (!g_Exiting)
    {
        fprintf(stderr, "\n*** ERROR: DO NOT call xmlCleanupParser()\n\n");
        abort();
    }
}
#endif
#endif

SAL_IMPLEMENT_MAIN() {
    int ret = soffice_main();
#ifdef DBG_UTIL
#ifdef __gnu_linux__
    g_Exiting = 1;
#endif
#endif
    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
