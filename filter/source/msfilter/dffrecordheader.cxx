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

#include <filter/msfilter/dffrecordheader.hxx>

bool ReadDffRecordHeader(SvStream& rIn, DffRecordHeader& rRec)
{
    rRec.nFilePos = rIn.Tell();
    if (rIn.remainingSize() >= 8)
    {
        sal_uInt16 nTmp(0);
        rIn.ReadUInt16(nTmp);
        rRec.nImpVerInst = nTmp;
        rRec.nRecVer = sal::static_int_cast<sal_uInt8>(nTmp & 0x000F);
        rRec.nRecInstance = nTmp >> 4;
        rIn.ReadUInt16(rRec.nRecType);
        rIn.ReadUInt32(rRec.nRecLen);

        // preserving overflow, optimally we would check
        // the record size against the parent header
        if (rRec.nRecLen > (SAL_MAX_UINT32 - rRec.nFilePos))
            rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
    }
    else
    {
        rRec.nImpVerInst = 0;
        rRec.nRecVer = 0;
        rRec.nRecInstance = 0;
        rIn.Seek(STREAM_SEEK_TO_END);
    }
    return rIn.good();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
