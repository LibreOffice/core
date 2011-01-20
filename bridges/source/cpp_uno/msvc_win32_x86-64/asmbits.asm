; -*- Mode: text; tab-width: 8; indent-tabs-mode: nil comment-column: 32; comment-start: "; " comment-start-skip: "; *" -*-

;*************************************************************************
;*
;* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
;* 
;* Copyright 2000, 2011 Oracle and/or its affiliates.
;*
;* OpenOffice.org - a multi-platform office productivity suite
;*
;* This file is part of OpenOffice.org.
;*
;* OpenOffice.org is free software: you can redistribute it and/or modify
;* it under the terms of the GNU Lesser General Public License version 3
;* only, as published by the Free Software Foundation.
;*
;* OpenOffice.org is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU Lesser General Public License version 3 for more details
;* (a copy is included in the LICENSE file that accompanied this code).
;*
;* You should have received a copy of the GNU Lesser General Public License
;* version 3 along with OpenOffice.org.  If not, see
;* <http://www.openoffice.org/license.html>
;* for a copy of the LGPLv3 License.
;*
;************************************************************************

; Emacs asm-mode is not really ideal for the convention in this
; file. So I use text-mode with custom comment syntax... not really
; ideal either. Maybe I should just re-format this to match asm-mode's
; conventions?

; NOTE: EXTREMELY UNFINISHED, I KNOW. WORK IN PROGRESS.

; I really don't have a good high-level understanding of the big
; picture and what the actual task of the C++/UNO bridge is yet... I
; should debug the x86 version and see what is actually going on to
; get an understanding.

; This is in a separate file for x86-64 as MSVC doesn't have in-line
; assembly for x64. The code here is still partly just a crude copy of
; the in-line x86 code from ../msvc_win32_intel that is totally
; pointless on x64. But parts have been properly changed into x64
; calling convention and might even work.

; Random web links and other documentation about low-level
; implementation details for the C++/UNO bridge on x64 Windows kept
; here:

; Caolan's "Lazy Hackers Guide To Porting" is useful:
; http://wiki.services.openoffice.org/wiki/Lazy_Hackers_Guide_To_Porting

; As for details about the x64 Windows calling convention, register
; usage, stack usage, exception handling etc, the official
; documentation (?) on MSDN is a bit fragmented and split up into a
; needlessly large number of short pages. But still:
; http://msdn.microsoft.com/en-us/library/7kcdt6fy%28v=VS.90%29.aspx

; Also see Raymond Chen's blog post:
; http://blogs.msdn.com/b/oldnewthing/archive/2004/01/14/58579.aspx

; This one is actually more readable: "Improving Automated Analysis of
; Windows x64 Binaries": 
; http://www.uninformed.org/?v=4&a=1

; The bridge uses dynamic code generation. For exception handling and
; unwinding to work across that (as I assume we want?), one apparently
; needs to use either RtlAddFunctionTable() or
; RtlInstallFunctionTableCallback(). See Windows SDK documentation.

; Random interesting discussion threads:
; http://social.msdn.microsoft.com/Forums/en/vcgeneral/thread/300bd6d3-9381-4d2d-8129-e48b392c05d8

; Ken Johnson's blog http://www.nynaeve.net/ has much interesting
; information, for instance:
; http://www.nynaeve.net/?p=11

; From <typelib/typeclass.h>
typelib_TypeClass_VOID = 0
typelib_TypeClass_CHAR = 1
typelib_TypeClass_BOOLEAN = 2
typelib_TypeClass_BYTE = 3
typelib_TypeClass_SHORT = 4
typelib_TypeClass_UNSIGNED_SHORT = 5
typelib_TypeClass_LONG = 6
typelib_TypeClass_UNSIGNED_LONG = 7
typelib_TypeClass_HYPER = 8
typelib_TypeClass_UNSIGNED_HYPER = 9
typelib_TypeClass_FLOAT = 10
typelib_TypeClass_DOUBLE = 11
typelib_TypeClass_ENUM = 15

extrn __copyConstruct :  proc
extrn __destruct :  proc
extrn cpp_mediate:   proc

.code

; First we have three code snippets that aren't real functions, but
; "naked" (in x86 calling convention parlance) functions jumped to
; from code generated in C++. 

; I think that for x64 Windows we shouldn't be jumping so wildly from
; generated code snippets into fixed code snippets here. "Procedures"
; should be continuous in memory, or the unwinding information will be
; broken. (For dynamically generated code, that is dynamically
; registered unwind information, obviously.)

; So instead of generating jumps into here, we should just make the
; C++ code copy the machine code that these three "naked" functions
; compile after the snippet it is generating. Of course, that means
; the code here needs to be position independent, or (eek) that we
; would need to come up with some home-brewed minimal relocation
; mechanism.

; Jumped to from code generated in except.cxx: ObjectFunction::ObjectFunction()

copyConstruct proc
        ; ObjectFunction this already on stack
        push    [rsp+8]         ; source exc object this
        push    rcx             ; exc object
        call    __copyConstruct
        add     rsp, 12         ; + ObjectFunction this
        ret     4
copyConstruct endp

; Ditto

destruct proc
        ; ObjectFunction this already on stack
        push    rcx             ; exc object
        call    __destruct
        add     rsp, 8              ; + ObjectFunction this
        ret
destruct endp

; Jumped to from code generated in cpp2uno.cxx:codeSnippet()

cpp_vtable_call proc
        sub     rsp, 8          ; space for immediate return type
        push    rsp
        push    rdx             ; vtable offset
        push    rax             ; function index
        mov     rax, rsp
        add     rax, 20
        push    rax             ; original stack ptr

        call    cpp_mediate
        add     rsp, 16

        cmp     rax, typelib_TypeClass_FLOAT
        je      Lfloat
        cmp     rax, typelib_TypeClass_DOUBLE
        je      Ldouble
        cmp     rax, typelib_TypeClass_HYPER
        je      Lhyper
        cmp     eax, typelib_TypeClass_UNSIGNED_HYPER
        je      Lhyper
        ; rest is eax
        pop     rax
        add     rsp, 4
        ret
Lhyper:
        pop     rax
        pop     rdx
        ret
Lfloat:
        fld     dword ptr [rsp]
        add     rsp, 8
        ret
Ldouble:
        fld     qword ptr [rsp]
        add     rsp, 8
        ret
cpp_vtable_call endp

; Called from uno2cpp.cxx

; This one is now hopefully proper x64 Windows code following the
; appropriate conventions and might actually work.

; void callVirtualMethod(                       ; location on entry
;    void * pAdjustedThisPtr,                   ; rcx
;    sal_Int32 nVtableIndex,                    ; rdx
;    void * pReturn,                            ; r8
;    typelib_TypeClass eReturnTypeClass,        ; r9
;    sal_Int64 * pStack,                        ; ((4+1)*8)[rsp]
;    sal_Int32 nStack                           ; ((5+1)*8)[rsp]
;    sal_uInt64 *pGPR,                          ; ((6+1)*8)[rsp]
;    double *pFPR)                              ; ((7+1)*8)[rsp]

callVirtualMethod proc frame
        ; Prologue
        mov     ((3+1)*8)[rsp], r9
        mov     ((2+1)*8)[rsp], r8
        mov     ((0+1)*8)[rsp], rcx
        push    rsi
        .pushreg rsi
        push    rdi
        .pushreg rdi

        ; Our stack frame size is 24 qwords (space for 20 parameters
        ; to pass, plus the two pushed registers rsi and rdi, plus
        ; return address, rounded up to 16 bytes)

        sub     rsp, (20+1)*8
        .allocstack (20+1)*8
        .endprolog

        ; Stack parameters first

        ; nStack is number of qwords of stack space (including spilled
        ; registers). If four or less, 4 is passed in anyway to make
        ; code here simpler.

        mov     rcx, ((24+5)*8)[rsp] ; nStack
        sub     rcx, 4
        jle     Lxmmregs

        mov     rsi, ((24+4)*8)[rsp] ; pStack
        add     rsi, 32
        lea     rdi, 32[rsp]
        rep movsq

Lxmmregs:
        ; Parameters passed in XMM registers

        ; We don't bother checking which actually needed, if any.
        mov     rax, ((24+7)*8)[rsp] ; pFPR
        movsd   xmm0, qword ptr [rax]
        movsd   xmm1, qword ptr 8[rax]
        movsd   xmm2, qword ptr 16[rax]
        movsd   xmm3, qword ptr 24[rax]

        ; Prepare pointer to function to call
        mov     rcx, ((24+0)*8)[rsp]
        mov     r12, [rcx]      ; pAdjustedThisPtr->vtable
        shl     rdx, 3          ; nVtableIndex *= 8
        add     r12, rdx

        ; Fill parameters passed in general purpose registers
        mov     rax, ((24+6)*8)[rsp] ; pGPR
        mov     rcx, (0*8)[rax]
        mov     rdx, (1*8)[rax]
        mov     r8, (2*8)[rax]
        mov     r9, (3*8)[rax]

        call    qword ptr [r12]

        ; Test return type
        mov     r9, ((24+3)*8)[rsp]
        cmp     r9, typelib_TypeClass_VOID
        je      Lepilog

        ; int32
        cmp     r9, typelib_TypeClass_LONG
        je      Lint32
        cmp     r9, typelib_TypeClass_UNSIGNED_LONG
        je      Lint32
        cmp     r9, typelib_TypeClass_ENUM
        je      Lint32

        ; int8
        cmp     r9, typelib_TypeClass_BOOLEAN
        je      Lint8
        cmp     r9, typelib_TypeClass_BYTE
        je      Lint8

        ; int16
        cmp     r9, typelib_TypeClass_CHAR
        je      Lint16
        cmp     r9, typelib_TypeClass_SHORT
        je      Lint16
        cmp     r9, typelib_TypeClass_UNSIGNED_SHORT
        je      Lint16

        ; int64
        cmp     r9, typelib_TypeClass_HYPER
        je      Lint64
        cmp     r9, typelib_TypeClass_UNSIGNED_HYPER
        je      Lint64

        ; float
        cmp     r9, typelib_TypeClass_FLOAT
        je      Lfloat

        ; double
        cmp     r9, typelib_TypeClass_DOUBLE
        je      Ldouble

        jmp     Lepilog        ; no simple type

Lint8:
        mov     byte ptr [r8], al
        jmp     Lepilog

Lint16:
        mov     word ptr [r8], ax
        jmp     Lepilog

Lint32:
        mov     dword ptr [r8], eax
        jmp     Lepilog

Lint64:
        mov     qword ptr [r8], rax
        jmp     Lepilog

Lfloat:
        movss   dword ptr [r8], xmm0
        jmp     Lepilog

Ldouble:
        movsd   qword ptr [r8], xmm0

Lepilog:
        ; Epilogue
        add     rsp, (20+1)*8
        pop     rdi
        pop     rsi
        ret
callVirtualMethod endp
    
end

; vim:set shiftwidth=4 softtabstop=4 expandtab:
