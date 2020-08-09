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

#include <sal/config.h>

#include <cstring>

#include <sal/types.h>
#include <sal/alloca.h>

#include "callvirtualfunction.hxx"

void callVirtualFunction(
    unsigned long function, unsigned long * gpr, unsigned long * fpr,
    unsigned long * stack, sal_Int32 sp, void * ret)
{
    void * stackargs;
    if (sp != 0) {
        stackargs = alloca(((sp + 1) >> 1) * 16);
        std::memcpy(stackargs, stack, sp * 8);
    }
    asm volatile(
        "ldp x0, x1, [%[gpr_]]\n\t"
        "ldp x2, x3, [%[gpr_], #16]\n\t"
        "ldp x4, x5, [%[gpr_], #32]\n\t"
        "ldp x6, x7, [%[gpr_], #48]\n\t"
        "ldr x8, %[ret_]\n\t"
        "ldr x9, %[function_]\n\t"
        "ldp d0, d1, [%[fpr_]]\n\t"
        "ldp d2, d3, [%[fpr_], #16]\n\t"
        "ldp d4, d5, [%[fpr_], #32]\n\t"
        "ldp d6, d7, [%[fpr_], #48]\n\t"
        "blr x9\n\t"
        "stp x0, x1, [%[gpr_]]\n\t"
        "stp d0, d1, [%[fpr_]]\n\t"
        "stp d2, d3, [%[fpr_], #16]\n\t"
        :: [gpr_]"r" (gpr), [fpr_]"r" (fpr), [function_]"m" (function),
           [ret_]"m" (ret),
           "m" (stackargs) // dummy input to prevent optimizing the alloca away
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "r16", "r17",
#if !defined ANDROID && !defined MACOSX
          "r18"/*TODO?*/,
#endif
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11",
          "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21",
          "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
          "memory"
          // only the bottom 64 bits of v8--15 need to be preserved by callees
    );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
