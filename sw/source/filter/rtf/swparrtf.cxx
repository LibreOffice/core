/*************************************************************************
 *
 *  $RCSfile: swparrtf.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:38:10 $
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
 *  Contributor(s): cmc@openoffice.org, tono@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <stack>

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
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX
#include <svx/hyznitem.hxx>
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
#ifndef _VIEWSH_HXX     // for the pagedescname from the ShellRes
#include <viewsh.hxx>
#endif
#ifndef _SHELLRES_HXX   // for the pagedescname from the ShellRes
#include <shellres.hxx>
#endif
#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#include <hfspacingitem.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif

#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>
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
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#ifndef SW_MS_MSFILTER_HXX
#include "../inc/msfilter.hxx"
#endif

// einige Hilfs-Funktionen
// char
inline const SvxFontHeightItem& GetSize(const SfxItemSet& rSet,BOOL bInP=TRUE)
    { return (const SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE,bInP); }
inline const SvxLRSpaceItem& GetLRSpace(const SfxItemSet& rSet,BOOL bInP=TRUE)
    { return (const SvxLRSpaceItem&)rSet.Get( RES_LR_SPACE,bInP); }


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
    maSegments(*this),
    pDoc( pD ),
    pTableNode( 0 ), pOldTblNd( 0 ), nAktBox( 0 ), nNewNumSectDef( USHRT_MAX ),
    nAktPageDesc( 0 ), nAktFirstPageDesc( 0 ),
    pGrfAttrSet( 0 ),
    aMergeBoxes( 0, 5 ),
    nInsTblRow( USHRT_MAX ),
    pSttNdIdx( 0 ),
    pRegionEndIdx( 0 ),
    pRelNumRule( new SwRelNumRuleSpaces( *pD, bReadNewDoc )),
    aTblFmts( 0, 10 ),
    mpBookmarkStart(0)
{
    mbIsFootnote = mbReadNoTbl = bReadSwFly = bSwPageDesc = bStyleTabValid =
    bInPgDscTbl = bNewNumList = false;
    bFirstContinue = true;

    pPam = new SwPaM( *rCrsr.GetPoint() );
    SetInsPos( SwxPosition( pPam ) );
    SetChkStyleAttr( 0 != bReadNewDoc );
    SetCalcValue( FALSE );
    SetReadDocInfo( TRUE );

    // diese sollen zusaetzlich ueber \pard zurueck gesetzt werden
    USHORT temp;
    temp = RES_TXTATR_CHARFMT;      AddPlainAttr( temp );
    temp = RES_PAGEDESC;            AddPardAttr( temp );
    temp = RES_BREAK;               AddPardAttr( temp );
    temp = RES_PARATR_NUMRULE;      AddPardAttr( temp );
    temp = FN_PARAM_NUM_LEVEL;          AddPardAttr( temp );
}

// Aufruf des Parsers
SvParserState SwRTFParser::CallParser()
{
    mbReadNoTbl = false;
    bFirstContinue = true;

    rInput.Seek(STREAM_SEEK_TO_BEGIN);
    rInput.ResetError();

    return SvxRTFParser::CallParser();
}

bool lcl_UsedPara(SwPaM &rPam)
{
    const SwCntntNode* pCNd;
    const SwAttrSet* pSet;
    if( rPam.GetPoint()->nContent.GetIndex() ||
        ( 0 != ( pCNd = rPam.GetCntntNode()) &&
          0 != ( pSet = pCNd->GetpSwAttrSet()) &&
         ( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE ) ||
           SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE ))))
        return true;
    return false;
}

void SwRTFParser::Continue( int nToken )
{
    if( bFirstContinue )
    {
        bFirstContinue = FALSE;

        if (IsNewDoc())
        {
            pDoc->SetParaSpaceMax(true, true);
            pDoc->SetTabCompat(true);
            pDoc->_SetUseVirtualDevice(true);
        }

        // einen temporaeren Index anlegen, auf Pos 0 so wird er nicht bewegt!
        pSttNdIdx = new SwNodeIndex( pDoc->GetNodes() );
        if( !IsNewDoc() )       // in ein Dokument einfuegen ?
        {
            const SwPosition* pPos = pPam->GetPoint();
            SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();

            pDoc->SplitNode( *pPos );

            *pSttNdIdx = pPos->nNode.GetIndex()-1;
            pDoc->SplitNode( *pPos );

            SwPaM aInsertionRangePam( *pPos );

            pPam->Move( fnMoveBackward );

            // #106634# split any redline over the insertion point
            aInsertionRangePam.SetMark();
            *aInsertionRangePam.GetPoint() = *pPam->GetPoint();
            aInsertionRangePam.Move( fnMoveBackward );
            pDoc->SplitRedline( aInsertionRangePam );

            pDoc->SetTxtFmtColl( *pPam, pDoc->GetTxtCollFromPoolSimple
                                 ( RES_POOLCOLL_STANDARD, FALSE ));

            // verhinder das einlesen von Tabellen in Fussnoten / Tabellen
            ULONG nNd = pPos->nNode.GetIndex();
            mbReadNoTbl = 0 != pSttNd->FindTableNode() ||
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

    // JP 13.08.98: TabellenUmrandungen optimieren - Bug 53525
    for( USHORT n = aTblFmts.Count(); n; )
    {
        if (SwTable* pTbl = SwTable::FindTable( (SwFrmFmt*)aTblFmts[ --n ] ))
            pTbl->GCBorderLines();
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

        sal_uInt16 nPageDescOffset = pDoc->GetPageDescCnt();
        maSegments.InsertSegments(IsNewDoc());
        UpdatePageDescs(*pDoc, nPageDescOffset);

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
            else if (0 != (pAktNd = pDoc->GetNodes()[nNodeIdx]->GetTxtNode()))
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

    pDoc->SetUpdateExpFldStat();
    pDoc->SetInitDBFields(true);

    // Laufbalken bei asynchronen Call nicht einschalten !!!
    ::EndProgress( pDoc->GetDocShell() );
}

bool rtfSections::SetCols(SwFrmFmt &rFmt, const rtfSection &rSection,
    USHORT nNettoWidth)
{
    //sprmSCcolumns - Anzahl der Spalten - 1
    USHORT nCols = rSection.NoCols();

    if (nCols < 2)
        return false;                   // keine oder bloedsinnige Spalten

    SwFmtCol aCol;                      // Erzeuge SwFmtCol

    //sprmSDxaColumns   - Default-Abstand 1.25 cm
    USHORT nColSpace = rSection.StandardColSeperation();

#if 0
    // sprmSLBetween
    if (rSection.maSep.fLBetween)
    {
        aCol.SetLineAdj( COLADJ_TOP );      // Line
        aCol.SetLineHeight( 100 );
        aCol.SetLineColor( Color( COL_BLACK ));
        aCol.SetLineWidth( 1 );
    }
#endif
    aCol.Init( nCols, nColSpace, nNettoWidth );

    // not SFEvenlySpaced
    if (rSection.maPageInfo.maColumns.size())
    {
        aCol._SetOrtho(false);
        USHORT nWishWidth = 0, nHalfPrev = 0;
        for (USHORT n = 0, i = 0; n < rSection.maPageInfo.maColumns.size(); n += 2, ++i )
        {
            SwColumn* pCol = aCol.GetColumns()[ i ];
            pCol->SetLeft( nHalfPrev );
            USHORT nSp = rSection.maPageInfo.maColumns[ n+1 ];
            nHalfPrev = nSp / 2;
            pCol->SetRight( nSp - nHalfPrev );
            pCol->SetWishWidth(rSection.maPageInfo.maColumns[ n ] +
                pCol->GetLeft() + pCol->GetRight());
            nWishWidth += pCol->GetWishWidth();
        }
        aCol.SetWishWidth( nWishWidth );
    }

    rFmt.SetAttr(aCol);
    return true;
}

void rtfSections::SetPage(SwPageDesc &rInPageDesc, SwFrmFmt &rFmt,
    const rtfSection &rSection, bool bIgnoreCols)
{
    // 1. Orientierung
    rInPageDesc.SetLandscape(rSection.IsLandScape());

    // 2. Papiergroesse
    SwFmtFrmSize aSz(rFmt.GetFrmSize());
    aSz.SetWidth(rSection.GetPageWidth());
    aSz.SetHeight(rSection.GetPageHeight());
    rFmt.SetAttr(aSz);

    rFmt.SetAttr(
        SvxLRSpaceItem(rSection.GetPageLeft(), rSection.GetPageRight()));

    if (!bIgnoreCols)
    {
        SetCols(rFmt, rSection, rSection.GetPageWidth() -
            rSection.GetPageLeft() - rSection.GetPageRight());
    }

    rFmt.SetAttr(rSection.maPageInfo.maBox);
}

bool HasHeader(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pHd;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_HEADER, false, &pHd))
        return ((const SwFmtHeader *)(pHd))->IsActive();
    return false;
}

bool HasFooter(const SwFrmFmt &rFmt)
{
    const SfxPoolItem *pFt;
    if (SFX_ITEM_SET == rFmt.GetItemState(RES_FOOTER, false, &pFt))
        return ((const SwFmtFooter *)(pFt))->IsActive();
    return false;
}

void rtfSections::GetPageULData(const rtfSection &rSection, bool bFirst,
    rtfSections::wwULSpaceData& rData)
{
    short nWWUp = rSection.maPageInfo.mnMargtsxn;
    short nWWLo = rSection.maPageInfo.mnMargbsxn;
    short nWWHTop = rSection.maPageInfo.mnHeadery;
    short nWWFBot = rSection.maPageInfo.mnFootery;

#if 0
    /*
    If there is gutter in 97+ and the dop says put it on top then get the
    gutter distance and set it to the top margin. When we are "two pages
    in one" the gutter is put at the top of odd pages, and bottom of
    even pages, something we cannot do. So we will put it on top of all
    pages, that way the pages are at least the right size.
    */
    if ( mrReader.pWDop->doptypography.f2on1 ||
        (!mrReader.bVer67 && mrReader.pWDop->iGutterPos &&
         rSection.maSep.fRTLGutter)
       )
    {
        nWWUp += rSection.maSep.dzaGutter;
    }
#endif

    if (bFirst)
    {
        if (
            rSection.mpTitlePage && HasHeader(rSection.mpTitlePage->GetMaster())
           )
        {
            rData.bHasHeader = true;
        }
    }
    else
    {
        if (rSection.mpPage &&
               (
               HasHeader(rSection.mpPage->GetMaster())
               || HasHeader(rSection.mpPage->GetLeft())
               )
           )
        {
            rData.bHasHeader = true;
        }
    }

    if( rData.bHasHeader )
    {
        rData.nSwUp  = nWWHTop;             // Header -> umrechnen
        rData.nSwHLo = nWWUp - nWWHTop;

        if (rData.nSwHLo < MM50)
            rData.nSwHLo = MM50;
    }
    else // kein Header -> Up einfach uebernehmen
        rData.nSwUp = nWWUp;

    if (bFirst)
    {
        if (
                rSection.mpTitlePage &&
                HasFooter(rSection.mpTitlePage->GetMaster())
           )
        {
            rData.bHasFooter = true;
        }
    }
    else
    {
        if (rSection.mpPage &&
           (
               HasFooter(rSection.mpPage->GetMaster())
               || HasFooter(rSection.mpPage->GetLeft())
           )
           )
        {
            rData.bHasFooter = true;
        }
    }

    if( rData.bHasFooter )
    {
        rData.nSwLo = nWWFBot;              // Footer -> Umrechnen
        rData.nSwFUp = nWWLo - nWWFBot;

        if (rData.nSwFUp < MM50)
            rData.nSwFUp = MM50;
    }
    else // kein Footer -> Lo einfach uebernehmen
        rData.nSwLo = nWWLo;
}

void rtfSections::SetPageULSpaceItems(SwFrmFmt &rFmt,
    rtfSections::wwULSpaceData& rData)
{
    if (rData.bHasHeader)               // ... und Header-Lower setzen
    {
        //Kopfzeilenhoehe minimal sezten
        if (SwFrmFmt* pHdFmt = (SwFrmFmt*)rFmt.GetHeader().GetHeaderFmt())
        {
            pHdFmt->SetAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwHLo));
            SvxULSpaceItem aHdUL(pHdFmt->GetULSpace());
            aHdUL.SetLower(rData.nSwHLo - MM50);
            pHdFmt->SetAttr(aHdUL);
            pHdFmt->SetAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    if (rData.bHasFooter)               // ... und Footer-Upper setzen
    {
        if (SwFrmFmt* pFtFmt = (SwFrmFmt*)rFmt.GetFooter().GetFooterFmt())
        {
            pFtFmt->SetAttr(SwFmtFrmSize(ATT_MIN_SIZE, 0, rData.nSwFUp));
            SvxULSpaceItem aFtUL(pFtFmt->GetULSpace());
            aFtUL.SetUpper(rData.nSwFUp - MM50);
            pFtFmt->SetAttr(aFtUL);
            pFtFmt->SetAttr(SwHeaderAndFooterEatSpacingItem(
                RES_HEADER_FOOTER_EAT_SPACING, true));
        }
    }

    SvxULSpaceItem aUL(rData.nSwUp, rData.nSwLo); // Page-UL setzen
    rFmt.SetAttr(aUL);
}

void rtfSections::SetSegmentToPageDesc(const rtfSection &rSection,
    bool bTitlePage, bool bIgnoreCols)
{
    SwPageDesc &rPage = bTitlePage ? *rSection.mpTitlePage : *rSection.mpPage;

//    SetNumberingType(rSection, rPage);

    SwFrmFmt &rFmt = rPage.GetMaster();
//    mrReader.SetDocumentGrid(rFmt, rSection);

    wwULSpaceData aULData;
    GetPageULData(rSection, bTitlePage, aULData);
    SetPageULSpaceItems(rFmt, aULData);

    SetPage(rPage, rFmt, rSection, bIgnoreCols);
}

void rtfSections::CopyFrom(const SwPageDesc &rFrom, SwPageDesc &rDest)
{
    UseOnPage ePage = rFrom.ReadUseOn();
    rDest.WriteUseOn(ePage);

    mrReader.pDoc->CopyHeader(rFrom.GetMaster(), rDest.GetMaster());
    SwFrmFmt &rDestFmt = rDest.GetMaster();
    rDestFmt.SetAttr(rFrom.GetMaster().GetHeader());
    mrReader.pDoc->CopyHeader(rFrom.GetLeft(), rDest.GetLeft());
    mrReader.pDoc->CopyFooter(rFrom.GetMaster(), rDest.GetMaster());
    mrReader.pDoc->CopyFooter(rFrom.GetLeft(), rDest.GetLeft());
}

void rtfSections::MoveFrom(SwPageDesc &rFrom, SwPageDesc &rDest)
{
    UseOnPage ePage = rFrom.ReadUseOn();
    rDest.WriteUseOn(ePage);

    SwFrmFmt &rDestMaster = rDest.GetMaster();
    SwFrmFmt &rFromMaster = rFrom.GetMaster();
    rDestMaster.SetAttr(rFromMaster.GetHeader());
    rDestMaster.SetAttr(rFromMaster.GetFooter());
    rFromMaster.SetAttr(SwFmtHeader());
    rFromMaster.SetAttr(SwFmtFooter());

    SwFrmFmt &rDestLeft = rDest.GetLeft();
    SwFrmFmt &rFromLeft = rFrom.GetLeft();
    rDestLeft.SetAttr(rFromLeft.GetHeader());
    rDestLeft.SetAttr(rFromLeft.GetFooter());
    rFromLeft.SetAttr(SwFmtHeader());
    rFromLeft.SetAttr(SwFmtFooter());
}

void rtfSections::SetHdFt(rtfSection &rSection)
{
    ASSERT(rSection.mpPage, "makes no sense to call without a main page");
    if (rSection.mpPage && rSection.maPageInfo.mpPageHdFt)
    {
        if (rSection.maPageInfo.mbPageHdFtUsed)
        {
            MoveFrom(*rSection.maPageInfo.mpPageHdFt, *rSection.mpPage);
            rSection.maPageInfo.mbPageHdFtUsed = false;
            rSection.maPageInfo.mpPageHdFt = rSection.mpPage;
        }
        else
            CopyFrom(*rSection.maPageInfo.mpPageHdFt, *rSection.mpPage);
    }

    if (rSection.mpTitlePage && rSection.maPageInfo.mpTitlePageHdFt)
    {
        if (rSection.maPageInfo.mbTitlePageHdFtUsed)
        {
            MoveFrom(*rSection.maPageInfo.mpTitlePageHdFt,
                    *rSection.mpTitlePage);
            rSection.maPageInfo.mbTitlePageHdFtUsed = false;
            rSection.maPageInfo.mpTitlePageHdFt = rSection.mpTitlePage;
        }
        else
        {
            CopyFrom(*rSection.maPageInfo.mpTitlePageHdFt,
                    *rSection.mpTitlePage);
        }
    }
}

SwSectionFmt *rtfSections::InsertSection(SwPaM& rMyPaM, rtfSection &rSection)
{
    SwSection aSection(CONTENT_SECTION, mrReader.pDoc->GetUniqueSectionName());

    SfxItemSet aSet( mrReader.pDoc->GetAttrPool(), aFrmFmtSetRange );

    sal_uInt8 nRTLPgn = maSegments.empty() ? 0 : maSegments.back().IsBiDi();
    aSet.Put(SvxFrameDirectionItem(
        nRTLPgn ? FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP));

#if 0
    if (2 == mrReader.pWDop->fpc)
        aSet.Put( SwFmtFtnAtTxtEnd(FTNEND_ATTXTEND));
    if (0 == mrReader.pWDop->epc)
        aSet.Put( SwFmtEndAtTxtEnd(FTNEND_ATTXTEND));
#endif

    rSection.mpSection = mrReader.pDoc->Insert( rMyPaM, aSection, &aSet );
    ASSERT(rSection.mpSection, "section not inserted!");
    if (!rSection.mpSection)
        return 0;

    SwPageDesc *pPage = 0;
    mySegrIter aEnd = maSegments.rend();
    for (mySegrIter aIter = maSegments.rbegin(); aIter != aEnd; ++aIter)
    {
        if (pPage = aIter->mpPage)
            break;
    }

    ASSERT(pPage, "no page outside this section!");

    if (!pPage)
        pPage = &mrReader.pDoc->_GetPageDesc(0);

    if (!pPage)
        return 0;

    SwFrmFmt& rFmt = pPage->GetMaster();
    const SwFmtFrmSize&   rSz = rFmt.GetFrmSize();
    const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
    SwTwips nWidth = rSz.GetWidth();
    long nLeft  = rLR.GetTxtLeft();
    long nRight = rLR.GetRight();

    SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
    ASSERT(pFmt, "impossible");
    if (!pFmt)
        return 0;
    SetCols(*pFmt, rSection, (USHORT)(nWidth - nLeft - nRight) );

#if 0
    //Set the columns to be UnBalanced if compatability option is set
    if (mrReader.pWDop->fNoColumnBalance  )
    {
        SwSectionFmt *pFmt = rSection.mpSection->GetFmt();
        pFmt->SetAttr(SwFmtNoBalancedColumns(true));
    }
#endif

    return pFmt;
}

void rtfSections::InsertSegments(bool bNewDoc)
{
    sal_uInt16 nDesc(0);
    mySegIter aEnd = maSegments.end();
    mySegIter aStart = maSegments.begin();
    for (mySegIter aIter = aStart; aIter != aEnd; ++aIter)
    {
        mySegIter aNext = aIter+1;

        bool bInsertSection = aIter != aStart ? aIter->IsContinous() : false;

        if (!bInsertSection)
        {
            /*
             If a cont section follow this section then we won't be
             creating a page desc with 2+ cols as we cannot host a one
             col section in a 2+ col pagedesc and make it look like
             word. But if the current section actually has columns then
             we are forced to insert a section here as well as a page
             descriptor.
            */

            /*
             Note for the future:
             If we want to import "protected sections" the here is
             where we would also test for that and force a section
             insertion if that was true.
            */
            bool bIgnoreCols = false;
            if (aNext != aEnd && aNext->IsContinous())
            {
                bIgnoreCols = true;
                if (aIter->NoCols() > 1)
                    bInsertSection = true;
            }

            if (aIter->HasTitlePage())
            {
                if (bNewDoc && aIter == aStart)
                {
                    aIter->mpTitlePage =
                        mrReader.pDoc->GetPageDescFromPool(RES_POOLPAGE_FIRST);
                }
                else
                {
                    USHORT nPos = mrReader.pDoc->MakePageDesc(
                        ViewShell::GetShellRes()->GetPageDescName(nDesc)
                        , 0, false);
                    aIter->mpTitlePage = &mrReader.pDoc->_GetPageDesc(nPos);
                }
                ASSERT(aIter->mpTitlePage, "no page!");
                if (!aIter->mpTitlePage)
                    continue;

                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            }

            if (!bNewDoc && aIter == aStart)
                continue;
            else if (bNewDoc && aIter == aStart)
            {
                aIter->mpPage =
                    mrReader.pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD);
            }
            else
            {
                USHORT nPos = mrReader.pDoc->MakePageDesc(
                    ViewShell::GetShellRes()->GetPageDescName(nDesc,
                        false, aIter->HasTitlePage()),
                        aIter->mpTitlePage, false);
                aIter->mpPage = &mrReader.pDoc->_GetPageDesc(nPos);
            }
            ASSERT(aIter->mpPage, "no page!");
            if (!aIter->mpPage)
                continue;

            SetHdFt(*aIter);

            if (aIter->mpTitlePage)
                SetSegmentToPageDesc(*aIter, true, bIgnoreCols);
            SetSegmentToPageDesc(*aIter, false, bIgnoreCols);

            SwFmtPageDesc aPgDesc(aIter->HasTitlePage() ?
                    aIter->mpTitlePage : aIter->mpPage);

            if (aIter->mpTitlePage)
                aIter->mpTitlePage->SetFollow(aIter->mpPage);

            if (aIter->PageRestartNo() ||
                ((aIter == aStart) && aIter->PageStartAt() != 1))
                aPgDesc.SetNumOffset(aIter->PageStartAt());

            /*
            If its a table here, apply the pagebreak to the table
            properties, otherwise we add it to the para at this
            position
            */
            if (aIter->maStart.GetNode().IsTableNode())
            {
                SwTable& rTable =
                    aIter->maStart.GetNode().GetTableNode()->GetTable();
                SwFrmFmt* pApply = rTable.GetFrmFmt();
                ASSERT(pApply, "impossible");
                if (pApply)
                    pApply->SetAttr(aPgDesc);
            }
            else
            {
                SwPosition aPamStart(aIter->maStart);
                aPamStart.nContent.Assign(
                    aIter->maStart.GetNode().GetCntntNode(), 0);
                SwPaM aPage(aPamStart);

                mrReader.pDoc->Insert(aPage, aPgDesc);
            }
            ++nDesc;
        }

        SwTxtNode* pTxtNd = 0;
        if (bInsertSection)
        {
            SwPaM aSectPaM(*mrReader.pPam);
            SwNodeIndex aAnchor(aSectPaM.GetPoint()->nNode);
            if (aNext != aEnd)
            {
                aAnchor = aNext->maStart;
                aSectPaM.GetPoint()->nNode = aAnchor;
                aSectPaM.GetPoint()->nContent.Assign(
                    aNext->maStart.GetNode().GetCntntNode(), 0);
                aSectPaM.Move(fnMoveBackward);
            }

            const SwPosition* pPos  = aSectPaM.GetPoint();
            const SwTxtNode* pSttNd =
                mrReader.pDoc->GetNodes()[ pPos->nNode ]->GetTxtNode();
            const SwTableNode* pTableNd = pSttNd ? pSttNd->FindTableNode() : 0;
            if (pTableNd)
            {
                pTxtNd =
                    mrReader.pDoc->GetNodes().MakeTxtNode(aAnchor,
                    mrReader.pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT ));

                aSectPaM.GetPoint()->nNode = SwNodeIndex(*pTxtNd);
                aSectPaM.GetPoint()->nContent.Assign(
                    aSectPaM.GetCntntNode(), 0);
            }

            aSectPaM.SetMark();

            aSectPaM.GetPoint()->nNode = aIter->maStart;
            aSectPaM.GetPoint()->nContent.Assign(
                aSectPaM.GetCntntNode(), 0);

            SwSectionFmt *pRet = InsertSection(aSectPaM, *aIter);
            //The last section if continous is always unbalanced
            if (aNext == aEnd && pRet)
                pRet->SetAttr(SwFmtNoBalancedColumns(true));
        }

        if (pTxtNd)
        {
            SwNodeIndex aIdx(*pTxtNd);
            SwPosition aPos(aIdx);
            SwPaM aTest(aPos);
            mrReader.pDoc->DelFullPara(aTest);
            pTxtNd = 0;
        }
    }
    myrDummyIter aDEnd = maDummyPageNos.rend();
    for (myrDummyIter aI = maDummyPageNos.rbegin(); aI != aDEnd; ++aI)
        mrReader.pDoc->DelPageDesc(*aI);
}

SwRTFParser::~SwRTFParser()
{
    for(::std::map<SwTableNode *, SwNodeIndex*>::iterator aIter
        = maTables.begin(); aIter != maTables.end(); ++aIter)
    {
        // exitiert schon ein Layout, dann muss an dieser Tabelle die
        // BoxFrames neu erzeugt
        SwTableNode *pTable = aIter->first;
        SwNodeIndex *pIndex = aIter->second;
        pTable->DelFrms();
        pTable->MakeFrms(pIndex);
    }

    delete pSttNdIdx;
    delete pRegionEndIdx;
    delete pPam;
    delete pRelNumRule;

    if (aFlyArr.Count())
        aFlyArr.DeleteAndDestroy( 0, aFlyArr.Count() );

    if (pGrfAttrSet)
        DELETEZ( pGrfAttrSet );
}

extern void sw3io_ConvertFromOldField( SwDoc& rDoc, USHORT& rWhich,
                                USHORT& rSubType, ULONG &rFmt,
                                USHORT nVersion );

void SwRTFParser::NextToken( int nToken )
{
    USHORT eDateFmt;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
        //We can only insert a footnote if we're not inside a footnote. e.g. #i7713#
        if (!mbIsFootnote)
        {
            ReadHeaderFooter( nToken );
            SkipToken( -1 );        // Klammer wieder zurueck
        }
        break;
    case RTF_SWG_PRTDATA:           ReadPrtData();              break;
    case RTF_FIELD:                 ReadField();                break;
    case RTF_SHPPICT:
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
        if( bNewNumList )
            SkipGroup();
        else
        {
            bStyleTabValid = TRUE;
            if (SwNumRule* pRule = ReadNumSecLevel( nToken ))
            {
                GetAttrSet().Put( SwNumRuleItem( pRule->GetName() ));

                if( SFX_ITEM_SET != GetAttrSet().GetItemState( FN_PARAM_NUM_LEVEL, FALSE ))
                    GetAttrSet().Put( SfxUInt16Item( FN_PARAM_NUM_LEVEL, 0 ));
            }
        }
        break;


    case RTF_BKMKSTART:
        if(RTF_TEXTTOKEN == GetNextToken())
            mpBookmarkStart = new BookmarkPosition(*pPam);
        else
            SkipToken(-1);

        SkipGroup();
        break;

    case RTF_BKMKEND:
        if(RTF_TEXTTOKEN == GetNextToken())
        {
            const String& sBookmark = aToken;
            KeyCode aEmptyKeyCode;
            if (mpBookmarkStart)
            {
                BookmarkPosition aBookmarkEnd(*pPam);
                SwPaM aBookmarkRegion(  mpBookmarkStart->maMkNode, mpBookmarkStart->mnMkCntnt,
                                        aBookmarkEnd.maMkNode, aBookmarkEnd.mnMkCntnt);
                if (*mpBookmarkStart == aBookmarkEnd)
                    aBookmarkRegion.DeleteMark();
                pDoc->MakeBookmark(aBookmarkRegion, aEmptyKeyCode, sBookmark, aEmptyStr);
            }
            delete mpBookmarkStart, mpBookmarkStart = 0;
        }
        else
            SkipToken(-1);

        SkipGroup();
        break;


    case RTF_PNSECLVL:
        if( bNewNumList )
            SkipGroup();
        else
            ReadNumSecLevel( nToken );
        break;

    case RTF_PNTEXT:
    case RTF_NONSHPPICT:
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
        if (lcl_UsedPara(*pPam))
            InsertPara();
        CheckInsNewTblLine();
        pDoc->Insert(*pPam, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE));
        break;

    case RTF_SECT:
        ReadSectControls( nToken );
        break;
    case RTF_CELL:
        if (CantUseTables())
            InsertPara();
        else
        {
            // Tabelle nicht mehr vorhanden ?
            if (USHRT_MAX != nInsTblRow && !pTableNode)
                NewTblLine();               // evt. Line copieren
            GotoNextBox();
        }
        break;

    case RTF_ROW:
        if (!CantUseTables())
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
        if (!CantUseTables())
        {
            if( !pTableNode )           // Tabelle nicht mehr vorhanden ?
            {
                if (RTF_TROWD != GetNextToken())
                    NewTblLine();           // evt. Line copieren
                SkipToken(-1);
            }
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
    case RTF_LYTPRTMET:
        if (IsNewDoc())
            pDoc->_SetUseVirtualDevice(false);
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
        pColl = pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD, FALSE );
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
            pDoc->SetTxtFmtColl( aPam, pColl, FALSE );
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

        pDoc->Insert( aPam, rSet.GetAttrSet(), SETATTR_DONTCHGNUMRULE );
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

    bool bNoNum = true;
    if (
        (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(RES_PARATR_NUMRULE))
     || (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState(FN_PARAM_NUM_LEVEL))
       )
    {
        bNoNum = false;
    }

    if (bNoNum)
    {
        for( ULONG n = nSNd; n <= nENd; ++n )
        {
            SwTxtNode* pTxtNd = pDoc->GetNodes()[ n ]->GetTxtNode();
            if( pTxtNd )
            {
                pTxtNd->SwCntntNode::SetAttr(
                    *GetDfltAttr(RES_PARATR_NUMRULE));
                pTxtNd->UpdateNum(SwNodeNum(NO_NUMBERING));
            }
        }
    }
}

DocPageInformation::DocPageInformation()
    : mnPaperw(12240), mnPaperh(15840), mnMargl(1800), mnMargr(1800),
    mnMargt(1440), mnMargb(1440), mnGutter(0), mnPgnStart(1), mbFacingp(false),
    mbLandscape(false), mbRTLdoc(false)
{
}

SectPageInformation::SectPageInformation(const DocPageInformation &rDoc)
    :
    mpTitlePageHdFt(0), mpPageHdFt(0), maBox(rDoc.maBox),
    mnPgwsxn(rDoc.mnPaperw), mnPghsxn(rDoc.mnPaperh),
    mnMarglsxn(rDoc.mnMargl), mnMargrsxn(rDoc.mnMargr),
    mnMargtsxn(rDoc.mnMargt), mnMargbsxn(rDoc.mnMargb),
    mnGutterxsn(rDoc.mnGutter), mnHeadery(720), mnFootery(720),
    mnPgnStarts(rDoc.mnPgnStart), mnCols(1), mnColsx(720),
    mnStextflow(rDoc.mbRTLdoc ? 3 : 0), mnBkc(2), mbLndscpsxn(rDoc.mbLandscape),
    mbTitlepg(false), mbFacpgsxn(rDoc.mbFacingp), mbRTLsection(rDoc.mbRTLdoc),
    mbPgnrestart(false), mbTitlePageHdFtUsed(false), mbPageHdFtUsed(false)
{
};

SectPageInformation::SectPageInformation(const SectPageInformation &rSect)
    : maColumns(rSect.maColumns), maBox(rSect.maBox), maNumType(rSect.maNumType),
    mpTitlePageHdFt(rSect.mpTitlePageHdFt), mpPageHdFt(rSect.mpPageHdFt),
    mnPgwsxn(rSect.mnPgwsxn), mnPghsxn(rSect.mnPghsxn),
    mnMarglsxn(rSect.mnMarglsxn), mnMargrsxn(rSect.mnMargrsxn),
    mnMargtsxn(rSect.mnMargtsxn), mnMargbsxn(rSect.mnMargbsxn),
    mnGutterxsn(rSect.mnGutterxsn), mnHeadery(rSect.mnHeadery),
    mnFootery(rSect.mnFootery), mnPgnStarts(rSect.mnPgnStarts),
    mnCols(rSect.mnCols), mnColsx(rSect.mnColsx),
    mnStextflow(rSect.mnStextflow), mnBkc(rSect.mnBkc),
    mbLndscpsxn(rSect.mbLndscpsxn), mbTitlepg(rSect.mbTitlepg),
    mbFacpgsxn(rSect.mbFacpgsxn), mbRTLsection(rSect.mbRTLsection),
    mbPgnrestart(rSect.mbPgnrestart),
    mbTitlePageHdFtUsed(rSect.mbTitlePageHdFtUsed),
    mbPageHdFtUsed(rSect.mbPageHdFtUsed)
{
};

rtfSection::rtfSection(const SwPosition &rPos,
    const SectPageInformation &rPageInfo)
    : maStart(rPos.nNode), maPageInfo(rPageInfo), mpSection(0), mpTitlePage(0),
    mpPage(0)
{
}

void rtfSections::push_back(const rtfSection &rSect)
{
    if (!maSegments.empty() && (maSegments.back().maStart == rSect.maStart))
        maSegments.pop_back();
    maSegments.push_back(rSect);
}

// lese alle Dokument-Controls ein
void SwRTFParser::SetPageInformationAsDefault(const DocPageInformation &rInfo)
{
    maSegments.push_back(rtfSection(*pPam->GetPoint(),
        SectPageInformation(rInfo)));

    if (!bSwPageDesc && IsNewDoc())
    {
        SwFmtFrmSize aFrmSize(ATT_FIX_SIZE, rInfo.mnPaperw, rInfo.mnPaperh);

        SvxLRSpaceItem aLR(rInfo.mnMargl, rInfo.mnMargr);
        SvxULSpaceItem aUL(rInfo.mnMargt, rInfo.mnMargb);

        UseOnPage eUseOn;
        if (rInfo.mbFacingp)
            eUseOn = UseOnPage(PD_MIRROR | PD_HEADERSHARE | PD_FOOTERSHARE);
        else
            eUseOn = UseOnPage(PD_ALL | PD_HEADERSHARE | PD_FOOTERSHARE);

        USHORT nPgStart(rInfo.mnPgnStart);

        SvxFrameDirectionItem aFrmDir(rInfo.mbRTLdoc ?
            FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP);

        // direkt an der Standartseite drehen
        SwPageDesc& rPg = pDoc->_GetPageDesc( 0 );
        rPg.WriteUseOn( eUseOn );

        if (rInfo.mbLandscape)
            rPg.SetLandscape(true);

        SwFrmFmt &rFmt1 = rPg.GetMaster(), &rFmt2 = rPg.GetLeft();

        rFmt1.SetAttr( aFrmSize );  rFmt2.SetAttr( aFrmSize );
        rFmt1.SetAttr( aLR );       rFmt2.SetAttr( aLR );
        rFmt1.SetAttr( aUL );       rFmt2.SetAttr( aUL );
        rFmt1.SetAttr( aFrmDir );   rFmt2.SetAttr( aFrmDir );

        // StartNummer der Seiten setzen
        if (nPgStart  != 1)
        {
            SwFmtPageDesc aPgDsc( &rPg );
            aPgDsc.SetNumOffset( nPgStart );
            pDoc->Insert( *pPam, aPgDsc );
        }
    }
}

void SwRTFParser::SetBorderLine(SvxBoxItem& rBox, sal_uInt16 nLine)
{
    int bWeiter = true;
    int nDistance = 0;
    int nPageDistance = 0;
    int nCol = 0;
    int nIdx = 0;
    int nLineThickness = 1;

    int nToken = GetNextToken();
    do {
        switch( nToken )
        {
        case RTF_BRDRS:
            nIdx = 1;
            break;

        case RTF_BRDRDB:
            nIdx = 3;
            break;

        case RTF_BRDRTRIPLE:
            nIdx = 10;
            break;

        case RTF_BRDRTNTHSG:
            nIdx = 11;
            break;

        case RTF_BRDRTHTNSG:
            nIdx = 12;
            break;

        case RTF_BRDRTNTHTNSG:
            nIdx = 13;
            break;

        case RTF_BRDRTNTHMG:
            nIdx = 14;
            break;

        case RTF_BRDRTHTNMG:
            nIdx = 15;
            break;

        case RTF_BRDRTNTHTNMG:
            nIdx = 16;
            break;

        case RTF_BRDRTNTHLG:
            nIdx = 17;
            break;

        case RTF_BRDRTHTNLG:
            nIdx = 18;
            break;

        case RTF_BRDRTNTHTNLG:
            nIdx = 19;
            break;

        case RTF_BRDRWAVY:
            nIdx = 20;
            break;

        case RTF_BRDRWAVYDB:
            nIdx = 21;
            break;

        case RTF_BRDREMBOSS:
            nIdx = 24;
            break;

        case RTF_BRDRENGRAVE:
            nIdx = 25;
            break;

        case RTF_BRSP:
             nPageDistance = nTokenValue;
            break;

        case RTF_BRDRDOT:           // SO does not have dashed or dotted lines
        case RTF_BRDRDASH:
        case RTF_BRDRDASHSM:
        case RTF_BRDRDASHD:
        case RTF_BRDRDASHDD:
        case RTF_BRDRDASHDOTSTR:
            break;

        case RTF_BRDRW:
            nLineThickness = nTokenValue;
            break;
        default:
            bWeiter = false;
            SkipToken(-1);
            break;
        }
        if (bWeiter)
            nToken = GetNextToken();
    } while (bWeiter && IsParserWorking());

    GetLineIndex(rBox, nLineThickness, nPageDistance, nCol, nIdx, nLine, nLine, 0);
}

// lese alle Dokument-Controls ein
void SwRTFParser::ReadDocControls( int nToken )
{
    int bWeiter = true;

    SwFtnInfo aFtnInfo;
    SwEndNoteInfo aEndInfo;
    bool bSetHyph = false;

    BOOL bEndInfoChgd = FALSE, bFtnInfoChgd = FALSE;

    do {
        USHORT nValue = USHORT( nTokenValue );
        switch( nToken )
        {
        case RTF_RTLDOC:
            maPageDefaults.mbRTLdoc = true;
            break;
        case RTF_LTRDOC:
            maPageDefaults.mbRTLdoc = false;
            break;
        case RTF_LANDSCAPE:
            maPageDefaults.mbLandscape = true;
            break;
        case RTF_PAPERW:
            if( 0 < nTokenValue )
                maPageDefaults.mnPaperw = nTokenValue;
            break;
        case RTF_PAPERH:
            if( 0 < nTokenValue )
                maPageDefaults.mnPaperh = nTokenValue;
            break;
        case RTF_MARGL:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargl = nTokenValue;
            break;
        case RTF_MARGR:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargr = nTokenValue;
            break;
        case RTF_MARGT:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargt = nTokenValue;
            break;
        case RTF_MARGB:
            if( 0 <= nTokenValue )
                maPageDefaults.mnMargb = nTokenValue;
            break;
        case RTF_FACINGP:
            maPageDefaults.mbFacingp = true;
            break;
        case RTF_PGNSTART:
            maPageDefaults.mnPgnStart = nTokenValue;
            break;
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
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNALC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNAUC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNRLC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNRUC:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER); bFtnInfoChgd = TRUE; break;
        case RTF_FTNNCHI:
            aFtnInfo.aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL); bFtnInfoChgd = TRUE; break;

        case RTF_AFTNNAR:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ARABIC); bEndInfoChgd = TRUE; break;
        case RTF_AFTNNALC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER_N);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNAUC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER_N);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNRLC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNRUC:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            bEndInfoChgd = TRUE;
            break;
        case RTF_AFTNNCHI:
            aEndInfo.aFmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            bEndInfoChgd = TRUE;
            break;
        case RTF_HYPHAUTO:
            if (nTokenValue)
                bSetHyph = true;
            //FOO//
            break;
        case RTF_PGBRDRT:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_TOP);
            break;

        case RTF_PGBRDRB:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_BOTTOM);
            break;

        case RTF_PGBRDRL:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_LEFT);
            break;

        case RTF_PGBRDRR:
            SetBorderLine(maPageDefaults.maBox, BOX_LINE_RIGHT);
            break;

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

    if (IsNewDoc())
    {
        if( bEndInfoChgd )
            pDoc->SetEndNoteInfo( aEndInfo );
        if( bFtnInfoChgd )
            pDoc->SetFtnInfo( aFtnInfo );
    }

    if (!bSwPageDesc)
    {
        SetPageInformationAsDefault(maPageDefaults);

        MakeStyleTab();

        SwTxtFmtColl* pColl = aTxtCollTbl.Get(0);
        if (!pColl)
        {
            pColl = pDoc->GetTxtCollFromPoolSimple(RES_POOLCOLL_STANDARD,
                FALSE );
        }

        ASSERT(pColl, "impossible to have no standard style");

        if (pColl)
        {
            if (
                IsNewDoc() && bSetHyph &&
                SFX_ITEM_SET != pColl->GetItemState(RES_PARATR_HYPHENZONE,
                false)
               )
            {
                pColl->SetAttr(SvxHyphenZoneItem(true));
            }

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
    USHORT* pNo = bFirst ? &nAktFirstPageDesc : &nAktPageDesc;
    USHORT nNew = pDoc->MakePageDesc( ViewShell::GetShellRes()->
                    GetPageDescName( pDoc->GetPageDescCnt(), bFirst ), 0,
                                      FALSE );
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
                rAkt.GetNumType().GetNumberingType() == rOld.GetNumType().GetNumberingType();

    if( bRet )
    {
        // dann ein paar Attribute vergleichen
        USHORT __READONLY_DATA aIdArr[] = { RES_FRM_SIZE, RES_UL_SPACE,
                                            RES_BACKGROUND, RES_SHADOW,
                                            RES_FRAMEDIR, RES_FRAMEDIR,
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

// MM. Test to see if the current Document pointer is
// pointing to a pages desc and if it is return it.
SwFmtPageDesc* SwRTFParser::GetCurrentPageDesc(SwPaM *pPam)
{
    const SfxPoolItem* pItem;
    const SwCntntNode* pNd = pPam->GetCntntNode();
    SwFmtPageDesc* aFmtPageDsc = 0;

    // If the current object is a page desc.
    if( pNd && pNd->GetpSwAttrSet() && SFX_ITEM_SET ==
        pNd->GetpSwAttrSet()->GetItemState( RES_PAGEDESC,
        FALSE, &pItem ) )
    {
        aFmtPageDsc = (SwFmtPageDesc*)pItem;
    }
    return aFmtPageDsc;
}

void SwRTFParser::DoHairyWriterPageDesc(int nToken)
{
    int bWeiter = TRUE;
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

void SwRTFParser::ReadSectControls( int nToken )
{
    //this is some hairy stuff to try and retain writer style page descriptors
    //in rtf, almost certainy a bad idea, but we've inherited it, so here it
    //stays
    if (bInPgDscTbl)
    {
        DoHairyWriterPageDesc(nToken);
        return;
    }

    ASSERT(!maSegments.empty(), "suspicious to have a section with no "
        "page info, though probably legal");
    if (maSegments.empty())
    {
        maSegments.push_back(rtfSection(*pPam->GetPoint(),
            SectPageInformation(maPageDefaults)));
    }

    SectPageInformation aNewSection(maSegments.back().maPageInfo);

    bool bNewSection = false;
    int bWeiter = true;
    do {
        USHORT nValue = USHORT( nTokenValue );
        switch( nToken )
        {
            case RTF_SECT:
                bNewSection = true;
                break;
            case RTF_SECTD:
                //Reset to page defaults
                aNewSection = SectPageInformation(maPageDefaults);
                break;
            case RTF_PGWSXN:
                if (0 < nTokenValue)
                    aNewSection.mnPgwsxn = nTokenValue;
                break;
            case RTF_PGHSXN:
                if (0 < nTokenValue)
                    aNewSection.mnPghsxn = nTokenValue;
                break;
            case RTF_MARGLSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMarglsxn = nTokenValue;
                break;
            case RTF_MARGRSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargrsxn = nTokenValue;
                break;
            case RTF_MARGTSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargtsxn = nTokenValue;
                break;
            case RTF_MARGBSXN:
                if (0 <= nTokenValue)
                    aNewSection.mnMargbsxn = nTokenValue;
                break;
            case RTF_FACPGSXN:
                aNewSection.mbFacpgsxn = true;
                break;
            case RTF_HEADERY:
                aNewSection.mnHeadery = nTokenValue;
                break;
            case RTF_FOOTERY:
                aNewSection.mnFootery = nTokenValue;
                break;
            case RTF_LNDSCPSXN:
                aNewSection.mbLndscpsxn = true;
                break;
            case RTF_PGNSTARTS:
                aNewSection.mnPgnStarts = nTokenValue;
                break;
            case RTF_PGNDEC:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ARABIC);
                break;
            case RTF_PGNUCRM:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ROMAN_UPPER);
                break;
            case RTF_PGNLCRM:
                aNewSection.maNumType.SetNumberingType(SVX_NUM_ROMAN_LOWER);
                break;
            case RTF_PGNUCLTR:
                aNewSection.maNumType.SetNumberingType(
                    SVX_NUM_CHARS_UPPER_LETTER_N);
                break;
            case RTF_PGNLCLTR:
                aNewSection.maNumType.SetNumberingType(
                    SVX_NUM_CHARS_LOWER_LETTER_N);
                break;
            case RTF_SBKNONE:
                aNewSection.mnBkc = 0;
                break;
            case RTF_SBKCOL:
                aNewSection.mnBkc = 1;
                break;
            case RTF_PGBRDRT:
                SetBorderLine(aNewSection.maBox, BOX_LINE_TOP);
                break;

            case RTF_PGBRDRB:
                SetBorderLine(aNewSection.maBox, BOX_LINE_BOTTOM);
                break;

            case RTF_PGBRDRL:
                SetBorderLine(aNewSection.maBox, BOX_LINE_LEFT);
                break;

            case RTF_PGBRDRR:
                SetBorderLine(aNewSection.maBox, BOX_LINE_RIGHT);
                break;

            case RTF_PGBRDROPT:
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
                aNewSection.mbTitlepg = true;
                break;
            case RTF_HEADER:
            case RTF_HEADERL:
            case RTF_HEADERR:
            case RTF_FOOTER:
            case RTF_FOOTERL:
            case RTF_FOOTERR:
                if (!aNewSection.mpPageHdFt)
                {
                    String aName(RTL_CONSTASCII_STRINGPARAM("rtfHdFt"));
                    aName += String::CreateFromInt32(maSegments.size());
                    sal_uInt16 nPageNo = pDoc->MakePageDesc(aName);
                    aNewSection.mpPageHdFt = &pDoc->_GetPageDesc(nPageNo);
                    aNewSection.mbPageHdFtUsed = true;
                    maSegments.maDummyPageNos.push_back(nPageNo);
                }
                ReadHeaderFooter(nToken, aNewSection.mpPageHdFt);
                break;
            case RTF_FOOTERF:
            case RTF_HEADERF:
                if (!aNewSection.mpTitlePageHdFt)
                {
                    String aTitle(RTL_CONSTASCII_STRINGPARAM("rtfTitleHdFt"));
                    aTitle += String::CreateFromInt32(maSegments.size());
                    sal_uInt16 nPageNo = pDoc->MakePageDesc(aTitle);
                    aNewSection.mpTitlePageHdFt = &pDoc->_GetPageDesc(nPageNo);
                    aNewSection.mbTitlePageHdFtUsed = true;
                    maSegments.maDummyPageNos.push_back(nPageNo);
                }
                ReadHeaderFooter(nToken, aNewSection.mpTitlePageHdFt);
                break;
            case RTF_COLS:
                aNewSection.mnCols = nTokenValue;
                break;
            case RTF_COLSX:
                aNewSection.mnColsx = nTokenValue;
                break;
            case RTF_COLNO:
                {
                    long nAktCol = nValue;
                    if (RTF_COLW == GetNextToken())
                    {
                        long nWidth = nTokenValue, nSpace = 0;
                        if( RTF_COLSR == GetNextToken() )
                            nSpace = nTokenValue;
                        else
                            SkipToken( -1 );        // wieder zurueck

                        if (--nAktCol == (aNewSection.maColumns.size() / 2 ))
                        {
                            aNewSection.maColumns.push_back(nWidth);
                            aNewSection.maColumns.push_back(nSpace);
                        }
                    }
                }
                break;
            case RTF_STEXTFLOW:
                aNewSection.mnStextflow = nTokenValue;
                break;
            case RTF_RTLSECT:
                aNewSection.mbRTLsection = true;
                break;
            case RTF_LTRSECT:
                aNewSection.mbRTLsection = false;
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
                        // erstmal komplett ueberlesen
                        SkipGroup();
                        // ueberlese noch die schliessende Klammer
                        GetNextToken();
                    }
                    if (nSkip)
                    {
                        bWeiter = ((-1 == nSkip) &&
                            (
                              RTF_FOOTER == nToken || RTF_HEADER == nToken ||
                              RTF_FOOTERR == nToken || RTF_HEADERR == nToken ||
                              RTF_FOOTERL == nToken || RTF_HEADERL == nToken ||
                              RTF_FOOTERF == nToken || RTF_HEADERF == nToken
                            ));
                        SkipToken (nSkip);      // Ignore wieder zurueck
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
                ASSERT(!this, "why are these tokens found in this section?");
                ReadDocControls( nToken );
                break;
            default:
                if (RTF_DOCFMT == (nToken & ~(0xff | RTF_SWGDEFS)))
                    ReadDocControls( nToken );
                else if (RTF_SECTFMT == (nToken & ~(0xff | RTF_SWGDEFS)) ||
                         RTF_UNKNOWNCONTROL == nToken)
                {
                    SvxRTFParser::NextToken(nToken);
                }
                else
                    bWeiter = false;
                break;
        }

        if (bWeiter)
            nToken = GetNextToken();
    } while (bWeiter && IsParserWorking());

    if (bNewSection || maSegments.empty())
    {
        AttrGroupEnd(); //#106493#
        maSegments.push_back(rtfSection(*pPam->GetPoint(), aNewSection));
    }
    else //modifying/replacing the current section
    {
        SwPaM aPamStart(maSegments.back().maStart);
        maSegments.pop_back();
        maSegments.push_back(rtfSection(*aPamStart.GetPoint(), aNewSection));
    }

    SkipToken(-1);
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
        pColl = pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD, FALSE );
    pDoc->SetTxtFmtColl( *pPam, pColl );

    int nToken, bSaveChkStyleAttr = IsChkStyleAttr();
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt !!

    SetChkStyleAttr(FALSE);     // Attribute nicht gegen die Styles checken

    bInPgDscTbl = true;
    USHORT nPos = 0;
    SwPageDesc* pPg;
    SwFrmFmt* pPgFmt;

    SvxULSpaceItem aUL, aHUL, aFUL;
    SvxLRSpaceItem aLR, aHLR, aFLR;
    SwFmtFrmSize aSz( ATT_FIX_SIZE, 11905, 16837 );     // DIN A4 defaulten
    SwFmtFrmSize aFSz( ATT_MIN_SIZE ), aHSz( ATT_MIN_SIZE );

    SvxFrameDirectionItem aFrmDir(FRMDIR_HORI_LEFT_TOP);

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
                pPgFmt->SetAttr( aFrmDir );
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
            if (nPos)   // kein && wg MAC
            {
                if (nPos != pDoc->MakePageDesc(
                    String::CreateFromInt32(nTokenValue)))
                {
                    ASSERT( FALSE, "PageDesc an falscher Position" );
                }
            }

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

        case RTF_RTLSECT:
            aFrmDir.SetValue(FRMDIR_HORI_RIGHT_TOP);
            break;

        case RTF_LTRSECT:
            aFrmDir.SetValue(FRMDIR_HORI_LEFT_TOP);
            break;

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
                USHORT n = SwStyleNameMapper::GetPoolIdFromUIName( aToken, GET_POOLID_PAGEDESC );
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

    bInPgDscTbl = false;
    nAktPageDesc = 0;
    nAktFirstPageDesc = 0;
    bSwPageDesc = true;
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

static const SwNodeIndex* SetHeader(SwFrmFmt* pHdFtFmt, BOOL bReuseOld)
{
    ASSERT(pHdFtFmt, "Impossible, no header");
    const SwFrmFmt* pExisting = bReuseOld ?
        pHdFtFmt->GetHeader().GetHeaderFmt() : 0;
    if (!pExisting)
    {
        //No existing header, create a new one
        pHdFtFmt->SetAttr(SwFmtHeader(TRUE));
        pExisting = pHdFtFmt->GetHeader().GetHeaderFmt();
    }
    return pExisting->GetCntnt().GetCntntIdx();
}

static const SwNodeIndex* SetFooter(SwFrmFmt* pHdFtFmt, BOOL bReuseOld)
{
    ASSERT(pHdFtFmt, "Impossible, no footer");
    const SwFrmFmt* pExisting = bReuseOld ?
        pHdFtFmt->GetFooter().GetFooterFmt() : 0;
    if (!pExisting)
    {
        //No exist footer, create a new one
        pHdFtFmt->SetAttr(SwFmtFooter(TRUE));
        pExisting = pHdFtFmt->GetFooter().GetFooterFmt();
    }
    return pExisting->GetCntnt().GetCntntIdx();
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
    int bDelFirstChar = FALSE;
    bool bOldIsFootnote = mbIsFootnote;
    BOOL bOldGrpStt = IsNewGroup();

    int nOpenBrakets = GetOpenBrakets() - 1;

    switch( nToken )
    {
    case RTF_FOOTNOTE:
        {
            bool bIsEndNote = RTF_FTNALT == GetNextToken();
            if (!bIsEndNote)
                SkipToken(-1);

            SwTxtNode* pTxtNd = pPam->GetNode()->GetTxtNode();
            SwFmtFtn aFtnNote(bIsEndNote);
            xub_StrLen nPos = pPam->GetPoint()->nContent.GetIndex();

            if (nPos && !bFootnoteAutoNum)
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
            mbIsFootnote = true;

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
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~PD_HEADERSHARE) | PD_ALL));
        SetHeader( pPageDesc->GetRightFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetHeader(pHdFtFmt, FALSE );
        break;

    case RTF_HEADERR:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~PD_HEADERSHARE) | PD_ALL));
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
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~PD_FOOTERSHARE) | PD_ALL));
        SetFooter( pPageDesc->GetRightFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetLeftFmt();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;

    case RTF_FOOTERR:
        // we cannot have left or right, must have always both
        pPageDesc->WriteUseOn( (UseOnPage)((pPageDesc->ReadUseOn() & ~PD_FOOTERSHARE) | PD_ALL));
        SetFooter( pPageDesc->GetLeftFmt(), TRUE );
        pHdFtFmt = pPageDesc->GetRightFmt();
        pSttIdx = SetFooter(pHdFtFmt, FALSE );
        break;
    }

    USHORT nOldFlyArrCnt = aFlyArr.Count();
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

        SwTxtFmtColl* pColl = aTxtCollTbl.Get( 0 );
        if( !pColl )
            pColl = pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD, FALSE );
        pDoc->SetTxtFmtColl( *pPam, pColl );

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
        // is add a new fly ?
        if( nOldFlyArrCnt < aFlyArr.Count() )
        {
            SwFlySave* pFlySave = aFlyArr[ aFlyArr.Count()-1 ];
            pFlySave->aFlySet.ClearItem( RES_ANCHOR );
            pHdFtFmt->SetAttr( pFlySave->aFlySet );
            aFlyArr.DeleteAndDestroy( aFlyArr.Count() - 1 );
        }
        else
        {
            // no, so remove the created textattribute
            SwFrmFmt* pFlyFmt = pTxtAttr->GetFlyCnt().GetFrmFmt();
            // remove the pam from the flynode
            *pPam->GetPoint() = aSavePos;
            pDoc->DelLayoutFmt( pFlyFmt );
        }
    }

    bFootnoteAutoNum = FALSE;       // default auf aus!

    // und alles wieder zurueck
    *pPam->GetPoint() = aSavePos;
    if (mbIsFootnote)
        SetNewGroup( bOldGrpStt );      // Status wieder zurueck
    else
        SetNewGroup( FALSE );           // { - Klammer war kein Group-Start!
    mbIsFootnote = bOldIsFootnote;
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
            pColl = pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD,
                                                    FALSE);
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
                    pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD,
                                                    FALSE ) );

    // sollte es eine Vorlage aus dem Pool sein ??
    USHORT n = SwStyleNameMapper::GetPoolIdFromUIName( aNm, GET_POOLID_TXTCOLL );
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
    USHORT n = SwStyleNameMapper::GetPoolIdFromUIName( aNm, GET_POOLID_CHRFMT );
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

//Takashi Ono for CJK
String SwRTFParser::XlateFmtColName(const String &rName) const
{
#define RES_NONE RES_POOLCOLL_DOC_END

    static const RES_POOL_COLLFMT_TYPE aArr[]={
        RES_POOLCOLL_STANDARD, RES_POOLCOLL_HEADLINE1, RES_POOLCOLL_HEADLINE2,
        RES_POOLCOLL_HEADLINE3, RES_POOLCOLL_HEADLINE4, RES_POOLCOLL_HEADLINE5,
        RES_POOLCOLL_HEADLINE6, RES_POOLCOLL_HEADLINE7, RES_POOLCOLL_HEADLINE8,
        RES_POOLCOLL_HEADLINE9,

        RES_POOLCOLL_TOX_IDX1, RES_POOLCOLL_TOX_IDX2, RES_POOLCOLL_TOX_IDX3,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_TOX_CNTNT1,

        RES_POOLCOLL_TOX_CNTNT2, RES_POOLCOLL_TOX_CNTNT3, RES_POOLCOLL_TOX_CNTNT4,
        RES_POOLCOLL_TOX_CNTNT5, RES_POOLCOLL_TOX_CNTNT6, RES_POOLCOLL_TOX_CNTNT7,
        RES_POOLCOLL_TOX_CNTNT8, RES_POOLCOLL_TOX_CNTNT9,
        RES_NONE, RES_POOLCOLL_FOOTNOTE,

        RES_NONE, RES_POOLCOLL_HEADER, RES_POOLCOLL_FOOTER, RES_POOLCOLL_TOX_IDXH,
        RES_NONE, RES_NONE, RES_POOLCOLL_JAKETADRESS, RES_POOLCOLL_SENDADRESS,
        RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_ENDNOTE, RES_NONE, RES_NONE, RES_NONE,
        RES_POOLCOLL_LISTS_BEGIN, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,

        RES_NONE,RES_NONE, RES_POOLCOLL_DOC_TITEL, RES_NONE, RES_POOLCOLL_SIGNATURE, RES_NONE,
        RES_POOLCOLL_TEXT, RES_POOLCOLL_TEXT_MOVE, RES_NONE, RES_NONE,

        RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_DOC_SUBTITEL };
    static const sal_Char *stiName[] = {
        "Normal",
        "heading 1",
        "heading 2",
        "heading 3",
        "heading 4",
        "heading 5",
        "heading 6",
        "heading 7",
        "heading 8",
        "heading 9",
        "index 1",
        "index 2",
        "index 3",
        "index 4",
        "index 5",
        "index 6",
        "index 7",
        "index 8",
        "index 9",
        "toc 1",
        "toc 2",
        "toc 3",
        "toc 4",
        "toc 5",
        "toc 6",
        "toc 7",
        "toc 8",
        "toc 9",
        "Normal Indent",
        "footnote text",
        "annotation text",
        "header",
        "footer",
        "index heading",
        "caption",
        "table of figures",
        "envelope address",
        "envelope return",
        "footnote reference",
        "annotation reference",
        "line number",
        "page number",
        "endnote reference",
        "endnote text",
        "table of authorities",
        "macro",
        "toa heading",
        "List",
        "List Bullet",
        "List Number",
        "List 2",
        "List 3",
        "List 4",
        "List 5",
        "List Bullet 2",
        "List Bullet 3",
        "List Bullet 4",
        "List Bullet 5",
        "List Number 2",
        "List Number 3",
        "List Number 4",
        "List Number 5",
        "Title",
        "Closing",
        "Signature",
        "Default Paragraph Font",
        "Body Text",
        "Body Text Indent",
        "List Continue",
        "List Continue 2",
        "List Continue 3",
        "List Continue 4",
        "List Continue 5",
        "Message Header",
        "Subtitle",
    };


    ASSERT( ( sizeof( aArr ) / sizeof( RES_POOL_COLLFMT_TYPE ) == 75 ),
            "Style-UEbersetzungstabelle hat falsche Groesse" );
    ASSERT( ( sizeof( stiName ) / sizeof( *stiName ) == 75 ),
            "Style-UEbersetzungstabelle hat falsche Groesse" );

    RES_POOL_COLLFMT_TYPE nId = RES_NONE;
    size_t nSize = sizeof(stiName) / sizeof(*stiName);

    for (size_t i = 0; i < nSize; ++i)
    {
        if ( rName == String( stiName[i], RTL_TEXTENCODING_MS_1252 ) )
        {
            nId = aArr[i];
            break;
        }
    }

    if (i >= nSize)
        return rName;

    USHORT nResId;
    if( RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END )
        nResId = RC_POOLCOLL_TEXT_BEGIN - RES_POOLCOLL_TEXT_BEGIN;
    else if (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END)
        nResId = RC_POOLCOLL_LISTS_BEGIN - RES_POOLCOLL_LISTS_BEGIN;
    else if (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END)
        nResId = RC_POOLCOLL_EXTRA_BEGIN - RES_POOLCOLL_EXTRA_BEGIN;
    else if (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END)
        nResId = RC_POOLCOLL_REGISTER_BEGIN - RES_POOLCOLL_REGISTER_BEGIN;
    else if (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END)
        nResId = RC_POOLCOLL_DOC_BEGIN - RES_POOLCOLL_DOC_BEGIN;
    else if (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END)
        nResId = RC_POOLCOLL_HTML_BEGIN - RES_POOLCOLL_HTML_BEGIN;
    return String( ResId( nResId + nId, pSwResMgr ) );
}

SwTxtFmtColl* SwRTFParser::MakeStyle( USHORT nNo, const SvxRTFStyleType& rStyle )
{
     int bCollExist;
    SwTxtFmtColl* pColl = MakeColl( XlateFmtColName( rStyle.sName ),
                                    USHORT(nNo),
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
                    : pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD,
                                                      FALSE );
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
                    : pDoc->GetTxtCollFromPoolSimple( RES_POOLCOLL_STANDARD,
                                                      FALSE );
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
            String sXE;
            std::stack<bool> minicontexts;
            minicontexts.push(false);
            int nOpenHiddens = 1;
            do {
                switch( nToken = GetNextToken() )
                {
                case '}':
                    minicontexts.pop();
                    if (minicontexts.empty())
                    {
                        // die Klammer wird noch gebraucht!
                        SkipToken( -1 );
                    }
                    break;
                case '{':
                    minicontexts.push(minicontexts.top());
                    break;
                case RTF_V:
                    if( nTokenValue )
                        ++nOpenHiddens;
                    else if (!--nOpenHiddens && minicontexts.size() == 2)
                    {
                        // das wars
                        minicontexts.pop();
                    }
                    break;
                case RTF_U:
                    {
                        String &rWhich = minicontexts.top() ? sXE : sHiddenTxt;
                        if (nTokenValue)
                            rWhich += (sal_Unicode)nTokenValue;
                        else
                            rWhich += aToken;
                    }
                    break;
                case RTF_TEXTTOKEN:
                    {
                        String &rWhich = minicontexts.top() ? sXE : sHiddenTxt;
                        rWhich += aToken;
                    }
                    break;
                case RTF_PLAIN:
                    if (!nOpenHiddens && minicontexts.size() == 2)
                    {
                        minicontexts.pop();
                        SkipToken( -1 );
                    }
                    break;
                case RTF_SUBENTRYINDEX:
                    if (minicontexts.top() == true)
                        sXE += ':';
                    break;
                case RTF_XE:
                    minicontexts.top() = true;
                    break;
                }
            } while (!minicontexts.empty() && IsParserWorking());

            if (sHiddenTxt.Len())
            {
                SwHiddenTxtField aFld( (SwHiddenTxtFieldType*)
                            pDoc->GetSysFldType( RES_HIDDENTXTFLD ),
                            TRUE, aEmptyStr, sHiddenTxt, TRUE );

                pDoc->Insert( *pPam, SwFmtFld( aFld ) );
            }

            if (sXE.Len())
            {
                sXE.Insert('\"', 0);
                sXE.Append('\"');
                sw::ms::ImportXE(*pDoc, *pPam, sXE);
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

BookmarkPosition::BookmarkPosition(const SwPaM &rPaM)
    : maMkNode(rPaM.GetMark()->nNode),
    mnMkCntnt(rPaM.GetMark()->nContent.GetIndex())
{
}

BookmarkPosition::BookmarkPosition(const BookmarkPosition &rEntry)
    : maMkNode(rEntry.maMkNode), mnMkCntnt(rEntry.mnMkCntnt)
{
}

bool BookmarkPosition::operator==(const BookmarkPosition rhs)
{
    return(maMkNode.GetIndex() == rhs.maMkNode.GetIndex() && mnMkCntnt == rhs.mnMkCntnt);
}

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

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
