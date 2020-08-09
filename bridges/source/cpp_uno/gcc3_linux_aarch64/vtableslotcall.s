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

    .arch armv8-a
    .text
    .align 2
#ifndef __APPLE__
    .global vtableSlotCall
    .hidden vtableSlotCall
    .type vtableSlotCall, %function
vtableSlotCall:
#else
	.global _vtableSlotCall
_vtableSlotCall:
#endif
    .cfi_startproc
    stp x29, x30, [sp, -192]!
    .cfi_def_cfa_offset 192
    .cfi_offset 29, -192
    .cfi_offset 30, -184
    add x11, sp, 192
    mov x29, sp
    stp x19, x20, [sp, 16]
    .cfi_offset 19, -176
    .cfi_offset 20, -168
    add x20, sp, 128
    add x19, sp, 64
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
    mov x5, x8
    mov x4, x11
    stp x6, x7, [sp, 112]
    stp d0, d1, [sp, 128]
    stp d2, d3, [sp, 144]
    stp d4, d5, [sp, 160]
    stp d6, d7, [sp, 176]
#ifndef __APPLE__
    bl vtableCall
#else
	bl _vtableCall
#endif
    ldp x0, x1, [x19]
    ldp d0, d1, [x20]
    ldp d2, d3, [x20, #16]
    ldp x19, x20, [sp, 16]
    ldp x29, x30, [sp], 192
    .cfi_restore 30
    .cfi_restore 29
    .cfi_restore 19
    .cfi_restore 20
    .cfi_def_cfa_offset 0
    ret
    .cfi_endproc
#ifndef __APPLE__
    .size vtableSlotCall, .-vtableSlotCall
    .section .note.GNU-stack, "", @progbits
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab */
