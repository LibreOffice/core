/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8BinTableImpl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-24 15:57:21 $
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
