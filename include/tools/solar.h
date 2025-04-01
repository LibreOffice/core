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


#include <sal/types.h>
#include <osl/endian.h>

#include <algorithm>

/** Intermediate type to solve type clash with Windows headers.
 Should be removed as soon as all code parts have been reviewed
 and the correct type is known. Most of the times ULONG is meant
 to be a 32-Bit unsigned integer type as sal_uInt32 is often
 used for data exchange or for similar method args. */
typedef sal_uIntPtr    sal_uLong; /* Replaces type ULONG */

// misc. macros to leverage platform and compiler differences

// solar binary types

/* Solar (portable) Binary (exchange) Type; OSI 6 subset
   always little endian;
   not necessarily aligned */

typedef sal_uInt8   SVBT16[2];
typedef sal_uInt8   SVBT32[4];
typedef sal_uInt8   SVBT64[8];

inline sal_uInt16 SVBT16ToUInt16( const SVBT16 p ) { return static_cast<sal_uInt16>
                                                     (static_cast<sal_uInt16>(p[0])
                                                   + (static_cast<sal_uInt16>(p[1]) <<  8)); }
inline sal_Int16 SVBT16ToInt16( const SVBT16 p ) { return sal_Int16(SVBT16ToUInt16(p)); }
inline sal_uInt32 SVBT32ToUInt32 ( const SVBT32 p ) { return static_cast<sal_uInt32>
                                                     (static_cast<sal_uInt32>(p[0])
                                                   + (static_cast<sal_uInt32>(p[1]) <<  8)
                                                   + (static_cast<sal_uInt32>(p[2]) << 16)
                                                   + (static_cast<sal_uInt32>(p[3]) << 24)); }
inline double   SVBT64ToDouble( const SVBT64 p )
{
    double n;
#if defined OSL_LITENDIAN
    std::copy(p, p + 8, reinterpret_cast<sal_uInt8*>(&n));
#else
    std::reverse_copy(p, p + 8, reinterpret_cast<sal_uInt8*>(&n));
#endif
    return n;
}

inline void     ShortToSVBT16( sal_uInt16 n, SVBT16 p )
{
    p[0] = static_cast<sal_uInt8>(n);
    p[1] = static_cast<sal_uInt8>(n >>  8);
}
inline void     UInt32ToSVBT32 ( sal_uInt32  n, SVBT32 p )
{
    p[0] = static_cast<sal_uInt8>(n);
    p[1] = static_cast<sal_uInt8>(n >>  8);
    p[2] = static_cast<sal_uInt8>(n >> 16);
    p[3] = static_cast<sal_uInt8>(n >> 24);
}
inline void     Int32ToSVBT32 ( sal_Int32  n, SVBT32 p ) { UInt32ToSVBT32(sal_uInt32(n), p); }
inline void     DoubleToSVBT64( double n, SVBT64 p )
{
#if defined OSL_LITENDIAN
    std::copy(reinterpret_cast<sal_uInt8*>(&n), reinterpret_cast<sal_uInt8*>(&n) + 8, p);
#else
    std::reverse_copy(reinterpret_cast<sal_uInt8*>(&n), reinterpret_cast<sal_uInt8*>(&n) + 8, p);
#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
