/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

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

extern "C" void
privateSnippetExecutor()
{
    asm volatile
        (
         "    subq  $160, %rsp\n"

         "    movq  %r10, -152(%rbp)        # Save (nVtableOffset << 32) + nFunctionIndex\n"

         "    movq  %rdi, -112(%rbp)        # Save GP registers\n"
         "    movq  %rsi, -104(%rbp)\n"
         "    movq  %rdx, -96(%rbp)\n"
         "    movq  %rcx, -88(%rbp)\n"
         "    movq  %r8 , -80(%rbp)\n"
         "    movq  %r9 , -72(%rbp)\n"

         "    movsd %xmm0, -64(%rbp)        # Save FP registers\n"
         "    movsd %xmm1, -56(%rbp)\n"
         "    movsd %xmm2, -48(%rbp)\n"
         "    movsd %xmm3, -40(%rbp)\n"
         "    movsd %xmm4, -32(%rbp)\n"
         "    movsd %xmm5, -24(%rbp)\n"
         "    movsd %xmm6, -16(%rbp)\n"
         "    movsd %xmm7, -8(%rbp)\n"

         "    leaq  -144(%rbp), %r9         # 6th param: sal_uInt64 * pRegisterReturn\n"
         "    leaq  16(%rbp), %r8           # 5rd param: void ** ovrflw\n"
         "    leaq  -64(%rbp), %rcx         # 4th param: void ** fpreg\n"
         "    leaq  -112(%rbp), %rdx        # 3rd param: void ** gpreg\n"
         "    movl  -148(%rbp), %esi        # 2nd param: sal_int32 nVtableOffset\n"
         "    movl  -152(%rbp), %edi        # 1st param: sal_int32 nFunctionIndex\n"

         "    call  _cpp_vtable_call\n"

         "    cmp   $10, %rax               # typelib_TypeClass_FLOAT\n"
         "    je    .Lfloat\n"
         "    cmp   $11, %rax               # typelib_TypeClass_DOUBLE\n"
         "    je    .Lfloat\n"

         "    movq  -144(%rbp), %rax        # Return value (int case)\n"
         "    movq  -136(%rbp), %rdx        # Return value (int case)\n"
         "    movq  -144(%rbp), %xmm0       # Return value (int case)\n"
         "    movq  -136(%rbp), %xmm1       # Return value (int case)\n"
         "    jmp   .Lfinish\n"

         ".Lfloat:\n"
         "    movlpd    -144(%rbp), %xmm0   # Return value (float/double case)\n"

         ".Lfinish:\n"
         "    addq  $160, %rsp\n"
         );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
