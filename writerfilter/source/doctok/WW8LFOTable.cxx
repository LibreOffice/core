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

#include "resources.hxx"

namespace writerfilter {
namespace doctok {

void WW8LFOTable::initPayload()
{
    sal_uInt32 nCount = getEntryCount();

    sal_uInt32 nOffset = 4;
    sal_uInt32 nOffsetLFOData = mnPlcfPayloadOffset;
    
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        WW8LFO aLFO(this, nOffset);
        
        entryOffsets.push_back(nOffset);
        nOffset += WW8LFO::getSize();
        
        payloadOffsets.push_back(nOffsetLFOData);
        payloadIndices.push_back(n);
        
        nOffsetLFOData += 4;
        
        sal_uInt32 nLvls = aLFO.get_clfolvl();
        
        for (sal_uInt32 k = 0; k < nLvls; ++k)
        {
            WW8LFOLevel aLevel(this, nOffsetLFOData);
            nOffsetLFOData += aLevel.calcSize();
        }
    }

    entryOffsets.push_back(nOffset);
    payloadOffsets.push_back(nOffsetLFOData);
}

sal_uInt32 WW8LFOTable::getEntryCount()
{
    return getU32(0);
}

writerfilter::Reference<Properties>::Pointer_t
WW8LFOTable::getEntry(sal_uInt32 nIndex)
{
    WW8LFO * pLFO = new WW8LFO(this, entryOffsets[nIndex]);

    pLFO->setIndex(nIndex);

    return writerfilter::Reference<Properties>::Pointer_t(pLFO);
}

writerfilter::Reference<Properties>::Pointer_t
WW8LFO::get_LFOData()
{
    WW8LFOTable * pLFOTable = dynamic_cast<WW8LFOTable *>(mpParent);
    sal_uInt32 nPayloadOffset = pLFOTable->getPayloadOffset(mnIndex);
    sal_uInt32 nPayloadSize = pLFOTable->getPayloadSize(mnIndex);
    
    return writerfilter::Reference<Properties>::Pointer_t
    (new WW8LFOData(mpParent, nPayloadOffset, nPayloadSize));
}

/*
writerfilter::Reference<Properties>::Pointer_t
WW8LFO::get_lfolevel(sal_uInt32 nIndex)
{
    WW8LFOTable * pLFOTable = dynamic_cast<WW8LFOTable *>(mpParent);
    sal_uInt32 nPayloadIndex = pLFOTable->getPayloadIndex(mnIndex) + nIndex;
    sal_uInt32 nPayloadOffset = pLFOTable->getPayloadOffset(nPayloadIndex);
    sal_uInt32 nPayloadSize = pLFOTable->getPayloadSize(nPayloadIndex);
    
    return writerfilter::Reference<Properties>::Pointer_t
        (new WW8LFOLevel(mpParent, nPayloadOffset, nPayloadSize));
}
*/

void WW8LFOLevel::resolveNoAuto(Properties & /*rHandler*/)
{
    
}

sal_uInt32 WW8LFOLevel::calcSize()
{
    sal_uInt32 nResult = WW8LFOLevel::getSize();

    if (get_fFormatting())
    {
        WW8ListLevel aLevel(mpParent, mnOffsetInParent + nResult);

        nResult += aLevel.calcSize();

        sal_uInt32 nXstSize = mpParent->getU16(mnOffsetInParent + nResult);

        nResult += (nXstSize + 1) * 2;
    }

    return nResult;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
