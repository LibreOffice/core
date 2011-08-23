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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "doc.hxx"
#include "pam.hxx"
#include "swtable.hxx"
#include "fltglbls.hxx"
namespace binfilter {




//------------------------------------------------------------------------
FilterGlobals::FilterGlobals( SwDoc& rDoc, const SwPaM& rPam )
    : aStandard( String::CreateFromAscii("General") ),
    aTblFmts( 0, 10 )
{
    pD = &rDoc;			// "globaler" Dokument-Pointer
    pPam = new SwPaM( *rPam.GetPoint() );
    pTable = 0;
    pNumFormatter = 0;

    eDefLanguage = LANGUAGE_SYSTEM;
    nDefFormat = 0xFFFFFFFF;

    nColStart = nRowStart = 1;
    nColEnd = nRowEnd = 0;
    nAnzCols = nAnzRows = 0;
}

FilterGlobals::~FilterGlobals()
{
    // JP 13.08.98: TabellenUmrandungen optimieren - Bug 53525
    for( USHORT n = aTblFmts.Count(); n; )
    {
        SwTable* pTbl = SwTable::FindTable( (SwFrmFmt*)aTblFmts[ --n ] );
        if( pTbl )
            pTbl->GCBorderLines();
    }

    delete pPam;
}

//------------------------------------------------------------------------
void FilterGlobals::SetRange( USHORT nCS, USHORT nCE, USHORT nRS, USHORT nRE )
{
    if( nCE < nCS )
    {
        if( nCS == 0 )
            nCE = 0;
        else
            nCE = nCS - 1;
    }

    if( nRE < nRS )
    {
        if( nRS == 0 )
            nRE = 0;
        else
            nRE = nRS - 1;
    }

    nColStart = nCS;
    nColEnd = nCE;
    nRowStart = nRS;
    nRowEnd = nRE;
    nAnzCols = 1 + nCE - nCS;
    nAnzRows = 1 + nRE - nRS;
}



BOOL FilterGlobals::ColRangeLimitter( USHORT &rCS, USHORT &rCE )
{
    //	PREC:	rCS <= rCE
    //	POST:	wenn [rCS, rCE] und [nColStart,nColEnd] disjunkte Intervalle,
    //				return = FALSE;
    //			sonst
    //				return = TRUE
    //				rCS = MAX(rCS,nColStart)
    //				rCE = MIN(rCE,nColEnd)
    BOOL bRet;
    DBG_ASSERT( rCS <= rCE,
        "-FilterGlobals::ColRangeLimitter(): Startspalte > Endspalte!" );
    if( rCS > nColEnd || rCE < nColStart )
        bRet = FALSE;
    else
        {
        bRet = TRUE;
        if( rCS < nColStart )	rCS = nColStart;
        if( rCE > nColEnd )		rCE = nColEnd;
        }
    return bRet;
}

void FilterGlobals::InsertText( USHORT nCol, USHORT nRow, const String& rStr )
{
    pPam->GetPoint()->nNode = *pTable->GetTabLines()[ nRow ]->
                                        GetTabBoxes()[ nCol ]->GetSttNd();
    pPam->Move( fnMoveForward, fnGoCntnt );

    pD->Insert( *pPam, rStr );
}

void FilterGlobals::CreateTable()
{
    pTable = pD->InsertTable( *pPam->GetPoint(),
                                AnzRows(),	// nRows
                                AnzCols(),	// nCols
                                HORI_LEFT );		// SwHoriOrient

    // JP 13.08.98: TabellenUmrandungen optimieren - Bug 53525
    void* p = pTable->GetFrmFmt();
    aTblFmts.Insert( p, aTblFmts.Count() );
}

void FilterGlobals::InsertAttr( const SfxPoolItem& rItem )
{
    SfxItemSet aTxtAttr( pD->GetAttrPool(), rItem.Which(), rItem.Which() );
    aTxtAttr.Put( rItem );
    pD->Insert( *pLotGlob->pPam, aTxtAttr );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
