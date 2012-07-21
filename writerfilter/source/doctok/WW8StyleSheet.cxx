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

#include <doctok/resources.hxx>

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

OUString WW8Style::get_xstzName()
{
    sal_uInt32 nCount = getU8(0xc);

    if (nCount > 0)
    {
        Sequence aSeq(mSequence, 0xe, nCount * 2);

        rtl_uString * pNew = 0;
        rtl_uString_newFromStr
            (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));

        return OUString(pNew);

    }

    return get_xstzName1();
}

OUString WW8Style::get_xstzName1()
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

                return OUString(pNew);

            }
        }
    }

    return OUString();
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
