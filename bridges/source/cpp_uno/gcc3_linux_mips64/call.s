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
        .ent	privateSnippetExecutor
        .type	privateSnippetExecutor, @function
privateSnippetExecutor:
        .set	noreorder
        daddiu	$sp,$sp,-160
        .cfi_def_cfa_offset 160
        sd	$ra,152($sp)
        .cfi_offset 31, -8
.LEHB0 = .
        // Save the float point registers
        sdc1	$f12,80($sp)
        sdc1	$f13,88($sp)
        sdc1	$f14,96($sp)
        sdc1	$f15,104($sp)
        sdc1	$f16,112($sp)
        sdc1	$f17,120($sp)
        sdc1	$f18,128($sp)
        sdc1	$f19,136($sp)
        // Save the general purpose registers
        sd	$a0,16($sp)
        sd	$a1,24($sp)
        sd	$a2,32($sp)
        sd	$a3,40($sp)
        sd	$a4,48($sp)
        sd	$a5,56($sp)
        sd	$a6,64($sp)
        sd	$a7,72($sp)
        // Load arguments
        // a0=index
        move	$a0,$v0
        // a1=offset
        move	$a1,$v1
        // a2=gpregptr
        daddiu	$a2,$sp,16
        // a3=fpregptr
        daddiu	$a3,$sp,80
        // a4=ovrflw
        daddiu	$a4,$sp,160
        // Call cpp_vtable_call
        jalr	$t9
        // a5=retregptr
        move	$a5,$sp

.LEHE0 = .
        // Perform return value
        li	$v1,10
        beq	$v0,$v1,.Lfloat
        li	$v1,11
        beq	$v0,$v1,.Lfloat
        ldc1	$f0,0($sp)
        ldc1	$f2,8($sp)
        ld	$v0,0($sp)
        b	.Lfinish
        ld	$v1,8($sp)
.Lfloat:
        ldc1	$f0,0($sp)
        ldc1	$f2,8($sp)

.Lfinish:
        ld	$ra,152($sp)
        .cfi_restore 31
        jr	$ra
        daddiu	$sp,$sp,160
        .cfi_def_cfa_offset 0

        .set	reorder
        .end	privateSnippetExecutor
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
