/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-19 09:29:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
bool examine_argument( typelib_TypeDescriptionReference *pTypeRef, bool bInReturn, int &nUsedGPR, int &nUsedSSE );

/** Does function that returns this type use a hidden parameter, or registers?

 The value can be returned either in a hidden 1st parameter (which is a
 pointer to a structure allocated by the caller), or in registers (rax, rdx
 for the integers, xmm0, xmm1 for the floating point numbers).
*/
bool return_in_hidden_param( typelib_TypeDescriptionReference *pTypeRef );

void fill_struct( typelib_TypeDescriptionReference *pTypeRef, void * const *pGPR, void * const *pSSE, void *pStruct );

} // namespace x86_64

#endif // _BRIDGES_CPP_UNO_X86_64_ABI_HXX_
