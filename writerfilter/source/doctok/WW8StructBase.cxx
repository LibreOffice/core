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

#include <WW8StructBase.hxx>

namespace writerfilter {
namespace doctok {
using namespace ::com::sun::star;

WW8StructBase::WW8StructBase(const WW8StructBase & rParent,
              sal_uInt32 nOffset, sal_uInt32 nCount)
: mSequence(rParent.mSequence, nOffset, nCount), mpParent(0), 
  mpDocument(rParent.getDocument())
{
    if (nOffset + nCount > rParent.getCount())
    {
        throw ExceptionOutOfBounds("WW8StructBase");
    }
}

WW8StructBase & WW8StructBase::Assign(const WW8StructBase & rSrc)
{
    mSequence = rSrc.mSequence;
    mpDocument = rSrc.mpDocument;

    return *this;
}

void WW8StructBase::setDocument(WW8DocumentImpl * pDocument)
{
    mpDocument = pDocument;
}

WW8DocumentImpl * WW8StructBase::getDocument() const
{
    return mpDocument;
}

sal_uInt8 WW8StructBase::getU8(sal_uInt32 nOffset) const
{ 
    return doctok::getU8(mSequence, nOffset);
}

sal_uInt16 WW8StructBase::getU16(sal_uInt32 nOffset) const
{ 
    return doctok::getU16(mSequence, nOffset);
}

sal_uInt32 WW8StructBase::getU32(sal_uInt32 nOffset) const
{ 
    return doctok::getU32(mSequence, nOffset);
}

sal_Bool WW8StructBase::getBit(sal_uInt32 nValue, sal_uInt16 nBit) const
{
    return (nValue & (1 << nBit)) != 0;
}

sal_uInt8 WW8StructBase::getNibble(sal_uInt32 nValue, 
                                   sal_uInt16 nShift) const
{
    return sal::static_int_cast<sal_uInt8>((nValue >> nShift) & 0xf);
}

sal_uInt8 getU8(const WW8StructBase::Sequence & rSeq, 
                sal_uInt32 nOffset)
{
    return rSeq[nOffset];
}

sal_uInt16 getU16(const WW8StructBase::Sequence & rSeq, 
                  sal_uInt32 nOffset)
{
    return getU8(rSeq, nOffset) | (getU8(rSeq, nOffset + 1) << 8);
}

sal_uInt32 getU32(const WW8StructBase::Sequence & rSeq, 
                  sal_uInt32 nOffset)
{
    sal_uInt32 nResult = getU8(rSeq, nOffset);
    nResult |= (getU8(rSeq, nOffset + 1) << 8);
    nResult |=  (getU8(rSeq, nOffset + 2) << 16);
    nResult |= (getU8(rSeq, nOffset + 3) << 24);
    
    return nResult;
}

rtl::OUString WW8StructBase::getString(sal_uInt32 nOffset, sal_uInt32 nCount) 
    const
{
    rtl::OUString aResult;
    
    if (nOffset < getCount())
    {            
        sal_uInt32 nCount1 = nCount;
        if (nOffset + nCount * 2 > getCount())
        {
            nCount1 = (getCount() - nOffset) / 2;
        }

        if (nCount1 > 0)
        {
            Sequence aSeq(mSequence, nOffset, nCount1 * 2);
            
            rtl_uString * pNew = 0;
            rtl_uString_newFromStr_WithLength
            (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]),
             nCount1);

            aResult = rtl::OUString(pNew);
        }
    }

    return aResult;
}

WW8StructBase * 
WW8StructBase::getRemainder(sal_uInt32 nOffset) const
{
    WW8StructBase * pResult = NULL;
    
    sal_uInt32 nCount = getCount();
    if (nCount > nOffset)
    {
        pResult = new WW8StructBase(*this, nOffset, nCount - nOffset);
    }
    
    return pResult;
}


rtl::OUString WW8StructBase::getString(sal_uInt32 nOffset) const
{
    sal_uInt32 nCount = getU16(nOffset);

    return getString(nOffset + 2, nCount);
}

WW8StructBaseTmpOffset::WW8StructBaseTmpOffset
(WW8StructBase * pStructBase)
: mnOffset(0), mpStructBase(pStructBase)
{
}

sal_uInt32 WW8StructBaseTmpOffset::set(sal_uInt32 nOffset)
{
    if (nOffset >= mpStructBase->getCount())
        throw ExceptionOutOfBounds("WW8StructBaseTmpOffset::set");

    mnOffset = nOffset;

    return mnOffset;
}

sal_uInt32 WW8StructBaseTmpOffset::get() const
{
    return mnOffset;
}

sal_uInt32 WW8StructBaseTmpOffset::inc(sal_uInt32 nOffset)
{
    if (mpStructBase->getCount() - mnOffset < nOffset)
        throw ExceptionOutOfBounds("WW8StructBaseTmpOffset::inc");

    mnOffset += nOffset;

    return mnOffset;
}

WW8StructBaseTmpOffset::operator sal_uInt32() const
{
    return mnOffset;
}

}}
