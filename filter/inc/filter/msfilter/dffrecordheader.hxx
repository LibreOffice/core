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

#ifndef _DFFRECORDHEADER_HXX
#define _DFFRECORDHEADER_HXX

#include <tools/solar.h>
#include <tools/stream.hxx>
#include <filter/msfilter/msfilterdllapi.h>
#include <svx/msdffdef.hxx>

class MSFILTER_DLLPUBLIC DffRecordHeader
{

public:
    sal_uInt8    nRecVer; // may be DFF_PSFLAG_CONTAINER
    sal_uInt16  nRecInstance;
    sal_uInt16  nImpVerInst;
    sal_uInt16  nRecType;
    sal_uInt32  nRecLen;
    sal_uLong   nFilePos;
public:
    DffRecordHeader() : nRecVer(0), nRecInstance(0), nImpVerInst(0), nRecType(0), nRecLen(0), nFilePos(0) {}
    FASTBOOL IsContainer() const { return nRecVer == DFF_PSFLAG_CONTAINER; }
    sal_uLong    GetRecBegFilePos() const { return nFilePos; }
    sal_uLong    GetRecEndFilePos() const { return nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen; }
    void SeekToEndOfRecord(SvStream& rIn) const { rIn.Seek(nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen ); }
    void SeekToContent(    SvStream& rIn) const { rIn.Seek(nFilePos + DFF_COMMON_RECORD_HEADER_SIZE ); }
    void SeekToBegOfRecord(SvStream& rIn) const { rIn.Seek( nFilePos ); }

    MSFILTER_DLLPUBLIC friend SvStream& operator>>(SvStream& rIn, DffRecordHeader& rRec);

};

#endif