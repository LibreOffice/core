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
