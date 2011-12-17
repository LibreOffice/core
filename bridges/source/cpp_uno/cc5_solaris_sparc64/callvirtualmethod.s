!/**************************************************************
! * 
! * Licensed to the Apache Software Foundation (ASF) under one
! * or more contributor license agreements.  See the NOTICE file
! * distributed with this work for additional information
! * regarding copyright ownership.  The ASF licenses this file
! * to you under the Apache License, Version 2.0 (the
! * "License"); you may not use this file except in compliance
! * with the License.  You may obtain a copy of the License at
! * 
! *   http://www.apache.org/licenses/LICENSE-2.0
! * 
! * Unless required by applicable law or agreed to in writing,
! * software distributed under the License is distributed on an
! * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
! * KIND, either express or implied.  See the License for the
! * specific language governing permissions and limitations
! * under the License.
! * 
! *************************************************************/



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
