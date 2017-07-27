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

#include <sal/main.h>
#include <sal/alloca.h>

#include <cstdio>
#include <cstring>

// cf. https://www.gnu.org/software/libc/manual/html_node/Variable-Size-Automatic.html

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    fprintf(stdout, "Example to show alloca().\n");

    int len=0;
    char *name;

    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
            len += strlen(argv[i]);

        name = (char*)alloca(len+argc+1); // argc spaces between words, with terminating nullptr
        fprintf(stdout, "    Just used alloca(%d)\n", len+argc+1);

        for (int i = 1; i < argc; i++)
            strcat(strcat(name, argv[i]), " ");

        fprintf(stdout, "    Arguments concatenated are \"%s\"\n", name);
    }
    else
    {
        fprintf(stderr, "Error, no arguments.\n");
        return 1;
    }


    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
