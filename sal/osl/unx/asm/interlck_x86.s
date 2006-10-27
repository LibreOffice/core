/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interlck_x86.s,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 11:59:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

