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

#include <resources.hxx>

namespace writerfilter {
namespace doctok {

void WW8StyleSheet::initPayload()
{
    sal_uInt32 nCount = getEntryCount();

    sal_uInt32 nOffset = get_size() + 2;
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        entryOffsets.push_back(nOffset);

        sal_uInt32 cbStd = getU16(nOffset);
        nOffset += cbStd + 2;
    }

    entryOffsets.push_back(nOffset);
}

sal_uInt32 WW8StyleSheet::calcSize()
{
    return getCount();
}

sal_uInt32 WW8StyleSheet::getEntryCount()
{
    return get_cstd();
}

writerfilter::Reference<Properties>::Pointer_t
WW8StyleSheet::getEntry(sal_uInt32 nIndex)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    sal_uInt32 nCount = entryOffsets[nIndex + 1] - entryOffsets[nIndex];

    if (nCount > get_cbSTDBaseInFile() + 2U)
    {
        WW8Style * pStyle = new WW8Style(this, entryOffsets[nIndex], nCount);

        pStyle->setIndex(nIndex);

        pResult = writerfilter::Reference<Properties>::Pointer_t(pStyle);
    }

    return pResult;
}

rtl::OUString WW8Style::get_xstzName()
{
    sal_uInt32 nCount = getU8(0xc);

    if (nCount > 0)
    {
        Sequence aSeq(mSequence, 0xe, nCount * 2);
        
        rtl_uString * pNew = 0;
        rtl_uString_newFromStr
            (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));
        
        return rtl::OUString(pNew);

    }

    return get_xstzName1();
}

rtl::OUString WW8Style::get_xstzName1()
{
    WW8StyleSheet * pParentStyleSheet = dynamic_cast<WW8StyleSheet *>(mpParent);

    if (mpParent != NULL)
    {
        sal_uInt32 nOffset = pParentStyleSheet->get_cbSTDBaseInFile() + 2;

        if (nOffset < getCount())
        {
            sal_uInt32 nCount = getU16(nOffset);
            
            if (nCount > 0)
            {
                Sequence aSeq(mSequence, nOffset + 2, nCount * 2);
                
                rtl_uString * pNew = 0;
                rtl_uString_newFromStr
                    (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));
                
                return rtl::OUString(pNew);
                
            }
        }
    }

    return rtl::OUString();
}

sal_uInt32 WW8Style::get_upxstart()
{
    sal_uInt32 nResult = 0;
    sal_uInt32 nCount = getU8(0xc);

    if (nCount > 0)
    {
        nResult = 0xc + 1 + nCount * 2;

        nResult += nResult % 2;
    }
    else
    {
        WW8StyleSheet * pParentStyleSheet = 
            dynamic_cast<WW8StyleSheet *>(mpParent);

        nResult = pParentStyleSheet->get_cbSTDBaseInFile() + 2;

        if (nResult < getCount())
        {
            sal_uInt32 nCountTmp = getU16(nResult);

            nResult += 4 + nCountTmp * 2;
        }
    }

    return nResult;
}

sal_uInt32 WW8Style::get_upx_count()
{
    return get_cupx();
}

writerfilter::Reference<Properties>::Pointer_t WW8Style::get_upx
(sal_uInt32 nIndex)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    WW8StructBaseTmpOffset aOffset(this);

    aOffset.set(get_upxstart());

    if (aOffset.get() > 0 )
    {
        sal_uInt32 nCount;

        for (sal_uInt32 n = 0; n < nIndex; ++n)
        {
            nCount = getU16(aOffset);

            aOffset.inc(nCount + 2);
            aOffset.inc(aOffset.get() % 2);
        }

        nCount = getU16(aOffset);

        if (nCount > 0)
        {
            aOffset.inc(2);
            
            bool bIsPap = get_cupx() == 2 && nIndex == 0;
            WW8PropertySet::Pointer_t  
                pProps(new WW8PropertySetImpl(*this, aOffset.get(), nCount, 
                                              bIsPap));
            
            WW8PropertiesReference * pRef = 
                new WW8PropertiesReference(pProps);
            
            pResult = writerfilter::Reference<Properties>::Pointer_t(pRef);
        }
    }

    return pResult;
}

void WW8Style::resolveNoAuto(Properties & /*rHandler*/)
{
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
