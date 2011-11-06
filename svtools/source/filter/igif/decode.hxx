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



#ifndef _DECODE_HXX
#define _DECODE_HXX

#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

struct GIFLZWTableEntry;

class GIFLZWDecompressor
{
    GIFLZWTableEntry*   pTable;
    HPBYTE              pOutBuf;
    HPBYTE              pOutBufData;
    HPBYTE              pBlockBuf;
    sal_uLong               nInputBitsBuf;
    sal_uInt16              nTableSize;
    sal_uInt16              nClearCode;
    sal_uInt16              nEOICode;
    sal_uInt16              nCodeSize;
    sal_uInt16              nOldCode;
    sal_uInt16              nOutBufDataLen;
    sal_uInt16              nInputBitsBufSize;
    sal_Bool                bEOIFound;
    sal_uInt8               nDataSize;
    sal_uInt8               nBlockBufSize;
    sal_uInt8               nBlockBufPos;

    void                AddToTable(sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData);
    sal_Bool                ProcessOneCode();


public:

                        GIFLZWDecompressor( sal_uInt8 cDataSize );
                        ~GIFLZWDecompressor();

    HPBYTE              DecompressBlock( HPBYTE pSrc, sal_uInt8 cBufSize, sal_uLong& rCount, sal_Bool& rEOI );
};

#endif
