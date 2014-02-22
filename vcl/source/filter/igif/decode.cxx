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



struct GIFLZWTableEntry
{
    GIFLZWTableEntry*   pPrev;
    GIFLZWTableEntry*   pFirst;
    sal_uInt8               nData;
};



GIFLZWDecompressor::GIFLZWDecompressor( sal_uInt8 cDataSize ) :
            nInputBitsBuf       ( 0 ),
            nOutBufDataLen      ( 0 ),
            nInputBitsBufSize   ( 0 ),
            bEOIFound           ( false ),
            nDataSize           ( cDataSize )
{
    pOutBuf = new sal_uInt8[ 4096 ];

    nClearCode = 1 << nDataSize;
    nEOICode = nClearCode + 1;
    nTableSize = nEOICode + 1;
    nCodeSize = nDataSize + 1;
    nOldCode = 0xffff;
    pOutBufData = pOutBuf + 4096;

    pTable = new GIFLZWTableEntry[ 4098 ];

    for( sal_uInt16 i = 0; i < nTableSize; i++ )
    {
        pTable[i].pPrev = NULL;
        pTable[i].pFirst = pTable + i;
        pTable[i].nData = (sal_uInt8) i;
    }
}



GIFLZWDecompressor::~GIFLZWDecompressor()
{
    delete[] pOutBuf;
    delete[] pTable;
}



HPBYTE GIFLZWDecompressor::DecompressBlock( HPBYTE pSrc, sal_uInt8 cBufSize,
                                            sal_uLong& rCount, bool& rEOI )
{
    sal_uLong   nTargetSize = 4096;
    sal_uLong   nCount = 0;
    HPBYTE  pTarget = (HPBYTE) rtl_allocateMemory( nTargetSize );
    HPBYTE  pTmpTarget = pTarget;

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
            HPBYTE  pTmp = (HPBYTE) rtl_allocateMemory( nNewSize );

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



void GIFLZWDecompressor::AddToTable( sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData )
{
    GIFLZWTableEntry* pE;

    if( nTableSize < 4096 )
    {
        pE = pTable + nTableSize;
        pE->pPrev = pTable + nPrevCode;
        pE->pFirst = pE->pPrev->pFirst;
        pE->nData = pTable[ nCodeFirstData ].pFirst->nData;
        nTableSize++;

        if ( ( nTableSize == (sal_uInt16) (1 << nCodeSize) ) && ( nTableSize < 4096 ) )
            nCodeSize++;
    }
}



bool GIFLZWDecompressor::ProcessOneCode()
{
    GIFLZWTableEntry*   pE;
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
            if ( nOldCode != 0xffff )
                AddToTable( nOldCode, nCode );
        }
        else if ( ( nCode > nEOICode ) && ( nCode <= nTableSize ) )
        {
            if ( nCode == nTableSize )
                AddToTable( nOldCode, nOldCode );
            else
                AddToTable( nOldCode, nCode );
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

        // write character(/-sequence) of code nCode in the output buffer:
        pE = pTable + nCode;
        do
        {
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
