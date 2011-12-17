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



.global fp_loadf0
.align 8
fp_loadf0:
    retl
    ld [%o0], %f0
.size fp_loadf0, . - fp_loadf0
.type fp_loadf0, #function

.global fp_loadf1
.align 8
fp_loadf1:
    retl
    ld [%o0], %f1
.size fp_loadf1, . - fp_loadf1
.type fp_loadf1, #function

.global fp_loadf2
.align 8
fp_loadf2:
    retl
    ld [%o0], %f2
.size fp_loadf2, . - fp_loadf2
.type fp_loadf2, #function

.global fp_loadf3
.align 8
fp_loadf3:
    retl
    ld [%o0], %f3
.size fp_loadf3, . - fp_loadf3
.type fp_loadf3, #function

.global fp_loadf4
.align 8
fp_loadf4:
    retl
    ld [%o0], %f4
.size fp_loadf4, . - fp_loadf4
.type fp_loadf4, #function

.global fp_loadf5
.align 8
fp_loadf5:
    retl
    ld [%o0], %f5
.size fp_loadf5, . - fp_loadf5
.type fp_loadf5, #function

.global fp_loadf6
.align 8
fp_loadf6:
    retl
    ld [%o0], %f6
.size fp_loadf6, . - fp_loadf6
.type fp_loadf6, #function

.global fp_loadf7
.align 8
fp_loadf7:
    retl
    ld [%o0], %f7
.size fp_loadf7, . - fp_loadf7
.type fp_loadf7, #function

.global fp_loadf9
.align 8
fp_loadf9:
    retl
    ld [%o0], %f9
.size fp_loadf9, . - fp_loadf9
.type fp_loadf9, #function

.global fp_loadf11
.align 8
fp_loadf11:
    retl
    ld [%o0], %f11
.size fp_loadf11, . - fp_loadf11
.type fp_loadf11, #function

.global fp_loadf13
.align 8
fp_loadf13:
    retl
    ld [%o0], %f13
.size fp_loadf13, . - fp_loadf13
.type fp_loadf13, #function

.global fp_loadf15
.align 8
fp_loadf15:
    retl
    ld [%o0], %f15
.size fp_loadf15, . - fp_loadf15
.type fp_loadf15, #function

.global fp_loadf17
.align 8
fp_loadf17:
    retl
    ld [%o0], %f17
.size fp_loadf17, . - fp_loadf17
.type fp_loadf17, #function

.global fp_loadf19
.align 8
fp_loadf19:
    retl
    ld [%o0], %f19
.size fp_loadf19, . - fp_loadf19
.type fp_loadf19, #function

.global fp_loadf21
.align 8
fp_loadf21:
    retl
    ld [%o0], %f21
.size fp_loadf21, . - fp_loadf21
.type fp_loadf21, #function

.global fp_loadf23
.align 8
fp_loadf23:
    retl
    ld [%o0], %f23
.size fp_loadf23, . - fp_loadf23
.type fp_loadf23, #function

.global fp_loadf25
.align 8
fp_loadf25:
    retl
    ld [%o0], %f25
.size fp_loadf25, . - fp_loadf25
.type fp_loadf25, #function

.global fp_loadf27
.align 8
fp_loadf27:
    retl
    ld [%o0], %f27
.size fp_loadf27, . - fp_loadf27
.type fp_loadf27, #function

.global fp_loadf29
.align 8
fp_loadf29:
    retl
    ld [%o0], %f29
.size fp_loadf29, . - fp_loadf29
.type fp_loadf29, #function

.global fp_loadf31
.align 8
fp_loadf31:
    retl
    ld [%o0], %f31
.size fp_loadf31, . - fp_loadf31
.type fp_loadf31, #function

.global fp_storef0
.align 8
fp_storef0:
    retl
    st %f0, [%o0]
.size fp_storef0, . - fp_storef0
.type fp_storef0, #function

.global fp_storef1
.align 8
fp_storef1:
    retl
    st %f1, [%o0]
.size fp_storef1, . - fp_storef1
.type fp_storef1, #function

.global fp_storef2
.align 8
fp_storef2:
    retl
    st %f2, [%o0]
.size fp_storef2, . - fp_storef2
.type fp_storef2, #function

.global fp_storef3
.align 8
fp_storef3:
    retl
    st %f3, [%o0]
.size fp_storef3, . - fp_storef3
.type fp_storef3, #function

.global fp_storef4
.align 8
fp_storef4:
    retl
    st %f4, [%o0]
.size fp_storef4, . - fp_storef4
.type fp_storef4, #function

.global fp_storef5
.align 8
fp_storef5:
    retl
    st %f5, [%o0]
.size fp_storef5, . - fp_storef5
.type fp_storef5, #function

.global fp_storef6
.align 8
fp_storef6:
    retl
    st %f6, [%o0]
.size fp_storef6, . - fp_storef6
.type fp_storef6, #function

.global fp_storef7
.align 8
fp_storef7:
    retl
    st %f7, [%o0]
.size fp_storef7, . - fp_storef7
.type fp_storef7, #function

.global fp_storef9
.align 8
fp_storef9:
    retl
    st %f9, [%o0]
.size fp_storef9, . - fp_storef9
.type fp_storef9, #function

.global fp_storef11
.align 8
fp_storef11:
    retl
    st %f11, [%o0]
.size fp_storef11, . - fp_storef11
.type fp_storef11, #function

.global fp_storef13
.align 8
fp_storef13:
    retl
    st %f13, [%o0]
.size fp_storef13, . - fp_storef13
.type fp_storef13, #function

.global fp_storef15
.align 8
fp_storef15:
    retl
    st %f15, [%o0]
.size fp_storef15, . - fp_storef15
.type fp_storef15, #function

.global fp_storef17
.align 8
fp_storef17:
    retl
    st %f17, [%o0]
.size fp_storef17, . - fp_storef17
.type fp_storef17, #function

.global fp_storef19
.align 8
fp_storef19:
    retl
    st %f19, [%o0]
.size fp_storef19, . - fp_storef19
.type fp_storef19, #function

.global fp_storef21
.align 8
fp_storef21:
    retl
    st %f21, [%o0]
.size fp_storef21, . - fp_storef21
.type fp_storef21, #function

.global fp_storef23
.align 8
fp_storef23:
    retl
    st %f23, [%o0]
.size fp_storef23, . - fp_storef23
.type fp_storef23, #function

.global fp_storef25
.align 8
fp_storef25:
    retl
    st %f25, [%o0]
.size fp_storef25, . - fp_storef25
.type fp_storef25, #function

.global fp_storef27
.align 8
fp_storef27:
    retl
    st %f27, [%o0]
.size fp_storef27, . - fp_storef27
.type fp_storef27, #function

.global fp_storef29
.align 8
fp_storef29:
    retl
    st %f29, [%o0]
.size fp_storef29, . - fp_storef29
.type fp_storef29, #function

.global fp_storef31
.align 8
fp_storef31:
    retl
    st %f31, [%o0]
.size fp_storef31, . - fp_storef31
.type fp_storef31, #function

.global fp_loadd0
.align 8
fp_loadd0:
    retl
    ldd [%o0], %f0
.size fp_loadd0, . - fp_loadd0
.type fp_loadd0, #function

.global fp_loadd2
.align 8
fp_loadd2:
    retl
    ldd [%o0], %f2
.size fp_loadd2, . - fp_loadd2
.type fp_loadd2, #function

.global fp_loadd4
.align 8
fp_loadd4:
    retl
    ldd [%o0], %f4
.size fp_loadd4, . - fp_loadd4
.type fp_loadd4, #function

.global fp_loadd6
.align 8
fp_loadd6:
    retl
    ldd [%o0], %f6
.size fp_loadd6, . - fp_loadd6
.type fp_loadd6, #function

.global fp_loadd8
.align 8
fp_loadd8:
    retl
    ldd [%o0], %f8
.size fp_loadd8, . - fp_loadd8
.type fp_loadd8, #function

.global fp_loadd10
.align 8
fp_loadd10:
    retl
    ldd [%o0], %f10
.size fp_loadd10, . - fp_loadd10
.type fp_loadd10, #function

.global fp_loadd12
.align 8
fp_loadd12:
    retl
    ldd [%o0], %f12
.size fp_loadd12, . - fp_loadd12
.type fp_loadd12, #function

.global fp_loadd14
.align 8
fp_loadd14:
    retl
    ldd [%o0], %f14
.size fp_loadd14, . - fp_loadd14
.type fp_loadd14, #function

.global fp_loadd16
.align 8
fp_loadd16:
    retl
    ldd [%o0], %f16
.size fp_loadd16, . - fp_loadd16
.type fp_loadd16, #function

.global fp_loadd18
.align 8
fp_loadd18:
    retl
    ldd [%o0], %f18
.size fp_loadd18, . - fp_loadd18
.type fp_loadd18, #function

.global fp_loadd20
.align 8
fp_loadd20:
    retl
    ldd [%o0], %f20
.size fp_loadd20, . - fp_loadd20
.type fp_loadd20, #function

.global fp_loadd22
.align 8
fp_loadd22:
    retl
    ldd [%o0], %f22
.size fp_loadd22, . - fp_loadd22
.type fp_loadd22, #function

.global fp_loadd24
.align 8
fp_loadd24:
    retl
    ldd [%o0], %f24
.size fp_loadd24, . - fp_loadd24
.type fp_loadd24, #function

.global fp_loadd26
.align 8
fp_loadd26:
    retl
    ldd [%o0], %f26
.size fp_loadd26, . - fp_loadd26
.type fp_loadd26, #function

.global fp_loadd28
.align 8
fp_loadd28:
    retl
    ldd [%o0], %f28
.size fp_loadd28, . - fp_loadd28
.type fp_loadd28, #function

.global fp_loadd30
.align 8
fp_loadd30:
    retl
    ldd [%o0], %f30
.size fp_loadd30, . - fp_loadd30
.type fp_loadd30, #function

.global fp_stored0
.align 8
fp_stored0:
    retl
    std %f0, [%o0]
.size fp_stored0, . - fp_stored0
.type fp_stored0, #function

.global fp_stored2
.align 8
fp_stored2:
    retl
    std %f2, [%o0]
.size fp_stored2, . - fp_stored2
.type fp_stored2, #function

.global fp_stored4
.align 8
fp_stored4:
    retl
    std %f4, [%o0]
.size fp_stored4, . - fp_stored4
.type fp_stored4, #function

.global fp_stored6
.align 8
fp_stored6:
    retl
    std %f6, [%o0]
.size fp_stored6, . - fp_stored6
.type fp_stored6, #function

.global fp_stored8
.align 8
fp_stored8:
    retl
    std %f8, [%o0]
.size fp_stored8, . - fp_stored8
.type fp_stored8, #function

.global fp_stored10
.align 8
fp_stored10:
    retl
    std %f10, [%o0]
.size fp_stored10, . - fp_stored10
.type fp_stored10, #function

.global fp_stored12
.align 8
fp_stored12:
    retl
    std %f12, [%o0]
.size fp_stored12, . - fp_stored12
.type fp_stored12, #function

.global fp_stored14
.align 8
fp_stored14:
    retl
    std %f14, [%o0]
.size fp_stored14, . - fp_stored14
.type fp_stored14, #function

.global fp_stored16
.align 8
fp_stored16:
    retl
    std %f16, [%o0]
.size fp_stored16, . - fp_stored16
.type fp_stored16, #function

.global fp_stored18
.align 8
fp_stored18:
    retl
    std %f18, [%o0]
.size fp_stored18, . - fp_stored18
.type fp_stored18, #function

.global fp_stored20
.align 8
fp_stored20:
    retl
    std %f20, [%o0]
.size fp_stored20, . - fp_stored20
.type fp_stored20, #function

.global fp_stored22
.align 8
fp_stored22:
    retl
    std %f22, [%o0]
.size fp_stored22, . - fp_stored22
.type fp_stored22, #function

.global fp_stored24
.align 8
fp_stored24:
    retl
    std %f24, [%o0]
.size fp_stored24, . - fp_stored24
.type fp_stored24, #function

.global fp_stored26
.align 8
fp_stored26:
    retl
    std %f26, [%o0]
.size fp_stored26, . - fp_stored26
.type fp_stored26, #function

.global fp_stored28
.align 8
fp_stored28:
    retl
    std %f28, [%o0]
.size fp_stored28, . - fp_stored28
.type fp_stored28, #function

.global fp_stored30
.align 8
fp_stored30:
    retl
    std %f30, [%o0]
.size fp_stored30, . - fp_stored30
.type fp_stored30, #function
