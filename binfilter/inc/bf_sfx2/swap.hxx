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
#ifndef _SWAP_HXX
#define _SWAP_HXX

#include <solar.h>
namespace binfilter {

inline BOOL SwapShort()
{
    short aLoHiTst = 0x55AA;
    return (*((char*)&aLoHiTst) == 0x55 );
}

inline BOOL SwapLong()
{
    long aLoHiTst = 0x5555AAAA;
    return (*((short*)&aLoHiTst) == 0x5555 );
}

union LoHi
{
    BYTE nBytes[2];
    short nShort;
};

#define SWAP( n )   (( ((LoHi&)n).nBytes[1] << 8 ) + ((LoHi&)n).nBytes[0] )

inline short Swap(short nV)
{
    return SwapShort()? SWAP(nV): nV;
}

inline unsigned short Swap(unsigned short nV)
{
    return SwapShort()? SWAP(nV): nV;
}


union LL
{
    USHORT nBytes[2];
    long l;
};

#define SWAPL(n)   (( ((LL&)n).nBytes[1] << 16 ) + ((LL&)n).nBytes[0] )

inline long SwapL(long l)
{
    LL aL;
    aL.l = l;
    aL.nBytes[0] = SWAP(aL.nBytes[0]);
    aL.nBytes[1] = SWAP(aL.nBytes[1]);
    aL.l = SWAPL(aL.l);
    return aL.l;
}

inline long Swap(long lV)
{
    return SwapLong()? SwapL(lV): lV;
}

inline unsigned long Swap(unsigned long lV)
{
    return SwapLong()? SwapL(lV): lV;
}

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
