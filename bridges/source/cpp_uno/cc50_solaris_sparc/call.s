.global callVirtualMethod
.type callVirtualMethod,2
callVirtualMethod:
		! allocate FIRST stack to have own local registers
		save		%sp, -96, %sp
		! copy in to out parameters for second stackframe
		mov		%i0, %o0
		mov		%i1, %o1
		mov		%i2, %o2
		mov		%i3, %o3
		mov		%i4, %o4
		mov		%i5, %o5
		
		! decide wether there are more than 6 parameter
		mov		-96, %l3			! default stack space
		subcc		%i5, 6, %l5
		ble		allocateSecondStack
		nop
		sll		%l5, 2, %l5
		sub		%l3, %l5, %l3
allocateSecondStack:
		save		%sp, %l3, %sp		! allocate new stack
		! copy stack longs if necessary
		subcc		%i5, 6, %l5
		ble		copyRegisters
		nop

		! prepare source location
		add		%i4, 24, %l4
		
		! prepare real stack
		add		%sp, 92, %l3
		
copyLong:
		ld			[%l4+0], %l0
		st			%l0, [%l3]
		add		%l4, 4, %l4
		add		%l3, 4, %l3
		deccc		%l5
		bne		copyLong
		nop
copyRegisters:
		mov		%i5, %l5
		mov		%i4, %l4

		ld			[%l4], %o0
		add		%l4, 4, %l4
		deccc		%l5
		ble		doCall

		ld			[%l4], %o1
		add		%l4, 4, %l4
		deccc		%l5
		ble		doCall

		ld			[%l4], %o2
		add		%l4, 4, %l4
		deccc		%l5
		ble		doCall

		ld			[%l4], %o3
		add		%l4, 4, %l4
		deccc		%l5
		ble		doCall

		ld			[%l4], %o4
		add		%l4, 4, %l4
		deccc		%l5
		ble		doCall

		ld			[%l4], %o5
		add		%l4, 4, %l4

		! prepare complex return pointer
		st			%i2, [%sp+64]
doCall:
		! get virtual table entry
		mov		%i1, %l1
		add		%l1, 2, %l1
		sll		%l1, 2, %l1
		ld			[%i0], %l3
		add		%l3, %l1, %l1
		ld			[%l1], %l0
		jmpl		%l0,%o7
		nop
.global callVirtualMethodExceptionHandler
.type callVirtualMethodExceptionHandler,2
callVirtualMethodExceptionHandler:
		! handle returns

		!byte types
		subcc %i3, 2, %l3		! typelib_TypeClass_BOOLEAN
		be handleByte
		subcc %i3, 3, %l3		! typelib_TypeClass_BYTE
		be handleByte

		! half word types
		subcc %i3, 4, %l3		! typelib_TypeClass_SHORT
		be handleShort
		subcc %i3, 5, %l3		! typelib_TypeClass_UNSIGNED_SHORT
		be handleShort
		subcc %i3, 1, %l3		! typelib_TypeClass_CHAR (sal_Unicode==sal_uInt16)
		be handleShort

		! word types
		subcc %i3, 6, %l3		! typelib_TypeClass_LONG
		be handleWord
		subcc %i3, 7, %l3		! typelib_TypeClass_UNSIGNED_LONG
		be handleWord
		subcc %i3, 15, %l3	! typelib_TypeClass_ENUM
		be handleWord

		! double word types
		subcc %i3, 8, %l3		! typelib_TypeClass_HYPER
		be handleDoubleWord
		subcc %i3, 9, %l3		! typelib_TypeClass_UNSIGNED_HYPER
		be handleDoubleWord

		! float
		subcc %i3, 10, %l3	! typelib_TypeClass_FLOAT
		be handleFloat

		! double
		subcc %i3, 11, %l3	! typelib_TypeClass_DOUBLE
		be handleDouble

		! default: return void
		nop					! empty prefetch
		ba doRestore
		nop
handleByte:
		stb %o0, [%i2]
		ba doRestore
		nop
handleShort:
		sth %o0, [%i2]
		ba doRestore
		nop
handleWord:
		st %o0, [%i2]
		ba doRestore
		nop
handleDoubleWord:
		st %o0, [%i2]
		st %o1, [%i2+4]
		ba doRestore
		nop
handleFloat:
		st %f0, [%i2]
		ba doRestore
		nop
handleDouble:
		std %f0, [%fp-8]
		ldd [%fp-8], %o0
		st %o0, [%i2]
		st %o1, [%i2+4]
		ba doRestore
		nop
doRestore:
		restore	! stack frame for called method
		ret
		restore	! stack frame for own locals
.size callVirtualMethodExceptionHandler,(.-callVirtualMethodExceptionHandler)
.size callVirtualMethod,(.-callVirtualMethod)
.align 8
