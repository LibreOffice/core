/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Clx.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:45:13 $
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
