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

#include <com/sun/star/io/XSeekable.hpp>
#include <WW8Clx.hxx>
#include <resources.hxx>

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
