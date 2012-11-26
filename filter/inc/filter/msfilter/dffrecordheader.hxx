/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    bool IsContainer() const { return nRecVer == DFF_PSFLAG_CONTAINER; }
    sal_uLong    GetRecBegFilePos() const { return nFilePos; }
    sal_uLong    GetRecEndFilePos() const { return nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen; }
    void SeekToEndOfRecord(SvStream& rIn) const { rIn.Seek(nFilePos + DFF_COMMON_RECORD_HEADER_SIZE + nRecLen ); }
    void SeekToContent(    SvStream& rIn) const { rIn.Seek(nFilePos + DFF_COMMON_RECORD_HEADER_SIZE ); }
    void SeekToBegOfRecord(SvStream& rIn) const { rIn.Seek( nFilePos ); }

    MSFILTER_DLLPUBLIC friend SvStream& operator>>(SvStream& rIn, DffRecordHeader& rRec);

};

#endif
