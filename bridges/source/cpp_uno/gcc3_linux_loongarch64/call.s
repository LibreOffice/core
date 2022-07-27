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
        .globl	privateSnippetExecutor
.LFB0 = .
        .cfi_startproc
        .cfi_personality 0x80,DW.ref.__gxx_personality_v0
        .cfi_lsda 0,.LLSDA0
        .type	privateSnippetExecutor, @function
privateSnippetExecutor:
        addi.d	$sp,$sp,-160
        .cfi_def_cfa_offset 160
        st.d	$ra,$sp,152
        .cfi_offset 1, -8
.LEHB0 = .
        // Save the float point registers
        fst.d	$f0,$sp,80
        fst.d	$f1,$sp,88
        fst.d	$f2,$sp,96
        fst.d	$f3,$sp,104
        fst.d	$f4,$sp,112
        fst.d	$f5,$sp,120
        fst.d	$f6,$sp,128
        fst.d	$f7,$sp,136
        // Save the general purpose registers
        st.d	$a0,$sp,16
        st.d	$a1,$sp,24
        st.d	$a2,$sp,32
        st.d	$a3,$sp,40
        st.d	$a4,$sp,48
        st.d	$a5,$sp,56
        st.d	$a6,$sp,64
        st.d	$a7,$sp,72
        // Load arguments
        // a0=index
        move	$a0,$t6
        // a1=offset
        move	$a1,$t7
        // a2=gpregptr
        addi.d	$a2,$sp,16
        // a3=fpregptr
        addi.d	$a3,$sp,80
        // a4=ovrflw
        addi.d	$a4,$sp,160
        // a5=retregptr
        move	$a5,$sp

        // Call cpp_vtable_call
        jirl	$ra,$t5,0

.LEHE0 = .
        // Perform return value
        fld.d	$f0,$sp,0
        fld.d	$f1,$sp,8
        ld.d	$a0,$sp,0
        ld.d	$a1,$sp,8

        ld.d	$ra,$sp,152
        .cfi_restore 1
        addi.d	$sp,$sp,160
        .cfi_def_cfa_offset 0
        jr	$ra
        .cfi_endproc

.LFE0:
        .globl	__gxx_personality_v0
        .section	.gcc_except_table,"aw",@progbits
        .align	3
.LLSDA0:
        .byte	0xff
        .byte	0x80
        .uleb128 .LLSDATT0-.LLSDATTD0
.LLSDATTD0:
        .byte	0x1
        .uleb128 .LLSDACSE0-.LLSDACSB0
.LLSDACSB0:
        .uleb128 .LEHB0-.LFB0
        .uleb128 .LEHE0-.LEHB0
        .uleb128 0
        .uleb128 0
.LLSDACSE0:
        .byte	0x7f
        .byte	0
        .align	3
        .8byte	DW.ref._ZTIi
.LLSDATT0:
        .byte	0x1
        .byte	0
        .text
        .size	privateSnippetExecutor, .-privateSnippetExecutor
        .hidden	DW.ref._ZTIi
        .weak	DW.ref._ZTIi
        .section	.data.DW.ref._ZTIi,"awG",@progbits,DW.ref._ZTIi,comdat
        .align	3
        .type	DW.ref._ZTIi, @object
        .size	DW.ref._ZTIi, 8
DW.ref._ZTIi:
        .dword	_ZTIi
        .hidden	DW.ref.__gxx_personality_v0
        .weak	DW.ref.__gxx_personality_v0
        .section	.data.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
        .align	3
        .type	DW.ref.__gxx_personality_v0, @object
        .size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
        .dword	__gxx_personality_v0
