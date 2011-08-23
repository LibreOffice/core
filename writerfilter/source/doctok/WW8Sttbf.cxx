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

#include <WW8Sttbf.hxx>
#include <resources.hxx>

namespace writerfilter {
namespace doctok
{

WW8Sttbf::WW8Sttbf(WW8Stream & rStream, sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount)
{
    sal_uInt32 nComplexOffset = 0;
    if (getU16(0) == 0xffff)
    {
        mbComplex = true;
        nComplexOffset = 2;
    }

    mnCount = getU16(nComplexOffset);
    mnExtraDataCount = getU16(nComplexOffset + 2);

    nOffset = (mbComplex ? 2 : 0) + 4;

    for (sal_uInt32 n = 0; n < mnCount; ++n)
    {
        mEntryOffsets.push_back(nOffset);

        sal_uInt32 nStringLength = getU16(nOffset);
        
        nOffset += 2 + nStringLength * (mbComplex ? 2 : 1);

        mExtraOffsets.push_back(nOffset);

        nOffset += mnExtraDataCount;
    }
}

sal_uInt32 WW8Sttbf::getEntryOffset(sal_uInt32 nPos) const
{
    return mEntryOffsets[nPos];
}

sal_uInt32 WW8Sttbf::getExtraOffset(sal_uInt32 nPos) const
{
    return mExtraOffsets[nPos];
}

sal_uInt32 WW8Sttbf::getEntryCount() const
{
    return mnCount;
}

rtl::OUString WW8Sttbf::getEntry(sal_uInt32 nPos) const
{
    return getString(getEntryOffset(nPos));
}

WW8StructBase::Pointer_t WW8Sttbf::getExtraData(sal_uInt32 nPos)
{
    return WW8StructBase::Pointer_t
        (new WW8StructBase(*this, getExtraOffset(nPos), mnExtraDataCount));
}

WW8SttbTableResource::WW8SttbTableResource(WW8Sttbf::Pointer_t pSttbf)
: mpSttbf(pSttbf)
{
}

WW8SttbTableResource::~WW8SttbTableResource()
{
}

void WW8SttbTableResource::resolve(Table & rTable)
{
    sal_uInt32 nCount = mpSttbf->getEntryCount();
    
    for (sal_uInt32 n = 0; n < nCount; n++)
    {
        WW8StringValue::Pointer_t pVal(new WW8StringValue(mpSttbf->getEntry(n)));
        ::writerfilter::Reference<Properties>::Pointer_t pProps(new WW8StringProperty(0, pVal));
        
        rTable.entry(n, pProps);
    }
}

string WW8SttbTableResource::getType() const
{
    return "WW8SttbTableResource";
}

WW8StringProperty::WW8StringProperty(sal_uInt32 nId, WW8StringValue::Pointer_t pValue)
: mnId(nId), mpValue(pValue)
{
}

WW8StringProperty::~WW8StringProperty()
{
}

void WW8StringProperty::resolve(Properties & rProperties)
{
    rProperties.attribute(mnId, *mpValue);
}

string WW8StringProperty::getType() const
{
    return "WW8StringProperty";
}

sal_uInt32 WW8SttbRgtplc::getEntryCount()
{
    return getU16(2);
}

::writerfilter::Reference<Properties>::Pointer_t 
WW8SttbRgtplc::getEntry(sal_uInt32 nIndex)
{
    ::writerfilter::Reference<Properties>::Pointer_t pResult;
    
    sal_uInt32 nOffset = 6;
    
    while (nIndex > 0)
    {
        sal_uInt16 nCount = getU16(nOffset);
        
        nOffset = nOffset + 2 + nCount;
        ++nIndex;
    }
    
    sal_uInt16 nCount = getU16(nOffset);
    
    if (nCount > 0)
    {
        WW8Tplc * pTplc = new WW8Tplc(*this, nOffset + 2, nCount);
        
        pResult.reset(pTplc);
    }
    
    return pResult;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
