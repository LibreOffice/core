/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8Sttbf.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:50:34 $
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

#include <WW8Sttbf.hxx>

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

}}
