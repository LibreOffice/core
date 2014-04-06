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

//  Das geht:   Versionserkennung WKS, WK1 und WK3
//              ...Rest steht in op.cpp

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

#include "op.h"

// Konstanten
const sal_uInt16        nBOF = 0x0000;

// externe Variablen
extern WKTYP        eTyp;   // Typ der gerade in bearbeitung befindlichen Datei
WKTYP               eTyp;

extern sal_Bool         bEOF;           // zeigt Ende der Datei
sal_Bool                bEOF;

extern rtl_TextEncoding eCharVon;
rtl_TextEncoding        eCharVon;

extern ScDocument*  pDoc;           // Aufhaenger zum Dokumentzugriff
ScDocument*         pDoc;


extern OPCODE_FKT   pOpFkt[ FKT_LIMIT ];
                                    // -> optab.cxx, Tabelle moeglicher Opcodes
extern OPCODE_FKT   pOpFkt123[ FKT_LIMIT123 ];
                                    // -> optab.cxx, Table of possible Opcodes

LOTUS_ROOT*         pLotusRoot = NULL;


std::map<sal_uInt16, ScPatternAttr> aLotusPatternPool;

static FltError
generate_Opcodes( SvStream& aStream, ScDocument& rDoc,
                  ScfStreamProgressBar& aPrgrsBar, WKTYP eType )
{
    OPCODE_FKT *pOps;
    int         nOps;

    switch(eType)
    {
        case eWK_1:
        case eWK_2:
        pOps = pOpFkt;
        nOps = FKT_LIMIT;
        break;
        case eWK123:
        pOps = pOpFkt123;
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
    while( !bEOF && !aStream.IsEof() && (aStream.Tell() < nStrmSize) )
    {
        sal_uInt16 nOpcode, nLength;

        aStream.ReadUInt16( nOpcode ).ReadUInt16( nLength );
        aPrgrsBar.Progress();
        if( nOpcode == LOTUS_EOF )
        bEOF = sal_True;

        else if( nOpcode == LOTUS_FILEPASSWD )
        return eERR_FILEPASSWD;

        else if( nOpcode < nOps )
        pOps[ nOpcode ] ( aStream, nLength );

        else if( eType == eWK123 &&
             nOpcode == LOTUS_PATTERN )
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
            OP_ApplyPatternArea123(aStream);
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

    MemDelete();

    rDoc.CalcAfterLoad();

    return eERR_OK;
}

WKTYP ScanVersion( SvStream& aStream )
{
    // PREC:    pWKDatei:   Zeiger auf offene Datei
    // POST:    return:     Typ der Datei
    sal_uInt16          nOpcode, nVersNr, nRecLen;

    // erstes Byte muss wegen BOF zwingend 0 sein!
    aStream.ReadUInt16( nOpcode );
    if( nOpcode != nBOF )
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
			{	// 4 bytes of 26 read => skip 22 (read instead of seek to make IsEof() work just in case)
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

FltError ScImportLotus123old( SvStream& aStream, ScDocument* pDocument, rtl_TextEncoding eSrc )
{
    aStream.Seek( 0UL );

    // Zeiger auf Dokument global machen
    pDoc = pDocument;

    bEOF = false;

    eCharVon = eSrc;

    // Speicher besorgen
    if( !MemNew() )
        return eERR_NOMEM;

    InitPage(); // Seitenformat initialisieren (nur Tab 0!)

        // Progressbar starten
    ScfStreamProgressBar aPrgrsBar( aStream, pDocument->GetDocumentShell() );

    // Datei-Typ ermitteln
    eTyp = ScanVersion( aStream );

    aLotusPatternPool.clear();

    return generate_Opcodes( aStream, *pDoc, aPrgrsBar, eTyp );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
