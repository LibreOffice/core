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
