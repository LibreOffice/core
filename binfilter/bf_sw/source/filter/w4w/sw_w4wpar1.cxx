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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <hintids.hxx>

#include <bf_svtools/zforlist.hxx>

#include <bf_svx/paperinf.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_sfx2/docfile.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/pbinitem.hxx>
#include <bf_svx/brkitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/wrlmitem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/escpitem.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/lspcitem.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/cmapitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/cscoitem.hxx>
#include <bf_svx/tstpitem.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/adjitem.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <bf_svx/unolingu.hxx>

#include <fmthbsh.hxx>
#include <fmthdft.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <fmtpdsc.hxx>
#include <frmatr.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <fmtftn.hxx>
#include <txtftn.hxx>
#include <pam.hxx>          // fuer SwPam
#include <doc.hxx>
#include <shellio.hxx>      // MAX_ASCII_PARA
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <paratr.hxx>
#include <fmtcol.hxx>
#include <pagedesc.hxx>
#include <docufld.hxx>
#include <poolfmt.hxx>      // RES_POOLCOLL_STANDARD
#include <ndgrf.hxx>
#include <w4wstk.hxx>          // fuer den Attribut Stack
#include <w4wpar.hxx>
#include <w4wflt.hxx>           // fuer den W4W Parser
#include <fltini.hxx>
#include <swfltopt.hxx>
#include <flddat.hxx>			// class SwDateTimeField
#include <swunohelper.hxx>

#include <sal/macros.h>

#include <swerror.h>
#include <mdiexp.hxx>          // ...SetPercent()
#include <statstr.hrc>          // ResId fuer Statusleiste
namespace binfilter {

#define HOR_SICHER 100		// Horizontale Bereiche werden mindestens so
                            // gross gemacht ( z.B. Tab-Boxengroesse - LRSpace )

#define MOGEL_WW6		// Bei WW6 werden PGW und SFL ignoriert, da falsch


#define CHARBUF_SIZE (SAL_N_ELEMENTS( aCharBuffer ))

// #define bInsertMode FALSE


inline const SwNodeIndex & PtNd( const SwPaM * pPam )
    { return pPam->GetPoint()->nNode; }
inline SwNodeIndex & PtNd( SwPaM* pPam )
    { return pPam->GetPoint()->nNode; }
inline const SwIndex & PtCnt( const SwPaM * pPam )
    { return pPam->GetPoint()->nContent; }
inline SwIndex & PtCnt( SwPaM* pPam )
    { return pPam->GetPoint()->nContent; }

// ------------- allg. Funktionen --------------------

// Ermittle die Anzahl der Bits der Varaiablen "n".
// (Algorithmus kommt aus einer MAIL)

static USHORT GetBitCnt( register ULONG n )
{
    n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n + (n >> 4)) & 0x0f0f0f0f;
    n += n >> 8;
    n += n >> 16;
    return ( (USHORT)n & 0xff);
}


/***************************************************************************
#* Prototypes
#**************************************************************************/

extern "C" {
#if defined(PM2) && defined(ZTC)
    #define COMPRECORD_MODIFIER __CLIB
#elif defined( WNT ) && defined( _CRTAPI1 )
// nur beim MSC von den NT Disks ist _CRTAPI1 definiert, nicht bei VC++
    #define COMPRECORD_MODIFIER _CRTAPI1
#elif defined( WNT )
    #define COMPRECORD_MODIFIER __cdecl
#else
    #define COMPRECORD_MODIFIER
#endif


    int COMPRECORD_MODIFIER CompRecord( const void *pFirst, const void *pSecond)
    {
        return strncmp( (char*) &(((W4WRecord*)pSecond)->aRecType.c0) ,
                        (char*) &(((W4WRecord*)pFirst)->aRecType.c0), 3 );
    }
    // die Routine ist absichtlich "falsch" herum ( 'A' > 'Z' ) definiert,
    // da sonst wg. Vorordnung der Quicksort laenger braucht

    #undef COMPRECORD_MODIFIER
}

static BOOL bSortRecordTab = FALSE;
static W4WRecord aRecordTab[] = {
    'A', 'F', 'R',  &SwW4WParser::Read_BeginFlushRight,
    'A', 'P', 'F',  &SwW4WParser::Read_EndAbsPosObj,
    'A', 'P', 'O',  &SwW4WParser::Read_BeginAbsPosObj,
    'B', 'B', 'T',  &SwW4WParser::Read_BeginBold,
    'B', 'C', 'L',  &SwW4WParser::Read_BeginColoredText,
    'B', 'C', 'M',  &SwW4WParser::Read_BeginColumnMode,
    'B', 'C', 'O',  &SwW4WParser::Read_BeginTabCell,
    'B', 'C', 'S',  &SwW4WParser::Read_BeginSmallCaps,
    'B', 'C', 'U',  &SwW4WParser::Read_BeginUpperCaps,
    'B', 'D', 'U',  &SwW4WParser::Read_BeginDoubleUnderline,
    'B', 'H', 'P',  &SwW4WParser::Read_BeginHyphen,
    'B', 'H', 'T',  &SwW4WParser::Read_BeginHiddenText,
    'B', 'M', 'T',  &SwW4WParser::Read_BeginMarkedText,
    'B', 'R', 'J',  &SwW4WParser::Read_BeginRightJustify,
    'B', 'R', 'O',  &SwW4WParser::Read_BeginTabRow,
    'B', 'S', 'O',  &SwW4WParser::Read_BeginStrikeOut,
    'B', 'S', 'P',  &SwW4WParser::Read_BeginShadow,

    'B', 'T', 'F',  &SwW4WParser::Read_BeginTextFlow,			// FrameMaker

    'B', 'U', 'L',  &SwW4WParser::Read_BeginUnderline,
    'C', 'D', 'S',  &SwW4WParser::Read_ColumnsDefinition,
    'C', 'M', 'T',  &SwW4WParser::Read_CommentLine,
    'C', 'T', 'X',  &SwW4WParser::Read_BeginCenterText,
    'D', 'I', 'D',  &SwW4WParser::Read_DefineDocumentType,
    'D', 'T', 'B',  &SwW4WParser::Read_Tab,
    'D', 'T', 'F',  &SwW4WParser::Read_DateTime,
    'E', 'A', 'T',  &SwW4WParser::Read_EndFlushRight,
    'E', 'B', 'S',  &SwW4WParser::Read_EndSubScript,
    'E', 'B', 'T',  &SwW4WParser::Read_EndBold,
    'E', 'C', 'L',  &SwW4WParser::Read_EndColoredText,
    'E', 'C', 'M',  &SwW4WParser::Read_EndColumnMode,
    'E', 'C', 'S',  &SwW4WParser::Read_EndSmallCaps,
    'E', 'C', 'T',  &SwW4WParser::Read_EndCenterText,
    'E', 'C', 'U',  &SwW4WParser::Read_EndUpperCaps,
    'E', 'D', 'U',  &SwW4WParser::Read_EndDoubleUnderline,
    'E', 'F', 'N',  &SwW4WParser::Read_FootNoteEnd,
    'E', 'F', 'T',  &SwW4WParser::Read_FootNoteEnd,
    'E', 'H', 'P',  &SwW4WParser::Read_EndHyphen,
    'E', 'H', 'T',  &SwW4WParser::Read_EndHiddenText,
    'E', 'K', 'R',  &SwW4WParser::Read_EndKerning,
    'E', 'M', 'T',  &SwW4WParser::Read_EndMarkedText,
    'E', 'P', 'N',  &SwW4WParser::Read_EndOfParaNum,
    'E', 'P', 'S',  &SwW4WParser::Read_EndSuperScript,
    'E', 'R', 'J',  &SwW4WParser::Read_EndRightJustify,
    'E', 'S', 'O',  &SwW4WParser::Read_EndStrikeOut,
    'E', 'S', 'P',  &SwW4WParser::Read_EndShadow,

    'E', 'T', 'F',  &SwW4WParser::Read_EndTextFlow,				// FrameMaker

    'E', 'U', 'L',  &SwW4WParser::Read_EndUnderline,
    'F', 'B', 'M',  &SwW4WParser::Read_FooterBottomMargin,
//	'F', 'D', 'T',  &SwW4WParser::Read_FontDefTable,            // ueberfluessig

    'F', 'L', 'O',  &SwW4WParser::Read_Flow,					// FrameMaker

    'F', 'M', ' ',  &SwW4WParser::Read_FooterMargin,
    'F', 'N', 'I',  &SwW4WParser::Read_FootNoteInfo,
    'F', 'N', 'T',  &SwW4WParser::Read_FootNoteStart1,
    'F', 'T', 'N',  &SwW4WParser::Read_FootNoteStart2,
    'H', 'C', 'B',  &SwW4WParser::Read_ColumnBreak,
    'H', 'E', 'X',  &SwW4WParser::Read_Hex,
    'H', 'H', 'C',  &SwW4WParser::Read_HardHyphen,
    'H', 'H', 'L',  &SwW4WParser::Read_HardHyphen,
    'H', 'H', 'P',  &SwW4WParser::Read_HardHyphen,
    'H', 'F', '1',  &SwW4WParser::Read_FooterStart,
    'H', 'F', '2',  &SwW4WParser::Read_HeaderStart,
    'H', 'F', 'X',  &SwW4WParser::Read_HeadFootEnd,

    'H', 'M', ' ',  &SwW4WParser::Read_HeadingMargin,
    'H', 'T', 'M',  &SwW4WParser::Read_HeaderTopMargin,
    'H', 'N', 'L',  &SwW4WParser::Read_HardNewLine,
    'H', 'N', 'P',  &SwW4WParser::Read_HardNewPage,
    'H', 'S', 'P',  &SwW4WParser::Read_HardSpace,

    'I', 'G', 'R',  &SwW4WParser::Read_IncludeGraphic,
    'I', 'P', 'S',  &SwW4WParser::Read_IndentPara,
    'I', 'T', 'F',  &SwW4WParser::Read_EndItalic,
    'I', 'T', 'O',  &SwW4WParser::Read_BeginItalic,
    'K', 'E', 'P',  &SwW4WParser::Read_Split,
    'K', 'E', 'R',  &SwW4WParser::Read_BeginKerning,
    'L', 'M', 'R',  &SwW4WParser::Read_SetLeftMarginRelease,
    'N', 'B', 'R',  &SwW4WParser::Read_NonBreakSpace,
    'N', 'L', 'N',  &SwW4WParser::Read_NewLineInPara,
    'N', 'T', 'B',  &SwW4WParser::Read_NewTabTable,
    'P', 'B', 'C',  &SwW4WParser::Read_ParaBorder,
    'P', 'C', 'T',  &SwW4WParser::Read_Picture,
    'P', 'D', 'S',  &SwW4WParser::Read_PictureDef,

    'P', 'D', 'T',  &SwW4WParser::Read_PageDefinitionTable,		// FrameMaker

    'P', 'G', 'N',  &SwW4WParser::Read_ParagraphNumber,
    'P', 'G', 'W',  &SwW4WParser::Read_PageWidth,
    'P', 'L', 'N',  &SwW4WParser::Read_PrintLastPageNo,
    'P', 'N', 'D',  &SwW4WParser::Read_ParaNumberDef,
    'P', 'O', 'R',  &SwW4WParser::Read_PageOrient,
    'P', 'P', 'N',  &SwW4WParser::Read_PrintPageNo,

    'R', 'D', 'T',  &SwW4WParser::Read_RectangleDefinitionTable,// FrameMaker

    'R', 'M', 'I',  &SwW4WParser::Read_SetRightMarginIndent,
    'R', 'S', 'M',  &SwW4WParser::Read_ReSetLeftRightMargin,
    'R', 'S', 'P',  &SwW4WParser::Read_ReSetLineSpacing,
    'R', 'U', 'L',  &SwW4WParser::Read_Ruler,
    'S', 'A', 'F',  &SwW4WParser::Read_SetSpaceAfter,
    'S', 'B', 'O',  &SwW4WParser::Read_StyleBasedOn,
    'S', 'B', 'F',  &SwW4WParser::Read_SetSpaceBefore,
    'S', 'B', 'P',  &SwW4WParser::Read_SetBottomMarginOfPage,
    'S', 'B', 'S',  &SwW4WParser::Read_BeginSubScript,
    'S', 'C', 'B',  &SwW4WParser::Read_ColumnBreak,
    'S', 'E', 'P',  &SwW4WParser::Read_Separator,
    'S', 'F', 'B',  &SwW4WParser::Read_SetTray,     //!!! konnte noch nicht getestet
                                                    // werden, da W4W den Befehl anscheinend
                                                    // vergisst.
    'S', 'F', 'L',  &SwW4WParser::Read_SetFormLenght,
    'S', 'H', 'C',  &SwW4WParser::Read_SoftHyphen,
    'S', 'H', 'L',  &SwW4WParser::Read_SoftHyphen,
    'S', 'H', 'P',  &SwW4WParser::Read_SoftHyphen,
    'S', 'L', 'G',  &SwW4WParser::Read_LangGroup,
    'S', 'N', 'L',  &SwW4WParser::Read_SoftNewLine,
    'S', 'N', 'P',  &SwW4WParser::Read_SoftNewPage,
    'S', 'P', 'F',  &SwW4WParser::Read_SetPitchAndOrFont,
    'S', 'P', 'N',  &SwW4WParser::Read_SetPageNumber,
    'S', 'P', 'S',  &SwW4WParser::Read_BeginSuperScript,
    'S', 'T', 'E',  &SwW4WParser::Read_StyleEnd,
    'S', 'T', 'F',  &SwW4WParser::Read_StyleOff,
    'S', 'T', 'M',  &SwW4WParser::Read_SetTempLeftRightMargin,
    'S', 'T', 'P',  &SwW4WParser::Read_SetTopMarginOfPage,
    'S', 'U', 'M',  &SwW4WParser::Read_SetUnderlineMode,
    'S', 'T', 'Y',  &SwW4WParser::Read_StyleOn,
    'S', 'Y', 'T',  &SwW4WParser::Read_StyleTable,
    'T', 'A', 'B',  &SwW4WParser::Read_Tab,

    'T', 'R', 'I',  &SwW4WParser::Read_TextRectangelId,			// FrameMaker

    'U', 'C', 'S',  &SwW4WParser::Read_UpperCharSet,
    'W', 'O', 'N',  &SwW4WParser::Read_WidowOrphOn,
    'W', 'O', 'Y',  &SwW4WParser::Read_WidowOrphOff,
    'X', 'C', 'S',  &SwW4WParser::Read_ExtendCharSet
};


void SwW4WParser::FlushChar( sal_Unicode c )
{
    if( bReadTxtIntoString )
        *pReadTxtString += c;
    else
    {
        aCharBuffer[ nChrCnt++ ] = c;
        if( nChrCnt >= CHARBUF_SIZE )
            Flush();
    }
}


void SwW4WParser::ActivateTxtFlags()
{
    if( !bHeadFootDef && !bStyleDef )
          bPageDefRdy
        = bIsTxtInDoc
        = bIsTxtInFNote
        = bIsTxtInPgDesc
        = bWasTxtSinceLastHF2
        = bWasTxtSinceLastHF1
        = bWasTxtSince_BREAK_PAGE
        = TRUE;
}


void SwW4WParser::DoSplit()
{
    /*
      Beachten: die Nummerierung des alten Absatzes
                muss erhalten bleiben...
    */
    SwTxtNode* pTxtNd = pCurPaM->GetNode()->GetTxtNode();
    pDoc->SplitNode( *pCurPaM->GetPoint() );

    if( pTxtNd->GetNum() )
    {
        SwNodeIndex aIdx( *pTxtNd, -1 );
        if( aIdx.GetNode().IsTxtNode() )
            aIdx.GetNode().GetTxtNode()->UpdateNum( *pTxtNd->GetNum() );
        pTxtNd->ResetAttr( RES_PARATR_NUMRULE );
        pTxtNd->UpdateNum( SwNodeNum( NO_NUMBERING ));
    }

    nParaLen = 0;
    bIsIpsInPara   = FALSE;		// Neuer Absatz -> neue Zaehlung
    bIsTxtInPara   = FALSE;
    bIsSTMInPara   = FALSE;
    bIsNumListPara = FALSE;
}

// Funktion fuer Vorgangs-Anzeige: Hier und in shellio.cxx das Verhaeltnis der
//								   Wandeldauer MASO zu uns justieren

void SwW4WParser::UpdatePercent( ULONG nPos, ULONG nFileSize )
{
    ::binfilter::SetProgressState( 38 + nPos * 62 / nFileSize, pDoc->GetDocShell() );
}


                // RSP wird bei vorherigem IPS nicht gebraucht, da redundant
                // bei WP ist es aber noetig

void SwW4WParser::Read_ReSetLineSpacing()		// (RSP)
{
    long nOld, nNew, nOptNewTw;

    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_PARATR_LINESPACING );
        return;
    }

    if( GetDecimal( nOld ) && !nError && GetDecimal( nNew ) && !nError ){
        if( W4WR_TXTERM != GetDecimal( nOptNewTw ) || nError )
            nOptNewTw = nNew * 120; // in half Lines

        BOOL bStd = FALSE;
        SvxLineSpacingItem aLSpc;

        if ( nOptNewTw % 120 == 0 ){
            bStd = TRUE;
            switch ( nOptNewTw ){
            case 240:		// 1-Zeilig
                      aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
                      aLSpc.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
                      break;

            case 360:		// 1.5-Zeilig
                      aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
                      aLSpc.SetPropLineSpace( 150 );
                      break;
            case 480:		// 2-Zeilig
                      aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
                      aLSpc.SetPropLineSpace( 200 );
                      break;
            default:		// Krumme Werte
                      bStd = FALSE;
                      break;
            }
        }
        if ( !bStd ){		// krumme Werte als Minimum uebernehmen
            aLSpc.SetLineHeight( (USHORT)nOptNewTw );
            aLSpc.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        }

        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_PARATR_LINESPACING );
        if ( nOptNewTw != 240 ) // Wert 240 wird zu Ausschalten benutzt
            SetAttr( aLSpc );
    }
}


void SwW4WParser::Read_SetRightMarginIndent()	// (RMI)
{
    if( bStyleOnOff )
    { 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_LR_SPACE );
    }
    else
    if( !bIsIpsInPara  )
    {

        long nSpaces, nOptRelRightTw;
        short nRight;

        if( W4WR_TXTERM != GetDecimal( nSpaces ) || nError )
            return;

        // gibt es die optionale Angabe ?
        if( W4WR_TXTERM == GetDecimal( nOptRelRightTw ) && !nError )
            nRight = (USHORT) nOptRelRightTw;	// evtl. UEberlauf ist beabsichtigt,
                                                // da bei W4W neg. Zahlen um 65536
                                                // erhoeht dargestellt werden
        else
            nRight = (USHORT)nSpaces*nColSize;	// in 1/10 Inch

        if( 0 <= nRight )
        {
            if( bIsColMode && pTabDefs )
            {			// Tabelle
                long nBoxSize = pTabDefs[nTabCol].nRightTw
                                - pTabDefs[nTabCol].nLeftTw;
                if( nRight + HOR_SICHER > nBoxSize / 2 )
                    nRight = 0; // Stuss-Werte -> keine Einrueckung
            }
            SvxLRSpaceItem aLR( *(SvxLRSpaceItem*)
                            pCtrlStck->GetFmtAttr( *pCurPaM, RES_LR_SPACE ));

            aLR.SetRight( nRight );
            SetAttr( aLR );
        }
    }
    /*
        Achtung: nach RMI sollte immer RSM folgen; ist zu ueberspringen
    */
    BOOL bOldNoExec   = bNoExec;
    BOOL bOldTxtInDoc = bTxtInDoc;
    bNoExec   = TRUE;
    bTxtInDoc = FALSE;
    ULONG nOldPos = rInp.Tell();	// FilePos merken
    SkipEndRecord();
    GetNextRecord();
    if(      pActW4WRecord
        && (    pActW4WRecord->fnReadRec
             != &SwW4WParser::Read_ReSetLeftRightMargin ) )
    {
        rInp.Seek( nOldPos ); 	 	// FilePos restaurieren
    }
    bNoExec   = bOldNoExec;
    bTxtInDoc = bOldTxtInDoc;
    nChrCnt   = 0;
    aCharBuffer[ nChrCnt ] = 0;
}


void SwW4WParser::Read_BeginSmallCaps() 	// (BCS)
{
    if( !bStyleOnOff )
        SetAttr( SvxCaseMapItem( SVX_CASEMAP_KAPITAELCHEN ) );
}


void SwW4WParser::Read_EndSmallCaps()		// (ECS)
{
    if( !bStyleOnOff )
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_CASEMAP );
}


void SwW4WParser::Read_BeginUpperCaps() 	// (BCU)
{
    if( !bStyleOnOff )
        SetAttr( SvxCaseMapItem( SVX_CASEMAP_VERSALIEN ) );
}


void SwW4WParser::Read_EndUpperCaps()		// (ECU)
{
    if( !bStyleOnOff )
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_CASEMAP );
}


void SwW4WParser::Read_PageOrient() 	// (POR)
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    long nOrient;

    if( W4WR_TXTERM != GetDecimal( nOrient ) || nError )
        return;

    if( bIsTxtInPgDesc && nDocType != 15 ) 		// -> neues SLay mit SeitenEnde falls noetig
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    pPageDesc->SetLandscape( nOrient != 0 );
}

                //!!! noch nicht getestet (W4W-Fehler)


void SwW4WParser::Read_SetTray()		// (SFB)
{
    long nOldTray, nTray;			// evtl noch neuen PageDesc anlegen

    if( bStyleDef ) return;

    if( GetDecimal( nOldTray ) && !nError
        && W4WR_TXTERM != GetDecimal( nTray ) || nError )
        return;

    pPageDesc->GetMaster().SetAttr( SvxPaperBinItem( RES_PAPER_BIN, (BYTE)nTray));
}


void SwW4WParser::Read_SoftHyphen() 	// (SHC, SHL, SHP)
{
    pDoc->Insert( *pCurPaM, CHAR_SOFTHYPHEN );
}


void SwW4WParser::Read_HardHyphen() 	// (HHC, HHL, HHP)
{
    FlushChar('-');	// Bitte _so_ stehen lassen,
                // Zeichen wird in Read_SoftNewLine() abgefragt!
}


// -----------------------------
SV_DECL_PTRARR_SORT( W4WStyleIdTab, W4WStyleIdTabEntryPtr, 0, 4 )

SwW4WParser::SwW4WParser( const SwPaM & rPaM, SvStream& rIstream,
                        BOOL bNewDoc, USHORT nFilterNo, const String& rVers )
    : pCurPaM( (SwPaM*)&rPaM ), bNew( bNewDoc ), rVersion( rVers ),
    rInp( rIstream )
{
    pDoc = rPaM.GetDoc();
    rInp.Seek( STREAM_SEEK_TO_END );
    rInp.ResetError(); 			   // Sehe nach, wie gross die
    nW4WFileSize = rInp.Tell();	  // W4W-Datei ist
    if( !nW4WFileSize )
    {
        ASSERT( !this, "SvStream::seek doesn't work correct" );
        nW4WFileSize = 1;
    }
    rInp.Seek( STREAM_SEEK_TO_BEGIN );
    rInp.ResetError();

    if( !bSortRecordTab )
    {
        qsort( (void*)aRecordTab,
            sizeof( aRecordTab ) / sizeof (W4WRecord),
            sizeof( W4WRecord ),
            CompRecord );
        bSortRecordTab = TRUE;
    }
    nError = 0; 	// kein Fehler
    pStyleTab = pStyleBaseTab = 0;

    {
        USHORT nFltNo = nFilterNo;
        const sal_Char* aNames[ 4 ] = {
            0, "W4W/W4W000", "W4W/W4WHD", "W4W/W4WFT" };
        sal_uInt32 aVal[ 4 ];

        SwFilterOptions aOpt;

        sal_Char aFirst[]="W4W/W4W000";
        aNames[0]=aFirst;

        // set into the first name the filter number
        sal_Char* pStr = aFirst;
        pStr += 7;
        *pStr++ = '0' + ( nFltNo / 100 ); nFltNo %= 100;
        *pStr++ = '0' + ( nFltNo / 10  ); nFltNo %=  10;
        *pStr++ = '0' +   nFltNo;

        // first test if the node & content exist. If not, then we may not
        // ask the configuration (produce assertions!)
        pStr = aFirst;
        pStr[ 3 ] = 0;

        if( aOpt.CheckNodeContentExist( pStr, pStr + 4 ))
        {
            pStr[ 3 ] = '/';
            aOpt.GetValues( 4, aNames, aVal );
        }
        else
        {
            aOpt.GetValues( 3, aNames+1, aVal+1 );
            aVal[0] = 0;
        }

        nIniFlags = aVal[ 0 ];
        if( !nIniFlags )
            nIniFlags = aVal[ 1 ];
        nIniHdSiz = aVal[ 2 ];
        nIniFtSiz = aVal[ 3 ];
    }

    // Frame-Erkennung und -Umwandlung vorlaeufig deaktiviert
    // wegen Zeitueberschreitung.
    //
    // noch bestehende Probleme:
    //
    //  - Content im W4W Zwischencode erscheint in Reihelfonge der FLOws,
    //    nicht in Reihenfolge der Seiten, d.h. Inhalt eines Frame auf S.1
    //    kommt womoeglich erst nach Inhalt der Seite 7 usw...
    //
    //  - ... ?
    //
    nIniFMFlags = W4WFL_FM_neverFrames;
}


BOOL SwW4WParser::CallParser()
{
    pCtrlStck = new W4WCtrlStack( *this );
    pActFlySection = 0;

    nError = 0; 			// kein Fehler
    nChrCnt = 0;			// Counter fuer den Buffer
    nTabCols = nTabRows = nTabRow = nTabCol = 0;
    nTabDeltaSpace = 0;
    nAktPgDesc = W4W_INVALID; // kein gueltiges Seitenlayout
    nAktStyleId = nTabStyleId = nNoNameStylesCount = 0;
    nApoBorderCode = 0;
    nDocType = 0;
    nParaLen = 0;
    nGrafPDSType = 0;
    nRecNo = 0;
    nGrWidthTw = 0;
    nGrHeightTw = 0;
    nRSMLeft = 0;
    nRSMRight = lA4Width;
    nRSMwpwParaLeft  = 0;
    nRSMwpwParaRight = 0;
    nLeftMgnCorr  = 0;
    nLastValueSTP = nLastValueSBP = nLastValueHTM =
    nLastValueHM  = nLastValueFBM = nLastValueFM  = 0;
    nNewValueSTP = nNewValueSBP = nNewValueHTM =
    nNewValueHM  = nNewValueFBM = nNewValueFM  = 0;
    nPDType = nHdFtType = 0;
    nTablInTablDepth = 0;
    nStartOfActRecord = 0;

    bSetPgWidth = bSetPgHeight = bPDTanalyzed
    = bStyleDef = bStyleEndRec = bStyleOn = bStyleOnOff
    = bPageDefRdy = bStyleOff = bHeadFootDef = bFootnoteDef = bNoExec
    = bBCMStep1 = bBCMStep2 = FALSE;

    bSepar = bIsTxtInPgDesc = bIsTxtInFNote
    = bWasTxtSinceLastHF2 = bWasTxtSinceLastHF1
    = bIsTxtInDoc = bIsTxtInPara = bIsSTMInPara = bIsNumListPara
    = bWasTxtSince_BREAK_PAGE
    = bWasHNPInPgDesc = bEndNoteInfoAlreadySet
    = bFtnInfoAlreadySet = FALSE;

    bIsNLN = bDefFontSet = bIsColMode = bWasCellAfterCBreak =
    bIsSTYInTab = bIsColDefTab = bPic = bPicPossible =
    bIsIpsInPara = bToxOpen = bPersAlign = FALSE;

    bPgMgnChanged = bWasXCS = bWasSLG = bIgnoreNTB =
    bWWWasRSM = bIsTab = bWPWWasRSM = bSingleParaMgn =
    bCheckTabAppendMode = bIsTabAppendMode =
    bWasPGNAfterPND = FALSE;

    bFootNoteNumberAutomatic = bEndNoteNumberAutomatic = TRUE;
    bWasFootNoteOnW4WPage = bReadTxtIntoString
    = bNixNoughtNothingExec = FALSE;
    eFootNotePageRestart  = VALUE_UNKNOWN;

    bTxtInDoc = TRUE;

    pPageDesc = pLastActPageDesc =
    pOpen1stPgPageDesc = pPageDescForNextHNP = 0;

    nLastReadFootNo = nLastReadEndNo = USHRT_MAX;

    nFootNoOverflow = nEndNoOverflow = 0;

    pTabDefs     = 0;
    pTabBorders  = 0;
    pMergeGroups = 0;

    pPDTInfos    = 0;
    pRDTInfos    = 0;
    pFLOInfos    = 0;
    pActRectInfo = 0;
    pPDTInfo     = 0;

    pBehindSection=0;

    pActNumRule   = 0;
    pActW4WRecord = 0;
    aLastRecType.Reset();

    nUnderlineMode = 2;

    pGraf = 0;
    pGraphic = 0;
    nFtnType = -1;
    nPgLeft  = 0;
    nPgRightDelta = 0;
    nPgRight = 10440;
    nPgWidth = lA4Width;
    nColSize = 144;
    nDefLanguage = 1031;   // besser: Default-Language

// ((const SwFmtLanguage&)pDoc->GetDefault( RES_CHRATR_LANGUAGE )).nLanguage;

    USHORT nPageDescOffset = pDoc->GetPageDescCnt();

    SwNodeIndex aSttNdIdx( pDoc->GetNodes() );
    SwRelNumRuleSpaces aRelNumRule( *pDoc, bNew );

    // eigener Page-Descriptor
    CreatePageDesc(   (bNew ? CRPGD_AND_INSERT : 0)
                    | CRPGD_REMOVE_HD
                    | CRPGD_REMOVE_FT );

    /*
        Hauptschleife des Parser
        ************************
        ************************
    */
    int nRet = 0;

    while( ( EOF != nRet ) && !nError )
        nRet = GetNextRecord();



    if( pCtrlStck )
        DeleteZStk( pCtrlStck );

    if( pStyleTab )
    {
        pStyleTab->DeleteAndDestroy( 0, pStyleTab->Count() );
        DELETEZ( pStyleTab );
    }

    if( pStyleBaseTab )
        pStyleBaseTab->DeleteAndDestroy( 0, pStyleBaseTab->Count() );

    // Pagedescriptoren am Dokument updaten (nur so werden auch die
    // linken Seiten usw. eingestellt).

    for (USHORT i=nPageDescOffset; i < pDoc->GetPageDescCnt(); i++ )
    {
                    //!!!!
//		const SwFmtFooter &rBla = pDoc->GetPageDesc( i ).GetLeft().GetFooter();
//		const SwFmtFooter &rBla2 = pDoc->GetPageDesc( i ).GetMaster().GetFooter();
        pDoc->ChgPageDesc( i, pDoc->GetPageDesc( i ) );
    }


    DELETEZ( pTabDefs );		// nur noetig, wenn ECM fehlt
    DELETEZ( pBehindSection );	// dito.


    // ggfs. FrameMaker Daten abraeumen
    if( pPDTInfos )
    {
        if( pPDTInfos->Count() )
        {
            for( USHORT n = pPDTInfos->Count(); 0 < n; )
            {
                --n; // Decrement IN der Schleife !!

                W4WPDTInfo& rInfo = *pPDTInfos->GetObject( n );

                if((USHRT_MAX != rInfo.nSwPdId) && !rInfo.bSwPdUsed )
                {// ^^^^^^^^^^^^^					NIE PageDesc NULL loeschen!
                    pDoc->DelPageDesc( rInfo.nSwPdId );
                }
            }
        }
        DELETEZ( pPDTInfos );
    }
    if( pRDTInfos )
    {
        for(W4WRectInfo* p = pRDTInfos->First(); p; p = pRDTInfos->Next())
            delete p;
        DELETEZ( pRDTInfos );
    }
    if( pFLOInfos )
    {
        /*
        for( USHORT n = 0; n < pFLOInfos->Count(); ++n )
        {
            W4WFLOInfo& rInfo = *pFLOInfos->GetObject( n );
            if( !rInfo.bFlowUsed )
            {
                // was machen wir im Falle eines unbenutzten Flow ??
            }
        }
        */
        DELETEZ( pFLOInfos );
    }


    /*
        ggfs. Fuss- und/oder Endnoten auf automatische Zaehlung schalten
    */
    if(    ( (USHRT_MAX != nLastReadFootNo) && bFootNoteNumberAutomatic )
        || ( (USHRT_MAX != nLastReadEndNo ) && bEndNoteNumberAutomatic  ) )
    {
        SwFtnIdxs& rFtnIdxs = pDoc->GetFtnIdxs();
        for( USHORT n = 0; n < rFtnIdxs.Count(); ++n )
        {
            SwTxtFtn& rFtn = *rFtnIdxs[ n ];
            if( rFtn.GetFtn().IsEndNote() ? bEndNoteNumberAutomatic
                                          : bFootNoteNumberAutomatic )
            {
                rFtn.SetNumber( 0, 0 );
            }
        }

        /*
            ggfs. noch die Fuss-Noten auf Neuanfang je Seite schalten
        */
        if( VALUE_TRUE == eFootNotePageRestart )
        {
            SwFtnInfo aInfo( pDoc->GetFtnInfo() );
            aInfo.eNum = FTNNUM_PAGE;
            pDoc->SetFtnInfo( aInfo );
        }

        rFtnIdxs.UpdateAllFtn();
    }

    // NumRules koennen erst nach dem setzen aller Attribute korrgiert werden
    aRelNumRule.SetNumRelSpaces( *pDoc );
    if( !bNew && aSttNdIdx.GetIndex() )
    {
        aSttNdIdx++;
        aRelNumRule.SetOultineRelSpaces( aSttNdIdx,
                                    pCurPaM->GetPoint()->nNode );
    }

//	return EOF == nRet && !nError;


    return 1;						// lese ein, auch wenn Error
}


void SwW4WParser::DeleteZStk( W4WCtrlStack*& rpStk )
{
    Flush();
    ASSERT(rpStk, "W4W-ControlStack bereits geloescht");
    const SwPosition& rPos = *pCurPaM->GetPoint();
    pCtrlStck->SetAttr( rPos, 0, FALSE );
    pCtrlStck->SetAttr( rPos, 0, FALSE );
    delete rpStk;
    rpStk = 0;
}


int SwW4WParser::GetNextRecord()	// Typ muss int sein, damit Vergleich mit EOF geht
{
    register unsigned int c;
    BOOL bBegIcf = FALSE;

#if OSL_DEBUG_LEVEL > 1
    if( bTxtInDoc && !bNoExec || bStyleDef )
        nRecNo++;
#endif

    while( !nError )
    {
        if( 0 == ( c = ReadChar()) )
            break;

        if( W4WR_BEGICF == c )
        {
            if( bBegIcf )
            {
                nError = ERR_RECORD;		// 2 mal BEGICF ??
                return nError;
            }
            bBegIcf = TRUE;
            continue;
        }

        if( W4WR_LED == c )  // Record-Anfang
        {
            if( !bBegIcf )
            {
                nError = ERR_RECORD;  	// LED ohne vorheriges BEGICF ??
                return nError;
            }

            // ok, es wurde ein Record Anfang erkannt
            nStartOfActRecord = rInp.Tell() - 1;

            // speicher erstmal den Puffer im Dokument
            if( bTxtInDoc )
                Flush();

            // lies die Record-Kennung
            W4WRecord aTmpRec;
            rInp.Read( (sal_Char*) &(aTmpRec.aRecType.c0), 3 );

            if( rInp.IsEof() )
            {
                nError = ERR_RECORD;
                return EOF;
            }

//			NOTE4( "<<%c%c%c>>", aTmpRec.aRecType.c0, aTmpRec.aRecType.c1,
//					aTmpRec.aRecType.c2 );

            // dann suche nach der Methode fuer den Record
            pActW4WRecord = 0;
            if( 0 != ( pActW4WRecord = (W4WRecord*)bsearch( (char *) &aTmpRec,
                                (char *) aRecordTab,
                                sizeof( aRecordTab ) / sizeof( W4WRecord ),
                                sizeof( W4WRecord ),
                                CompRecord ) ))
            {
                if( bNixNoughtNothingExec )
                {
                    nStreamPosInRec = rInp.Tell();
                }
                else
                {
                    if( bNoExec )
                    {
                        // Modus: Suche nach Ende des Headers/Footers/Tabelle
                        if (  ((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_HeadFootEnd)
                            ||((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_FootNoteEnd)
                            ||((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_BeginTabRow)
                            ||((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_BeginTabCell)
                            ||((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_EndColumnMode)
                            ||((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_ColumnsDefinition)
                            ||((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_BeginColumnMode  )
                           )
                        {
                            (this->*pActW4WRecord->fnReadRec)();	// Ignoriere alles Andere
                        }
                    }
                    else if (bStyleDef && bSepar)
                    {	// Modus: Styledef. Aussschaltsequenz
                        if ((pActW4WRecord->fnReadRec)==&SwW4WParser::Read_Separator)
                            Read_HeadFootEnd(); 	// Ignoriere alles Andere
                    }
                    else
                    {	// normaler Modus
                        (this->*pActW4WRecord->fnReadRec)();
                    }
                }
                // im Stream 1 Position zurueck und dann alles ueberlies
                // bis zum Ende des Records

                rInp.SeekRel( -1 );
            }
            SkipEndRecord();
            if( !aTmpRec.aRecType.IsEqual( RTL_CONSTASCII_STRINGPARAM("BCM" ))
                || !aLastRecType.IsEqual( RTL_CONSTASCII_STRINGPARAM("ECM" )))
                aLastRecType = aTmpRec.aRecType;
        }
        bBegIcf = FALSE;
        if ( c >= 32 && bWasXCS ){	// naechsten Buchstaben / UCF / HEX
            bWasXCS = FALSE;		// nach einem XCS ueberlesen
            continue;
        }
        if( c >= 0x20 && c < 0xff ) // Zeichen ueber 0x7f kommen in Namen
                                    // innerhalb von Befehlen (z.B. BMT) vor
                                    // 0xff bringt Bernd ins Schleudern
            FlushChar( c );
        else
            return (BYTE)c; 	  // ein unbekanntes Zeichen
             // nError = ERR_CHAR; // was soll damit geschehen ??
    }
    return rInp.IsEof() ? EOF : nError;
}


// SkipParaX ueberliest einen Parameter. Der Stream darf hinter dem BEGICF
// stehen. SkipParaX aehnelt SkipPara, laesst sich aber durch geschachtelte
// ICFs nicht verarschen.

BYTE SwW4WParser::SkipParaX()
{
    register unsigned int c;
    register unsigned int nRecurs = 0;

    while( TRUE )
    {
        if ( ( c = ReadChar() ) == 0 )		// eof ergibt 0-Wert als Rueckgabe
        {									// eof oder ungueltigiges Zeichen
            nError = ERR_CHAR;
            return 0;
        }
        if ( W4WR_TXTERM == c && !nRecurs )
            break;					// fertig

        if ( W4WR_RED == c )	// z.B. fuer codierte Umlaute (XCS/UCS)
            nRecurs--;
        else if( W4WR_LED == c )
            nRecurs++;
    }
    return (BYTE)c;
}

BYTE SwW4WParser::GetChar( BYTE& rHexVal )
{
    register unsigned int c;
    register unsigned int c2;
    rHexVal = 0;				// Default fuer Error

    if( ( c =  ReadChar() ) == 0 ){ 			// eof oder Mist im File
        nError = ERR_CHAR;
        return 0;
    }
    if( W4WR_RED == c || W4WR_TXTERM == c ){
        return (BYTE)c;
    }
    if( W4WR_BEGICF == c )
        SkipParaX();			// ignoriere mit XCS/UCS codierte Zeichen
                                //!! nicht ganz schoen

    if( ( c2 = ReadChar() ) == 0 ) // lese schon naechstes Zeichen
        nError = ERR_CHAR;			// sollte nur TXTERM / RED sein !
    rHexVal = (BYTE)c;
    return (BYTE)c2;
}


BOOL SwW4WParser::GetString( String& rString, const int nEndCode1, const int nEndCode2 )
{
    nChrCnt = 0;
    int nGetCode = GetNextRecord();
    BOOL bOk =     ( !nError )
                && (   ( nEndCode1 == nGetCode )
                    || ( nEndCode2 == nGetCode ) );
    if( bOk && nChrCnt )
    {
        aCharBuffer[ nChrCnt ] = 0;
        rString = aCharBuffer;
    }
    else
        rString = aEmptyStr;
    nChrCnt = 0;
    aCharBuffer[ nChrCnt ] = 0;
    return bOk;
}


    // returnt W4WR_RED/W4WR_TXTERM
    // in rByteVal steht der als erste Ziffer gelesene BYTE-Wert

BYTE SwW4WParser::GetDeciByte( BYTE& rByteVal )
{
    register BYTE n;
    register BOOL Ok = FALSE;
    BYTE nCode;

    while( this )
    {
        n =  ReadChar();
        if ( '0' > n || '9' < n )   // eof durch 0 abgedeckt
            break;
        if( !Ok )
        {
            nCode = (n - '0');
            Ok    = TRUE;
        }
    }
    if( n == 0 )
        nError = ERR_CHAR;

    if( Ok && ( n == W4WR_TXTERM ) )
    { 	// ordentlich abgeschlossene Eingabe,
        // dann abspeichern, sonst rByteVal unveraendert
        rByteVal = nCode;
    }
    return n;
}


    // GetHexByte returnt W4WR_RED/W4WR_TXTERM wenn einstellige HexZahl, sonst 0
    // in rHexVal steht gewandelter HexCharakter

BYTE SwW4WParser::GetHexByte( BYTE& rHexVal )
{
    register unsigned int c1 = 0;	// shorts sind schneller als char (WIN)
    register unsigned int c2 = 0;
    register unsigned int nRet;

    rHexVal = 0;

    c1 = ReadChar();			// eof liefert 0 zurueck
    if ( c1 == 0 ){ 			// falsches Zeichen oder eof
        nError = ERR_CHAR;
        return 0;
    }
    if( W4WR_RED == c1 || W4WR_TXTERM == c1 ){
        return (BYTE)c1;
    }
    c2 = ReadChar();
    if ( c2 == 0 ){ 			// falsches Zeichen oder eof
        nError = ERR_CHAR;
        return (BYTE)c2;
    }

                                    // Hi-Byte
    if( c1 >= '0' && c1 <= '9' )        nRet = (c1 - '0');
    else if( c1 >= 'A' && c1 <= 'F' )   nRet = (c1 - 'A' + 10);
    else if( c1 >= 'a' && c1 <= 'f' )   nRet = (c1 - 'a' + 10);

    if( W4WR_RED == c2 || W4WR_TXTERM == c2 ){
        rHexVal = (BYTE)nRet;
        return (BYTE)c2;
    }

    nRet <<= 4; 	// *16

                                    // Lo-Byte
    if( c2 >= '0' && c2 <= '9' )        nRet |= (c2 - '0');
    else if( c2 >= 'A' && c2 <= 'F' )   nRet |= (c2 - 'A' + 10);
    else if( c2 >= 'a' && c2 <= 'f' )   nRet |= (c2 - 'a' + 10);

    c2 = ReadChar();				// lese schon naechstes Zeichen
    if( c2 == 0 )					// pInp->eof()
        nError = ERR_CHAR;			// sollte nur TXTERM / RED sein !

    rHexVal = (BYTE)nRet;
    return (BYTE)c2;
}

    // returnt W4WR_RED/W4WR_TXTERM wenn ordentliche HexZahl, sonst 0
    // in rHexVal steht gewandelter HexUShort

BYTE SwW4WParser::GetHexUShort( USHORT& rHexVal )
{
    register unsigned int c;
    register unsigned int nVal = 0;
    rHexVal = 0;				// Default fuer Error

    for (int i=0; i<4; i++){

        c = ReadChar();
        if( c == 0 ){				// eof oder Mist im File
            nError = ERR_CHAR;
            return 0;
        }
        if( W4WR_RED == c || W4WR_TXTERM == c ){
            return (BYTE)c;
        }

        nVal <<= 4;

        if( c >= 'a' && c <= 'f' )      nVal += (c - 'a' + 10);
        else if( c >= 'A' && c <= 'F' ) nVal += (c - 'A' + 10);
        else if( c >= '0' && c <= '9' ) nVal += (c - '0');
        else { nError = ERR_CHAR; return (BYTE)c; }
    }
    c = ReadChar();				// lese schon naechstes Zeichen
    if( c == 0 )
        nError = ERR_CHAR;			// sollte nur TXTERM / RED sein !
    rHexVal = (USHORT)nVal;
    return (BYTE)c;
}

    // returnt W4WR_RED/W4WR_TXTERM
    // in rDecVal steht der gewandelte Decimal-Wert

BYTE SwW4WParser::GetDecimal( long& rDecVal )
{
    register unsigned int c;			// schneller als BYTE (win)
    register UINT16 uVal = 0;

    while( TRUE ){
        c = ReadChar();
        if ( '0' > c || '9' < c )   // eof durch 0 abgedeckt
            break;
        uVal = ( uVal * 10 ) + c - '0';
    }
    if( c == 0 )
        nError = ERR_CHAR;

    if( c == W4WR_TXTERM ){ 	// ordentlich abgeschlossene Eingabe
                                // dann abspeichern
                                // sonst rDecVal unveraendert

        register INT16 iVal = uVal; // Werte > 32767 -> negativ
                                    // ( wird in W4W so codiert )
        rDecVal = iVal;
    }
    return (BYTE)c;
}


    // ueberliest alles bis zum W4WR_TXTERM oder W4WR_RED (wird returnt)

BYTE SwW4WParser::SkipPara()
{
    register unsigned int c;
    while( TRUE ){
        c = ReadChar();
        if ( W4WR_TXTERM == c || W4WR_RED == c || 0 == c )	// 0 == eof
            break;
    }

    if( c == 0 ) nError = ERR_CHAR;
    return (BYTE)c;
}
    // ueberliest alles bis zum W4WR_TXTERM oder W4WR_RED (wird returnt)

BYTE SwW4WParser::SkipEndRecord()
{
    register unsigned int c;
    register unsigned int nRecurs = 1;

    while( nRecurs ){
        c = ReadChar();		// eof ergibt 0-Wert als Rueckgabe
        if ( c == 0 ){			// eof oder ungueltigiges Zeichen
            nError = ERR_CHAR;
            return 0;
        }
        if ( W4WR_RED == c ){	// z.B. fuer codierte Umlaute (XCS/UCS)
            nRecurs--;
        }else if( W4WR_LED == c ) {
            nRecurs++;
        }
    }
    return (BYTE)c;
}

void SwW4WParser::Flush()
{
    nParaLen += nChrCnt;
    aCharBuffer[ nChrCnt ] = 0;

    if( !nChrCnt )
        return;

    if( bTxtInDoc )
    {
        bIsTxtInPara = TRUE;
        ActivateTxtFlags();

//		bIsNLN = FALSE;
        if( bIsTxtInPgDesc )
        {
            UpdatePageMarginSettings( CALLED_BY_FLUSH_OR_HNL );
        }
        if ( nParaLen >= MAX_ASCII_PARA - CHARBUF_SIZE - 100 )
        {
            sal_Unicode* pBuf = aCharBuffer;
            for( ; 0 != *pBuf; ++pBuf )
                if( ' ' == *pBuf )            // suche ' '
                    break;
            sal_Unicode c = *pBuf;
            *pBuf = 0;
            if( bReadTxtIntoString )
            {
                *pReadTxtString += aCharBuffer;
                *pReadTxtString += '\n';
            }
            else
            {
                pDoc->Insert( *pCurPaM, aCharBuffer, FALSE );
                pDoc->SplitNode( *pCurPaM->GetPoint() );
            }
            nParaLen = 0;
            *pBuf = c;
            if( pBuf != aCharBuffer )
            {
                if( bReadTxtIntoString )
                    *pReadTxtString += pBuf;
                else
                    pDoc->Insert( *pCurPaM, pBuf, FALSE );
            }
        }
        else
        {
            if( bReadTxtIntoString )
                *pReadTxtString += aCharBuffer;
            else
                pDoc->Insert( *pCurPaM, aCharBuffer, FALSE );
        }
    }
    else if( nChrCnt > CHARBUF_SIZE)
    {
        // das ist normalerweise ein Fehler !!!
        nError = ERR_NAMETOLONG;
    }
    nChrCnt = 0;
}


void SwW4WParser::Read_UpperCharSet()			// (UCS)
{
    // nach einem XCS folge meistens ein UCS / HEX oder ein normaler
    // Buchstabe, der benutzt werden soll, wenn das XCS ignoriert wird.
    // Deshalb muss das naechste USC / HEX / Buchstabe nach XCS
    // ignoriert werden. ( Gespraech Karl Forster )
    if ( bWasXCS ) 				// naechsten Bucchstaben / UCF / HEX
    {
        bWasXCS = FALSE;		// nach einem XCS ueberlesen
        return;
    }
    BYTE c, cRet;
    while( !nError && W4WR_RED != ( cRet = GetHexByte( c )) )
        if( cRet == W4WR_TXTERM )
        {
            sal_Unicode cC = ByteString::ConvertToUnicode( c,
                                                RTL_TEXTENCODING_IBM_437 );

            FlushChar( cC );			// z.B. Umlaute
        }
    if( W4WR_RED != cRet )
        nError = ERR_CHAR;
}


void SwW4WParser::Read_ExtendCharSet()			// (XCS)
{
    BYTE c;
    long nValue;
    if( W4WR_TXTERM == GetDecimal( nValue ) && !nError &&
        GetHexByte( c ) && !nError )
    {
        rtl_TextEncoding eCodeSet = RTL_TEXTENCODING_MS_1252;

        if( nValue == 850 && c == 0xef )	//! Sonderbehandlung fuer Haeckchen
        {	                                // von WordPerfect
            nValue = 819;
            c = 180;						// Macke W4W ??
        }
        if ( !( nIniFlags & W4WFL_NO_WW_SPECCHAR )
             && ( nDocType == 44 || nDocType == 49 ) //! WW2: Hier stimmen die
             && nValue == 9998						// Umlaute "A, "U, "s nicht
             && ( c == 0xc4 || c == 0xdc || c == 0xdf ))
            nValue = 819;					// mache dann Umlaute aus Symbolen

        switch( nValue )
        {
        case 9999:	// Complete Mactintosh Char Set
            eCodeSet = RTL_TEXTENCODING_APPLE_ROMAN;
#ifdef MAC
            if ( nDocType == 1 && rVersion == "0" ) 	// Dos-Ascii
                eCodeSet = RTL_TEXTENCODING_IBM_850;	// Fehler im Dos-Filter
                                                        // umpopeln
#endif
            break;
        case 437:	// Standard US PC code page
            eCodeSet = RTL_TEXTENCODING_IBM_437;
            break;
        case 850:	// Standard international PC code page
            eCodeSet = RTL_TEXTENCODING_IBM_850;
            break;
        case 819:	// ANSI code page
            eCodeSet = ( 39 == nDocType &&
                            rVersion.EqualsAscii( "0" ))  // MS Works f. DOS
                        ? RTL_TEXTENCODING_IBM_850
                        : RTL_TEXTENCODING_MS_1252;
            break;
        case 8591:	// ISO 8859-1
            eCodeSet = RTL_TEXTENCODING_ISO_8859_1;
            break;
        case 8592:	// ISO 8859-2
            eCodeSet = RTL_TEXTENCODING_ISO_8859_2;
            break;
        case 9998:	// Windows Standard-Symbol-Charset
            {
                SvxFontItem aFont( FAMILY_DONTKNOW,  String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Symbol" )),
                                    aEmptyStr, PITCH_DONTKNOW,
                                    RTL_TEXTENCODING_SYMBOL );
                Flush();
                SetAttr( aFont );				// neuer Font
                FlushChar( c );
                Flush();								// Sonderzeichen
                // Font wieder zurueck
                pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_FONT );
                bWasXCS = TRUE;
                return;
            }
        }

        sal_Unicode cC = ByteString::ConvertToUnicode( c, eCodeSet );
        FlushChar( cC );

        // folgt ein UCS-Record oder ein HEX-Record hinter einem XCS-Rec.,
        //	  dann ueberlies diesen.
        //	  folgt ein Zeichen, dann ueberlese dieses
        bWasXCS = TRUE;
    }
    else if( !nError )
        nError = ERR_CHAR;
}


void SwW4WParser::Read_Hex()		// (HEX)
{
    if( !bWasXCS || nDocType == 15 )
    {
        BYTE c;
        if( !nError && GetHexByte( c )  &&
            0x15 == c ) 					// Juristenparagraph
        {
            sal_Unicode cC = ByteString::ConvertToUnicode( '\xa7',
                                        RTL_TEXTENCODING_MS_1252 );
            if( bReadTxtIntoString )
                *pReadTxtString += cC;
            else
                pDoc->Insert( *pCurPaM, cC );
        }
    }
    bWasXCS = FALSE;		// HEX nach einem XCS ist hiermit ueberlesen
}


void SwW4WParser::Read_DefineDocumentType() 	// (DID)
{
    long nType;

    if( GetDecimal( nType ) && !nError )
    {
        nDocType = (USHORT)nType;
        if( nDocType == 15 &&  !( nIniFlags & W4WFL_NO_PCTEXT4_124 ) )
            nColSize = 124;
    }
}


void SwW4WParser::Read_SoftNewLine()		  // (SNL)
{
    /*
        was macht Read_SoftNewLine() ?

        Pruefen, ob links davon mindestens ein Dutzend Zeichen stehen
        und das .<SNL> nach direkt hinter der Kombination
        BUCHSTABE + KLEINBUCHSTABE + MINUS steht
        und nach dem SNL mindestens zwei Kleinbuchstaben kommen.

        Ist dies der Fall, wird das Minus entfernt und das SNL durch
        ein .<SHC> ersetzt, da wir annehmen, dass es sich um einen
        weichen Trenner handelt, der von W4W bloss nicht gemeldet wurde.

        Ansonsten darf das SNL-Flag dennoch nicht ignoriert werden, da
        sonst das vorige und das folgende Wort verschmelzen.
        Stattdessen ist (lt. W4W-Handbuch) immer dann ein Space
        einzufuegen, wenn vorher weder ein Space, noch ein Tab noch
        ein Hyphen kam.
    */
    xub_StrLen nDocPos = PtCnt( pCurPaM ).GetIndex();

    if( nDocPos )
    {
        BOOL bAllDone = FALSE;
        const String& rTxt = pCurPaM->GetPoint()->nNode.GetNode().
                                                    GetTxtNode()->GetTxt();
        sal_Unicode cLastChar = rTxt.GetChar( nDocPos-1 );

        if( 11 < nDocPos )
        {
            const CharClass& rCC = GetAppCharClass();
            sal_Int32 nChrType;

            if( '-' == cLastChar &&
                CharClass::isLetterType(
                    rCC.getCharacterType( rTxt, nDocPos-3 ) ) &&
                CharClass::isLetterType( nChrType =
                    rCC.getCharacterType( rTxt, nDocPos-2 ) ) &&
                0 == ( ::com::sun::star::i18n::KCharacterType::UPPER &
                        nChrType ))
            {
                SwNodeIndex aNdIdx( PtNd( pCurPaM ), -1 );

                SkipEndRecord();
                GetNextRecord();
                rInp.SeekRel( -1 );	// ein Zeichen zurueck

                if( aNdIdx.GetIndex()+1 == PtNd( pCurPaM ).GetIndex() )
                {
                    if( nChrCnt )
                        nChrType = rCC.getCharacterType(
                                                String( aCharBuffer ), 0 );
                    else
                        nChrType = rCC.getCharacterType( rTxt, nDocPos );
                    if( CharClass::isLetterType( nChrType ) &&
                        0 == ( ::com::sun::star::i18n::KCharacterType::UPPER &
                        nChrType ))
                    {
                        SwTxtNode* pNd = PtNd(  pCurPaM ).GetNode().GetTxtNode();
                        xub_StrLen nCnt =  PtCnt( pCurPaM ).GetIndex();
                        PtNd(  pCurPaM ) = aNdIdx.GetIndex()+1;
                        PtCnt( pCurPaM ).Assign(
                            pCurPaM->GetCntntNode(), nDocPos-1 );

                        SvxFontItem* pFont = (SvxFontItem*)GetFmtAttr( RES_CHRATR_FONT );

                        if(    ( !pFont  )
                            || ( RTL_TEXTENCODING_SYMBOL != pFont->GetCharSet() ) )
                        {
                            /*
                                bereits eingefuegten Trennstrich loeschen und
                                weichen Trenner einfuegen
                            */
                            pNd->Erase( PtCnt( pCurPaM ), 1 );
                            pDoc->Insert( *pCurPaM, CHAR_SOFTHYPHEN );
                            bAllDone = TRUE;
                        }
                        PtNd(  pCurPaM ) = *pNd;
                        PtCnt( pCurPaM ).Assign( pCurPaM->GetCntntNode(), nCnt );
                    }
                }
            }
        }
        if(    ( !bAllDone )
            && ( ' '  != cLastChar )
            && ( '\t' != cLastChar )
            && ( '-'  != cLastChar ) ) FlushChar( ' ' );
    }
}



void SwW4WParser::Read_NonBreakSpace()			// (NBR)
{
    pDoc->Insert( *pCurPaM, CHAR_HARDBLANK );
}



void SwW4WParser::Read_HardNewLine()			// (HNL)
{
    UpdatePercent( rInp.Tell(), nW4WFileSize );

    if( bCheckTabAppendMode )
    {
        return;
    }

    if( bIsNLN )
    {
        bIsNLN = FALSE;
        return; 			// Nach NLN folgt zu ignorierendes HNL
    }

    BYTE c = ReadChar();

    if(    (0   == c)
        || (EOF == c) )
    {
        nError = ERR_CHAR; 	// falsches Zeichen oder EOF
        return;
    }
    if( '1' == c ) 	// ignorieren, da Absatzabstaende interpretiert werden
        return;

    bIsTxtInPara = FALSE;

         // Abfrage vorsichtshalber
    if( ( bStyleOff || bTxtInDoc ) && !bStyleDef )
    {
        /* nach Handbuch W4W schaltet HNL
         * CTX - Centered Text mit aus
         * BRJ - Blockfatz bleibt an !!!
         */
        const SwPosition& rPos = *pCurPaM->GetPoint();
        if ( !bPersAlign )
            pCtrlStck->SetAttr( rPos, RES_PARATR_ADJUST );

        pCtrlStck->SetAttr( rPos, RES_UL_SPACE ); //??? Temp. Margins auch ?
        pCtrlStck->SetAttr( rPos, RES_PARATR_SPLIT );
//			pCtrlStck->SetAttr( pCurPaM, RES_PARATR_TABSTOP ); //??? Temp. Margins auch ?
                    // nicht ganz schoen, aber ich weiss nichts besseres
//			pCtrlStck->SetAttr( pCurPaM, RES_CHRATR_FONT ); //??? Temp. Margins auch ?
//			pCtrlStck->SetAttr( pCurPaM, RES_CHRATR_FONTSIZE ); //??? Temp. Margins auch ?


        // ggfs. obere/unterer Raender setzen
        UpdatePageMarginSettings( CALLED_BY_FLUSH_OR_HNL );

        // Beende Definition von nur fuer diesen Node gueltigem Rand
        if( bSingleParaMgn )
            pCtrlStck->SetAttr( rPos, RES_LR_SPACE );

        /*
            Jetzt neuen Node erzeugen:
        */
        DoSplit();

        // wenn RSM offen, setze Rand fuer neuen Absatz wieder genauso
        if(    bSingleParaMgn
            && (    nRSMwpwParaLeft
                 || (    nRSMwpwParaRight
                      && ( nRSMwpwParaRight != nPgRight ) ) ) )
        {
            Read_IndentPara1( nRSMwpwParaLeft, nRSMwpwParaLeft,
                              nRSMwpwParaRight );
            bSingleParaMgn = FALSE;
        }

        if( bIsColMode && !bIsSTYInTab )
        {
            // wenn kein Style hart gesetzt ist
            // Override Style "Tabellenkopf", "Tabelleninhalt"
            // mit vorher gueltigem Style
            const SwPosition& rPos = *pCurPaM->GetPoint();
            pCtrlStck->NewAttr( rPos,  SwW4WStyle( nTabStyleId ) );
            pCtrlStck->SetAttr( rPos, RES_FLTR_STYLESHEET );
        }
        ActivateTxtFlags();

//		return;

        // zusaetzliche Massnahmen beim Style-Ausschalten
        if( bStyleOff )
        {
            SwCntntNode* pCNd = pCurPaM->GetCntntNode();
            nAktStyleId = 0;
            if( pCNd->IsTxtNode() )
            {
                SwTxtFmtColl* pAktColl = GetAktColl();
                if( !pAktColl )
                {
//	  				  pAktColl = (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl();
                    pAktColl = pDoc->GetTxtCollFromPoolSimple
                        ( RES_POOLCOLL_STANDARD, FALSE );
                }
                pDoc->SetTxtFmtColl( *pCurPaM, pAktColl, FALSE );
            }
            else
            {
                // in den anderen Content-Nodes nur das Frame-Format setzen.
                pCNd->ChgFmtColl( (SwGrfFmtColl*)pDoc->GetDfltGrfFmtColl() );
            }
            bStyleOff = FALSE;
        }
    }
}


void SwW4WParser::Read_HardNewPage()		  // (HNP)
{
    if(    bHeadFootDef // it's Nonsense to embedd PageBreaks into Header/Footer
        || bFootnoteDef	// (smart Winword *does* such nonsense)
        || bCheckTabAppendMode )
    {
        return;
    }

    if( bStyleDef )
    {
        pDoc->Insert( *pCurPaM, SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ) );
    }
    else
    {
        if( bIsTxtInPara )	// Pagebreak / PgDesc needs an *new* node
            DoSplit();

        if( pPageDescForNextHNP )
        {
//		pDoc->Insert( *pCurPaM, SwFmtPageDesc( &pDoc->GetPageDesc( 3 )));
            pDoc->Insert( *pCurPaM,
//						  SwFmtPageDesc( &pDoc->GetPageDesc( nAktPgDesc )));
                          SwFmtPageDesc( pPageDescForNextHNP ));
            pPageDescForNextHNP = 0;
            if ( bPgMgnChanged ) SetPageMgn();
        }
        else
        {
            if ( bPgMgnChanged )	// Raender wurden geaendert
            {                       // ( beim letzten RSM )
                if( bIsTxtInPgDesc )
                    CreatePageDesc( CRPGD_AND_INSERT );
                SetPageMgn();
            }
            if(    bIsTxtInPgDesc
                || bWasHNPInPgDesc )
            {  	// alter PgDesc mit Text oder mindestens 1 .<HNP> -> PageBreak
                pDoc->Insert( *pCurPaM, SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ) );
                bWasTxtSince_BREAK_PAGE = FALSE;
            }
        }
        bWasHNPInPgDesc = TRUE;
        bWasFootNoteOnW4WPage = FALSE;
    }
}




void SwW4WParser::Read_SoftNewPage()		  // (SNP)
{
    if( !bStyleDef )
    {
        bWasFootNoteOnW4WPage = FALSE;
    }
}



void SwW4WParser::Read_NewLineInPara()		  // (NLN)
{
    FlushChar( '\x0a' );
    bIsNLN = TRUE;
}


void SwW4WParser::Read_DateTime()			// (DTF)
{
    BYTE nSep, nFormat;
    String sDate, sTime, *pFirst = 0, *pLast = 0;
    for( int n = 0;
            n < 15 &&
            W4WR_TXTERM == GetHexByte( nFormat ) && !nError && nFormat &&
               W4WR_TXTERM == GetHexByte( nSep ) && !nError; ++n )
    {
        const char* pDAdd = 0, *pTAdd = 0, *pAdd;
        String* pStr;
        switch( nFormat )
        {
        case 0x30:	pTAdd = "AM/PM";	break;
        case 0x31:	pDAdd = "TT";		break;
        case 0x32:	pDAdd = "MM";		break;
        case 0x33:	pDAdd = "MMMM";		break;
        case 0x34:	pDAdd = "JJJJ";		break;
        case 0x35:	pDAdd = "JJ";		break;
        case 0x36:	pDAdd = "NNNN";		break;
        case 0x37:	pTAdd = "HH";		break;
        case 0x38:	pTAdd = "HH";		break;
        case 0x39:	pTAdd = "MM";		break;
        }

        if( 0 != ( pAdd = pTAdd ))
            pStr = &sTime;
        else if( 0 != ( pAdd = pDAdd ))
            pStr = &sDate;
        else
            continue;

        if( !pFirst )
            pFirst = pStr;
        else if( !pLast && pFirst != pStr )
            pLast = pStr;
        pStr->AppendAscii( pAdd );
        if( nSep )
            *pStr += static_cast< char >(nSep);
    }

    if( pFirst )
    {
        // get Doc Numberformatter
        SvNumberFormatter* pFormatter = pDoc->GetNumberFormatter();

//		ULONG nLang = ((SvxLanguageItem&)(pDoc->GetAttrPool().
//			   			GetDefaultItem( RES_CHRATR_LANGUAGE ))).GetValue();

        // tell the Formatter about the new entry
        UINT16 nCheckPos = 0;
        INT16  nType = NUMBERFORMAT_DEFINED;
        sal_uInt32  nKey;

        pFormatter->PutandConvertEntry( *pFirst, nCheckPos, nType, nKey,
                                        LANGUAGE_GERMAN,
                                        /*nLang*/LANGUAGE_SYSTEM );
        SwDateTimeField aFld( (SwDateTimeFieldType*)
                        pDoc->GetSysFldType( RES_DATETIMEFLD ),
                        &sTime == pFirst ? TIMEFLD : DATEFLD, nKey );
        pDoc->Insert( *pCurPaM, SwFmtFld( aFld ) );

        if( pLast )
        {
            nCheckPos = 0;
            nType = NUMBERFORMAT_DEFINED;

            pFormatter->PutandConvertEntry( *pLast, nCheckPos, nType, nKey,
                                            LANGUAGE_GERMAN,
                                            /*nLang*/LANGUAGE_SYSTEM );
            SwDateTimeField aFld( (SwDateTimeFieldType*)
                        pDoc->GetSysFldType( RES_DATETIMEFLD ),
                        &sTime == pLast ? TIMEFLD : DATEFLD, nKey );
            pDoc->Insert( *pCurPaM, SwFmtFld( aFld ) );
        }
    }
}


void SwW4WParser::Read_Tab()				// (TAB), (DTB)
{
    if( bStyleOnOff )
        return;

    /*
        ersten, fuehrenden TAB innerhalb einer Fussnote uebergehen
    */
    if( bFootnoteDef && !bIsTxtInFNote )
    {
        bIsTxtInFNote = TRUE;
        return;
    }

    long nValue;

    if( W4WR_TXTERM != GetDecimal( nValue ) || nError
        || nValue == 0 )		// real Tab (no Filler)
    {
        FlushChar( '\t' );
    }
}



void SwW4WParser::Read_HardSpace()			// (HSP)
{
    // erstmal so, spaeter das entsprechende Attribute setzen
    FlushChar( ' ' );
}



// -------- Methoden fuer die Attribut-Verarbeitung: ------------

// SwW4WParser::GetFmtAttr ist
// statt pCtrlStck->GetFmtAttr( *pCurPaM, RES_LR_SPACE )); zu benutzen, wenn
// das Attribut auch waehrend der Styledefinition wiedergefunden werden soll

const SfxPoolItem* SwW4WParser::GetFmtAttr( USHORT nWhich )
{
    if( bStyleDef )
    {
        SwTxtFmtColl* pAktColl = GetAktColl();
        ASSERT( pAktColl, "StyleId ungueltig" );

        return &pAktColl->GetAttr( nWhich );
    }
    else
        return pCtrlStck->GetFmtAttr( *pCurPaM, nWhich );
}

void SwW4WParser::SetAttr( const SfxPoolItem& rAttr )
{
    if( bStyleDef )
    {
        BOOL bSetAttr;
        SwTxtFmtColl* pAktColl = GetAktColl( &bSetAttr );
        ASSERT( pAktColl, "StyleId ungueltig" );
        if( bSetAttr )
            pAktColl->SetAttr( rAttr );
    }
    else
        pCtrlStck->NewAttr( *pCurPaM->GetPoint(), rAttr );
}

// SetAttrOff setzt ein Attribut in den Attributstack. Ob ein Anfang oder
// ein Ende gemeint ist ( z.B. ITF ), wird daran zu erkennen versucht,
// ob noch ein angefangenes Attribut auf dem Stack haengt.
//
// Wenn Styledefinitionen passieren, ist diese Mimik ausgeschaltet:
// wenn bStyleOnOff TRUE ist, wird ein Attributende angenommen,
// d.h. das Attribut wird nicht gesetzt, da der Style das schon erledigt,
// es werden aber etwaige gleichartige harte Attribute beendet, um die
// Einstellungen im Style nicht gleich wieder per harter Attributierung
// ueberzumangeln.

void SwW4WParser::SetAttrOff( const SfxPoolItem& rAttr )
{
    if( bStyleOnOff )
    {
        Read_HardAttrOff( rAttr.Which() );
    }
    else if( bStyleDef )
    {
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), rAttr.Which() );
    }
    else
    {
        const SwPosition& rPos = *pCurPaM->GetPoint();
        if( !pCtrlStck->SetAttr( rPos, rAttr.Which() ) )
             pCtrlStck->NewAttr( rPos, rAttr );	// Das Attribut gab's noch
                                        // nicht auf den Stack, es muss wohl
                                        // ein aktives Ausschalten gemeint sein
    }
}


void SwW4WParser::Read_BeginBold()	// (BBT)
{
    if( !bStyleOnOff )
        SetAttr( SvxWeightItem( WEIGHT_BOLD ));
}


void SwW4WParser::Read_EndBold()	// (EBT)
{
    SetAttrOff( SvxWeightItem( WEIGHT_NORMAL ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_WEIGHT );
}


void SwW4WParser::Read_SetUnderlineMode()	  // (SUM)
{
    long nOldSUM, nSUM;

    if( W4WR_TXTERM != GetDecimal( nOldSUM ) ||
        W4WR_TXTERM != GetDecimal( nSUM ) ||
        nError )
        return;

    nUnderlineMode =  (BYTE)nSUM;			// 0 = single Word
                                            // 1 = double Word
                                            // 2,3 = Continuous ( default )
}

void SwW4WParser::Read_BeginUnderline() 	// (BUL)
{
    if( !bStyleOnOff )
    {
        SetAttr( SvxUnderlineItem( UNDERLINE_SINGLE ));
        if ( nUnderlineMode < 2 )
            SetAttr( SvxWordLineModeItem( TRUE ));
    }
}


void SwW4WParser::Read_EndUnderline()		// (EUL)
{
    SetAttrOff( SvxUnderlineItem( UNDERLINE_NONE ));
    SetAttrOff( SvxWordLineModeItem( FALSE ));
}


void SwW4WParser::Read_BeginDoubleUnderline()	// (BDU)
{
    if( !bStyleOnOff ){
        SetAttr( SvxUnderlineItem( UNDERLINE_DOUBLE ));
        if ( nUnderlineMode < 2 )
            SetAttr( SvxWordLineModeItem( TRUE ));
    }
}


void SwW4WParser::Read_EndDoubleUnderline() 	// (EDU)
{
    SetAttrOff( SvxUnderlineItem( UNDERLINE_NONE ));
    SetAttrOff( SvxWordLineModeItem( FALSE ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_UNDERLINE );
}


void SwW4WParser::Read_BeginItalic()	// (ITO)
{
    if( !bStyleOnOff )
        SetAttr( SvxPostureItem( ITALIC_NORMAL ));
}


void SwW4WParser::Read_EndItalic()		// (ITF)
{
    SetAttrOff( SvxPostureItem( ITALIC_NONE ));
}


void SwW4WParser::Read_BeginSuperScript()	// (SPS)
{
    if( !bStyleOnOff )
        SetAttr( SvxEscapementItem( SVX_ESCAPEMENT_SUPERSCRIPT ));
}


void SwW4WParser::Read_EndSuperScript() 	// (EPS)
{
    SetAttrOff( SvxEscapementItem( SVX_ESCAPEMENT_OFF ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
}


void SwW4WParser::Read_BeginSubScript() 	// (SBS)
{
    if( !bStyleOnOff )
        SetAttr( SvxEscapementItem( SVX_ESCAPEMENT_SUBSCRIPT ));
}


void SwW4WParser::Read_EndSubScript()		// (EBS)
{
    SetAttrOff( SvxEscapementItem( SVX_ESCAPEMENT_OFF ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_ESCAPEMENT );
}


void SwW4WParser::Read_BeginColoredText()	 // (BCL)
{
    if( !bStyleOnOff )						// evtl muessen noch die Bits in
                                            //	nCol abgefragt werden
    {
        long nCol, nRed, nGreen, nBlue;
        if( GetDecimal( nCol ) && !nError && GetDecimal( nRed ) && !nError
            && GetDecimal( nGreen ) && !nError && GetDecimal( nBlue )
            && !nError )
            SetAttr( SvxColorItem(
                    Color( (BYTE)nRed, (BYTE)nGreen, (BYTE)nBlue )));
    }
}

void SwW4WParser::Read_EndColoredText() 	// (ECL)
{
    SetAttrOff( SvxColorItem( Color( 0, 0, 0 ) ) );
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_COLOR );
}


void SwW4WParser::Read_BeginStrikeOut() 	// (BSO)
{
    if( !bStyleOnOff )
    {
        BYTE c;
        c = ReadChar();
        if( rInp.IsEof() )
        {
            nError = ERR_RECORD;
            return;
        }
        FontStrikeout eStrike = STRIKEOUT_SINGLE;
        if( c > ' ' && c < '~' ){
            switch( c ){
            case ' ': eStrike = STRIKEOUT_NONE; break;
            case '-':
            case '_': eStrike = STRIKEOUT_SINGLE; break;
            default:  eStrike = STRIKEOUT_DOUBLE; break;
            }
        }
        SetAttr( SvxCrossedOutItem( eStrike ) );
    }
}


void SwW4WParser::Read_EndStrikeOut()	// (ESO)
{
    SetAttrOff( SvxCrossedOutItem( STRIKEOUT_NONE ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_CROSSEDOUT );
}


void SwW4WParser::Read_BeginShadow()	// (BSP)
{
    if( !bStyleOnOff )
        SetAttr( SvxShadowedItem( TRUE ));
}


void SwW4WParser::Read_EndShadow()		// (ESP)
{
    SetAttrOff( SvxShadowedItem( FALSE ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_SHADOWED );
}


// Read_HardAttrOff beendet die harten Attribute.
// muss fuer jedes Attribut gerufen werden, das beim Style-einschalten
// oder Style-ausschalten enthalten ist.
// Wenn beim Einschalten der Pam am Anfang einer Zeile steht ( trifft
// meistens beim Einschalten des Styles zu ) und es kein Charformat ist,
// wird das Attributende am Ende des letzten Absatzes gesetzt,
// da sonst das harte Attribut eine Zeile zuweit gueltig waere.

void SwW4WParser::Read_HardAttrOff( USHORT nResId )
{

    BOOL bForward = FALSE;
    BOOL bBack = bStyleOn && pCtrlStck->IsAttrOpen( nResId )
                 && (RES_CHRATR_BEGIN > nResId || nResId >= RES_CHRATR_END )
                 && pCurPaM->GetPoint()->nContent.GetIndex() == 0;

    if( bBack )
        bForward = pCurPaM->Move( fnMoveBackward, fnGoCntnt );

    pCtrlStck->SetAttr( *pCurPaM->GetPoint(), nResId, TRUE, bForward );

    if( bForward )
        pCurPaM->Move( fnMoveForward, fnGoCntnt );
}


void SwW4WParser::Read_SetPageNumber()		  // (SPN)
{							// Der Befehl wird nicht ausgewertet. Stattdessen
    if ( nDocType == 44 )	// wird er bei WW2 dafuer benutzt, das Ende der
        bPageDefRdy = TRUE; // Seitenbeschreibung zu erkennen.
}


struct TabT{
    long nPos;
    BYTE cType;
    BYTE cLead;
};


void SwW4WParser::Read_Ruler()		  // (RUL)
{
    long nDefByte, nNumTabs;
    long i;

    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_PARATR_TABSTOP );
        return;
    }

    if ( !bPageDefRdy ) return;   // auch nicht global setzen

    if( W4WR_TXTERM != GetDecimal( nDefByte ) || nError
        || W4WR_TXTERM != GetDecimal( nNumTabs ) || nError
        || nNumTabs > 100 )
        return;

    TabT *pTab = new TabT[nNumTabs];

    for (i=0; i<nNumTabs; i++){ 	// lies Positionen
        if( W4WR_TXTERM != GetDecimal( pTab[i].nPos ) || nError ){
            DELETEZ( pTab );
            return;
        }
    }

    BOOL bOpt = TRUE;

    for (i=0; i<nNumTabs; i++)	// lies optionale Typen
    {
        if ( bOpt )
        {
            if( W4WR_TXTERM != GetChar( pTab[i].cType ) || nError )
            {
                pTab[i].cType = 'L';    // Opt. Parameter fehlen
                bOpt = FALSE;
            }
        }
        else
        {	// Opt. Parameter nicht da -> Default links
            pTab[i].cType = 'L';
        }
    }

    for (i=0; i<nNumTabs; i++){ 	// lies optionale Leadings
        if ( bOpt ){
            if( W4WR_TXTERM != GetChar( pTab[i].cLead ) || nError ){
                pTab[i].cLead = cDfltFillChar;	// Opt. Parameter fehlen
                bOpt = FALSE;
            }
        }else{			// Opt. Parameter nicht da -> Default kein Leading
            pTab[i].cLead = cDfltFillChar;
        }
    }

    if ( nDocType == 44 || nDocType == 49 )
    {	// Default-Tabs von WinWord erkennen
        for (i=nNumTabs-1; i>=0; i--)	// von hinten nach vorne durchsuchen
        {
            if ( pTab[i].cType == 'L' )    // nur L kann Default sein
            {
                USHORT nPos = (USHORT)(pTab[i].nPos);
                if ( nDefByte & 1 ) 	 // Abstand zum l. Seiterand
                    nPos -= nPgLeft;
                if ((nPos % 708 == 0)||(nPos % 709 == 0)){	// std-Einstellung
                    pTab[i].cType = 'l';                    // Default-Tab
                    if ( i>0 && pTab[i].nPos - pTab[i-1].nPos < 50 )
                        pTab[i].cType = 'V';            // falscher Default-Tab
                }
                else break; 				// kein Def-Tab -> alle davor auch nicht
            }
            else break;					// dito
        }
    }

    SvxTabStopItem aTabStops( 0, 0 );
    SvxTabStop aTabStop;
//	BOOL bZeroTab = FALSE;
    long nLeftMargin = GetLeftMargin(); //!! geht anders schneller

    for (i=0; i<nNumTabs; i++)	// Erzeuge Attribut
    {
        // SeitenRand abziehen ?
        if ( nDefByte & 1 )
            aTabStop.GetTabPos() = pTab[i].nPos - nLeftMargin;
        else
            aTabStop.GetTabPos() = pTab[i].nPos;

        // Bestimme die Art des Tabulators
        switch( pTab[ i ].cType )
        {
            case 'l': aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DEFAULT; break; // eigene Codierung
            case 'L': aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT; break;
            case 'R': aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT; break;
            case 'C': aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER; break;
            case 'D': aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL; break;
            case 'V':
            {
                continue;	// Ignoriere virtuelle Tabs
            }
            default:  aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT; break;
        }

        aTabStop.GetFill() = pTab[i].cLead;

//		if ( aTabStop.GetTabPos() < 10 && aTabStop.GetTabPos() > -10 )
//			bZeroTab = TRUE;

        aTabStops.Insert( aTabStop );
    }

//!!! nicht mehr noetig ??? anscheinend doch !
//	if ( !bZeroTab )						// wenn neg. Erstzeeileneinzug benutzt wird,
//		aTabStops.Insert( SvxTabStop( 0, SVX_TAB_ADJUST_DEFAULT ) ); // muss bei Null ein Tab gesetzt werden, wenn

    // keine doppelten Tabs
    if( !bStyleDef )
        pCtrlStck->StealAttr( *pCurPaM->GetPoint(), RES_PARATR_TABSTOP );
    // jetzt noch das Attribut setzen (ueber Stack)
    SetAttr( aTabStops );

    DELETEZ( pTab );

    bIgnoreNTB = TRUE;	// wenn einmal RUL auftaucht, werden alle
                        // kommenden NTBs des Dokumentes ignoriert
    bIsTab = TRUE;
}


void SwW4WParser::Read_NewTabTable()		// (NTB)
{
    if ( bIgnoreNTB ) return;

    register BYTE c;

    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_PARATR_TABSTOP );
        return;
    }


    if ( !bPageDefRdy ) return;   // auch nicht global setzen

    BYTE aTabPosArr[ 32 ];
    BYTE nCnt;
    for( nCnt = 0; nCnt < 32; ++nCnt )
        if( W4WR_TXTERM != GetHexByte( aTabPosArr[ nCnt ] ))
        {
            nError = ERR_RECORD;
            return;
        }
    BYTE aTabTypesArr[ 20 ];
    for( nCnt = 0; nCnt < 20; ++nCnt )
        if( W4WR_TXTERM != GetHexByte( aTabTypesArr[ nCnt ] ))
        {
            nError = ERR_RECORD;
            return;
        }

    BOOL bTabLeadings = FALSE;
    BYTE aTabLeadingArr[ 40 ];
    long nTabTwipCnt = 0;
    BOOL bTabTwips =  FALSE;

    // TabLeadings sind optional !!
    c = ReadChar();
    if( !rInp.IsEof() && c != W4WR_RED )
    {
        bTabLeadings = TRUE;
        rInp.SeekRel( - 1 );
//		  pInput->putback( c );

        for( nCnt = 0; nCnt < 40; ++nCnt )
            if( W4WR_TXTERM != (c = GetHexByte( aTabLeadingArr[ nCnt ] )))
            {
                nError = ERR_RECORD;
                return;
            }

        // Tab-Twips-Positions sind optional !!
        // sind sie in Twips angegeben, lese sie erst beim Erzeugen der Tabs
        bTabTwips = W4WR_RED != (c = GetDecimal( nTabTwipCnt )) && !nError;
    }


    // ok, jetzt sind alle Daten vorhanden, also erzeuge sie:
    // 1. stelle fest, wieviele es ueberhaupt gibt:
    if( !bTabTwips )
    {
        nTabTwipCnt = 0;
        for( nCnt = 0; nCnt < 32; nCnt += 4 )
            nTabTwipCnt += GetBitCnt( (ULONG) aTabPosArr + nCnt );
    }

    long nLeftMargin = GetLeftMargin();


    SvxTabStopItem aTabStops( 0, 0 );
    USHORT nTabPos = 0;
    static SvxTabAdjust __READONLY_DATA aAdjustArr[ 4 ] =
        {	SVX_TAB_ADJUST_LEFT, SVX_TAB_ADJUST_CENTER,
            SVX_TAB_ADJUST_RIGHT, SVX_TAB_ADJUST_DECIMAL };
    USHORT nTmp;
    long nTabTwipPos = 0;
//	BOOL bZeroTab = FALSE;

    for( nCnt = 0; nCnt < 32; ++nCnt )
    {
        if( !aTabPosArr[ nCnt ] )
            continue;

        for( BYTE nMaske = 0x80, i = 0; nMaske; nMaske >>= 1, ++i )
            if( aTabPosArr[ nCnt ] & nMaske )
            {
                SvxTabStop aTabStop;
                if( bTabTwips )
                {
                    if( W4WR_TXTERM != (c = GetDecimal( nTabTwipPos )) || nError )
                    {
                        nError = ERR_RECORD;
                        return;
                    }
                    aTabStop.GetTabPos() = nTabTwipPos;
                }
                else
                    aTabStop.GetTabPos() = nColSize * ( nCnt * 8 + i );
                aTabStop.GetTabPos() -= nLeftMargin;	// SeitenRand abziehen

                BYTE nType = aTabTypesArr[ nTabPos / 2 ];

                nTmp = nTabPos & 1 ? nType & 0x0f : (nType & 0xf0) >> 4;
                if (nTmp <= 4){ 		// no virtual Tab
                    aTabStop.GetAdjustment() = aAdjustArr[nTmp];

                    if( bTabLeadings && aTabLeadingArr[ nTabPos ] )
                        aTabStop.GetFill() = aTabLeadingArr[ nTabPos ];

                    aTabStops.Insert( aTabStop );
//					if( aTabStop.GetTabPos() < 10 &&
//						aTabStop.GetTabPos() > -10 )
//						bZeroTab = TRUE;
                }
                ++nTabPos;
            }
    }
//	if ( !bZeroTab )						// wenn neg. Erstzeeileneinzug benutzt wird,
//		aTabStops.Insert( SvxTabStop( 0 ) ); // muss bei Null ein Tab gesetzt werden, wenn
                                            // sich hier noch keiner befindet.

    const SwPosition& rPos = *pCurPaM->GetPoint();
    if ( !bStyleDef )						// keine doppelten Tabs
        pCtrlStck->StealAttr( rPos, RES_PARATR_TABSTOP );
    pCtrlStck->SetAttr( rPos, RES_PARATR_TABSTOP );	// erst alte Tabs setzen
    SetAttr( aTabStops );			// jetzt noch das Attribut setzen (ueber Stack)
//	  pCtrlStck->SetAttr( rPos, RES_PARATR_TABSTOP );
}


void SwW4WParser::Read_BeginFlushRight()				// (AFR)
{
    long nAlign;
    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_PARATR_ADJUST );
        return;
    }
    if( GetDecimal( nAlign ) && !nError )
    {
        if( 10 == nAlign )
            SetAttr( SvxAdjustItem( SVX_ADJUST_RIGHT ));
        else if( 44 == nAlign || 46 == nAlign )
            FlushChar( '\t' );                         //!!! Was machen RechtsTabs ?
    }
}


void SwW4WParser::Read_EndFlushRight()					// (EAT)
{
    SetAttrOff( SvxAdjustItem( SVX_ADJUST_LEFT ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_PARATR_ADJUST );
}


void SwW4WParser::Read_BeginCenterText()				// (CTX)
{
    if( bStyleOnOff )
        // in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_PARATR_ADJUST );
    else
    {
        long nAlign;
        if( GetDecimal( nAlign ) && !nError )
        {
            if( 0 == nAlign )
                SetAttr( SvxAdjustItem( SVX_ADJUST_CENTER ));
            else
            if( 1 == nAlign )
                FlushChar( '\t' );
        }
    }
}


void SwW4WParser::Read_EndCenterText()					// (ECT)
{
    SetAttrOff( SvxAdjustItem( SVX_ADJUST_LEFT ));
//	  if( !bStyleOnOff )
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_PARATR_ADJUST );
}


void SwW4WParser::Read_BeginRightJustify()				// (BRJ)
{
    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_PARATR_ADJUST );
        return;
    }
    bPersAlign = TRUE;
    SetAttr( SvxAdjustItem( SVX_ADJUST_BLOCK ));
}


void SwW4WParser::Read_EndRightJustify()				// (ERJ)
{
    SetAttrOff( SvxAdjustItem( SVX_ADJUST_LEFT ));
    bPersAlign = FALSE;
//	  if( !bStyleOnOff ){
//		  bPersAlign = FALSE;
//		  pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_PARATR_ADJUST );
//	  }
}


void SwW4WParser::Read_PrintPageNo()					// (PPN)
{
                            //	Seitennummernfeld einsetzen:
    SwPageNumberField aPageFld( (SwPageNumberFieldType*)
                                pDoc->GetSysFldType( RES_PAGENUMBERFLD ) );
    aPageFld.ChangeFormat( SVX_NUM_ARABIC );
    pDoc->Insert( *pCurPaM, SwFmtFld( aPageFld ) );
}


void SwW4WParser::Read_PrintLastPageNo()				// (PLN)
{
                        // SeitenZahl - Feld
    SwDocStatField aPageFld( (SwDocStatFieldType*)
                                pDoc->GetSysFldType( RES_DOCSTATFLD ),
                              DS_PAGE, SVX_NUM_ARABIC );
    pDoc->Insert( *pCurPaM, SwFmtFld( aPageFld ));
}

// -------- Methoden fuer die Font-Verarbeitung: ------------


void SwW4WParser::Read_SetPitchAndOrFont()		// (SPF)
{
    if ( bStyleOnOff ){ // Style On /Off -> Harte Attributierung zuende
        const SwPosition& rPos = *pCurPaM->GetPoint();
        pCtrlStck->SetAttr( rPos, RES_CHRATR_FONT );
        pCtrlStck->SetAttr( rPos, RES_CHRATR_FONTSIZE );
        return;
    }

    BOOL bOldTxtInDoc = bTxtInDoc;
    bTxtInDoc = FALSE;

    long nOldPitch, nOldFont, nNewPitch, nNewFont, nSize;
    long nOptProp, nOptSerif;

                //	die von W4W angegeben alten Werte nOld...
                //	stimmen nicht immer, d.h.,
                //	man darf sich zur Optimierung der Aufrufe nicht
                //	darauf verlassen

                // Im Moment werden die Aufrufe nicht optimiert, d.h bei
                // jedem SetText-Record werden Pitch, SSize und Name neu
                // gesetzt

    Flush();			// Vor GetNextRecord() Puffer leeren
    if( GetDecimal( nOldPitch ) && !nError &&
        GetDecimal( nOldFont ) && !nError &&
        GetDecimal( nNewPitch ) && !nError &&
        GetDecimal( nNewFont ) && !nError &&
        GetDecimal( nSize ) && !nError &&
        EOF != GetNextRecord() )
    {
        if( W4WR_TXTERM != GetDecimal( nOptProp ) ||		// weitere Info zum Font ??
            W4WR_TXTERM != GetDecimal( nOptSerif ) || nError ){
            nOptProp = -1;								// keine weitere Info
            nOptSerif = -1;
        }

        Flush();							// '\0' hinter Fontnamen setzen
//		NOTE7( "[Font: (Old: %ld|%ld)(New: %ld|%ld) SSize: %ld, Name: (%s)",
//				nOldPitch, nOldFont, nNewPitch, nNewFont, nSize, aCharBuffer );

/* Nach ST den Pitch besser ignorieren, da er im SW immer fest mit der Fontgroesse
   verknuepft ist (und damit keine schmaleren Buchstaben moeglich sind)

        // erstmal das alte zuruecksetzen
        if( !bStyleDef )
            pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_CHRATR_CHARWIDTH );

        SetAttr( SwFmtCharWidth( Fraction( nNewPitch, 1 )));
*/

        if ( bNew )
        {

            // loesche alle fuehrende Blanks vom Namen
            String sFntNm( aCharBuffer );
            sFntNm.EraseLeadingChars().EraseTrailingChars();

            SvxFontItem aFont( ((SvxFontItem&)pDoc->GetAttrPool().GetDefaultItem(
                                RES_CHRATR_FONT ) ));

            FontFamily eFamily = FAMILY_DONTKNOW;

            if( sFntNm.Len() )
            {
#if defined( WIN ) || defined ( WNT )
                                    // ersetze alte Fonts durch neue AEquivalente
                if ( sFntNm.EqualsAscii(  "Helv" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Arial" ));
                else if ( sFntNm.EqualsAscii( "Tms Rmn" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Times New Roman" ));
                else if ( sFntNm.EqualsAscii( "Courier" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Courier New" ));
#endif // WIN
#if defined( OS2 )
                if ( sFntNm.EqualsAscii( "Helv" ) ||
                     sFntNm.EqualsAscii( "Arial" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Helvetica" ));
                else if( sFntNm.EqualsAscii( "Tms Rmn" )
                      || sFntNm.EqualsAscii( "Times New Roman" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Times New Roman" ));
                else if ( sFntNm.EqualsAscii( "Courier New" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Courier" ));
#endif // OS2
#if defined( MAC )
                if ( sFntNm.EqualsAscii( "Helv" ) ||
                     sFntNm.EqualsAscii( "Arial" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Helvetica" ));
                else if( sFntNm.EqualsAscii( "Tms Rmn" )
                      || sFntNm.EqualsAscii( "Times New Roman" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Times" ));
                else if ( sFntNm.EqualsAscii( "Courier New" ))
                    sFntNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "Courier" ));
#endif // MAC

                if ( sFntNm.EqualsAscii( "Script" ))
                    eFamily = FAMILY_SCRIPT;
            }

            switch( nOptProp )
            {
            case 0:
                aFont.GetPitch() = PITCH_FIXED;
                aFont.GetFamily() = FAMILY_MODERN;
                break;

            case 1:
                aFont.GetPitch() = PITCH_VARIABLE;
                if( !nOptSerif )
                    aFont.GetFamily() = FAMILY_SWISS;
                else if( 1 == nOptSerif )
                    aFont.GetFamily() = FAMILY_ROMAN;
                break;
            }

            SvxFontHeightItem aSz( (const ULONG)nSize );
            if ( !bPageDefRdy )			// Setze Defaults fuer ganzes Dokument
            {
                pDoc->SetDefault( aFont );		// Font ignorieren, da W4W WW2-Filter
                                                // faelschlicherweise am Anfang
                                                // immer Courier setzt ??

                pDoc->SetDefault( aSz ); // Groesse setzen

            }
            else
            {
                SetAttr( aFont );		// neuer Font
                SetAttr( aSz ); 		// in Twips
            }
        }
        bDefFontSet = TRUE;
    }
    bTxtInDoc = bOldTxtInDoc;
}

#ifdef NIE


void SwW4WParser::Read_FontDefTable()		// (FDT)
{											//!!! Ist m.E. ueberfluessig, SPF tut's besser
    BOOL bOldTxtInDoc = bTxtInDoc;
    bTxtInDoc = FALSE;

    long nCntFnts, nFntFlds;
    BYTE nDummy;
    int cRet;
    if( GetDecimal( nCntFnts ) && !nError &&
        GetDecimal( nFntFlds ) && !nError &&
        nCntFnts )
    {
        if( W4WR_TXTERM != SkipPara() ) 	// Dummy Feld einlesen
        {									// am Ende vom Record ?
            nError = ERR_RECORD;			// darf nicht sein !
            return;
        }

//		NOTE3( "[ FontTab: (Sz: %ld, FldSz: %ld )", nCntFnts, nFntFlds );

        long nFntId;
        for( USHORT n = 0; n < nCntFnts; ++n )
        {
            // lese jetzt die Fontdefinitionen:
            if( GetDecimal( nFntId ) && !nError &&
                EOF != ( cRet = GetNextRecord()) && !nError &&
                W4WR_TXTERM == cRet )
            {
                for( USHORT nCnt = 2; nCnt < nFntFlds && !nError; ++nCnt )
                    if( W4WR_TXTERM != SkipPara() ) // am Ende vom Record ?
                    {
                        nError = ERR_RECORD;		// darf nicht sein !
                        return;
                    }
                Flush();
//				NOTE3( "(Id: %ld, (%s) )", nFntId, aCharBuffer );
            }
        }
//		NOTE1( "]" );
    }
    bTxtInDoc = bOldTxtInDoc;
}

#endif // NIE


/***********************************************************
************ Methoden fuer die Formatierung: ***************
***********************************************************/

void SwW4WParser::Read_SetFormLenght()		// (SFL)
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    long nNewHeight;

#ifdef MOGEL_WW6
    if ( nDocType == 49 ) return;	// WW6 : PGW, SFL macht Mist
#endif

    // FrameMaker: Seitengroesse ist oft erst nach FLO, PDT und RDT definiert.
    if( pPDTInfo )
    {
        if( pPDTInfo->PageSizeKnown() )
        {
            // PgDesc ist bereits aktiv, aber noch nicht vollstaendig bemasst

        }
    }


    if ( bIsTxtInPgDesc && nDocType != 15 )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    if( nError ||
        W4WR_TXTERM != SkipPara() || W4WR_TXTERM != SkipPara() ||
        W4WR_TXTERM != GetDecimal( nNewHeight ) || nError )
        return;

    // Seitenhoehe einstellen
    SwFrmFmt &rFmt = pPageDesc->GetMaster();
    SwFmtFrmSize aSz( rFmt.GetFrmSize() );

    long nOptNew;		// gibt es die optionale Angabe ?

    if ( nDocType == 1 )	// Ascii -> zwangsweise A4 ( statt letter )
        return;

    if( W4WR_TXTERM != SkipPara() ||
                    //!! unterer Rand wird bislang ignoriert
        W4WR_TXTERM != GetDecimal( nOptNew ) || nError )
        nOptNew = nNewHeight * 240; 	// Angabe nur in 1/6 Inch

    if (( nOptNew < lA4Height + 240 ) && ( nOptNew > lA4Height - 240 ))
        nOptNew = lA4Height;	// fast A4 -> genau A4

    aSz.SetHeight( nOptNew );
    rFmt.SetAttr( aSz );
    bSetPgHeight = TRUE;
}


void SwW4WParser::Read_PageWidth()					// (PGW)
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    long nNewWidth;

#ifdef MOGEL_WW6
    if ( nDocType == 49 ) return;	// WW6 : PGW, SFL macht Mist
#endif

    // nicht bei PC Text 4 und nicht bei FrameMaker
    if ( bIsTxtInPgDesc && (nDocType != 15) && !pPDTInfos )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    if( nError || W4WR_TXTERM != GetDecimal( nNewWidth ) || nError )
        return;

    // Seitenbreite einstellen
    SwFrmFmt &rFmt = pPageDesc->GetMaster();
    SwFmtFrmSize aSz( rFmt.GetFrmSize() );

    long nOptNew;		// gibt es die optionale Angabe ?
    if( W4WR_TXTERM != GetDecimal( nOptNew ) || nError )
        nOptNew = nNewWidth * 144;		// in 1/10 Inch statt in twips

    if (( nOptNew < lA4Width + 240 ) && ( nOptNew > lA4Width - 240 ))
        nOptNew = lA4Width; 	// fast A4 -> genau A4

    aSz.SetWidth( nOptNew );			  // in Twip
    rFmt.SetAttr( aSz );
    nPgWidth = USHORT(nOptNew);

    if( pPDTInfos )
    {
        if( pActRectInfo )
        {
            const SvxLRSpaceItem& rPageLR = rFmt.GetLRSpace();
            const SwRect& rRect = pActRectInfo->aRect;
            USHORT nMyLeft  = rPageLR.GetLeft();
            USHORT nMyRight = abs( nOptNew - rRect.Width() - nMyLeft );

            rFmt.SetAttr( SvxLRSpaceItem( nMyLeft, nMyRight ) );

        }
    }
    else
    {
        UpdateCacheVars();		// neuer rechter Rand

        bSetPgWidth = TRUE;
        if ( nDocType != 44 )	// WinWord: alle Attribute vor
            bPageDefRdy = TRUE;	//          Styledefinitionen vergessen
    }
}


void SwW4WParser::SetPageMgn()	// Seitenraender einstellen nach nRSM...
{
    SwFrmFmt &rFmt = pPageDesc->GetMaster();
    long nNewRight = rFmt.GetFrmSize().GetWidth() - nRSMRight;
    if ( nNewRight < 0 )
        nNewRight = 0;
    SvxLRSpaceItem aLR;
    aLR.SetTxtLeft( USHORT(nRSMLeft) );
    aLR.SetRight( USHORT(nNewRight) );
    rFmt.SetAttr( aLR );

    UpdateCacheVars();

    bPgMgnChanged = FALSE;
}


    // bei WPWin 6,7,8 setzt W4W ein RSM, um einen Absatz(!) links
    // einzuruecken - aehnlich <IPS> behandeln! ( Karl-Heinz Zimmer )

void SwW4WParser::Read_ReSetLeftRightMargin()		// (RSM)
{
    long nNewLeft, nNewRight;

    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_LR_SPACE );
        return;
    }
    if ( bIsColMode )	// ignoriere RSM in Tabelle ( W4W-WW2-Bug )
        return;

    if( nError || W4WR_TXTERM != SkipPara() || W4WR_TXTERM != SkipPara() ||
        W4WR_TXTERM != GetDecimal( nNewLeft ) ||
        W4WR_TXTERM != GetDecimal( nNewRight ) ||
        nError )
        return;

    long nOptNew;		// gibt es die optionale Angabe ?
    BOOL bOption = W4WR_TXTERM == GetDecimal( nOptNew );
    if( bOption && !nError )
        nNewLeft = nOptNew; 			// in Twip
    else
        nNewLeft *= 144;		// in 1/10 Inch, *nicht* nColSize;

    // gibt es die optionale Angabe ?
    if( bOption && W4WR_TXTERM == GetDecimal( nOptNew ) && !nError )
        nNewRight = nOptNew;			// in Twip
    else
        nNewRight *= 144;		// in 1/10 Inch, *nicht* nColSize;

    if( !bStyleDef )
    {
        if( (1800 == nNewLeft) && (44 == nDocType) // Mogelei gegen WinWord-Bug
            && ( !bWWWasRSM )
            && ( SvxLocaleToLanguage( GetAppLocaleData().getLocale() ) // Der Mac,
                == LANGUAGE_GERMAN )                              // der spinnt
            )
        {
            nNewLeft     = 1417;
            nLeftMgnCorr = 1800 - (USHORT)nNewLeft;
            nNewRight    = 10823;
            bWWWasRSM    = TRUE;
        }
        else
        {
            nLeftMgnCorr = 0;
        }
    }

    // Zahlenueberlauf oder andere W4W-Fehler
    if (   ( nNewLeft < 0 )
        || ( nNewLeft >= (long)nPgWidth - nColSize )
        || (    ( nNewLeft < nRSMLeft )
                // Sonderbehandlung fuer WordPerfekt f. Win 6,7,8
             && ( 48 == nDocType )
             && (    pCurPaM->GetPoint()->nNode.GetIndex()
                  >= pDoc->GetNodes().GetEndOfContent().
                                StartOfSectionIndex()+2
                )
           )
       )
    {
        nNewLeft = 0;
    }
    if( nNewLeft >= nNewRight - nColSize )	{	// Zahlenueberlauf oder andere W4W-Fehler
        nNewLeft = 0;
    }
    if( nNewLeft >= nNewRight - nColSize )		// dito
        nNewRight = (long)nPgWidth - nColSize;
    if( nNewRight > (long)nPgWidth )
        nNewRight = nPgWidth;

    if( 1 == nDocType && nNewLeft < 850 )	// ASCII-Reader mit falschem LRand
        return;

    if(     bStyleDef                       // behandle RSM in StyleDef wie IPS/STM
                // WordPerfekt f. Win 6,7,8
         || (    ( 48 == nDocType )
              && ( bWPWWasRSM )
              && (    ( 0 == nNewLeft )
                   || ( nNewLeft >= nRSMLeft )
                   || (    pCurPaM->GetPoint()->nNode.GetIndex()
                        >= pDoc->GetNodes().GetEndOfContent().
                                 StartOfSectionIndex()+2
                      )
                 )
      )     )
    {
        if( bStyleDef )
          Read_IndentPara1( nNewLeft, nNewLeft, nNewRight);
        else
        {
            nRSMwpwParaLeft  = nNewLeft;
            nRSMwpwParaRight = nNewRight;
        }
    }
    else
    {
        if( !bStyleDef )
            bWPWWasRSM = TRUE;	// fuer WPWin - Sonderbehandlung

        nRSMLeft  = nNewLeft;
        nRSMRight = nNewRight;
        bPgMgnChanged = TRUE;

        if( !bIsTxtInPgDesc )
            SetPageMgn();
    }
}
            // kam im selben Absatz bereits ein IPS, dann sollte ein			//
            // folgendes STM ignoriert werden, da STM den
            // Erstzeileneinzug zuruecksetzt. ( Karl Forster )
            //
            // aber: bei WPWin 6,7,8 setzt W4W ein STM, um einen
            // TAB anzuzeigen, der nicht als erstes Zeichen in der Zeile
            // steht. dann wie .<TAB> behandeln! ( Karl-Heinz Zimmer )

void SwW4WParser::Read_SetTempLeftRightMargin() 		// (STM)
{
    long nCurrPos, nFinalPos;


    /*
        erstmal alle Parameter lesen, aber noch nicht setzen...
    */
    if( W4WR_TXTERM != GetDecimal( nCurrPos )  ||
        W4WR_TXTERM != GetDecimal( nFinalPos ) || nError ) return;

    // ggfs. die optionalen Angaben lesen
    long nOptCurr, nOptFinal;
    if( W4WR_TXTERM == GetDecimal( nOptCurr ) &&
        W4WR_TXTERM == GetDecimal( nOptFinal ) && !nError )
    {
        nCurrPos  = nOptCurr; 		// in Twip
        nFinalPos = nOptFinal; 		// in Twip
    }
    else
    {
        nCurrPos  *= nColSize;		// in 1/10 Inch / ??
        nFinalPos *= nColSize;		// in 1/10 Inch / ??
    }

    // Parameter korrigieren
    if( ( 33 != nDocType ) || !bStyleDef )
    {
        nCurrPos  -= nPgLeft;
        nFinalPos -= nPgLeft;
    }
        // die Angaben gelten ab Seitenrand !!
        // ausser bei Ami in der StyleDef
        // Bei AmiPro kommt vor der Styledefinition kein RSM vor,
        // in den Style beziehen sich Einrueckungen auf einen
        // linken Seitenrand von 0

    if( nCurrPos  < 0 ) nCurrPos  = 0;
    if( nFinalPos < 0 ) nFinalPos = 0;


    /*
        eigentliche Aktionen, je nach Situation...
    */
    if( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_LR_SPACE );
        return;
    }

    // Sonderbehandlung fuer WordPerfekt f. Win 6,7,8
    if( ( 48 == nDocType ) && !bStyleDef )
    {
        if( bIsNumListPara )
        {	/*
                Bei num. Listen in WP folgt auf .<PGN> - .<EPN> ein
                .<STM> welches die Einrueckung des Listenabsatzes regelt.
                Hier gelten nCurrPos als linker Absatz-Einzug
                und nFinalPos als Erstzeileneinzug.
            */

            // loesche ggfs. altes Attr. vom Stack
            pCtrlStck->StealAttr( *pCurPaM->GetPoint(), RES_LR_SPACE );

            SwNumFmt aMyNumFmt( pActNumRule->Get( nActNumLevel ) );

            aMyNumFmt.SetAbsLSpace(       (USHORT)nFinalPos );
            aMyNumFmt.SetFirstLineOffset( (short)(nCurrPos - nFinalPos) );

            pActNumRule->Set( nActNumLevel, aMyNumFmt );

            return;
        }
        else
        if( pCurPaM->GetPoint()->nContent.GetIndex() )
        {
            FlushChar( '\t' );
            return;
        }
    }

    if( bIsIpsInPara ) return;

    if( bIsColMode && pTabDefs )
    {	// Tabelle
        long nBoxSize = pTabDefs[nTabCol].nRightTw
                        - pTabDefs[nTabCol].nLeftTw;
        if( nFinalPos + HOR_SICHER > nBoxSize / 2 )	//!!! RMargin ??
            nFinalPos = 0; // Stuss-Werte -> keine Einrueckung
    }
    /*
    else
    if( pCurPaM->GetPoint()->nContent.GetIndex() )
    {
        FlushChar( '\t' );
        return;
    }
    */

    /*
        Hier nun tatsaechlich die harte Einrueckung...
    */
    SvxLRSpaceItem aLR( *(SvxLRSpaceItem*) GetFmtAttr( RES_LR_SPACE ));

    // Sonderbehandlung bei PC-Text-4
    if(    ( nDocType == 15 )
        && (    nChrCnt
             || PtNd( pCurPaM ).GetNode().GetTxtNode()->GetTxt().Len()
           )
        && !( nIniFlags & W4WFL_NO_PCTEXT4_INDENT ) )
    {
        aLR.SetTxtFirstLineOfst( -(short)nFinalPos );
        aLR.SetTxtLeft(           (short)nFinalPos );
    }
    else
        aLR.SetTxtLeft( (short)nFinalPos );
    SetAttr( aLR );

    // Flag setzen, damit Einrueckung bei num. Listen ggfs. wieder
    // entfernt werden kann...
    bIsSTMInPara = TRUE;

    // Flag setzen, damit bei HLN alter Wert restauriert wird.
    bSingleParaMgn = TRUE;
}


    // .<LMR> wird leider nicht ueberfluessig, da WP kein IPS liefert.
    // Wenn vorher ein IPS aufgetreten ist, wird dieses jedoch
    // ignoriert, da IPS eine weitergehende Aussage macht

void SwW4WParser::Read_SetLeftMarginRelease()			// (LMR)
{
    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_LR_SPACE );
        return;
    }
    if( bIsIpsInPara  ) return;

    long nSpaces, nOptRelLeftTw, nOptAbsLeftTw;
    short nFirstLeft;
    BOOL bAbs = FALSE;

    if( W4WR_TXTERM != GetDecimal( nSpaces ) || nError )
        return;

    // gibt es die optionale Angabe ?
    if( W4WR_TXTERM == GetDecimal( nOptRelLeftTw ) && !nError )
    {
        nFirstLeft = *((short*)&nOptRelLeftTw); // evtl. UEberlauf ist beabsichtigt,
                                            // da bei W4W neg. Zahlen um 65536
                                            // erhoeht werden
        if( W4WR_TXTERM == GetDecimal( nOptAbsLeftTw ) && !nError )
        {
            nFirstLeft = (short)(nOptAbsLeftTw - GetLeftMargin());
            bAbs = TRUE;
        }
    }
    else
        nFirstLeft = (short)nSpaces * nColSize;	// in 1/10 Inch / ??

/*
    // erstmal das alte zuruecksetzen
    if( !bStyleDef )
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_LR_SPACE );
*/
/*
    SwFmtLRSpace aLR( *(SwFmtLRSpace*)
                    pCtrlStck->GetFmtAttr( *pCurPaM, RES_LR_SPACE ));
*/
    SvxLRSpaceItem aLR( *(SvxLRSpaceItem*) GetFmtAttr( RES_LR_SPACE ) );



    if( bAbs )
        aLR.SetTxtFirstLineOfst( nFirstLeft );
    else		// relative Angaben sind leider kumulativ
    {
        short nIndent    = GetLeftMargin() - nFirstLeft;
        short nAbsIndent = aLR.GetTxtLeft();
        if( nIndent > ( nAbsIndent * -1 ) )
            nIndent = nAbsIndent;

        aLR.SetTxtFirstLineOfst( nIndent * -1 );
    }
    SetAttr( aLR );

#if 0
    // MIB 16.5.2001: A default tab at pos 0 isn't required since 1996.
    if( nDocType == 33 && bStyleDef && !bIsTab )
    {
                    // AmiPro hat nicht so irrsinnig viele Tabs im Metafile,
                    // so dass die dort eingebaute Mimik zum Setzen eines
                    // Default-Tabs an die Pos 0 ( fuer 1.Zeile < LRand )
                    // nicht ausreicht.

        SvxTabStopItem aTabStops( 0, 0 );
        aTabStops.Insert( SvxTabStop( 0, SVX_TAB_ADJUST_DEFAULT ) );
        SetAttr( aTabStops );
        bIsTab = TRUE;
    }
#endif
}
                // fuer IPS und RSM waehrend StyleDefinition
                // und fuer RSM bei WordPerfekt f. Win 6,7,8

void SwW4WParser::Read_IndentPara1(long  nLeft, long n1st, long nRight)
{
    n1st -= nLeft;						// 1st rel. zu left
    nLeft -= nPgLeft;					// left rel. zu Page
    if( nLeft < 0) nLeft = 0;
    if( n1st < -nLeft ) n1st = -nLeft;
    nRight -= nPgRight; 				// right rel. zu Page
    if( nRight < 0 ) nRight = 0;

                                        //!!! FlyFrames auch testen ??
    if( bIsColMode && pTabDefs ){		// Tabelle
        long nBoxSize = pTabDefs[nTabCol].nRightTw
                        - pTabDefs[nTabCol].nLeftTw;
        if( nLeft + nRight + HOR_SICHER > nBoxSize
            || n1st + nRight + HOR_SICHER > nBoxSize )
            nRight = nLeft = n1st = 0; // Stuss-Werte -> keine Einrueckung
    }
/*
    // erstmal das alte zuruecksetzen
    if( !bStyleDef )
        pCtrlStck->SetAttr( *pCurPaM->GetPoint(), RES_LR_SPACE );
*/
    SvxLRSpaceItem aLR;
    aLR.SetTxtLeft( USHORT(nLeft) );
    aLR.SetRight( USHORT(nRight) );
        aLR.SetTxtFirstLineOfst( (short)n1st );

    SetAttr( aLR );
}


void SwW4WParser::Read_IndentPara() 					// (IPS)
{
    if ( bStyleOnOff ){ 	// in Style An / Aus -> Ende des Attributes
        Read_HardAttrOff( RES_LR_SPACE );
        return;
    }

    long nLCol, n1Col;
                            // Die folgenden Parameter sind entgegen der
                            // Dokumentation optional, bei DosWord fehlen sie

    long nOptLeft, nOpt1st, nOptRight;

    if( GetDecimal( nLCol ) && !nError && GetDecimal( n1Col ) && !nError )
    {

        if ( W4WR_TXTERM != GetDecimal( nOptLeft ) || nError
            || W4WR_TXTERM != GetDecimal( nOpt1st ) || nError
            || W4WR_TXTERM != GetDecimal( nOptRight ) || nError )
        {
            nOptLeft = nLCol * nColSize;
            nOpt1st = n1Col * nColSize;
            nOptRight = nPgRight;
        }
        Read_IndentPara1( nOptLeft, nOpt1st, nOptRight);
        bIsIpsInPara = TRUE;		// -> ignoriere STM, LMR, RMI
        // Flag setzen, damit bei HLN alter Wert restauriert wird.
        bSingleParaMgn = TRUE;
    }
}


/*
 *  Es folgen die Methoden fuer vertikale Abstaende.
 *
 *  zur Erinnerung hier nochmal die unterschiedlichen Randschemata (k.h.z.)
 *
 *
 *  W4W:                                           Beachte: Bei WinWord 2
 *    +---------------------------                          und bei AmiPro
 *    |                 - HTM      \                        liefert das miese
 *    |  +-----------------         \                       W4W uns lediglich
 *    |  |                           \
 *    |  | Kopf                       \                     STP und SBP
 *    |  |                              - STP
 *    |  +-----------------          /                      die uebrigen Daten
 *    |                 - HM        /                       fehlen, so dass
 *    |  +------------------------ /                        mit moeglichst
 *    |  |                                                  plausiblen Hilfs-
 *    |  |                                                  werten gearbeitet
 *    |  | Rumpf                                            werden muss . . .
 *    |  |
 *    |  |                                                  Bei WordPerfekt
 *    |  +------------------------                          f. Win. stehen
 *    |                 - FM       \                        uns dann alle
 *    |  +-----------------         \                       moeglichen Werte
 *    |  |                           \                      zur Verfuegung
 *    |  | Fuss                       \                     und wir koennen
 *    |  |                              - SBP               die Raender
 *    |  +-----------------          /                      richtig genau
 *    |                 - FBM       /                       austuefteln.
 *    +--------------------------- /
 *
 *
 *
 *  Writer:
 *    +---------------------------
 *    |     - Seitenrand oben                   nPgUpper
 *    |  +---------------------
 *    |  |                     \
 *    |  | Kopf                 - Kopf-Hoehe    nHdHeight
 *    |  |                     /
 *    |  +----------------    /
 *    |     - Kopf-Margin    /                  nHdLower
 *    |  +------------------/
 *    |  |
 *    |  |
 *    |  | Rumpf
 *    |  |
 *    |  |
 *    |  +------------------\
 *    |     - Fuss-Margin    \                  nFtUpper
 *    |  +----------------    \
 *    |  |                     \
 *    |  | Fuss                 - Fuss-Hoehe    nFtHeight
 *    |  |                     /
 *    |  +---------------------
 *    |     - Seitenrand unten                  nPgLower
 *    +---------------------------
 */


void SwW4WParser::UpdateHdFtMarginSettings( SwFrmFmt *pHdFtFmt,
                                            long nHeight,
                                            long nDelta,
                                            BOOL bDoTheHeader )
{
    // aktuelle Werte ermitteln
    SwFmtFrmSize   aSz( pHdFtFmt->GetFrmSize() );	// Height / Width
    SvxULSpaceItem aUL( pHdFtFmt->GetULSpace() );	// Upper  / Lower

    // sicherstellen, dass MIN-Size eingestellt ist,
    // (sonst gilt naemlich VAR-Size)
    // und die Werte aendern
    aSz.SetSizeType( ATT_MIN_SIZE );
    aSz.SetHeight( nHeight );
    if( bDoTheHeader )
        aUL.SetLower( USHORT( nDelta ) );			// nDelta = nHdLower
    else
        aUL.SetUpper( USHORT( nDelta ) );    	    // nDelta = nFtLower

    // geaenderte Werte eintragen
    pHdFtFmt->SetAttr( aSz );
    pHdFtFmt->SetAttr( aUL );
}


void SwW4WParser::AdjustTempVar1( long& rHeight,
                                  long& rPgDelta,
                                  long& rHdFtDelta,
                                  long  nIniHdFtSiz )
{
    /*
     * mindest. Kopf-/Fussbereich-Hoehe steht in der Ini-Datei
     * oder wird mit 0,50 cm angesetzt
     */
    long nMin = (nIniHdFtSiz ? nIniHdFtSiz : MM50) * 3 / 2;
    if( rHeight < nMin )
        rHeight = nMin;
#if 0
// why get the page upper / lower the half of the header/footer height?
    rPgDelta   = rHeight  / 2;
    rHdFtDelta = rPgDelta / 2;
#else
    // then the half height for the header/footer and the other half
    // height get half the page and the header/footer distance
    rHdFtDelta = rPgDelta  = rHeight / 4;
    rHeight -= rPgDelta;
#endif
}



void SwW4WParser::SetPageDescVSpaces( SwPageDesc& rPageDesc,
                                      W4W_UpdtPgMrgSet eCalledByWhom )
{
    // Variablen fuer Writer
    long	nPgUpper,
            nHdHeight, nHdLower,
            nFtUpper,  nFtHeight,
            nPgLower;
    BOOL	bHasHeader,
            bHasFooter;

    // Umrechnen vom W4W-Schema zum Writer-Schema (s.oben)
    nPgUpper  = nNewValueHTM;
    nPgLower  = nNewValueFBM;

    nHdHeight = nNewValueSTP - nPgUpper;
    nFtHeight = nNewValueSBP - nPgLower;

    nHdLower  = nNewValueHM;
    nFtUpper  = nNewValueFM;

    // weitere Plausibilitaetskontrolle, ggfs. Anpassung an Minimalwert
    bHasHeader = (nHdHeight > 0);
    if(    bHasHeader
        && (nHdHeight - nHdLower < MM50) )      // siehe SWTYPES.HXX
    {
        nHdHeight    = nHdLower + MM50;
        nNewValueSTP = nHdHeight + nPgUpper;
    }
    bHasFooter = (nFtHeight > 0);
    if(    bHasFooter
        && (nFtHeight - nFtUpper < MM50) )
    {
        nFtHeight    = nFtUpper + MM50;
        nNewValueSBP = nFtHeight + nPgLower;
    }

    // passende Werte-Initialisierungen, falls Aufruf aus HF2 bzw. HF1
    if( CALLED_BY_HF2 == eCalledByWhom &&
        ( !bHasHeader || ( !nPgUpper && !nHdLower ) ) )
    {
        bHasHeader = TRUE;
        AdjustTempVar1( nHdHeight, nPgUpper, nHdLower, nIniHdSiz );
    }
    else if( CALLED_BY_HF1 == eCalledByWhom &&
            ( !bHasFooter || ( !nPgLower && !nFtUpper ) ) )
    {
        bHasFooter = TRUE;
        AdjustTempVar1( nFtHeight, nPgLower, nFtUpper, nIniFtSiz );
    }

    /*
        Setzen der neuen Werte
        bzw. Loeschen des Kopf- oder Fuss-Bereichs
    */
    SwFrmFmt &rPageFmt = rPageDesc.GetMaster();

    // Kopfbereich
    if( eCalledByWhom != CALLED_BY_HF1 )
    {
        SwFrmFmt *pHdFmt;
        pHdFmt = (SwFrmFmt*)rPageFmt.GetHeader().GetHeaderFmt();
        if( pHdFmt )
        {
            if( bHasHeader )
            {
                if( !nPgUpper && !nHdLower )
                    AdjustTempVar1( nHdHeight, nPgUpper, nHdLower, nIniHdSiz );
                UpdateHdFtMarginSettings( pHdFmt, nHdHeight, nHdLower, TRUE);
            }
            else
                rPageFmt.SetAttr(SwFmtHeader(BOOL( FALSE )));
        }
    }
    // Fussbereich
    if( eCalledByWhom != CALLED_BY_HF2 )
    {
        SwFrmFmt *pFtFmt;
        pFtFmt = (SwFrmFmt*)rPageFmt.GetFooter().GetFooterFmt();
        if( pFtFmt )
        {
            if( bHasFooter )
            {
                if( !nPgLower && !nFtUpper )
                    AdjustTempVar1( nFtHeight, nPgLower, nFtUpper, nIniFtSiz );
                UpdateHdFtMarginSettings(pFtFmt, nFtHeight, nFtUpper, FALSE);
            }
            else
                rPageFmt.SetAttr(SwFmtFooter(BOOL( FALSE )));
        }
    }

    // eine letzte Korrektur: falls keine Hd(bzw. Ft)-Abstaende definiert,
    // setze den gesamten oberen (unteren) Abstand als Seitenrand ein
    if( nHdHeight && (!nPgUpper) && (!nHdLower) ) nPgUpper = nHdHeight;
    if( nFtHeight && (!nPgLower) && (!nFtUpper) ) nPgLower = nFtHeight;

    // Seitenrand oben/unten
    if( eCalledByWhom == CALLED_BY_FLUSH_OR_HNL )
    {
        SvxULSpaceItem aUL( (const USHORT) nPgUpper, (const USHORT) nPgLower );
        rPageFmt.SetAttr( aUL );
    }
    else
    {
        SvxULSpaceItem aUL( rPageFmt.GetULSpace() );	// Upper  / Lower
        switch ( eCalledByWhom ){
        case CALLED_BY_HF2: // oberer Seitenrand
                            aUL.SetUpper( USHORT( nPgUpper ) );
                            if( nNewValueSBP != nLastValueSBP )
                                aUL.SetLower( USHORT( nPgLower ) );
                            break;
        case CALLED_BY_HF1: // unterer Seitenrand
                            aUL.SetLower( USHORT( nPgLower ) );
                            if( nNewValueSTP != nLastValueSTP )
                                aUL.SetUpper( USHORT( nPgUpper ) );
                            break;
        default:			// was wollen wir denn hier ???
                            ASSERT( !this, "eCalledByWhom has undefined value" );
                            break;
        }
        rPageFmt.SetAttr( aUL );
    }
}

void SwW4WParser::UpdatePageMarginSettings( W4W_UpdtPgMrgSet eCalledByWhom )
{
    BOOL bValueChanged = (     (nNewValueSTP != nLastValueSTP)
                            || (nNewValueSBP != nLastValueSBP)
                            || (nNewValueHTM != nLastValueHTM)
                            || (nNewValueHM  != nLastValueHM )
                            || (nNewValueFBM != nLastValueFBM)
                            || (nNewValueFM  != nLastValueFM ) );
    if(    ( !bValueChanged )
        && ( eCalledByWhom != CALLED_BY_HF2 )
        && ( eCalledByWhom != CALLED_BY_HF1 ) )
    {
        return;
    }
    if( bValueChanged )
    {
        /* Plausibilitaetspruefung, ggfs. Korrektur der W4W-Variablen:

           Wir pruefen, ob STP oder SBP etwa zu KLEIN werden,
           wenn NUR die Werte HTM, HM, FBM oder FM geaendert wurden
           und korrigieren dann ggfs. STP oder SBP,
           da wir in diesem Fall von einem W4W-Fehler ausgehen.

           Wir pruefen NICHT, ob HTM, HM, FBM oder FM zu gross sind,
                       =====
           nachdem STP bzw. SBP veraendert wurden,
           sondern lassen die Variablen in diesen Faellen unveraendert,
           schalten aber dann im Writer den Header- bzw. Footer-Bereich aus.
           Grund: so bleiben die Header-/Footer-Raender erhalten und
           koennen bei spaeterer Reaktivierung des Header-/Footer-Bereichs
           sofort wieder in die Berechnung einfliessen.
        */

        if(    (	(nNewValueHTM != nLastValueHTM)
                 || (nNewValueHM  != nLastValueHM )
               )
            && (nNewValueSTP == nLastValueSTP) )
        {
            /*
                Wenn HTM oder HM geaendert wurden, STP unveraendert blieb
                und STP hierdurch zu klein wird,
                wird STP um die Aenderungs-Differenz von HTM und HM
                erhoeht.
                Grund: die HOEHE des Kopfes veraendert sich,
                wenn HTM oder HM veraendert werden, darf aber natuerlich
                nicht negativ werden. Wir vermuten dann einen W4W-Fehler:
                es wurde wohl vergessen STP anzupassen !
            */
            if( nNewValueSTP < nNewValueHTM + nNewValueHM )
            {
                nNewValueSTP =
                    nLastValueSTP + ( nNewValueHTM +  nNewValueHM)
                                  - (nLastValueHTM + nLastValueHM);
            }
        }
        // Fuss-Bereich
        if(    (	(nNewValueFBM != nLastValueFBM)
                 || (nNewValueFM  != nLastValueFM )
               )
            && (nNewValueSBP == nLastValueSBP) )
        {
            // ( Vorgehensweise analog zum Kopfbereich - s.o. )
            if( nNewValueSBP < nNewValueFBM + nNewValueFM )
            {
                nNewValueSBP =
                    nLastValueSBP + ( nNewValueFBM +  nNewValueFM)
                                  - (nLastValueFBM + nLastValueFM);
            }
        }
    }

    // Jetzt Abstaende tatsaechlich einstellen
    if( pOpen1stPgPageDesc )
        SetPageDescVSpaces( *pOpen1stPgPageDesc, eCalledByWhom );
    SetPageDescVSpaces(     *pPageDesc,          eCalledByWhom );

    // Aktualisieren der W4W-Variablen
    nLastValueSTP = nNewValueSTP;
    nLastValueSBP = nNewValueSBP;
    nLastValueHTM = nNewValueHTM;
    nLastValueHM  = nNewValueHM ;
    nLastValueFBM = nNewValueFBM;
    nLastValueFM  = nNewValueFM ;
}


void SwW4WParser::Read_SetTopMarginOfPage() 		// (STP)
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    if ( bIsTxtInPgDesc && bWasTxtSinceLastHF2 && (nDocType != 15) )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    // skip "Old top margin" and read "New Top margin"
    long nNew;
    if( nError
        || W4WR_TXTERM != SkipPara()
        || W4WR_TXTERM != GetDecimal( nNew )
        || nError )
        return;

    long nOptNew;
    // gibt es die optionale Angabe ?
    if(    ( W4WR_TXTERM == GetDecimal( nOptNew ) )
        && !nError )
    {
        nNewValueSTP = nOptNew;					// in Twip
    }
    else
    {
        if( nDocType == 15 && !( nIniFlags & W4WFL_NO_PCTEXT4_STP_DEC2 ) )
        {
            long nTw = ( nNew - 2 ) * 240;
            if( nTw < 100 )
                nTw = 100;
            nNewValueSTP = nTw;		  			// PCText4 Korektur-Raten
        }
        else
        {
            nNewValueSTP = nNew * 240; 		 	// in 1/6 Inch
        }
    }
}


void SwW4WParser::Read_SetBottomMarginOfPage()		// (SBP)
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    if ( bIsTxtInPgDesc && bWasTxtSinceLastHF1 && (nDocType != 15) )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    long nNew;
    if(    nError
        || W4WR_TXTERM != GetDecimal( nNew )
        || nError )
        return;

    long nOptNew;
    // gibt es die optionale Angabe ?
    if(    ( W4WR_TXTERM == GetDecimal( nOptNew ) )
        && !nError )
        nNewValueSBP = nOptNew;					// in Twip
    else
        nNewValueSBP = nNew * 240;				// in 1/6 Inch
}


void SwW4WParser::Read_HeaderTopMargin()			// HTM
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    if ( bIsTxtInPgDesc && bWasTxtSinceLastHF2 && (nDocType != 15) )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    long nNew;
    if(    nError
        || W4WR_TXTERM != GetDecimal( nNew )
        || nError )
        return;

    long nOptNew;
    // gibt es die optionale Angabe ?
    if(    ( W4WR_TXTERM == GetDecimal( nOptNew ) )
        && !nError )
        nNewValueHTM = nOptNew;					// in Twip
    else
        nNewValueHTM = nNew * 240;				// in 1/6 Inch
}


void SwW4WParser::Read_HeadingMargin()				// HM
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    if ( bIsTxtInPgDesc && bWasTxtSinceLastHF2 && (nDocType != 15) )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    long nNew;
    if(    nError
        || W4WR_TXTERM != GetDecimal( nNew )
        || nError )
        return;

    long nOptNew;
    // gibt es die optionale Angabe ?
    if(    ( W4WR_TXTERM == GetDecimal( nOptNew ) )
        && !nError )
        nNewValueHM = nOptNew;					// in Twip
    else
        nNewValueHM = nNew * 240;				// in 1/6 Inch
}


void SwW4WParser::Read_FooterBottomMargin()   		// FBM
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    if ( bIsTxtInPgDesc && bWasTxtSinceLastHF1 && (nDocType != 15) )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    long nNew;
    if(    nError
        || W4WR_TXTERM != GetDecimal( nNew )
        || nError )
        return;

    long nOptNew;
    // gibt es die optionale Angabe ?
    if(    ( W4WR_TXTERM == GetDecimal( nOptNew ) )
        && !nError )
        nNewValueFBM = nOptNew;					// in Twip
    else
        nNewValueFBM = nNew * 240;				// in 1/6 Inch
}


void SwW4WParser::Read_FooterMargin()	 	 		// FM
{
    if( bStyleDef || bHeadFootDef || bFootnoteDef ) return;

    if ( bIsTxtInPgDesc && bWasTxtSinceLastHF1 && (nDocType != 15) )
        CreatePageDesc(CRPGD_AND_INSERT | CRPGD_UPDT_MRGN);

    long nNew;
    if(    nError
        || W4WR_TXTERM != GetDecimal( nNew )
        || nError )
        return;

    long nOptNew;
    // gibt es die optionale Angabe ?
    if(    ( W4WR_TXTERM == GetDecimal( nOptNew ) )
        && !nError )
        nNewValueFM = nOptNew;					// in Twip
    else
        nNewValueFM = nNew * 240;				// in 1/6 Inch
}


ULONG W4WReader::Read( SwDoc &rDoc, SwPaM &rPam, const String & )
{
    if( pStrm || pStg )
    {
        ASSERT( !this,
        "W4W-Reader darf nur mit einem Medium aufgerufen werden" );
        return ERR_SWG_READ_ERROR;
    }
    else
        // Stream darf auf gar keinen fall won irgend jemanden geoeffnet sein!!!
        pMedium->CloseInStream();

    // ist die DLL ueberhaupt vorhanden?
    if( !W4WDLLExist( W4WDLL_IMPORT, GetFilter() ) )
        return ERR_W4W_DLL_ERROR | ERROR_SW_READ_BASE;

    String aTmpFile;
    String sFileName( pMedium->GetPhysicalName() );
    ASSERT( sFileName.Len(),
        "W4W-Reader hat keinen Dateinamen uebergeben bekommen" );
    if( !GetFilter() || !sFileName.Len() )
        return ERR_SWG_READ_ERROR;

//  ::StartProgress( STR_STATSTR_W4WREAD, 0, 100, rDoc.GetDocShell() );

    ULONG nError = LoadFile( sFileName, GetFilter(), GetVersion(), aTmpFile );

    if( !nError )
    {	// dieser Block muss sein, damit das File geschlossen ist,
        // wenn es nach Benutzung geloescht werden soll

        // den Stream kommt mit dem internen Format oeffnen
        SfxMedium aMedium( aTmpFile, STREAM_READ, TRUE );
        SvStream* pStrm = aMedium.GetInStream();
        if( pStrm && SVSTREAM_OK == pStrm->GetError() )
        {
            if( !bInsertMode )
            {
                // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
                Reader::ResetFrmFmts( rDoc );
            }

            SwW4WParser *pParser = new SwW4WParser( rPam, *pStrm,
                            !bInsertMode, GetFilter(), GetVersion() );
            if( !pParser->CallParser() )
// !! hier muss der interne Fehler noch auf den SWG-Fehler gemappt werden !!
                nError = ERR_SWG_READ_ERROR;

            delete pParser; 			// Fehlermeldungen werden immer
        }								// vom Parser selber ausgegeben
    }

//  ::EndProgress( rDoc.GetDocShell() );

    SWUnoHelper::UCB_DeleteFile( aTmpFile );

    return nError;
}

int W4WReader::GetReaderType()
{
    return bStorageFlag ? SW_STORAGE_READER : SW_STREAM_READER;
}

// wir wollen die Streams / Storages nicht geoeffnet haben
int W4WReader::SetStrmStgPtr()
{
    return TRUE;
}


void W4WReader::SetFltName( const String& rFltName )
{
    String sFilter( rFltName.Copy( 3 ));
    xub_StrLen nPos = sFilter.Search( '_' );
    sVersion = sFilter.Copy( nPos + 1 );
    sFilter.Erase( nPos + 1 );
    if( 0 != ( bStorageFlag = 'C' == sFilter.GetChar( 0 )))
        sFilter.Erase( 0, 1 );

    nFilter = sFilter.ToInt32();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
