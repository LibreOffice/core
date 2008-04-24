/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vtableslotcall.s,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-04-24 16:27:25 $
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
