/*************************************************************************
 *
 *  $RCSfile: filter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
//  Das geht:   Versionserkennung WKS, WK1 und WK3
//              ...Rest steht in op.cpp

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <tools/solar.h>
#include <string.h>
#include <assert.h>

#include "filter.hxx"
#include "document.hxx"
#include "compiler.hxx"
#include "scerrors.hxx"

#include "fltprgrs.hxx"
#include "root.hxx"
#include "lotrange.hxx"
#include "optab.h"
#include "scmem.h"
#include "decl.h"
#include "tool.h"

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

extern long         nDateiLaenge;   // -> datei.cpp, ...der gerade offenen Datei

LOTUS_ROOT*         pLotusRoot = NULL;


WKTYP               ScanVersion( SvStream& aStream );


extern FltError     ScImportLotus123old( SvStream&, ScDocument*, CharSet eSrc );



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
    FilterProgressBar   aPrgrsBar( aStream );

    // Datei-Typ ermitteln
    eTyp = ScanVersion( aStream );

    switch( eTyp )  // auswaehlen der Tabelle
    {
        case eWK_1:
        case eWK_2:     break;
        case eWK3:      return eERR_NI;
        case eWK_Error: return eERR_FORMAT;
        default:        return eERR_UNKN_WK;
    }

    // Init von Lotus Root-Daten -> Ctor LotusImport

    // Aufdroeseln der Opcodes

    while( !bEOF )
    {
        UINT16          nOpcode, nLaenge;

        // ein Lotus-Record einlesen
        aStream >> nOpcode >> nLaenge;

        aPrgrsBar.Progress();


        if( aStream.IsEof() )
            bEOF = TRUE;
        else if( nOpcode == 75 )
            return eERR_FILEPASSWD;
        else if( nOpcode < FKT_LIMIT )
            ( pOpFkt[ nOpcode ] )( aStream, nLaenge );
        else
            aStream.SeekRel( nLaenge );
    }

    MemDelete();

    pDoc->CalcAfterLoad();

    return eERR_OK;
}


WKTYP ScanVersion( SvStream& aStream )
{
    // PREC:    pWKDatei:   Zeiger auf offene Datei
    // POST:    return:     Typ der Datei
    UINT16          nOpcode, nVersNr, nRecLaenge;

    // erstes Byte muss wegen BOF zwingend 0 sein!
    aStream >> nOpcode;
    if( nOpcode != nBOF )
        return eWK_UNKNOWN;

    aStream >> nRecLaenge >> nVersNr;

    if( aStream.IsEof() )
        return eWK_Error;

    switch( nVersNr )
    {
        case 0x0404:
            if( nRecLaenge == 2 )
                return eWK_1;
            else
                return eWK_UNKNOWN;
            break;

        case 0x0406:
            if( nRecLaenge == 2 )
                return eWK_2;
            else
                return eWK_UNKNOWN;
            break;

        case 0x1000:
            aStream >> nVersNr;
            if( aStream.IsEof() ) return eWK_Error;
            if( nVersNr == 0x0004 && nRecLaenge == 26 )
            {   // 4 Bytes von 26 gelesen->22 ueberlesen
                aStream.Read( pDummy1, 22 );
                return eWK3;
            }
            break;
    }

    return eWK_UNKNOWN;
}




