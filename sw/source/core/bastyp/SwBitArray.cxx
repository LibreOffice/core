/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SwBitArray.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <string.h>
#include "SwBitArray.hxx"

using namespace std;

SwBitArray::SwBitArray(sal_uInt32 _nSize)
{
    nSize = _nSize;
    mArray = new sal_uInt32[(nSize - 1)/ mGroupSize + 1];
    Reset();
}

SwBitArray::SwBitArray(const SwBitArray & rArray)
    : nSize(rArray.nSize)
{
    mArray = new sal_uInt32[calcSize()];
    memcpy(mArray, rArray.mArray, calcSize());
}

SwBitArray::~SwBitArray()
{
    delete [] mArray;
}

BOOL SwBitArray::IsValid(sal_uInt32 n) const
{
    return n < nSize;
}

void SwBitArray::Set(sal_uInt32 n, BOOL nValue)
{
    sal_uInt32 * pGroup = NULL;

    if (IsValid(n))
    {
        pGroup = GetGroup(n);

        if (nValue)
            *pGroup |= 1 << (n % mGroupSize);
        else
            *pGroup &= ~(1 << (n % mGroupSize));
    }
}

void SwBitArray::Reset()
{
    memset(mArray, 0, mGroupSize * (nSize / mGroupSize + 1));
}

BOOL SwBitArray::Get(sal_uInt32 n) const
{
    BOOL bResult = FALSE;
    sal_uInt32 * pGroup = NULL;

    if (IsValid(n))
    {
        pGroup = GetGroup(n);

        bResult = 0 != (*pGroup & (1 << (n % mGroupSize)));
    }

    return bResult;
}

SwBitArray & SwBitArray::operator = (const SwBitArray & rArray)
{
    if (Size() == rArray.Size())
    {
        memcpy(mArray, rArray.mArray, calcSize());
    }

    return *this;
}

SwBitArray operator & (const SwBitArray & rA, const SwBitArray & rB)
{
    SwBitArray aResult(rA);

    if (rA.Size() == rB.Size())
    {
        for (size_t i = 0; i < rA.calcSize(); i++)
            aResult.mArray[i] &= rB.mArray[i];
    }

    return aResult;
}

SwBitArray operator | (const SwBitArray & rA, const SwBitArray & rB)
{
    SwBitArray aResult(rA);

    if (rA.Size() == rB.Size())
    {
        for (size_t i = 0; i < rA.calcSize(); i++)
            aResult.mArray[i] |= rB.mArray[i];
    }

    return aResult;
}

SwBitArray operator ^ (const SwBitArray & rA, const SwBitArray & rB)
{
    SwBitArray aResult(rA);

    if (rA.Size() == rB.Size())
    {
        for (size_t i = 0; i < rA.calcSize(); i++)
            aResult.mArray[i] ^= rB.mArray[i];
    }

    return aResult;
}

SwBitArray operator ~ (const SwBitArray & rA)
{
    SwBitArray aResult(rA);

    for (size_t i = 0; i < rA.calcSize(); i++)
        aResult.mArray[i] = ~ rA.mArray[i];

    return aResult;
}

#if OSL_DEBUG_LEVEL > 1
ostream & operator << (ostream & o, const SwBitArray & rBitArray)
{
    char buffer[256];

    sprintf(buffer, "%p", &rBitArray);
    o << "[ " << buffer << " ";
    for (sal_uInt32 n = 0; n < rBitArray.Size(); n++)
    {
        if (rBitArray.Get(n))
            o << "1";
        else
            o << "0";
    }
    o << " ]";

    return o;
}
#endif
