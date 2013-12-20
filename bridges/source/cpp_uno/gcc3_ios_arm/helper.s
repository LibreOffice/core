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

#include "codesnippets.S"

#if defined(__arm)
// ARM support code for LibreOffice C++/UNO bridging
//
// Written by Peter Naulls <peter@chocky.org>
// Modified by Caolan McNamara <caolanm@redhat.com>
// Fixed by Michael Casadevall <mcasadevall@kubuntu.org>
// Modified for iOS by Tor Lillqvist <tml@iki.fi>

    .text
    .align 4

_privateSnippetExecutor:
    stmfd   sp!, {r0-r3}    // follow other parameters on stack
    mov     r0, ip          // r0 points to functionoffset/vtable
    mov     r1, sp          // r1 points to this and params
                            // (see cpp2uno.cxx:codeSnippet())
    stmfd   sp!, {r4, lr}   // save return address
                            // (r4 pushed to preserve stack alignment)
    bl      _cpp_vtable_call

    add     sp, sp, #4      // no need to restore r4 (we didn't touch it)
    ldr     pc, [sp], #20   // return, discarding function arguments

#elif defined(__arm64)

    .text
    .align 4

_privateSnippetExecutor:

    stp     x6, x7, [sp, #-16]!
    stp     x4, x5, [sp, #-16]!
    stp     x2, x3, [sp, #-16]!
    stp     x0, x1, [sp, #-16]!

    mov     x0, x15
    stp     x8, lr, [sp, #-16]!
    mov     x1, sp

    bl      _cpp_vtable_call

    ldp     x8, lr, [sp, #0]
    add     sp, sp, #80
    ret     lr

#else
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
#endif

// vim:set shiftwidth=4 softtabstop=4 expandtab:
