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

//  Discover WKS, WK1 und WK3; s.a op.cpp

#include <string.h>
#include <map>

#include "filter.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "scerrors.hxx"

#include "root.hxx"
#include "lotrange.hxx"
#include "optab.h"
#include "scmem.h"
#include "decl.h"
#include "tool.h"
#include "fprogressbar.hxx"
#include "lotfilter.hxx"

static FltError
generate_Opcodes(LotusContext &rContext, SvStream& aStream,
                  ScfStreamProgressBar& aPrgrsBar)
{
    OPCODE_FKT *pOps;
    int         nOps;

    switch (rContext.eTyp)
    {
        case eWK_1:
        case eWK_2:
            pOps = rContext.pOpFkt;
            nOps = FKT_LIMIT;
        break;
        case eWK123:
            pOps = rContext.pOpFkt123;
            nOps = FKT_LIMIT123;
        break;
        case eWK3:      return eERR_NI;
        case eWK_Error: return eERR_FORMAT;
        default:        return eERR_UNKN_WK;
     }

    // #i76299# seems that SvStream::IsEof() does not work correctly
    aStream.Seek( STREAM_SEEK_TO_END );
    sal_Size nStrmSize = aStream.Tell();
    aStream.Seek( STREAM_SEEK_TO_BEGIN );
    while( !rContext.bEOF && !aStream.IsEof() && (aStream.Tell() < nStrmSize) )
    {
        sal_uInt16 nOpcode, nLength;

        aStream.ReadUInt16( nOpcode ).ReadUInt16( nLength );
        aPrgrsBar.Progress();
        if( nOpcode == LOTUS_EOF )
            rContext.bEOF = true;
        else if( nOpcode == LOTUS_FILEPASSWD )
            return eERR_FILEPASSWD;
        else if( nOpcode < nOps )
            pOps[ nOpcode ] (rContext, aStream, nLength);
        else if (rContext.eTyp == eWK123 && nOpcode == LOTUS_PATTERN)
        {
            // This is really ugly - needs re-factoring ...
            aStream.SeekRel(nLength);
            aStream.ReadUInt16( nOpcode ).ReadUInt16( nLength );
            if ( nOpcode == 0x29a)
            {
                aStream.SeekRel(nLength);
                aStream.ReadUInt16( nOpcode ).ReadUInt16( nLength );
                if ( nOpcode == 0x804 )
                {
                    aStream.SeekRel(nLength);
                    OP_ApplyPatternArea123(rContext, aStream);
                }
                else
                    aStream.SeekRel(nLength);
            }
            else
                aStream.SeekRel(nLength);
        }
        else
            aStream.SeekRel( nLength );
    }

    MemDelete(rContext);

    rContext.pDoc->CalcAfterLoad();

    return eERR_OK;
}

WKTYP ScanVersion(LotusContext &rContext, SvStream& aStream)
{
    // PREC:    pWKDatei:   pointer to open file
    // POST:    return:     type of file
    sal_uInt16 nOpcode(0), nVersNr(0), nRecLen(0);

    // first byte has to be 0 because of BOF!
    aStream.ReadUInt16( nOpcode );
    if (nOpcode != rContext.nBOF)
        return eWK_UNKNOWN;

    aStream.ReadUInt16( nRecLen ).ReadUInt16( nVersNr );

    if( aStream.IsEof() )
        return eWK_Error;

    switch( nVersNr )
    {
        case 0x0404:
            if( nRecLen == 2 )
                return eWK_1;
            else
                return eWK_UNKNOWN;

        case 0x0406:
            if( nRecLen == 2 )
                return eWK_2;
            else
                return eWK_UNKNOWN;

        case 0x1000:
            aStream.ReadUInt16( nVersNr );
            if( aStream.IsEof() ) return eWK_Error;
            if( nVersNr == 0x0004 && nRecLen == 26 )
            {   // 4 bytes of 26 read => skip 22 (read instead of seek to make IsEof() work just in case)
                sal_Char aDummy[22];
                aStream.Read( aDummy, 22 );
                return aStream.IsEof() ? eWK_Error : eWK3;
            }
            break;
        case 0x1003:
            if( nRecLen == 0x1a )
                return eWK123;
            else
                return eWK_UNKNOWN;
        case 0x1005:
            if( nRecLen == 0x1a )
                return eWK123;
            else
                return eWK_UNKNOWN;
    }

    return eWK_UNKNOWN;
}

FltError ScImportLotus123old(LotusContext& rContext, SvStream& aStream, ScDocument* pDocument, rtl_TextEncoding eSrc )
{
    aStream.Seek( 0UL );

    // make document pointer global
    rContext.pDoc = pDocument;
    rContext.bEOF = false;
    rContext.eCharVon = eSrc;

    // allocate memory
    if( !MemNew(rContext) )
        return eERR_NOMEM;

    // initialize page format (only Tab 0!)
    // initialize page format; meaning: get defaults from SC TODO:
    //scGetPageFormat( 0, &aPage );

    // start progressbar
    ScfStreamProgressBar aPrgrsBar( aStream, pDocument->GetDocumentShell() );

    // detect file type
    rContext.eTyp = ScanVersion(rContext, aStream);
    rContext.aLotusPatternPool.clear();

    return generate_Opcodes(rContext, aStream, aPrgrsBar);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
