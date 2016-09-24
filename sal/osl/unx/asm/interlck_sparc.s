/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


/*
 * Implements osl_[increment|decrement]InterlockedCount:
 * sparcv9/sparcv8plus architecture:    use the "cas"  instruction
 * 
 * 32 bit mode with v8plus support or 64 bit mode:
 * sparcv9 mode is implied. Assemble with -xarch=v8plus (32 bit) or
 * -xarch=v9 (64 bit).
 * 
 */

#if !defined(__sparcv8plus) && !defined(__sparcv9) && !defined(__sparc_v9__)

#error LibreOffice requires SPARCv8plus or SPARCv9 CPU with "cas" instruction

#endif

.section   ".text"
 .global  osl_incrementInterlockedCount
 .align   8

!   Implements osl_[increment|decrement]InterlockedCount with sparcv9(sparcv8plus) "cas" 
!   instruction.

osl_incrementInterlockedCount:

1:      ld      [%o0], %o1
        add     %o1, 1, %o2
!       allow linux to build for v8
        .word 0xD5E21009 
!       cas     [%o0], %o1, %o2
        cmp     %o1, %o2
        bne     1b
        nop                         ! delay slot
        retl
        add     %o2, 1, %o0         ! delay slot

 .type  osl_incrementInterlockedCount,#function
 .size  osl_incrementInterlockedCount,.-osl_incrementInterlockedCount


.section   ".text"
 .global  osl_decrementInterlockedCount
 .align   8

osl_decrementInterlockedCount:

1:      ld      [%o0], %o1
        sub     %o1, 1, %o2
!       allow linux to build for v8
        .word 0xD5E21009            
!       cas     [%o0], %o1, %o2
        cmp     %o1, %o2
        bne     1b
        nop                         ! delay slot
        retl
        sub     %o2, 1, %o0         ! delay slot

 .type  osl_decrementInterlockedCount,#function
 .size  osl_decrementInterlockedCount,.-osl_decrementInterlockedCount

