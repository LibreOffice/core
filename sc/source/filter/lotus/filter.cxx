/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
//  Das geht:   Versionserkennung WKS, WK1 und WK3
//              ...Rest steht in op.cpp

//------------------------------------------------------------------------

#include <tools/solar.h>
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

// Konstanten ------------------------------------------------------------
const sal_uInt16        nBOF = 0x0000;



// externe Variablen -----------------------------------------------------
extern WKTYP        eTyp;   // Typ der gerade in bearbeitung befindlichen Datei
WKTYP               eTyp;

extern sal_Bool         bEOF;           // zeigt Ende der Datei
sal_Bool                bEOF;

extern CharSet      eCharNach;      // Zeichenkonvertierung von->nach
CharSet             eCharNach;

extern CharSet      eCharVon;
CharSet             eCharVon;

extern ScDocument*  pDoc;           // Aufhaenger zum Dokumentzugriff
ScDocument*         pDoc;


extern sal_Char*    pPuffer;        // -> memory.cxx
extern sal_Char*    pDummy1;        // -> memory.cxx

extern OPCODE_FKT   pOpFkt[ FKT_LIMIT ];
                                    // -> optab.cxx, Tabelle moeglicher Opcodes
extern OPCODE_FKT   pOpFkt123[ FKT_LIMIT123 ];
                                    // -> optab.cxx, Table of possible Opcodes

extern long         nDateiLaenge;   // -> datei.cpp, ...der gerade offenen Datei

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

        aStream >> nOpcode >> nLength;
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
        aStream >> nOpcode >> nLength;
        if ( nOpcode == 0x29a)
        {
            aStream.SeekRel(nLength);
            aStream >> nOpcode >> nLength;
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
    aStream >> nOpcode;
    if( nOpcode != nBOF )
        return eWK_UNKNOWN;

    aStream >> nRecLen >> nVersNr;

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
            aStream >> nVersNr;
            if( aStream.IsEof() ) return eWK_Error;
            if( nVersNr == 0x0004 && nRecLen == 26 )
            {   // 4 Bytes von 26 gelesen->22 ueberlesen
                aStream.Read( pDummy1, 22 );
                return eWK3;
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

FltError ScImportLotus123old( SvStream& aStream, ScDocument* pDocument, CharSet eSrc )
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
