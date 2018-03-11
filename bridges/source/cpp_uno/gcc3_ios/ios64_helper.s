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
    .text
    .align 4

codeSnippet_00000000_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 00000000
    .long 0
codeSnippet_00000000_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 00000000
    .long 1
codeSnippet_00000000_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 00000000
    .long 2
codeSnippet_00000000_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 00000000
    .long 3
codeSnippet_00000001_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000001
    .long 0
codeSnippet_00000001_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000001
    .long 1
codeSnippet_00000001_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000001
    .long 2
codeSnippet_00000001_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000001
    .long 3
codeSnippet_00000002_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000002
    .long 0
codeSnippet_00000002_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000002
    .long 1
codeSnippet_00000002_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000002
    .long 2
codeSnippet_00000002_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000002
    .long 3
codeSnippet_00000003_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000003
    .long 0
codeSnippet_00000003_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000003
    .long 1
codeSnippet_00000003_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000003
    .long 2
codeSnippet_00000003_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000003
    .long 3
codeSnippet_00000004_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000004
    .long 0
codeSnippet_00000004_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000004
    .long 1
codeSnippet_00000004_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000004
    .long 2
codeSnippet_00000004_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000004
    .long 3
codeSnippet_00000005_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000005
    .long 0
codeSnippet_00000005_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000005
    .long 1
codeSnippet_00000005_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000005
    .long 2
codeSnippet_00000005_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000005
    .long 3
codeSnippet_00000006_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000006
    .long 0
codeSnippet_00000006_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000006
    .long 1
codeSnippet_00000006_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000006
    .long 2
codeSnippet_00000006_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000006
    .long 3
codeSnippet_00000007_0:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000007
    .long 0
codeSnippet_00000007_1:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000007
    .long 1
codeSnippet_00000007_2:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000007
    .long 2
codeSnippet_00000007_3:
    adr x15, .+8
    b _privateSnippetExecutor
    .long 0x000007
    .long 3
    .globl _nFunIndexes
_nFunIndexes:
    .long 8
    .globl _nVtableOffsets
_nVtableOffsets:
    .long 4
    .globl _codeSnippets
_codeSnippets:
    .long codeSnippet_00000000_0 - _codeSnippets
    .long codeSnippet_00000000_1 - _codeSnippets
    .long codeSnippet_00000000_2 - _codeSnippets
    .long codeSnippet_00000000_3 - _codeSnippets
    .long codeSnippet_00000001_0 - _codeSnippets
    .long codeSnippet_00000001_1 - _codeSnippets
    .long codeSnippet_00000001_2 - _codeSnippets
    .long codeSnippet_00000001_3 - _codeSnippets
    .long codeSnippet_00000002_0 - _codeSnippets
    .long codeSnippet_00000002_1 - _codeSnippets
    .long codeSnippet_00000002_2 - _codeSnippets
    .long codeSnippet_00000002_3 - _codeSnippets
    .long codeSnippet_00000003_0 - _codeSnippets
    .long codeSnippet_00000003_1 - _codeSnippets
    .long codeSnippet_00000003_2 - _codeSnippets
    .long codeSnippet_00000003_3 - _codeSnippets
    .long codeSnippet_00000004_0 - _codeSnippets
    .long codeSnippet_00000004_1 - _codeSnippets
    .long codeSnippet_00000004_2 - _codeSnippets
    .long codeSnippet_00000004_3 - _codeSnippets
    .long codeSnippet_00000005_0 - _codeSnippets
    .long codeSnippet_00000005_1 - _codeSnippets
    .long codeSnippet_00000005_2 - _codeSnippets
    .long codeSnippet_00000005_3 - _codeSnippets
    .long codeSnippet_00000006_0 - _codeSnippets
    .long codeSnippet_00000006_1 - _codeSnippets
    .long codeSnippet_00000006_2 - _codeSnippets
    .long codeSnippet_00000006_3 - _codeSnippets
    .long codeSnippet_00000007_0 - _codeSnippets
    .long codeSnippet_00000007_1 - _codeSnippets
    .long codeSnippet_00000007_2 - _codeSnippets
    .long codeSnippet_00000007_3 - _codeSnippets



    .text
    .align 4

_privateSnippetExecutor:

    .cfi_startproc
    // _privateSnippetExecutor is jumped to from each of the
    // codeSnippet_*  generated by generate-snippets.pl

    // Store potential args in general purpose registers
    stp     x6, x7, [sp, #-16]!
    stp     x4, x5, [sp, #-16]!
    stp     x2, x3, [sp, #-16]!
    stp     x0, x1, [sp, #-16]!

    // Store potential args in floating point/SIMD registers
    stp     d6, d7, [sp, #-16]!
    stp     d4, d5, [sp, #-16]!
    stp     d2, d3, [sp, #-16]!
    stp     d0, d1, [sp, #-16]!

    // First argument to cpp_vtable_call: The x15 set up in the codeSnippet instance
    mov     x0, x15
    // Store x8 (potential pointer to return value storage) and lr
    stp     x8, lr, [sp, #-16]!
    // Second argument: The pointer to all the above
    mov     x1, sp

    bl      _cpp_vtable_call

    ldp     x8, lr, [sp, #0]
    add     sp, sp, #144
    ret     lr
    .cfi_endproc

// vim:set shiftwidth=4 softtabstop=4 expandtab:
