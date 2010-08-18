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

.global vtableCall

.global vtableSlotCall
.align 8
vtableSlotCall:
    ! save %sp, -176, %sp  already done in code snippet
    stx %i0, [%fp + 2047 + 128]
    stx %i1, [%fp + 2047 + 136]
    stx %i2, [%fp + 2047 + 144]
    stx %i3, [%fp + 2047 + 152]
    stx %i4, [%fp + 2047 + 160]
    stx %i5, [%fp + 2047 + 168]
    ! %o0: functionIndex, stored by code snippet
    ! %o1: vtableOffset, stored by code snippet
    call vtableCall
    add %fp, 2047 + 128, %o2
    ldx [%fp + 2047 + 128], %i0
    ldx [%fp + 2047 + 136], %i1
    ldx [%fp + 2047 + 144], %i2
    ldx [%fp + 2047 + 152], %i3
    ret
    restore
.size vtableSlotCall, . - vtableSlotCall
.type vtableSlotCall, #function
