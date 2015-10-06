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

#ifndef INCLUDED_BASEBMP_INC_ENDIAN_HXX
#define INCLUDED_BASEBMP_INC_ENDIAN_HXX

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
BASEBMP_BYTE_SWAP(sal_uInt16,OSL_SWAPWORD)
BASEBMP_BYTE_SWAP(sal_uInt32,OSL_SWAPDWORD)

#undef BASEBMP_BYTE_SWAP

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_INC_ENDIAN_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
