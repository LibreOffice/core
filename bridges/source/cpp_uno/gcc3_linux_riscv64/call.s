/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

    .text
    .align 2
    .global privateSnippetExecutor
    .hidden privateSnippetExecutor
    .type privateSnippetExecutor, %function
privateSnippetExecutor:
    .cfi_startproc
    addi sp,sp,-160
    .cfi_def_cfa_offset 160
    sd   ra,152(sp)
    .cfi_offset 1, -8
    fsd  fa0,80(sp)
    fsd  fa1,88(sp)
    fsd  fa2,96(sp)
    fsd  fa3,104(sp)
    fsd  fa4,112(sp)
    fsd  fa5,120(sp)
    fsd  fa6,128(sp)
    fsd  fa7,136(sp)
    sd   a0,16(sp)
    sd   a1,24(sp)
    sd   a2,32(sp)
    sd   a3,40(sp)
    sd   a4,48(sp)
    sd   a5,56(sp)
    sd   a6,64(sp)
    sd   a7,72(sp)
    // a0 = functionIndex
    // a1 = vtableOffset
    // a2 = gpreg
    // a3 = fpreg
    // a4 = overflw
    // a5 = pRegisterReturn
    add  a0,t4,zero
    add  a1,t5,zero
    addi a2,sp,16
    addi a3,sp,80
    addi a4,sp,160
    add  a5,sp,zero
    // jump to cpp_vtable_call
    jalr ra,t6,0

    bne  a0,zero,.OneFloatOneInt
    ld   a0,0(sp)
    ld   a1,8(sp)
    fld  fa0,0(sp)
    fld  fa1,8(sp)
    jal  zero,.EndProgram
.OneFloatOneInt:
    ld   a0,0(sp)
    fld  fa0,8(sp)
.EndProgram:
    ld   ra,152(sp)
    .cfi_restore 1
    addi sp,sp,160
    .cfi_def_cfa_offset 0
    jalr zero,ra,0
    .cfi_endproc
    .size privateSnippetExecutor, .-privateSnippetExecutor
    .section .note.GNU-stack, "", @progbits

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
