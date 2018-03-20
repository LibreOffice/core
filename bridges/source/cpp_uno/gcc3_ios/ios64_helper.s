// -*- Mode: Asm; tab-width: 4; tab-stop-list: (4 12 32); comment-column: 30; comment-start: "// "; indent-tabs-mode: nil -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file incorporates work covered by the following license notice:
//
//   Licensed to the Apache Software Foundation (ASF) under one or more
//   contributor license agreements. See the NOTICE file distributed
//   with this work for additional information regarding copyright
//   ownership. The ASF licenses this file to you under the Apache
//   License, Version 2.0 (the "License"); you may not use this file
//   except in compliance with the License. You may obtain a copy of
//   the License at http://www.apache.org/licenses/LICENSE-2.0 .
//
    .section        __TEXT,__text,regular,pure_instructions

    .p2align        2
codeSnippet_0_0:
    mov x14, 0
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_0_1:
    mov x14, 0
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_0_2:
    mov x14, 0
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_0_3:
    mov x14, 0
    mov x15, 3
    b _privateSnippetExecutor
codeSnippet_1_0:
    mov x14, 1
    mov x15, 0
    b _privateSnippetExecutor
    .long 0x000001
    .long 0
codeSnippet_1_1:
    mov x14, 1
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_1_2:
    mov x14, 1
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_1_3:
    mov x14, 1
    mov x15, 3
    b _privateSnippetExecutor
codeSnippet_2_0:
    mov x14, 2
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_2_1:
    mov x14, 2
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_2_2:
    mov x14, 2
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_2_3:
    mov x14, 2
    mov x15, 3
    b _privateSnippetExecutor
    .long 0x000002
    .long 3
codeSnippet_3_0:
    mov x14, 3
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_3_1:
    mov x14, 3
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_3_2:
    mov x14, 3
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_3_3:
    mov x14, 3
    mov x15, 3
    b _privateSnippetExecutor
codeSnippet_4_0:
    mov x14, 4
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_4_1:
    mov x14, 4
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_4_2:
    mov x14, 4
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_4_3:
    mov x14, 4
    mov x15, 3
    b _privateSnippetExecutor
codeSnippet_5_0:
    mov x14, 5
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_5_1:
    mov x14, 5
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_5_2:
    mov x14, 5
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_5_3:
    mov x14, 5
    mov x15, 3
    b _privateSnippetExecutor
codeSnippet_6_0:
    mov x14, 6
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_6_1:
    mov x14, 6
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_6_2:
    mov x14, 6
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_6_3:
    mov x14, 6
    mov x15, 3
    b _privateSnippetExecutor
codeSnippet_7_0:
    mov x14, 7
    mov x15, 0
    b _privateSnippetExecutor
codeSnippet_7_1:
    mov x14, 7
    mov x15, 1
    b _privateSnippetExecutor
codeSnippet_7_2:
    mov x14, 7
    mov x15, 2
    b _privateSnippetExecutor
codeSnippet_7_3:
    mov x14, 7
    mov x15, 3
    b _privateSnippetExecutor

    .globl _codeSnippets
_codeSnippets:
    .long codeSnippet_0_0 - _codeSnippets
    .long codeSnippet_0_1 - _codeSnippets
    .long codeSnippet_0_2 - _codeSnippets
    .long codeSnippet_0_3 - _codeSnippets
    .long codeSnippet_1_0 - _codeSnippets
    .long codeSnippet_1_1 - _codeSnippets
    .long codeSnippet_1_2 - _codeSnippets
    .long codeSnippet_1_3 - _codeSnippets
    .long codeSnippet_2_0 - _codeSnippets
    .long codeSnippet_2_1 - _codeSnippets
    .long codeSnippet_2_2 - _codeSnippets
    .long codeSnippet_2_3 - _codeSnippets
    .long codeSnippet_3_0 - _codeSnippets
    .long codeSnippet_3_1 - _codeSnippets
    .long codeSnippet_3_2 - _codeSnippets
    .long codeSnippet_3_3 - _codeSnippets
    .long codeSnippet_4_0 - _codeSnippets
    .long codeSnippet_4_1 - _codeSnippets
    .long codeSnippet_4_2 - _codeSnippets
    .long codeSnippet_4_3 - _codeSnippets
    .long codeSnippet_5_0 - _codeSnippets
    .long codeSnippet_5_1 - _codeSnippets
    .long codeSnippet_5_2 - _codeSnippets
    .long codeSnippet_5_3 - _codeSnippets
    .long codeSnippet_6_0 - _codeSnippets
    .long codeSnippet_6_1 - _codeSnippets
    .long codeSnippet_6_2 - _codeSnippets
    .long codeSnippet_6_3 - _codeSnippets
    .long codeSnippet_7_0 - _codeSnippets
    .long codeSnippet_7_1 - _codeSnippets
    .long codeSnippet_7_2 - _codeSnippets
    .long codeSnippet_7_3 - _codeSnippets



    .private_extern _privateSnippetExecutor
    .globl  _privateSnippetExecutor
    .p2align        2
_privateSnippetExecutor:
    .cfi_startproc
    .cfi_def_cfa w29, 16
    .cfi_offset w30, -8
    .cfi_offset w29, -16

    // _privateSnippetExecutor is jumped to from codeSnippet_*

    // push all GP, FP/SIMD registers to the stack
    stp x6, x7, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    stp d6, d7, [sp, #-16]!
    stp d4, d5, [sp, #-16]!
    stp d2, d3, [sp, #-16]!
    stp d0, d1, [sp, #-16]!

    // push x8 (RC pointer) and lr to stack
    stp     x8, lr, [sp, #-16]!

    // First argument (x15 set up in the codeSnippet instance)
    // Second argument: The pointer to all the above
    mov x0, x14
    mov x1, x15
    mov x2, sp
    bl  _cpp_vtable_call

    // restore x8 (RC pointer) and lr (skip RC from cpp_vtable_call)
    ldp x8, lr, [sp, #0]

    // restore stack
    add sp, sp, #144

    // continue with throw/catch
    ret lr
    .cfi_endproc

// vim:set shiftwidth=4 softtabstop=4 expandtab:
