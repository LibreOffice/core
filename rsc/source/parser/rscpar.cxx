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

#include <string.h>
#include <rscpar.hxx>
#include <rscdb.hxx>

void RscFileInst::Init()
{
    nLineNo = 0;
    nLineBufLen = 256;
    pLine = (char *)rtl_allocateMemory( nLineBufLen );
    *pLine = '\0';
    nScanPos = 0;
    cLastChar = '\0';
    bEof = sal_False;
};

RscFileInst::RscFileInst( RscTypCont * pTC, sal_uLong lIndexSrc,
                          sal_uLong lFIndex, FILE * fFile )
{
    pTypCont = pTC;
    Init();

    lFileIndex = lFIndex;
    lSrcIndex = lIndexSrc;
    fInputFile = fFile;

    //Status: Zeiger am Ende des Lesepuffers
    nInputPos = nInputEndPos = nInputBufLen = READBUFFER_MAX;
    pInput    = (char *)rtl_allocateMemory( nInputBufLen );
}

RscFileInst::~RscFileInst(){
    if( pInput )
        rtl_freeMemory( pInput );
    if( pLine )
        rtl_freeMemory( pLine );
}

int RscFileInst::GetChar()
{
    if( pLine[ nScanPos ] )
        return( pLine[ nScanPos++ ] );
    else if( nInputPos >= nInputEndPos && nInputEndPos != nInputBufLen )
    {
        // Dateiende
        bEof = sal_True;
        return 0;
    }
    else
    {
        GetNewLine();
        return( '\n' );
    }
}

void RscFileInst::GetNewLine()
{
    nLineNo++;
    nScanPos = 0;

    //laeuft bis Dateiende
    sal_uInt32 nLen = 0;
    while( (nInputPos < nInputEndPos) || (nInputEndPos == nInputBufLen) )
    {
        if( (nInputPos >= nInputEndPos) && fInputFile )
        {
            nInputEndPos = fread( pInput, 1, nInputBufLen, fInputFile );
            nInputPos = 0;
        }

        while( nInputPos < nInputEndPos )
        {
            //immer eine Zeile lesen
            if( nLen >= nLineBufLen )
            {
                nLineBufLen += 256;
                // einen dazu fuer '\0'
                pLine = (char*)rtl_reallocateMemory( pLine, nLineBufLen +1 );
            }

            // cr lf, lf cr, lf oder cr wird '\0'
            if( pInput[ nInputPos ] == '\n' ){
                nInputPos++;
                if( cLastChar != '\r' ){
                    cLastChar = '\n';
                    pLine[ nLen++ ] = '\0';
                    goto END;
                }
            }
            else if( pInput[ nInputPos ] == '\r' ){
                nInputPos++;
                if( cLastChar != '\n' ){
                    cLastChar = '\r';
                    pLine[ nLen++ ] = '\0';
                    goto END;
                }
            }
            else
            {
                pLine[ nLen++ ] = pInput[ nInputPos++ ];
                if( nLen > 2 )
                {
                    if( (unsigned char)pLine[nLen-3] == 0xef &&
                        (unsigned char)pLine[nLen-2] == 0xbb &&
                        (unsigned char)pLine[nLen-1] == 0xbf )
                    {
                        nLen -= 3;
                    }
                }
            }
        };
    };

    // Abbruch ueber EOF
    pLine[ nLen ] = '\0';

END:
    if( pTypCont->pEH->GetListFile() ){
        char buf[ 10 ];

        sprintf( buf, "%5d ", (int)GetLineNo() );
        pTypCont->pEH->LstOut( buf );
        pTypCont->pEH->LstOut( GetLine() );
        pTypCont->pEH->LstOut( "\n" );
    }
}

void RscFileInst::SetError( ERRTYPE aError )
{
    if( aError.IsOk() )
    {
        aFirstError = aError;
        nErrorLine  = GetLineNo();
        nErrorPos   = GetScanPos() -1;
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
