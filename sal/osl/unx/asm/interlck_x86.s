/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


.section   .text,"ax"
 .globl   osl_incrementInterlockedCount

osl_incrementInterlockedCount:

    push       %ebp
    mov        %esp,%ebp
    push       %ebx
    call       1f
1:
    pop        %ebx
    add        $_GLOBAL_OFFSET_TABLE_+0x1,%ebx
    mov        8(%ebp),%ecx
    mov        $1,%eax
    mov        osl_isSingleCPU@GOT(%ebx),%edx
    cmp        $0,(%edx)
    je         2f
    xadd       %eax,(%ecx)
    jmp        3f
2:
    lock
    xadd       %eax,(%ecx)
3:
    inc        %eax
    pop        %ebx
    mov        %ebp,%esp
    pop        %ebp
    ret        
 
 .type  osl_incrementInterlockedCount,@function
 .size  osl_incrementInterlockedCount,.-osl_incrementInterlockedCount

.section   .text,"ax"
 .globl   osl_decrementInterlockedCount

osl_decrementInterlockedCount:

    push       %ebp
    mov        %esp,%ebp
    push       %ebx
    call       1f
1:
    pop        %ebx
    add        $_GLOBAL_OFFSET_TABLE_+0x1,%ebx
    mov        8(%ebp),%ecx
    orl        $-1,%eax
    mov        osl_isSingleCPU@GOT(%ebx),%edx
    cmp        $0,(%edx)
    je         2f
    xadd       %eax,(%ecx)
    jmp        3f
2:  
    lock 
    xadd       %eax,(%ecx)
3:
    dec        %eax
    pop        %ebx
    mov        %ebp,%esp
    pop        %ebp
    ret        
 
 .type  osl_decrementInterlockedCount,@function
 .size  osl_decrementInterlockedCount,.-osl_decrementInterlockedCount

