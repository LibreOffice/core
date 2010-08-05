#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#***********************************************************************/

.intel_syntax

.globl _DllMgr_call32
.globl _DllMgr_callFp

_DllMgr_call32:
_DllMgr_callFp:
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
    # for extra safety, do not trust esp after call (in case the Basic Declare
    # signature is wrong):
    mov edi, [ebp-8]
    mov esi, [ebp-4]
    mov esp, ebp
    pop ebp
    ret 12
