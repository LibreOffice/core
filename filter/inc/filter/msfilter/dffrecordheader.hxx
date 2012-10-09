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

#ifndef _DFFRECORDHEADER_HXX
#define _DFFRECORDHEADER_HXX

#include <tools/solar.h>
#include <tools/stream.hxx>
#include <filter/msfilter/msfilterdllapi.h>
#include <svx/msdffdef.hxx>

class MSFILTER_DLLPUBLIC DffRecordHeader
{
public:
    sal_uInt8   nRecVer; // may be DFF_PSFLAG_CONTAINER
    sal_uInt16  nRecInstance;
    sal_uInt16  nImpVerInst;
    sal_uInt16  nRecType;
    sal_uInt32  nRecLen;
    sal_uLong   nFilePos;

    DffRecordHeader() : nRecVer(0), nRecInstance(0), nImpVerInst(0),
                        nRecType(0), nRecLen(0), nFilePos(0) {}
    bool        IsContainer() const { return nRecVer == DFF_PSFLAG_CONTAINER; }
    sal_uLong   GetRecBegFilePos() const { return nFilePos; }
    sal_uLong   GetRecEndFilePos() const
        { return nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen; }
    bool SeekToEndOfRecord(SvStream& rIn) const
    {
        sal_Size nPos = nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen;
        return nPos == rIn.Seek(nPos);
    }
    bool SeekToContent(SvStream& rIn) const
    {
        sal_Size nPos = nFilePos + DFF_COMMON_RECORD_HEADER_SIZE;
        return nPos == rIn.Seek(nPos);
    }
    bool SeekToBegOfRecord(SvStream& rIn) const
    {
        return nFilePos == rIn.Seek(nFilePos);
    }

    MSFILTER_DLLPUBLIC friend SvStream& operator>>(SvStream& rIn, DffRecordHeader& rRec);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
