/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/




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

