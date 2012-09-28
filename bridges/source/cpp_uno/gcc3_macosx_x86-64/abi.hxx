/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _BRIDGES_CPP_UNO_X86_64_ABI_HXX_
#define _BRIDGES_CPP_UNO_X86_64_ABI_HXX_

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

#endif // _BRIDGES_CPP_UNO_X86_64_ABI_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
