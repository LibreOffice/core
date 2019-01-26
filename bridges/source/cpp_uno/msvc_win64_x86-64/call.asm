;
; Licensed to the Apache Software Foundation (ASF) under one
; or more contributor license agreements.  See the NOTICE file
; distributed with this work for additional information
; regarding copyright ownership.  The ASF licenses this file
; to you under the Apache License, Version 2.0 (the
; "License"); you may not use this file except in compliance
; with the License.  You may obtain a copy of the License at
; 
;   http://www.apache.org/licenses/LICENSE-2.0
; 
; Unless required by applicable law or agreed to in writing,
; software distributed under the License is distributed on an
; "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
; KIND, either express or implied.  See the License for the
; specific language governing permissions and limitations
; under the License.
; 


typelib_TypeClass_VOID equ 0
typelib_TypeClass_CHAR equ 1
typelib_TypeClass_BOOLEAN equ 2
typelib_TypeClass_BYTE equ 3
typelib_TypeClass_SHORT equ 4
typelib_TypeClass_UNSIGNED_SHORT equ 5
typelib_TypeClass_LONG equ 6
typelib_TypeClass_UNSIGNED_LONG equ 7
typelib_TypeClass_HYPER equ 8
typelib_TypeClass_UNSIGNED_HYPER equ 9
typelib_TypeClass_FLOAT equ 10
typelib_TypeClass_DOUBLE equ 11
typelib_TypeClass_ENUM equ 15

EXTERN cpp_vtable_call: PROC

.CODE

;
;               | ...                        |
;               +----------------------------+
;               | argument 4                 |
; rbp+48 rsp+40 +----------------------------+ -------
;               | argument 3, r9/xmm3 home   | ^ shadow
; rbp+40 rsp+32 +----------------------------+ | space,
;               | argument 2, r8/xmm2 home   | | guaranteed to be present but uninitialized,
; rbp+32 rsp+24 +----------------------------+ | we have to copy
;               | argument 1, rdx/xmm1 home  | | the first 4 parameters there from the registers,
; rbp+24 rsp+16 +----------------------------+ | to form the continuous array of arguments.
;               | argument 0, rcx/xmm0 home  | v
; rbp+16 rsp+08 +----------------------------+ -------
;               | return address             |
; rbp+08 rsp--> +----------------------------+
;               | old rbp                    |
; rbp---------> +----------------------------+
;               | pRegisterReturn memory     |
; rbp-08 -----> +----------------------------+
;               |                            |
; rbp-16 -----> +----------------------------+ -------
;               |                            | ^
; rbp-24 -----> +----------------------------+ | shadow space
;               |                            | | for cpp_vtable_call
; rbp-32 -----> +----------------------------+ |
;               |                            | |
; rbp-40 -----> +----------------------------+ |
;               |                            | v
; rbp-48 -----> +----------------------------+ -------
;
; rax = functionIndex
; r10 = vtableOffset
; r11 = &privateSnippetExecutor
;

privateSnippetExecutor PROC FRAME

	push	rbp
	mov	rbp, rsp
	sub	rsp, 48
	.ALLOCSTACK(48)
	.ENDPROLOG

	; 4th param: sal_uInt64 *pRegisterReturn 
	lea r9, -8[rbp]

	; 3rd param: sal_Int32 nVtableOffset
	mov r8, r10

	; 2nd param: sal_Int32 nFunctionIndex
	mov rdx, rax

	; 1st param: void ** pCallStack
	lea rcx, 8[rbp]

	call cpp_vtable_call

	; Integers would return in RAX and floats in XMM0, but both are free for us to clobber,
	; and the caller knows where to look:
	mov rax, -8[rbp]
	movsd xmm0, qword ptr -8[rbp]

	leave
	ret

privateSnippetExecutor ENDP


;
;               | ...                                         |
; rbp+64 -----> +---------------------------------------------+
;               | sal_uInt32 nStack                           |
; rbp+56 -----> +---------------------------------------------+
;               | sal_uInt64 *pStack                          |
; rbp+48 -----> +---------------------------------------------+ -------
;               | typelib_TypeClass eReturnTypeClass, r9 home | ^ shadow
; rbp+40 -----> +---------------------------------------------+ | space,
;               | void *pRegisterReturn,              r8 home | | guaranteed to be present but uninitialized,
; rbp+32 -----> +---------------------------------------------+ | we have to copy
;               | sal_Int32 nVtableIndex,            rdx home | | the first 4 parameters there from the registers,
; rbp+24 -----> +---------------------------------------------+ | to form the continuous array of arguments.
;               | void* pAdjustedThisPtr,            rcx home | v
; rbp+16 -----> +---------------------------------------------+ -------
;               | return address                              |
; rbp+08 -----> +---------------------------------------------+
;               | old rbp                                     |
; rbp --------> +---------------------------------------------+ <---- 16 byte boundary
;               | (possible 16 byte alignment placeholder)    |
; rbp-08 -----> +---------------------------------------------+
;               | (stack for virtual method)                  |
;               | ...                                         |
;               | (shadow space for virtual method)           |
; rsp --------> +---------------------------------------------+ <---- 16 byte boundary

callVirtualMethod PROC FRAME

	push rbp
	mov rbp, rsp
	.ENDPROLOG

	; Save our register arguments to the shadow space:
	mov 16[rbp], rcx
	mov 24[rbp], rdx
	mov 32[rbp], r8
	mov 40[rbp], r9

	; We must maintain the stack aligned to a 16 byte boundary:
	mov eax, 56[rbp]
	cmp rax, 0
	je stackIsEmpty
	mov r11, rax
	test rax,1
	jz stackSizeEven
	sub rsp, 8
stackSizeEven:
	mov r10, 48[rbp]
	shl rax, 3        ; nStack is in units of sal_uInt64, and sizeof(sal_uInt64) == 8
	add rax, r10

copyStack:
	sub rax, 8
	push [rax]
	dec r11
	jne copyStack

	; First 4 args are passed in registers. Floating point args needs to be
	; in floating point registers, but those are free for us to clobber
	; anyway, and the callee knows where to look, so put each arg in both
	; its general purpose and its floating point register:
	mov rcx, [rsp]
	movsd xmm0, qword ptr [rsp]
	mov rdx, 8[rsp]
	movsd xmm1, qword ptr 8[rsp]
	mov r8, 16[rsp]
	movsd xmm2, qword ptr 16[rsp]
	mov r9, 24[rsp]
	movsd xmm3, qword ptr 24[rsp]
	jmp callMethod

stackIsEmpty:
	sub rsp, 32       ; we still need shadow space
	
callMethod:
	; Find the method pointer
	mov rax, 16[rbp]
	mov r10, [rax]    ; pointer to vtable
	mov r11d, 24[rbp]
	shl r11, 3        ; sizeof(void*) == 8
	add r10, r11
	call qword ptr [r10]

	mov r10d, 40[rbp]
	cmp r10, typelib_TypeClass_VOID
	je cleanup
	cmp r10, typelib_TypeClass_LONG
	je Lint32
	cmp r10, typelib_TypeClass_UNSIGNED_LONG
	je Lint32
	cmp r10, typelib_TypeClass_ENUM
	je Lint32
	cmp r10, typelib_TypeClass_BOOLEAN
	je Lint8
	cmp r10, typelib_TypeClass_BYTE
	je Lint8
	cmp r10, typelib_TypeClass_CHAR
	je Lint16
	cmp r10, typelib_TypeClass_SHORT
	je Lint16
	cmp r10, typelib_TypeClass_UNSIGNED_SHORT
	je Lint16
	cmp r10, typelib_TypeClass_FLOAT
	je Lfloat
	cmp r10, typelib_TypeClass_DOUBLE
	je Lfloat
	cmp r10, typelib_TypeClass_HYPER
	je Lint64
	cmp r10, typelib_TypeClass_UNSIGNED_HYPER
	je Lint64

	; https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention?view=vs-2017
	; "The same pointer must be returned by the callee in RAX."
	jmp Lint64

Lint64:
	mov 32[rbp], rax
	jmp cleanup

Lint32:
	mov 32[rbp], eax
	jmp cleanup

Lint16:
	mov 32[rbp], ax
	jmp cleanup

Lint8:
	mov 32[rbp], al
	jmp cleanup

Lfloat:
	movsd qword ptr 32[rbp], xmm0
	jmp cleanup

cleanup:
	leave
	ret

callVirtualMethod ENDP


END
