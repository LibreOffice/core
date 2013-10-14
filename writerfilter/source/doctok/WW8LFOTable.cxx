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

#include "doctok/resources.hxx"

namespace writerfilter {
namespace doctok {

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
