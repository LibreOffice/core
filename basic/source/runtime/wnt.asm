;**************************************************************
;  
;  Licensed to the Apache Software Foundation (ASF) under one
;  or more contributor license agreements.  See the NOTICE file
;  distributed with this work for additional information
;  regarding copyright ownership.  The ASF licenses this file
;  to you under the Apache License, Version 2.0 (the
;  "License"); you may not use this file except in compliance
;  with the License.  You may obtain a copy of the License at
;  
;    http://www.apache.org/licenses/LICENSE-2.0
;  
;  Unless required by applicable law or agreed to in writing,
;  software distributed under the License is distributed on an
;  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
;  KIND, either express or implied.  See the License for the
;  specific language governing permissions and limitations
;  under the License.
;  
;**************************************************************

.386

PUBLIC _DllMgr_call32@12
PUBLIC _DllMgr_callFp@12

_TEXT SEGMENT
_DllMgr_call32@12:
_DllMgr_callFp@12:
    push ebp
    mov ebp, esp
    push esi
    push edi
    mov ecx, [ebp+16]
    jecxz $1
    sub esp, ecx
    mov edi, esp
    mov esi, [ebp+12]
    shr ecx, 2
    rep movsd
$1: call DWORD PTR [ebp+8]
    ; for extra safety, do not trust esp after call (in case the Basic Declare
    ; signature is wrong):
    mov edi, [ebp-8]
    mov esi, [ebp-4]
    mov esp, ebp
    pop ebp
    ret 12
_TEXT ENDS
END
