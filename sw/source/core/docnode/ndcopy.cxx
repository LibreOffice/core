/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ndcopy.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:38:30 $
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
#include "precompiled_sw.hxx"


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
#ifdef PRODUCT
#define CHECK_TABLE(t)
#else
#ifdef DEBUG
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif
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

SV_DECL_VARARR( _MapTblFrmFmts, _MapTblFrmFmt, 0, 10 )
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
    if( !pCpyAttrNd->HasSwAttrSet() )
        // wurde ein AttrSet fuer die Numerierung angelegt, so loesche diesen!
        pTxtNd->ResetAllAttr();

    // if Copy-Textnode unequal to Copy-Attrnode, then copy first
    // the attributes into the new Node.
    if( pCpyAttrNd != pCpyTxtNd )
    {
        pCpyAttrNd->CopyAttr( pTxtNd, 0, 0 );
        if( pCpyAttrNd->HasSwAttrSet() )
        {
            SwAttrSet aSet( *pCpyAttrNd->GetpSwAttrSet() );
            aSet.ClearItem( RES_PAGEDESC );
            aSet.ClearItem( RES_BREAK );
            aSet.CopyToModify( *pTxtNd );
        }
    }

        // ??? reicht das ??? was ist mit PostIts/Feldern/FeldTypen ???
    pCpyTxtNd->Copy( pTxtNd, SwIndex( pCpyTxtNd ), pCpyTxtNd->GetTxt().Len() );

    // --> OD 2005-11-01 #i53235#
    // --> OD 2005-11-02 #i51089 - TUNING#
    if ( pCpyAttrNd->GetNum() && pCpyAttrNd->GetNum()->GetNumRule() )
    {
        pCpyAttrNd->CopyNumber(*pTxtNd);
    }
    // <--

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
        : pDoc(pDc), nOldTblSttIdx(nOldStt), rMapArr(rArr),
        pInsLine(0), pInsBox(0), pTblNd(&rTblNd), pOldTable( pOldTbl )
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
        pNewBox->setRowSpan( rpBox->getRowSpan() );
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

    rTbl.SetRowsToRepeat( GetTable().GetRowsToRepeat() );
    rTbl.SetTblChgMode( GetTable().GetTblChgMode() );
    rTbl.SetTableModel( GetTable().IsNewModel() );

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

    // If there is a table in this table, the table format for the outer table
    // does not seem to be used, because the table does not have any contents yet
    // (see IsUsed). Therefore the inner table gets the same name as the outer table.
    // We have to make sure that the table node of the SwTable is accessible, even
    // without any content in aSortCntBoxes. #i26629#
    pTblNd->GetTable().SetTableNode( pTblNd );
    rNds._Copy( aRg, aInsPos, FALSE );
    pTblNd->GetTable().SetTableNode( 0 );

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

    if( pDDEType )
        pDDEType->IncRefCnt();

    CHECK_TABLE( GetTable() );
    return pTblNd;
}

void SwTxtNode::CopyCollFmt( SwTxtNode& rDestNd )
{
    // kopiere die Formate in das andere Dokument:

    // Sonderbehandlung fuer PageBreak/PageDesc/ColBrk
    SwDoc* pDestDoc = rDestNd.GetDoc();
    SwAttrSet aPgBrkSet( pDestDoc->GetAttrPool(), aBreakSetRange );
    const SwAttrSet* pSet;

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

/*
    The lcl_CopyBookmarks function has to copy bookmarks from the source to the destination nodes
    array. It is called after a call of the _CopyNodes(..) function. But this function does not copy
    every node (at least at the moment: 2/08/2006 ), section start and end nodes will not be copied if the corresponding end/start node is outside the copied pam.
    The lcl_NonCopyCount function counts the number of these nodes, given the copied pam and a node
    index inside the pam.
    rPam is the original source pam, rLastIdx is the last calculated position, rDelCount the number
    of "non-copy" nodes between rPam.Start() and rLastIdx.
    nNewIdx is the new position of interest.
*/

void lcl_NonCopyCount( const SwPaM& rPam, SwNodeIndex& rLastIdx, const ULONG nNewIdx, ULONG& rDelCount )
{
    ULONG nStart = rPam.Start()->nNode.GetIndex();
    ULONG nEnd = rPam.End()->nNode.GetIndex();
    if( rLastIdx.GetIndex() < nNewIdx ) // Moving forward?
    {
        do // count "non-copy" nodes
        {
            SwNode& rNode = rLastIdx.GetNode();
            if( ( rNode.IsSectionNode() && rNode.EndOfSectionIndex() >= nEnd )
                || ( rNode.IsEndNode() && rNode.StartOfSectionNode()->GetIndex() < nStart ) )
                ++rDelCount;
            rLastIdx++;
        }
        while( rLastIdx.GetIndex() < nNewIdx );
    }
    else if( rDelCount ) // optimization: if there are no "non-copy" nodes until now,
                         // no move backward needed
    {
        while( rLastIdx.GetIndex() > nNewIdx )
        {
            SwNode& rNode = rLastIdx.GetNode();
            if( ( rNode.IsSectionNode() && rNode.EndOfSectionIndex() >= nEnd )
                || ( rNode.IsEndNode() && rNode.StartOfSectionNode()->GetIndex() < nStart ) )
                --rDelCount;
            rLastIdx--;
        }
    }
}

void lcl_SetCpyPos( const SwPosition& rOrigPos,
                    const SwPosition& rOrigStt,
                    const SwPosition& rCpyStt,
                    SwPosition& rChgPos,
                    ULONG nDelCount )
{
    ULONG nNdOff = rOrigPos.nNode.GetIndex();
    nNdOff -= rOrigStt.nNode.GetIndex();
    nNdOff -= nDelCount;
    xub_StrLen nCntntPos = rOrigPos.nContent.GetIndex();

    if( nNdOff )
        rChgPos.nNode = nNdOff + rCpyStt.nNode.GetIndex();
    else
    {
        // dann nur den Content anpassen
        if( nCntntPos > rOrigStt.nContent.GetIndex() )
            nCntntPos = nCntntPos - rOrigStt.nContent.GetIndex();
        else
            nCntntPos = 0;
        nCntntPos = nCntntPos + rCpyStt.nContent.GetIndex();
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
    // We have to count the "non-copied" nodes..
    ULONG nDelCount = 0;
    SwNodeIndex aCorrIdx( rStt.nNode );
    for( USHORT nCnt = pSrcDoc->getBookmarks().Count(); nCnt; )
    {
        // liegt auf der Position ??
        if( ( pBkmk = pSrcDoc->getBookmarks()[ --nCnt ])->GetBookmarkPos() < rStt
            || pBkmk->GetBookmarkPos() >= rEnd )
            continue;

        int bHasOtherPos = 0 != pBkmk->GetOtherBookmarkPos();
        if( bHasOtherPos && ( *pBkmk->GetOtherBookmarkPos() < rStt ||
            *pBkmk->GetOtherBookmarkPos() >= rEnd ) )
            continue;

        SwPaM aTmpPam( *pCpyStt );

        lcl_NonCopyCount( rPam, aCorrIdx, pBkmk->GetBookmarkPos().nNode.GetIndex(), nDelCount );
        lcl_SetCpyPos( pBkmk->GetBookmarkPos(), rStt, *pCpyStt, *aTmpPam.GetPoint(), nDelCount );
        if( bHasOtherPos )
        {
            aTmpPam.SetMark();
            lcl_NonCopyCount( rPam, aCorrIdx, pBkmk->GetOtherBookmarkPos()->nNode.GetIndex(), nDelCount );
            lcl_SetCpyPos( *pBkmk->GetOtherBookmarkPos(), rStt, *pCpyStt,
                            *aTmpPam.GetMark(), nDelCount );
        }

        String sNewNm( pBkmk->GetName() );
        if( !pDestDoc->IsCopyIsMove() &&
            USHRT_MAX != pDestDoc->findBookmark( sNewNm ) )
            pDestDoc->makeUniqueBookmarkName( sNewNm );
        pDestDoc->makeBookmark( aTmpPam, pBkmk->GetKeyCode(), sNewNm,
                                pBkmk->GetShortName(), pBkmk->GetType() );
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
        // We have to count the "non-copied" nodes
        ULONG nDelCount = 0;
        SwNodeIndex aCorrIdx( pStt->nNode );

        USHORT n = 0;
        pSrcDoc->GetRedline( *pStt, &n );
        for( ; n < rTbl.Count(); ++n )
        {
            const SwRedline* pRedl = rTbl[ n ];
            if( nsRedlineType_t::REDLINE_DELETE == pRedl->GetType() && pRedl->IsVisible() )
            {
                const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

                SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case POS_COLLIDE_END:
                case POS_BEFORE:                // Pos1 liegt vor Pos2
                    break;

                case POS_COLLIDE_START:
                case POS_BEHIND:                // Pos1 liegt hinter Pos2
                    n = rTbl.Count();
                    break;

                default:
                    {
                        pDelPam = new SwPaM( *pCpyStt, pDelPam );
                        if( *pStt < *pRStt )
                        {
                            lcl_NonCopyCount( rPam, aCorrIdx, pRStt->nNode.GetIndex(), nDelCount );
                            lcl_SetCpyPos( *pRStt, *pStt, *pCpyStt,
                                            *pDelPam->GetPoint(), nDelCount );
                        }
                        pDelPam->SetMark();

                        if( *pEnd < *pREnd )
                            *pDelPam->GetPoint() = *pCpyEnd;
                        else
                        {
                            lcl_NonCopyCount( rPam, aCorrIdx, pREnd->nNode.GetIndex(), nDelCount );
                            lcl_SetCpyPos( *pREnd, *pStt, *pCpyStt,
                                            *pDelPam->GetPoint(), nDelCount );
                        }
                    }
                }
            }
        }

        if( pDelPam )
        {
            RedlineMode_t eOld = pDestDoc->GetRedlineMode();
            pDestDoc->SetRedlineMode_intern( (RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

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

bool SwDoc::Copy( SwPaM& rPam, SwPosition& rPos ) const
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
    bool bColumnSel = pDoc->IsClipBoard() && pDoc->IsColumnSelection();

    // kein Copy abfangen.
    if( !rPam.HasMark() || ( *pStt >= *pEnd && !bColumnSel ) )
        return FALSE;

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

    RedlineMode_t eOld = pDoc->GetRedlineMode();

    BOOL bRet = FALSE;

    if( pDoc != this )
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
        pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

        BOOL bDoUndo = pDoc->DoesUndo();
        pDoc->DoUndo( FALSE );  // Auf jedenfall Undo abschalten
        // dann kopiere den Bereich im unteren DokumentBereich,
        // (mit Start/End-Nodes geklammert) und verschiebe diese
        // dann an die gewuenschte Stelle.

        SwUndoCpyDoc* pUndo = 0;
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
        pDoc->Move( aPam, rPos, DOC_MOVEDEFAULT );              // auf gewuenschte Position moven

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
            pDoc->AppendRedline( new SwRedline( nsRedlineType_t::REDLINE_INSERT, *pRedlineRange ), true);
        else
            pDoc->SplitRedline( *pRedlineRange );
        delete pRedlineRange;
    }

    return bRet;
}

// Kopieren eines Bereiches im oder in ein anderes Dokument !
// Die Position darf nicht im Bereich liegen !!

BOOL lcl_MarksWholeNode(const SwPaM & rPam)
{
    BOOL bResult = FALSE;
    const SwPosition * pStt = rPam.Start(), * pEnd = rPam.End();

    if (NULL != pStt && NULL != pEnd)
    {
        SwTxtNode * pSttNd = pStt->nNode.GetNode().GetTxtNode();
        SwTxtNode * pEndNd = pEnd->nNode.GetNode().GetTxtNode();

        if (NULL != pSttNd && NULL != pEndNd &&
            pStt->nContent.GetIndex() == 0 &&
            pEnd->nContent.GetIndex() == pEndNd->Len())
        {
            bResult = TRUE;
        }
    }

    return bResult;
}

BOOL SwDoc::_Copy( SwPaM& rPam, SwPosition& rPos,
                    BOOL bMakeNewFrms, SwPaM* pCpyRange ) const
{
    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
    bool bColumnSel = pDoc->IsClipBoard() && pDoc->IsColumnSelection();

    SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
    // kein Copy abfangen.
    if( !rPam.HasMark() || ( *pStt >= *pEnd && !bColumnSel ) ||
        //JP 29.6.2001: 88963 - dont copy if inspos is in region of start to end
        //JP 15.11.2001: don't test inclusive the end, ever exclusive
        ( pDoc == this && *pStt <= rPos && rPos < *pEnd ))
        return FALSE;

    BOOL bEndEqualIns = pDoc == this && rPos == *pEnd;

    // falls Undo eingeschaltet, erzeuge das UndoCopy-Objekt
    SwUndoCpyDoc* pUndo = 0;
    SwPaM aCpyPam( rPos );

    SwTblNumFmtMerge aTNFM( *this, *pDoc );

    if( pDoc->DoesUndo() )
    {
        pDoc->ClearRedo();
        pUndo = new SwUndoCpyDoc( aCpyPam );
        pDoc->AppendUndo( pUndo );
    }

    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));


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
    BOOL bStartIsTxtNode = 0 != pSttNd;

    const SwNumRule * pNumRuleToPropagate =
        pDoc->SearchNumRule(rPos, FALSE, FALSE, TRUE, 0);

    // Block, damit aus diesem gesprungen werden kann !!
    do {
        if( pSttNd )
        {
            // den Anfang nicht komplett kopieren ?
            if( !bCopyCollFmt || bColumnSel || pStt->nContent.GetIndex() )
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
                else if( !bOneNode || bColumnSel )
                {
                    BYTE nNumLevel = static_cast<BYTE>(pDestNd->GetLevel());

                    xub_StrLen nCntntEnd = pEnd->nContent.GetIndex();
                    BOOL bDoesUndo = pDoc->DoesUndo();
                    pDoc->DoUndo( FALSE );
                    pDoc->SplitNode( rPos, false );
                    pDoc->DoUndo( bDoesUndo );

                    // Nummerierung korrigieren, SplitNode erzeugt immer einen
                    // neuen Level
                    pDestNd->SetLevel(nNumLevel);

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
                        pEndNd = pEnd->nNode.GetNode().GetTxtNode();
                    }
                    else if( rPos == *pEnd )        // Wurde das Ende auch verschoben
                    {
                        pEnd->nNode--;
                        pEnd->nContent.Assign( pDestNd, nCntntEnd );
                        aRg.aEnd = pEnd->nNode;
                        pEndNd = pEnd->nNode.GetNode().GetTxtNode();
                    }
                }

                const SfxPoolItem * pItem = NULL;
                const SfxItemSet * pAttrSet = pDestNd->GetpSwAttrSet();

                /* #107213#: Safe numrule item at destination. */
                int aState = SFX_ITEM_UNKNOWN;
                SwNumRuleItem aNumRuleItem;

                if (pAttrSet != NULL)
                {
                    aState = pAttrSet->GetItemState
                        (RES_PARATR_NUMRULE, FALSE, &pItem);

                    if (SFX_ITEM_SET == aState)
                        aNumRuleItem = *((SwNumRuleItem *) pItem);
                }
                /* #107213# */

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
                        pSttNd->CopyNumber(*pDestNd);


                                       /* #107213# If only a part of one paragraph is copied
                           restore the numrule at the destination. */
                        if (! lcl_MarksWholeNode(rPam))
                        {
                            if (SFX_ITEM_SET == aState)
                                pDestNd->SwCntntNode::SetAttr(aNumRuleItem);
                            else
                                pDestNd->ResetAttr(RES_PARATR_NUMRULE);
                        }
                    }

                    break;
                }

                aRg.aStart++;
            }
        }
        else if( pDestNd )
        {
            // Problems with insertion of table selections into "normal" text solved.
            // We have to set the correct PaM for Undo, if this PaM starts in a textnode,
            // the undo operation will try to merge this node after removing the table.
            // If we didn't split a textnode, the PaM should start at the inserted table node
            if( rPos.nContent.GetIndex() == pDestNd->Len() )
            {    // Insertion at the last position of a textnode (empty or not)
                aInsPos++; // The table will be inserted behind the text node
            }
            else if( rPos.nContent.GetIndex() )
            {   // Insertion in the middle of a text node, it has to be split
                // (and joined from undo)
                bStartIsTxtNode = TRUE;
                // splitte den TextNode, bei dem Eingefuegt wird.
                BYTE nNumLevel = static_cast<BYTE>(pDestNd->GetLevel());

                xub_StrLen nCntntEnd = pEnd->nContent.GetIndex();
                BOOL bDoesUndo = pDoc->DoesUndo();
                pDoc->DoUndo( FALSE );
                pDoc->SplitNode( rPos, false );
                pDoc->DoUndo( bDoesUndo );

                // Nummerierung korrigieren, SplitNode erzeugt immer einen
                // neuen Level
                pDestNd->SetLevel(nNumLevel);

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
            else if( bCanMoveBack )
            {   //Insertion at the first position of a text node. It will not be splitted, the table
                // will be inserted before the text node.
                // See below, before the SetInsertRange funciton of the undo object will be called,
                // the CpyPam would be moved to the next content position. This has to be avoided
                // We want to be moved to the table node itself thus we have to set bCanMoveBack
                // and to manipulate aCpyPam.
                bCanMoveBack = false;
                aCpyPam.GetPoint()->nNode--;
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

                // #112756# #98130# if we have to insert an extra text node
                // at the destination, this node will be our new destination
                // (text) node, and thus we set bStartisTxtNode to true. This
                // will ensure that this node will be deleted during Undo
                // using JoinNext.
                DBG_ASSERT( !bStartIsTxtNode, "Oops, undo may be instable now." );
                bStartIsTxtNode = TRUE;
            }

            const SfxPoolItem * pItem = NULL;
            const SfxItemSet* pAttrSet = pDestNd->GetpSwAttrSet();

            /* #107213# Save numrule at destination */
            int aState = SFX_ITEM_UNKNOWN;
            SwNumRuleItem aNumRuleItem;

            if (pAttrSet != NULL)
            {
                aState = pAttrSet->GetItemState
                    (RES_PARATR_NUMRULE, FALSE, &pItem);

                if (SFX_ITEM_SET == aState)
                    aNumRuleItem = *((SwNumRuleItem *) pItem);
            }
            /* #107213# */

            BOOL bEmptyDestNd = 0 == pDestNd->GetTxt().Len();
            pEndNd->Copy( pDestNd, aDestIdx, SwIndex( pEndNd ),
                            pEnd->nContent.GetIndex() );

            // auch alle FormatVorlagen kopieren
            if( bCopyCollFmt && ( bOneNode || bEmptyDestNd ))
            {
                pEndNd->CopyCollFmt( *pDestNd );

                pEndNd->CopyNumber(*pDestNd);

                if (bOneNode)
                {
                    /* #107213# If only a part of one paragraph is copied
                       restore the numrule at the destination. */
                    if ( ! lcl_MarksWholeNode(rPam))
                    {
                        if (SFX_ITEM_SET == aState)
                            pDestNd->SwCntntNode::SetAttr(aNumRuleItem);
                        else
                            pDestNd->ResetAttr(RES_PARATR_NUMRULE);
                    }
                }
            }
        }

        if( aRg.aStart != aRg.aEnd )
        {
            SfxItemSet aBrkSet( pDoc->GetAttrPool(), aBreakSetRange );
            if( pSttNd && bCopyCollFmt && pDestNd->HasSwAttrSet() )
            {
                aBrkSet.Put( *pDestNd->GetpSwAttrSet() );
                if( SFX_ITEM_SET == aBrkSet.GetItemState( RES_BREAK, FALSE ) )
                    pDestNd->ResetAttr( RES_BREAK );
                if( SFX_ITEM_SET == aBrkSet.GetItemState( RES_PAGEDESC, FALSE ) )
                    pDestNd->ResetAttr( RES_PAGEDESC );
            }

            if( aInsPos == pEnd->nNode )
            {
                SwNodeIndex aSaveIdx( aInsPos, -1 );
                CopyWithFlyInFly( aRg, aInsPos, bMakeNewFrms, FALSE );
                aSaveIdx++;
                pEnd->nNode = aSaveIdx;
                pEnd->nContent.Assign( aSaveIdx.GetNode().GetTxtNode(), 0 );
            }
            else
                CopyWithFlyInFly( aRg, aInsPos, bMakeNewFrms, FALSE );

            bCopyBookmarks = FALSE;

            // harte Umbrueche wieder in den ersten Node setzen
            if( aBrkSet.Count() && 0 != ( pDestNd = pDoc->GetNodes()[
                    aCpyPam.GetPoint()->nNode.GetIndex()+1 ]->GetTxtNode() ) )
            {
                pDestNd->SwCntntNode::SetAttr( aBrkSet );
            }
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

    aCpyPam.Move( fnMoveForward, bCanMoveBack ? fnGoCntnt : fnGoNode );
    aCpyPam.Exchange();

    // dann kopiere noch alle Bookmarks
    if( bCopyBookmarks && getBookmarks().Count() )
        lcl_CopyBookmarks( rPam, aCpyPam );

    if( nsRedlineMode_t::REDLINE_DELETE_REDLINES & eOld )
        lcl_DeleteRedlines( rPam, aCpyPam );

    // falls Undo eingeschaltet ist, so speicher den eingefuegten Bereich
    if( pDoc->DoesUndo() )
        pUndo->SetInsertRange( aCpyPam, TRUE, bStartIsTxtNode );

    if( pCpyRange )
    {
        pCpyRange->SetMark();
        *pCpyRange->GetPoint() = *aCpyPam.GetPoint();
        *pCpyRange->GetMark() = *aCpyPam.GetMark();
    }

    if (pNumRuleToPropagate)
        pDoc->ReplaceNumRule(aCpyPam, *pNumRuleToPropagate);

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

    _SaveRedlEndPosForRestore aRedlRest( rInsPos, 0 );

    SwNodeIndex aSavePos( rInsPos, -1 );
    BOOL bEndIsEqualEndPos = rInsPos == rRg.aEnd;
    GetNodes()._CopyNodes( rRg, rInsPos, bMakeNewFrms, TRUE );
    aSavePos++;
    if( bEndIsEqualEndPos )
        ((SwNodeIndex&)rRg.aEnd) = aSavePos;

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
            ASSERT( rInsPos.GetIndex() - aSavePos.GetIndex() ==
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
    if( getBookmarks().Count() )
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPaM aCpyTmp( aCpyRange.aStart, aCpyRange.aEnd );

        lcl_CopyBookmarks( aRgTmp, aCpyTmp );
    }

    if( bDelRedlines && ( nsRedlineMode_t::REDLINE_DELETE_REDLINES & pDest->GetRedlineMode() ))
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
    USHORT n;

    for( n = 0; n < nArrLen; ++n )
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
        const _ZSortFly& rZSortFly = aArr[ n ];

        // --> OD 2006-01-04 #i59964#
        // correct determination of new anchor position
        SwFmtAnchor aAnchor( *rZSortFly.GetAnchor() );
        SwPosition* pNewPos = (SwPosition*)aAnchor.GetCntntAnchor();
        // for at-paragraph and at-character anchored objects the new anchor
        // position can *not* be determined by the difference of the current
        // anchor position to the start of the copied range, because not
        // complete selected sections in the copied range aren't copied - see
        // method <SwNodes::_CopyNodes(..)>.
        // Thus, the new anchor position in the destination document is found
        // by counting the text nodes.
        if ( aAnchor.GetAnchorId() == FLY_AT_CNTNT ||
             aAnchor.GetAnchorId() == FLY_AUTO_CNTNT )
        {
            // First, determine number of anchor text node in the copied range.
            // Note: The anchor text node *have* to be inside the copied range.
            ULONG nAnchorTxtNdNumInRange( 0L );
            bool bAnchorTxtNdFound( false );
            SwNodeIndex aIdx( rRg.aStart );
            while ( !bAnchorTxtNdFound && aIdx <= rRg.aEnd )
            {
                if ( aIdx.GetNode().IsTxtNode() )
                {
                    ++nAnchorTxtNdNumInRange;
                    bAnchorTxtNdFound = aAnchor.GetCntntAnchor()->nNode == aIdx;
                }

                ++aIdx;
            }
            if ( !bAnchorTxtNdFound )
            {
                // This case can *not* happen, but to be robust take the first
                // text node in the destination document.
                ASSERT( false,
                        "<SwDoc::_CopyFlyInFly(..)> - anchor text node in copied range not found" );
                nAnchorTxtNdNumInRange = 1;
            }
            // Second, search corresponding text node in destination document
            // by counting forward from start insert position <rSttIdx> the
            // determined number of text nodes.
            aIdx = rSttIdx;
            SwNodeIndex aAnchorNdIdx( rSttIdx );
            const SwNode& aEndOfContentNd =
                                    aIdx.GetNode().GetNodes().GetEndOfContent();
            while ( nAnchorTxtNdNumInRange > 0 &&
                    &(aIdx.GetNode()) != &aEndOfContentNd )
            {
                if ( aIdx.GetNode().IsTxtNode() )
                {
                    --nAnchorTxtNdNumInRange;
                    aAnchorNdIdx = aIdx;
                }

                ++aIdx;
            }
            if ( !aAnchorNdIdx.GetNode().IsTxtNode() )
            {
                // This case can *not* happen, but to be robust take the first
                // text node in the destination document.
                ASSERT( false,
                        "<SwDoc::_CopyFlyInFly(..)> - found anchor node index isn't a text node" );
                aAnchorNdIdx = rSttIdx;
                while ( !aAnchorNdIdx.GetNode().IsTxtNode() )
                {
                    ++aAnchorNdIdx;
                }
            }
            // apply found anchor text node as new anchor position
            pNewPos->nNode = aAnchorNdIdx;
        }
        else
        {
            long nOffset = pNewPos->nNode.GetIndex() - rRg.aStart.GetIndex();
            SwNodeIndex aIdx( rSttIdx, nOffset );
            pNewPos->nNode = aIdx;
        }
        // <--
        // die am Zeichen Flys wieder ans das vorgegebene Zeichen setzen
        if ( FLY_AUTO_CNTNT == aAnchor.GetAnchorId() &&
             pNewPos->nNode.GetNode().IsTxtNode() )
        {
            pNewPos->nContent.Assign( (SwTxtNode*)&pNewPos->nNode.GetNode(),
                                        pNewPos->nContent.GetIndex() );
        }
        else
        {
            pNewPos->nContent.Assign( 0, 0 );
        }

        // ueberpruefe Rekursion: Inhalt in "seinen eigenen" Frame
        // kopieren. Dann nicht kopieren
        BOOL bMakeCpy = TRUE;
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
                        aAnchor, false, true ), aNewArr.Count() );
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




