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

.globl privateSnippetExecutorGeneral
    .type   privateSnippetExecutorGeneral,@function
privateSnippetExecutorGeneral:
.LFBg:
    endbr64
    movl    %esp,%ecx
    pushl   %ebp              # proper stack frame needed for exception handling
.LCFIg0:
    movl    %esp,%ebp
.LCFIg1:
    subl    $0x8,%esp         # 32bit returnValue, and preserve potential 128bit
                              #  stack alignment
    pushl   %esp              # 32bit &returnValue
    pushl   %ecx              # 32bit pCallStack
    pushl   %edx              # 32bit nVtableOffset
    pushl   %eax              # 32bit nFunctionIndex
    call    cpp_vtable_call
    movl    16(%esp),%eax     # 32bit returnValue
    leave
    ret
.LFEg:
    .size   privateSnippetExecutorGeneral,.-privateSnippetExecutorGeneral

.globl privateSnippetExecutorVoid
    .type   privateSnippetExecutorVoid,@function
privateSnippetExecutorVoid:
.LFBv:
    endbr64
    movl    %esp,%ecx
    pushl   %ebp              # proper stack frame needed for exception handling
.LCFIv0:
    movl    %esp,%ebp
.LCFIv1:
    andl    $0xFFFFFFF0,%esp  # preserve potential 128bit stack alignment
    pushl   $0                # 32bit null pointer (returnValue not used)
    pushl   %ecx              # 32bit pCallStack
    pushl   %edx              # 32bit nVtableOffset
    pushl   %eax              # 32bit nFunctionIndex
    call    cpp_vtable_call
    leave
    ret
.LFEv:
    .size   privateSnippetExecutorVoid,.-privateSnippetExecutorVoid

.globl privateSnippetExecutorHyper
    .type   privateSnippetExecutorHyper,@function
privateSnippetExecutorHyper:
.LFBh:
    endbr64
    movl    %esp,%ecx
    pushl   %ebp              # proper stack frame needed for exception handling
.LCFIh0:
    movl    %esp,%ebp
.LCFIh1:
    subl    $0x8,%esp         # 64bit returnValue (preserves potential 128bit
                              #  stack alignment)
    pushl   %esp              # 32bit &returnValue
    pushl   %ecx              # 32bit pCallStack
    pushl   %edx              # 32bit nVtableOffset
    pushl   %eax              # 32bit nFunctionIndex
    call    cpp_vtable_call
    movl    16(%esp),%eax     # 64bit returnValue, lower half
    movl    20(%esp),%edx     # 64bit returnValue, upper half
    leave
    ret
.LFEh:
    .size   privateSnippetExecutorHyper,.-privateSnippetExecutorHyper

.globl privateSnippetExecutorFloat
    .type   privateSnippetExecutorFloat,@function
privateSnippetExecutorFloat:
.LFBf:
    endbr64
    movl    %esp,%ecx
    pushl   %ebp              # proper stack frame needed for exception handling
.LCFIf0:
    movl    %esp,%ebp
.LCFIf1:
    subl    $0x8,%esp         # 32bit returnValue, and preserve potential 128bit
                              #  stack alignment
    pushl   %esp              # 32bit &returnValue
    pushl   %ecx              # 32bit pCallStack
    pushl   %edx              # 32bit nVtableOffset
    pushl   %eax              # 32bit nFunctionIndex
    call    cpp_vtable_call
    flds    16(%esp)          # 32bit returnValue
    leave
    ret
.LFEf:
    .size   privateSnippetExecutorFloat,.-privateSnippetExecutorFloat

.globl privateSnippetExecutorDouble
    .type   privateSnippetExecutorDouble,@function
privateSnippetExecutorDouble:
.LFBd:
    endbr64
    movl    %esp,%ecx
    pushl   %ebp              # proper stack frame needed for exception handling
.LCFId0:
    movl    %esp,%ebp
.LCFId1:
    subl    $0x8,%esp         # 64bit returnValue (preserves potential 128bit
                              #  stack alignment)
    pushl   %esp              # 32bit &returnValue
    pushl   %ecx              # 32bit pCallStack
    pushl   %edx              # 32bit nVtableOffset
    pushl   %eax              # 32bit nFunctionIndex
    call    cpp_vtable_call
    fldl    16(%esp)          # 64bit returnValue
    leave
    ret
.LFEd:
    .size   privateSnippetExecutorDouble,.-privateSnippetExecutorDouble

.globl privateSnippetExecutorClass
    .type   privateSnippetExecutorClass,@function
privateSnippetExecutorClass:
.LFBc:
    endbr64
    movl    %esp,%ecx
    pushl   %ebp              # proper stack frame needed for exception handling
.LCFIc0:
    movl    %esp,%ebp
.LCFIc1:
    subl    $0x8,%esp         # 32bit returnValue, and preserve potential 128bit
                              #  stack alignment
    pushl   %esp              # 32bit &returnValue
    pushl   %ecx              # 32bit pCallStack
    pushl   %edx              # 32bit nVtableOffset
    pushl   %eax              # 32bit nFunctionIndex
    call    cpp_vtable_call
    movl    16(%esp),%eax     # 32bit returnValue
    leave
    ret     $4
.LFEc:
    .size   privateSnippetExecutorClass,.-privateSnippetExecutorClass

    .section .eh_frame,"a",@progbits
.Lframe1:
    .long   .LECIE1-.LSCIE1   # length
.LSCIE1:
    .long   0                 # CIE_ID
    .byte   1                 # version
    .string "zR"              # augmentation
    .uleb128 1                # code_alignment_factor
    .sleb128 -4               # data_alignment_factor
    .byte   8                 # return_address_register
    .uleb128 1                # augmentation size 1:
    .byte   0x1B              #  FDE Encoding (pcrel sdata4)
                              # initial_instructions:
    .byte   0x0C              #  DW_CFA_def_cfa %esp, 4
    .uleb128 4
    .uleb128 4
    .byte   0x88              #  DW_CFA_offset ret, 1
    .uleb128 1
    .align 4
.LECIE1:
.LSFDEg:
    .long   .LEFDEg-.LASFDEg  # length
.LASFDEg:
    .long   .LASFDEg-.Lframe1 # CIE_pointer
    .long   .LFBg-.           # initial_location
    .long   .LFEg-.LFBg       # address_range
    .uleb128 0                # augmentation size 0
                              # instructions:
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIg0-.LFBg
    .byte   0x0E              #  DW_CFA_def_cfa_offset 8
    .uleb128 8
    .byte   0x85              #  DW_CFA_offset %ebp, 2
    .uleb128 2
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIg1-.LCFIg0
    .byte   0x0D              #  DW_CFA_def_cfa_register %ebp
    .uleb128 5
    .align 4
.LEFDEg:
.LSFDEv:
    .long   .LEFDEv-.LASFDEv  # length
.LASFDEv:
    .long   .LASFDEv-.Lframe1 # CIE_pointer
    .long   .LFBv-.           # initial_location
    .long   .LFEv-.LFBv       # address_range
    .uleb128 0                # augmentation size 0
                              # instructions:
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIv0-.LFBv
    .byte   0x0E              #  DW_CFA_def_cfa_offset 8
    .uleb128 8
    .byte   0x85              #  DW_CFA_offset %ebp, 2
    .uleb128 2
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIv1-.LCFIv0
    .byte   0x0D              #  DW_CFA_def_cfa_register %ebp
    .uleb128 5
    .align 4
.LEFDEv:
.LSFDEh:
    .long   .LEFDEh-.LASFDEh  # length
.LASFDEh:
    .long   .LASFDEh-.Lframe1 # CIE_pointer
    .long   .LFBh-.           # initial_location
    .long   .LFEh-.LFBh       # address_range
    .uleb128 0                # augmentation size 0
                              # instructions:
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIh0-.LFBh
    .byte   0x0E              #  DW_CFA_def_cfa_offset 8
    .uleb128 8
    .byte   0x85              #  DW_CFA_offset %ebp, 2
    .uleb128 2
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIh1-.LCFIh0
    .byte   0x0D              #  DW_CFA_def_cfa_register %ebp
    .uleb128 5
    .align 4
.LEFDEh:
.LSFDEf:
    .long   .LEFDEf-.LASFDEf  # length
.LASFDEf:
    .long   .LASFDEf-.Lframe1 # CIE_pointer
    .long   .LFBf-.           # initial_location
    .long   .LFEf-.LFBf       # address_range
    .uleb128 0                # augmentation size 0
                              # instructions:
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIf0-.LFBf
    .byte   0x0E              #  DW_CFA_def_cfa_offset 8
    .uleb128 8
    .byte   0x85              #  DW_CFA_offset %ebp, 2
    .uleb128 2
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIf1-.LCFIf0
    .byte   0x0D              #  DW_CFA_def_cfa_register %ebp
    .uleb128 5
    .align 4
.LEFDEf:
.LSFDEd:
    .long   .LEFDEd-.LASFDEd  # length
.LASFDEd:
    .long   .LASFDEd-.Lframe1 # CIE_pointer
    .long   .LFBd-.           # initial_location
    .long   .LFEd-.LFBd       # address_range
    .uleb128 0                # augmentation size 0
                              # instructions:
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFId0-.LFBd
    .byte   0x0E              #  DW_CFA_def_cfa_offset 8
    .uleb128 8
    .byte   0x85              #  DW_CFA_offset %ebp, 2
    .uleb128 2
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFId1-.LCFId0
    .byte   0x0D              #  DW_CFA_def_cfa_register %ebp
    .uleb128 5
    .align 4
.LEFDEd:
.LSFDEc:
    .long   .LEFDEc-.LASFDEc  # length
.LASFDEc:
    .long   .LASFDEc-.Lframe1 # CIE_pointer
    .long   .LFBc-.           # initial_location
    .long   .LFEc-.LFBc       # address_range
    .uleb128 0                # augmentation size 0
                              # instructions:
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIc0-.LFBc
    .byte   0x0E              #  DW_CFA_def_cfa_offset 8
    .uleb128 8
    .byte   0x85              #  DW_CFA_offset %ebp, 2
    .uleb128 2
    .byte   0x04              #  DW_CFA_advance_loc4
    .long   .LCFIc1-.LCFIc0
    .byte   0x0D              #  DW_CFA_def_cfa_register %ebp
    .uleb128 5
    .align 4
.LEFDEc:
    .section .note.GNU-stack,"",@progbits
    .section .note.gnu.property,"a"
    .align 8
    .long     1f - 0f
    .long     4f - 1f
    .long     5
0:
    .string     "GNU"
1:
    .align 8
    .long     0xc0000002
    .long     3f - 2f
2:
    .long     0x3
3:
    .align 8
4:
