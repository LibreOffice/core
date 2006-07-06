/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: endian.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-06 10:00:40 $
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
