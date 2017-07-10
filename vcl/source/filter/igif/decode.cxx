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

#include "decode.hxx"

#include <rtl/alloc.h>

struct GIFLZWTableEntry
{
    GIFLZWTableEntry*   pPrev;
    GIFLZWTableEntry*   pFirst;
    sal_uInt8               nData;
};

GIFLZWDecompressor::GIFLZWDecompressor(sal_uInt8 cDataSize)
    : pBlockBuf(nullptr)
    , nInputBitsBuf(0)
    , nOutBufDataLen(0)
    , nInputBitsBufSize(0)
    , bEOIFound(false)
    , nDataSize(cDataSize)
    , nBlockBufSize(0)
    , nBlockBufPos(0)
{
    nClearCode = 1 << nDataSize;
    nEOICode = nClearCode + 1;
    nTableSize = nEOICode + 1;
    nCodeSize = nDataSize + 1;
    nOldCode = 0xffff;
    pOutBufData = pOutBuf.data() + 4096;

    pTable.reset( new GIFLZWTableEntry[ 4098 ] );

    for (sal_uInt16 i = 0; i < nTableSize; ++i)
    {
        pTable[i].pPrev = nullptr;
        pTable[i].pFirst = pTable.get() + i;
        pTable[i].nData = (sal_uInt8) i;
    }

    memset(pTable.get() + nTableSize, 0, sizeof(GIFLZWTableEntry) * (4098 - nTableSize));
}

GIFLZWDecompressor::~GIFLZWDecompressor()
{
}

Scanline GIFLZWDecompressor::DecompressBlock( sal_uInt8* pSrc, sal_uInt8 cBufSize,
                                            sal_uLong& rCount, bool& rEOI )
{
    sal_uLong   nTargetSize = 4096;
    sal_uLong   nCount = 0;
    sal_uInt8*  pTarget = static_cast<sal_uInt8*>(rtl_allocateMemory( nTargetSize ));
    sal_uInt8*  pTmpTarget = pTarget;

    nBlockBufSize = cBufSize;
    nBlockBufPos = 0;
    pBlockBuf = pSrc;

    while( ProcessOneCode() )
    {
        nCount += nOutBufDataLen;

        if( nCount > nTargetSize )
        {
            sal_uLong   nNewSize = nTargetSize << 1;
            sal_uLong   nOffset = pTmpTarget - pTarget;
            sal_uInt8*  pTmp = static_cast<sal_uInt8*>(rtl_allocateMemory( nNewSize ));

            memcpy( pTmp, pTarget, nTargetSize );
            rtl_freeMemory( pTarget );

            nTargetSize = nNewSize;
            pTmpTarget = ( pTarget = pTmp ) + nOffset;
        }

        memcpy( pTmpTarget, pOutBufData, nOutBufDataLen );
        pTmpTarget += nOutBufDataLen;
        pOutBufData += nOutBufDataLen;
        nOutBufDataLen = 0;

        if ( bEOIFound )
            break;
    }

    rCount = nCount;
    rEOI = bEOIFound;

    return pTarget;
}

bool GIFLZWDecompressor::AddToTable( sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData )
{
    if( nTableSize < 4096 )
    {
        GIFLZWTableEntry* pE = pTable.get() + nTableSize;
        pE->pPrev = pTable.get() + nPrevCode;
        pE->pFirst = pE->pPrev->pFirst;
        GIFLZWTableEntry *pEntry = pTable[nCodeFirstData].pFirst;
        if (!pEntry)
            return false;
        pE->nData = pEntry->nData;
        nTableSize++;

        if ( ( nTableSize == (sal_uInt16) (1 << nCodeSize) ) && ( nTableSize < 4096 ) )
            nCodeSize++;
    }
    return true;
}

bool GIFLZWDecompressor::ProcessOneCode()
{
    sal_uInt16              nCode;
    bool                bRet = false;
    bool                bEndOfBlock = false;

    while( nInputBitsBufSize < nCodeSize )
    {
        if( nBlockBufPos >= nBlockBufSize )
        {
            bEndOfBlock = true;
            break;
        }

        nInputBitsBuf |= ( (sal_uLong) pBlockBuf[ nBlockBufPos++ ] ) << nInputBitsBufSize;
        nInputBitsBufSize += 8;
    }

    if ( !bEndOfBlock )
    {
        // fetch code from input buffer
        nCode = sal::static_int_cast< sal_uInt16 >(
            ( (sal_uInt16) nInputBitsBuf ) & ( ~( 0xffff << nCodeSize ) ));
        nInputBitsBuf >>= nCodeSize;
        nInputBitsBufSize = nInputBitsBufSize - nCodeSize;

        if ( nCode < nClearCode )
        {
            bool bOk = true;
            if ( nOldCode != 0xffff )
                bOk = AddToTable(nOldCode, nCode);
            if (!bOk)
                return false;
        }
        else if ( ( nCode > nEOICode ) && ( nCode <= nTableSize ) )
        {
            if ( nOldCode != 0xffff )
            {
                bool bOk;
                if ( nCode == nTableSize )
                    bOk = AddToTable( nOldCode, nOldCode );
                else
                    bOk = AddToTable( nOldCode, nCode );
                if (!bOk)
                    return false;
            }
        }
        else
        {
            if ( nCode == nClearCode )
            {
                nTableSize = nEOICode + 1;
                nCodeSize = nDataSize + 1;
                nOldCode = 0xffff;
                nOutBufDataLen = 0;
            }
            else
                bEOIFound = true;

            return true;
        }

        nOldCode = nCode;

        if (nCode >= 4096)
            return false;

        // write character(/-sequence) of code nCode in the output buffer:
        GIFLZWTableEntry* pE = pTable.get() + nCode;
        do
        {
            if (pOutBufData == pOutBuf.data()) //can't go back past start
                return false;
            nOutBufDataLen++;
            *(--pOutBufData) = pE->nData;
            pE = pE->pPrev;
        }
        while( pE );

        bRet = true;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
