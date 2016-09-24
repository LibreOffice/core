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

.global doFlushCode
doFlushCode:
.L:     flush %o0
        deccc %o1
        bne .L
        add %o0, 8, %o0
        retl
        nop
.size doFlushCode,(.-doFlushCode)
.align 8

// %{o->i}0: index
// %{o->i}1: pCallStack = %{s->f}p+2047
// %{o->i}2: vtableOffset
// %{o->i}3: cpp_vtable_call
// %{o->i}4: frameSize (negative)
// [%{s->f}p+2047+128]: param 0 ...
        .file "call.s"
        .text
        .align 4
        .global privateSnippetExecutor
        .type privateSnippetExecutor, #function
privateSnippetExecutor:
.LFB0:
        //// Already done by codeSnippet
        .cfi_startproc
        save %sp, -176, %sp
        .cfi_window_save
        // Register  8 (%o0) saved to register 24 (%i0)
        .cfi_register  8, 24
        // Register  9 (%o1) saved to register 25 (%i1)
        .cfi_register  9, 25
        // Register 10 (%o2) saved to register 26 (%i2)
        .cfi_register 10, 26
        // Register 11 (%o3) saved to register 27 (%i3)
        .cfi_register 11, 27
        // Register 12 (%o4) saved to register 28 (%i4)
        .cfi_register 12, 28
        // Register 15 (%o7) saved to register 31 (%i7)
        .cfi_register 15, 31
        // Use register 30 (%i6 - saved stack pointer) for Call Frame Address
        .cfi_def_cfa_register 30
        mov %i0, %o0
        mov %i1, %o1
        mov %i2, %o2
        jmpl %i3, %o7
        nop
        mov %o0, %i0
        mov %o1, %i1
        mov %o2, %i2
        mov %o3, %i3
        ret
        restore
        .cfi_endproc
.LFE0:
        .size privateSnippetExecutor,(.-privateSnippetExecutor)
        .section .note.GNU-stack,"",@progbits

.align 8
