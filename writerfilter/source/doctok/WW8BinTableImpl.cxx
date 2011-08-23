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

#include "WW8BinTableImpl.hxx"

namespace writerfilter {
namespace doctok
{
using namespace ::std;

void PageNumber::dump(OutputWithDepth<string> & /*output*/) const
{
}

sal_uInt32 WW8BinTableImpl::getPageNumber(const Fc & rFc) const
{
    sal_uInt32 nResult = 0;

    if (mPageMap.find(rFc) == mPageMap.end())
    {
#if 0
        sal_uInt32 n = getEntryCount();
        
        while (rFc < getFc(n))
        {
            --n;
        }

        nResult = getPageNumber(n);
        mPageMap[rFc] = nResult;
#else
        sal_uInt32 left = 0;
        sal_uInt32 right = getEntryCount();
        
        while (right - left > 1)
        {
            sal_uInt32 middle = (right + left) / 2;

            Fc aFc = getFc(middle);

            if (rFc < aFc)
                right = middle;
            else
                left = middle;

        }
        
        nResult = getPageNumber(left);
        mPageMap[rFc] = nResult;
#endif

    }
    else
        nResult = mPageMap[rFc];

    return nResult;
}

string WW8BinTableImpl::toString() const
{
    string aResult;
    char sBuffer[255];

    aResult += "(";

    for (sal_uInt32 n = 0; n < getEntryCount(); n++)
    {
        if (n > 0)
            aResult += ", ";

        snprintf(sBuffer, 255, "%" SAL_PRIxUINT32, getFc(n).get());
        aResult += sBuffer;
        aResult += "->";
        snprintf(sBuffer, 255, "%" SAL_PRIxUINT32, getPageNumber(n));
        aResult += sBuffer;
    }

    aResult += ")";

    return aResult;
}
}}
