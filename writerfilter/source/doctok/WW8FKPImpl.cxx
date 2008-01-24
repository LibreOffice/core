/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8FKPImpl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-24 15:57:48 $
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

#include "WW8FKPImpl.hxx"
#include "WW8PropertySetImpl.hxx"

namespace writerfilter {
namespace doctok
{
#if 0
sal_uInt32 WW8FKPImpl::getIndex(const Fc & rFc) const
{
    sal_uInt32 nLeft = 0;
    sal_uInt32 nRight = getEntryCount();

    while (nRight - nLeft > 1)
    {
        sal_uInt32 nMiddle = (nLeft + nRight) / 2;
        Fc aFcMiddle = getFc(nMiddle);

        if (! (rFc < aFcMiddle))
        {
            nLeft = nMiddle;
        }
        else
        {
            nRight = nMiddle;
        }
    }

    return nLeft;
}
#else
sal_uInt32 WW8FKPImpl::getIndex(const Fc & rFc) const
{
    sal_uInt32 nResult = getEntryCount();

    while (rFc < getFc(nResult))
        nResult--;

    return nResult;
}
#endif


writerfilter::Reference<Properties>::Pointer_t WW8CHPFKPImpl::getProperties(const Fc & rFc) const
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    sal_uInt32 n = getIndex(rFc);

    sal_uInt16 nOffset = 2 * getU8(getRgb() + n);

    if (nOffset > getRgb() + getEntryCount())
    {
        sal_uInt16 nCbChpx = getU8(nOffset);

        if (nCbChpx > 1 && nOffset + nCbChpx + 1 <= 511)
        {
            pResult = writerfilter::Reference<Properties>::Pointer_t
                (new WW8PropertySetImpl(*this, nOffset + 1, nCbChpx));
        }
    }
#if 0
    else
    {
        clog << "outsider!!!" << endl;
    }
#endif

    return pResult;
}

void WW8CHPFKPImpl::dump(OutputWithDepth<string> & o) const
{
    o.addItem("<fkp type='CHP'>");

    sal_uInt32 nCount = getEntryCount();
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        char sBuffer[256];

        snprintf(sBuffer, sizeof(sBuffer),
                 "<fkpentry fc='%" SAL_PRIxUINT32 "' offsetInFkp='%x'/>",
                 getFc(n).get(), 2 * getU8(getRgb() + n));

        o.addItem(sBuffer);
    }

    WW8StructBase::dump(o);
    o.addItem("</fkp>");
}

writerfilter::Reference<Properties>::Pointer_t
WW8PAPFKPImpl::getProperties(const Fc & rFc) const
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    sal_uInt32 n = getIndex(rFc);

    sal_uInt16 nOffset = 2 * getU8(getRgb() + n * 13);

    if (nOffset != 0)
    {
        if (nOffset > getRgb() + getEntryCount() * 13)
        {
            sal_uInt32 nOffsetIStd = nOffset + 1;
            sal_uInt16 nCbPapx = getU8(nOffset) * 2;

            if (nCbPapx == 0)
            {
                nOffsetIStd = nOffset + 2;
                nCbPapx = getU8(nOffset + 1) * 2 + 2;
            }

            sal_uInt32 nOffsetEnd = nOffset + nCbPapx;
            //sal_uInt32 nOffsetSprms = nOffsetIStd + 2;

            if (nCbPapx > 1 && nOffset + nCbPapx <= 511)
            {
                pResult = writerfilter::Reference<Properties>::Pointer_t
                    (new WW8PropertySetImpl(*this, nOffsetIStd,
                                            nOffsetEnd - nOffsetIStd,
                                            true));
            }
        }
#if 0
        else
        {
            clog << "outsider!!!" << endl;
        }
#endif
    }

    return pResult;
}

void WW8PAPFKPImpl::dump(OutputWithDepth<string> & o) const
{
    o.addItem("<fkp type='PAP'>");

    sal_uInt32 nCount = getEntryCount();
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        char sBuffer[256];

        snprintf(sBuffer, sizeof(sBuffer),
                 "<fkpentry fc='%" SAL_PRIxUINT32 "' offsetInFKP='%x'/>",
                 getFc(n).get(), 2 * getU8(getRgb() + n * 13));

        o.addItem(sBuffer);
    }

    WW8StructBase::dump(o);

    o.addItem("</fkp>");
}

bool operator < (const PageNumberAndFKP & rA,
                 const PageNumberAndFKP & rB)
{
    return rA.mnPageNumber < rB.mnPageNumber;
}

WW8FKPCache::~WW8FKPCache()
{
}

WW8FKP::Pointer_t WW8FKPCacheImpl::get(sal_uInt32 nPageNumber,
                                       bool bComplex)
{
    WW8FKP::Pointer_t pResult;

    PageNumbersAndFKPs::iterator aIt;
    aIt = mPageNumbersAndFKPs.find
        (PageNumberAndFKP(nPageNumber, WW8FKP::Pointer_t()));

    if (aIt != mPageNumbersAndFKPs.end())
        pResult = aIt->getFKP();
    else
    {
        if (mPageNumbersAndFKPs.size() > mnCacheSize)
        {
            PageNumbersAndFKPs::iterator aItDel =
                mPageNumbersAndFKPs.find
                (PageNumberAndFKP(mPageNumbers.front(),
                                  WW8FKP::Pointer_t()));

            mPageNumbersAndFKPs.erase(aItDel);
            mPageNumbers.pop_front();
        }

        pResult = createFKP(nPageNumber, bComplex);

        PageNumberAndFKP aPageNumberAndFKP(nPageNumber, pResult);
        mPageNumbersAndFKPs.insert(aPageNumberAndFKP);
        mPageNumbers.push_back(nPageNumber);
    }

    return pResult;
}

WW8FKP::Pointer_t WW8CHPFKPCacheImpl::createFKP(sal_uInt32 nPageNumber,
                                                bool bComplex)
{
    return WW8FKP::Pointer_t(new WW8CHPFKPImpl
                             (*mpStream, nPageNumber, bComplex));
}

WW8FKP::Pointer_t WW8PAPFKPCacheImpl::createFKP(sal_uInt32 nPageNumber,
                                                bool bComplex)
{
    return WW8FKP::Pointer_t(new WW8PAPFKPImpl
                             (*mpStream, nPageNumber, bComplex));
}
}}
