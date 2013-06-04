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
.globl privateSnippetExecutor
privateSnippetExecutor:
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
	.long	.-privateSnippetExecutor
	# see http://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html
	# for details of the .eh_frame, the "Common Information Entry" and "Frame Description Entry" formats
	# and http://mentorembedded.github.io/cxx-abi/exceptions.pdf for more info
	.section	.eh_frame,"a"
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
