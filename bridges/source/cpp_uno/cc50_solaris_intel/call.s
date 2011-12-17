//**************************************************************
/ * 
/ * Licensed to the Apache Software Foundation (ASF) under one
/ * or more contributor license agreements.  See the NOTICE file
/ * distributed with this work for additional information
/ * regarding copyright ownership.  The ASF licenses this file
/ * to you under the Apache License, Version 2.0 (the
/ * "License"); you may not use this file except in compliance
/ * with the License.  You may obtain a copy of the License at
/ * 
/ *   http://www.apache.org/licenses/LICENSE-2.0
/ * 
/ * Unless required by applicable law or agreed to in writing,
/ * software distributed under the License is distributed on an
/ * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
/ * KIND, either express or implied.  See the License for the
/ * specific language governing permissions and limitations
/ * under the License.
/ * 
/ *************************************************************/

    .align 4
    .globl privateSnippetExecutorGeneral
privateSnippetExecutorGeneral:
    movl    %esp,%ecx
    pushl   %ebp              / proper stack frame needed for exception handling
    movl    %esp,%ebp
    andl    $0xFFFFFFF8,%esp  / align following 64bit arg
    subl    $0x8,%esp         / 64bit nRegReturn
    pushl   %ecx              / 32bit pCallStack
    pushl   %edx              / 32bit nVtableOffset
    pushl   %eax              / 32bit nFunctionIndex
    call    cpp_vtable_call
    movl    12(%esp),%eax     / 64 bit nRegReturn, lower half
    leave
    ret
    .type privateSnippetExecutorGeneral, @function
    .size privateSnippetExecutorGeneral, .-privateSnippetExecutorGeneral

    .align 4
    .globl privateSnippetExecutorVoid
privateSnippetExecutorVoid:
    movl    %esp,%ecx
    pushl   %ebp              / proper stack frame needed for exception handling
    movl    %esp,%ebp
    andl    $0xFFFFFFF8,%esp  / align following 64bit arg
    subl    $0x8,%esp         / 64bit nRegReturn
    pushl   %ecx              / 32bit pCallStack
    pushl   %edx              / 32bit nVtableOffset
    pushl   %eax              / 32bit nFunctionIndex
    call    cpp_vtable_call
    leave
    ret
    .type privateSnippetExecutorVoid, @function
    .size privateSnippetExecutorVoid, .-privateSnippetExecutorVoid

    .align 4
    .globl privateSnippetExecutorHyper
privateSnippetExecutorHyper:
    movl    %esp,%ecx
    pushl   %ebp              / proper stack frame needed for exception handling
    movl    %esp,%ebp
    andl    $0xFFFFFFF8,%esp  / align following 64bit arg
    subl    $0x8,%esp         / 64bit nRegReturn
    pushl   %ecx              / 32bit pCallStack
    pushl   %edx              / 32bit nVtableOffset
    pushl   %eax              / 32bit nFunctionIndex
    call    cpp_vtable_call
    movl    12(%esp),%eax     / 64 bit nRegReturn, lower half
    movl    16(%esp),%edx     / 64 bit nRegReturn, upper half
    leave
    ret
    .type privateSnippetExecutorHyper, @function
    .size privateSnippetExecutorHyper, .-privateSnippetExecutorHyper

    .align 4
    .globl privateSnippetExecutorFloat
privateSnippetExecutorFloat:
    movl    %esp,%ecx
    pushl   %ebp              / proper stack frame needed for exception handling
    movl    %esp,%ebp
    andl    $0xFFFFFFF8,%esp  / align following 64bit arg
    subl    $0x8,%esp         / 64bit nRegReturn
    pushl   %ecx              / 32bit pCallStack
    pushl   %edx              / 32bit nVtableOffset
    pushl   %eax              / 32bit nFunctionIndex
    call    cpp_vtable_call
    flds    12(%esp)          / 64 bit nRegReturn, lower half
    leave
    ret
    .type privateSnippetExecutorFloat, @function
    .size privateSnippetExecutorFloat, .-privateSnippetExecutorFloat

    .align 4
    .globl privateSnippetExecutorDouble
privateSnippetExecutorDouble:
    movl    %esp,%ecx
    pushl   %ebp              / proper stack frame needed for exception handling
    movl    %esp,%ebp
    andl    $0xFFFFFFF8,%esp  / align following 64bit arg
    subl    $0x8,%esp         / 64bit nRegReturn
    pushl   %ecx              / 32bit pCallStack
    pushl   %edx              / 32bit nVtableOffset
    pushl   %eax              / 32bit nFunctionIndex
    call    cpp_vtable_call
    fldl    12(%esp)          / 64 bit nRegReturn
    leave
    ret
    .type privateSnippetExecutorDouble, @function
    .size privateSnippetExecutorDouble, .-privateSnippetExecutorDouble

    .align 4
    .globl privateSnippetExecutorStruct
privateSnippetExecutorStruct:
    movl    %esp,%ecx
    pushl   %ebp              / proper stack frame needed for exception handling
    movl    %esp,%ebp
    andl    $0xFFFFFFF8,%esp  / align following 64bit arg
    subl    $0x8,%esp         / 64bit nRegReturn
    pushl   %ecx              / 32bit pCallStack
    pushl   %edx              / 32bit nVtableOffset
    pushl   %eax              / 32bit nFunctionIndex
    call    cpp_vtable_call
    movl    12(%esp),%eax     / 64 bit nRegReturn, lower half
    leave
    ret     $4
    .type privateSnippetExecutorStruct, @function
    .size privateSnippetExecutorStruct, .-privateSnippetExecutorStruct

	.align 4
	.globl callVirtualMethod
callVirtualMethod:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	%edx, -4(%ebp)
	movl	%ecx, -8(%ebp)
	movl	%eax, -12(%ebp)
	movl	%esp, -16(%ebp)
	movl	%ebx, -20(%ebp)

	/ set ebx to GOT
.L_GOT_BEGIN_2:
	call	.L_GOT_END_2
.L_GOT_END_2:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.L_GOT_END_2],%ebx
.callBeginPosition:
	movl	28(%ebp), %eax
	movl	%eax, %edx
	dec		%edx
	shl		$2, %edx
	add		24(%ebp), %edx
.copyLong:
	movl	0(%edx), %ecx
	sub		$4, %edx
	push	%ecx
	dec		%eax
	jne		.copyLong
.doCall:
	movl	8(%ebp), %edx
	movl	0(%edx), %edx
	movl	12(%ebp), %eax
	add		$2, %eax
	shl		$2, %eax
	add		%eax, %edx
	movl	0(%edx), %edx

	call	*%edx

.callVirtualMethodExceptionPosition:
	/ handle returns
	movl	20(%ebp), %ecx

	/ byte types
	cmp     $2, %ecx	/ typelib_TypeClass_BOOLEAN
	je		.handleByte
	cmp		$3, %ecx
	je		.handleByte	/ typelib_TypeClass_BYTE
	
	/ half word types
	cmp    $4, %ecx		/ typelib_TypeClass_SHORT
	je     .handleShort
	cmp    $5, %ecx		/ typelib_TypeClass_UNSIGNED_SHORT
	je     .handleShort

	/ word types
	cmp    $6, %ecx		/ typelib_TypeClass_LONG
	je     .handleWord
	cmp    $7, %ecx		/ typelib_TypeClass_UNSIGNED_LONG
	je     .handleWord
	cmp    $1, %ecx		/ typelib_TypeClass_CHAR (wchar_t)
	je     .handleWord
	cmp    $15, %ecx	/ typelib_TypeClass_ENUM
	je     .handleWord

	/ double word types
	cmp    $8, %ecx		/ typelib_TypeClass_HYPER
	je     .handleDoubleWord
	cmp    $9, %ecx		/ typelib_TypeClass_UNSIGNED_HYPER
	je     .handleDoubleWord

	/ float
	cmp    $10, %ecx	/ typelib_TypeClass_FLOAT
	je     .handleFloat

	/ double
	cmp    $11, %ecx	/ typelib_TypeClass_DOUBLE
	je     .handleDouble

	/ default: return void
	jmp    .doRestore
.handleByte:
	movl	16(%ebp), %ecx
	movb	%al, 0(%ecx)
	jmp		.doRestore
.handleShort:
	movl	16(%ebp), %ecx
	movw	%ax, 0(%ecx)
	jmp		.doRestore
.handleWord:
	movl	16(%ebp), %ecx
	movl	%eax, 0(%ecx)
	jmp		.doRestore
.handleDoubleWord:
	movl	16(%ebp), %ecx
	movl	%eax, 0(%ecx)
	movl	%edx, 4(%ecx)
	jmp		.doRestore
.handleFloat:
	movl	16(%ebp), %ecx
	fstps	0(%ecx)
	jmp		.doRestore
.handleDouble:
	movl	16(%ebp), %ecx
	fstpl	0(%ecx)
	jmp		.doRestore
.doRestore:
	movl	-4(%ebp), %edx
	movl	-8(%ebp), %ecx
	movl	-12(%ebp), %eax
	movl	-20(%ebp), %ebx
	movl	%ebp, %esp
	popl	%ebp
	ret
	.type callVirtualMethod, @function
	.size callVirtualMethod, .-callVirtualMethod

	.globl callVirtualMethodExceptionHandler
callVirtualMethodExceptionHandler:
	movl	-4(%ebp), %edx
	movl	-8(%ebp), %ecx
	movl	-12(%ebp), %eax
	movl	-16(%ebp), %esp
	movl	-20(%ebp), %ebx
	call	__1cG__CrunMex_rethrow_q6F_v_@PLT
	ret
	
	.type callVirtualMethodExceptionHandler, @function
	.size callVirtualMethodExceptionHandler, .-callVirtualMethodExceptionHandler


	.section	.exception_ranges,"aw"
	.align		4

	.4byte		.callBeginPosition@rel
	.4byte		.callVirtualMethodExceptionPosition-.callBeginPosition
	.4byte		callVirtualMethodExceptionHandler-.callBeginPosition
	.zero		8
