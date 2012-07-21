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

void WW8FontTable::initPayload()
{
    sal_uInt32 nCount = getU8(0);

    sal_uInt32 nOffset = 1;

    while (nCount > 0)
    {
        entryOffsets.push_back(nOffset);

        sal_uInt32 nFFNSize = getU8(nOffset) + 1;

        if (nFFNSize > 1)
            nCount--;

        nOffset += nFFNSize;
    }

    entryOffsets.push_back(nOffset);
    mnPlcfPayloadOffset = nOffset;
}

sal_uInt32 WW8FontTable::getEntryCount()
{
    return entryOffsets.size() - 1;
}

writerfilter::Reference<Properties>::Pointer_t
WW8FontTable::getEntry(sal_uInt32 nIndex)
{
    writerfilter::Reference<Properties>::Pointer_t pResult;

    sal_uInt32 nCount = entryOffsets[nIndex + 1] - entryOffsets[nIndex];

    if (nCount > 1)
    {
        WW8Font * pFont = new WW8Font(this,
                                      entryOffsets[nIndex], nCount);

        pFont->setIndex(nIndex);

        pResult = writerfilter::Reference<Properties>::Pointer_t(pFont);
    }

    return pResult;
}

sal_uInt32 WW8Font::get_f()
{
    return mnIndex;
}

OUString WW8Font::get_xszFfn()
{
    sal_uInt32 nOffset = 0x28;
    sal_uInt32 nCount = get_cbFfnM1() - nOffset;

    Sequence aSeq(mSequence, nOffset, nCount);

    rtl_uString * pNew = 0;
    rtl_uString_newFromStr
        (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));

    return OUString(pNew);
}

OUString WW8Font::get_altName()
{
    sal_uInt32 nOffset = 0x28 + get_ixchSzAlt();
    sal_uInt32 nCount = get_cbFfnM1() - nOffset;

    Sequence aSeq(mSequence, nOffset, nCount);

    rtl_uString * pNew = 0;
    rtl_uString_newFromStr
        (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));

    return OUString(pNew);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
