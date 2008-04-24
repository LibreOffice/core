/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callvirtualmethod.s,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-04-24 16:24:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2008 by Sun Microsystems, Inc.
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

.global callVirtualMethod
.align 8
callVirtualMethod:
    ! %o0: unsigned long function
    ! %o1: unsigned long * stack
    ! %o2: sal_Int32 excess
    add %o2, 176, %o3
    neg %o3
    save %sp, %o3, %sp
    ldx [%i1 + 0 * 8], %o0
    ldx [%i1 + 1 * 8], %o1
    ldx [%i1 + 2 * 8], %o2
    ldx [%i1 + 3 * 8], %o3
    tst %i2
    bz,pt %xcc, 1f
    ldx [%i1 + 4 * 8], %o4
    add %i1, 6 * 8, %l0
    add %sp, 2047 + 176, %l1
0:  deccc 8, %i2
    ldx [%l0 + %i2], %l2
    bnz,pt %xcc, 0b
    stx %l2, [%l1 + %i2]
1:  call %i0
    ldx [%i1 + 5 * 8], %o5
    stx %o0, [%i1 + 0 * 8]
    stx %o1, [%i1 + 1 * 8]
    stx %o2, [%i1 + 2 * 8]
    stx %o3, [%i1 + 3 * 8]
    ret
    restore
.size callVirtualMethod, . - callVirtualMethod
.type callVirtualMethod, #function
