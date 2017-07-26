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
#include <sal/macros.h>

#include <cstdio>

SAL_IMPLEMENT_MAIN()
{
    int aIntArray[] = { 1, 3, 5, 7 };
    fprintf(stdout, "Examples to show use of macros defined in macro.h.\n");

    // SAL_N_ELEMENTS example
    fprintf(stdout, "\nExample of SAL_N_ELEMENTS(aIntArray):\n");
    fprintf(stdout, "    aIntArray[] has %lu elements.\n", SAL_N_ELEMENTS(aIntArray));

    // SAL_BOUND examples
    int nNumbers[2];
    fprintf(stdout, "\nExamples of SAL_BOUND(test, lower, higher):\n");
    fprintf(stdout, "    int aIntArray[] = { 1, 3, 5, 7 };\n");

    for (int nNum = 0; nNum < 8; nNum++)
    {
        nNumbers[0] = 0;
        nNumbers[1] = 1;
        fprintf(stdout, "    SAL_BOUND(%d, %d, %d): The number %d is closest to %d between aIntArray[%d] and aIntArray[%d]\n",
                nNum, nNumbers[0], nNumbers[1], nNum, SAL_BOUND(nNum, nNumbers[0], nNumbers[1]), nNumbers[0], nNumbers[1]);

        nNumbers[0] = 1;
        nNumbers[1] = 2;
        fprintf(stdout, "    SAL_BOUND(%d, %d, %d): The number %d is closest to %d between aIntArray[%d] and aIntArray[%d]\n",
                nNum, nNumbers[0], nNumbers[1], nNum, SAL_BOUND(nNum, nNumbers[0], nNumbers[1]), nNumbers[0], nNumbers[1]);


        nNumbers[0] = 2;
        nNumbers[1] = 3;
        fprintf(stdout, "    SAL_BOUND(%d, %d, %d): The number %d is closest to %d between aIntArray[%d] and aIntArray[%d]\n",
                nNum, nNumbers[0], nNumbers[1], nNum, SAL_BOUND(nNum, nNumbers[0], nNumbers[1]), nNumbers[0], nNumbers[1]);

        nNumbers[0] = 0;
        nNumbers[1] = 4;
        fprintf(stdout, "    SAL_BOUND(%d, %d, %d): The number %d is closest to %d between aIntArray[%d] and aIntArray[%d]\n",
                nNum, nNumbers[0], nNumbers[1], nNum, SAL_BOUND(nNum, nNumbers[0], nNumbers[1]), nNumbers[0], nNumbers[1]);

        nNumbers[0] = 4;
        nNumbers[1] = 0;
        fprintf(stdout, "    SAL_BOUND(%d, %d, %d): The number %d is closest to %d between aIntArray[%d] and aIntArray[%d]\n\n",
                nNum, nNumbers[0], nNumbers[1], nNum, SAL_BOUND(nNum, nNumbers[0], nNumbers[1]), nNumbers[0], nNumbers[1]);
    }

    // SAL_STRINGIFY example
    fprintf(stdout, "\nExamples of SAL_STRINGIFY(x):\n");

#define foo Example string "foo"
    fprintf(stdout, "\n    #define foo Example string \"foo\"\n");
    fprintf(stdout, "    SAL_STRINGIFY(foo): %s\n", SAL_STRINGIFY(foo));

#define foo2 Example string \"foo\"
    fprintf(stdout, "\n    #define foo Example string \\\"foo\\\"\n");
    fprintf(stdout, "    SAL_STRINGIFY(foo2): %s\n", SAL_STRINGIFY(foo2));

#define foo3 2
    fprintf(stdout, "\n    #define foo3 2\n");
    fprintf(stdout, "    SAL_STRINGIFY(foo3): %s\n", SAL_STRINGIFY(foo3));

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
