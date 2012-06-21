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

#include <com/sun/star/io/XSeekable.hpp>
#include <WW8Clx.hxx>
#include <doctok/resources.hxx>

namespace writerfilter {
namespace doctok
{
WW8Clx::WW8Clx(WW8Stream & rStream,
               sal_uInt32 nOffset, sal_uInt32 nCount)
: WW8StructBase(rStream, nOffset, nCount), nOffsetPieceTable(0)
{
    while (getU8(nOffsetPieceTable) != 2)
    {
        nOffsetPieceTable += getU16(nOffsetPieceTable + 1) + 3;
    }
}

sal_uInt32 WW8Clx::getPieceCount() const
{
    return (getU32(nOffsetPieceTable + 1) - 4) / 12;
}

sal_uInt32 WW8Clx::getCp(sal_uInt32 nIndex) const
{
    return getU32(nOffsetPieceTable + 5 + nIndex * 4);
}

sal_uInt32 WW8Clx::getFc(sal_uInt32 nIndex) const
{
    sal_uInt32 nResult = getU32(nOffsetPieceTable + 5 +
                                (getPieceCount() + 1) * 4 +
                                nIndex * 8 + 2);

    if (nResult & 0x40000000)
        nResult = (nResult & ~0x40000000) / 2;

    return nResult;
}

sal_Bool WW8Clx::isComplexFc(sal_uInt32 nIndex) const
{
    sal_Bool bResult = sal_False;
    sal_uInt32 nTmp = getU32(nOffsetPieceTable + 5 +
                             (getPieceCount() + 1) * 4 +
                             nIndex * 8 + 2);
    if (nTmp & 0x40000000)
        bResult = sal_True;

    return bResult;
}

void WW8Clx::dump(OutputWithDepth<string> & o) const
{
    WW8StructBase::dump(o);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
