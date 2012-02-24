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


#include <editeng/brkitem.hxx>

#include <hintids.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <fldbas.hxx>
#include <swtable.hxx>
#include <ddefld.hxx>
#include <undobj.hxx>
#include <IMark.hxx>
#include <mvsave.hxx>
#include <cellatr.hxx>
#include <swtblfmt.hxx>
#include <swddetbl.hxx>
#include <docary.hxx>
#include <fmtcnct.hxx>
#include <redline.hxx>
#include <paratr.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <SwNodeNum.hxx>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

namespace
{
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

    static void lcl_NonCopyCount( const SwPaM& rPam, SwNodeIndex& rLastIdx, const sal_uLong nNewIdx, sal_uLong& rDelCount )
    {
        sal_uLong nStart = rPam.Start()->nNode.GetIndex();
        sal_uLong nEnd = rPam.End()->nNode.GetIndex();
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

    static void lcl_SetCpyPos( const SwPosition& rOrigPos,
                        const SwPosition& rOrigStt,
                        const SwPosition& rCpyStt,
                        SwPosition& rChgPos,
                        sal_uLong nDelCount )
    {
        sal_uLong nNdOff = rOrigPos.nNode.GetIndex();
        nNdOff -= rOrigStt.nNode.GetIndex();
        nNdOff -= nDelCount;
        xub_StrLen nCntntPos = rOrigPos.nContent.GetIndex();

        // Always adjust <nNode> at to be changed <SwPosition> instance <rChgPos>
        rChgPos.nNode = nNdOff + rCpyStt.nNode.GetIndex();
        if( !nNdOff )
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

    // TODO: use SaveBookmark (from _DelBookmarks)
    static void lcl_CopyBookmarks(const SwPaM& rPam, SwPaM& rCpyPam)
    {
        const SwDoc* pSrcDoc = rPam.GetDoc();
        SwDoc* pDestDoc =  rCpyPam.GetDoc();
        const IDocumentMarkAccess* const pSrcMarkAccess = pSrcDoc->getIDocumentMarkAccess();
        ::sw::UndoGuard const undoGuard(pDestDoc->GetIDocumentUndoRedo());

        const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
        SwPosition* pCpyStt = rCpyPam.Start();

        typedef ::std::vector< const ::sw::mark::IMark* > mark_vector_t;
        mark_vector_t vMarksToCopy;
        for(IDocumentMarkAccess::const_iterator_t ppMark = pSrcMarkAccess->getMarksBegin();
            ppMark != pSrcMarkAccess->getMarksEnd();
            ppMark++)
        {
            const ::sw::mark::IMark* const pMark = ppMark->get();
            const SwPosition& rMarkStart = pMark->GetMarkStart();
            const SwPosition& rMarkEnd = pMark->GetMarkEnd();
            // only include marks that are in the range and not touching
            // both start and end
            bool bIsNotOnBoundary = pMark->IsExpanded()
                ? (rMarkStart != rStt || rMarkEnd != rEnd)  // rMarkStart != rMarkEnd
                : (rMarkStart != rStt && rMarkEnd != rEnd); // rMarkStart == rMarkEnd
            if(rMarkStart >= rStt && rMarkEnd <= rEnd && bIsNotOnBoundary)
            {
                vMarksToCopy.push_back(pMark);
            }
        }
        // We have to count the "non-copied" nodes..
        SwNodeIndex aCorrIdx(rStt.nNode);
        sal_uLong nDelCount = 0;
        for(mark_vector_t::const_iterator ppMark = vMarksToCopy.begin();
            ppMark != vMarksToCopy.end();
            ++ppMark)
        {
            const ::sw::mark::IMark* const pMark = *ppMark;
            SwPaM aTmpPam(*pCpyStt);
            lcl_NonCopyCount(rPam, aCorrIdx, pMark->GetMarkPos().nNode.GetIndex(), nDelCount);
            lcl_SetCpyPos( pMark->GetMarkPos(), rStt, *pCpyStt, *aTmpPam.GetPoint(), nDelCount);
            if(pMark->IsExpanded())
            {
                aTmpPam.SetMark();
                lcl_NonCopyCount(rPam, aCorrIdx, pMark->GetOtherMarkPos().nNode.GetIndex(), nDelCount);
                lcl_SetCpyPos(pMark->GetOtherMarkPos(), rStt, *pCpyStt, *aTmpPam.GetMark(), nDelCount);
            }

            ::sw::mark::IMark* const pNewMark = pDestDoc->getIDocumentMarkAccess()->makeMark(
                aTmpPam,
                pMark->GetName(),
                IDocumentMarkAccess::GetType(*pMark));
            // Explicitly try to get exactly the same name as in the source
            // because NavigatorReminders, DdeBookmarks etc. ignore the proposed name
            pDestDoc->getIDocumentMarkAccess()->renameMark(pNewMark, pMark->GetName());

            // copying additional attributes for bookmarks or fieldmarks
            ::sw::mark::IBookmark* const pNewBookmark =
                dynamic_cast< ::sw::mark::IBookmark* const >(pNewMark);
            if(pNewBookmark)
            {
                const ::sw::mark::IBookmark* const pOldBookmark = dynamic_cast< const ::sw::mark::IBookmark* >(pMark);
                pNewBookmark->SetKeyCode(pOldBookmark->GetKeyCode());
                pNewBookmark->SetShortName(pOldBookmark->GetShortName());
            }
            ::sw::mark::IFieldmark* const pNewFieldmark =
                dynamic_cast< ::sw::mark::IFieldmark* const >(pNewMark);
            if(pNewFieldmark)
            {
                const ::sw::mark::IFieldmark* const pOldFieldmark = dynamic_cast< const ::sw::mark::IFieldmark* >(pMark);
                pNewFieldmark->SetFieldname(pOldFieldmark->GetFieldname());
                pNewFieldmark->SetFieldHelptext(pOldFieldmark->GetFieldHelptext());
                ::sw::mark::IFieldmark::parameter_map_t* pNewParams = pNewFieldmark->GetParameters();
                const ::sw::mark::IFieldmark::parameter_map_t* pOldParams = pOldFieldmark->GetParameters();
                ::sw::mark::IFieldmark::parameter_map_t::const_iterator pIt = pOldParams->begin();
                for (; pIt != pOldParams->end(); ++pIt )
                {
                    pNewParams->insert( *pIt );
                }
            }

            ::sfx2::Metadatable const*const pMetadatable(
                    dynamic_cast< ::sfx2::Metadatable const* >(pMark));
            ::sfx2::Metadatable      *const pNewMetadatable(
                    dynamic_cast< ::sfx2::Metadatable      * >(pNewMark));
            if (pMetadatable && pNewMetadatable)
            {
                pNewMetadatable->RegisterAsCopyOf(*pMetadatable);
            }
        }
    }
}

// Struktur fuer das Mappen von alten und neuen Frame-Formaten an den
// Boxen und Lines einer Tabelle

struct _MapTblFrmFmt
{
    const SwFrmFmt *pOld, *pNew;
    _MapTblFrmFmt( const SwFrmFmt *pOldFmt, const SwFrmFmt*pNewFmt )
        : pOld( pOldFmt ), pNew( pNewFmt )
    {}
};

SV_DECL_VARARR( _MapTblFrmFmts, _MapTblFrmFmt, 0 )
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

    // METADATA: register copy
    pTxtNd->RegisterAsCopyOf(*pCpyTxtNd);

    // kopiere Attribute/Text
    if( !pCpyAttrNd->HasSwAttrSet() )
        // wurde ein AttrSet fuer die Numerierung angelegt, so loesche diesen!
        pCpyAttrNd->ResetAllAttr();

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
    // #i96213# - force copy of all attributes
    pCpyTxtNd->CopyText( pTxtNd, SwIndex( pCpyTxtNd ),
        pCpyTxtNd->GetTxt().Len(), true );

//FEATURE::CONDCOLL
    if( RES_CONDTXTFMTCOLL == pColl->Which() )
        pTxtNd->ChkCondColl();
//FEATURE::CONDCOLL

    return pTxtNd;
}


sal_Bool lcl_SrchNew( const _MapTblFrmFmt& rMap, void * pPara )
{
    if( rMap.pOld != *(const SwFrmFmt**)pPara )
        return sal_True;
    *((const SwFrmFmt**)pPara) = rMap.pNew;
    return sal_False;       // abbrechen, Pointer gefunden
}


struct _CopyTable
{
    SwDoc* pDoc;
    sal_uLong nOldTblSttIdx;
    _MapTblFrmFmts& rMapArr;
    SwTableLine* pInsLine;
    SwTableBox* pInsBox;
    SwTableNode *pTblNd;
    const SwTable *pOldTable;

    _CopyTable( SwDoc* pDc, _MapTblFrmFmts& rArr, sal_uLong nOldStt,
                SwTableNode& rTblNd, const SwTable* pOldTbl )
        : pDoc(pDc), nOldTblSttIdx(nOldStt), rMapArr(rArr),
        pInsLine(0), pInsBox(0), pTblNd(&rTblNd), pOldTable( pOldTbl )
    {}
};

sal_Bool lcl_CopyTblBox( const SwTableBox*& rpBox, void* pPara );

sal_Bool lcl_CopyTblLine( const SwTableLine*& rpLine, void* pPara );

sal_Bool lcl_CopyTblBox( const SwTableBox*& rpBox, void* pPara )
{
    _CopyTable* pCT = (_CopyTable*)pPara;

    SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rpBox->GetFrmFmt();
    pCT->rMapArr.ForEach( lcl_SrchNew, &pBoxFmt );
    if( pBoxFmt == rpBox->GetFrmFmt() ) // ein neues anlegen ??
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pBoxFmt->GetItemState( RES_BOXATR_FORMULA, sal_False,
            &pItem ) && ((SwTblBoxFormula*)pItem)->IsIntrnlName() )
        {
            ((SwTblBoxFormula*)pItem)->PtrToBoxNm( pCT->pOldTable );
        }

        pBoxFmt = pCT->pDoc->MakeTableBoxFmt();
        pBoxFmt->CopyAttrs( *rpBox->GetFrmFmt() );

        if( rpBox->GetSttIdx() )
        {
            SvNumberFormatter* pN = pCT->pDoc->GetNumberFormatter( sal_False );
            if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == pBoxFmt->
                GetItemState( RES_BOXATR_FORMAT, sal_False, &pItem ) )
            {
                sal_uLong nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
                sal_uLong nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
                if( nNewIdx != nOldIdx )
                    pBoxFmt->SetFmtAttr( SwTblBoxNumFormat( nNewIdx ));

            }
        }

        pCT->rMapArr.Insert( _MapTblFrmFmt( rpBox->GetFrmFmt(), pBoxFmt ),
                                pCT->rMapArr.Count() );
    }

    sal_uInt16 nLines = rpBox->GetTabLines().Count();
    SwTableBox* pNewBox;
    if( nLines )
        pNewBox = new SwTableBox( pBoxFmt, nLines, pCT->pInsLine );
    else
    {
        SwNodeIndex aNewIdx( *pCT->pTblNd,
                            rpBox->GetSttIdx() - pCT->nOldTblSttIdx );
        OSL_ENSURE( aNewIdx.GetNode().IsStartNode(), "Index nicht auf einem StartNode" );
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
    return sal_True;
}

sal_Bool lcl_CopyTblLine( const SwTableLine*& rpLine, void* pPara )
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
    return sal_True;
}

SwTableNode* SwTableNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // in welchen Array steht ich denn Nodes, UndoNodes ??
    SwNodes& rNds = (SwNodes&)GetNodes();

    {
        if( rIdx < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            rIdx >= pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;
    }

    // das TableFrmFmt kopieren
    String sTblName( GetTable().GetFrmFmt()->GetName() );
    if( !pDoc->IsCopyIsMove() )
    {
        const SwFrmFmts& rTblFmts = *pDoc->GetTblFrmFmts();
        for( sal_uInt16 n = rTblFmts.Count(); n; )
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
    rTbl.RegisterToFormat( *pTblFmt );

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
        OSL_ENSURE( pDDEType, "unbekannter FieldType" );

        // tauschen am Node den Tabellen-Pointer aus
        SwDDETable* pNewTable = new SwDDETable( pTblNd->GetTable(), pDDEType );
        pTblNd->SetNewTable( pNewTable, sal_False );
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
    rNds._Copy( aRg, aInsPos, sal_False );
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
        if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False, &pAttr ) )
            aPgBrkSet.Put( *pAttr );

        if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, sal_False, &pAttr ) )
            aPgBrkSet.Put( *pAttr );
    }

    rDestNd.ChgFmtColl( pDestDoc->CopyTxtColl( *GetTxtColl() ));
    if( 0 != ( pSet = GetpSwAttrSet() ) )
        pSet->CopyToModify( rDestNd );

    if( aPgBrkSet.Count() )
        rDestNd.SetAttr( aPgBrkSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
