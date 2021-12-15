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
	.type	privateSnippetExecutor, @function
privateSnippetExecutor:
.LFB3:
	endbr64
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

	testl	%eax, %eax
	je	.Lfpint
	jg	.Lintfp

	movq	-144(%rbp), %rax		# Potential return value (general case)
	movq	-136(%rbp), %rdx		# Potential return value (general case)
	movq	-144(%rbp), %xmm0		# Potential return value (general case)
	movq	-136(%rbp), %xmm1		# Potential return value (general case)
	jmp	.Lfinish
.Lfpint:
	movq	-144(%rbp), %xmm0		# Return value (special fp and integer case)
	movq	-136(%rbp), %rax		# Return value (special fp and integer case)
	jmp	.Lfinish
.Lintfp:
	movq	-144(%rbp), %rax		# Return value (special integer and fp case)
	movq	-136(%rbp), %xmm0		# Return value (special integer and fp case)

.Lfinish:
	leave
	ret
.LFE3:
	.size	privateSnippetExecutor, .-privateSnippetExecutor
	# see http://refspecs.linuxfoundation.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html
	# for details of the .eh_frame, the "Common Information Entry" and "Frame Description Entry" formats
	# and http://mentorembedded.github.io/cxx-abi/exceptions.pdf for more info
	.section	.eh_frame,"a",@unwind
.Lframe1:
	.long	.LECIE1-.LSCIE1         # CIE Length
.LSCIE1:
	.long	0x0                     # CIE ID
	.byte	0x1                     # CIE Version
	.string	"zR"                    # CIE Augmentation String
	.uleb128 0x1                    # CIE Code Alignment Factor
	.sleb128 -8                     # CIE Data Alignment Factor
	.byte	0x10                    # CIE Return Address Register: pseudo "Return Address RA"
	.uleb128 0x1                    # CIE Augmentation Data Length
	.byte	0x1b                    # CIE Augmentation Data
	                                # CIE Initial Instructions:
	.byte	0xc                     #  DW_CFA_def_cfa %rsp +8
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90                    #  DW_CFA_offset (pseudo "Return Address RA") +1 (i.e., -8)
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1        # FDE Length
.LASFDE1:
	.long	.LASFDE1-.Lframe1       # FDE CIE Pointer
	.long	.LFB3-.                 # FDE PC Begin
	.long	.LFE3-.LFB3             # FDE PC Range
	.uleb128 0x0                    # FDE Augmentation Data Length
	                                # FDE Call Frame Instructions:
	.byte	0x4                     #  DW_CFA_advance_loc4 .LCFI0
	.long	.LCFI0-.LFB3
	.byte	0xe                     #  DW_CFA_def_cfa_offset +16
	.uleb128 0x10
	.byte	0x86                    #  DW_CFA_offset %rbp +2 (i.e., -16)
	.uleb128 0x2
	.byte	0x4                     #  DW_CFA_advance_loc4 .LCFI1
	.long	.LCFI1-.LCFI0
	.byte	0xd                     #  DW_CFA_def_cfa_register %rbp
	.uleb128 0x6
	.align 8
.LEFDE1:
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
