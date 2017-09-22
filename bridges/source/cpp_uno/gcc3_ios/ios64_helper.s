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

#ifdef __arm64

.text
#if defined(__arm) || defined(__arm64)

// Each codeSnippetX function stores into ip (arm64: x15) an address and branches to _privateSnippetExecutor
// The address is that following the branch instruction, containing two 32-bit ints:
// - the function index, which for 32-bit can have the 0x80000000 bit set
//   to indicate that a hidden parameter is used for returning large values
// - the vtable offset

    .align 4

codeSnippet_00000000_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 00000000
    .long 0
#ifndef __arm64
codeSnippet_80000000_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000000
    .long 0
#endif
codeSnippet_00000000_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 00000000
    .long 1
#ifndef __arm64
codeSnippet_80000000_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000000
    .long 1
#endif
codeSnippet_00000000_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 00000000
    .long 2
#ifndef __arm64
codeSnippet_80000000_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000000
    .long 2
#endif
codeSnippet_00000000_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 00000000
    .long 3
#ifndef __arm64
codeSnippet_80000000_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000000
    .long 3
#endif
codeSnippet_00000001_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000001
    .long 0
#ifndef __arm64
codeSnippet_80000001_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000001
    .long 0
#endif
codeSnippet_00000001_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000001
    .long 1
#ifndef __arm64
codeSnippet_80000001_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000001
    .long 1
#endif
codeSnippet_00000001_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000001
    .long 2
#ifndef __arm64
codeSnippet_80000001_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000001
    .long 2
#endif
codeSnippet_00000001_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000001
    .long 3
#ifndef __arm64
codeSnippet_80000001_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000001
    .long 3
#endif
codeSnippet_00000002_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000002
    .long 0
#ifndef __arm64
codeSnippet_80000002_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000002
    .long 0
#endif
codeSnippet_00000002_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000002
    .long 1
#ifndef __arm64
codeSnippet_80000002_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000002
    .long 1
#endif
codeSnippet_00000002_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000002
    .long 2
#ifndef __arm64
codeSnippet_80000002_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000002
    .long 2
#endif
codeSnippet_00000002_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000002
    .long 3
#ifndef __arm64
codeSnippet_80000002_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000002
    .long 3
#endif
codeSnippet_00000003_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000003
    .long 0
#ifndef __arm64
codeSnippet_80000003_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000003
    .long 0
#endif
codeSnippet_00000003_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000003
    .long 1
#ifndef __arm64
codeSnippet_80000003_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000003
    .long 1
#endif
codeSnippet_00000003_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000003
    .long 2
#ifndef __arm64
codeSnippet_80000003_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000003
    .long 2
#endif
codeSnippet_00000003_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000003
    .long 3
#ifndef __arm64
codeSnippet_80000003_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000003
    .long 3
#endif
codeSnippet_00000004_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000004
    .long 0
#ifndef __arm64
codeSnippet_80000004_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000004
    .long 0
#endif
codeSnippet_00000004_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000004
    .long 1
#ifndef __arm64
codeSnippet_80000004_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000004
    .long 1
#endif
codeSnippet_00000004_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000004
    .long 2
#ifndef __arm64
codeSnippet_80000004_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000004
    .long 2
#endif
codeSnippet_00000004_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000004
    .long 3
#ifndef __arm64
codeSnippet_80000004_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000004
    .long 3
#endif
codeSnippet_00000005_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000005
    .long 0
#ifndef __arm64
codeSnippet_80000005_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000005
    .long 0
#endif
codeSnippet_00000005_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000005
    .long 1
#ifndef __arm64
codeSnippet_80000005_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000005
    .long 1
#endif
codeSnippet_00000005_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000005
    .long 2
#ifndef __arm64
codeSnippet_80000005_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000005
    .long 2
#endif
codeSnippet_00000005_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000005
    .long 3
#ifndef __arm64
codeSnippet_80000005_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000005
    .long 3
#endif
codeSnippet_00000006_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000006
    .long 0
#ifndef __arm64
codeSnippet_80000006_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000006
    .long 0
#endif
codeSnippet_00000006_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000006
    .long 1
#ifndef __arm64
codeSnippet_80000006_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000006
    .long 1
#endif
codeSnippet_00000006_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000006
    .long 2
#ifndef __arm64
codeSnippet_80000006_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000006
    .long 2
#endif
codeSnippet_00000006_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000006
    .long 3
#ifndef __arm64
codeSnippet_80000006_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000006
    .long 3
#endif
codeSnippet_00000007_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000007
    .long 0
#ifndef __arm64
codeSnippet_80000007_0:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000007
    .long 0
#endif
codeSnippet_00000007_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000007
    .long 1
#ifndef __arm64
codeSnippet_80000007_1:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000007
    .long 1
#endif
codeSnippet_00000007_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000007
    .long 2
#ifndef __arm64
codeSnippet_80000007_2:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000007
    .long 2
#endif
codeSnippet_00000007_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x000007
    .long 3
#ifndef __arm64
codeSnippet_80000007_3:
#ifdef __arm
    mov ip, pc
#else
    adr x15, .+8
#endif
    b _privateSnippetExecutor
    .long 0x80000007
    .long 3
#endif
#else
    .align 1, 0x90
codeSnippet_00000000_0_General:
    movl $00000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000000_0_General:
    movl $0x80000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000000_0_Void:
    movl $00000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000000_0_Void:
    movl $0x80000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000000_0_Hyper:
    movl $00000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000000_0_Hyper:
    movl $0x80000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000000_0_Float:
    movl $00000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000000_0_Float:
    movl $0x80000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000000_0_Double:
    movl $00000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000000_0_Double:
    movl $0x80000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000000_0_Class:
    movl $00000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000000_0_Class:
    movl $0x80000000, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000000_1_General:
    movl $00000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000000_1_General:
    movl $0x80000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000000_1_Void:
    movl $00000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000000_1_Void:
    movl $0x80000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000000_1_Hyper:
    movl $00000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000000_1_Hyper:
    movl $0x80000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000000_1_Float:
    movl $00000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000000_1_Float:
    movl $0x80000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000000_1_Double:
    movl $00000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000000_1_Double:
    movl $0x80000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000000_1_Class:
    movl $00000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000000_1_Class:
    movl $0x80000000, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000000_2_General:
    movl $00000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000000_2_General:
    movl $0x80000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000000_2_Void:
    movl $00000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000000_2_Void:
    movl $0x80000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000000_2_Hyper:
    movl $00000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000000_2_Hyper:
    movl $0x80000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000000_2_Float:
    movl $00000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000000_2_Float:
    movl $0x80000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000000_2_Double:
    movl $00000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000000_2_Double:
    movl $0x80000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000000_2_Class:
    movl $00000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000000_2_Class:
    movl $0x80000000, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000000_3_General:
    movl $00000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000000_3_General:
    movl $0x80000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000000_3_Void:
    movl $00000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000000_3_Void:
    movl $0x80000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000000_3_Hyper:
    movl $00000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000000_3_Hyper:
    movl $0x80000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000000_3_Float:
    movl $00000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000000_3_Float:
    movl $0x80000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000000_3_Double:
    movl $00000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000000_3_Double:
    movl $0x80000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000000_3_Class:
    movl $00000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000000_3_Class:
    movl $0x80000000, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000001_0_General:
    movl $0x000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000001_0_General:
    movl $0x80000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000001_0_Void:
    movl $0x000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000001_0_Void:
    movl $0x80000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000001_0_Hyper:
    movl $0x000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000001_0_Hyper:
    movl $0x80000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000001_0_Float:
    movl $0x000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000001_0_Float:
    movl $0x80000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000001_0_Double:
    movl $0x000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000001_0_Double:
    movl $0x80000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000001_0_Class:
    movl $0x000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000001_0_Class:
    movl $0x80000001, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000001_1_General:
    movl $0x000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000001_1_General:
    movl $0x80000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000001_1_Void:
    movl $0x000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000001_1_Void:
    movl $0x80000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000001_1_Hyper:
    movl $0x000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000001_1_Hyper:
    movl $0x80000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000001_1_Float:
    movl $0x000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000001_1_Float:
    movl $0x80000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000001_1_Double:
    movl $0x000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000001_1_Double:
    movl $0x80000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000001_1_Class:
    movl $0x000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000001_1_Class:
    movl $0x80000001, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000001_2_General:
    movl $0x000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000001_2_General:
    movl $0x80000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000001_2_Void:
    movl $0x000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000001_2_Void:
    movl $0x80000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000001_2_Hyper:
    movl $0x000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000001_2_Hyper:
    movl $0x80000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000001_2_Float:
    movl $0x000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000001_2_Float:
    movl $0x80000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000001_2_Double:
    movl $0x000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000001_2_Double:
    movl $0x80000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000001_2_Class:
    movl $0x000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000001_2_Class:
    movl $0x80000001, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000001_3_General:
    movl $0x000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000001_3_General:
    movl $0x80000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000001_3_Void:
    movl $0x000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000001_3_Void:
    movl $0x80000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000001_3_Hyper:
    movl $0x000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000001_3_Hyper:
    movl $0x80000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000001_3_Float:
    movl $0x000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000001_3_Float:
    movl $0x80000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000001_3_Double:
    movl $0x000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000001_3_Double:
    movl $0x80000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000001_3_Class:
    movl $0x000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000001_3_Class:
    movl $0x80000001, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000002_0_General:
    movl $0x000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000002_0_General:
    movl $0x80000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000002_0_Void:
    movl $0x000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000002_0_Void:
    movl $0x80000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000002_0_Hyper:
    movl $0x000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000002_0_Hyper:
    movl $0x80000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000002_0_Float:
    movl $0x000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000002_0_Float:
    movl $0x80000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000002_0_Double:
    movl $0x000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000002_0_Double:
    movl $0x80000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000002_0_Class:
    movl $0x000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000002_0_Class:
    movl $0x80000002, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000002_1_General:
    movl $0x000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000002_1_General:
    movl $0x80000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000002_1_Void:
    movl $0x000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000002_1_Void:
    movl $0x80000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000002_1_Hyper:
    movl $0x000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000002_1_Hyper:
    movl $0x80000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000002_1_Float:
    movl $0x000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000002_1_Float:
    movl $0x80000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000002_1_Double:
    movl $0x000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000002_1_Double:
    movl $0x80000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000002_1_Class:
    movl $0x000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000002_1_Class:
    movl $0x80000002, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000002_2_General:
    movl $0x000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000002_2_General:
    movl $0x80000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000002_2_Void:
    movl $0x000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000002_2_Void:
    movl $0x80000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000002_2_Hyper:
    movl $0x000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000002_2_Hyper:
    movl $0x80000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000002_2_Float:
    movl $0x000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000002_2_Float:
    movl $0x80000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000002_2_Double:
    movl $0x000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000002_2_Double:
    movl $0x80000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000002_2_Class:
    movl $0x000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000002_2_Class:
    movl $0x80000002, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000002_3_General:
    movl $0x000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000002_3_General:
    movl $0x80000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000002_3_Void:
    movl $0x000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000002_3_Void:
    movl $0x80000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000002_3_Hyper:
    movl $0x000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000002_3_Hyper:
    movl $0x80000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000002_3_Float:
    movl $0x000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000002_3_Float:
    movl $0x80000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000002_3_Double:
    movl $0x000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000002_3_Double:
    movl $0x80000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000002_3_Class:
    movl $0x000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000002_3_Class:
    movl $0x80000002, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000003_0_General:
    movl $0x000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000003_0_General:
    movl $0x80000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000003_0_Void:
    movl $0x000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000003_0_Void:
    movl $0x80000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000003_0_Hyper:
    movl $0x000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000003_0_Hyper:
    movl $0x80000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000003_0_Float:
    movl $0x000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000003_0_Float:
    movl $0x80000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000003_0_Double:
    movl $0x000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000003_0_Double:
    movl $0x80000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000003_0_Class:
    movl $0x000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000003_0_Class:
    movl $0x80000003, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000003_1_General:
    movl $0x000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000003_1_General:
    movl $0x80000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000003_1_Void:
    movl $0x000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000003_1_Void:
    movl $0x80000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000003_1_Hyper:
    movl $0x000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000003_1_Hyper:
    movl $0x80000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000003_1_Float:
    movl $0x000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000003_1_Float:
    movl $0x80000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000003_1_Double:
    movl $0x000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000003_1_Double:
    movl $0x80000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000003_1_Class:
    movl $0x000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000003_1_Class:
    movl $0x80000003, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000003_2_General:
    movl $0x000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000003_2_General:
    movl $0x80000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000003_2_Void:
    movl $0x000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000003_2_Void:
    movl $0x80000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000003_2_Hyper:
    movl $0x000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000003_2_Hyper:
    movl $0x80000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000003_2_Float:
    movl $0x000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000003_2_Float:
    movl $0x80000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000003_2_Double:
    movl $0x000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000003_2_Double:
    movl $0x80000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000003_2_Class:
    movl $0x000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000003_2_Class:
    movl $0x80000003, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000003_3_General:
    movl $0x000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000003_3_General:
    movl $0x80000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000003_3_Void:
    movl $0x000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000003_3_Void:
    movl $0x80000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000003_3_Hyper:
    movl $0x000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000003_3_Hyper:
    movl $0x80000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000003_3_Float:
    movl $0x000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000003_3_Float:
    movl $0x80000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000003_3_Double:
    movl $0x000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000003_3_Double:
    movl $0x80000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000003_3_Class:
    movl $0x000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000003_3_Class:
    movl $0x80000003, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000004_0_General:
    movl $0x000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000004_0_General:
    movl $0x80000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000004_0_Void:
    movl $0x000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000004_0_Void:
    movl $0x80000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000004_0_Hyper:
    movl $0x000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000004_0_Hyper:
    movl $0x80000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000004_0_Float:
    movl $0x000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000004_0_Float:
    movl $0x80000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000004_0_Double:
    movl $0x000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000004_0_Double:
    movl $0x80000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000004_0_Class:
    movl $0x000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000004_0_Class:
    movl $0x80000004, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000004_1_General:
    movl $0x000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000004_1_General:
    movl $0x80000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000004_1_Void:
    movl $0x000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000004_1_Void:
    movl $0x80000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000004_1_Hyper:
    movl $0x000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000004_1_Hyper:
    movl $0x80000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000004_1_Float:
    movl $0x000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000004_1_Float:
    movl $0x80000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000004_1_Double:
    movl $0x000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000004_1_Double:
    movl $0x80000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000004_1_Class:
    movl $0x000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000004_1_Class:
    movl $0x80000004, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000004_2_General:
    movl $0x000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000004_2_General:
    movl $0x80000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000004_2_Void:
    movl $0x000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000004_2_Void:
    movl $0x80000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000004_2_Hyper:
    movl $0x000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000004_2_Hyper:
    movl $0x80000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000004_2_Float:
    movl $0x000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000004_2_Float:
    movl $0x80000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000004_2_Double:
    movl $0x000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000004_2_Double:
    movl $0x80000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000004_2_Class:
    movl $0x000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000004_2_Class:
    movl $0x80000004, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000004_3_General:
    movl $0x000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000004_3_General:
    movl $0x80000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000004_3_Void:
    movl $0x000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000004_3_Void:
    movl $0x80000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000004_3_Hyper:
    movl $0x000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000004_3_Hyper:
    movl $0x80000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000004_3_Float:
    movl $0x000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000004_3_Float:
    movl $0x80000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000004_3_Double:
    movl $0x000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000004_3_Double:
    movl $0x80000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000004_3_Class:
    movl $0x000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000004_3_Class:
    movl $0x80000004, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000005_0_General:
    movl $0x000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000005_0_General:
    movl $0x80000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000005_0_Void:
    movl $0x000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000005_0_Void:
    movl $0x80000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000005_0_Hyper:
    movl $0x000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000005_0_Hyper:
    movl $0x80000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000005_0_Float:
    movl $0x000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000005_0_Float:
    movl $0x80000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000005_0_Double:
    movl $0x000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000005_0_Double:
    movl $0x80000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000005_0_Class:
    movl $0x000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000005_0_Class:
    movl $0x80000005, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000005_1_General:
    movl $0x000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000005_1_General:
    movl $0x80000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000005_1_Void:
    movl $0x000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000005_1_Void:
    movl $0x80000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000005_1_Hyper:
    movl $0x000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000005_1_Hyper:
    movl $0x80000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000005_1_Float:
    movl $0x000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000005_1_Float:
    movl $0x80000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000005_1_Double:
    movl $0x000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000005_1_Double:
    movl $0x80000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000005_1_Class:
    movl $0x000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000005_1_Class:
    movl $0x80000005, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000005_2_General:
    movl $0x000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000005_2_General:
    movl $0x80000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000005_2_Void:
    movl $0x000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000005_2_Void:
    movl $0x80000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000005_2_Hyper:
    movl $0x000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000005_2_Hyper:
    movl $0x80000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000005_2_Float:
    movl $0x000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000005_2_Float:
    movl $0x80000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000005_2_Double:
    movl $0x000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000005_2_Double:
    movl $0x80000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000005_2_Class:
    movl $0x000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000005_2_Class:
    movl $0x80000005, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000005_3_General:
    movl $0x000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000005_3_General:
    movl $0x80000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000005_3_Void:
    movl $0x000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000005_3_Void:
    movl $0x80000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000005_3_Hyper:
    movl $0x000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000005_3_Hyper:
    movl $0x80000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000005_3_Float:
    movl $0x000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000005_3_Float:
    movl $0x80000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000005_3_Double:
    movl $0x000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000005_3_Double:
    movl $0x80000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000005_3_Class:
    movl $0x000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000005_3_Class:
    movl $0x80000005, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000006_0_General:
    movl $0x000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000006_0_General:
    movl $0x80000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000006_0_Void:
    movl $0x000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000006_0_Void:
    movl $0x80000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000006_0_Hyper:
    movl $0x000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000006_0_Hyper:
    movl $0x80000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000006_0_Float:
    movl $0x000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000006_0_Float:
    movl $0x80000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000006_0_Double:
    movl $0x000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000006_0_Double:
    movl $0x80000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000006_0_Class:
    movl $0x000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000006_0_Class:
    movl $0x80000006, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000006_1_General:
    movl $0x000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000006_1_General:
    movl $0x80000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000006_1_Void:
    movl $0x000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000006_1_Void:
    movl $0x80000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000006_1_Hyper:
    movl $0x000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000006_1_Hyper:
    movl $0x80000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000006_1_Float:
    movl $0x000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000006_1_Float:
    movl $0x80000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000006_1_Double:
    movl $0x000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000006_1_Double:
    movl $0x80000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000006_1_Class:
    movl $0x000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000006_1_Class:
    movl $0x80000006, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000006_2_General:
    movl $0x000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000006_2_General:
    movl $0x80000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000006_2_Void:
    movl $0x000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000006_2_Void:
    movl $0x80000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000006_2_Hyper:
    movl $0x000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000006_2_Hyper:
    movl $0x80000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000006_2_Float:
    movl $0x000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000006_2_Float:
    movl $0x80000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000006_2_Double:
    movl $0x000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000006_2_Double:
    movl $0x80000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000006_2_Class:
    movl $0x000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000006_2_Class:
    movl $0x80000006, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000006_3_General:
    movl $0x000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000006_3_General:
    movl $0x80000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000006_3_Void:
    movl $0x000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000006_3_Void:
    movl $0x80000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000006_3_Hyper:
    movl $0x000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000006_3_Hyper:
    movl $0x80000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000006_3_Float:
    movl $0x000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000006_3_Float:
    movl $0x80000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000006_3_Double:
    movl $0x000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000006_3_Double:
    movl $0x80000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000006_3_Class:
    movl $0x000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000006_3_Class:
    movl $0x80000006, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000007_0_General:
    movl $0x000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000007_0_General:
    movl $0x80000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000007_0_Void:
    movl $0x000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000007_0_Void:
    movl $0x80000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000007_0_Hyper:
    movl $0x000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000007_0_Hyper:
    movl $0x80000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000007_0_Float:
    movl $0x000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000007_0_Float:
    movl $0x80000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000007_0_Double:
    movl $0x000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000007_0_Double:
    movl $0x80000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000007_0_Class:
    movl $0x000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000007_0_Class:
    movl $0x80000007, %eax
    movl $0, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000007_1_General:
    movl $0x000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000007_1_General:
    movl $0x80000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000007_1_Void:
    movl $0x000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000007_1_Void:
    movl $0x80000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000007_1_Hyper:
    movl $0x000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000007_1_Hyper:
    movl $0x80000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000007_1_Float:
    movl $0x000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000007_1_Float:
    movl $0x80000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000007_1_Double:
    movl $0x000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000007_1_Double:
    movl $0x80000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000007_1_Class:
    movl $0x000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000007_1_Class:
    movl $0x80000007, %eax
    movl $1, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000007_2_General:
    movl $0x000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000007_2_General:
    movl $0x80000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000007_2_Void:
    movl $0x000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000007_2_Void:
    movl $0x80000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000007_2_Hyper:
    movl $0x000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000007_2_Hyper:
    movl $0x80000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000007_2_Float:
    movl $0x000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000007_2_Float:
    movl $0x80000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000007_2_Double:
    movl $0x000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000007_2_Double:
    movl $0x80000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000007_2_Class:
    movl $0x000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000007_2_Class:
    movl $0x80000007, %eax
    movl $2, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_00000007_3_General:
    movl $0x000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_80000007_3_General:
    movl $0x80000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorGeneral
codeSnippet_00000007_3_Void:
    movl $0x000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_80000007_3_Void:
    movl $0x80000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorVoid
codeSnippet_00000007_3_Hyper:
    movl $0x000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_80000007_3_Hyper:
    movl $0x80000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorHyper
codeSnippet_00000007_3_Float:
    movl $0x000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_80000007_3_Float:
    movl $0x80000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorFloat
codeSnippet_00000007_3_Double:
    movl $0x000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_80000007_3_Double:
    movl $0x80000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorDouble
codeSnippet_00000007_3_Class:
    movl $0x000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
codeSnippet_80000007_3_Class:
    movl $0x80000007, %eax
    movl $3, %edx
    jmp _privateSnippetExecutorClass
#endif
    .globl _nFunIndexes
_nFunIndexes:
    .long 8
    .globl _nVtableOffsets
_nVtableOffsets:
    .long 4
    .globl _codeSnippets
_codeSnippets:
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000000_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000000_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000000_0_General - _codeSnippets
    .long codeSnippet_80000000_0_General - _codeSnippets
    .long codeSnippet_00000000_0_Void - _codeSnippets
    .long codeSnippet_80000000_0_Void - _codeSnippets
    .long codeSnippet_00000000_0_Hyper - _codeSnippets
    .long codeSnippet_80000000_0_Hyper - _codeSnippets
    .long codeSnippet_00000000_0_Float - _codeSnippets
    .long codeSnippet_80000000_0_Float - _codeSnippets
    .long codeSnippet_00000000_0_Double - _codeSnippets
    .long codeSnippet_80000000_0_Double - _codeSnippets
    .long codeSnippet_00000000_0_Class - _codeSnippets
    .long codeSnippet_80000000_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000000_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000000_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000000_1_General - _codeSnippets
    .long codeSnippet_80000000_1_General - _codeSnippets
    .long codeSnippet_00000000_1_Void - _codeSnippets
    .long codeSnippet_80000000_1_Void - _codeSnippets
    .long codeSnippet_00000000_1_Hyper - _codeSnippets
    .long codeSnippet_80000000_1_Hyper - _codeSnippets
    .long codeSnippet_00000000_1_Float - _codeSnippets
    .long codeSnippet_80000000_1_Float - _codeSnippets
    .long codeSnippet_00000000_1_Double - _codeSnippets
    .long codeSnippet_80000000_1_Double - _codeSnippets
    .long codeSnippet_00000000_1_Class - _codeSnippets
    .long codeSnippet_80000000_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000000_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000000_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000000_2_General - _codeSnippets
    .long codeSnippet_80000000_2_General - _codeSnippets
    .long codeSnippet_00000000_2_Void - _codeSnippets
    .long codeSnippet_80000000_2_Void - _codeSnippets
    .long codeSnippet_00000000_2_Hyper - _codeSnippets
    .long codeSnippet_80000000_2_Hyper - _codeSnippets
    .long codeSnippet_00000000_2_Float - _codeSnippets
    .long codeSnippet_80000000_2_Float - _codeSnippets
    .long codeSnippet_00000000_2_Double - _codeSnippets
    .long codeSnippet_80000000_2_Double - _codeSnippets
    .long codeSnippet_00000000_2_Class - _codeSnippets
    .long codeSnippet_80000000_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000000_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000000_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000000_3_General - _codeSnippets
    .long codeSnippet_80000000_3_General - _codeSnippets
    .long codeSnippet_00000000_3_Void - _codeSnippets
    .long codeSnippet_80000000_3_Void - _codeSnippets
    .long codeSnippet_00000000_3_Hyper - _codeSnippets
    .long codeSnippet_80000000_3_Hyper - _codeSnippets
    .long codeSnippet_00000000_3_Float - _codeSnippets
    .long codeSnippet_80000000_3_Float - _codeSnippets
    .long codeSnippet_00000000_3_Double - _codeSnippets
    .long codeSnippet_80000000_3_Double - _codeSnippets
    .long codeSnippet_00000000_3_Class - _codeSnippets
    .long codeSnippet_80000000_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000001_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000001_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000001_0_General - _codeSnippets
    .long codeSnippet_80000001_0_General - _codeSnippets
    .long codeSnippet_00000001_0_Void - _codeSnippets
    .long codeSnippet_80000001_0_Void - _codeSnippets
    .long codeSnippet_00000001_0_Hyper - _codeSnippets
    .long codeSnippet_80000001_0_Hyper - _codeSnippets
    .long codeSnippet_00000001_0_Float - _codeSnippets
    .long codeSnippet_80000001_0_Float - _codeSnippets
    .long codeSnippet_00000001_0_Double - _codeSnippets
    .long codeSnippet_80000001_0_Double - _codeSnippets
    .long codeSnippet_00000001_0_Class - _codeSnippets
    .long codeSnippet_80000001_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000001_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000001_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000001_1_General - _codeSnippets
    .long codeSnippet_80000001_1_General - _codeSnippets
    .long codeSnippet_00000001_1_Void - _codeSnippets
    .long codeSnippet_80000001_1_Void - _codeSnippets
    .long codeSnippet_00000001_1_Hyper - _codeSnippets
    .long codeSnippet_80000001_1_Hyper - _codeSnippets
    .long codeSnippet_00000001_1_Float - _codeSnippets
    .long codeSnippet_80000001_1_Float - _codeSnippets
    .long codeSnippet_00000001_1_Double - _codeSnippets
    .long codeSnippet_80000001_1_Double - _codeSnippets
    .long codeSnippet_00000001_1_Class - _codeSnippets
    .long codeSnippet_80000001_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000001_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000001_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000001_2_General - _codeSnippets
    .long codeSnippet_80000001_2_General - _codeSnippets
    .long codeSnippet_00000001_2_Void - _codeSnippets
    .long codeSnippet_80000001_2_Void - _codeSnippets
    .long codeSnippet_00000001_2_Hyper - _codeSnippets
    .long codeSnippet_80000001_2_Hyper - _codeSnippets
    .long codeSnippet_00000001_2_Float - _codeSnippets
    .long codeSnippet_80000001_2_Float - _codeSnippets
    .long codeSnippet_00000001_2_Double - _codeSnippets
    .long codeSnippet_80000001_2_Double - _codeSnippets
    .long codeSnippet_00000001_2_Class - _codeSnippets
    .long codeSnippet_80000001_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000001_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000001_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000001_3_General - _codeSnippets
    .long codeSnippet_80000001_3_General - _codeSnippets
    .long codeSnippet_00000001_3_Void - _codeSnippets
    .long codeSnippet_80000001_3_Void - _codeSnippets
    .long codeSnippet_00000001_3_Hyper - _codeSnippets
    .long codeSnippet_80000001_3_Hyper - _codeSnippets
    .long codeSnippet_00000001_3_Float - _codeSnippets
    .long codeSnippet_80000001_3_Float - _codeSnippets
    .long codeSnippet_00000001_3_Double - _codeSnippets
    .long codeSnippet_80000001_3_Double - _codeSnippets
    .long codeSnippet_00000001_3_Class - _codeSnippets
    .long codeSnippet_80000001_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000002_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000002_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000002_0_General - _codeSnippets
    .long codeSnippet_80000002_0_General - _codeSnippets
    .long codeSnippet_00000002_0_Void - _codeSnippets
    .long codeSnippet_80000002_0_Void - _codeSnippets
    .long codeSnippet_00000002_0_Hyper - _codeSnippets
    .long codeSnippet_80000002_0_Hyper - _codeSnippets
    .long codeSnippet_00000002_0_Float - _codeSnippets
    .long codeSnippet_80000002_0_Float - _codeSnippets
    .long codeSnippet_00000002_0_Double - _codeSnippets
    .long codeSnippet_80000002_0_Double - _codeSnippets
    .long codeSnippet_00000002_0_Class - _codeSnippets
    .long codeSnippet_80000002_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000002_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000002_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000002_1_General - _codeSnippets
    .long codeSnippet_80000002_1_General - _codeSnippets
    .long codeSnippet_00000002_1_Void - _codeSnippets
    .long codeSnippet_80000002_1_Void - _codeSnippets
    .long codeSnippet_00000002_1_Hyper - _codeSnippets
    .long codeSnippet_80000002_1_Hyper - _codeSnippets
    .long codeSnippet_00000002_1_Float - _codeSnippets
    .long codeSnippet_80000002_1_Float - _codeSnippets
    .long codeSnippet_00000002_1_Double - _codeSnippets
    .long codeSnippet_80000002_1_Double - _codeSnippets
    .long codeSnippet_00000002_1_Class - _codeSnippets
    .long codeSnippet_80000002_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000002_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000002_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000002_2_General - _codeSnippets
    .long codeSnippet_80000002_2_General - _codeSnippets
    .long codeSnippet_00000002_2_Void - _codeSnippets
    .long codeSnippet_80000002_2_Void - _codeSnippets
    .long codeSnippet_00000002_2_Hyper - _codeSnippets
    .long codeSnippet_80000002_2_Hyper - _codeSnippets
    .long codeSnippet_00000002_2_Float - _codeSnippets
    .long codeSnippet_80000002_2_Float - _codeSnippets
    .long codeSnippet_00000002_2_Double - _codeSnippets
    .long codeSnippet_80000002_2_Double - _codeSnippets
    .long codeSnippet_00000002_2_Class - _codeSnippets
    .long codeSnippet_80000002_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000002_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000002_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000002_3_General - _codeSnippets
    .long codeSnippet_80000002_3_General - _codeSnippets
    .long codeSnippet_00000002_3_Void - _codeSnippets
    .long codeSnippet_80000002_3_Void - _codeSnippets
    .long codeSnippet_00000002_3_Hyper - _codeSnippets
    .long codeSnippet_80000002_3_Hyper - _codeSnippets
    .long codeSnippet_00000002_3_Float - _codeSnippets
    .long codeSnippet_80000002_3_Float - _codeSnippets
    .long codeSnippet_00000002_3_Double - _codeSnippets
    .long codeSnippet_80000002_3_Double - _codeSnippets
    .long codeSnippet_00000002_3_Class - _codeSnippets
    .long codeSnippet_80000002_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000003_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000003_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000003_0_General - _codeSnippets
    .long codeSnippet_80000003_0_General - _codeSnippets
    .long codeSnippet_00000003_0_Void - _codeSnippets
    .long codeSnippet_80000003_0_Void - _codeSnippets
    .long codeSnippet_00000003_0_Hyper - _codeSnippets
    .long codeSnippet_80000003_0_Hyper - _codeSnippets
    .long codeSnippet_00000003_0_Float - _codeSnippets
    .long codeSnippet_80000003_0_Float - _codeSnippets
    .long codeSnippet_00000003_0_Double - _codeSnippets
    .long codeSnippet_80000003_0_Double - _codeSnippets
    .long codeSnippet_00000003_0_Class - _codeSnippets
    .long codeSnippet_80000003_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000003_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000003_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000003_1_General - _codeSnippets
    .long codeSnippet_80000003_1_General - _codeSnippets
    .long codeSnippet_00000003_1_Void - _codeSnippets
    .long codeSnippet_80000003_1_Void - _codeSnippets
    .long codeSnippet_00000003_1_Hyper - _codeSnippets
    .long codeSnippet_80000003_1_Hyper - _codeSnippets
    .long codeSnippet_00000003_1_Float - _codeSnippets
    .long codeSnippet_80000003_1_Float - _codeSnippets
    .long codeSnippet_00000003_1_Double - _codeSnippets
    .long codeSnippet_80000003_1_Double - _codeSnippets
    .long codeSnippet_00000003_1_Class - _codeSnippets
    .long codeSnippet_80000003_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000003_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000003_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000003_2_General - _codeSnippets
    .long codeSnippet_80000003_2_General - _codeSnippets
    .long codeSnippet_00000003_2_Void - _codeSnippets
    .long codeSnippet_80000003_2_Void - _codeSnippets
    .long codeSnippet_00000003_2_Hyper - _codeSnippets
    .long codeSnippet_80000003_2_Hyper - _codeSnippets
    .long codeSnippet_00000003_2_Float - _codeSnippets
    .long codeSnippet_80000003_2_Float - _codeSnippets
    .long codeSnippet_00000003_2_Double - _codeSnippets
    .long codeSnippet_80000003_2_Double - _codeSnippets
    .long codeSnippet_00000003_2_Class - _codeSnippets
    .long codeSnippet_80000003_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000003_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000003_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000003_3_General - _codeSnippets
    .long codeSnippet_80000003_3_General - _codeSnippets
    .long codeSnippet_00000003_3_Void - _codeSnippets
    .long codeSnippet_80000003_3_Void - _codeSnippets
    .long codeSnippet_00000003_3_Hyper - _codeSnippets
    .long codeSnippet_80000003_3_Hyper - _codeSnippets
    .long codeSnippet_00000003_3_Float - _codeSnippets
    .long codeSnippet_80000003_3_Float - _codeSnippets
    .long codeSnippet_00000003_3_Double - _codeSnippets
    .long codeSnippet_80000003_3_Double - _codeSnippets
    .long codeSnippet_00000003_3_Class - _codeSnippets
    .long codeSnippet_80000003_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000004_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000004_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000004_0_General - _codeSnippets
    .long codeSnippet_80000004_0_General - _codeSnippets
    .long codeSnippet_00000004_0_Void - _codeSnippets
    .long codeSnippet_80000004_0_Void - _codeSnippets
    .long codeSnippet_00000004_0_Hyper - _codeSnippets
    .long codeSnippet_80000004_0_Hyper - _codeSnippets
    .long codeSnippet_00000004_0_Float - _codeSnippets
    .long codeSnippet_80000004_0_Float - _codeSnippets
    .long codeSnippet_00000004_0_Double - _codeSnippets
    .long codeSnippet_80000004_0_Double - _codeSnippets
    .long codeSnippet_00000004_0_Class - _codeSnippets
    .long codeSnippet_80000004_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000004_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000004_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000004_1_General - _codeSnippets
    .long codeSnippet_80000004_1_General - _codeSnippets
    .long codeSnippet_00000004_1_Void - _codeSnippets
    .long codeSnippet_80000004_1_Void - _codeSnippets
    .long codeSnippet_00000004_1_Hyper - _codeSnippets
    .long codeSnippet_80000004_1_Hyper - _codeSnippets
    .long codeSnippet_00000004_1_Float - _codeSnippets
    .long codeSnippet_80000004_1_Float - _codeSnippets
    .long codeSnippet_00000004_1_Double - _codeSnippets
    .long codeSnippet_80000004_1_Double - _codeSnippets
    .long codeSnippet_00000004_1_Class - _codeSnippets
    .long codeSnippet_80000004_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000004_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000004_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000004_2_General - _codeSnippets
    .long codeSnippet_80000004_2_General - _codeSnippets
    .long codeSnippet_00000004_2_Void - _codeSnippets
    .long codeSnippet_80000004_2_Void - _codeSnippets
    .long codeSnippet_00000004_2_Hyper - _codeSnippets
    .long codeSnippet_80000004_2_Hyper - _codeSnippets
    .long codeSnippet_00000004_2_Float - _codeSnippets
    .long codeSnippet_80000004_2_Float - _codeSnippets
    .long codeSnippet_00000004_2_Double - _codeSnippets
    .long codeSnippet_80000004_2_Double - _codeSnippets
    .long codeSnippet_00000004_2_Class - _codeSnippets
    .long codeSnippet_80000004_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000004_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000004_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000004_3_General - _codeSnippets
    .long codeSnippet_80000004_3_General - _codeSnippets
    .long codeSnippet_00000004_3_Void - _codeSnippets
    .long codeSnippet_80000004_3_Void - _codeSnippets
    .long codeSnippet_00000004_3_Hyper - _codeSnippets
    .long codeSnippet_80000004_3_Hyper - _codeSnippets
    .long codeSnippet_00000004_3_Float - _codeSnippets
    .long codeSnippet_80000004_3_Float - _codeSnippets
    .long codeSnippet_00000004_3_Double - _codeSnippets
    .long codeSnippet_80000004_3_Double - _codeSnippets
    .long codeSnippet_00000004_3_Class - _codeSnippets
    .long codeSnippet_80000004_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000005_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000005_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000005_0_General - _codeSnippets
    .long codeSnippet_80000005_0_General - _codeSnippets
    .long codeSnippet_00000005_0_Void - _codeSnippets
    .long codeSnippet_80000005_0_Void - _codeSnippets
    .long codeSnippet_00000005_0_Hyper - _codeSnippets
    .long codeSnippet_80000005_0_Hyper - _codeSnippets
    .long codeSnippet_00000005_0_Float - _codeSnippets
    .long codeSnippet_80000005_0_Float - _codeSnippets
    .long codeSnippet_00000005_0_Double - _codeSnippets
    .long codeSnippet_80000005_0_Double - _codeSnippets
    .long codeSnippet_00000005_0_Class - _codeSnippets
    .long codeSnippet_80000005_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000005_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000005_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000005_1_General - _codeSnippets
    .long codeSnippet_80000005_1_General - _codeSnippets
    .long codeSnippet_00000005_1_Void - _codeSnippets
    .long codeSnippet_80000005_1_Void - _codeSnippets
    .long codeSnippet_00000005_1_Hyper - _codeSnippets
    .long codeSnippet_80000005_1_Hyper - _codeSnippets
    .long codeSnippet_00000005_1_Float - _codeSnippets
    .long codeSnippet_80000005_1_Float - _codeSnippets
    .long codeSnippet_00000005_1_Double - _codeSnippets
    .long codeSnippet_80000005_1_Double - _codeSnippets
    .long codeSnippet_00000005_1_Class - _codeSnippets
    .long codeSnippet_80000005_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000005_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000005_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000005_2_General - _codeSnippets
    .long codeSnippet_80000005_2_General - _codeSnippets
    .long codeSnippet_00000005_2_Void - _codeSnippets
    .long codeSnippet_80000005_2_Void - _codeSnippets
    .long codeSnippet_00000005_2_Hyper - _codeSnippets
    .long codeSnippet_80000005_2_Hyper - _codeSnippets
    .long codeSnippet_00000005_2_Float - _codeSnippets
    .long codeSnippet_80000005_2_Float - _codeSnippets
    .long codeSnippet_00000005_2_Double - _codeSnippets
    .long codeSnippet_80000005_2_Double - _codeSnippets
    .long codeSnippet_00000005_2_Class - _codeSnippets
    .long codeSnippet_80000005_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000005_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000005_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000005_3_General - _codeSnippets
    .long codeSnippet_80000005_3_General - _codeSnippets
    .long codeSnippet_00000005_3_Void - _codeSnippets
    .long codeSnippet_80000005_3_Void - _codeSnippets
    .long codeSnippet_00000005_3_Hyper - _codeSnippets
    .long codeSnippet_80000005_3_Hyper - _codeSnippets
    .long codeSnippet_00000005_3_Float - _codeSnippets
    .long codeSnippet_80000005_3_Float - _codeSnippets
    .long codeSnippet_00000005_3_Double - _codeSnippets
    .long codeSnippet_80000005_3_Double - _codeSnippets
    .long codeSnippet_00000005_3_Class - _codeSnippets
    .long codeSnippet_80000005_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000006_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000006_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000006_0_General - _codeSnippets
    .long codeSnippet_80000006_0_General - _codeSnippets
    .long codeSnippet_00000006_0_Void - _codeSnippets
    .long codeSnippet_80000006_0_Void - _codeSnippets
    .long codeSnippet_00000006_0_Hyper - _codeSnippets
    .long codeSnippet_80000006_0_Hyper - _codeSnippets
    .long codeSnippet_00000006_0_Float - _codeSnippets
    .long codeSnippet_80000006_0_Float - _codeSnippets
    .long codeSnippet_00000006_0_Double - _codeSnippets
    .long codeSnippet_80000006_0_Double - _codeSnippets
    .long codeSnippet_00000006_0_Class - _codeSnippets
    .long codeSnippet_80000006_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000006_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000006_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000006_1_General - _codeSnippets
    .long codeSnippet_80000006_1_General - _codeSnippets
    .long codeSnippet_00000006_1_Void - _codeSnippets
    .long codeSnippet_80000006_1_Void - _codeSnippets
    .long codeSnippet_00000006_1_Hyper - _codeSnippets
    .long codeSnippet_80000006_1_Hyper - _codeSnippets
    .long codeSnippet_00000006_1_Float - _codeSnippets
    .long codeSnippet_80000006_1_Float - _codeSnippets
    .long codeSnippet_00000006_1_Double - _codeSnippets
    .long codeSnippet_80000006_1_Double - _codeSnippets
    .long codeSnippet_00000006_1_Class - _codeSnippets
    .long codeSnippet_80000006_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000006_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000006_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000006_2_General - _codeSnippets
    .long codeSnippet_80000006_2_General - _codeSnippets
    .long codeSnippet_00000006_2_Void - _codeSnippets
    .long codeSnippet_80000006_2_Void - _codeSnippets
    .long codeSnippet_00000006_2_Hyper - _codeSnippets
    .long codeSnippet_80000006_2_Hyper - _codeSnippets
    .long codeSnippet_00000006_2_Float - _codeSnippets
    .long codeSnippet_80000006_2_Float - _codeSnippets
    .long codeSnippet_00000006_2_Double - _codeSnippets
    .long codeSnippet_80000006_2_Double - _codeSnippets
    .long codeSnippet_00000006_2_Class - _codeSnippets
    .long codeSnippet_80000006_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000006_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000006_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000006_3_General - _codeSnippets
    .long codeSnippet_80000006_3_General - _codeSnippets
    .long codeSnippet_00000006_3_Void - _codeSnippets
    .long codeSnippet_80000006_3_Void - _codeSnippets
    .long codeSnippet_00000006_3_Hyper - _codeSnippets
    .long codeSnippet_80000006_3_Hyper - _codeSnippets
    .long codeSnippet_00000006_3_Float - _codeSnippets
    .long codeSnippet_80000006_3_Float - _codeSnippets
    .long codeSnippet_00000006_3_Double - _codeSnippets
    .long codeSnippet_80000006_3_Double - _codeSnippets
    .long codeSnippet_00000006_3_Class - _codeSnippets
    .long codeSnippet_80000006_3_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000007_0 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000007_0 - _codeSnippets
#endif
#else
    .long codeSnippet_00000007_0_General - _codeSnippets
    .long codeSnippet_80000007_0_General - _codeSnippets
    .long codeSnippet_00000007_0_Void - _codeSnippets
    .long codeSnippet_80000007_0_Void - _codeSnippets
    .long codeSnippet_00000007_0_Hyper - _codeSnippets
    .long codeSnippet_80000007_0_Hyper - _codeSnippets
    .long codeSnippet_00000007_0_Float - _codeSnippets
    .long codeSnippet_80000007_0_Float - _codeSnippets
    .long codeSnippet_00000007_0_Double - _codeSnippets
    .long codeSnippet_80000007_0_Double - _codeSnippets
    .long codeSnippet_00000007_0_Class - _codeSnippets
    .long codeSnippet_80000007_0_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000007_1 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000007_1 - _codeSnippets
#endif
#else
    .long codeSnippet_00000007_1_General - _codeSnippets
    .long codeSnippet_80000007_1_General - _codeSnippets
    .long codeSnippet_00000007_1_Void - _codeSnippets
    .long codeSnippet_80000007_1_Void - _codeSnippets
    .long codeSnippet_00000007_1_Hyper - _codeSnippets
    .long codeSnippet_80000007_1_Hyper - _codeSnippets
    .long codeSnippet_00000007_1_Float - _codeSnippets
    .long codeSnippet_80000007_1_Float - _codeSnippets
    .long codeSnippet_00000007_1_Double - _codeSnippets
    .long codeSnippet_80000007_1_Double - _codeSnippets
    .long codeSnippet_00000007_1_Class - _codeSnippets
    .long codeSnippet_80000007_1_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000007_2 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000007_2 - _codeSnippets
#endif
#else
    .long codeSnippet_00000007_2_General - _codeSnippets
    .long codeSnippet_80000007_2_General - _codeSnippets
    .long codeSnippet_00000007_2_Void - _codeSnippets
    .long codeSnippet_80000007_2_Void - _codeSnippets
    .long codeSnippet_00000007_2_Hyper - _codeSnippets
    .long codeSnippet_80000007_2_Hyper - _codeSnippets
    .long codeSnippet_00000007_2_Float - _codeSnippets
    .long codeSnippet_80000007_2_Float - _codeSnippets
    .long codeSnippet_00000007_2_Double - _codeSnippets
    .long codeSnippet_80000007_2_Double - _codeSnippets
    .long codeSnippet_00000007_2_Class - _codeSnippets
    .long codeSnippet_80000007_2_Class - _codeSnippets
#endif
#if defined(__arm) || defined(__arm64)
    .long codeSnippet_00000007_3 - _codeSnippets
#ifndef __arm64
    .long codeSnippet_80000007_3 - _codeSnippets
#endif
#else
    .long codeSnippet_00000007_3_General - _codeSnippets
    .long codeSnippet_80000007_3_General - _codeSnippets
    .long codeSnippet_00000007_3_Void - _codeSnippets
    .long codeSnippet_80000007_3_Void - _codeSnippets
    .long codeSnippet_00000007_3_Hyper - _codeSnippets
    .long codeSnippet_80000007_3_Hyper - _codeSnippets
    .long codeSnippet_00000007_3_Float - _codeSnippets
    .long codeSnippet_80000007_3_Float - _codeSnippets
    .long codeSnippet_00000007_3_Double - _codeSnippets
    .long codeSnippet_80000007_3_Double - _codeSnippets
    .long codeSnippet_00000007_3_Class - _codeSnippets
    .long codeSnippet_80000007_3_Class - _codeSnippets
#endif


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

#elif defined(__i386)
    // i386 code, for the simulator
    .text

.align 1, 0x90
_privateSnippetExecutorGeneral:
LFBg:
    movl    %esp,%ecx
    pushl   %ebp              // proper stack frame needed for exception handling
LCFIg0:
    movl    %esp,%ebp
LCFIg1:
    subl    $0x8,%esp         // padding + 32bit returnValue
    pushl   %esp              // 32bit &returnValue
    pushl   %ecx              // 32bit pCallStack
    pushl   %edx              // 32bit nVtableOffset
    pushl   %eax              // 32bit nFunctionIndex
    call    L_cpp_vtable_call$stub
    movl    16(%esp),%eax     // 32bit returnValue
    leave
    ret
LFEg:
    .long   .-_privateSnippetExecutorGeneral

.align 1, 0x90
_privateSnippetExecutorVoid:
LFBv:
    movl    %esp,%ecx
    pushl   %ebp              // proper stack frame needed for exception handling
LCFIv0:
    movl    %esp,%ebp
LCFIv1:
    sub     $8,%esp           // padding
    pushl   $0                // 32bit null pointer (returnValue not used)
    pushl   %ecx              // 32bit pCallStack
    pushl   %edx              // 32bit nVtableOffset
    pushl   %eax              // 32bit nFunctionIndex
    call    L_cpp_vtable_call$stub
    leave
    ret
LFEv:
    .long   .-_privateSnippetExecutorVoid

.align 1, 0x90
_privateSnippetExecutorHyper:
LFBh:
    movl    %esp,%ecx
    pushl   %ebp              // proper stack frame needed for exception handling
LCFIh0:
    movl    %esp,%ebp
LCFIh1:
    subl    $0x8,%esp         // 64bit returnValue
    pushl   %esp              // 32bit &returnValue
    pushl   %ecx              // 32bit pCallStack
    pushl   %edx              // 32bit nVtableOffset
    pushl   %eax              // 32bit nFunctionIndex
    call    L_cpp_vtable_call$stub
    movl    16(%esp),%eax     // 64bit returnValue, lower half
    movl    20(%esp),%edx     // 64bit returnValue, upper half
    leave
    ret
LFEh:
    .long   .-_privateSnippetExecutorHyper

.align 1, 0x90
_privateSnippetExecutorFloat:
LFBf:
    movl    %esp,%ecx
    pushl   %ebp              // proper stack frame needed for exception handling
LCFIf0:
    movl    %esp,%ebp
LCFIf1:
    subl    $0x8,%esp         // padding + 32bit returnValue
    pushl   %esp              // 32bit &returnValue
    pushl   %ecx              // 32bit pCallStack
    pushl   %edx              // 32bit nVtableOffset
    pushl   %eax              // 32bit nFunctionIndex
    call    L_cpp_vtable_call$stub
    flds    16(%esp)          // 32bit returnValue
    leave
    ret
LFEf:
    .long   .-_privateSnippetExecutorFloat

.align 1, 0x90
_privateSnippetExecutorDouble:
LFBd:
    movl    %esp,%ecx
    pushl   %ebp              // proper stack frame needed for exception handling
LCFId0:
    movl    %esp,%ebp
LCFId1:
    subl    $0x8,%esp         // 64bit returnValue
    pushl   %esp              // 32bit &returnValue
    pushl   %ecx              // 32bit pCallStack
    pushl   %edx              // 32bit nVtableOffset
    pushl   %eax              // 32bit nFunctionIndex
    call    L_cpp_vtable_call$stub
    fldl    16(%esp)          // 64bit returnValue
    leave
    ret
LFEd:
    .long   .-_privateSnippetExecutorDouble

.align 1, 0x90
_privateSnippetExecutorClass:
LFBc:
    movl    %esp,%ecx
    pushl   %ebp              // proper stack frame needed for exception handling
LCFIc0:
    movl    %esp,%ebp
LCFIc1:
    subl    $0x8,%esp         // padding + 32bit returnValue
    pushl   %esp              // 32bit &returnValue
    pushl   %ecx              // 32bit pCallStack
    pushl   %edx              // 32bit nVtableOffset
    pushl   %eax              // 32bit nFunctionIndex
    call    L_cpp_vtable_call$stub
    movl    16(%esp),%eax     // 32bit returnValue
    leave
    ret     $4
LFEc:
    .long   .-_privateSnippetExecutorClass

    .section __TEXT,__eh_frame,coalesced,no_toc+strip_static_syms+live_support
EH_frame1:
    .set L$set$frame1,LECIE1-LSCIE1
    .long   L$set$frame1      // length
LSCIE1:
    .long   0                 // CIE_ID
    .byte   1                 // version
    .ascii  "zPR\0"           // augmentation
    .byte   1                 // code_alignment_factor (.uleb128 1)
    .byte   0x7c              // data_alignment_factor (.sleb128 -4)
    .byte   8                 // return_address_register
    .byte   0x6               // augmentation size 7:
    .byte   0x9b              //  ???
    .long   L___gxx_personality_v0$non_lazy_ptr-.
    .byte   0x10
                              // initial_instructions:
    .byte   0x0C              //  DW_CFA_def_cfa %esp, 4
    .byte   5
    .byte   4
    .byte   0x88              //  DW_CFA_offset ret, 1
    .byte   1
    .align 2
LECIE1:
_privateSnippetExecutorGeneral.eh:
LSFDEg:
    .set L$set$g1,LEFDEg-LASFDEg
    .long   L$set$g1          // length
LASFDEg:
    .long   LASFDEg-EH_frame1 // CIE_pointer
    .long   LFBg-.            // initial_location
    .long   LFEg-LFBg         // address_range
    .byte   0                 // augmentation size 0
                              // instructions:
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIg0-LFBg
    .byte   0x0E              //  DW_CFA_def_cfa_offset 8
    .byte   8
    .byte   0x84              //  DW_CFA_offset %ebp, 2
    .byte   2
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIg1-LCFIg0
    .byte   0x0D              //  DW_CFA_def_cfa_register %ebp
    .byte   4
    .align 2
LEFDEg:
_privateSnippetExecutorVoid.eh:
LSFDEv:
    .set L$set$v1,LEFDEv-LASFDEv
    .long   L$set$v1          // length
LASFDEv:
    .long   LASFDEv-EH_frame1 // CIE_pointer
    .long   LFBv-.            // initial_location
    .long   LFEv-LFBv         // address_range
    .byte   0                 // augmentation size 0
                              // instructions:
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIv0-LFBv
    .byte   0x0E              //  DW_CFA_def_cfa_offset 8
    .byte   8
    .byte   0x84              //  DW_CFA_offset %ebp, 2
    .byte   2
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIv1-LCFIv0
    .byte   0x0D              //  DW_CFA_def_cfa_register %ebp
    .byte   4
    .align 2
LEFDEv:
_privateSnippetExecutorHyper.eh:
LSFDEh:
    .set L$set$h1,LEFDEh-LASFDEh
    .long   L$set$h1          // length
LASFDEh:
    .long   LASFDEh-EH_frame1 // CIE_pointer
    .long   LFBh-.            // initial_location
    .long   LFEh-LFBh         // address_range
    .byte   0                 // augmentation size 0
                              // instructions:
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIh0-LFBh
    .byte   0x0E              //  DW_CFA_def_cfa_offset 8
    .byte   8
    .byte   0x84              //  DW_CFA_offset %ebp, 2
    .byte   2
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIh1-LCFIh0
    .byte   0x0D              //  DW_CFA_def_cfa_register %ebp
    .byte   4
    .align 2
LEFDEh:
_privateSnippetExecutorFloat.eh:
LSFDEf:
    .set L$set$f1,LEFDEf-LASFDEf
    .long   L$set$f1          // length
LASFDEf:
    .long   LASFDEf-EH_frame1 // CIE_pointer
    .long   LFBf-.            // initial_location
    .long   LFEf-LFBf         // address_range
    .byte   0                 // augmentation size 0
                              // instructions:
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIf0-LFBf
    .byte   0x0E              //  DW_CFA_def_cfa_offset 8
    .byte   8
    .byte   0x84              //  DW_CFA_offset %ebp, 2
    .byte   2
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIf1-LCFIf0
    .byte   0x0D              //  DW_CFA_def_cfa_register %ebp
    .byte   4
    .align 2
LEFDEf:
_privateSnippetExecutorDouble.eh:
LSFDEd:
    .set L$set$d1,LEFDEd-LASFDEd
    .long   L$set$d1          // length
LASFDEd:
    .long   LASFDEd-EH_frame1 // CIE_pointer
    .long   LFBd-.            // initial_location
    .long   LFEd-LFBd         // address_range
    .byte   0                 // augmentation size 0
                              // instructions:
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFId0-LFBd
    .byte   0x0E              //  DW_CFA_def_cfa_offset 8
    .byte   8
    .byte   0x84              //  DW_CFA_offset %ebp, 2
    .byte   2
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFId1-LCFId0
    .byte   0x0D              //  DW_CFA_def_cfa_register %ebp
    .byte   4
    .align 2
LEFDEd:
_privateSnippetExecutorClass.eh:
LSFDEc:
    .set L$set$c1,LEFDEc-LASFDEc
    .long   L$set$c1          // length
LASFDEc:
    .long   LASFDEc-EH_frame1 // CIE_pointer
    .long   LFBc-.            // initial_location
    .long   LFEc-LFBc         // address_range
    .byte   0                 // augmentation size 0
                              // instructions:
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIc0-LFBc
    .byte   0x0E              //  DW_CFA_def_cfa_offset 8
    .byte   8
    .byte   0x84              //  DW_CFA_offset %ebp, 2
    .byte   2
    .byte   0x04              //  DW_CFA_advance_loc4
    .long   LCFIc1-LCFIc0
    .byte   0x0D              //  DW_CFA_def_cfa_register %ebp
    .byte   4
    .align 2
LEFDEc:
    .section __IMPORT,__jump_table,symbol_stubs,self_modifying_code+pure_instructions,5
L_cpp_vtable_call$stub:
    .indirect_symbol _cpp_vtable_call
    hlt ; hlt ; hlt ; hlt ; hlt
    .section __IMPORT,__pointers,non_lazy_symbol_pointers
L___gxx_personality_v0$non_lazy_ptr:
    .indirect_symbol ___gxx_personality_v0
    .long 0
    .constructor
    .destructor
    .align 1
#else

	.text
	.align 2
.globl privateSnippetExecutor
_privateSnippetExecutor:
.LFB3:
	pushq	%rbp
.LCFI0:
	movq	%rsp, %rbp
.LCFI1:
	subq	$160, %rsp
.LCFI2:
	movq	%r10, -152(%rbp)		# Save (nVtableOffset << 32) + nFunctionIndex

	movq	%rdi, -112(%rbp)		# Save GP registers
	movq	%rsi, -104(%rbp)
	movq	%rdx, -96(%rbp)
	movq	%rcx, -88(%rbp)
	movq	%r8 , -80(%rbp)
	movq	%r9 , -72(%rbp)
	
	movsd	%xmm0, -64(%rbp)		# Save FP registers
	movsd	%xmm1, -56(%rbp)
	movsd	%xmm2, -48(%rbp)
	movsd	%xmm3, -40(%rbp)
	movsd	%xmm4, -32(%rbp)
	movsd	%xmm5, -24(%rbp)
	movsd	%xmm6, -16(%rbp)
	movsd	%xmm7, -8(%rbp)

	leaq	-144(%rbp), %r9			# 6th param: sal_uInt64 * pRegisterReturn
	leaq	16(%rbp), %r8			# 5rd param: void ** ovrflw
	leaq	-64(%rbp), %rcx			# 4th param: void ** fpreg
	leaq	-112(%rbp), %rdx		# 3rd param: void ** gpreg
	movl	-148(%rbp), %esi		# 2nd param: sal_int32 nVtableOffset
	movl	-152(%rbp), %edi		# 1st param: sal_int32 nFunctionIndex
	
	call	cpp_vtable_call

	cmp	$10, %rax					# typelib_TypeClass_FLOAT
	je	.Lfloat
	cmp	$11, %rax					# typelib_TypeClass_DOUBLE
	je	.Lfloat

	movq	-144(%rbp), %rax		# Return value (int case)
	movq	-136(%rbp), %rdx		# Return value (int case)
	movq	-144(%rbp), %xmm0		# Return value (int case)
	movq	-136(%rbp), %xmm1		# Return value (int case)
	jmp	.Lfinish
.Lfloat:
	movlpd	-144(%rbp), %xmm0		# Return value (float/double case)

.Lfinish:
	leave
	ret
.LFE3:
	# see http://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html
	# for details of the .eh_frame, the "Common Information Entry" and "Frame Description Entry" formats
	# and http://mentorembedded.github.io/cxx-abi/exceptions.pdf for more info
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zR"
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.uleb128 0x1
	.byte	0x1b
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB3-.
	.long	.LFE3-.LFB3
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI0-.LFB3
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI1-.LCFI0
	.byte	0xd
	.uleb128 0x6
	.align 8
.LEFDE1:
#endif

// vim:set shiftwidth=4 softtabstop=4 expandtab:
