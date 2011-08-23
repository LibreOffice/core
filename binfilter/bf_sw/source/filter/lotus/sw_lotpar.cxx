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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _FLTGLBLS_HXX
#include <fltglbls.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _LOTPAR_HXX
#include <lotpar.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif
namespace binfilter {

LotGlob *pLotGlob = NULL;





SwLotusParser::SwLotusParser( SwDoc &rD, const SwPaM & rCrsr,
                                SvStream& rInInit, int bReadNewDoc,
                                CharSet eQ )
    : bNewDoc( bReadNewDoc )
{
    eQuellChar = eQ;
    pIn = &rInInit;
    pLotGlob = new LotGlob( rD, rCrsr );

    nReadBuffSize = 2048;
    pReadBuff = new char[ nReadBuffSize ];

//	aColRowBuff.SetDefWidth( ( USHORT ) ( 9192 ) );	// 4 Zeichen Standardbreite
}

SwLotusParser::~SwLotusParser()
{
    delete[] pReadBuff;
    delete pLotGlob;
    pLotGlob = NULL;
}


ULONG SwLotusParser::CallParser()
{
    static const sal_Char* aNames[4] = {
        "Excel_Lotus/MinRow", "Excel_Lotus/MaxRow",
        "Excel_Lotus/MinCol", "Excel_Lotus/MaxCol"
    };
    sal_uInt32 aVal[4];
    SwFilterOptions aOpt( 4, aNames, aVal );

    USHORT nMinRow = ( USHORT ) aVal[ 0 ];
    USHORT nMaxRow = ( USHORT ) aVal[ 1 ];
    USHORT nMinCol = ( USHORT ) aVal[ 2 ];
    USHORT nMaxCol = ( USHORT ) aVal[ 3 ];

    USHORT nAnzNodes = 65000U - pLotGlob->pD->GetNodes().Count();

    if( nMaxRow < nMinRow )
    {
        USHORT nTemp = nMinRow;
        nMinRow = nMaxRow;
        nMaxRow = nTemp;
    }

    if( nMaxCol < nMinCol )
    {
        USHORT nTemp = nMinCol;
        nMinCol = nMaxCol;
        nMaxCol = nTemp;
    }

    if( nMaxRow - nMinRow == 0 )
        nMaxRow = nMinRow + 30;		// Default bei 0 Rows

    if( nMaxCol - nMinCol == 0 )
        nMaxCol = nMinCol + 15;		// Default bei 0 Cols

    if( nAnzNodes < ( nMaxRow - nMinRow ) * ( nMaxCol - nMinCol ) * 3 )
        return ERR_EXCLOT_WRONG_RANGE;

    pLotGlob->SetRange( nMinCol, nMaxCol, nMinRow, nMaxRow ); // Default-Einstellung

    Parse();

    return 0;	// kein Fehler
}


ULONG LotusReader::Read( SwDoc &rDoc, SwPaM &rPam,
                    const String & /* FileName, falls benoetigt wird */ )
{
    ULONG nRet;
    if( !pStrm )
    {
        ASSERT( FALSE, "Lotus-Read ohne Stream" );
        nRet = ERR_SWG_READ_ERROR;
    }
    else
    {
        SwLotusParser* pParser = new SwLotusParser( rDoc, rPam, *pStrm,
                                                    !bInsertMode, eCodeSet );
        nRet = pParser->CallParser();		// 0 == kein Fehler aufgetreten
        delete pParser;
    }

    return nRet;
}



}
