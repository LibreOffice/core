/*************************************************************************
 *
 *  $RCSfile: ndcopy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define _ZFORLIST_DECLARE_TABLE
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif

#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif


// Struktur fuer das Mappen von alten und neuen Frame-Formaten an den
// Boxen und Lines einer Tabelle

struct _MapTblFrmFmt
{
    const SwFrmFmt *pOld, *pNew;
    _MapTblFrmFmt( const SwFrmFmt *pOldFmt, const SwFrmFmt*pNewFmt )
        : pOld( pOldFmt ), pNew( pNewFmt )
    {}
};

SV_DECL_VARARR( _MapTblFrmFmts, _MapTblFrmFmt, 0, 10 );
SV_IMPL_VARARR( _MapTblFrmFmts, _MapTblFrmFmt );

SwCntntNode* SwTxtNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // the Copy-Textnode is the Node with the Text, the Copy-Attrnode is the
    // node with the collection and hard attributes. Normally ist the same
    // node, but if insert a glossary without formatting, then the Attrnode
    // is the prev node of the destionation position in dest. document.
    SwTxtNode* pCpyTxtNd = (SwTxtNode*)this;
    SwTxtNode* pCpyAttrNd = pCpyTxtNd;

    // kopiere die Formate in das andere Dokument:
    SwTxtFmtColl* pColl = 0;
    if( pDoc->IsInsOnlyTextGlossary() )
    {
        SwNodeIndex aIdx( rIdx, -1 );
        if( aIdx.GetNode().IsTxtNode() )
        {
            pCpyAttrNd = aIdx.GetNode().GetTxtNode();
            pColl = &pCpyAttrNd->GetTxtColl()->GetNextTxtFmtColl();
        }
    }
    if( !pColl )
        pColl = pDoc->CopyTxtColl( *GetTxtColl() );

    SwTxtNode* pTxtNd = pDoc->GetNodes().MakeTxtNode( rIdx, pColl );

    // kopiere Attribute/Text
    if( !pCpyAttrNd->GetpSwAttrSet() )
        // wurde ein AttrSet fuer die Numerierung angelegt, so loesche diesen!
        pTxtNd->ResetAllAttr();

    // if Copy-Textnode unequal to Copy-Attrnode, then copy first
    // the attributes into the new Node.
    if( pCpyAttrNd != pCpyTxtNd )
    {
        pCpyAttrNd->CopyAttr( pTxtNd, 0, 0 );
        if( pCpyAttrNd->GetpSwAttrSet() )
        {
            SwAttrSet aSet( *pCpyAttrNd->GetpSwAttrSet() );
            aSet.ClearItem( RES_PAGEDESC );
            aSet.ClearItem( RES_BREAK );
            aSet.CopyToModify( *pTxtNd );
        }
    }

        // ??? reicht das ??? was ist mit PostIts/Feldern/FeldTypen ???
    pCpyTxtNd->Copy( pTxtNd, SwIndex( pCpyTxtNd ), pCpyTxtNd->GetTxt().Len() );

    if( pCpyAttrNd->GetNum() )
        pTxtNd->UpdateNum( *pCpyAttrNd->GetNum() );

//FEATURE::CONDCOLL
    if( RES_CONDTXTFMTCOLL == pColl->Which() )
        pTxtNd->ChkCondColl();
//FEATURE::CONDCOLL

    return pTxtNd;
}


BOOL lcl_SrchNew( const _MapTblFrmFmt& rMap, void * pPara )
{
    if( rMap.pOld != *(const SwFrmFmt**)pPara )
        return TRUE;
    *((const SwFrmFmt**)pPara) = rMap.pNew;
    return FALSE;       // abbrechen, Pointer gefunden
}


struct _CopyTable
{
    SwDoc* pDoc;
    ULONG nOldTblSttIdx;
    _MapTblFrmFmts& rMapArr;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    SwTableNode *pTblNd;
    const SwTable *pOldTable;

    _CopyTable( SwDoc* pDc, _MapTblFrmFmts& rArr, ULONG nOldStt,
                SwTableNode& rTblNd, const SwTable* pOldTbl )
        : pDoc(pDc), pTblNd(&rTblNd), nOldTblSttIdx(nOldStt),
        rMapArr(rArr), pOldTable( pOldTbl ), pInsLine(0), pInsBox(0)
    {}
};

BOOL lcl_CopyTblBox( const SwTableBox*& rpBox, void* pPara );

BOOL lcl_CopyTblLine( const SwTableLine*& rpLine, void* pPara );

BOOL lcl_CopyTblBox( const SwTableBox*& rpBox, void* pPara )
{
    _CopyTable* pCT = (_CopyTable*)pPara;

    SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rpBox->GetFrmFmt();
    pCT->rMapArr.ForEach( lcl_SrchNew, &pBoxFmt );
    if( pBoxFmt == rpBox->GetFrmFmt() ) // ein neues anlegen ??
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pBoxFmt->GetItemState( RES_BOXATR_FORMULA, FALSE,
            &pItem ) && ((SwTblBoxFormula*)pItem)->IsIntrnlName() )
        {
            ((SwTblBoxFormula*)pItem)->PtrToBoxNm( pCT->pOldTable );
        }

        pBoxFmt = pCT->pDoc->MakeTableBoxFmt();
        pBoxFmt->CopyAttrs( *rpBox->GetFrmFmt() );

        if( rpBox->GetSttIdx() )
        {
            SvNumberFormatter* pN = pCT->pDoc->GetNumberFormatter( FALSE );
            if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == pBoxFmt->
                GetItemState( RES_BOXATR_FORMAT, FALSE, &pItem ) )
            {
                ULONG nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                ULONG nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                if( nNewIdx != nOldIdx )
                    pBoxFmt->SetAttr( SwTblBoxNumFormat( nNewIdx ));

            }
        }

        pCT->rMapArr.Insert( _MapTblFrmFmt( rpBox->GetFrmFmt(), pBoxFmt ),
                                pCT->rMapArr.Count() );
    }

    USHORT nLines = rpBox->GetTabLines().Count();
    SwTableBox* pNewBox;
    if( nLines )
        pNewBox = new SwTableBox( pBoxFmt, nLines, pCT->pInsLine );
    else
    {
        SwNodeIndex aNewIdx( *pCT->pTblNd,
                            rpBox->GetSttIdx() - pCT->nOldTblSttIdx );
        ASSERT( aNewIdx.GetNode().IsStartNode(), "Index nicht auf einem StartNode" );
        pNewBox = new SwTableBox( pBoxFmt, aNewIdx, pCT->pInsLine );
    }

    pCT->pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pNewBox,
                    pCT->pInsLine->GetTabBoxes().Count() );

    if( nLines )
    {
        _CopyTable aPara( *pCT );
        aPara.pInsBox = pNewBox;
        ((SwTableBox*)rpBox)->GetTabLines().ForEach( &lcl_CopyTblLine, &aPara );
    }
    else if( pNewBox->IsInHeadline( &pCT->pTblNd->GetTable() ))
        // in der HeadLine sind die Absaetze mit BedingtenVorlage anzupassen
        pNewBox->GetSttNd()->CheckSectionCondColl();
    return TRUE;
}

BOOL lcl_CopyTblLine( const SwTableLine*& rpLine, void* pPara )
{
    _CopyTable* pCT = (_CopyTable*)pPara;
    SwTableLineFmt* pLineFmt = (SwTableLineFmt*)rpLine->GetFrmFmt();
    pCT->rMapArr.ForEach( lcl_SrchNew, &pLineFmt );
    if( pLineFmt == rpLine->GetFrmFmt() )   // ein neues anlegen ??
    {
        pLineFmt = pCT->pDoc->MakeTableLineFmt();
        pLineFmt->CopyAttrs( *rpLine->GetFrmFmt() );
        pCT->rMapArr.Insert( _MapTblFrmFmt( rpLine->GetFrmFmt(), pLineFmt ),
                                pCT->rMapArr.Count());
    }
    SwTableLine* pNewLine = new SwTableLine( pLineFmt,
                            rpLine->GetTabBoxes().Count(), pCT->pInsBox );
    // die neue Zeile in die Tabelle eintragen
    if( pCT->pInsBox )
    {
        pCT->pInsBox->GetTabLines().C40_INSERT( SwTableLine, pNewLine,
                pCT->pInsBox->GetTabLines().Count() );
    }
    else
    {
        pCT->pTblNd->GetTable().GetTabLines().C40_INSERT( SwTableLine, pNewLine,
                pCT->pTblNd->GetTable().GetTabLines().Count() );
    }
    pCT->pInsLine = pNewLine;
    ((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_CopyTblBox, pCT );
    return TRUE;
}

SwTableNode* SwTableNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // in welchen Array steht ich denn Nodes, UndoNodes ??
    SwNodes& rNds = (SwNodes&)GetNodes();

    if( pDoc->IsIdxInTbl( rIdx ) )
        // zur Zeit keine Tabelle in Tabelle kopieren unterstuetzen
        // (sprich: Text + Tabelle + Text )
        return 0;

    {
        // nicht in Fussnoten kopieren !!
/*
!! Mal ohne Frames
        SwCntntNode* pCNd = pDoc->GetNodes()[ rIdx ]->GetCntntNode();
        SwFrm* pFrm;
        if( (pCNd && 0 != ( pFrm = pCNd->GetFrm()))
                ? pFrm->FindFtnFrm()
                : rIdx < pDoc->GetNodes().EndOfInserts &&
                    pDoc->GetNodes()[pDoc->GetNodes().EndOfInserts]->StartOfSection()
                    < rIdx )
*/
        if( rIdx < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            rIdx >= pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;
    }

    // das TableFrmFmt kopieren
    String sTblName( GetTable().GetFrmFmt()->GetName() );
    if( !pDoc->IsCopyIsMove() )
    {
        const SwFrmFmts& rTblFmts = *pDoc->GetTblFrmFmts();
        for( USHORT n = rTblFmts.Count(); n; )
            if( rTblFmts[ --n ]->GetName() == sTblName )
            {
                sTblName = pDoc->GetUniqueTblName();
                break;
            }
    }

    SwFrmFmt* pTblFmt = pDoc->MakeTblFrmFmt( sTblName, pDoc->GetDfltFrmFmt() );
    pTblFmt->CopyAttrs( *GetTable().GetFrmFmt() );
    SwTableNode* pTblNd = new SwTableNode( rIdx );
    SwEndNode* pEndNd = new SwEndNode( rIdx, *pTblNd );
    SwNodeIndex aInsPos( *pEndNd );

    SwTable& rTbl = (SwTable&)pTblNd->GetTable();
    pTblFmt->Add( &rTbl );      // das Frame-Format setzen

    rTbl.SetHeadlineRepeat( GetTable().IsHeadlineRepeat() );
    rTbl.SetTblChgMode( GetTable().GetTblChgMode() );

    SwDDEFieldType* pDDEType = 0;
    if( IS_TYPE( SwDDETable, &GetTable() ))
    {
        // es wird eine DDE-Tabelle kopiert
        // ist im neuen Dokument ueberhaupt der FeldTyp vorhanden ?
        pDDEType = ((SwDDETable&)GetTable()).GetDDEFldType();
        if( pDDEType->IsDeleted() )
            pDoc->InsDeletedFldType( *pDDEType );
        else
            pDDEType = (SwDDEFieldType*)pDoc->InsertFldType( *pDDEType );
        ASSERT( pDDEType, "unbekannter FieldType" );

        // tauschen am Node den Tabellen-Pointer aus
        SwDDETable* pNewTable = new SwDDETable( pTblNd->GetTable(), pDDEType );
        pTblNd->SetNewTable( pNewTable, FALSE );
    }
    // dann kopiere erstmal den Inhalt der Tabelle, die Zuordnung der
    // Boxen/Lines und das anlegen der Frames erfolgt spaeter
    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );  // (wo stehe in denn nun ??)
    rNds._Copy( aRg, aInsPos, FALSE );

    // Sonderbehandlung fuer eine einzelne Box
    if( 1 == GetTable().GetTabSortBoxes().Count() )
    {
        aRg.aStart.Assign( *pTblNd, 1 );
        aRg.aEnd.Assign( *pTblNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRg, SwTableBoxStartNode );
    }

    // loesche alle Frames vom kopierten Bereich, diese werden beim
    // erzeugen des TableFrames angelegt !
    pTblNd->DelFrms();

    _MapTblFrmFmts aMapArr;
    _CopyTable aPara( pDoc, aMapArr, GetIndex(), *pTblNd, &GetTable() );

    ((SwTable&)GetTable()).GetTabLines().ForEach( &lcl_CopyTblLine, &aPara );

    if( pDDEType && pDoc->GetRootFrm() )
        pDDEType->IncRefCnt();

    return pTblNd;
}

void SwTxtNode::CopyCollFmt( SwTxtNode& rDestNd )
{
    // kopiere die Formate in das andere Dokument:

    // Sonderbehandlung fuer PageBreak/PageDesc/ColBrk
    SwDoc* pDestDoc = rDestNd.GetDoc();
    SwAttrSet aPgBrkSet( pDestDoc->GetAttrPool(), aBreakSetRange );
    SwAttrSet* pSet;

    if( 0 != ( pSet = rDestNd.GetpSwAttrSet() ) )
    {
        // Sonderbehandlung fuer unsere Break-Attribute
        const SfxPoolItem* pAttr;
        if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, FALSE, &pAttr ) )
            aPgBrkSet.Put( *pAttr );

        if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, FALSE, &pAttr ) )
            aPgBrkSet.Put( *pAttr );
    }

    rDestNd.ChgFmtColl( pDestDoc->CopyTxtColl( *GetTxtColl() ));
    if( 0 != ( pSet = GetpSwAttrSet() ) )
        pSet->CopyToModify( rDestNd );

    if( aPgBrkSet.Count() )
        rDestNd.SwCntntNode::SetAttr( aPgBrkSet );
}


//  ----- Copy-Methode vom SwDoc ------

    // verhinder das Kopieren in Fly's, die im Bereich verankert sind.
BOOL lcl_ChkFlyFly( SwDoc* pDoc, ULONG nSttNd, ULONG nEndNd,
                        ULONG nInsNd )
{
    const SwFrmFmt* pFmt;
    const SwFmtAnchor* pAnchor;
    const SwPosition* pAPos;
    const SwSpzFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

    for( USHORT n = 0; n < rFrmFmtTbl.Count(); ++n )
    {
        pFmt = rFrmFmtTbl[n];
        pAnchor = &pFmt->GetAnchor();
        if( 0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
            ( FLY_IN_CNTNT == pAnchor->GetAnchorId() ||
              FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ||
              FLY_AT_FLY == pAnchor->GetAnchorId() ||
              FLY_AT_CNTNT == pAnchor->GetAnchorId() ) &&
            nSttNd <= pAPos->nNode.GetIndex() &&
            pAPos->nNode.GetIndex() < nEndNd )
        {
            const SwFmtCntnt& rCntnt = pFmt->GetCntnt();
            SwStartNode* pSNd;
            if( !rCntnt.GetCntntIdx() ||
                0 == ( pSNd = rCntnt.GetCntntIdx()->GetNode().GetStartNode() ))
                continue;

            if( pSNd->GetIndex() < nInsNd &&
                nInsNd < pSNd->EndOfSectionIndex() )
                return TRUE;        // nicht kopieren !!

            if( lcl_ChkFlyFly( pDoc, pSNd->GetIndex(),
                        pSNd->EndOfSectionIndex(), nInsNd ) )
                return TRUE;        // nicht kopieren !!
        }
    }

    return FALSE;
}

void lcl_SetCpyPos( const SwPosition& rOrigPos,
                    const SwPosition& rOrigStt,
                    const SwPosition& rCpyStt,
                    SwPosition& rChgPos )
{
    ULONG nNdOff = rOrigPos.nNode.GetIndex();
    nNdOff -= rOrigStt.nNode.GetIndex();
    xub_StrLen nCntntPos = rOrigPos.nContent.GetIndex();

    if( nNdOff )
        rChgPos.nNode = nNdOff + rCpyStt.nNode.GetIndex();
    else
    {
        // dann nur den Content anpassen
        if( nCntntPos > rOrigStt.nContent.GetIndex() )
            nCntntPos -= rOrigStt.nContent.GetIndex();
        else
            nCntntPos = 0;
        nCntntPos += rCpyStt.nContent.GetIndex();
    }
    rChgPos.nContent.Assign( rChgPos.nNode.GetNode().GetCntntNode(), nCntntPos );
}

void lcl_CopyBookmarks( const SwPaM& rPam, SwPaM& rCpyPam )
{
    const SwDoc* pSrcDoc = rPam.GetDoc();
    SwDoc* pDestDoc =  rCpyPam.GetDoc();
    BOOL bDoesUndo = pDestDoc->DoesUndo();
    pDestDoc->DoUndo( FALSE );

    const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
    SwPosition* pCpyStt = rCpyPam.Start();

    const SwBookmark* pBkmk;
    for( USHORT nCnt = pSrcDoc->GetBookmarks().Count(); nCnt; )
    {
        // liegt auf der Position ??
        if( ( pBkmk = pSrcDoc->GetBookmarks()[ --nCnt ])->GetPos() < rStt
            || pBkmk->GetPos() >= rEnd )
            continue;

        int bHasOtherPos = 0 != pBkmk->GetOtherPos();
        if( bHasOtherPos && ( *pBkmk->GetOtherPos() < rStt ||
            *pBkmk->GetOtherPos() >= rEnd ) )
            continue;

        SwPaM aTmpPam( *pCpyStt );
        lcl_SetCpyPos( pBkmk->GetPos(), rStt, *pCpyStt, *aTmpPam.GetPoint() );
        if( bHasOtherPos )
        {
            aTmpPam.SetMark();
            lcl_SetCpyPos( *pBkmk->GetOtherPos(), rStt, *pCpyStt,
                            *aTmpPam.GetMark() );
        }

        String sNewNm( pBkmk->GetName() );
        if( !pDestDoc->IsCopyIsMove() &&
            USHRT_MAX != pDestDoc->FindBookmark( sNewNm ) )
            pDestDoc->MakeUniqueBookmarkName( sNewNm );
        pDestDoc->MakeBookmark( aTmpPam, pBkmk->GetKeyCode(), sNewNm,
                                pBkmk->GetShortName() );
    }
    pDestDoc->DoUndo( bDoesUndo );
}

void lcl_DeleteRedlines( const SwPaM& rPam, SwPaM& rCpyPam )
{
    const SwDoc* pSrcDoc = rPam.GetDoc();
    const SwRedlineTbl& rTbl = pSrcDoc->GetRedlineTbl();
    if( rTbl.Count() )
    {
        SwDoc* pDestDoc = rCpyPam.GetDoc();
        SwPosition* pCpyStt = rCpyPam.Start(), *pCpyEnd = rCpyPam.End();
        SwPaM* pDelPam = 0;
        const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
        USHORT n = 0;
        pSrcDoc->GetRedline( *pStt, &n );
        for( ; n < rTbl.Count(); ++n )
        {
            const SwRedline* pRedl = rTbl[ n ];
            if( REDLINE_DELETE == pRedl->GetType() && pRedl->IsVisible() )
            {
                const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

                SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case POS_BEFORE:                // Pos1 liegt vor Pos2
                    break;

                case POS_BEHIND:                // Pos1 liegt hinter Pos2
                    n = rTbl.Count();
                    break;

                default:
                    {
                        pDelPam = new SwPaM( *pCpyStt, pDelPam );
                        if( *pStt < *pRStt )
                            lcl_SetCpyPos( *pRStt, *pStt, *pCpyStt,
                                            *pDelPam->GetPoint() );
                        pDelPam->SetMark();

                        if( *pEnd < *pREnd )
                            *pDelPam->GetPoint() = *pCpyEnd;
                        else
                            lcl_SetCpyPos( *pREnd, *pStt, *pCpyStt,
                                            *pDelPam->GetPoint() );
                    }
                }
            }
        }

        if( pDelPam )
        {
            SwRedlineMode eOld = pDestDoc->GetRedlineMode();
            pDestDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );

            BOOL bDoesUndo = pDestDoc->DoesUndo();
            pDestDoc->DoUndo( FALSE );

            do {
                pDestDoc->DeleteAndJoin( *(SwPaM*)pDelPam->GetNext() );
                if( pDelPam->GetNext() == pDelPam )
                    break;
                delete pDelPam->GetNext();
            } while( TRUE );
            delete pDelPam;

            pDestDoc->DoUndo( bDoesUndo );
            pDestDoc->SetRedlineMode_intern( eOld );
        }
    }
}

void lcl_DeleteRedlines( const SwNodeRange& rRg, SwNodeRange& rCpyRg )
{
    SwDoc* pSrcDoc = rRg.aStart.GetNode().GetDoc();
    if( pSrcDoc->GetRedlineTbl().Count() )
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPaM aCpyTmp( rCpyRg.aStart, rCpyRg.aEnd );
        lcl_DeleteRedlines( aRgTmp, aCpyTmp );
    }
}

// Kopieren eines Bereiches im oder in ein anderes Dokument !

BOOL SwDoc::Copy( SwPaM& rPam, SwPosition& rPos ) const
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
    // kein Copy abfangen.
    if( !rPam.HasMark() || *pStt >= *pEnd )
        return FALSE;

    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();

    // verhinder das Kopieren in Fly's, die im Bereich verankert sind.
    if( pDoc == this )
    {
        // Start-/EndNode noch korrigieren
        ULONG nStt = pStt->nNode.GetIndex(),
                nEnd = pEnd->nNode.GetIndex(),
                nDiff = nEnd - nStt +1;
        SwNode* pNd = GetNodes()[ nStt ];
        if( pNd->IsCntntNode() && pStt->nContent.GetIndex() )
            ++nStt, --nDiff;
        if( (pNd = GetNodes()[ nEnd ])->IsCntntNode() &&
            ((SwCntntNode*)pNd)->Len() != pEnd->nContent.GetIndex() )
            --nEnd, --nDiff;
        if( nDiff &&
            lcl_ChkFlyFly( pDoc, nStt, nEnd, rPos.nNode.GetIndex() ) )
            return FALSE;
    }

    SwPaM* pRedlineRange = 0;
    if( pDoc->IsRedlineOn() ||
        (!pDoc->IsIgnoreRedline() && pDoc->GetRedlineTbl().Count() ) )
        pRedlineRange = new SwPaM( rPos );

    SwRedlineMode eOld = pDoc->GetRedlineMode();

    BOOL bRet = FALSE;

    if( pDoc && pDoc != this )
        bRet = _Copy( rPam, rPos, TRUE, pRedlineRange );    // nur normales Kopieren

    // Copy in sich selbst (ueber mehrere Nodes wird hier gesondert
    // behandelt; in einem TextNode wird normal behandelt)
    else if( ! ( *pStt <= rPos && rPos < *pEnd &&
            ( pStt->nNode != pEnd->nNode ||
              !pStt->nNode.GetNode().IsTxtNode() )) )
        bRet = _Copy( rPam, rPos, TRUE, pRedlineRange );    // nur normales Kopieren

    else
    {
        ASSERT( this == pDoc, " falscher Copy-Zweig!" );
        pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );

        BOOL bDoUndo = pDoc->DoesUndo();
        pDoc->DoUndo( FALSE );  // Auf jedenfall Undo abschalten
        // dann kopiere den Bereich im unteren DokumentBereich,
        // (mit Start/End-Nodes geklammert) und verschiebe diese
        // dann an die gewuenschte Stelle.

        SwUndoCpyDoc* pUndo;
        SwPaM aPam( rPos );         // UndoBereich sichern
        if( bDoUndo )
        {
            pDoc->ClearRedo();
            pUndo = new SwUndoCpyDoc( aPam );
        }

        SwStartNode* pSttNd = pDoc->GetNodes().MakeEmptySection(
                                SwNodeIndex( GetNodes().GetEndOfAutotext() ));
        aPam.GetPoint()->nNode = *pSttNd->EndOfSectionNode();
        pDoc->_Copy( rPam, *aPam.GetPoint(), FALSE );       // kopieren ohne Frames

        aPam.GetPoint()->nNode = pDoc->GetNodes().GetEndOfAutotext();
        aPam.SetMark();
        SwCntntNode* pNode = pDoc->GetNodes().GoPrevious( &aPam.GetMark()->nNode );
        pNode->MakeEndIndex( &aPam.GetMark()->nContent );

        aPam.GetPoint()->nNode = *aPam.GetNode()->StartOfSectionNode();
        pNode = pDoc->GetNodes().GoNext( &aPam.GetPoint()->nNode );
        pNode->MakeStartIndex( &aPam.GetPoint()->nContent );
        pDoc->Move( aPam, rPos );               // auf gewuenschte Position moven

        pNode = aPam.GetCntntNode();
        *aPam.GetPoint() = rPos;        // Cursor umsetzen fuers Undo !
        aPam.SetMark();                 // auch den Mark umsetzen !!
        aPam.DeleteMark();              // aber keinen Bereich makieren !!
        pDoc->DeleteSection( pNode );           // Bereich wieder loeschen

        // falls Undo eingeschaltet ist, so speicher den eingefuegten Bereich
        pDoc->DoUndo( bDoUndo );
        if( bDoUndo )
        {
            pUndo->SetInsertRange( aPam );
            pDoc->AppendUndo( pUndo );
        }

        if( pRedlineRange )
        {
            pRedlineRange->SetMark();
            *pRedlineRange->GetPoint() = *aPam.GetPoint();
            *pRedlineRange->GetMark() = *aPam.GetMark();
        }

        pDoc->SetModified();
        bRet = TRUE;
    }

    pDoc->SetRedlineMode_intern( eOld );
    if( pRedlineRange )
    {
        if( pDoc->IsRedlineOn() )
            pDoc->AppendRedline( new SwRedline( REDLINE_INSERT, *pRedlineRange ));
        else
            pDoc->SplitRedline( *pRedlineRange );
        delete pRedlineRange;
    }

    return bRet;
}

// Kopieren eines Bereiches im oder in ein anderes Dokument !
// Die Position darf nicht im Bereich liegen !!

BOOL SwDoc::_Copy( SwPaM& rPam, SwPosition& rPos,
                    BOOL bMakeNewFrms, SwPaM* pCpyRange ) const
{
    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();

    SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
    // kein Copy abfangen.
    if( !rPam.HasMark() || *pStt >= *pEnd ||
        ( pDoc == this && *pStt <= rPos && rPos < *pEnd ))
        return FALSE;

    BOOL bEndEqualIns = pDoc == this && rPos == *pEnd;

    // falls Undo eingeschaltet, erzeuge das UndoCopy-Objekt
    SwUndoCpyDoc* pUndo;
    SwPaM aCpyPam( rPos );

    SwTblNumFmtMerge aTNFM( *this, *pDoc );

    if( pDoc->DoesUndo() )
    {
        pDoc->ClearRedo();
        pUndo = new SwUndoCpyDoc( aCpyPam );
        pDoc->AppendUndo( pUndo );
    }

    SwRedlineMode eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern( eOld | REDLINE_IGNORE );


    // bewege den Pam von der Insert-Position ein zurueck, dadurch wird
    // die Position nicht "verschoben"
    aCpyPam.SetMark();
    BOOL bCanMoveBack = aCpyPam.Move( fnMoveBackward, fnGoCntnt );
    if( !bCanMoveBack )
        aCpyPam.GetPoint()->nNode--;

    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    SwNodeIndex aInsPos( rPos.nNode );
    BOOL bOneNode = pStt->nNode == pEnd->nNode;
    SwTxtNode* pSttNd = pStt->nNode.GetNode().GetTxtNode();
    SwTxtNode* pEndNd = pEnd->nNode.GetNode().GetTxtNode();
    SwTxtNode* pDestNd = aInsPos.GetNode().GetTxtNode();
    BOOL bCopyCollFmt = !pDoc->IsInsOnlyTextGlossary() && (
                        ( pDestNd && !pDestNd->GetTxt().Len() ) ||
                        ( !bOneNode && !rPos.nContent.GetIndex() ));
    BOOL bCopyBookmarks = TRUE;

    // Block, damit aus diesem gesprungen werden kann !!
    do {
        if( pSttNd )
        {
            // den Anfang nicht komplett kopieren ?
            if( !bCopyCollFmt || pStt->nContent.GetIndex() )
            {
                SwIndex aDestIdx( rPos.nContent );
                BOOL bCopyOk = FALSE;
                if( !pDestNd )
                {
                    if( pStt->nContent.GetIndex() || bOneNode )
                        pDestNd = pDoc->GetNodes().MakeTxtNode( aInsPos,
                            pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD));
                    else
                    {
                        pDestNd = (SwTxtNode*)pSttNd->MakeCopy( pDoc, aInsPos );
                        bCopyOk = TRUE;
                    }
                    aDestIdx.Assign( pDestNd, 0 );
                    bCopyCollFmt = TRUE;
                }
                else if( !bOneNode )
                {
                    BYTE nNumLevel = !pDestNd->GetNum() ? 0
                                            : pDestNd->GetNum()->GetLevel();

                    xub_StrLen nCntntEnd = pEnd->nContent.GetIndex();
                    BOOL bDoesUndo = pDoc->DoesUndo();
                    pDoc->DoUndo( FALSE );
                    pDoc->SplitNode( rPos );
                    pDoc->DoUndo( bDoesUndo );

                    // Nummerierung korrigieren, SplitNode erzeugt immer einen
                    // neuen Level
                    if( NO_NUMLEVEL & nNumLevel )
                        pDestNd->UpdateNum( SwNodeNum( nNumLevel ));

                    if( bCanMoveBack && rPos == *aCpyPam.GetPoint() )
                    {
                        // nach dem SplitNode, den CpyPam wieder richtig aufspannen
                        aCpyPam.Move( fnMoveBackward, fnGoCntnt );
                        aCpyPam.Move( fnMoveBackward, fnGoCntnt );
                    }

                    pDestNd = pDoc->GetNodes()[ aInsPos.GetIndex()-1 ]->GetTxtNode();
                    aDestIdx.Assign( pDestNd, pDestNd->GetTxt().Len() );

                    // korrigiere den Bereich wieder !!
                    if( bEndEqualIns )
                    {
                        BOOL bChg = pEnd != rPam.GetPoint();
                        if( bChg )
                            rPam.Exchange();
                        rPam.Move( fnMoveBackward, fnGoCntnt );
                        if( bChg )
                            rPam.Exchange();

                        aRg.aEnd = pEnd->nNode;
                    }
                    else if( rPos == *pEnd )        // Wurde das Ende auch verschoben
                    {
                        pEnd->nNode--;
                        pEnd->nContent.Assign( pDestNd, nCntntEnd );
                        aRg.aEnd = pEnd->nNode;
                        pEndNd = pEnd->nNode.GetNode().GetTxtNode();
                    }
                }

                if( !bCopyOk )
                {
                    xub_StrLen nCpyLen = ( bOneNode ? pEnd->nContent.GetIndex()
                                            : pSttNd->GetTxt().Len() )
                                    - pStt->nContent.GetIndex();
                    pSttNd->Copy( pDestNd, aDestIdx, pStt->nContent, nCpyLen );
                    if( bEndEqualIns )
                        pEnd->nContent -= nCpyLen;
                }

                if( bOneNode )
                {
                    // ist der DestinationNode leer, kopiere die Vorlage mit
                    if( bCopyCollFmt )
                    {
                        pSttNd->CopyCollFmt( *pDestNd );
                        if( pSttNd->GetNum() )
                            pDestNd->UpdateNum( *pSttNd->GetNum() );
                    }
                    break;
                }
                aRg.aStart++;
            }
        }
        else if( pDestNd )
        {
            if( rPos.nContent.GetIndex() == pDestNd->Len() )
                aInsPos++;
            else if( rPos.nContent.GetIndex() )
            {
                // splitte den TextNode, bei dem Eingefuegt wird.
                BYTE nNumLevel = !pDestNd->GetNum() ? 0
                                        : pDestNd->GetNum()->GetLevel();

                xub_StrLen nCntntEnd = pEnd->nContent.GetIndex();
                BOOL bDoesUndo = pDoc->DoesUndo();
                pDoc->DoUndo( FALSE );
                pDoc->SplitNode( rPos );
                pDoc->DoUndo( bDoesUndo );

                // Nummerierung korrigieren, SplitNode erzeugt immer einen
                // neuen Level
                if( NO_NUMLEVEL & nNumLevel )
                    pDestNd->UpdateNum( SwNodeNum( nNumLevel ));

                if( bCanMoveBack && rPos == *aCpyPam.GetPoint() )
                {
                    // nach dem SplitNode, den CpyPam wieder richtig aufspannen
                    aCpyPam.Move( fnMoveBackward, fnGoCntnt );
                    aCpyPam.Move( fnMoveBackward, fnGoCntnt );
                }

                // korrigiere den Bereich wieder !!
                if( bEndEqualIns )
                    aRg.aEnd--;
                else if( rPos == *pEnd )        // Wurde das Ende auch verschoben
                {
                    rPos.nNode-=2;
                    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                                            nCntntEnd );
                    rPos.nNode++;
                    aRg.aEnd--;
                }
            }
        }

        pDestNd = aInsPos.GetNode().GetTxtNode();
        if( pEndNd )
        {
            SwIndex aDestIdx( rPos.nContent );
            if( !pDestNd )
            {
                pDestNd = pDoc->GetNodes().MakeTxtNode( aInsPos,
                            pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD));
                aDestIdx.Assign( pDestNd, 0  );
                aInsPos--;
            }

            BOOL bEmptyDestNd = 0 == pDestNd->GetTxt().Len();
            pEndNd->Copy( pDestNd, aDestIdx, SwIndex( pEndNd ),
                            pEnd->nContent.GetIndex() );

            // auch alle FormatVorlagen kopieren
            if( bCopyCollFmt && ( bOneNode || bEmptyDestNd ))
            {
                pEndNd->CopyCollFmt( *pDestNd );
                if( pEndNd->GetNum() )
                    pDestNd->UpdateNum( *pEndNd->GetNum() );
            }
        }

        int bHasRange = aRg.aStart != aRg.aEnd;

        SfxItemSet aBrkSet( pDoc->GetAttrPool(), aBreakSetRange );
        if( bHasRange && pSttNd && bCopyCollFmt && pDestNd->GetpSwAttrSet() )
        {
            aBrkSet.Put( *pDestNd->GetpSwAttrSet() );
            if( SFX_ITEM_SET == aBrkSet.GetItemState( RES_BREAK, FALSE ) )
                pDestNd->ResetAttr( RES_BREAK );
            if( SFX_ITEM_SET == aBrkSet.GetItemState( RES_PAGEDESC, FALSE ) )
                pDestNd->ResetAttr( RES_PAGEDESC );
        }

        SwNodeIndex aSavePos( aInsPos, bHasRange ? -1 : 0 );
        if( bHasRange )
        {
            CopyWithFlyInFly( aRg, aInsPos, bMakeNewFrms, FALSE );
            aSavePos++;
            bCopyBookmarks = FALSE;
        }

        // harte Umbrueche wieder in den ersten Node setzen
        if( aBrkSet.Count() && 0 != ( pDestNd = pDoc->GetNodes()[
                aCpyPam.GetPoint()->nNode.GetIndex()+1 ]->GetTxtNode() ) )
        {
            pDestNd->SwCntntNode::SetAttr( aBrkSet );
        }

    } while( FALSE );

    // Position ummelden ( falls verschoben / im anderen Node )
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                            rPos.nContent.GetIndex() );

    if( rPos.nNode != aInsPos )
    {
        aCpyPam.GetMark()->nNode = aInsPos;
        aCpyPam.GetMark()->nContent.Assign( aCpyPam.GetCntntNode(FALSE), 0 );
        rPos = *aCpyPam.GetMark();
    }
    else
        *aCpyPam.GetMark() = rPos;


#ifndef PPC
    aCpyPam.Move( fnMoveForward, bCanMoveBack ? fnGoCntnt : fnGoNode );
#else
    /*
     * All das wegen dem PPC, der leider ein kleines Problem mit Funktions -
     * pointer und tenaeren Ausdruecken hat.
     * Wer diese Zeilen nach MAERZ 94 sieht sage mir das bitte.
     * OK 19.01.94 18:16
     */
    if( bCanMoveBack )
        aCpyPam.Move( fnMoveForward, fnGoCntnt );
    else
        aCpyPam.Move( fnMoveForward, fnGoNode );
#endif


    aCpyPam.Exchange();

    // dann kopiere noch alle Bookmarks
    if( bCopyBookmarks && GetBookmarks().Count() )
        lcl_CopyBookmarks( rPam, aCpyPam );

    if( REDLINE_DELETE_REDLINES & eOld )
        lcl_DeleteRedlines( rPam, aCpyPam );

    // falls Undo eingeschaltet ist, so speicher den eingefuegten Bereich
    if( pDoc->DoesUndo() )
        pUndo->SetInsertRange( aCpyPam, TRUE, 0 != pSttNd );

    if( pCpyRange )
    {
        pCpyRange->SetMark();
        *pCpyRange->GetPoint() = *aCpyPam.GetPoint();
        *pCpyRange->GetMark() = *aCpyPam.GetMark();
    }
    pDoc->SetRedlineMode_intern( eOld );
    pDoc->SetModified();
    return TRUE;
}


//  ----- Copy-Methode vom SwDoc - "kopiere Fly's in Fly's" ------

void SwDoc::CopyWithFlyInFly( const SwNodeRange& rRg,
                            const SwNodeIndex& rInsPos, BOOL bMakeNewFrms,
                            BOOL bDelRedlines, BOOL bCopyFlyAtFly ) const
{
    SwDoc* pDest = rInsPos.GetNode().GetDoc();

    _SaveRedlEndPosForRestore aRedlRest( rInsPos );

    SwNodeIndex aSavePos( rInsPos, -1 );
    GetNodes()._CopyNodes( rRg, rInsPos, bMakeNewFrms, TRUE );
    aSavePos++;

    aRedlRest.Restore();

#ifndef PRODUCT
    {
        //JP 17.06.99: Bug 66973 - check count only if the selection is in
        //              the same (or no) section. Becaus not full selected
        //              section are not copied.
        const SwSectionNode* pSSectNd = rRg.aStart.GetNode().FindSectionNode();
        SwNodeIndex aTmpI( rRg.aEnd, -1 );
        const SwSectionNode* pESectNd = aTmpI.GetNode().FindSectionNode();
        if( pSSectNd == pESectNd &&
            !rRg.aStart.GetNode().IsSectionNode() &&
            !aTmpI.GetNode().IsEndNode() )
        {
            ASSERT( ( rInsPos.GetIndex() - aSavePos.GetIndex() ) ==
                    rRg.aEnd.GetIndex() - rRg.aStart.GetIndex(),
                    "Es wurden zu wenig Nodes kopiert!" )
        }
    }
#endif

    // Undo abschalten
    BOOL bUndo = pDest->DoesUndo();
    pDest->DoUndo( FALSE );
    _CopyFlyInFly( rRg, aSavePos, bCopyFlyAtFly );
    pDest->DoUndo( bUndo );

    SwNodeRange aCpyRange( aSavePos, rInsPos );

    // dann kopiere noch alle Bookmarks
    if( GetBookmarks().Count() )
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPaM aCpyTmp( aCpyRange.aStart, aCpyRange.aEnd );

        lcl_CopyBookmarks( aRgTmp, aCpyTmp );
    }

    if( bDelRedlines && ( REDLINE_DELETE_REDLINES & pDest->GetRedlineMode() ))
        lcl_DeleteRedlines( rRg, aCpyRange );

    pDest->GetNodes()._DelDummyNodes( aCpyRange );
}

void lcl_ChainFmts( SwFlyFrmFmt *pSrc, SwFlyFrmFmt *pDest )
{
    SwFmtChain aSrc( pSrc->GetChain() );
    if ( !aSrc.GetNext() )
    {
        aSrc.SetNext( pDest );
        pSrc->SetAttr( aSrc );
    }
    SwFmtChain aDest( pDest->GetChain() );
    if ( !aDest.GetPrev() )
    {
        aDest.SetPrev( pSrc );
        pDest->SetAttr( aDest );
    }
}

void SwDoc::_CopyFlyInFly( const SwNodeRange& rRg, const SwNodeIndex& rSttIdx,
                            BOOL bCopyFlyAtFly ) const
{
    // Bug 22727: suche erst mal alle Flys zusammen, sortiere sie entsprechend
    //            ihrer Ordnungsnummer und kopiere sie erst dann. Damit wird
    //            die Ordnungsnummer (wird nur im DrawModel verwaltet)
    //            beibehalten.
    SwDoc* pDest = rSttIdx.GetNode().GetDoc();
    _ZSortFlys aArr;
    USHORT nArrLen = GetSpzFrmFmts()->Count();
    for( USHORT n = 0; n < nArrLen; ++n )
    {
        const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[n];
        const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
        const SwPosition* pAPos;
        if ( ( pAnchor->GetAnchorId() == FLY_AT_CNTNT ||
               pAnchor->GetAnchorId() == FLY_AT_FLY ||
               pAnchor->GetAnchorId() == FLY_AUTO_CNTNT ) &&
             0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
             (( bCopyFlyAtFly && FLY_AT_FLY == pAnchor->GetAnchorId() )
                    ? rRg.aStart <= pAPos->nNode.GetIndex() + 1
                    : ( IsRedlineMove()
                            ? rRg.aStart < pAPos->nNode
                            : rRg.aStart <= pAPos->nNode )) &&
             pAPos->nNode < rRg.aEnd )
        {
            aArr.Insert( _ZSortFly( pFmt, pAnchor, nArrLen + aArr.Count() ));
        }
    }

    //Alle kopierten (also die neu erzeugten) Rahmen in ein weiteres Array
    //stopfen. Dort sizten sie passend zu den Originalen, damit hinterher
    //die Chains entsprechend aufgebaut werden koennen.
    SvPtrarr aNewArr( 10, 10 );

    for( n = 0; n < aArr.Count(); ++n )
    {
        // neuen Anker anlegen
        const _ZSortFly& rZSortFly = aArr[ n ];
        SwFmtAnchor aAnchor( *rZSortFly.GetAnchor() );
        SwPosition *pNewPos = (SwPosition*)aAnchor.GetCntntAnchor();
        long nOffset = pNewPos->nNode.GetIndex() -
                            rRg.aStart.GetIndex();
        SwNodeIndex aIdx( rSttIdx, nOffset );
        pNewPos->nNode = aIdx;
        // die am Zeichen Flys wieder ans das vorgegebene Zeichen setzen
        if( FLY_AUTO_CNTNT == aAnchor.GetAnchorId() &&
            aIdx.GetNode().IsTxtNode() )
            pNewPos->nContent.Assign( (SwTxtNode*)&aIdx.GetNode(),
                                        pNewPos->nContent.GetIndex() );
        else
            pNewPos->nContent.Assign( 0, 0 );

        // ueberpruefe Rekursion: Inhalt in "seinen eigenen" Frame
        // kopieren. Dann nicht kopieren
        FASTBOOL bMakeCpy = TRUE;
        if( pDest == this )
        {
            const SwFmtCntnt& rCntnt = rZSortFly.GetFmt()->GetCntnt();
            const SwStartNode* pSNd;
            if( rCntnt.GetCntntIdx() &&
                0 != ( pSNd = rCntnt.GetCntntIdx()->GetNode().GetStartNode() ) &&
                pSNd->GetIndex() < rSttIdx.GetIndex() &&
                rSttIdx.GetIndex() < pSNd->EndOfSectionIndex() )
            {
                bMakeCpy = FALSE;
                aArr.Remove( n, 1 );
                --n;
            }
        }

        // Format kopieren und den neuen Anker setzen
        if( bMakeCpy )
            aNewArr.Insert( pDest->CopyLayoutFmt( *rZSortFly.GetFmt(),
                        aAnchor, FALSE, TRUE/*FALSE*/ ), aNewArr.Count() );
    }

    //Alle chains, die im Original vorhanden sind, soweit wie moeglich wieder
    //aufbauen.
    ASSERT( aArr.Count() == aNewArr.Count(), "Missing new Flys" );
    if ( aArr.Count() == aNewArr.Count() )
    {
        for ( n = 0; n < aArr.Count(); ++n )
        {
            const SwFrmFmt *pFmt = aArr[n].GetFmt();
            const SwFmtChain &rChain = pFmt->GetChain();
            int nCnt = 0 != rChain.GetPrev();
            nCnt += rChain.GetNext() ? 1: 0;
            for ( USHORT k = 0; nCnt && k < aArr.Count(); ++k )
            {
                const _ZSortFly &rTmp = aArr[k];
                const SwFrmFmt *pTmp = rTmp.GetFmt();
                if ( rChain.GetPrev() == pTmp )
                {
                    ::lcl_ChainFmts( (SwFlyFrmFmt*)aNewArr[k],
                                     (SwFlyFrmFmt*)aNewArr[n] );
                    --nCnt;
                }
                else if ( rChain.GetNext() == pTmp )
                {
                    ::lcl_ChainFmts( (SwFlyFrmFmt*)aNewArr[n],
                                     (SwFlyFrmFmt*)aNewArr[k] );
                    --nCnt;
                }
            }
        }
    }
}




