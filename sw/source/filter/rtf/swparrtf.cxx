/*************************************************************************
 *
 *  $RCSfile: swparrtf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
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

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef __RSC //autogen
#include <tools/errinf.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _RTFTOKEN_H
#include <svtools/rtftoken.h>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _TEXTCONV_HXX //autogen
#include <svx/textconv.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOCSTAT_HXX //autogen
#include <docstat.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // ...Percent()
#endif
#ifndef _SWPARRTF_HXX
#include <swparrtf.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _SECTIOM_HXX
#include <section.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>          // ResId fuer Statusleiste
#endif


// einige Hilfs-Funktionen
// char
inline const SvxFontHeightItem& GetSize(const SfxItemSet& rSet,BOOL bInP=TRUE)
    { return (const SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,BOOL bInP=TRUE)
    { return (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE,bInP); }


/*  */
#ifdef DEBUG_JP

static void OutStyle( const SwPaM& rPam, const SwTxtFmtColl& rColl );
static void OutSet( const SwPaM& rPam, const SfxItemSet& rSet );
static void OutText( const SwPaM& rPam, const sal_Char* pText );
static void DumpStart();
static void DumpEnde();

#define DUMP_INIT                   DumpStart();
#define DUMP_FINIT                  DumpEnde();
#define DUMP_SET( pam, set )        OutSet( pam, set );
#define DUMP_STYLE( pam, ss )       OutStyle( pam, ss );
#define DUMP_TXT( pam, txt )        OutText( pam, txt );

#else

#define DUMP_INIT
#define DUMP_FINIT
#define DUMP_SET( pam, set )
#define DUMP_STYLE( pam, ss )
#define DUMP_TXT( pam, txt )

#endif

/*  */

// Aufruf fuer die allg. Reader-Schnittstelle
ULONG RtfReader::Read( SwDoc &rDoc,SwPaM &rPam, const String &)
{
    if( !pStrm )
    {
        ASSERT( FALSE, "RTF-Read ohne Stream" );
        return ERR_SWG_READ_ERROR;
    }

    //JP 18.01.96: Alle Ueberschriften sind normalerweise ohne
    //              Kapitelnummer. Darum hier explizit abschalten
    //              weil das Default jetzt wieder auf AN ist.
    if( !bInsertMode )
    {
        Reader::SetNoOutlineNum( rDoc );

        // MIB 27.09.96: Umrandung uns Abstaende aus Frm-Vorlagen entf.
        Reader::ResetFrmFmts( rDoc );
    }

    ULONG nRet = 0;
    SvParserRef xParser = new SwRTFParser( &rDoc, rPam, *pStrm,!bInsertMode );
    SvParserState eState = xParser->CallParser();
    if( SVPAR_PENDING != eState && SVPAR_ACCEPTED != eState )
    {
        String sErr( String::CreateFromInt32( xParser->GetLineNr() ));
        sErr += ',';
        sErr += String::CreateFromInt32( xParser->GetLinePos() );

        nRet = *new StringErrorInfo( ERR_FORMAT_ROWCOL, sErr,
                                    ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
    }
    return nRet;
}

SwRTFParser::SwRTFParser( SwDoc* pD, const SwPaM& rCrsr, SvStream& rIn,
                        int bReadNewDoc )
    : SvxRTFParser( pD->GetAttrPool(), rIn, bReadNewDoc ),
    pDoc( pD ),
    pTableNode( 0 ), pOldTblNd( 0 ), nAktBox( 0 ), nNewNumSectDef( USHRT_MAX ),
    nAktPageDesc( 0 ), nAktFirstPageDesc( 0 ),
    pGrfAttrSet( 0 ),
    aMergeBoxes( 0, 5 ),
    nInsTblRow( USHRT_MAX ),
    pSttNdIdx( 0 ),
    pRegionEndIdx( 0 ),
    pRelNumRule( new SwRelNumRuleSpaces( *pD, bReadNewDoc )),
    aTblFmts( 0, 10 )
{
    bReadNoTbl = bReadSwFly = bSwPageDesc = bStyleTabValid =
    bInPgDscTbl = bNewNumList = FALSE;
    bFirstContinue = bFirstDocControl = TRUE;

    pPam = new SwPaM( *rCrsr.GetPoint() );
    SetInsPos( SwxPosition( pPam ) );
    SetChkStyleAttr( TRUE );
    SetCalcValue( FALSE );
    SetReadDocInfo( TRUE );

    // diese sollen zusaetzlich ueber \pard zurueck gesetzt werden
    USHORT temp;
    temp = RES_TXTATR_CHARFMT;      AddPlainAttr( temp );
    temp = RES_PAGEDESC;            AddPardAttr( temp );
    temp = RES_BREAK;               AddPardAttr( temp );
    temp = RES_PARATR_NUMRULE;      AddPardAttr( temp );
    temp = FN_PARAM_NUM_LEVEL;          AddPardAttr( temp );

    DUMP_INIT
}

// Aufruf des Parsers
SvParserState SwRTFParser::CallParser()
{
    bReadNoTbl = FALSE;
    bFirstContinue = TRUE;

    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();

    return SvxRTFParser::CallParser();
}


void SwRTFParser::Continue( int nToken )
{
    if( bFirstContinue )
    {
        bFirstContinue = FALSE;

        // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( !IsNewDoc() )       // in ein Dokument einfuegen ?
        {
            const SwPosition* pPos = pPam->GetPoint();
            SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();

            pDoc->SplitNode( *pPos );

            *pSttNdIdx = pPos->nNode.GetIndex()-1;
            pDoc->SplitNode( *pPos );

            pPam->Move( fnMoveBackward );
            pDoc->SetTxtFmtColl( *pPam, pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));

            // verhinder das einlesen von Tabellen in Fussnoten / Tabellen
            ULONG nNd = pPos->nNode.GetIndex();
            bReadNoTbl = 0 != pSttNd->FindTableNode() ||
                        ( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
                        pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
        }

        // Laufbalken anzeigen, aber nur bei synchronem Call
        ULONG nCurrPos = rInput.Tell();
        rInput.Seek(STREAM_SEEK_TO_END);
        rInput.ResetError();
        ::StartProgress( STR_STATSTR_W4WREAD, 0, rInput.Tell(), pDoc->GetDocShell());
        rInput.Seek( nCurrPos );
        rInput.ResetError();
    }

    SvxRTFParser::Continue( nToken );

    if( SVPAR_PENDING == GetStatus() )
        return ;                // weiter gehts beim naechsten mal

    // Laufbalken bei asynchronen Call nicht einschalten !!!
    ::EndProgress( pDoc->GetDocShell() );

#ifdef DEBUG_JP
{
extern Writer* GetDebugWriter(const String&);
        Writer* pWriter = GetDebugWriter(aEmptyStr);
        if( pWriter )
        {
            SwWriter( SvFileStream( "f:\\tmp\\$$rtf_1.db", STREAM_WRITE ),
                        *pDoc ).Write( pWriter );
        }
}
#endif


    {
        // JP 13.08.98: TabellenUmrandungen optimieren - Bug 53525
        for( USHORT n = aTblFmts.Count(); n; )
        {
            SwTable* pTbl = SwTable::FindTable( (SwFrmFmt*)aTblFmts[ --n ] );
            if( pTbl )
                pTbl->GCBorderLines();
        }
    }

    pRelNumRule->SetNumRelSpaces( *pDoc );

    // den Start wieder korrigieren
    if( !IsNewDoc() && pSttNdIdx->GetIndex() )
    {
        //die Flys muessen zuerst zurecht gerueckt werden, denn sonst wird
        // ein am 1. Absatz verankerter Fly falsch eingefuegt
        if( SVPAR_ACCEPTED == eState )
        {
            if( aFlyArr.Count() )
                SetFlysInDoc();
            pRelNumRule->SetOultineRelSpaces( *pSttNdIdx, pPam->GetPoint()->nNode );
        }

        SwTxtNode* pTxtNode = pSttNdIdx->GetNode().GetTxtNode();
        SwNodeIndex aNxtIdx( *pSttNdIdx );
        if( pTxtNode && pTxtNode->CanJoinNext( &aNxtIdx ))
        {
            xub_StrLen nStt = pTxtNode->GetTxt().Len();
            // wenn der Cursor noch in dem Node steht, dann setze in an das Ende
            if( pPam->GetPoint()->nNode == aNxtIdx )
            {
                pPam->GetPoint()->nNode = *pSttNdIdx;
                pPam->GetPoint()->nContent.Assign( pTxtNode, nStt );
            }

#ifndef PRODUCT
// !!! sollte nicht moeglich sein, oder ??
ASSERT( pSttNdIdx->GetIndex()+1 != pPam->GetBound( TRUE ).nNode.GetIndex(),
            "Pam.Bound1 steht noch im Node" );
ASSERT( pSttNdIdx->GetIndex()+1 != pPam->GetBound( FALSE ).nNode.GetIndex(),
            "Pam.Bound2 steht noch im Node" );

if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( TRUE ).nNode.GetIndex() )
{
    register xub_StrLen nCntPos = pPam->GetBound( TRUE ).nContent.GetIndex();
    pPam->GetBound( TRUE ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
if( pSttNdIdx->GetIndex()+1 == pPam->GetBound( FALSE ).nNode.GetIndex() )
{
    register xub_StrLen nCntPos = pPam->GetBound( FALSE ).nContent.GetIndex();
    pPam->GetBound( FALSE ).nContent.Assign( pTxtNode,
                    pTxtNode->GetTxt().Len() + nCntPos );
}
#endif
            // Zeichen Attribute beibehalten!
            SwTxtNode* pDelNd = aNxtIdx.GetNode().GetTxtNode();
            if( pTxtNode->GetTxt().Len() )
                pDelNd->FmtToTxtAttr( pTxtNode );
            else
                pTxtNode->ChgFmtColl( pDelNd->GetTxtColl() );
            pTxtNode->JoinNext();
        }
    }

    if( SVPAR_ACCEPTED == eState )
    {
        // den letzen Bereich wieder zumachen
        if( pRegionEndIdx )
        {
            // JP 06.01.00: Task 71411 - the last section in WW are not a
            //              balanced Section.
            if( !GetVersionNo() )
            {
                SwSectionNode* pSectNd = pRegionEndIdx->GetNode().
                                    FindStartNode()->GetSectionNode();
                if( pSectNd )
                    pSectNd->GetSection().GetFmt()->SetAttr(
                                    SwFmtNoBalancedColumns( TRUE ) );
            }

            DelLastNode();
            pPam->GetPoint()->nNode = *pRegionEndIdx;
            pPam->Move( fnMoveForward, fnGoNode );
            delete pRegionEndIdx, pRegionEndIdx = 0;
        }

        if( aFlyArr.Count() )
            SetFlysInDoc();

        // jetzt noch den letzten ueberfluessigen Absatz loeschen
        SwPosition* pPos = pPam->GetPoint();
        if( !pPos->nContent.GetIndex() )
        {
            SwTxtNode* pAktNd;
            ULONG nNodeIdx = pPos->nNode.GetIndex();
            if( IsNewDoc() )
            {
                SwNode* pTmp = pDoc->GetNodes()[ nNodeIdx -1 ];
                if( pTmp->IsCntntNode() && !pTmp->FindTableNode() )
                    DelLastNode();
            }
            else if( 0 != ( pAktNd = pDoc->GetNodes()[ nNodeIdx ]->GetTxtNode()) )
            {
                if( pAktNd->CanJoinNext( &pPos->nNode ))
                {
                    SwTxtNode* pNextNd = pPos->nNode.GetNode().GetTxtNode();
                    pPos->nContent.Assign( pNextNd, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pNextNd->JoinPrev();
                }
                else if( !pAktNd->GetTxt().Len() &&
                        pAktNd->StartOfSectionIndex()+2 <
                        pAktNd->EndOfSectionIndex() )
                {
                    pPos->nContent.Assign( 0, 0 );
                    pPam->SetMark(); pPam->DeleteMark();
                    pDoc->GetNodes().Delete( pPos->nNode, 1 );
                    pPam->Move( fnMoveBackward );
                }
            }
        }

        // nun noch das SplitNode vom Ende aufheben
        else if( !IsNewDoc() )
        {
            if( pPos->nContent.GetIndex() )     // dann gabs am Ende kein \par,
                pPam->Move( fnMoveForward, fnGoNode );  // als zum naechsten Node
            SwTxtNode* pTxtNode = pPos->nNode.GetNode().GetTxtNode();
            SwNodeIndex aPrvIdx( pPos->nNode );
            if( pTxtNode && pTxtNode->CanJoinPrev( &aPrvIdx ) &&
                *pSttNdIdx <= aPrvIdx )
            {
                // eigentlich muss hier ein JoinNext erfolgen, aber alle Cursor
                // usw. sind im pTxtNode angemeldet, so dass der bestehen
                // bleiben MUSS.

                // Absatz in Zeichen-Attribute umwandeln, aus dem Prev die
                // Absatzattribute und die Vorlage uebernehmen!
                SwTxtNode* pPrev = aPrvIdx.GetNode().GetTxtNode();
                pTxtNode->ChgFmtColl( pPrev->GetTxtColl() );
                pTxtNode->FmtToTxtAttr( pPrev );
                pTxtNode->SwCntntNode::ResetAllAttr();

                if( pPrev->GetpSwAttrSet() )
                    pTxtNode->SwCntntNode::SetAttr( *pPrev->GetpSwAttrSet() );

                if( &pPam->GetBound(TRUE).nNode.GetNode() == pPrev )
                    pPam->GetBound(TRUE).nContent.Assign( pTxtNode, 0 );
                if( &pPam->GetBound(FALSE).nNode.GetNode() == pPrev )
                    pPam->GetBound(FALSE).nContent.Assign( pTxtNode, 0 );

                pTxtNode->JoinPrev();
            }
        }
        else if( GetDocInfo() )
            // evt. eingelesen DocInfo setzen
            pDoc->SetInfo( *GetDocInfo() );
    }
    delete pSttNdIdx, pSttNdIdx = 0;
    delete pRegionEndIdx, pRegionEndIdx = 0;
    RemoveUnusedNumRules();
}

SwRTFParser::~SwRTFParser()
{
    delete pSttNdIdx;
    delete pRegionEndIdx;
    delete pPam;
    delete pRelNumRule;

    if( aFlyArr.Count() )
        aFlyArr.DeleteAndDestroy( 0, aFlyArr.Count() );

    if( pGrfAttrSet )   DELETEZ( pGrfAttrSet );
    DUMP_FINIT
}

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                USHORT& rSubType, ULONG &rFmt,
                                USHORT nVersion );

void SwRTFParser::NextToken( int nToken )
{
    USHORT eDateFmt;

    switch( nToken )
    {
    case RTF_FOOTNOTE:      ReadHeaderFooter( nToken );
                            SkipToken( -1 );        // Klammer wieder zurueck
                            break;
    case RTF_SWG_PRTDATA:           ReadPrtData();              break;
    case RTF_FIELD:                 ReadField();                break;
    case RTF_PICT:                  ReadBitmapData();           break;
#ifdef READ_OLE_OBJECT
    case RTF_OBJECT:                ReadOLEData();              break;
#endif
    case RTF_TROWD:                 ReadTable( nToken );        break;
    case RTF_PGDSCTBL:              ReadPageDescTbl();          break;
    case RTF_LISTTABLE:             ReadListTable();            break;
    case RTF_LISTOVERRIDETABLE:     ReadListOverrideTable();    break;

    case RTF_LISTTEXT:
        GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
        SkipGroup();
        break;

    case RTF_PN:
    case RTF_PNSECLVL:
        if( bNewNumList )
            SkipGroup();
        else
            ReadNumSecLevel( nToken );
        break;

    case RTF_PNTEXT:
        SkipGroup();
        break;

    case RTF_OUTLINELEVEL:
        {
            BYTE nLevel = MAXLEVEL <= nTokenValue ? MAXLEVEL - 1
                                                  : BYTE( nTokenValue );
            GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
        }
        break;

    case RTF_DEFFORMAT:
    case RTF_DEFTAB:
    case RTF_DEFLANG:
        // sind zwar Dok-Controls, werden aber manchmal auch vor der
        // Font/Style/Color-Tabelle gesetzt!
        SvxRTFParser::NextToken( nToken );
        break;

    case RTF_PAGE:
        {
            const SwCntntNode* pCNd;
            const SwAttrSet* pSet;
            if( pPam->GetPoint()->nContent.GetIndex() ||
                ( 0 != ( pCNd = pPam->GetCntntNode()) &&
                  0 != ( pSet = pCNd->GetpSwAttrSet()) &&
                 ( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE ) ||
                   SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE ))))
                InsertPara();
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ) );
        }
        break;

    case RTF_SECT:
        ReadSectControls( nToken );
        break;
    case RTF_CELL:
        if( bReadNoTbl )
            InsertPara();
        else
        {
            if( USHRT_MAX != nInsTblRow && !pTableNode )    // Tabelle nicht mehr vorhanden ?
                NewTblLine();               // evt. Line copieren
            GotoNextBox();
        }
        break;

    case RTF_ROW:
        if( !bReadNoTbl )
        {
            // aus der Line raus
            nAktBox = 0;
            pTableNode = 0;
            // noch in der Tabelle drin?
            SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
            const SwTableNode* pTblNd = rIdx.GetNode().FindTableNode();
            if( pTblNd )
            {
                // search the end of this row
                const SwStartNode* pBoxStt =
                                    rIdx.GetNode().FindTableBoxStartNode();
                const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                                pBoxStt->GetIndex() );
                const SwTableLine* pLn = pBox->GetUpper();
                pBox = pLn->GetTabBoxes()[ pLn->GetTabBoxes().Count() - 1 ];
                rIdx = *pBox->GetSttNd()->EndOfSectionNode();
                pPam->Move( fnMoveForward, fnGoNode );
            }
            nInsTblRow = GetOpenBrakets();
            SetPardTokenRead( FALSE );
        }
        ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
        break;

    case RTF_INTBL:
        if( !bReadNoTbl )
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
                NewTblLine();           // evt. Line copieren
            else
            {
                // Crsr nicht mehr in der Tabelle ?
                if( !pPam->GetNode()->FindTableNode() )
                {
                    // dann wieder in die letzte Box setzen
                    // (kann durch einlesen von Flys geschehen!)
                    pPam->GetPoint()->nNode = *pTableNode->EndOfSectionNode();
                    pPam->Move( fnMoveBackward );
                }
            }
        }
        break;

    case RTF_FLY_INPARA:
            // \pard  und plain ueberlesen !
        if( '}' != GetNextToken() && '}' != GetNextToken() )
        {
            // Zeichengebundener Fly in Fly
            ReadHeaderFooter( nToken );
            SetPardTokenRead( FALSE );
        }
        break;

    case RTF_PGDSCNO:
        if( IsNewDoc() && bSwPageDesc &&
            USHORT(nTokenValue) < pDoc->GetPageDescCnt() )
        {
            const SwPageDesc* pPgDsc = &pDoc->GetPageDesc( USHORT(nTokenValue) );
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtPageDesc( pPgDsc ));
        }
        break;

    case RTF_COLUM:
        pDoc->Insert( *pPam, SvxFmtBreakItem( SVX_BREAK_COLUMN_BEFORE ));
        break;

    case RTF_DXFRTEXT:      // werden nur im Zusammenhang mit Flys ausgewertet
    case RTF_DFRMTXTX:
    case RTF_DFRMTXTY:
        break;

    case RTF_CHDATE:    eDateFmt = DF_SHORT;    goto SETCHDATEFIELD;
    case RTF_CHDATEA:   eDateFmt = DF_SSYS;     goto SETCHDATEFIELD;
    case RTF_CHDATEL:   eDateFmt = DF_LSYS;     goto SETCHDATEFIELD;
SETCHDATEFIELD:
        {
            USHORT nSubType = DATEFLD, nWhich = RES_DATEFLD;
            ULONG nFormat = eDateFmt;
            sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );

            SwDateTimeField aDateFld( (SwDateTimeFieldType*)
                                        pDoc->GetSysFldType( RES_DATETIMEFLD ), DATEFLD, nFormat);
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtFld( aDateFld ));
        }
        break;

    case RTF_CHTIME:
        {
            USHORT nSubType = TIMEFLD, nWhich = RES_TIMEFLD;
            ULONG nFormat = TF_SSMM_24;
            sw3io_ConvertFromOldField( *pDoc, nWhich, nSubType, nFormat, 0x0110 );
            SwDateTimeField aTimeFld( (SwDateTimeFieldType*)
                    pDoc->GetSysFldType( RES_DATETIMEFLD ), TIMEFLD, nFormat);
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtFld( aTimeFld ));
        }
        break;

    case RTF_CHPGN:
        {
            SwPageNumberField aPageFld( (SwPageNumberFieldType*)
                                    pDoc->GetSysFldType( RES_PAGENUMBERFLD ),
                                    PG_RANDOM, SVX_NUM_ARABIC );
            CheckInsNewTblLine();
            pDoc->Insert( *pPam, SwFmtFld( aPageFld));
        }
        break;

    case RTF_CHFTN:
        bFootnoteAutoNum = TRUE;
        break;

    case RTF_NOFPAGES:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nPage = (USHORT)nTokenValue;
        break;

    case RTF_NOFWORDS:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nWord = (USHORT)nTokenValue;
        break;
    case RTF_NOFCHARS:
        if( IsNewDoc() && nTokenValue && -1 != nTokenValue )
            ((SwDocStat&)pDoc->GetDocStat()).nChar = (USHORT)nTokenValue;
        break;

    case RTF_U:
        {
            CheckInsNewTblLine();
            if( nTokenValue )
                aToken = (sal_Unicode )nTokenValue;
            pDoc->Insert( *pPam, aToken );
        }
        break;

//  case RTF_REVDTTM:
//      UnknownAttrToken( nToken, &GetAttrSet() );
//      break;


// RTF_SUBENTRYINDEX

    default:
        switch( nToken & ~(0xff | RTF_SWGDEFS) )
        {
        case RTF_DOCFMT:
            ReadDocControls( nToken );
            break;
        case RTF_SECTFMT:
            ReadSectControls( nToken );
            break;
        case RTF_APOCTL:
            ReadFly( nToken );
            break;
        case RTF_BRDRDEF | RTF_TABLEDEF:
        case RTF_SHADINGDEF | RTF_TABLEDEF:
        case RTF_TABLEDEF:
            ReadTable( nToken );
            break;

        case RTF_INFO:
            ReadInfo();
            break;

        default:
            if( USHRT_MAX != nInsTblRow &&
                (nInsTblRow > GetOpenBrakets() || IsPardTokenRead() ))
                nInsTblRow = USHRT_MAX;

            SvxRTFParser::NextToken( nToken );
            break;
        }
    }
    if( USHRT_MAX != nInsTblRow &&
        (nInsTblRow > GetOpenBrakets() || IsPardTokenRead() ))
        nInsTblRow = USHRT_MAX;
}


void SwRTFParser::InsertText()
{
    DUMP_TXT( *pPam, aToken )
    // dann fuege den String ein, ohne das Attribute am Ende
    // aufgespannt werden.
    CheckInsNewTblLine();
    pDoc->Insert( *pPam, aToken );
}


void SwRTFParser::InsertPara()
{
    CheckInsNewTblLine();
    pDoc->AppendTxtNode(*pPam->GetPoint());

    // setze das default Style
    if( !bStyleTabValid )
        MakeStyleTab();

    SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
    if( !pColl )
        pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    pDoc->SetTxtFmtColl( *pPam, pColl );

    ::SetProgressState( rInput.Tell(), pDoc->GetDocShell() );
}



void SwRTFParser::MovePos( int bForward )
{
    if( bForward )
        pPam->Move( fnMoveForward );
    else
        pPam->Move( fnMoveBackward );
}

int SwRTFParser::IsEndPara( SvxNodeIdx* pNd, xub_StrLen nCnt ) const
{
    SwCntntNode *pNode = pDoc->GetNodes()[pNd->GetIdx()]->GetCntntNode();
    return pNode && pNode->Len() == nCnt;
}

void SwRTFParser::SetEndPrevPara( SvxNodeIdx*& rpNodePos, xub_StrLen& rCntPos )
{
    SwNodeIndex aIdx( pPam->GetPoint()->nNode );
    SwCntntNode* pNode = pDoc->GetNodes().GoPrevious( &aIdx );
    if( !pNode )
        ASSERT( FALSE, "keinen vorherigen ContentNode gefunden" );

    rpNodePos = new SwNodeIdx( aIdx );
    rCntPos = pNode->Len();
}

void SwRTFParser::SetAttrInDoc( SvxRTFItemStackType &rSet )
{
    ULONG nSNd = rSet.GetSttNodeIdx(), nENd = rSet.GetEndNodeIdx();
    xub_StrLen nSCnt = rSet.GetSttCnt(), nECnt = rSet.GetEndCnt();

    SwPaM aPam( *pPam->GetPoint() );

#ifndef PRODUCT
    ASSERT( nSNd <= nENd, "Start groesser als Ende" );
    SwNode* pDebugNd = pDoc->GetNodes()[ nSNd ];
    ASSERT( pDebugNd->IsCntntNode(), "Start kein ContentNode" );
    pDebugNd = pDoc->GetNodes()[ nENd ];
    ASSERT( pDebugNd->IsCntntNode(), "Ende kein ContentNode" );
#endif

    SwCntntNode* pCNd = pDoc->GetNodes()[ nSNd ]->GetCntntNode();
    aPam.GetPoint()->nNode = nSNd;
    aPam.GetPoint()->nContent.Assign( pCNd, nSCnt );
    aPam.SetMark();
    if( nENd == nSNd )
        aPam.GetPoint()->nContent = nECnt;
    else
    {
        aPam.GetPoint()->nNode = nENd;
        pCNd = aPam.GetCntntNode();
        aPam.GetPoint()->nContent.Assign( pCNd, nECnt );
    }

    // setze ueber den Bereich das entsprechende Style
    if( rSet.StyleNo() )
    {
        // setze jetzt das Style
        if( !bStyleTabValid )
            MakeStyleTab();
        SwTxtFmtColl* pColl = aTxtCollTbl.Get( rSet.StyleNo() );
        if( pColl )
        {
            pDoc->SetTxtFmtColl( aPam, pColl, FALSE );
            DUMP_STYLE( aPam, *pColl )
        }
    }

    const SfxPoolItem* pItem;
    if( rSet.GetAttrSet().Count() )
    {

        // falls eine Zeichenvorlage im Set steht, deren Attribute
        // aus dem Set loeschen. Sonst sind diese doppelt, was man ja
        // nicht will.
        if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
            RES_TXTATR_CHARFMT, FALSE, &pItem ) &&
            ((SwFmtCharFmt*)pItem)->GetCharFmt() )
        {
            const String& rName = ((SwFmtCharFmt*)pItem)->GetCharFmt()->GetName();
            SvxRTFStyleType* pStyle = GetStyleTbl().First();
            do {
                if( pStyle->bIsCharFmt && pStyle->sName == rName )
                {
                    // alle Attribute, die schon vom Style definiert sind, aus dem
                    // akt. AttrSet entfernen
                    SfxItemSet &rAttrSet = rSet.GetAttrSet(),
                               &rStyleSet = pStyle->aAttrSet;
                    SfxItemIter aIter( rAttrSet );
                    USHORT nWhich = aIter.GetCurItem()->Which();
                    while( TRUE )
                    {
                        if( SFX_ITEM_SET == rStyleSet.GetItemState(
                            nWhich, FALSE, &pItem ) && *pItem == *aIter.GetCurItem())
                            rAttrSet.ClearItem( nWhich );       // loeschen

                        if( aIter.IsAtEnd() )
                            break;
                        nWhich = aIter.NextItem()->Which();
                    }
                    break;
                }
            } while( 0 != (pStyle = GetStyleTbl().Next()) );
        }
        // dann setze ueber diesen Bereich die Attrbiute
        SetSwgValues( rSet.GetAttrSet() );


#if 0
///!!!!!!!!!!!!!!!!!!!!!
        if( !rSet.GetAttrSet().GetParent() )
        {
            const SfxItemPool& rPool = pDoc->GetAttrPool();
            SfxItemSet &rAttrSet = rSet.GetAttrSet();
            SfxItemIter aIter( rAttrSet );
            USHORT nWhich = aIter.GetCurItem()->Which();
            while( TRUE )
            {
                if( SFX_WHICH_MAX < nWhich ||
                    rPool.GetDefaultItem( nWhich ) == *aIter.GetCurItem() )
                    rAttrSet.ClearItem( nWhich );       // loeschen

                if( aIter.IsAtEnd() )
                    break;
                nWhich = aIter.NextItem()->Which();
            }
        }
        if( rSet.GetAttrSet().Count() )
#endif

        pDoc->Insert( aPam, rSet.GetAttrSet(), SETATTR_DONTCHGNUMRULE );
        DUMP_SET( aPam, rSet.GetAttrSet() )
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
        FN_PARAM_NUM_LEVEL, FALSE, &pItem ))
    {
        // dann ueber den Bereich an den Nodes das NodeNum setzen
        for( ULONG n = nSNd; n <= nENd; ++n )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
            if( pTxtNd )
            {
                pTxtNd->UpdateNum( SwNodeNum( (BYTE)
                            ((SfxUInt16Item*)pItem)->GetValue() ));
                // Update vom LR-Space abschalten?
            }
        }
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(
        RES_PARATR_NUMRULE, FALSE, &pItem ))
    {
        const SwNumRule* pRule = pDoc->FindNumRulePtr(
                                    ((SwNumRuleItem*)pItem)->GetValue() );
        if( pRule && ( pRule->IsContinusNum() || !bNewNumList ))
        {
            // diese Rule hat keinen Level, also muss die Einrueckung
            // erhalten bleiben!
            // dann ueber den Bereich an den Nodes das Flag zuruecksetzen
            SwNodeNum aNdNum( 0 );
            for( ULONG n = nSNd; n <= nENd; ++n )
            {
                SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
                if( pTxtNd )
                {
                    if( !pTxtNd->GetNum() )
                        pTxtNd->UpdateNum( aNdNum );
                    // Update vom LR-Space abschalten
                    pTxtNd->SetNumLSpace( FALSE );
                }
            }
        }
    }
}


// lese alle Dokument-Controls ein
void SwRTFParser::ReadDocControls( int nToken )
{
    int bWeiter = TRUE;

    SwFmtFrmSize aFrmSize(ATT_FIX_SIZE, 12240, 15840 );
    SvxULSpaceItem aUL;
    SvxLRSpaceItem aLR;
    SwFtnInfo aFtnInfo;
    SwEndNoteInfo aEndInfo;
    UseOnPage eUseOn;
    USHORT nPgStart = USHRT_MAX;
    if( bFirstDocControl )
    {
        // RTF-Defaults setzen
        SwFmtFrmSize aFrmSize(ATT_FIX_SIZE, 12240, 15840 );

        aUL.SetUpper( 1440 ); aUL.SetLower( 1440 );
        aLR.SetRight( 1800 ); aLR.SetLeft(  1800 );
        eUseOn = UseOnPage(PD_ALL | PD_HEADERSHARE | PD_FOOTERSHARE);
        aFtnInfo.ePos = FTNPOS_CHAPTER; aFtnInfo.eNum = FTNNUM_DOC;
        bFirstDocControl = FALSE;
    }
    else
    {
        const SwPageDesc& rStdPgDsc = pDoc->GetPageDesc( 0 );
        aFrmSize = rStdPgDsc.GetMaster().GetFrmSize();
        aUL = rStdPgDsc.GetMaster().GetULSpace();
        aLR = rStdPgDsc.GetMaster().GetLRSpace();

        eUseOn = rStdPgDsc.ReadUseOn();

        aEndInfo = pDoc->GetEndNoteInfo();
        aFtnInfo = pDoc->GetFtnInfo();
    }

    BOOL bEndInfoChgd = FALSE, bFtnInfoChgd = FALSE;

    do {
        USHORT nValue = USHORT( nTokenValue );
        switch( nToken )
        {
/*      case '}':       --nOpenBrakets; nToken = RTF_DOCFMT; break;
        case '{':       ++nOpenBrakets; nToken = RTF_DOCFMT; break;
        case RTF_IGNOREFLAG:            nToken = RTF_DOCFMT; break;
*/
        case RTF_PAPERW:
            if( 0 < nTokenValue )
                aFrmSize.SetWidth( nTokenValue );
            break;
        case RTF_PAPERH:
            if( 0 < nTokenValue )
                aFrmSize.SetHeight( nTokenValue );
            break;
        case RTF_MARGL:
            if( 0 <= nTokenValue )
                aLR.SetLeft( nValue );
            break;
        case RTF_MARGR:
            if( 0 <= nTokenValue )
                aLR.SetRight( nValue );
            break;
        case RTF_MARGT:
            if( 0 <= nTokenValue )
                aUL.SetUpper( nValue );
            break;
        case RTF_MARGB:
            if( 0 <= nTokenValue )
                aUL.SetLower( nValue );
            break;

        case RTF_FACINGP:   eUseOn = UseOnPage(PD_MIRROR | PD_HEADERSHARE | PD_FOOTERSHARE);
                            break;
        case RTF_PGNSTART:  nPgStart = nValue;      break;

        case RTF_ENDDOC:
        case RTF_ENDNOTES:
            aFtnInfo.ePos = FTNPOS_CHAPTER; bFtnInfoChgd = TRUE;
            break;
        case RTF_FTNTJ:
        case RTF_FTNBJ:
            aFtnInfo.ePos = FTNPOS_PAGE; bFtnInfoChgd = TRUE;
            break;

        case RTF_AENDDOC:
        case RTF_AENDNOTES:
        case RTF_AFTNTJ:
        case RTF_AFTNBJ:
        case RTF_AFTNRESTART:
        case RTF_AFTNRSTCONT:
            break;      // wir kenn nur am Doc Ende und Doc weite Num.!

        case RTF_FTNSTART:
            if( nValue )
            {
                aFtnInfo.nFtnOffset = nValue-1;
                bFtnInfoChgd = TRUE;
            }
            break;
        case RTF_AFTNSTART:
            if( nValue )
            {
                aEndInfo.nFtnOffset = nValue-1;
                bEndInfoChgd = TRUE;
            }
            break;
        case RTF_FTNRSTPG:
            aFtnInfo.eNum = FTNNUM_PAGE; bFtnInfoChgd = TRUE;
            break;
        case RTF_FTNRESTART:
            aFtnInfo.eNum = FTNNUM_CHAPTER; bFtnInfoChgd = TRUE;
            break;
        case RTF_FTNRSTCONT:
            aFtnInfo.eNum = FTNNUM_DOC; bFtnInfoChgd = TRUE;
            break;

        case RTF_FTNNAR:
            aFtnInfo.aFmt.eType = SVX_NUM_ARABIC; bFtnInfoChgd = TRUE; break;
        case RTF_FTNNALC:
            aFtnInfo.aFmt.eType = SVX_NUM_CHARS_LOWER_LETTER_N; bFtnInfoChgd = TRUE; break;
        case RTF_FTNNAUC:
            aFtnInfo.aFmt.eType = SVX_NUM_CHARS_UPPER_LETTER_N; bFtnInfoChgd = TRUE; break;
        case RTF_FTNNRLC:
            aFtnInfo.aFmt.eType = SVX_NUM_ROMAN_LOWER; bFtnInfoChgd = TRUE; break;
        case RTF_FTNNRUC:
            aFtnInfo.aFmt.eType = SVX_NUM_ROMAN_UPPER; bFtnInfoChgd = TRUE; break;
        case RTF_FTNNCHI:
            aFtnInfo.aFmt.eType = SVX_NUM_CHAR_SPECIAL; bFtnInfoChgd = TRUE; break;

        case RTF_AFTNNAR:
            aEndInfo.aFmt.eType = SVX_NUM_ARABIC; bEndInfoChgd = TRUE; break;
        case RTF_AFTNNALC:
            aEndInfo.aFmt.eType = SVX_NUM_CHARS_LOWER_LETTER_N; bEndInfoChgd = TRUE; break;
        case RTF_AFTNNAUC:
            aEndInfo.aFmt.eType = SVX_NUM_CHARS_UPPER_LETTER_N; bEndInfoChgd = TRUE; break;
        case RTF_AFTNNRLC:
            aEndInfo.aFmt.eType = SVX_NUM_ROMAN_LOWER; bEndInfoChgd = TRUE; break;
        case RTF_AFTNNRUC:
            aEndInfo.aFmt.eType = SVX_NUM_ROMAN_UPPER; bEndInfoChgd = TRUE; break;
        case RTF_AFTNNCHI:
            aEndInfo.aFmt.eType = SVX_NUM_CHAR_SPECIAL; bEndInfoChgd = TRUE; break;


        case '{':
            {
                short nSkip = 0;
                if( RTF_IGNOREFLAG != GetNextToken() )
                    nSkip = -1;
                else if( RTF_DOCFMT != (( nToken = GetNextToken() )
                        & ~(0xff | RTF_SWGDEFS)) )
                    nSkip = -2;
                else
                {
                    SkipGroup();        // erstmal komplett ueberlesen
                    // ueberlese noch die schliessende Klammer
                    GetNextToken();
                }
                if( nSkip )
                {
                    SkipToken( nSkip );     // Ignore wieder zurueck
                    bWeiter = FALSE;
                }
            }
            break;

        default:
            if( RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                RTF_UNKNOWNCONTROL == nToken )
                SvxRTFParser::NextToken( nToken );
            else
                bWeiter = FALSE;
            break;
        }
        if( bWeiter )
            nToken = GetNextToken();
    } while( bWeiter && IsParserWorking() );

    if( IsNewDoc() )
    {
        if( bEndInfoChgd )
            pDoc->SetEndNoteInfo( aEndInfo );
        if( bFtnInfoChgd )
            pDoc->SetFtnInfo( aFtnInfo );
    }

    if( !bSwPageDesc )
    {
        if( IsNewDoc() )
        {
            // direkt an der Standartseite drehen
            SwPageDesc& rPg = pDoc->_GetPageDesc( 0 );
            rPg.WriteUseOn( eUseOn );
            SwFrmFmt &rFmt1 = rPg.GetMaster(), &rFmt2 = rPg.GetLeft();

            rFmt1.SetAttr( aFrmSize );  rFmt2.SetAttr( aFrmSize );
            rFmt1.SetAttr( aLR );       rFmt2.SetAttr( aLR );
            rFmt1.SetAttr( aUL );       rFmt2.SetAttr( aUL );

            // StartNummer der Seiten setzen
            if( USHRT_MAX != nPgStart )
            {
                const SfxPoolItem* pItem;
                const SwCntntNode* pNd = pPam->GetCntntNode();
                if( pNd && pNd->GetpSwAttrSet() && SFX_ITEM_SET ==
                    pNd->GetpSwAttrSet()->GetItemState( RES_PAGEDESC,
                    FALSE, &pItem ) )
                {
                    // set a new PageNum
                    ((SwFmtPageDesc*)pItem)->SetNumOffset( nPgStart );
                }
                else
                {
                    SwFmtPageDesc aPgDsc( &rPg );
                    aPgDsc.SetNumOffset( nPgStart );
                    pDoc->Insert( *pPam, aPgDsc );
                }
            }
        }

        MakeStyleTab();
        // das default-Style schon gleich am ersten Node setzen
//          if( IsNewDoc() )
        {
            SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
            if( !pColl )
                pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
            pDoc->SetTxtFmtColl( *pPam, pColl );
        }
    }

    SkipToken( -1 );
}

void SwRTFParser::MakeStyleTab()
{
    // dann erzeuge aus der SvxStyle-Tabelle die Swg-Collections
    if( GetStyleTbl().Count() )
    {
        USHORT nValidOutlineLevels = 0;
        if( !IsNewDoc() )
        {
            // search all outlined collections
            BYTE nLvl;
            const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
            for( USHORT n = rColls.Count(); n; )
                if( MAXLEVEL > (nLvl = rColls[ --n ]->GetOutlineLevel() ))
                    nValidOutlineLevels |= 1 << nLvl;
        }

        SvxRTFStyleType* pStyle = GetStyleTbl().First();
        do {
            USHORT nNo = USHORT( GetStyleTbl().GetCurKey() );
            if( pStyle->bIsCharFmt )
            {
                if( !aCharFmtTbl.Get( nNo ) )
                    // existiert noch nicht, also anlegen
                    MakeCharStyle( nNo, *pStyle );
            }
            else if( !aTxtCollTbl.Get( nNo ) )
            {
                // existiert noch nicht, also anlegen
                if( MAXLEVEL > pStyle->nOutlineNo )
                {
                    USHORT nOutlFlag = 1 << pStyle->nOutlineNo;
                    if( nValidOutlineLevels & nOutlFlag )
                        pStyle->nOutlineNo = (BYTE)-1;
                    else
                        nValidOutlineLevels |= nOutlFlag;
                }

                MakeStyle( nNo, *pStyle );
            }

        } while( 0 != (pStyle = GetStyleTbl().Next()) );
        bStyleTabValid = TRUE;
    }
}

SwPageDesc* SwRTFParser::_MakeNewPageDesc( int bFirst )
{
    USHORT* pNo = &nAktPageDesc;
    String aNm( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "RTF-SectionPage(" )));

    if( bFirst )
    {
        aNm.InsertAscii( "First", 4 );
        pNo = &nAktFirstPageDesc;
    }
    aNm += String::CreateFromInt32( pDoc->GetPageDescCnt() );
    aNm += ')';

    USHORT nNew = pDoc->MakePageDesc( aNm, 0 );
    SwPageDesc& rAkt = pDoc->_GetPageDesc( nNew );
    SwPageDesc& rOld = pDoc->_GetPageDesc( *pNo );
    pDoc->CopyPageDesc( rOld, rAkt, FALSE );

    *pNo = nNew;

    // setze den Follow-PageDesc immer auf sich selbst
    rAkt.SetFollow( &rAkt );

    // falls auf \\page ein \\setd folgt, dann muss noch ein neuer
    // Absatz eingefuegt werden.
    SwCntntNode* pCNd = pPam->GetCntntNode();
    ASSERT( pCNd, "wo ist mein Content-Node" );
    if( SFX_ITEM_SET == pCNd->GetSwAttrSet().GetItemState( RES_BREAK, FALSE )
        // sollten nicht am NodeAnfang stehen, erzeuge einen neuen Absatz
        || pPam->GetPoint()->nContent.GetIndex() )
        InsertPara();

    return &rAkt;
}

BOOL lcl_CompareRTFPageDesc( const SwPageDesc& rOld, const SwPageDesc& rAkt )
{
    BOOL bRet = /*rAkt.ReadUseOn() == rOld.ReadUseOn() && Bug 63599 */
                rAkt.GetLandscape() == rOld.GetLandscape() &&
                rAkt.GetNumType().eType == rOld.GetNumType().eType;

    if( bRet )
    {
        // dann ein paar Attribute vergleichen
        USHORT __READONLY_DATA aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
                                            RES_BACKGROUND, RES_SHADOW,
                                            0 };
        const SfxPoolItem* pOldItem, *pAktItem;
        for( USHORT n = 0; aIdArr[ n ] && bRet; n += 2 )
        {
            for( USHORT nId = aIdArr[ n ]; nId <= aIdArr[ n+1]; ++nId )
            {
                int eOldSt = rOld.GetMaster().GetItemState( nId, FALSE, &pOldItem ),
                    eAktSt = rAkt.GetMaster().GetItemState( nId, FALSE, &pAktItem );
                if( eOldSt != eAktSt ||
                    ( SFX_ITEM_SET == eOldSt && *pOldItem != *pAktItem ))
                {
                    bRet = FALSE;
                    break;
                }
            }
        }
    }
    return bRet;
}

BOOL lcl_SetFmtCol( SwFmt& rFmt, USHORT nCols, USHORT nColSpace,
                    const SvUShorts& rColumns )
{
    BOOL bSet = FALSE;
    if( nCols && USHRT_MAX != nCols )
    {
        SwFmtCol aCol;
        if( USHRT_MAX == nColSpace )
            nColSpace = 720;

        aCol.Init( nCols, nColSpace, USHRT_MAX );
        if( nCols == ( rColumns.Count() / 2 ) )
        {
            aCol._SetOrtho( FALSE );
            USHORT nWishWidth = 0, nHalfPrev = 0;
            for( USHORT n = 0, i = 0; n < rColumns.Count(); n += 2, ++i )
            {
                SwColumn* pCol = aCol.GetColumns()[ i ];
                pCol->SetLeft( nHalfPrev );
                USHORT nSp = rColumns[ n+1 ];
                nHalfPrev = nSp / 2;
                pCol->SetRight( nSp - nHalfPrev );
                pCol->SetWishWidth( rColumns[ n ] +
                                    pCol->GetLeft() + pCol->GetRight() );
                nWishWidth += pCol->GetWishWidth();
            }
            aCol.SetWishWidth( nWishWidth );
        }
        rFmt.SetAttr( aCol );
        bSet = TRUE;
    }
    return bSet;
}

// lese alle Section-Controls ein
void SwRTFParser::ReadSectControls( int nToken )
{
    int bWeiter = TRUE;

    // werden die eigenen PageDesc - Informationen gelesen, dann alles
    // ueberlesen
    // Wird in ein bestehendes Doc eingelesen oder wurden eigene PageDesc
    // gelesen, dann sind nur die Bereiche von interresse
    BOOL bInsPageDesc = IsNewDoc() && !bSwPageDesc;

    if( bInPgDscTbl )
    {
        do {
            if( '{' == nToken )
            {
                switch( nToken = GetNextToken() )
                {
                case RTF_IGNOREFLAG:
                    if( RTF_SECTFMT != (( nToken = GetNextToken() )
                        & ~(0xff | RTF_SWGDEFS)) )
                    {
                        SkipToken( -2 );    // Ignore und Token wieder zurueck
                        bWeiter = FALSE;
                        break;
                    }
                    // kein break, Gruppe ueberspringen

                case RTF_FOOTER:
                case RTF_HEADER:
                case RTF_FOOTERR:
                case RTF_HEADERR:
                case RTF_FOOTERL:
                case RTF_HEADERL:
                case RTF_FOOTERF:
                case RTF_HEADERF:
                    SkipGroup();        // erstmal komplett ueberlesen
                    // ueberlese noch die schliessende Klammer
                    GetNextToken();
                    break;

                default:
                    SkipToken( -1 );            // Ignore wieder zurueck
                    bWeiter = FALSE;
                    break;
                }
            }
            else if( RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                RTF_UNKNOWNCONTROL == nToken )
                SvxRTFParser::NextToken( nToken );
            else
                bWeiter = FALSE;
            if( bWeiter )
                nToken = GetNextToken();
        } while( bWeiter && IsParserWorking() );
        SkipToken( -1 );                    // letztes Token wieder zurueck
        return;
    }

    // RTF-Defaults setzen
    BOOL bFirstCall = 1 == pDoc->GetPageDescCnt();
    BOOL bHeaderUL = FALSE, bHeaderLR = FALSE,
        bFooterUL = FALSE, bFooterLR = FALSE;

    USHORT nLastPageDesc = nAktPageDesc,
            nLastFirstPageDesc = nAktFirstPageDesc;

    BOOL bMakeNewPageDesc = RTF_SECT == nToken;
    BOOL bInsNewPageDesc = bMakeNewPageDesc || bFirstCall || bSwPageDesc;
    SwPageDesc* pAkt, *pFirst = 0;
    if( bInsNewPageDesc )
    {
        pAkt = _MakeNewPageDesc( FALSE );
        if( bMakeNewPageDesc )
        {
            nToken = GetNextToken();
            // some export filter write the RTF_SECT in brakets -> skip over
            while( '}' == nToken )
            {
                NextToken( nToken );
                nToken = GetNextToken();
            }
        }
    }
    else
    {
        pAkt = &pDoc->_GetPageDesc( nAktPageDesc );
        if( nAktFirstPageDesc )
            pFirst = &pDoc->_GetPageDesc( nAktFirstPageDesc );
    }

    SwFmtFrmSize aSz( pAkt->GetMaster().GetFrmSize() ),
                aHSz( ATT_MIN_SIZE );
    SvxULSpaceItem aUL( pAkt->GetMaster().GetULSpace() ), aHUL, aFUL;
    SvxLRSpaceItem aLR( pAkt->GetMaster().GetLRSpace() ), aHLR, aFLR;
    BOOL    bPgWiChgd = FALSE, bPgHeChgd = FALSE,
            bPgUpChgd = FALSE, bPgLoChgd = FALSE,
            bPgLeChgd = FALSE, bPgRiChgd = FALSE,
            bPgFcChgd = FALSE,
            bNoBalancedCols = FALSE;

    USHORT nRestoreUpper = USHRT_MAX, nRestoreLower = USHRT_MAX;
    SwFrmFmt* pFmt = (SwFrmFmt*)pAkt->GetMaster().GetHeader().GetHeaderFmt();
    if( pFmt )
    {
        aHUL = pFmt->GetULSpace();
        aHLR = pFmt->GetLRSpace();
        aHSz = pFmt->GetFrmSize();
        if( aHSz.GetHeight() )
        {
            nRestoreUpper = (USHORT)aHSz.GetHeight();
            aUL.SetUpper( aUL.GetUpper() + nRestoreUpper );
            aHSz.SetHeight( 0 );
        }
    }

    if( 0 != (pFmt = (SwFrmFmt*)pAkt->GetMaster().GetFooter().GetFooterFmt()) )
    {
        aFUL = pFmt->GetULSpace();
        aFLR = pFmt->GetLRSpace();
        if( aFUL.GetUpper() )
        {
            nRestoreLower = aFUL.GetUpper();
            aUL.SetLower( aUL.GetLower() + nRestoreLower  );
            aFUL.SetUpper( 0 );
        }
    }

    BOOL bCheckEqualPgDesc = TRUE, bPgDescChgd = FALSE;
    SvxBreak eBreak = SVX_BREAK_PAGE_BEFORE;

    USHORT nCols = USHRT_MAX, nColSpace = USHRT_MAX, nAktCol = 0;
    SvUShorts aColumns;

    USHORT nPgStart = USHRT_MAX;
    SwNumType aNumType;

    do {
        BOOL bIsSectToken = FALSE;
        USHORT nValue = USHORT( nTokenValue );
        switch( nToken )
        {
        case RTF_SECT:
            bWeiter = FALSE;
            break;

        case RTF_SECTD:
            {
                // Wert von der StandardPage holen !
                const SwFrmFmt& rFmt = pDoc->GetPageDesc(0).GetMaster();
                aSz = rFmt.GetFrmSize();
                aUL = rFmt.GetULSpace();
                aLR = rFmt.GetLRSpace();

                nRestoreUpper = USHRT_MAX, nRestoreLower = USHRT_MAX;
                aHUL.SetUpper( 720 );   aHUL.SetLower( 0 );
                aFUL.SetUpper( 0 );     aFUL.SetLower( 720 );
                aHLR.SetLeft( 0 );      aHLR.SetRight( 0 );
                aFLR.SetLeft( 0 );      aFLR.SetRight( 0 );
                pAkt->WriteUseOn( UseOnPage(PD_ALL | PD_HEADERSHARE | PD_FOOTERSHARE) );
                pAkt->SetLandscape( FALSE );

                // remove Columns/Header/Footer
                pAkt->GetMaster().ResetAttr( RES_COL );
                if( pFirst )
                    pFirst->GetMaster().ResetAttr( RES_COL );

                nPgStart = USHRT_MAX;
                nCols = USHRT_MAX;
                nColSpace = USHRT_MAX;
                aNumType.eType = SVX_NUM_ARABIC;
                bHeaderUL = bHeaderLR = bFooterUL = bFooterLR = TRUE;
                eBreak = SVX_BREAK_PAGE_BEFORE;
                nNewNumSectDef = USHRT_MAX;

                bPgWiChgd = bPgHeChgd = bPgUpChgd = bPgLoChgd = bPgLeChgd =
                    bPgRiChgd = bPgFcChgd = TRUE;
            bPgDescChgd = TRUE;
            }
            break;

        case RTF_PGWSXN:
            if( 0 < nTokenValue )
            {
                aSz.SetWidth( nTokenValue );
                bPgWiChgd = TRUE;
                bPgDescChgd = TRUE;
            }
            break;

        case RTF_PGHSXN:
            if( 0 < nTokenValue )
            {
                aSz.SetHeight( nTokenValue );
                bPgHeChgd = TRUE;
                bPgDescChgd = TRUE;
            }
            break;
        case RTF_MARGLSXN:
            if( 0 <= nTokenValue )
            {
                aLR.SetLeft( nValue );
                bPgLeChgd =TRUE;
                bPgDescChgd = TRUE;
            }
            break;
        case RTF_MARGRSXN:
            if( 0 <= nTokenValue )
            {
                aLR.SetRight( nValue );
                bPgRiChgd = TRUE;
                bPgDescChgd = TRUE;
            }
            break;

        case RTF_MARGTSXN:
            if( 0 <= nTokenValue )
            {
                nRestoreUpper = USHRT_MAX;
                aUL.SetUpper( nValue );
                bPgUpChgd = TRUE;
                bPgDescChgd = TRUE;
            }
            break;

        case RTF_MARGBSXN:
            if( 0 <= nTokenValue )
            {
                nRestoreLower = USHRT_MAX;
                aUL.SetLower( nValue );
                bPgLoChgd = TRUE;
                bPgDescChgd = TRUE;
            }
            break;

        case RTF_FACPGSXN:
            pAkt->SetUseOn( UseOnPage(PD_MIRROR | ( pAkt->GetUseOn() &
                                ~(PD_HEADERSHARE | PD_FOOTERSHARE) )));
            bPgDescChgd = TRUE;
            break;

        case RTF_HEADERY:
            aHUL.SetUpper( nValue );
            bHeaderUL = TRUE;
            bPgDescChgd = TRUE;
            break;

        case RTF_FOOTERY:
            aFUL.SetLower( nValue );
            bFooterUL = TRUE;
            bPgDescChgd = TRUE;
            break;

        case RTF_LNDSCPSXN:
            pAkt->SetLandscape( TRUE );
            bPgFcChgd = TRUE;
            bPgDescChgd = TRUE;
            break;

        case RTF_PGNSTARTS:
            nPgStart = nValue;
            bPgDescChgd = TRUE;
            break;

        case RTF_PGNDEC:
            aNumType.eType = SVX_NUM_ARABIC;
            bPgDescChgd = TRUE;
            break;
        case RTF_PGNUCRM:
            aNumType.eType = SVX_NUM_ROMAN_UPPER;
            bPgDescChgd = TRUE;
            break;
        case RTF_PGNLCRM:
            aNumType.eType = SVX_NUM_ROMAN_LOWER;
            bPgDescChgd = TRUE;
            break;
        case RTF_PGNUCLTR:
            aNumType.eType = SVX_NUM_CHARS_UPPER_LETTER_N;
            bPgDescChgd = TRUE;
            break;
        case RTF_PGNLCLTR:
            aNumType.eType = SVX_NUM_CHARS_LOWER_LETTER_N;
            bPgDescChgd = TRUE;
            break;

        case RTF_SBKNONE:
            bIsSectToken = TRUE;
            eBreak = SVX_BREAK_NONE;
            bPgDescChgd = !bFirstCall;
            break;

        case RTF_SBKCOL:
            eBreak = SVX_BREAK_COLUMN_BEFORE;
            bPgDescChgd = TRUE;
            break;

        case RTF_ENDNHERE:
        case RTF_BINFSXN:
        case RTF_BINSXN:
        case RTF_SBKPAGE:
        case RTF_SBKEVEN:
        case RTF_SBKODD:
        case RTF_LINEBETCOL:
        case RTF_LINEMOD:
        case RTF_LINEX:
        case RTF_LINESTARTS:
        case RTF_LINERESTART:
        case RTF_LINEPAGE:
        case RTF_LINECONT:
        case RTF_GUTTERSXN:
        case RTF_PGNCONT:
        case RTF_PGNRESTART:
        case RTF_PGNX:
        case RTF_PGNY:
        case RTF_VERTALT:
        case RTF_VERTALB:
        case RTF_VERTALC:
        case RTF_VERTALJ:
            break;

        case RTF_TITLEPG:
            if( bInsPageDesc && !pFirst )
                pFirst = _MakeNewPageDesc( TRUE );
            bCheckEqualPgDesc = FALSE;
            bPgDescChgd = TRUE;
            break;

        case RTF_HEADER:
        case RTF_HEADERL:
        case RTF_HEADERR:
        case RTF_FOOTER:
        case RTF_FOOTERL:
        case RTF_FOOTERR:
            if( bInsPageDesc )
                ReadHeaderFooter( nToken, pAkt );
            else
                SkipGroup(), GetNextToken();    //Gruppe mit schl. Klammer lesen
            bCheckEqualPgDesc = FALSE;
            bPgDescChgd = TRUE;
            break;

        case RTF_FOOTERF:
        case RTF_HEADERF:
            if( bInsPageDesc )
            {
                if( !pFirst )
                    pFirst = _MakeNewPageDesc( TRUE );
                ReadHeaderFooter( nToken, pFirst );
            }
            else
                SkipGroup(), GetNextToken();    //Gruppe mit schl. Klammer lesen
            bCheckEqualPgDesc = FALSE;
            bPgDescChgd = TRUE;
            break;

        case RTF_COLS:
            bIsSectToken = TRUE;
            nCols = nValue;
            bPgDescChgd = TRUE;
            break;

        case RTF_COLSX:
            bIsSectToken = TRUE;
            nColSpace = nValue;
            bPgDescChgd = TRUE;
            break;

        case RTF_COLNO:
            bIsSectToken = TRUE;
            nAktCol = nValue;
            if( RTF_COLW == GetNextToken() )
            {
                USHORT nWidth = USHORT( nTokenValue ), nSpace = 0;
                if( RTF_COLSR == GetNextToken() )
                    nSpace = USHORT( nTokenValue );
                else
                    SkipToken( -1 );        // wieder zurueck

                if( --nAktCol == ( aColumns.Count() / 2 ) )
                {
                    aColumns.Insert( nWidth, aColumns.Count() );
                    aColumns.Insert( nSpace, aColumns.Count() );
                }
            }
            bPgDescChgd = TRUE;
            break;


        case '{':
            {
                short nSkip = 0;
                if( RTF_IGNOREFLAG != ( nToken = GetNextToken() ))
                    nSkip = -1;
                else if( RTF_SECTFMT != (( nToken = GetNextToken() )
                         & ~(0xff | RTF_SWGDEFS)) &&
                        ( RTF_DOCFMT != ( nToken & ~(0xff | RTF_SWGDEFS))) )
                    nSkip = -2;
                else
                {
                    bPgDescChgd = TRUE;
                    switch( nToken )
                    {
                    case RTF_HEADER_YB:
                        aHUL.SetLower( USHORT(nTokenValue) );
                        bHeaderUL = TRUE;
                        if( RTF_HEADER_XL == GetNextToken() &&
                            RTF_HEADER_XR == GetNextToken() )
                        {
                            bHeaderLR = TRUE;
                            aHLR.SetLeft( USHORT(GetStackPtr( -1 )->nTokenValue) );
                            aHLR.SetRight( USHORT(nTokenValue));
                        }
                        else
                            SkipGroup();
                        break;

                    case RTF_FOOTER_YT:
                        aFUL.SetUpper( USHORT(nTokenValue));
                        bFooterUL = TRUE;
                        if( RTF_FOOTER_XL == GetNextToken() &&
                            RTF_FOOTER_XR == GetNextToken() )
                        {
                            bFooterLR = TRUE;
                            aFLR.SetLeft( USHORT(GetStackPtr( -1 )->nTokenValue) );
                            aFLR.SetRight( USHORT(nTokenValue));
                        }
                        else
                            SkipGroup();
                        break;

                    case RTF_BALANCED_COLUMN:
                        bIsSectToken = TRUE;
                        bPgDescChgd = TRUE;
                        bNoBalancedCols = TRUE;
                        break;

                    default:
                        SkipGroup();        // erstmal komplett ueberlesen
                        break;
                    }

                    // ueberlese noch die schliessende Klammer
                    GetNextToken();
                }
                if( nSkip )
                {
                    bWeiter = -1 == nSkip && (
                            RTF_FOOTER == nToken || RTF_HEADER == nToken ||
                            RTF_FOOTERR == nToken || RTF_HEADERR == nToken ||
                            RTF_FOOTERL == nToken || RTF_HEADERL == nToken ||
                            RTF_FOOTERF == nToken || RTF_HEADERF == nToken );
                    SkipToken( nSkip );     // Ignore wieder zurueck
                }
            }
            break;

        case RTF_PAPERW:
        case RTF_PAPERH:
        case RTF_MARGL:
        case RTF_MARGR:
        case RTF_MARGT:
        case RTF_MARGB:
        case RTF_FACINGP:
            {
                ReadDocControls( nToken );

                // set the new Values
                const SwPageDesc& rStdPgDsc = pDoc->GetPageDesc( 0 );
                const SwFmtFrmSize& rSz = rStdPgDsc.GetMaster().GetFrmSize();
                const SvxULSpaceItem rUL = rStdPgDsc.GetMaster().GetULSpace();
                const SvxLRSpaceItem rLR = rStdPgDsc.GetMaster().GetLRSpace();

                if( !bPgWiChgd ) aSz.SetWidth( rSz.GetWidth() );
                if( !bPgHeChgd ) aSz.SetHeight( rSz.GetHeight() );
                if( !bPgUpChgd ) aUL.SetUpper( rUL.GetUpper() );
                if( !bPgLoChgd ) aUL.SetLower( rUL.GetLower() );
                if( !bPgLeChgd ) aLR.SetLeft( rLR.GetLeft() );
                if( !bPgRiChgd ) aLR.SetRight( rLR.GetRight() );
                if( !bPgFcChgd ) pAkt->SetLandscape( rStdPgDsc.GetLandscape() );
            }
            break;
        default:
            if( RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)) )
                ReadDocControls( nToken );
            else if( RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                     RTF_UNKNOWNCONTROL == nToken )
                SvxRTFParser::NextToken( nToken );
            else
                bWeiter = FALSE;
            break;
        }


        if( bWeiter )
        {
            // kein eigener Bereich und kein Section Attribut?
            if( !bInsPageDesc && !bIsSectToken )
                bCheckEqualPgDesc = FALSE;
            nToken = GetNextToken();
        }
    } while( bWeiter && IsParserWorking() );


    if( !bPgDescChgd )
    {
        // der aktuelle muss wieder entnfernt werden
        if( bInsNewPageDesc )
        {
            pDoc->DelPageDesc( nAktPageDesc );
            if( pFirst )
                pDoc->DelPageDesc( nAktFirstPageDesc );
            nAktPageDesc = nLastPageDesc;
            nAktFirstPageDesc = nLastFirstPageDesc;
        }
        SkipToken( -1 );
        return ;
    }


    // den letzen Bereich wieder zumachen
    if( pRegionEndIdx )
    {
        DelLastNode();
        pPam->GetPoint()->nNode = *pRegionEndIdx;
        pPam->Move( fnMoveForward, fnGoNode );
        delete pRegionEndIdx, pRegionEndIdx = 0;
    }


    pFmt = &pAkt->GetMaster();

    if( bInsPageDesc )
    {
        if( bHeaderUL )
        {
            if( aUL.GetUpper() > aHUL.GetUpper() )
            {
                aHSz.SetHeight( aUL.GetUpper() - aHUL.GetUpper() );
                aHUL.SetUpper( 0 );
                aUL.SetUpper( USHORT(aUL.GetUpper() - aHSz.GetHeight()) );

                if( !pFmt->GetHeader().GetHeaderFmt() )
                    pFmt->SetAttr( SwFmtHeader( (BOOL)TRUE ));
                if( pFirst && !pFirst->GetMaster().GetHeader().GetHeaderFmt() )
                    pFirst->GetMaster().SetAttr( SwFmtHeader( (BOOL)TRUE ));
            }
        }
        else if( USHRT_MAX != nRestoreUpper )
            aUL.SetUpper( aUL.GetUpper() - nRestoreUpper );

        if( bFooterUL  )
        {
            if( aUL.GetLower() > aFUL.GetLower() )
            {
                aFUL.SetUpper( aUL.GetLower() - aFUL.GetLower() );
                aUL.SetLower( aFUL.GetLower() );
                aFUL.SetLower( 0 );
                if( !pFmt->GetFooter().GetFooterFmt() )
                    pFmt->SetAttr( SwFmtFooter( (BOOL)TRUE ));
                if( pFirst && !pFirst->GetMaster().GetFooter().GetFooterFmt() )
                    pFirst->GetMaster().SetAttr( SwFmtFooter( (BOOL)TRUE ));
            }
            else
            {
                aUL.SetLower( aFUL.GetLower() );
                aFUL.SetLower( 0 );
            }
        }
        else if( USHRT_MAX != nRestoreLower )
            aUL.SetLower( aUL.GetLower() - nRestoreLower );

        pFmt->SetAttr( aSz );
        pFmt->SetAttr( aLR );
        pFmt->SetAttr( aUL );

        pAkt->SetNumType( aNumType );
        if( pFirst )
            pFirst->SetNumType( aNumType );

        if( ( bHeaderUL || bHeaderLR ) &&
            0 != (pFmt = (SwFrmFmt*)pAkt->GetMaster().GetHeader().GetHeaderFmt()) )
        {
            if( bHeaderUL && aHSz.GetHeight() ) pFmt->SetAttr( aHSz );
            if( bHeaderLR ) pFmt->SetAttr( aHLR );
        }

        if( ( bFooterUL || bFooterLR ) &&
            0 != ( pFmt = (SwFrmFmt*)pAkt->GetMaster().GetFooter().GetFooterFmt()) )
        {
            if( bFooterUL ) pFmt->SetAttr( aFUL );
            if( bFooterLR ) pFmt->SetAttr( aFLR );
        }
    }


    if( nCols && USHRT_MAX != nCols )
    {
        if( ::lcl_SetFmtCol( pAkt->GetMaster(), nCols, nColSpace, aColumns )
            && pFirst )
            pFirst->GetMaster().SetAttr( pAkt->GetMaster().GetAttr( RES_COL ));

        //JP 19.03.99 - Spaltigkeit NIE an der Seite setzen - dieses
        //              ist immer ein Kennzeichen fuer einen Bereich, weil
        //              die Seitenspalten NIE vom Layout aufgebrochen werden.
        //JP 24.02.00 - but dont insert column section into a column page
        //              Bug 73480
        if( bSwPageDesc )
        {
            const SwPageDesc& rOld = pDoc->GetPageDesc( nLastPageDesc );
            if( rOld.GetMaster().GetAttr( RES_COL ) !=
                pAkt->GetMaster().GetAttr( RES_COL ) )
            {
                bCheckEqualPgDesc = TRUE;
                bInsPageDesc = FALSE;
                eBreak = SVX_BREAK_NONE;

// !! ???? muss das hier passieren ??
                // fuer den Absatz dahinter sorgen
                pDoc->AppendTxtNode(*pPam->GetPoint());
                pPam->Move( fnMoveBackward );
                pPam->SetMark();
            }
        }
    }
    else if( bInsNewPageDesc )
    {
        pAkt->GetMaster().ResetAttr( RES_COL );
        if( pFirst )
            pFirst->GetMaster().ResetAttr( RES_COL );
    }

    if( pFirst )
    {
        // setze am ersten den richtigen Follow
        pFirst->SetFollow( pAkt );

        // und setze noch die die Werte aus PageDesc:
        pFirst->SetLandscape( pAkt->GetLandscape() );

        pFmt = &pFirst->GetMaster();

        pFmt->SetAttr( aSz );
        pFmt->SetAttr( aLR );
        pFmt->SetAttr( aUL );

        if( ( bHeaderUL || bHeaderLR ) &&
            0 != (pFmt = (SwFrmFmt*)pFirst->GetMaster().GetHeader().GetHeaderFmt()) )
        {
            if( bHeaderUL && aHSz.GetHeight() ) pFmt->SetAttr( aHSz );
            if( bHeaderLR ) pFmt->SetAttr( aHLR );
        }

        if( ( bFooterUL || bFooterLR ) &&
            0 != ( pFmt = (SwFrmFmt*)pFirst->GetMaster().GetFooter().GetFooterFmt()) )
        {
            if( bFooterUL ) pFmt->SetAttr( aFUL );
            if( bFooterLR ) pFmt->SetAttr( aFLR );
        }
    }

    const SwPageDesc& rOld = pDoc->GetPageDesc( nLastPageDesc );
    if( bInsNewPageDesc && pFirst &&
        pDoc->GetPageDesc( nLastFirstPageDesc ).GetFollow() != &rOld )
    {
        bCheckEqualPgDesc = FALSE;
    }

    if( bInsPageDesc && ( !bCheckEqualPgDesc || !bInsNewPageDesc ||
        // dann pruefe ob die Attribute beider PageDesc gleich sind
        !::lcl_CompareRTFPageDesc(pDoc->GetPageDesc( nLastPageDesc ), *pAkt ))
        || ( bMakeNewPageDesc && SVX_BREAK_NONE != eBreak ) )
    {
        // Pagedescriptoren am Dokument updaten (nur so werden auch die
        // linken Seiten usw. eingestellt).

        // Spaltigkeit in den Set uebernehmen
        SfxItemSet* pSet = 0;
        const SfxPoolItem* pItem;
        SfxItemSet aSet( pDoc->GetAttrPool(), RES_COL, RES_COLUMNBALANCE );
        if( SFX_ITEM_SET == pAkt->GetMaster().GetItemState(
                                            RES_COL, FALSE, &pItem )
            && 1 < ((SwFmtCol*)pItem)->GetColumns().Count() )
        {
            aSet.Put( *pItem );
            pSet = &aSet;
            pAkt->GetMaster().ResetAttr( RES_COL );
            if( pFirst )
                pFirst->GetMaster().ResetAttr( RES_COL );
        }

        pDoc->ChgPageDesc( nAktPageDesc, *pAkt );

        if( pSet )
        {
            SwSection aSect( CONTENT_SECTION, pDoc->GetUniqueSectionName() );
            SwSection* pSect = pDoc->Insert( *pPam, aSect, pSet );
            pPam->DeleteMark();

            SwSectionNode* pSectNd = pSect->GetFmt()->GetSectionNode( TRUE );
            if( pRegionEndIdx )
                *pRegionEndIdx = *pSectNd->EndOfSectionNode();
            else
                pRegionEndIdx = new SwNodeIndex( *pSectNd->EndOfSectionNode());

            pPam->GetPoint()->nNode = *pSectNd;
            pPam->Move( fnMoveForward, fnGoNode );

            // Attribut Enden ggfs. anpassen!!!!
            if( GetAttrStack().Count() )
            {
                // Attribut Stack-Eintraege, muessen ans Ende des vorherigen
                // Nodes verschoben werden.
                for( USHORT n = GetAttrStack().Count(); n; )
                {
                    SvxRTFItemStackType* pStkEntry = (SvxRTFItemStackType*)
                                                    GetAttrStack()[ --n ];
                    if( pRegionEndIdx->GetIndex() == pStkEntry->GetSttNode().GetIdx() )
                        pStkEntry->SetStartPos( SwxPosition( pPam ) );
                }
            }
        }

        if( bInsNewPageDesc )
        {
            SwFmtPageDesc aPgDsc( pAkt );

            if( pFirst )
            {
                // Pagedescriptoren am Dokument updaten (nur so werden auch die
                // linken Seiten usw. eingestellt).
                pDoc->ChgPageDesc( nAktFirstPageDesc, *pFirst );
                pFirst->Add( &aPgDsc );
            }

            if( USHRT_MAX != nPgStart )
                aPgDsc.SetNumOffset( nPgStart );
            pDoc->Insert( *pPam, aPgDsc );
        }
    }
    else
    {
        // sollte es ein BereichsWechsel sein?
        if( bCheckEqualPgDesc && SVX_BREAK_NONE == eBreak
            /*&& (!bInsPageDesc || neuen Bereich aufsetzen
             rOld.GetMaster().GetCol() != pAkt->GetMaster().GetCol() )*/ )
        {
            // Spaltigkeit in den Set uebernehmen
            SfxItemSet* pSet = 0;
            const SfxPoolItem* pItem;
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_COL, RES_COLUMNBALANCE);
            if( SFX_ITEM_SET == pAkt->GetMaster().GetItemState(
                                                RES_COL, FALSE, &pItem )
                && 1 < ((SwFmtCol*)pItem)->GetColumns().Count() )
            {
                aSet.Put( *pItem );
                pSet = &aSet;
            }

            if( bNoBalancedCols )
            {
                aSet.Put( SwFmtNoBalancedColumns( TRUE ) );
                pSet = &aSet;
            }

            if( !bFirstCall || pSet )
            {
                SwSection aSect( CONTENT_SECTION, pDoc->GetUniqueSectionName() );
                SwSection* pSect = pDoc->Insert( *pPam, aSect, pSet );
                pPam->DeleteMark();

                SwSectionNode* pSectNd = pSect->GetFmt()->GetSectionNode( TRUE );
                if( pRegionEndIdx )
                    *pRegionEndIdx = *pSectNd->EndOfSectionNode();
                else
                    pRegionEndIdx = new SwNodeIndex( *pSectNd->EndOfSectionNode());

                pPam->GetPoint()->nNode = *pSectNd;
                pPam->Move( fnMoveForward, fnGoNode );

                // Attribut Enden ggfs. anpassen!!!!
                if( GetAttrStack().Count() )
                {
                    // Attribut Stack-Eintraege, muessen ans Ende des vorherigen
                    // Nodes verschoben werden.
                    for( USHORT n = GetAttrStack().Count(); n; )
                    {
                        SvxRTFItemStackType* pStkEntry = (SvxRTFItemStackType*)
                                                        GetAttrStack()[ --n ];
                        if( pRegionEndIdx->GetIndex() == pStkEntry->GetSttNode().GetIdx() )
                            pStkEntry->SetStartPos( SwxPosition( pPam ) );
                    }
                }
            }
        }
        else if( bInsPageDesc )
        {
            if(SVX_BREAK_COLUMN_BEFORE == eBreak )
                pDoc->Insert( *pPam, SvxFmtBreakItem( eBreak ));
            else if( SVX_BREAK_NONE != eBreak )
            {
                SwFmtPageDesc aPgDsc( ( pFirst && &rOld ==
                            pDoc->GetPageDesc( nLastFirstPageDesc ).GetFollow() )
                                ? &pDoc->_GetPageDesc( nLastFirstPageDesc )
                                : &rOld );

                if( USHRT_MAX != nPgStart )
                    aPgDsc.SetNumOffset( nPgStart );
                pDoc->Insert( *pPam, aPgDsc );
            }
        }

        // der aktuelle muss wieder entnfernt werden
        if( bInsNewPageDesc )
        {
            pDoc->DelPageDesc( nAktPageDesc );
            if( pFirst )
                pDoc->DelPageDesc( nAktFirstPageDesc );
            nAktPageDesc = nLastPageDesc;
            nAktFirstPageDesc = nLastFirstPageDesc;
        }
    }

    SkipToken( -1 );
}



void SwRTFParser::ReadPageDescTbl()
{
    if( !IsNewDoc() )
    {
        SkipGroup();
        return;
    }

    // dann erzeuge aus der SvxStyle-Tabelle die Swg-Collections, damit
    // diese auch in den Headers/Footer benutzt werden koennen!
    MakeStyleTab();
    // das default-Style schon gleich am ersten Node setzen
    SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
    if( !pColl )
        pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
    pDoc->SetTxtFmtColl( *pPam, pColl );

    int nToken, bSaveChkStyleAttr = IsChkStyleAttr();
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!

    SetChkStyleAttr(FALSE);     // Attribute nicht gegen die Styles checken

    bInPgDscTbl = TRUE;
    USHORT nPos = 0;
    SwPageDesc* pPg;
    SwFrmFmt* pPgFmt;

    SvxULSpaceItem aUL, aHUL, aFUL;
    SvxLRSpaceItem aLR, aHLR, aFLR;
    SwFmtFrmSize aSz( ATT_FIX_SIZE, 11905, 16837 );     // DIN A4 defaulten
    SwFmtFrmSize aFSz( ATT_MIN_SIZE ), aHSz( ATT_MIN_SIZE );

    USHORT nCols = USHRT_MAX, nColSpace = USHRT_MAX, nAktCol = 0;
    SvUShorts aColumns;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( nToken = GetNextToken() )
        {
        case '{':
            ++nOpenBrakets;
            break;

        case '}':
            if( 1 == --nOpenBrakets )
            {
                // PageDesc ist fertig, setze am Doc
                pPgFmt->SetAttr( aLR );
                pPgFmt->SetAttr( aUL );
                pPgFmt->SetAttr( aSz );
                ::lcl_SetFmtCol( *pPgFmt, nCols, nColSpace, aColumns );
                if( pPgFmt->GetHeader().GetHeaderFmt() )
                {
                    SwFrmFmt* pHFmt = (SwFrmFmt*)pPgFmt->GetHeader().GetHeaderFmt();
                    pHFmt->SetAttr( aHUL );
                    pHFmt->SetAttr( aHLR );
                    pHFmt->SetAttr( aHSz );
                }
                if( pPgFmt->GetFooter().GetFooterFmt() )
                {
                    SwFrmFmt* pFFmt = (SwFrmFmt*)pPgFmt->GetFooter().GetFooterFmt();
                    pFFmt->SetAttr( aHUL );
                    pFFmt->SetAttr( aHLR );
                    pFFmt->SetAttr( aHSz );
                }
                pDoc->ChgPageDesc( nPos++, *pPg );
            }
            break;

        case RTF_PGDSC:
            if( nPos)   // kein && wg MAC
              if (nPos != pDoc->MakePageDesc(
                                      String::CreateFromInt32( nTokenValue ) ) )
                ASSERT( FALSE, "PageDesc an falscher Position" );

            pPg = &pDoc->_GetPageDesc( nPos );
            pPg->SetLandscape( FALSE );
            pPgFmt = &pPg->GetMaster();
#ifndef CFRONT
    SETPAGEDESC_DEFAULTS:
#endif
            // aSz = pPgFmt->GetFrmSize();
            aSz.SetWidth( 11905 ); aSz.SetHeight( 16837 );      // DIN A4 defaulten
            aLR.SetLeft( 0 );   aLR.SetRight( 0 );
            aUL.SetLower( 0 );  aUL.SetUpper( 0 );
            aHLR.SetLeft( 0 );  aHLR.SetRight( 0 );
            aHUL.SetLower( 0 ); aHUL.SetUpper( 0 );
            aFLR.SetLeft( 0 );  aFLR.SetRight( 0 );
            aFUL.SetLower( 0 ); aFUL.SetUpper( 0 );
            nCols = USHRT_MAX; nColSpace = USHRT_MAX; nAktCol = 0;
            aFSz.SetSizeType( ATT_MIN_SIZE ); aFSz.SetHeight( 0 );
            aHSz.SetSizeType( ATT_MIN_SIZE ); aHSz.SetHeight( 0 );
            break;

        case RTF_PGDSCUSE:
            pPg->WriteUseOn( (UseOnPage)nTokenValue );
            break;

        case RTF_PGDSCNXT:
            // setze erstmal nur die Nummer als Follow. Am Ende der
            // Tabelle wird diese entsprechend korrigiert !!
            if( nTokenValue )
                pPg->SetFollow( (const SwPageDesc*)nTokenValue );
            else
                pPg->SetFollow( &pDoc->GetPageDesc( 0 ) );
            break;

        case RTF_FORMULA:   /* Zeichen "\|" !!! */
            pPgFmt->SetAttr( aLR );
            pPgFmt->SetAttr( aUL );
            pPgFmt->SetAttr( aSz );
            ::lcl_SetFmtCol( *pPgFmt, nCols, nColSpace, aColumns );
            if( pPgFmt->GetHeader().GetHeaderFmt() )
            {
                SwFrmFmt* pHFmt = (SwFrmFmt*)pPgFmt->GetHeader().GetHeaderFmt();
                pHFmt->SetAttr( aHUL );
                pHFmt->SetAttr( aHLR );
                pHFmt->SetAttr( aHSz );
            }
            if( pPgFmt->GetFooter().GetFooterFmt() )
            {
                SwFrmFmt* pFFmt = (SwFrmFmt*)pPgFmt->GetFooter().GetFooterFmt();
                pFFmt->SetAttr( aHUL );
                pFFmt->SetAttr( aHLR );
                pFFmt->SetAttr( aHSz );
            }

            pPgFmt = &pPg->GetLeft();
#ifndef CFRONT
            goto SETPAGEDESC_DEFAULTS;
#else
            aLR.SetLeft( 0 );   aLR.SetRight( 0 );
            aUL.SetLower( 0 );  aUL.SetUpper( 0 );
            aHLR.SetLeft( 0 );  aHLR.SetRight( 0 );
            aHUL.SetLower( 0 ); aHUL.SetUpper( 0 );
            aFLR.SetLeft( 0 );  aFLR.SetRight( 0 );
            aFUL.SetLower( 0 ); aFUL.SetUpper( 0 );
//          aSz = pPgFmt->GetFrmSize();
            aSz.SetWidth( 11905 ); aSz.SetHeight( 16837 );      // DIN A4 defaulten
            nCols = USHRT_MAX; nColSpace = USHRT_MAX; nAktCol = 0;
            aFSz.SetSizeType( ATT_MIN_SIZE ); aFSz.SetHeight( 0 );
            aHSz.SetSizeType( ATT_MIN_SIZE ); aHSz.SetHeight( 0 );
            break;
#endif

        // alt: LI/RI/SA/SB, neu: MARG?SXN
        case RTF_MARGLSXN:
        case RTF_LI:        aLR.SetLeft( (USHORT)nTokenValue );     break;
        case RTF_MARGRSXN:
        case RTF_RI:        aLR.SetRight( (USHORT)nTokenValue );    break;
        case RTF_MARGTSXN:
        case RTF_SA:        aUL.SetUpper( (USHORT)nTokenValue );    break;
        case RTF_MARGBSXN:
        case RTF_SB:        aUL.SetLower( (USHORT)nTokenValue );    break;
        case RTF_PGWSXN:    aSz.SetWidth( nTokenValue );            break;
        case RTF_PGHSXN:    aSz.SetHeight( nTokenValue );           break;

        case RTF_HEADERY:       aHUL.SetUpper( (USHORT)nTokenValue );   break;
        case RTF_HEADER_YB:     aHUL.SetLower( (USHORT)nTokenValue );   break;
        case RTF_HEADER_XL:     aHLR.SetLeft( (USHORT)nTokenValue );    break;
        case RTF_HEADER_XR:     aHLR.SetRight( (USHORT)nTokenValue );   break;
        case RTF_FOOTERY:       aFUL.SetLower( (USHORT)nTokenValue );   break;
        case RTF_FOOTER_YT:     aFUL.SetUpper( (USHORT)nTokenValue );   break;
        case RTF_FOOTER_XL:     aFLR.SetLeft( (USHORT)nTokenValue );    break;
        case RTF_FOOTER_XR:     aFLR.SetRight( (USHORT)nTokenValue );   break;

        case RTF_HEADER_YH:
                if( 0 > nTokenValue )
                {
                    aHSz.SetSizeType( ATT_FIX_SIZE );
                    nTokenValue = -nTokenValue;
                }
                aHSz.SetHeight( (USHORT)nTokenValue );
                break;

        case RTF_FOOTER_YH:
                if( 0 > nTokenValue )
                {
                    aFSz.SetSizeType( ATT_FIX_SIZE );
                    nTokenValue = -nTokenValue;
                }
                aFSz.SetHeight( (USHORT)nTokenValue );
                break;


        case RTF_LNDSCPSXN:     pPg->SetLandscape( TRUE );          break;

        case RTF_COLS:          nCols = (USHORT)nTokenValue;        break;
        case RTF_COLSX:         nColSpace = (USHORT)nTokenValue;    break;

        case RTF_COLNO:
            nAktCol = (USHORT)nTokenValue;
            if( RTF_COLW == GetNextToken() )
            {
                USHORT nWidth = USHORT( nTokenValue ), nSpace = 0;
                if( RTF_COLSR == GetNextToken() )
                    nSpace = USHORT( nTokenValue );
                else
                    SkipToken( -1 );        // wieder zurueck

                if( --nAktCol == ( aColumns.Count() / 2 ) )
                {
                    aColumns.Insert( nWidth, aColumns.Count() );
                    aColumns.Insert( nSpace, aColumns.Count() );
                }
            }
            break;

        case RTF_PAGEBB:
            {
                pPgFmt->SetAttr( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ) );
            }
            break;

        case RTF_HEADER:
        case RTF_HEADERL:
        case RTF_HEADERR:
        case RTF_FOOTER:
        case RTF_FOOTERL:
        case RTF_FOOTERR:
        case RTF_FOOTERF:
        case RTF_HEADERF:
            ReadHeaderFooter( nToken, pPg );

            --nOpenBrakets;     // Klammer wird im ReadAttr ueberlesen!
            break;


        case RTF_TEXTTOKEN:
            if( !DelCharAtEnd( aToken, ';' ).Len() )
                break;
            pPg->SetName( aToken );

            {
                // sollte es eine Vorlage aus dem Pool sein ??
                USHORT n = pDoc->GetPoolId( aToken, GET_POOLID_PAGEDESC );
                if( USHRT_MAX != n )
                    // dann setze bei der Neuen die entsp. PoolId
                    pPg->SetPoolFmtId( n );
            }
            break;

        case RTF_BRDBOX:
            if( 3 == nOpenBrakets )
            {
                ReadBorderAttr( SkipToken( -2 ), (SfxItemSet&)pPgFmt->GetAttrSet() );
                --nOpenBrakets;     // Klammer wird im ReadAttr ueberlesen!
            }
            break;

        case RTF_SHADOW:
            if( 3 == nOpenBrakets )
            {
                ReadAttr( SkipToken( -2 ), (SfxItemSet*)&pPgFmt->GetAttrSet() );
                --nOpenBrakets;     // Klammer wird im ReadAttr ueberlesen!
            }
            break;


        default:
            if( (nToken & ~0xff ) == RTF_SHADINGDEF )
                ReadBackgroundAttr( nToken, (SfxItemSet&)pPgFmt->GetAttrSet() );
            break;
        }
    }


    // setze jetzt noch bei allen die entsprechenden Follows !!
    // Die, die ueber die Tabelle eingelesen wurden und einen
    // Follow definiert haben, ist dieser als Tabposition im
    // Follow schon gesetzt.
    for( nPos = 0; nPos < pDoc->GetPageDescCnt(); ++nPos )
    {
        SwPageDesc* pPgDsc = &pDoc->_GetPageDesc( nPos );
        if( (USHORT)(long)pPgDsc->GetFollow() < pDoc->GetPageDescCnt() )
            pPgDsc->SetFollow( &pDoc->GetPageDesc(
                    (USHORT)(long)pPgDsc->GetFollow() ));
    }

    SetChkStyleAttr( bSaveChkStyleAttr );

    bInPgDscTbl = FALSE;
    nAktPageDesc = 0;
    nAktFirstPageDesc = 0;
    bSwPageDesc = TRUE;
    SkipToken( -1 );
}

// -------------- Methoden --------------------

/*
void SwRTFParser::ReadUnknownData()
{
    SvRTFParser::ReadUnknownData();
}

void SwRTFParser::ReadOLEData()
{
    SvRTFParser::ReadOLEData();
}
*/

void SwRTFParser::ReadPrtData()
{
    // der Eingabe Stream steht auf der aktuellen Position
    USHORT nLen = USHORT( nTokenValue ), nCnt = 0;
    BYTE * pData = new BYTE[ nLen ];

    while( IsParserWorking() )          // lese bis zur schliessenden Klammer
    {
        int nToken = GetNextToken();
        if( RTF_TEXTTOKEN == nToken )
        {
            xub_StrLen nTknLen = HexToBin( aToken );
            if( STRING_NOTFOUND != nTknLen )
            {
                memcpy( pData + nCnt, (sal_Char*)aToken.GetBuffer(), nTknLen );
                nCnt += nTknLen;
            }
        }
        else if( '}' == nToken )
            break;
    }

/*-----------------25.10.94 16:37-------------------
!!! OK 25.10.94 16:37
    // um Byte-Dreher (Mac/Unix/Win) bei der Laenge auszuschliessen,
    // setze hier expliziet die Laenge.
    ((JobSetup*)pData)->nLen = nLen;

    // so, jetzt sind alle Daten gelesen, setze den Bitsack am Document
    // dieser kopiert die Daten !!
    pDoc->SetJobsetup( (JobSetup*)pData );
    delete pData;
--------------------------------------------------*/
    SkipToken( -1 );        // schliessende Klammer wieder zurueck!!
}

static const SwNodeIndex* SetHeader(SwFrmFmt* pHdFtFmt, BOOL bCheck )
{
    SwFrmFmt* pTmp = pHdFtFmt;
    pHdFtFmt = bCheck ? (SwFrmFmt*)pHdFtFmt->GetHeader().GetHeaderFmt() : 0;
    if( !pHdFtFmt )
    {
        // noch keine Header, dann erzeuge einen. Ob die Kopf/Fuss-
        // zeilen angezeigt werden, bestimmen \header? \footer? ...
        pTmp->SetAttr( SwFmtHeader( TRUE ));
        pHdFtFmt = (SwFrmFmt*)pTmp->GetHeader().GetHeaderFmt();
    }
    return  pHdFtFmt->GetCntnt().GetCntntIdx();
}

static const SwNodeIndex* SetFooter(SwFrmFmt* pHdFtFmt, BOOL bCheck)
{
    SwFrmFmt* pTmp = pHdFtFmt;
    pHdFtFmt = bCheck ? (SwFrmFmt*)pHdFtFmt->GetFooter().GetFooterFmt() : 0;
    if( !pHdFtFmt )
    {
        // noch keine Header, dann erzeuge einen. Ob die Kopf/Fuss-
        // zeilen angezeigt werden, bestimmen \header? \footer? ...
        pTmp->SetAttr( SwFmtFooter( TRUE ));
        pHdFtFmt = (SwFrmFmt*)pTmp->GetFooter().GetFooterFmt();
    }
    return pHdFtFmt->GetCntnt().GetCntntIdx();
}


void SwRTFParser::ReadHeaderFooter( int nToken, SwPageDesc* pPageDesc )
{
    ASSERT( RTF_FOOTNOTE == nToken ||
            RTF_FLY_INPARA == nToken ||
            pPageDesc, "PageDesc fehlt" );

    // alle wichtigen Sachen sichern
    SwPosition aSavePos( *pPam->GetPoint() );
    SvxRTFItemStack aSaveStack;
    aSaveStack.Insert( &GetAttrStack(), 0 );
    GetAttrStack().Remove( 0, GetAttrStack().Count() );

    // save the fly array - after read, all flys may be set into
    // the header/footer
    SwFlySaveArr aSaveArray( 255 < aFlyArr.Count() ? aFlyArr.Count() : 255 );
    aSaveArray.Insert( &aFlyArr, 0 );
    aFlyArr.Remove( 0, aFlyArr.Count() );
    BOOL bSetFlyInDoc = TRUE;

    const SwNodeIndex* pSttIdx = 0;
    SwFrmFmt* pHdFtFmt = 0;
    SwTxtAttr* pTxtAttr = 0;
    int bDelFirstChar = FALSE, bIsFootnote = FALSE;
    BOOL bOldGrpStt = IsNewGroup();

    int nOpenBrakets = GetOpenBrakets() - 1;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
        {
            BOOL bIsEndNote = RTF_FTNALT == GetNextToken();
            if( !bIsEndNote )
                SkipToken( -1 );

            SwTxtNode* pTxtNd = pPam->GetNode()->GetTxtNode();
            SwFmtFtn aFtnNote( bIsEndNote );
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();

            if( nPos && !bFootnoteAutoNum )
            {
                pPam->GetPoint()->nContent--;
                nPos--;
                aFtnNote.SetNumStr( pTxtNd->GetTxt().GetChar( nPos ) );
                ((String&)pTxtNd->GetTxt()).SetChar( nPos, CH_TXTATR_INWORD );
                bDelFirstChar = TRUE;
            }

            pTxtAttr = pTxtNd->Insert( aFtnNote, nPos, nPos,
                        bDelFirstChar ? SETATTR_NOTXTATRCHR : 0 );

            ASSERT( pTxtAttr, "konnte die Fussnote nicht einfuegen/finden" );

            if( pTxtAttr )
                pSttIdx = ((SwTxtFtn*)pTxtAttr)->GetStartNode();
            bIsFootnote = TRUE;

            // wurde an der Position ein Escapement aufgespannt, so entferne
            // das jetzt. Fussnoten sind bei uns immer hochgestellt.
            SvxRTFItemStackTypePtr pTmp = aSaveStack.Top();
            if( pTmp && pTmp->GetSttNodeIdx() ==
                pPam->GetPoint()->nNode.GetIndex() &&
                pTmp->GetSttCnt() == nPos )
                pTmp->GetAttrSet().ClearItem( RES_CHRATR_ESCAPEMENT );
        }
        break;

    case RTF_FLY_INPARA:
        {
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                            RES_FRMATR_END-1 );
            aSet.Put( SwFmtAnchor( FLY_IN_CNTNT ));
            pHdFtFmt = pDoc->MakeFlySection( FLY_IN_CNTNT, pPam->GetPoint(), &aSet );

            pTxtAttr = pPam->GetNode()->GetTxtNode()->GetTxtAttr(
                                                nPos, RES_TXTATR_FLYCNT );
            ASSERT( pTxtAttr, "konnte den Fly nicht einfuegen/finden" );

            pSttIdx = pHdFtFmt->GetCntnt().GetCntntIdx();
            bSetFlyInDoc = FALSE;
        }
        break;

    case RTF_HEADERF:
    case RTF_HEADER:
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() | PD_HEADERSHARE) );
        pHdFtFmt = &pPageDesc->GetMaster();
        pSttIdx = SetHeader( pHdFtFmt, FALSE );
        break;

    case RTF_HEADERL:
        // wir koennen keine linken oder rechten haben, immer beide
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() & ~PD_HEADERSHARE) );
        SetHeader( pPageDesc->GetRightFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetHeader(pHdFtFmt, FALSE );
        break;

    case RTF_HEADERR:
        // wir koennen keine linken oder rechten haben, immer beide
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() & ~PD_HEADERSHARE) );
        SetHeader( pPageDesc->GetLeftFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetHeader(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERF:
    case RTF_FOOTER:
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() | PD_FOOTERSHARE) );
        pHdFtFmt = &pPageDesc->GetMaster();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERL:
        // wir koennen keine linken oder rechten haben, immer beide
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() & ~PD_FOOTERSHARE) );
        SetFooter( pPageDesc->GetRightFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERR:
        // wir koennen keine linken oder rechten haben, immer beide
        pPageDesc->WriteUseOn( (UseOnPage)(pPageDesc->ReadUseOn() & ~PD_FOOTERSHARE) );
        SetFooter( pPageDesc->GetLeftFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;
    }

    if( !pSttIdx )
        SkipGroup();
    else
    {
        // es ist auf jedenfall jetzt ein TextNode im Kopf/Fusszeilen-Bereich
        // vorhanden. Dieser muss jetzt nur noch gefunden und der neue Cursor
        // dort hinein gesetzt werden.
        SwCntntNode *pNode = pDoc->GetNodes()[ pSttIdx->GetIndex()+1 ]->
                                GetCntntNode();

        // immer ans Ende der Section einfuegen !!
        pPam->GetPoint()->nNode = *pNode->EndOfSectionNode();
        pPam->Move( fnMoveBackward );

        SetNewGroup( TRUE );

        while( !( nOpenBrakets == GetOpenBrakets() && !GetStackPos()) && IsParserWorking() )
        {
            switch( nToken = GetNextToken() )
            {
            case RTF_U:
                if( bDelFirstChar )
                {
                    bDelFirstChar = FALSE;
                    nToken = 0;
                }
                break;

            case RTF_TEXTTOKEN:
                if( bDelFirstChar )
                {
                    if( !aToken.Erase( 0, 1 ).Len() )
                        nToken = 0;
                    bDelFirstChar = FALSE;
                }
                break;
            }
            if( nToken )
                NextToken( nToken );
        }

        SetAllAttrOfStk();
        if( aFlyArr.Count() && bSetFlyInDoc )
            SetFlysInDoc();

        // sollte der letze Node leer sein, dann loesche ihn
        // (\par heisst ja Absatzende und nicht neuer Absatz!)
        DelLastNode();
    }

    // vom FlyFmt noch die richtigen Attribute setzen
    if( pTxtAttr && RES_TXTATR_FLYCNT == pTxtAttr->Which() )
    {
        ASSERT( aFlyArr.Count(), "kein Fly definiert" );
        SwFlySave* pFlySave = aFlyArr[ aFlyArr.Count()-1 ];
        pFlySave->aFlySet.ClearItem( RES_ANCHOR );
        pHdFtFmt->SetAttr( pFlySave->aFlySet );
        aFlyArr.DeleteAndDestroy( aFlyArr.Count() - 1 );
    }

    bFootnoteAutoNum = FALSE;       // default auf aus!

    // und alles wieder zurueck
    *pPam->GetPoint() = aSavePos;
    if( bIsFootnote )
        SetNewGroup( bOldGrpStt );      // Status wieder zurueck
    else
        SetNewGroup( FALSE );           // { - Klammer war kein Group-Start!
    GetAttrStack().Insert( &aSaveStack, 0 );

    aFlyArr.Insert( &aSaveArray, 0 );
    aSaveArray.Remove( 0, aSaveArray.Count() );
}

void SwRTFParser::SetSwgValues( SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;
    // Escapement korrigieren
    if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ESCAPEMENT, FALSE, &pItem ))
    {
        /* prozentuale Veraenderung errechnen !
            * Formel :      (FontSize * 1/20 ) pts      Escapement * 2
            *               -----------------------  = ----------------
            *                     100%                          x
            */

        // die richtige
        long nEsc = ((SvxEscapementItem*)pItem)->GetEsc();

        // automatische Ausrichtung wurde schon richtig berechnet
        if( DFLT_ESC_AUTO_SUPER != nEsc && DFLT_ESC_AUTO_SUB != nEsc )
        {
            const SvxFontHeightItem& rFH = GetSize( rSet );
            nEsc *= 1000L;
            nEsc /= long(rFH.GetHeight());

            SvxEscapementItem aEsc( (short) nEsc,
                                ((SvxEscapementItem*)pItem)->GetProp());
            rSet.Put( aEsc );
        }
    }

    // TabStops anpassen
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_TABSTOP, FALSE, &pItem ))
    {
        const SvxLRSpaceItem& rLR = GetLRSpace( rSet );
        SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );

        register long nOffset = rLR.GetTxtLeft();
        if( nOffset )
        {
            // Tabs anpassen !!
            SvxTabStop* pTabs = (SvxTabStop*)aTStop.GetStart();
            for( USHORT n = aTStop.Count(); n; --n, ++pTabs)
                if( SVX_TAB_ADJUST_DEFAULT != pTabs->GetAdjustment() )
                    pTabs->GetTabPos() -= nOffset;

            // negativer Einzug, dann auf 0 Pos einen Tab setzen
            if( rLR.GetTxtFirstLineOfst() < 0 )
                aTStop.Insert( SvxTabStop() );
        }

        if( !aTStop.Count() )
        {
            const SvxTabStopItem& rDflt = (const SvxTabStopItem&)rSet.
                                GetPool()->GetDefaultItem(RES_PARATR_TABSTOP);
            if( rDflt.Count() )
                aTStop.Insert( &rDflt, 0 );
        }
        rSet.Put( aTStop );
    }
    else if( SFX_ITEM_SET == rSet.GetItemState( RES_LR_SPACE, FALSE, &pItem )
            && ((SvxLRSpaceItem*)pItem)->GetTxtFirstLineOfst() < 0 )
    {
        // negativer Einzug, dann auf 0 Pos einen Tab setzen
        rSet.Put( SvxTabStopItem( 1, 0 ));
    }

    // NumRules anpassen
    if( !bStyleTabValid &&
        SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE, FALSE, &pItem ))
    {
        // dann steht im Namen nur ein Verweis in das ListArray
        SwNumRule* pRule = GetNumRuleOfListNo( ((SwNumRuleItem*)pItem)->
                                                GetValue().ToInt32() );
        if( pRule )
            rSet.Put( SwNumRuleItem( pRule->GetName() ));
        else
            rSet.ClearItem( RES_PARATR_NUMRULE );
    }


/*
 ????????????????????????????????????????????????????????????????????
 ?? muss die LineSpacing Hoehe 200Twip betragen ??
 ?? in rtfitem.hxx wird es auf 0 defaultet. Wenn ja, dann muss hier
 ?? ein neues Item gesetzt werden!!!!
 ????????????????????????????????????????????????????????????????????

    // LineSpacing korrigieren
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_LINESPACING, FALSE, &pItem ))
    {
        const SvxLineSpacingItem* pLS = (const SvxLineSpacingItem*)pItem;
        SvxLineSpacingItem aNew;

        aNew.SetInterLineSpace( pLS->GetInterLineSpace() );
        aNew.GetLineSpaceRule() = pLS->GetLineSpaceRule();
        aNew.SetPropLineSpace( pLS->GetPropLineSpace() );
        aNew.GetInterLineSpaceRule() = pLS->GetInterLineSpaceRule();

        rSet.Put( aNew );
    }
?????????????????????????????????????????????????????????????????? */

}


SwTxtFmtColl* SwRTFParser::MakeColl( const String& rName, USHORT nPos,
                                        BYTE nOutlineLevel, int& rbCollExist )
{
    if( BYTE(-1) == nOutlineLevel )
        nOutlineLevel = NO_NUMBERING;

    rbCollExist = FALSE;
    SwTxtFmtColl* pColl;
    String aNm( rName );
    if( !aNm.Len() )
    {
        if( !nPos )
        {
            pColl = pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
            pColl->SetOutlineLevel( nOutlineLevel );
            return pColl;
        }

        aNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));      // erzeuge einen Namen
        aNm += String::CreateFromInt32( nPos );
        aNm += ')';
    }

    // suche jetzt nach einem Style mit dem gleichen Namen (0=dflt. FmtColl)
    if( 0 != (pColl = pDoc->FindTxtFmtCollByName( aNm ) ) )
    {
        // neues Doc: keine alten Attribute behalten !!
        if( IsNewDoc() )
        {
            pColl->ResetAllAttr();
            pColl->SetOutlineLevel( nOutlineLevel );
        }
        else
            rbCollExist = TRUE;
        return pColl;
    }

    // Collection neu erzeugen
    pColl = pDoc->MakeTxtFmtColl( aNm,
                    pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD ) );

    // sollte es eine Vorlage aus dem Pool sein ??
    USHORT n = pDoc->GetPoolId( aNm, GET_POOLID_TXTCOLL );
    if( USHRT_MAX != n )
        // dann setze bei der Neuen die entsp. PoolId
        pColl->SetPoolFmtId( n );

    pColl->SetOutlineLevel( nOutlineLevel );
    return pColl;
}

SwCharFmt* SwRTFParser::MakeCharFmt( const String& rName, USHORT nPos,
                                    int& rbCollExist )
{
    rbCollExist = FALSE;
    SwCharFmt* pFmt;
    String aNm( rName );
    if( !aNm.Len() )
    {
        aNm.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "NoName(" ));
        aNm += String::CreateFromInt32( nPos );
        aNm += ')';
    }

    // suche jetzt nach einem Style mit dem gleichen Namen (0=dflt. FmtColl)
    if( 0 != (pFmt = pDoc->FindCharFmtByName( aNm ) ) )
    {
        // neues Doc: keine alten Attribute behalten !!
        if( IsNewDoc() )
            pFmt->ResetAllAttr();
        else
            rbCollExist = TRUE;
        return pFmt;
    }

    // Format neu erzeugen
    pFmt = pDoc->MakeCharFmt( aNm, pDoc->GetDfltCharFmt() );

    // sollte es eine Vorlage aus dem Pool sein ??
    USHORT n = pDoc->GetPoolId( aNm, GET_POOLID_CHRFMT );
    if( USHRT_MAX != n )
        // dann setze bei der Neuen die entsp. PoolId
        pFmt->SetPoolFmtId( n );

    return pFmt;
}

void SwRTFParser::SetStyleAttr( SfxItemSet& rCollSet,
                                const SfxItemSet& rStyleSet,
                                const SfxItemSet& rDerivedSet )
{
    rCollSet.Put( rStyleSet );
    if( rDerivedSet.Count() )
    {
        // suche alle Attribute, die neu gesetzt werden:
        const SfxPoolItem* pItem;
        SfxItemIter aIter( rDerivedSet );
        USHORT nWhich = aIter.GetCurItem()->Which();
        while( TRUE )
        {
            switch( rStyleSet.GetItemState( nWhich, FALSE, &pItem ) )
            {
            case SFX_ITEM_DEFAULT:
                // auf default zuruecksetzen
                if( RES_FRMATR_END > nWhich )
                    rCollSet.Put( rCollSet.GetPool()->GetDefaultItem( nWhich ));
                break;
            case SFX_ITEM_SET:
                if( *pItem == *aIter.GetCurItem() )     // gleiches Attribut?
                    // definition kommt aus dem Parent
                    rCollSet.ClearItem( nWhich );       // loeschen
                break;
            }

            if( aIter.IsAtEnd() )
                break;
            nWhich = aIter.NextItem()->Which();
        }
    }
    // und jetzt noch auf unsere Werte abgleichen
    SetSwgValues( rCollSet );
}

SwTxtFmtColl* SwRTFParser::MakeStyle( USHORT nNo, const SvxRTFStyleType& rStyle )
{
    int bCollExist;
    SwTxtFmtColl* pColl = MakeColl( rStyle.sName, USHORT(nNo),
                                    rStyle.nOutlineNo, bCollExist );
    aTxtCollTbl.Insert( nNo, pColl );

    // in bestehendes Dok einfuegen, dann keine Ableitung usw. setzen
    if( bCollExist )
        return pColl;

    USHORT nStyleNo = rStyle.nBasedOn;
    if( nStyleNo != nNo )
    {
        SvxRTFStyleType* pDerivedStyle = GetStyleTbl().Get( nStyleNo );
        SwTxtFmtColl* pDerivedColl = aTxtCollTbl.Get( nStyleNo );
        if( !pDerivedColl )         // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            pDerivedColl = pDerivedStyle
                    ? MakeStyle( nStyleNo, *pDerivedStyle )
                    : pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
        }

        if( pColl == pDerivedColl )
            ((SfxItemSet&)pColl->GetAttrSet()).Put( rStyle.aAttrSet );
        else
        {
            pColl->SetDerivedFrom( pDerivedColl );

            // setze die richtigen Attribute
            const SfxItemSet* pDerivedSet;
            if( pDerivedStyle )
                pDerivedSet = &pDerivedStyle->aAttrSet;
            else
                pDerivedSet = &pDerivedColl->GetAttrSet();

            SetStyleAttr( (SfxItemSet&)pColl->GetAttrSet(),
                            rStyle.aAttrSet, *pDerivedSet );
        }
    }
    else
        ((SfxItemSet&)pColl->GetAttrSet()).Put( rStyle.aAttrSet );


    nStyleNo = rStyle.nNext;
    if( nStyleNo != nNo )
    {
        SwTxtFmtColl* pNext = aTxtCollTbl.Get( nStyleNo );
        if( !pNext )            // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            SvxRTFStyleType* pMkStyle = GetStyleTbl().Get( nStyleNo );
            pNext = pMkStyle
                    ? MakeStyle( nStyleNo, *pMkStyle )
                    : pDoc->GetTxtCollFromPool( RES_POOLCOLL_STANDARD );
        }
        pColl->SetNextTxtFmtColl( *pNext );
    }
    return pColl;
}

SwCharFmt* SwRTFParser::MakeCharStyle( USHORT nNo, const SvxRTFStyleType& rStyle )
{
    int bCollExist;
    SwCharFmt* pFmt = MakeCharFmt( rStyle.sName, USHORT(nNo), bCollExist );
    aCharFmtTbl.Insert( nNo, pFmt );

    // in bestehendes Dok einfuegen, dann keine Ableitung usw. setzen
    if( bCollExist )
        return pFmt;

    USHORT nStyleNo = rStyle.nBasedOn;
    if( nStyleNo != nNo )
    {
        SvxRTFStyleType* pDerivedStyle = GetStyleTbl().Get( nStyleNo );
        SwCharFmt* pDerivedFmt = aCharFmtTbl.Get( nStyleNo );
        if( !pDerivedFmt )          // noch nicht vorhanden, also anlegen
        {
            // ist die ueberhaupt als Style vorhanden ?
            pDerivedFmt = pDerivedStyle
                    ? MakeCharStyle( nStyleNo, *pDerivedStyle )
                    : pDoc->GetDfltCharFmt();
        }

        if( pFmt == pDerivedFmt )
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( rStyle.aAttrSet );
        else
        {
            pFmt->SetDerivedFrom( pDerivedFmt );

            // setze die richtigen Attribute
            const SfxItemSet* pDerivedSet;
            if( pDerivedStyle )
                pDerivedSet = &pDerivedStyle->aAttrSet;
            else
                pDerivedSet = &pDerivedFmt->GetAttrSet();

            SetStyleAttr( (SfxItemSet&)pFmt->GetAttrSet(),
                            rStyle.aAttrSet, *pDerivedSet );
        }
    }
    else
        ((SfxItemSet&)pFmt->GetAttrSet()).Put( rStyle.aAttrSet );

    return pFmt;
}

// loesche den letzten Node (Tabelle/Fly/Ftn/..)
void SwRTFParser::DelLastNode()
{
    // sollte der letze Node leer sein, dann loesche ihn
    // (\par heisst ja Absatzende und nicht neuer Absatz!)
    if( !pPam->GetPoint()->nContent.GetIndex() )
    {
        ULONG nNodeIdx = pPam->GetPoint()->nNode.GetIndex();
        SwCntntNode* pCNd = pDoc->GetNodes()[ nNodeIdx ]->GetCntntNode();
        if( pCNd && pCNd->StartOfSectionIndex()+2 <
            pCNd->EndOfSectionIndex() )
        {
            if( GetAttrStack().Count() )
            {
                // Attribut Stack-Eintraege, muessen ans Ende des vorherigen
                // Nodes verschoben werden.
                BOOL bMove = FALSE;
                for( USHORT n = GetAttrStack().Count(); n; )
                {
                    SvxRTFItemStackType* pStkEntry = (SvxRTFItemStackType*)
                                                    GetAttrStack()[ --n ];
                    if( nNodeIdx == pStkEntry->GetSttNode().GetIdx() )
                    {
                        if( !bMove )
                        {
                            pPam->Move( fnMoveBackward );
                            bMove = TRUE;
                        }
                        pStkEntry->SetStartPos( SwxPosition( pPam ) );
                    }
                }
                if( bMove )
                    pPam->Move( fnMoveForward );
            }
            pPam->GetPoint()->nContent.Assign( 0, 0 );
            pPam->SetMark();
            pPam->DeleteMark();
            pDoc->GetNodes().Delete( pPam->GetPoint()->nNode );
        }
    }
}

    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
void SwRTFParser::UnknownAttrToken( int nToken, SfxItemSet* pSet )
{
    switch( nToken )
    {
    case RTF_INTBL:
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
                NewTblLine();           // evt. Line copieren
            else
            {
                // Crsr nicht mehr in der Tabelle ?
                if( !pPam->GetNode()->FindTableNode() )
                {
                    ULONG nOldPos = pPam->GetPoint()->nNode.GetIndex();

                    // dann wieder in die letzte Box setzen
                    // (kann durch einlesen von Flys geschehen!)
                    pPam->GetPoint()->nNode = *pTableNode->EndOfSectionNode();
                    pPam->Move( fnMoveBackward );

                    // alle Attribute, die schon auf den nachfolgen zeigen
                    // auf die neue Box umsetzen !!
                    SvxRTFItemStack& rAttrStk = GetAttrStack();
                    const SvxRTFItemStackType* pStk;
                    for( USHORT n = 0; n < rAttrStk.Count(); ++n )
                        if( ( pStk = rAttrStk[ n ])->GetSttNodeIdx() == nOldPos &&
                            !pStk->GetSttCnt() )
                            ((SvxRTFItemStackType*)pStk)->SetStartPos( SwxPosition( pPam ) );
                }
            }
        }
        break;

    case RTF_PAGEBB:
        {
            pSet->Put( SvxFmtBreakItem( SVX_BREAK_PAGE_BEFORE ));
        }
        break;

    case RTF_PGBRK:
        {
            pSet->Put( SvxFmtBreakItem( 1 == nTokenValue ?
                                SVX_BREAK_PAGE_BOTH : SVX_BREAK_PAGE_AFTER ));
        }
        break;

    case RTF_PGDSCNO:
        if( IsNewDoc() && bSwPageDesc &&
            USHORT(nTokenValue) < pDoc->GetPageDescCnt() )
        {
            const SwPageDesc* pPgDsc = &pDoc->GetPageDesc( (USHORT)nTokenValue );
            pDoc->Insert( *pPam, SwFmtPageDesc( pPgDsc ));
        }
        break;

    case RTF_V:
        if( 0 != nTokenValue )
        {
            // alles ueberlesen bis dieses wieder abgeschaltet wird. Durch
            // plain in der Gruppe oder durch V0
            String sHiddenTxt;
            int nOpenBrakets = 0;
            int nOpenHiddens = 1;
            do {
                switch( nToken = GetNextToken() )
                {
                case '}':
                    if( !--nOpenBrakets /*&& IsNewGroup()*/ )
                        // die Klammer wird noch gebraucht!
                        SkipToken( -1 );
                    break;
                case '{':
                    ++nOpenBrakets;
                    break;

                case RTF_V:
                    if( nTokenValue )
                        ++nOpenHiddens;
                    else if( !--nOpenHiddens && 1 == nOpenBrakets )
                        // das wars
                        nOpenBrakets = 0;
                    break;

                case RTF_U:
                    {
                        if( nTokenValue )
                            sHiddenTxt += (sal_Unicode)nTokenValue;
                        else
                            sHiddenTxt += aToken;
                    }
                    break;

                case RTF_TEXTTOKEN:
                    sHiddenTxt += aToken;
                    break;

                case RTF_PLAIN:
                    if( !nOpenHiddens && 1 == nOpenBrakets )
                    {
                        nOpenBrakets = 0;
                        SkipToken( -1 );
                    }
                    break;
                }
            } while( nOpenBrakets && IsParserWorking() );
            if( sHiddenTxt.Len() )
            {
                SwHiddenTxtField aFld( (SwHiddenTxtFieldType*)
                            pDoc->GetSysFldType( RES_HIDDENTXTFLD ),
                            TRUE, aEmptyStr, sHiddenTxt, TRUE );

                pDoc->Insert( *pPam, SwFmtFld( aFld ) );
            }
        }
        break;

    case RTF_CS:
        {
            SwCharFmt* pFmt = aCharFmtTbl.Get( nTokenValue );
            if( pFmt )
                pSet->Put( SwFmtCharFmt( pFmt ));
        }
        break;

    case RTF_LS:
        if( -1 != nTokenValue )
        {
            if( bStyleTabValid )
            {
                // dann ist auch die ListTabelle gueltig, also suche die
                // enstprechende NumRule
                SwNumRule* pRule = GetNumRuleOfListNo( nTokenValue );
                if( pRule )
                    pSet->Put( SwNumRuleItem( pRule->GetName() ));

                if( SFX_ITEM_SET != pSet->GetItemState( FN_PARAM_NUM_LEVEL, FALSE ))
                    pSet->Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
            }
            else
                // wir sind in der Style-Definitions - Phase. Der Name
                // wird dann spaeter umgesetzt
                pSet->Put( SwNumRuleItem( String::CreateFromInt32( nTokenValue )));
        }
        break;

    case RTF_ILVL:
    case RTF_SOUTLVL:
        {
            BYTE nLevel = MAXLEVEL <= nTokenValue ? MAXLEVEL - 1
                                                  : BYTE( nTokenValue );
            pSet->Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, nLevel ));
        }
        break;

/*
    case RTF_SBYS:
    case RTF_EXPND:
    case RTF_KEEP:
    case RTF_KEEPN:
*/


    // Revision Time Stamp
/*  case RTF_REVDTTM:
        {
            USHORT nMin, nHour, nDay, nMon, nYear, nDayOfWeek;
// long wird gelesen, wird haben aber nur einen short!!!
            ULONG nDtm = aToken.Copy( 7 );
            //  3         2         1         0
            // 10987654321098765432109876543210
            // DOW|   Y    | M | D  | H  | Min
            //
            nMin       = 0x3F & ( nDtm >>= 0 );
            nHour      = 0x1f & ( nDtm >>= 6 );
            nDay       = 0x1f & ( nDtm >>= 5 );
            nMon       = 0x0F & ( nDtm >>= 5 );
            nYear      =(0x7F & ( nDtm >>= 4 )) + 1900;
            nDayOfWeek = 0x07 & ( nDtm >>= 7 );

            Time aTime( nHour, nMin );
            Date aDate( nDay, nMon, nYear );
        }
        break;
*/
    }
}

void SwRTFParser::ReadInfo( const sal_Char* pChkForVerNo )
{
sal_Char __READONLY_DATA aChkForVerNo[] = "StarWriter";

    // falls nicht schon was vorgegeben wurde, setzen wir unseren Namen
    // rein. Wenn das im Kommentar match, wird im Parser die VersionNummer
    // gelesen und gesetzt
    if( !pChkForVerNo )
        pChkForVerNo = aChkForVerNo;

    SvxRTFParser::ReadInfo( pChkForVerNo );
}


#ifdef USED
void SwRTFParser::SaveState( int nToken )
{
    SvxRTFParser::SaveState( nToken );
}

void SwRTFParser::RestoreState()
{
    SvxRTFParser::RestoreState();
}
#endif

/**/

ULONG SwNodeIdx::GetIdx() const
{
    return aIdx.GetIndex();
}

SvxNodeIdx* SwNodeIdx::Clone() const
{
    return new SwNodeIdx( aIdx );
}

SvxPosition* SwxPosition::Clone() const
{
    return new SwxPosition( pPam );
}

SvxNodeIdx* SwxPosition::MakeNodeIdx() const
{
    return new SwNodeIdx( pPam->GetPoint()->nNode );
}

ULONG   SwxPosition::GetNodeIdx() const
{
    return pPam->GetPoint()->nNode.GetIndex();
}

xub_StrLen SwxPosition::GetCntIdx() const
{
    return pPam->GetPoint()->nContent.GetIndex();
}


/*  */

#ifdef DEBUG_JP

#include <string.h>     // fuer sprintf
#include <stdio.h>
#include <svx/cmapitem.hxx>
#include <svx/colritem.hxx>
#include <svx/cntritem.hxx>
#include <svx/crsditem.hxx>
#include <svx/fontitem.hxx>
#include <svx/kernitem.hxx>
#include <svx/langitem.hxx>
#include <svx/postitem.hxx>
#include <svx/shdditem.hxx>
#include <svx/udlnitem.hxx>
#include <svx/wghtitem.hxx>
#include <svx/wrlmitem.hxx>
#include <svx/adjitem.hxx>

static SvFileStream* pOut = 0;
static void OutAttr( const SfxItemSet& rSet )
{
    if( !rSet.Count() )
        return;

    char sOut[ 200 ];
    SfxItemIter aIter( rSet );
    const SfxPoolItem* pItem = aIter.GetCurItem();
    while( TRUE )
    {
        sOut[0] = 0;
        switch( pItem->Which() )
        {
        case RES_CHRATR_CASEMAP:
            {
                sprintf( sOut,
                    "CaseMap: Style[%d]",
                    ((const SvxCaseMapItem*)pItem)->GetValue() );
            }
            break;
        case RES_CHRATR_COLOR:
            {
                sprintf( sOut,
                    "Color: Red[%d] Green[%d] Blue[%d]",
                    ((const SvxColorItem*)pItem)->GetValue().GetRed(),
                    ((const SvxColorItem*)pItem)->GetValue().GetGreen(),
                    ((const SvxColorItem*)pItem)->GetValue().GetBlue() );
            }
            break;

        case RES_CHRATR_CONTOUR:
            {
                sprintf( sOut,
                        "Contour: Style[%s]",
            ( ((const SvxContourItem*)pItem)->GetValue() ? "ON" : "OFF" ) );
            }
            break;
        case RES_CHRATR_CROSSEDOUT:
            {
                sprintf( sOut,
                    "CrossedOut: Style[%d]",
                    ((const SvxCrossedOutItem*)pItem)->GetValue() );
            }
            break;
        case RES_CHRATR_ESCAPEMENT:
            {
                sprintf( sOut,
                    "Escapement: Style[%d, %d]",
                    ((const SvxEscapementItem*)pItem)->GetEsc(),
                    ((const SvxEscapementItem*)pItem)->GetProp() );
            }
            break;
        case RES_CHRATR_FONT:
            {
                const SvxFontItem& rFont = *(const SvxFontItem*)pItem;
                sprintf( sOut,
                    "Font: Fam[%d] Nm[%s] Pitch[%d] CharSet[%d]",
                    rFont.GetFamily(),
                    rFont.GetFamilyName().GetStr(),
                    rFont.GetPitch(),
                    rFont.GetCharSet() );
            }
            break;
        case RES_CHRATR_FONTSIZE:
            {
                sprintf( sOut,
                    "FontSize: Height[%d]",
                    ((const SvxFontHeightItem*)pItem)->GetHeight() );
            }
            break;
        case RES_CHRATR_KERNING:
            {
                sprintf( sOut,
                    "Kerning: Style[%d]",
                    ((const SvxKerningItem*)pItem)->GetValue() );
            }
            break;
        case RES_CHRATR_LANGUAGE:
            {
                sprintf( sOut,
                    "Language: Style[%d]",
                    ((const SvxLanguageItem*)pItem)->GetValue() );
            }
            break;
        case RES_CHRATR_POSTURE:
            {
                sprintf( sOut,
                    "Posture: Style[%d]",
                    ((const SvxPostureItem*)pItem)->GetValue() );
            }
            break;

        case RES_CHRATR_SHADOWED:
            {
                sprintf( sOut,
                    "Shadowed: Style[%s]",
            ( ((const SvxShadowedItem*)pItem)->GetValue() ? "ON" : "OFF" ) );
            }
            break;

        case RES_CHRATR_UNDERLINE:
            {
                sprintf( sOut,
                    "Underline: Style[%d]",
                    ((const SvxUnderlineItem*)pItem)->GetValue() );
            }
            break;
        case RES_CHRATR_WEIGHT:
            {
                sprintf( sOut,
                    "Weight: Style[%d]",
                    ((const SvxWeightItem*)pItem)->GetValue() );
            }
            break;
        case RES_CHRATR_WORDLINEMODE:
            {
                sprintf( sOut,
                    "WordLine: Style[%s]",
            ( ((const SvxWordLineModeItem*)pItem)->GetValue() ? "ON" : "OFF" ) );
            }
            break;

        case RES_TXTATR_CHARFMT:
            *pOut << "CharFmt: "
                  << ((SwFmtCharFmt*)pItem)->GetCharFmt()->GetName()->GetStr()
                  << endl;
            }
            break;
        case RES_PARATR_LINESPACING:
            {
                sprintf( sOut,
                    "LineSpacing: Size[%d] Sp.Rule[%d] In.Sp.Rule[%d] In.Space[%d] Height[%d]",
                    ((const SvxLineSpacingItem*)pItem)->GetPropLineSpace(),
                    ((const SvxLineSpacingItem*)pItem)->GetLineSpaceRule(),
                    ((const SvxLineSpacingItem*)pItem)->GetInterLineSpaceRule(),
                    ((const SvxLineSpacingItem*)pItem)->GetInterLineSpace(),
                    ((const SvxLineSpacingItem*)pItem)->GetLineHeight() );
            }
            break;

        case RES_PARATR_ADJUST:
            {
                sprintf( sOut,
                    "Adjust: Style[%d]",
                    ((const SvxAdjustItem*)pItem)->GetAdjust() );
            }
            break;
        case RES_PARATR_TABSTOP:
            {
                const SvxTabStopItem & rAttr = *(const SvxTabStopItem*)pItem;
                *pOut << "TabStops: (" << endl;
                for( USHORT n = 0; n < rAttr.Count(); n++ )
                {
                    const SvxTabStop & rTab = rAttr[ n ];
                    sprintf( sOut,
                                "\t\tTabStop: Style[%d] Pos[%d] Around[%c] Fill[%c]",
                                rTab.GetAdjustment(),
                                rTab.GetTabPos(),
                                rTab.GetDecimal(),
                                rTab.GetFill() );
                    *pOut << sOut << endl;
                }
                *pOut << ')' << endl;
                sOut[0] = 0;
            }
            break;
        case RES_PARATR_HYPHENZONE:
                *pOut << "HYPHENZONE" << endl;
                break;

        case RES_LR_SPACE:
            {
                sprintf( sOut,
                    "LRSpace: Left[%d], Right[%d]",
                    ((const SvxLRSpaceItem*)pItem)->GetLeft(),
                    ((const SvxLRSpaceItem*)pItem)->GetRight() );
            }
            break;
        case RES_UL_SPACE:
            {
                sprintf( sOut,
                    "ULSpace: Upper[%d], Lower[%d]",
                    ((const SvxULSpaceItem*)pItem)->GetUpper(),
                    ((const SvxULSpaceItem*)pItem)->GetLower() );
            }
            break;

        case RES_BACKGROUND:
                *pOut << "BACKGROUND" << endl;
                break;
        case RES_BOX:
                *pOut << "BOX" << endl;
                break;
        case RES_SHADOW:
                *pOut << "SHADOW" << endl;
                break;

        default:
            sprintf( sOut, "UnknownItem: %d", pItem->Which() );
        }

        if( sOut[0] )
            *pOut << sOut << endl;

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
}

static void OutStyle( const SwPaM& rPam, const SwTxtFmtColl& rColl )
{
    char sOut[ 200 ];
    sprintf( sOut, "Style: [%5d|%5d] - [%5d|%5d]",
        rPam.GetMark()->nNode.GetIndex(),
        rPam.GetMark()->nContent.GetIndex(),
        rPam.GetPoint()->nNode.GetIndex(),
        rPam.GetPoint()->nContent.GetIndex() );
    *pOut << sOut
        << " ®"
        << rColl.GetName()->GetStr()
        << "¯ {" << endl;
    OutAttr( rColl.GetAttrSet() );
    *pOut << '}' << endl;
}

static void OutSet( const SwPaM& rPam, const SfxItemSet& rSet )
{
    char sOut[ 200 ];
    sprintf( sOut, "Attr: [%5d|%5d] - [%5d|%5d] {",
        rPam.GetMark()->nNode.GetIndex(),
        rPam.GetMark()->nContent.GetIndex(),
        rPam.GetPoint()->nNode.GetIndex(),
        rPam.GetPoint()->nContent.GetIndex() );
    *pOut << sOut << endl;
    OutAttr( rSet );
    *pOut << '}' << endl;
}

static void OutText( const SwPaM& rPam, const char* pText )
{
    char sOut[ 200 ];
    sprintf( sOut, "Text: [%5d|%5d] - [%5d|%5d]",
        rPam.GetMark()->nNode.GetIndex(),
        rPam.GetMark()->nContent.GetIndex(),
        rPam.GetPoint()->nNode.GetIndex(),
        rPam.GetPoint()->nContent.GetIndex() );
    *pOut << sOut
        << " ®"
        << pText
        << '¯' << endl;
}

static void DumpStart()
{
    TempFile aTempFile;

    pOut = new SvFileStream( aTempFile.GetName(),
                            STREAM_WRITE | STREAM_TRUNC );
    *pOut << "Dump des RTF30-Parsers" << endl;
}

static void DumpEnde()
{
    *pOut  << endl<< "Das wars" << endl;
    delete pOut;
    pOut = 0;
}

#endif


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/rtf/swparrtf.cxx,v 1.1.1.1 2000-09-18 17:14:56 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.159  2000/09/18 16:04:51  willem.vandorp
      OpenOffice header added.

      Revision 1.158  2000/08/07 15:13:02  jp
      Bug #77342#: use String::CreateFromInt32

      Revision 1.157  2000/08/04 10:48:19  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character; use rtfout functions

      Revision 1.156  2000/07/20 13:16:26  jp
      change old txtatr-character to the two new characters

      Revision 1.155  2000/06/13 09:42:01  os
      using UCB

      Revision 1.154  2000/05/26 07:22:44  os
      old SW Basic API Slots removed

      Revision 1.153  2000/05/09 17:23:01  jp
      Changes for Unicode

      Revision 1.152  2000/03/21 16:30:56  jp
      Bug #74351# ReadSectControls: reset column from the first pagedesc too

      Revision 1.151  2000/03/14 17:25:21  jp
      Bug #73968#: read and set outline/num-levels

      Revision 1.150  2000/03/14 09:22:29  jp
      Bug #73941#: remove unused numrules, share override rules

      Revision 1.149  2000/02/24 18:31:42  jp
      Bug #73480#: ReadSectControls - dont set columns at PageDesc and Region

      Revision 1.148  2000/02/11 17:36:06  jp
      Bug #72146#: read UniCode character

      Revision 1.147  2000/02/11 14:38:06  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.146  2000/01/25 20:13:27  jp
      Bug #72146#: read UniCode character

      Revision 1.145  2000/01/07 12:26:54  jp
      Task #71411#: read/write NoBalancedColumns

      Revision 1.144  1999/12/13 13:15:42  jp
      Bug #70916#: ROW - skip to the end of the current line and then move the cursor one node forward

*************************************************************************/

