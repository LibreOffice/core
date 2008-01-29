/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filter.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:32:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
const UINT16        nBOF = 0x0000;



// externe Variablen -----------------------------------------------------
extern WKTYP        eTyp;   // Typ der gerade in bearbeitung befindlichen Datei
WKTYP               eTyp;

extern BOOL         bEOF;           // zeigt Ende der Datei
BOOL                bEOF;

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


std::map<UINT16, ScPatternAttr> aLotusPatternPool;

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
        UINT16 nOpcode, nLength;

        aStream >> nOpcode >> nLength;
        aPrgrsBar.Progress();
        if( nOpcode == LOTUS_EOF )
        bEOF = TRUE;

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
    UINT16          nOpcode, nVersNr, nRecLen;

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

    bEOF = FALSE;

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


