/*************************************************************************
 *
 *  $RCSfile: SwBitArray.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-26 13:58:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#if 1
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
