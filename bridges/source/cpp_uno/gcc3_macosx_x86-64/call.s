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
	.align 4, 0x90
	.globl _privateSnippetExecutor
_privateSnippetExecutor:
	.cfi_startproc
.LFB3:
	pushq	%rbp
.LCFI0:
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
.LCFI1:
	subq	$160, %rsp
.LCFI2:
	.cfi_def_cfa_register %rbp
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
	
	call	_cpp_vtable_call

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
	.cfi_endproc

.subsections_via_symbols
