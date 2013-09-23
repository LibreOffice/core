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

#include "WW8StructBase.hxx"

namespace writerfilter {
namespace doctok {
using namespace ::com::sun::star;

WW8StructBase::WW8StructBase(const WW8StructBase & rParent,
              sal_uInt32 nOffset, sal_uInt32 nCount)
: mSequence(rParent.mSequence, nOffset, nCount), mpParent(0)
{
    if (nOffset + nCount > rParent.getCount())
    {
        throw ExceptionOutOfBounds("WW8StructBase");
    }
}

WW8StructBase & WW8StructBase::Assign(const WW8StructBase & rSrc)
{
    mSequence = rSrc.mSequence;

    return *this;
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

OUString WW8StructBase::getString(sal_uInt32 nOffset, sal_uInt32 nCount)
    const
{
    OUString aResult;

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

            aResult = OUString(pNew);
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


OUString WW8StructBase::getString(sal_uInt32 nOffset) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
