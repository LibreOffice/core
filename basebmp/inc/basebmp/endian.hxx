/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: endian.hxx,v $
 * $Revision: 1.2 $
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

#ifndef INCLUDED_BASEBMP_ENDIAN_HXX
#define INCLUDED_BASEBMP_ENDIAN_HXX

#include <osl/endian.h>

namespace basebmp
{

/// Swap the order of bytes for the given POD type
template< typename T > inline T byteSwap( T );

#define BASEBMP_BYTE_SWAP(Type,SwapFunc) \
   template<> inline Type byteSwap<Type>( Type v ) \
   { \
       return SwapFunc(v); \
   }

// byteSwap<T> shall fail for any type T not in the list below
BASEBMP_BYTE_SWAP(sal_Int8,)
BASEBMP_BYTE_SWAP(sal_uInt8,)
BASEBMP_BYTE_SWAP(sal_Int16,OSL_SWAPWORD)
BASEBMP_BYTE_SWAP(sal_uInt16,OSL_SWAPWORD)
BASEBMP_BYTE_SWAP(sal_Int32,OSL_SWAPDWORD)
BASEBMP_BYTE_SWAP(sal_uInt32,OSL_SWAPDWORD)

#undef BASEBMP_BYTE_SWAP

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ENDIAN_HXX */
