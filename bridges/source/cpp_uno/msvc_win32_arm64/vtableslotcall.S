/* -*- tab-width: 4; indent-tabs-mode: nil; fill-column: 100 -*- */
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

    OPT 2   // disable listing
// macros to add unwind information
#include "ksarm64.h"
    OPT 1   // re-enable listing

    EXPORT  vtableSlotCall
    IMPORT  vtableCall

    TEXTAREA, ALIGN=2

    NESTED_ENTRY vtableSlotCall

        PROLOG_SAVE_REG_PAIR   fp, lr, #-192!
        PROLOG_SAVE_REG_PAIR   x19, x20, #16

        add   x11, sp, 192
        add   x20, sp, 128
        add   x19, sp, 64

        stp x11, x11, [sp, 32]
        str x11, [sp, 48]
        stp wzr, wzr, [sp, 56]
        stp x0, x1, [sp, 64]
        mov w0, w9
        mov w1, w10
        stp x2, x3, [sp, 80]
        mov x3, x20
        mov x2, x19
        stp x4, x5, [sp, 96]
        mov x4, x11
        stp x6, x7, [sp, 112]
        stp d0, d1, [sp, 128]
        stp d2, d3, [sp, 144]
        stp d4, d5, [sp, 160]
        stp d6, d7, [sp, 176]

        bl vtableCall

        ldp x0, x1, [x19]
        ldp d0, d1, [x20]
        ldp d2, d3, [x20, #16]

        EPILOG_STACK_RESTORE
        EPILOG_RESTORE_REG_PAIR   x19, x20, #16
        EPILOG_RESTORE_REG_PAIR   fp, lr, #192!
        EPILOG_RETURN

        NESTED_END vtableSlotCall

    END

/* vim:set shiftwidth=4 softtabstop=4 expandtab */
