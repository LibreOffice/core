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

#pragma once

// This is an implementation of the x86-64 ABI as described in 'System V
// Application Binary Interface, AMD64 Architecture Processor Supplement'
// (http://www.x86-64.org/documentation/abi-0.95.pdf)

#include <typelib/typedescription.hxx>

namespace x86_64
{

/* 6 general purpose registers are used for parameter passing */
const sal_uInt32 MAX_GPR_REGS = 6;

/* 8 SSE registers are used for parameter passing */
const sal_uInt32 MAX_SSE_REGS = 8;

/* Count number of required registers.

 Examine the argument and return set number of register required in each
 class.

 Return false iff parameter should be passed in memory.
*/
bool examine_argument( typelib_TypeDescriptionReference *pTypeRef, bool bInReturn, int &nUsedGPR, int &nUsedSSE ) throw ();

/** Does function that returns this type use a hidden parameter, or registers?

 The value can be returned either in a hidden 1st parameter (which is a
 pointer to a structure allocated by the caller), or in registers (rax, rdx
 for the integers, xmm0, xmm1 for the floating point numbers).
*/
bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef ) throw ();

void fill_struct( typelib_TypeDescriptionReference *pTypeRef, const sal_uInt64* pGPR, const double* pSSE, void *pStruct ) throw ();

} // namespace x86_64

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
