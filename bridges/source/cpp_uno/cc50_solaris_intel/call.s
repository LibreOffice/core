	.globl privateSnippetExecutor
	.align 4
privateSnippetExecutor:
.L21:
		subl	$32, %esp
		/ original %ebx, %eax, %ecx saved by jmp snippet into -12, -16, -20 resp
		movl	%esp, -24(%ebp)
		movl	%ecx, -28(%ebp)

		/ real code
		movl	%ebp, %eax
		addl	$4, %eax
		pushl	%eax
		pushl	%ebx / nTablePos

		/ set ebx to GOT
.L_GOT_BEGIN:
		call	.L_GOT_END
.L_GOT_END:
		popl	%ebx
		addl	$_GLOBAL_OFFSET_TABLE_+[.-.L_GOT_END],%ebx

		/ call collector
		call	__1cHsunpro5Pcpp_vtable_call6Fippv_v_@PLT
.L22:
		movl	-28(%ebp), %ecx
		cmp		$0, %ecx / float
		jne		.L23
		flds	-8(%ebp)
		jmp		.L29
.L23:
		cmp		$1, %ecx / double
		jne		.L24
		fldl	-8(%ebp)
		jmp		.L29
.L24:
		cmp		$2, %ecx / longlong
		jne		.L25
		movl	-4(%ebp), %edx
		movl	-8(%ebp), %eax
		jmp		.L29
.L25:
		cmp		$3, %ecx / long
		jne		.L28
		movl	-8(%ebp), %eax
		jmp		.L29
.L28:
		movl	-16(%ebp), %eax
.L29:		
		movl	-12(%ebp), %ebx
		movl	-20(%ebp), %ecx
		movl	%ebp, %esp
		popl	%ebp
		ret
	.type privateSnippetExecutor, @function
	.size privateSnippetExecutor, .-privateSnippetExecutor
		
	.globl privateSnippetExceptionHandler
privateSnippetExceptionHandler:
	movl	-12(%ebp), %ebx
	movl	-16(%ebp), %eax
	movl	-20(%ebp), %ecx
	movl	-24(%ebp), %esp
	call	__1cG__CrunMex_rethrow_q6F_v_@PLT
	ret
	.type privateSnippetExceptionHandler, @function
	.size privateSnippetExceptionHandler, .-privateSnippetExceptionHandler

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

	.4byte		.L21@rel
	.4byte		.L22-.L21
	.4byte		privateSnippetExceptionHandler-.L21
	.zero		8
	.4byte		.callBeginPosition@rel
	.4byte		.callVirtualMethodExceptionPosition-.callBeginPosition
	.4byte		callVirtualMethodExceptionHandler-.callBeginPosition
	.zero		8
