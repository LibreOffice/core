/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8FontTable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:47:11 $
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

#include <resources.hxx>

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

sal_uInt32 WW8FontTable::calcPayloadOffset()
{
    return 0;
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

rtl::OUString WW8Font::get_xszFfn()
{
    sal_uInt32 nOffset = 0x28;
    sal_uInt32 nCount = get_cbFfnM1() - nOffset;

    Sequence aSeq(mSequence, nOffset, nCount);

    rtl_uString * pNew = 0;
    rtl_uString_newFromStr
        (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));

    return rtl::OUString(pNew);
}

rtl::OUString WW8Font::get_altName()
{
    sal_uInt32 nOffset = 0x28 + get_ixchSzAlt();
    sal_uInt32 nCount = get_cbFfnM1() - nOffset;

    Sequence aSeq(mSequence, nOffset, nCount);

    rtl_uString * pNew = 0;
    rtl_uString_newFromStr
        (&pNew, reinterpret_cast<const sal_Unicode *>(&aSeq[0]));

    return rtl::OUString(pNew);
}

}}
