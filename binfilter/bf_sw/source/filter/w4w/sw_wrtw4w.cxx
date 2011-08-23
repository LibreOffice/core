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

#include <stdlib.h>
#include <stdio.h>


#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <bf_svx/paperinf.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <bf_svx/boxitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif


#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // ...Percent()
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _W4WFLT_HXX
#include <w4wflt.hxx>
#endif
#ifndef _PAGEITER_HXX
#include <pageiter.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _W4WPAR_HXX
#include <w4wpar.hxx>
#endif
#ifndef _WRTW4W_HXX
#include <wrtw4w.hxx>
#endif
#ifndef _SWFLTOPT_HXX
#include <swfltopt.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif
namespace binfilter {

const sal_Char __FAR_DATA sW4W_RECBEGIN[3] = { cW4W_BEGICF, cW4W_LED, 0 };
const sal_Char __FAR_DATA sW4W_TERMEND[3] = { cW4W_TXTERM, cW4W_RED, 0 };


//-------------------------------------------------------------------
// Tabelle

void GetW4WWriter( const String& rFltName, WriterRef& xRet )
{
    xRet = new SwW4WWriter( rFltName );
}


BOOL SwW4WWriter::IsStgWriter()
{
    return bStorageFlag;
}

void SwW4WWriter::OutW4W_CountTableSize( const SwTable* pTab, USHORT& rLines,
                                         USHORT& rMaxCols, USHORT& rBrtCol )
{
    rLines = pTab->GetTabLines().Count();
    USHORT nC, i;
    rMaxCols = 0;
    rBrtCol = 0;
    for( i=0; i<rLines; i++){
        nC = pTab->GetTabLines()[i]->GetTabBoxes().Count();
        if ( nC > rMaxCols ){
             rMaxCols = nC;
             rBrtCol = i;
        }
    }
}


BYTE SwW4WWriter::OutW4W_GetTableColPos( const SwTableLine* pL,
                            SwHoriOrient eHor, SwTwips* pCellPos )
{
    USHORT nCols = pL->GetTabBoxes().Count();
    SwTwips nLeft = nPgLeft; //(USHORT)(pLR->GetLeft());
    USHORT j;


    BYTE nCode = 0x0a;
    pCellPos[0]=0;

    for( j=0; j<nCols; j++ ){				// Berechne Cell-Positionen
        const SwTableBox* pB = pL->GetTabBoxes()[j];
        SwTwips nWid = pB->GetFrmFmt()->GetFrmSize().GetWidth();
        if ( nWid > 0x10000 )
            nWid = ( nWid / 0x10000 ) * ( nPgRight - nPgLeft )
                   / ( LONG_MAX / 0x10000 );  // SpezialWert
        pCellPos[j+1] = pCellPos[j] + nWid;
    }
    USHORT nWidth = (USHORT)pCellPos[nCols];		 // Merke Tabellenbreite

    switch ( eHor ){
    case HORI_NONE:   nCode |= 0x40; break;
    case HORI_RIGHT:  nCode |= 0x30;
                      nLeft = nPgRight - nWidth;
                      break;
    case HORI_CENTER: nCode |= 0x20;
                      nLeft = ( nPgLeft + nPgRight - nWidth ) / 2;
                      break;
    case HORI_FULL:   nCode |= 0x10;
                      nLeft = ( nPgLeft + nPgRight - nWidth ) / 2;
                      break;
                            // alles andere wird zu left
    }

    nLeft -= nFlyLeft;	// zumindest WW2 verlangt Pos relativ zu FlyFrame
    if ( nLeft < 0 )
        nLeft = 0;

    for( j=0; j<=nCols; j++ ){		// korrigiere Cell-Positionen
        pCellPos[j] += nLeft;
    }
    return nCode;
}


void SwW4WWriter::OutW4W_WriteTableHeader( BYTE nCode, USHORT nCols,
                                           SwTwips* pCellPos )
{
    USHORT j;

    OutHex( Strm() << sW4W_RECBEGIN << "CDS", nCode )
                << cW4W_TXTERM;
    OutULong( nCols ) << cW4W_TXTERM;

    for( j=0; j<nCols; j++ )
    {
        OutULong( pCellPos[j] / 144 ) << cW4W_TXTERM;
        OutULong( (pCellPos[j+1] -1 ) / 144) << cW4W_TXTERM
                << '0' << cW4W_TXTERM;
    }

    for( j=0; j<nCols; j++ )
    {
        OutULong( pCellPos[j] ) << cW4W_TXTERM;
        OutULong( pCellPos[j+1] -1 ) << cW4W_TXTERM
                << '0' << cW4W_TXTERM;
    }
    Strm() << cW4W_RED << sW4W_RECBEGIN << "BCM" << cW4W_RED;
}

void SwW4WWriter::OutW4W_WriteTableLineHeader( USHORT nCols, USHORT nLineHeight,
                                               USHORT *pBorder )
{
    USHORT j;
    OutULong( Strm() << sW4W_RECBEGIN << "BRO", nCols )
                << cW4W_TXTERM;
    OutULong( nLineHeight / 240 ) << cW4W_TXTERM;
    OutULong( nLineHeight ) << cW4W_TXTERM;

    if ( pBorder )
        for ( j=0; j<nCols; j++ )
            OutHex4( pBorder[j] ) << cW4W_TXTERM; // Border-Code
    else
        for ( j=0; j<nCols; j++ )
                Strm() << "0000" << cW4W_TXTERM;    // Border-Code 0

    Strm() << '0' << sW4W_TERMEND;     // Left justify
}

void SwW4WWriter::OutW4W_WriteTableBoxHeader( USHORT nNumCol, USHORT nSpan )
{
    OutULong( Strm() << sW4W_RECBEGIN << "BCO", nNumCol )
                << cW4W_TXTERM;
    OutULong( nSpan ) << cW4W_TXTERM
            << '1' << cW4W_TXTERM   // hor. ausgeglichen
            << '0' << cW4W_TXTERM   // vert. ausgeglichen
            << '0' << cW4W_TXTERM   // Linksbuendig
            << '2' << cW4W_TXTERM   // 2 Dezimalen
            << '0' << sW4W_TERMEND;    // V-Align: Top
}



// OutW4W_TestTableLineWW2 returned, ob sich die Position der Spalten
//							geaendert hat

BOOL SwW4WWriter::OutW4W_TestTableLineWW2( const SwTable* pTab,
    USHORT nLineNo, USHORT nCols, SwTwips* pCellPos )
{
    const SwTableLine* pL = pTab->GetTabLines()[nLineNo];

    if ( nCols != pL->GetTabBoxes().Count() )
        return TRUE;

    SwTwips nPos=pCellPos[0];

    for( USHORT j=0; j<pL->GetTabBoxes().Count(); j++){

        const SwTableBox* pB = pL->GetTabBoxes()[j];
        SwTwips nW = pB->GetFrmFmt()->GetFrmSize().GetWidth();
        if ( nW > 0x10000 ) 			// SpezialWert
            nW = (nW/0x10000) * (nPgRight-nPgLeft) / (LONG_MAX/0x10000 );
        nPos += nW; 					// aktualisiere x-Position
        SwTwips nDiff = pCellPos[j+1] - nPos;
        if ( nDiff > 56 || nDiff < -56 )
            return TRUE;
    }
    return FALSE;
}

// OutW4W_GetBorder holt zu einem gegebenen Frameformat den Border,
// konvertiert ihn in das MASO-Format und liefert ihn in rBorder zurueck.
// ist kein Rahmen vorhanden, wird FALSE returned.

BOOL SwW4WWriter::OutW4W_GetBorder( const SvxBoxItem* pBox, USHORT& rBorder )
{
    rBorder = 0;

    if ( !pBox )  return FALSE;

    for ( USHORT i=0; i<4; i++ ){
        const SvxBorderLine *pBrd;
        switch (i){
        case 0: pBrd = pBox->GetRight(); break;
        case 1: pBrd = pBox->GetBottom(); break;
        case 2: pBrd = pBox->GetLeft(); break;
        case 3: pBrd = pBox->GetTop(); break;
        }
        if ( pBrd ){
            if ( pBrd->GetInWidth() && pBrd->GetOutWidth() ){
                rBorder |= 2 << (i*4);	  // doppelt umrandet
            }else{
                SwTwips nBr = pBrd->GetInWidth() + pBrd->GetOutWidth();
                if ( nBr <= 5 )
                    rBorder |= 7 << (i*4);	// Haarlinie
                else if ( nBr <= 50 )
                    rBorder |= 1 << (i*4);	// duenne Linie
                else if ( nBr <= 80 )
                    rBorder |= 5 << (i*4);	// dicke Linie
                else
                    rBorder |= 6 << (i*4);	// extra dicke Linie
            }
        }
    }
    return TRUE;
}

void SwW4WWriter::OutW4W_TableLineWW2( const SwTable* pTab,

    USHORT nLineNo, USHORT nCols, USHORT nLineHeight, SwTwips* pCellPos )
{
    const SwTableLine* pL = pTab->GetTabLines()[nLineNo];
    USHORT j;

    USHORT* pBorders = new USHORT[nCols];

    for( j=0; j<nCols; j++){						// hole Umrandungen
        if ( j < pL->GetTabBoxes().Count()){
            const SwFrmFmt * pFr;
            pFr = pL->GetTabBoxes()[j]->GetFrmFmt();	// hole FrameFmt
            if ( pFr )
                OutW4W_GetBorder( &pFr->GetBox(), pBorders[j] );
        }else{
            pBorders[j] = 0;
        }
    }

    OutW4W_WriteTableLineHeader( nCols, nLineHeight, pBorders );

    DELETEZ( pBorders );

    for( j=0; j<nCols; j++){						// hole Zellinhalt

        const SwStartNode *pSN = 0;

        OutW4W_WriteTableBoxHeader( j, 1 );

        if ( j < pL->GetTabBoxes().Count())
            pSN = pL->GetTabBoxes()[j]->GetSttNd(); // hole StartNode
                                                    // des TabellenInhalts
        if ( pSN ){ 		// StartNode wirklich vorhanden ?

            pOrigPam = pCurPam = NewSwPaM( *pDoc,
                                pSN->GetIndex(), pSN->EndOfSectionIndex() );

            BOOL bOldTab = bTable;
            bTable = TRUE;
            pCurPam->Exchange();
            Out_SwDoc( pCurPam, FALSE ); // Inhalt der Zelle (mit Attributen)
            bTable = bOldTab;

            delete pCurPam;
        }
        Strm() << sW4W_RECBEGIN << "HCB" << cW4W_RED;
    }
}

// OutW4W_SwTableWW2 schreibt eine Tabelle 'raus. Dabei wird bei jeder
// Aenderung der Zellenbreiten die aktuelle Tabelle beendet und eine
// neue mit den passenden Zellenbreiten direkt drangehaengt. Dieses
// ist bei WW2 noetig, da der WW2-Exportfilter die verbundenen Zellen
// ignoriert statt unterstuetzt. Evtl. kann diese Vorgehensweise auch
// fuer andere Exportformate guenstig sein, da Verschiebungen von
// Trennlinien im Zielformat dargestellt werden.

void SwW4WWriter::OutW4W_SwTableWW2( const SwTable* pTab, USHORT nLines,
        USHORT nMaxCols, USHORT nBCol, SwHoriOrient eHor, SwTwips* pCellPos )
{
    const SwTableLine* pL = pTab->GetTabLines()[0];
    USHORT nCols = pL->GetTabBoxes().Count();
    USHORT nHei = (USHORT)(pL->GetFrmFmt()->GetFrmSize().GetHeight());

    BYTE nCode = OutW4W_GetTableColPos( pL, eHor, pCellPos );
    OutW4W_WriteTableHeader( nCode, nCols, pCellPos );

    for( USHORT i=0; i<nLines; i++){
        BOOL bRet = OutW4W_TestTableLineWW2( pTab, i, nCols, pCellPos );
        if ( bRet ){		// Spalten geaendert -> neue Tabelle
            Strm() << sW4W_RECBEGIN << "ECM" << cW4W_RED;
            pL = pTab->GetTabLines()[i];
            nCols = pL->GetTabBoxes().Count();
            BYTE nCode = OutW4W_GetTableColPos( pL, eHor, pCellPos );
            OutW4W_WriteTableHeader( nCode, nCols, pCellPos );
        }
        OutW4W_TableLineWW2( pTab, i, nCols, nHei, pCellPos );
    }
    Strm() << sW4W_RECBEGIN << "ECM" << cW4W_RED;
}

void SwW4WWriter::OutW4W_TableLineNorm( const SwTable* pTab,
    USHORT nLineNo, USHORT nMaxCols, USHORT nLineHeight, SwTwips* pCellPos )
{
    const SwTableLine* pL = pTab->GetTabLines()[nLineNo];
    USHORT nHidden = nMaxCols - pL->GetTabBoxes().Count();
    USHORT* pBorders = new USHORT[nMaxCols];
    USHORT j, k;

    SwTwips nPos=pCellPos[0];
    for( j=0, k=0; j<nMaxCols&&k<nMaxCols; j++){		// Hole Umrandungen

                //!!! 2 Indices mitfuehren: j fuer Sw, k fuer W4W

        USHORT nSpan = 1;
        const SwFrmFmt * pFr = 0;

        if ( j < pL->GetTabBoxes().Count()){
            const SwTableBox* pB = pL->GetTabBoxes()[j];
            SwTwips nW = pB->GetFrmFmt()->GetFrmSize().GetWidth();
            if ( nW > 0x10000 ) 			// SpezialWert
                nW = (nW/0x10000) * (nPgRight-nPgLeft) / (LONG_MAX/0x10000 );
            nPos += nW; 					// aktualisiere x-Position
            pFr = pB->GetFrmFmt();			// hole FrameFmt fuer Umrandungen
        }

        if ( nHidden )
            while ( k+nSpan < nMaxCols
                    && nPos > pCellPos[k+nSpan] + 10 ){
                nSpan++;
            }
        if ( pFr )
            OutW4W_GetBorder( &pFr->GetBox(), pBorders[j] );	// hole Umrandungen

        k += nSpan;
    }


    OutW4W_WriteTableLineHeader( nMaxCols, nLineHeight, pBorders );

    DELETEZ( pBorders );

    nPos=pCellPos[0];

    for( j=0, k=0; j<nMaxCols&&k<nMaxCols; j++){	 // Hole Zelleninhalt

                //!!! 2 Indices mitfuehren: j fuer Sw, k fuer W4W

        USHORT nSpan = 1;
        const SwStartNode *pSN = 0;

        if ( j < pL->GetTabBoxes().Count()){
            const SwTableBox* pB = pL->GetTabBoxes()[j];
            SwTwips nW = pB->GetFrmFmt()->GetFrmSize().GetWidth();
            if ( nW > 0x10000 ) 			// SpezialWert
                nW = (nW/0x10000) * (nPgRight-nPgLeft) / (LONG_MAX/0x10000 );
            nPos += nW; 					// aktualisiere x-Position
            pSN = pB->GetSttNd();			// hole StartNode TabellenInhalt
        }

        if ( nHidden )
            while ( k+nSpan < nMaxCols
                    && nPos > pCellPos[k+nSpan] + 10 ){
                nSpan++;
            }

        OutW4W_WriteTableBoxHeader( k, nSpan );

        if ( pSN ){ 		// StartNode wirklich vorhanden ?

            pOrigPam = pCurPam = NewSwPaM( *pDoc,
                                pSN->GetIndex(), pSN->EndOfSectionIndex() );

            BOOL bOldTab = bTable;
            bTable = TRUE;
            pCurPam->Exchange();
            Out_SwDoc( pCurPam, FALSE ); // Inhalt der Zelle (mit Attributen)
            bTable = bOldTab;

            delete pCurPam;
        }

        Strm() << sW4W_RECBEGIN << "HCB" << cW4W_RED;

        k += nSpan;
    }
}


// OutW4W_SwTableNorm schreibt eine Tabelle 'raus. Dabei wird versucht,
// hor. verbundene Zellen anhand der Zellenbreiten zu erkennen
// und als entsprechend verbundene Zellen herauszuschreiben.

void SwW4WWriter::OutW4W_SwTableNorm( const SwTable* pTab, USHORT nLines,
        USHORT nMaxCols, USHORT nBCol, SwHoriOrient eHor, SwTwips* pCellPos )
{
    const SwTableLine* pL = pTab->GetTabLines()[nBCol];
    USHORT nHei = (USHORT)(pL->GetFrmFmt()->GetFrmSize().GetHeight());

    BYTE nCode = OutW4W_GetTableColPos( pL, eHor, pCellPos );
    OutW4W_WriteTableHeader( nCode, nMaxCols, pCellPos );


    for( USHORT i=0; i<nLines; i++)
        OutW4W_TableLineNorm( pTab, i, nMaxCols, nHei, pCellPos );

    Strm() << sW4W_RECBEGIN << "ECM" << cW4W_RED;
}


void SwW4WWriter::OutW4W_SwTable( const SwTableNode* pTableNd )
{
#if OSL_DEBUG_LEVEL > 1
    Strm() << sW4W_RECBEGIN << "NOP_Table_Node" << sW4W_TERMEND;
//	Sound::Beep();
#endif
    const SwTable* pTab = &(pTableNd->GetTable());
    USHORT nLines, nMaxCols, nBCol;

    OutW4W_CountTableSize( pTab, nLines, nMaxCols, nBCol );

    SwHoriOrient eHor = pTab->GetFrmFmt()->GetHoriOrient().GetHoriOrient();
    SwTwips* pCellPos = new SwTwips[nMaxCols+1];

    SwPaM* pOldPam = pCurPam;
    SwPaM* pOldEndPam = pOrigPam;
    BOOL bOldWriteAll = bWriteAll;

    if ( GetFilter() == 44 || GetFilter() == 49 )
        OutW4W_SwTableWW2( pTab, nLines, nMaxCols, nBCol, eHor, pCellPos );
    else
        OutW4W_SwTableNorm( pTab, nLines, nMaxCols, nBCol, eHor, pCellPos );

    bWriteAll = bOldWriteAll;
    pCurPam = pOldPam;
    pOrigPam = pOldEndPam;

    pCurPam->GetPoint()->nNode = *pTableNd->EndOfSectionNode();
    DELETEZ( pCellPos );
}

//-------------------------------------------------------------------

SwW4WWriter::SwW4WWriter( const String& rFltName )
{
    bFirstLine = TRUE;
    nFontHeight = 0;
    pFlyFmt = 0;
    pStyleEndStrm = pPostNlStrm = 0;
    bFirstLine = bStyleDef = bStyleOnOff = bTable = bPageDesc = bEvenPage
        = bLastPg = bHdFt = bFly = bTxtAttr = FALSE;
    nPgLeft = 0;
    nPgRight = lA4Width;
    nIniFlags = 0;
    pNdFmt = 0;
    bStorageFlag = FALSE;

    String sFilter( rFltName.Copy( 3 ));
    xub_StrLen nFndPos = sFilter.Search( '_' );
    sVersion = sFilter.Copy( nFndPos + 1 );
    sFilter.Erase( nFndPos + 1 );
    if( 0 != ( bStorageFlag = 'C' == sFilter.GetChar(0)))
        sFilter.Erase( 0, 1 );

    nFilter = sFilter.ToInt32();
}


SwW4WWriter::~SwW4WWriter() {}

ULONG SwW4WWriter::WriteStream()
{
    {
        USHORT nFltNo = GetFilter();
        const sal_Char* aNames[ 2 ] = { "W4W/W4W000", "W4W/W4W000" };
        sal_uInt32 aVal[ 2 ];

        SwFilterOptions aOpt;

        // set into the first name the filter number
        sal_Char* pStr = (sal_Char*)aNames[0];
        pStr += 7;
        *pStr++ = '0' + ( nFltNo / 100 ); nFltNo %= 100;
        *pStr++ = '0' + ( nFltNo / 10  ); nFltNo %=  10;
        *pStr++ = '0' +   nFltNo;

        // first test if the node & content exist. If not, then we may not
        // ask the configuration (produce assertions!)
        pStr = (sal_Char*)aNames[0];
        pStr[ 3 ] = 0;

        if( aOpt.CheckNodeContentExist( pStr, pStr + 4 ))
        {
            pStr[ 3 ] = '/';
            aOpt.GetValues( 2, aNames, aVal );
        }
        else
        {
            aOpt.GetValues( 1, aNames+1, aVal+1 );
            aVal[0] = 0;
        }

        nIniFlags = aVal[ 0 ];
        if( !nIniFlags )
            nIniFlags = aVal[ 1 ];
    }

    bStyleDef = bStyleOnOff = bTable = bPageDesc = bEvenPage = bLastPg
        = bHdFt = bIsTxtInPgDesc = bFly = FALSE;
    nFlyWidth = nFlyHeight = 0;
    nFlyLeft = nFlyRight = 0;
        // Hier wird festgelegt, welches Ziel welche Buchstabencodierung
        // per XCS vorgeworfen bekommt. HEX und UCS benutzen immer den
        // IBMPC-Code. Wenn es nicht stimmt, klappt der
        // Re-Import der Umlaute nicht. Es koennte auch etwas
        // schlimmeres passieren, wenn der Export-Filter zu bloed ist.
        //
        // Im Moment wird nur fuer WW1/2/6 ANSI angenommen, sonst IBMPC.
        // Wahrscheinlich erwarten alle Win-Ziele oder vielleicht auch
        // alle Ziele Ansi, das konnte ich aber noch nicht ausprobieren.
        // Dasselbe wird fuer die Styles benutzt, allerdings ohne
        // XCS, sondern als direktes Zeichen, da ich vermute, dass es dann
        // besser durch die Filter geht.
    switch ( nFilter )
    {
    case 44 :
    case 49 : eTargCharSet = RTL_TEXTENCODING_MS_1252; break;
    default : eTargCharSet = RTL_TEXTENCODING_IBM_850; break;
    }
    nFontId = 0;

    nMaxNode = pDoc->GetNodes().Count();

    pFlyFmt = 0;		// kein FlyFrmFormat gesetzt
    pStyleEndStrm = pPostNlStrm = 0;  // kein StyleEndStream gesetzt

    pNxtPg = new SwPosition( *pOrigPam->GetMark() );

//		ValidateLayout();	// Nach ValidateLayout() aus pageiter.cxx
                        // sind alle Umbrueche richtig

    pIter = new SwPageIter( *pDoc, *pOrigPam->GetMark() );

    ASSERT( pIter->GetPageDesc(), "1. Seite: PageDesc weg");
    pPgDsc2 = pPageDesc = pIter->GetPageDesc();
    pPgFrm = (SwFrmFmt*)&(pPgDsc2->GetMaster());
    pIter->NextPage();
    bLastPg = !( pIter->GetPosition( *pNxtPg ));

    // Tabelle fuer die freifliegenden Rahmen erzeugen, aber nur wenn
    // das gesamte Dokument geschrieben wird
    nAktFlyPos = 0;
    BYTE nSz = (BYTE)Min( pDoc->GetSpzFrmFmts()->Count(), USHORT(255) );
    SwPosFlyFrms aFlyPos( nSz, nSz );
    pDoc->GetAllFlyFmts( aFlyPos, bWriteAll ? 0 : pOrigPam );
    pFlyPos = &aFlyPos;
    nDefLanguage = ((const SvxLanguageItem&)pDoc
        ->GetDefault( RES_CHRATR_LANGUAGE )).GetLanguage();

    // schreibe die StyleTabelle, allg. Angaben,Header/Footer/Footnotes
    MakeHeader();

    // kein PageDesc im 1. Absatz vorhanden ?
    SfxItemSet* pSet = pCurPam->GetCntntNode(FALSE)->GetpSwAttrSet();
    if( !pSet || SFX_ITEM_SET != pSet->GetItemState( RES_PAGEDESC, FALSE ))
    {
//			Sound::Beep();			   // dann gebe den Default-Pagedesc aus
        OutW4W_SwFmtPageDesc1( *this, pPgDsc2 );
    }

    // Tabelle fuer die freifliegenden Rahmen erzeugen, aber nur wenn
    // das gesamte Dokument geschrieben wird
    nAktFlyPos = 0;

    Out_SwDoc( pOrigPam, TRUE );

    DELETEZ( pIter );
    DELETEZ( pNxtPg );

    // loesche die Tabelle mit den freifliegenden Rahmen
    for( USHORT i = aFlyPos.Count(); i > 0; )
        delete aFlyPos[ --i ];
    pFlyPos = 0;

    return 0;
}

void SwW4WWriter::Out_SwDoc( SwPaM* pPam, BOOL bAttrOn )
{
    BOOL bSaveWriteAll = bWriteAll; 	// sichern


    // gebe alle Bereiche des Pams in das W4W-File aus.
    do {
        // Wenn der Pam innerhalb einer Tabelle steht, ruecke an den
        // Tabellenanfang zurueck, damit Tabellen am Anfang eines Docs oder
        // eines Flys mit uebernommen werden
        if ( !bTable ){
            const SwNode* pNode = pDoc->GetNodes()[pCurPam->GetPoint()->nNode];
            const SwTableNode* pTblNd = pNode->FindTableNode();
            if ( pTblNd ){
                pCurPam->GetPoint()->nNode = *pTblNd;	// TableNode ist StartNode
                // TableNode ist kein ContentNode, deshalb darf kein Assign stattfinden
            }
        }

        bWriteAll = bSaveWriteAll;
        bFirstLine = TRUE;

        // setze die dflt. Formate mit 0, damit auf jedenfall am Anfang
        // eines jeden Cursors die Format-Attribute ausgegeben werden.

        while( pCurPam->GetPoint()->nNode < pCurPam->GetMark()->nNode ||
              (pCurPam->GetPoint()->nNode == pCurPam->GetMark()->nNode &&
               pCurPam->GetPoint()->nContent <= pCurPam->GetMark()->nContent ) )
        {
            SwNode * pNd =
                pDoc->GetNodes()[ pCurPam->GetPoint()->nNode ];
            SwCntntNode * pCtNd = pNd->GetCntntNode();

            if( pCtNd ) 					// Normaler ContentNode
            {
                if( !bFirstLine )
                    pCurPam->GetPoint()->nContent.Assign( pCtNd, 0 );
                Out( aW4WNodeFnTab, *pCtNd, *this );
                OutFlyFrm();
            }
            else if( pNd->IsTableNode() )	// TableNode
                OutW4W_SwTable( pNd->GetTableNode() );

            ULONG nPos = pCurPam->GetPoint()->nNode++; 	// Bewegen

            ::binfilter::SetProgressState( nPos * 67L / nMaxNode, pDoc->GetDocShell() );   // Wie weit ?

            /* sollen nur die Selektierten Bereiche gesichert werden, so
             * duerfen nur die vollstaendigen Nodes gespeichert werde,
             * d.H. der 1. und n. Node teilweise, der 2. bis n-1. Node
             * vollstaendig. (vollstaendig heisst mit allen Formaten! )
             */
            bWriteAll = bSaveWriteAll ||
                    pCurPam->GetPoint()->nNode != pCurPam->GetMark()->nNode;
            bFirstLine = FALSE;
        }
    } while( CopyNextPam( &pPam ) );		// bis alle PaM's bearbeitet

    bWriteAll = bSaveWriteAll;			// wieder auf alten Wert zurueck
}


// schreibe die StyleTabelle, algemeine Angaben,Header/Footer/Footnotes


void SwW4WWriter::MakeHeader()
{

    Strm() << sW4W_RECBEGIN << "DID0" << sW4W_TERMEND;
    OutW4WFontTab();		// baue die FontTbl

    // die globalen Daten nicht speichern, wenn nur teilweise ausgegeben wird
    if( !bWriteAll )
        return;

    // gebe die Groesse und die Raender der Seite aus
    if( pDoc->GetPageDescCnt() )
    {
//		SwFrmFmt &rFmtPage = (SwFrmFmt&)pDoc->GetPageDesc(0).GetMaster();
        SwFrmFmt &rFmtPage = (SwFrmFmt&)*pPgFrm;
        const SwFmtFrmSize& rSz = rFmtPage.GetFrmSize();
        const SvxLRSpaceItem& rLR = rFmtPage.GetLRSpace();
        const SvxULSpaceItem& rUL = rFmtPage.GetULSpace();

        long nTxtLen = rSz.GetHeight() - rUL.GetUpper() - rUL.GetLower();
        Strm() << sW4W_RECBEGIN << "SFL0" << cW4W_TXTERM << '0' << cW4W_TXTERM;

        OutLong( rSz.GetHeight() / 240 ) << cW4W_TXTERM;
        OutLong( nTxtLen / 240 ) << cW4W_TXTERM;
        OutLong( rSz.GetHeight() ) << cW4W_TXTERM;
        OutLong( nTxtLen ) << sW4W_TERMEND;

        Strm() << sW4W_RECBEGIN << "PGW";
        OutLong( rSz.GetWidth() / 144) << cW4W_TXTERM;
        OutLong( rSz.GetWidth() ) << sW4W_TERMEND;

        nPgLeft = (USHORT)rLR.GetLeft();
        nPgRight = (USHORT)(rSz.GetWidth() - rLR.GetRight());

        Strm() << sW4W_RECBEGIN << "RSM0" << cW4W_TXTERM << '0' << cW4W_TXTERM;
        OutLong( rLR.GetLeft() / 144 ) << cW4W_TXTERM;
        OutLong( rSz.GetWidth() - rLR.GetRight() / 144 ) << cW4W_TXTERM;
        OutLong( nPgLeft ) << cW4W_TXTERM;
        OutLong( nPgRight ) << sW4W_TERMEND;

        Strm() << sW4W_RECBEGIN << "STP0" << cW4W_TXTERM;
        OutLong( rUL.GetUpper() / 240 ) << cW4W_TXTERM;
        OutLong( rUL.GetUpper() ) << sW4W_TERMEND;

        Strm() << sW4W_RECBEGIN << "SBP";
        OutLong( rUL.GetLower() / 240 ) << cW4W_TXTERM;
        OutLong( rUL.GetLower() )<< sW4W_TERMEND;

        Strm() << sW4W_RECBEGIN << "SLG";
        OutLong( nDefLanguage ) << sW4W_TERMEND;
    }

    OutW4WStyleTab();		// Besser hier
}

void SwW4WWriter::OutW4WFontTab()
{
    USHORT n = 0, nGet = 0;
    const SfxItemPool& rPool = pDoc->GetAttrPool();

    const SvxFontItem *pDfltFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                                    RES_CHRATR_FONT );
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );

    // Zaehle die Anzahl
    USHORT nMaxItem = rPool.GetItemCount( RES_CHRATR_FONT );
    for( nGet = 0; nGet < nMaxItem; ++nGet )
        if( rPool.GetItem( RES_CHRATR_FONT, nGet ) )
            n++;

    if( pDfltFont )
        ++n;

    // der default Font steht auf der Position 0 in der Tabelle !!
    // (eigentlich reichen 2 Eintraege, aber nur mit 3 funktionierts)
    Strm() << sW4W_RECBEGIN << "FDT";
    OutLong( n ) << cW4W_TXTERM << '3' << cW4W_TXTERM << "00" << cW4W_TXTERM;

    n = 0;
    OutLong( n++ ) << cW4W_TXTERM;
    OutW4W_String( *this, pFont->GetFamilyName() ).Strm()
            << cW4W_TXTERM << "00" << cW4W_TXTERM;
    if( pDfltFont )
    {
        OutLong( n++ ) << cW4W_TXTERM;
        OutW4W_String( *this, pFont->GetFamilyName() ).Strm()
            << cW4W_TXTERM << "00" << cW4W_TXTERM;
    }

    for( nGet = 0; nGet < nMaxItem; ++nGet )
        if( rPool.GetItem( RES_CHRATR_FONT, nGet ) )
        {
            OutLong( n++ ) << cW4W_TXTERM;
            OutW4W_String( *this, pFont->GetFamilyName() ).Strm()
                << cW4W_TXTERM << "00" << cW4W_TXTERM;
        }
    Strm() << cW4W_RED;
}



void SwW4WWriter::OutW4WStyleTab()
{
    const SvPtrarr & rArr = *pDoc->GetTxtFmtColls();
    // das 0-Style ist das Default, wird nie ausgegeben !!
    if( rArr.Count() <= 1 )
        return;

    // das Default-TextStyle wird nicht mit ausgegeben !!
    nFontId = 0;
    for( USHORT n = 1; n < rArr.Count(); n++ )
    {
        SwTxtFmtColl * pColl = (SwTxtFmtColl*)rArr[ n ];

        Strm() << sW4W_RECBEGIN << "SYT";
        OutW4W_String( *this, pColl->GetName() ).Strm() << cW4W_TXTERM;
        OutLong( n ) << cW4W_TXTERM;

        if ( !pColl->IsDefault() ) 				// Abgeleitet von ?
        {
            SwFmt* pC = pColl->DerivedFrom();
            void* pN = pC;
            USHORT nSty = rArr.GetPos( pN );   // Index der Ableitung
            if ( nSty>0 && nSty < USHRT_MAX )		// wirklich sinnvoller Index
            {										// und nicht "(keins)" ?
                Strm() << sW4W_RECBEGIN << "SBO";
                OutW4W_String( *this, pC->GetName()).Strm() << cW4W_TXTERM;
                OutLong( nSty ) << sW4W_TERMEND;
            }
        }

        BOOL bOldStyle = bStyleDef; bStyleDef = TRUE;
        pStyleEndStrm = new SvMemoryStream;
        OutW4W_SwFmt( *this, *pColl );

        pStyleEndStrm->Seek( 0L );
        Strm() << sW4W_RECBEGIN << "SEP" << cW4W_RED << *pStyleEndStrm;
        delete pStyleEndStrm;
        pStyleEndStrm = 0;

        bStyleDef = bOldStyle;

        Strm() << cW4W_RED;
    }
}



// Bei Fonts werden ihre Inhalte verglichen

const SvxFontItem& SwW4WWriter::GetFont( USHORT nId ) const
{
    if( !nId )
        return *(const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );

    USHORT n = 1;
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                                        RES_CHRATR_FONT );
    if( pFont && n++ == nId )
        return *pFont;

    USHORT nMaxItem = rPool.GetItemCount( RES_CHRATR_FONT );
    for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
        if( 0 != (pFont = (const SvxFontItem*) rPool.GetItem(
            RES_CHRATR_FONT, nGet ) ) && n++ == nId )
            break;
//	pFont = (const SvxFontItem*)rPool.GetItem( RES_CHRATR_FONT, nId - n );
    ASSERT( pFont, "falsche Font-Id" );
    return *pFont;
}

USHORT SwW4WWriter::GetId( const SvxFontItem& rFont )
{
    const SfxItemPool& rPool = pDoc->GetAttrPool();
    const SvxFontItem* pFont = (const SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT );
    if( rFont == *pFont )
        return 0;

    USHORT n = 1;
    if( 0 != ( pFont = (const SvxFontItem*)rPool.GetPoolDefaultItem(
                                                        RES_CHRATR_FONT )))
    {
        if( rFont == *pFont )
            return 1;
        ++n;
    }

    USHORT nMaxItem = rPool.GetItemCount( RES_CHRATR_FONT );
    for( USHORT nGet = 0; nGet < nMaxItem; ++nGet )
        if( 0 != (pFont = (const SvxFontItem*)rPool.GetItem(
            RES_CHRATR_FONT, nGet )) )
        {
            if( rFont == *pFont )
                return n;
            ++n;
        }

    ASSERT( !this, "Font nicht in der Tabelle" );
    return 0;
}

USHORT SwW4WWriter::GetId( const SwTxtFmtColl& rColl )
{
    // suche das angegebene Format
    const SvPtrarr & rArr = *pDoc->GetTxtFmtColls();
    for( USHORT n = 0; n < rArr.Count(); n++ )
        if( (SwTxtFmtColl*)rArr[ n ] == &rColl )
            return n;
    ASSERT( !this, "TextCollection nicht in der Tabelle" );
    return 0;
}

void SwW4WWriter::OutFlyFrm()
{
    if( !pFlyPos )
        return;

    // gebe alle freifliegenden Rahmen die sich auf den akt. Absatz
    // und evt. auf das aktuelle Zeichen beziehen, aus.

    // suche nach dem Anfang der FlyFrames
    USHORT n;
    for( n = 0; n < pFlyPos->Count() &&
            (*pFlyPos)[n]->GetNdIndex().GetIndex() <
                pCurPam->GetPoint()->nNode.GetIndex(); ++n )
        ;
    if( n < pFlyPos->Count() )
        while( n < pFlyPos->Count() &&
                pCurPam->GetPoint()->nNode.GetIndex() ==
                    (*pFlyPos)[n]->GetNdIndex().GetIndex() )
        {
            // den Array-Iterator weiterschalten, damit nicht doppelt
            // ausgegeben wird !!
            OutW4WFlyFrms( (const SwFlyFrmFmt&)(*pFlyPos)[n++]->GetFmt() );
        }
}



void SwW4WWriter::OutW4WFlyFrms( const SwFlyFrmFmt& rFlyFrmFmt )
{
    // ein FlyFrame wurde erkannt, gebe erstmal den aus

    // Hole vom Node und vom letzten Node die Position in der Section
    const SwFmtCntnt& rFlyCntnt = rFlyFrmFmt.GetCntnt();
    if( !rFlyCntnt.GetCntntIdx() )
        return;

    ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
    ULONG nEnd = rFlyCntnt.GetCntntIdx()->GetNode().EndOfSectionIndex();

    if( nStt >= nEnd )		// kein Bereich, also kein gueltiger Node
        return;


#if OSL_DEBUG_LEVEL > 1
    Strm() << sW4W_RECBEGIN << "NOP_FlyFrame" << sW4W_TERMEND;
//	  Sound::Beep();
#endif

    pFlyFmt = (SwFlyFrmFmt*)&rFlyFrmFmt;

    USHORT nAType = 1;
    const SwFmtAnchor& rA = pFlyFmt->GetAnchor();  //!!!
    switch ( rA.GetAnchorId() )
    {
    case FLY_PAGE:		nAType = 1; break;
    case FLY_AT_CNTNT:	nAType = 0; break;
    case FLY_IN_CNTNT:	nAType = 2; break;
    default:				ASSERT( !this, "+ Watn dette fuer'n FlyFrame");
    }

    USHORT nHAlign = 0;
    const SwFmtHoriOrient& rH = pFlyFmt->GetHoriOrient();
    switch ( rH.GetHoriOrient() )
    {
    case HORI_NONE: 	nHAlign = 0; break;
    case HORI_RIGHT:	nHAlign = 1; break;
    case HORI_CENTER:	nHAlign = 2; break;
    case HORI_LEFT: 	nHAlign = 0; break;
                                        // 3 = Full Just
    }
    SwTwips nXPos = rH.GetPos();	//!!! Relation & Anchor ??

    USHORT nVAlign = 1;
    const SwFmtVertOrient& rV = pFlyFmt->GetVertOrient();
    switch ( rV.GetVertOrient() )
    {
    case VERT_NONE: 		nVAlign = 4; break;
    case VERT_TOP:
    case VERT_LINE_TOP:		nVAlign = 1; break;
    case VERT_CENTER:
    case VERT_LINE_CENTER:	nVAlign = 2; break;
    case VERT_BOTTOM:
    case VERT_LINE_BOTTOM:	nVAlign = 3; break;
    }
    SwTwips nYPos = rV.GetPos();
    USHORT nAlignW = 2; 	// Default: absolute ( x und y )

    USHORT nWrap = 0;
    if( SURROUND_NONE != pFlyFmt->GetSurround().GetSurround() )
        ++nWrap;

    SwTwips nTpSpace = 0, nLeSpace = 0, nBoSpace = 0, nRiSpace = 0;

    const SwFmtFrmSize& rS = pFlyFmt->GetFrmSize();
    SwTwips nXSize = rS.GetWidth();
    SwTwips nYSize = rS.GetHeight();
    nFlyWidth = nXSize; 			// Fuer Anpassung Graphic-Groesse
    nFlyHeight = nYSize;
    nFlyLeft = nXPos;				// Fuer Anpassung Raender
    nFlyRight = nYPos;				// ( Absolute Positionen )

                            // Ist der Fly relativ zu Seitenraendern ?
    if ( nAType == 1								// RND_STD_PAGE ?
         && rH.GetRelationOrient() != PRTAREA	// x und y relativ zur
         && rV.GetRelationOrient() != PRTAREA	// Printarea ?
                 && nXPos >= (SwTwips)pPgFrm->GetLRSpace().GetLeft()	 // und keine daraus resul-
                 && nYPos >= (SwTwips)pPgFrm->GetULSpace().GetUpper())//tierenden negativen
    {												// Koordinaten ?

        nXPos -= pPgFrm->GetLRSpace().GetLeft();	// dann umrechnen auf rela-
        nYPos -= pPgFrm->GetULSpace().GetUpper();	// tive Koordinaten
        nAlignW = 0;					// Wert auf "Align with Margin" setzen
    }

    Strm() << sW4W_RECBEGIN << "APO00" << cW4W_TXTERM;
    OutLong( nAType ) << cW4W_TXTERM;
    OutLong( nVAlign ) << cW4W_TXTERM;
    OutLong( nHAlign ) << cW4W_TXTERM;
    OutLong( nAlignW ) << cW4W_TXTERM;
    OutLong( nWrap ) << cW4W_TXTERM;
    OutLong( nXSize ) << cW4W_TXTERM;
    OutLong( nYSize ) << cW4W_TXTERM;
    OutULong( nXPos ) << cW4W_TXTERM;
    OutULong( nYPos ) << cW4W_TXTERM;
    OutLong( nTpSpace ) << cW4W_TXTERM;
    OutLong( nLeSpace ) << cW4W_TXTERM;
    OutLong( nBoSpace ) << cW4W_TXTERM;
    OutLong( nRiSpace ) << cW4W_TXTERM << "0000" << cW4W_TXTERM
                << '0' << cW4W_TXTERM    // shading / VPos from frame ???
                << sW4W_TERMEND;		   // FileName

    {
        W4WSaveData aSaveData( *this, nStt, nEnd );
        BOOL bOldFly = bFly;
        bFly = TRUE;

        Out_SwDoc( pCurPam, TRUE ); //!!! Keine ueberfluessigen Attrs !?!

        bFly = bOldFly;

    }	// SaveData D'tor

    Strm() << sW4W_RECBEGIN << "APF" << cW4W_RED;

#if OSL_DEBUG_LEVEL > 1
    Strm() << sW4W_RECBEGIN << "NOP_End_FlyFrame" << sW4W_TERMEND;
#endif

    nFlyWidth = nFlyHeight = 0; 	// APO zuende
    nFlyLeft = nFlyRight = 0;		// APO zuende

}

#ifdef NIE


void SwW4WWriter::OutW4WFlyFrms( const SwFlyFrmFmt& rFlyFrmFmt )
{
    // ein FlyFrame wurde erkannt, gebe erstmal den aus

    // Hole vom Node und vom letzten Node die Position in der Section
    const SwFmtCntnt* pFlyCntnt = (const SwFmtCntnt*)
                                rFlyFrmFmt.GetAttr( RES_CNTNT );
    if( !pFlyCntnt )
        return;

    ULONG nStt = pFlyCntnt->GetCntntIdx()->GetIndex()+1;
    ULONG nEnd = pFlyCntnt->GetCntntIdx()->GetNode().EndOfSectionIndex();

    if( nStt >= nEnd )		// kein Bereich, also kein gueltiger Node
        return;

    SwPaM *pOldPam = pCurPam, *pOldEnd = pOrigPam;
    BOOL bSaveWriteAll = bWriteAll;
    SwFlyFrmFmt* pOldFlyFmt = pFlyFmt;

    pOrigPam = pCurPam = NewSwPaM( *pDoc, nStt, nEnd );
    pCurPam->Exchange();
    bWriteAll = TRUE;

    pFlyFmt = (SwFlyFrmFmt*)&rFlyFrmFmt;
    Out_SwDoc( pCurPam, TRUE );

    delete pCurPam; 				// Pam wieder loeschen

    pCurPam = pOldPam;
    pOrigPam = pOldEnd;
    bWriteAll = bSaveWriteAll;
    pFlyFmt = pOldFlyFmt;
}
#endif //NIE

// ermmittle  die Distanz zum linken+rechten Rand
// D.H es werden die akt. Raender vom Frame-Format und von der Seite
// addiert. (das kommt dann so "ungefaehr" hin ). Der negative
// Erstzeileneinzug wird beim linken Rand beachtet.
// Wenn es innerhalb von FlyFrames 'rundgeht, werden keine absoluten
// Werte geliefert, sondern statt den Frame-Raendern die Seitenraender
// addiert / subtrahiert. Das ist aber gut so, da WW2 es genau so erwartet.

void SwW4WWriter::GetMargins( SwTwips& rLeft, SwTwips& rWidth )
{
    ASSERT( pNdFmt, "Attribut LRSpace vom Node nicht gesetzt!" );
    if( !pNdFmt )
        return;

    // gebe die Groesse und die Raender der Seite aus
    ASSERT( pDoc->GetPageDescCnt(), "kein Seiten-Format definiert??" );
//	SwFrmFmt &rFmtPage = (SwFrmFmt&)pDoc->GetPageDesc(0).GetMaster();
    SwFrmFmt &rFmtPage = (SwFrmFmt&)*pPgFrm;
    SwTwips nWidth = rFmtPage.GetFrmSize().GetWidth();
    const SvxLRSpaceItem& rPageLR = rFmtPage.GetLRSpace();
    const SvxLRSpaceItem& rNdLR = pNdFmt->GetLRSpace();

    rLeft = rPageLR.GetLeft() + rNdLR.GetTxtLeft();
    rWidth = nWidth - rLeft - rPageLR.GetRight() - rNdLR.GetRight();
}


    // gebe die TextCollection am TextNode aus. Die Attribute
    // werden im Style an- und abgeschaltet. Das Abschalten wird im
    // extra Stream gespeichert !!
void SwW4WWriter::Out_SwTxtColl_OnOff( const SwTxtFmtColl& rTxtColl,
                                        const SwAttrSet* pAutoSet, BOOL bOn )
{
    if( !bOn )
    {
        ASSERT( pStyleEndStrm, "Style Ende ohne StyleEndStream??"  );
        pStyleEndStrm->Seek( 0L );
        *pStrm << *pStyleEndStrm;
        delete pStyleEndStrm;
        pStyleEndStrm = 0;
        return;
    }

    USHORT nId = GetId( rTxtColl );

    // Anfang vom Style, sprich alle Attribute einschalten
    // und den StyleEnd-Stream zuruecksetzen
    if( pStyleEndStrm )
        delete pStyleEndStrm;
    pStyleEndStrm = new SvMemoryStream;

    if( nId )
    {
        BOOL bOldStyle = bStyleOnOff; bStyleOnOff = TRUE;

        // erstmal den StyleEnd-Stream vorbereiten:
        {
            SvStream* pOld = pStrm;
            pStrm = pStyleEndStrm;			// fuer den String diesen Stream
            Strm() << sW4W_RECBEGIN << "STF";         // als akt Ausgabe
            OutW4W_String( *this, rTxtColl.GetName() ).Strm() << cW4W_TXTERM;
            OutULong( nId ) << sW4W_TERMEND;
            pStrm = pOld;
        }
        Strm() << sW4W_RECBEGIN << "STY";
        OutW4W_String( *this, rTxtColl.GetName()).Strm() << cW4W_TXTERM;
        OutULong( nId ) << sW4W_TERMEND;

        OutW4W_SwFmt( *this, rTxtColl );
        Strm() << sW4W_RECBEGIN << "STE" << cW4W_RED;
        GetStrm(TRUE) << sW4W_RECBEGIN << "STE" << cW4W_RED;
        bStyleOnOff = bOldStyle;
    }

    // gilt am Node harte Attributierung ?
    if( pAutoSet )
        Out_SfxItemSet( aW4WAttrFnTab, *this, *pAutoSet, FALSE );	// ausgeben
}


    // nicht Inline, um Pointer zu pruefen
SvStream& SwW4WWriter::GetStrm( BOOL bStyleStream ) //$ ostream
{
    if( !bStyleStream )
        return *pStrm;

    ASSERT( pStyleEndStrm, "kein StyleStream vorhanden !!" );
    return *pStyleEndStrm;
}

SvStream& SwW4WWriter::GetNlStrm() //$ ostream
{
    if ( !pPostNlStrm )
        pPostNlStrm = new SvMemoryStream;
    return *pPostNlStrm;
}


// Struktur speichert die aktuellen Daten des Writers zwischen, um


// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
W4WSaveData::W4WSaveData( SwW4WWriter& rWriter, ULONG nStt, ULONG nEnd )
    : rWrt( rWriter ), bOldWriteAll( rWrt.bWriteAll ),
    pOldPam( rWrt.pCurPam ), pOldEnd( rWrt.GetEndPaM() ),
    pOldFlyFmt( rWrt.pFlyFmt ), pOldPageDesc( rWrt.pPageDesc ),
    pOldStyleEndStrm( rWrt.pStyleEndStrm ), pOldNdFmt( rWrt.pNdFmt )
{
    rWrt.pCurPam = rWrt.NewSwPaM( *rWrt.pDoc, nStt, nEnd );
    rWrt.SetEndPaM( rWrt.pCurPam );
    rWrt.pCurPam->Exchange( );
    rWrt.bWriteAll = TRUE;

    rWrt.pStyleEndStrm = 0;
}


W4WSaveData::~W4WSaveData()
{
    delete rWrt.pCurPam;					// Pam wieder loeschen

    rWrt.pCurPam = pOldPam;
    rWrt.SetEndPaM( pOldEnd );
    rWrt.bWriteAll = bOldWriteAll;
    rWrt.pFlyFmt = pOldFlyFmt;
    rWrt.pPageDesc = pOldPageDesc;

    rWrt.pStyleEndStrm = pOldStyleEndStrm;
    rWrt.pNdFmt = pOldNdFmt;
}


}
