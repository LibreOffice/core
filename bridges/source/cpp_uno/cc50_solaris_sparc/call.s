.global privateSnippetExecutor
.type privateSnippetExecutor,2
privateSnippetExecutor:
		save %sp, -104, %sp
		! %i0 saved by codeSnippet
		st %i1, [%fp+72]
		st %i2, [%fp+76]
		st %i3, [%fp+80]
		st %i4, [%fp+84]
		st %i5, [%fp+88]
		mov %g1, %o0
		mov %fp, %o1
		call __1cHsunpro5Pcpp_vtable_call6Fippv_i_
		nop
.privateSnippetExecutorExceptionPosition:
		subcc %o0, 11, %g0
		bne .noDouble
		nop
		ld [%fp+68], %i0
		ld [%fp+72], %i1
		std %i0, [%fp-8]
		ldd [%fp-8], %f0
		ba .noFloat
		nop
.noDouble:
		subcc %o0, 10, %g0
		bne .noFloat
		nop
		ld [%fp+68], %f0
.noFloat:
		ld [%fp+68], %i0
		ld [%fp+72], %i1
		ld [%fp+76], %i2
		ld [%fp+80], %i3
		ld [%fp+84], %i4
		ld [%fp+88], %i5
		ret
		restore
.size privateSnippetExecutor,(.-privateSnippetExecutor)
.align 8

	
.global callVirtualMethod
.type callVirtualMethod,2
callVirtualMethod:
		! allocate FIRST stack to have own local registers
		sethi		 %hi(-96), %g1
		or		 %g1, %lo(-96), %g1
		subcc		 %g1, %o5, %g1
		subcc		 %g1, %o5, %g1
		subcc		 %g1, %o5, %g1
		subcc		 %g1, %o5, %g1
		save		%sp, %g1, %sp
		! copy stack longs if necessary
		subcc		%i5, 6, %l5
		ble		.copyRegisters
		nop

		! prepare source location
		add		%i4, 24, %l4
		
		! prepare real stack
		add		%sp, 92, %l3
		
.copyLong:
		ld		[%l4+0], %l0
		st		%l0, [%l3]
		add		%l4, 4, %l4
		add		%l3, 4, %l3
		deccc		%l5
		bne		.copyLong
		nop
.copyRegisters:
		mov		%i5, %l5
		mov		%i4, %l4

		ld		[%l4], %o0
		add		%l4, 4, %l4
		deccc		%l5
		ble		.doCall

		ld		[%l4], %o1
		add		%l4, 4, %l4
		deccc		%l5
		ble		.doCall

		ld		[%l4], %o2
		add		%l4, 4, %l4
		deccc		%l5
		ble		.doCall

		ld			[%l4], %o3
		add		%l4, 4, %l4
		deccc		%l5
		ble		.doCall

		ld			[%l4], %o4
		add		%l4, 4, %l4
		deccc		%l5
		ble		.doCall

		ld			[%l4], %o5
		add		%l4, 4, %l4

		! prepare complex return pointer
		st			%i2, [%sp+64]
.doCall:
		! get virtual table entry
		mov		%i1, %l1
		add		%l1, 2, %l1
		sll		%l1, 2, %l1
		ld			[%i0], %l3
		add		%l3, %l1, %l1
		ld			[%l1], %l0
		jmpl		%l0,%o7
		nop
.callVirtualMethodExceptionPosition:
		! handle returns

		!byte types
		subcc %i3, 2, %l3		! typelib_TypeClass_BOOLEAN
		be .handleByte
		subcc %i3, 3, %l3		! typelib_TypeClass_BYTE
		be .handleByte

		! half word types
		subcc %i3, 4, %l3		! typelib_TypeClass_SHORT
		be .handleShort
		subcc %i3, 5, %l3		! typelib_TypeClass_UNSIGNED_SHORT
		be .handleShort
		subcc %i3, 1, %l3		! typelib_TypeClass_CHAR (sal_Unicode==sal_uInt16)
		be .handleShort

		! word types
		subcc %i3, 6, %l3		! typelib_TypeClass_LONG
		be .handleWord
		subcc %i3, 7, %l3		! typelib_TypeClass_UNSIGNED_LONG
		be .handleWord
		subcc %i3, 15, %l3	! typelib_TypeClass_ENUM
		be .handleWord

		! double word types
		subcc %i3, 8, %l3		! typelib_TypeClass_HYPER
		be .handleDoubleWord
		subcc %i3, 9, %l3		! typelib_TypeClass_UNSIGNED_HYPER
		be .handleDoubleWord

		! float
		subcc %i3, 10, %l3	! typelib_TypeClass_FLOAT
		be .handleFloat

		! double
		subcc %i3, 11, %l3	! typelib_TypeClass_DOUBLE
		be .handleDouble

		! default: return void
		nop					! empty prefetch
		ba .doRestore
		nop
.handleByte:
		stb %o0, [%i2]
		ba .doRestore
		nop
.handleShort:
		sth %o0, [%i2]
		ba .doRestore
		nop
.handleWord:
		st %o0, [%i2]
		ba .doRestore
		nop
.handleDoubleWord:
		st %o0, [%i2]
		st %o1, [%i2+4]
		ba .doRestore
		nop
.handleFloat:
		st %f0, [%i2]
		ba .doRestore
		nop
.handleDouble:
		std %f0, [%fp-8]
		ldd [%fp-8], %o0
		st %o0, [%i2]
		st %o1, [%i2+4]
		ba .doRestore
		nop
.doRestore:
		ret
		restore	! stack frame for own locals
.size callVirtualMethod,(.-callVirtualMethod)
.align 8

.rethrow_handler:
		call	__1cG__CrunMex_rethrow_q6F_v_
		nop

.section	".exception_ranges",#alloc
.word		%r_disp32(.privateSnippetExecutorExceptionPosition)
.word		0
.word		.rethrow_handler-.privateSnippetExecutorExceptionPosition
.word		0,0
.word		%r_disp32(.callVirtualMethodExceptionPosition)
.word		0
.word		.rethrow_handler-.callVirtualMethodExceptionPosition
.word		0,0
