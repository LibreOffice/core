/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <editeng/formatbreakitem.hxx>

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
#include <unocrsr.hxx>
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
#include <set>
#include <vector>
#include <boost/foreach.hpp>

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
        if( rLastIdx.GetIndex() < nNewIdx ) 
        {
            do 
            {
                SwNode& rNode = rLastIdx.GetNode();
                if( ( rNode.IsSectionNode() && rNode.EndOfSectionIndex() >= nEnd )
                    || ( rNode.IsEndNode() && rNode.StartOfSectionNode()->GetIndex() < nStart ) )
                    ++rDelCount;
                ++rLastIdx;
            }
            while( rLastIdx.GetIndex() < nNewIdx );
        }
        else if( rDelCount ) 
                             
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
        sal_Int32 nCntntPos = rOrigPos.nContent.GetIndex();

        
        rChgPos.nNode = nNdOff + rCpyStt.nNode.GetIndex();
        if( !nNdOff )
        {
            
            if( nCntntPos > rOrigStt.nContent.GetIndex() )
                nCntntPos -= rOrigStt.nContent.GetIndex();
            else
                nCntntPos = 0;
            nCntntPos += rCpyStt.nContent.GetIndex();
        }
        rChgPos.nContent.Assign( rChgPos.nNode.GetNode().GetCntntNode(), nCntntPos );
    }

    
    static void lcl_CopyBookmarks(
        const SwPaM& rPam,
        SwPaM& rCpyPam )
    {
        const SwDoc* pSrcDoc = rPam.GetDoc();
        SwDoc* pDestDoc =  rCpyPam.GetDoc();
        const IDocumentMarkAccess* const pSrcMarkAccess = pSrcDoc->getIDocumentMarkAccess();
        ::sw::UndoGuard const undoGuard(pDestDoc->GetIDocumentUndoRedo());

        const SwPosition &rStt = *rPam.Start(), &rEnd = *rPam.End();
        SwPosition* pCpyStt = rCpyPam.Start();

        typedef ::std::vector< const ::sw::mark::IMark* > mark_vector_t;
        mark_vector_t vMarksToCopy;
        for ( IDocumentMarkAccess::const_iterator_t ppMark = pSrcMarkAccess->getAllMarksBegin();
              ppMark != pSrcMarkAccess->getAllMarksEnd();
              ++ppMark )
        {
            const ::sw::mark::IMark* const pMark = ppMark->get();

            const SwPosition& rMarkStart = pMark->GetMarkStart();
            const SwPosition& rMarkEnd = pMark->GetMarkEnd();
            
            
            const bool bIsNotOnBoundary =
                pMark->IsExpanded()
                ? (rMarkStart != rStt || rMarkEnd != rEnd)  
                : (rMarkStart != rStt && rMarkEnd != rEnd); 
            if ( rMarkStart >= rStt && rMarkEnd <= rEnd
                 && ( bIsNotOnBoundary
                      || IDocumentMarkAccess::GetType( *pMark ) == IDocumentMarkAccess::ANNOTATIONMARK ) )
            {
                vMarksToCopy.push_back(pMark);
            }
        }
        
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
            
            
            pDestDoc->getIDocumentMarkAccess()->renameMark(pNewMark, pMark->GetName());

            
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



struct _MapTblFrmFmt
{
    const SwFrmFmt *pOld, *pNew;
    _MapTblFrmFmt( const SwFrmFmt *pOldFmt, const SwFrmFmt*pNewFmt )
        : pOld( pOldFmt ), pNew( pNewFmt )
    {}
};

typedef std::vector<_MapTblFrmFmt> _MapTblFrmFmts;

SwCntntNode* SwTxtNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    
    
    
    
    SwTxtNode* pCpyTxtNd = (SwTxtNode*)this;
    SwTxtNode* pCpyAttrNd = pCpyTxtNd;

    
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

    
    pTxtNd->RegisterAsCopyOf(*pCpyTxtNd);

    
    if( !pCpyAttrNd->HasSwAttrSet() )
        
        pTxtNd->ResetAllAttr();

    
    
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

    
    
    pCpyTxtNd->CopyText( pTxtNd, SwIndex( pCpyTxtNd ),
        pCpyTxtNd->GetTxt().getLength(), true );

    if( RES_CONDTXTFMTCOLL == pColl->Which() )
        pTxtNd->ChkCondColl();

    return pTxtNd;
}

static bool lcl_SrchNew( const _MapTblFrmFmt& rMap, const SwFrmFmt** pPara )
{
    if( rMap.pOld != *pPara )
        return true;
    *pPara = rMap.pNew;
    return false;
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

static void lcl_CopyTblLine( const SwTableLine* pLine, _CopyTable* pCT );

static void lcl_CopyTblBox( SwTableBox* pBox, _CopyTable* pCT )
{
    SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)pBox->GetFrmFmt();
    for( _MapTblFrmFmts::const_iterator it = pCT->rMapArr.begin(); it != pCT->rMapArr.end(); ++it )
        if ( !lcl_SrchNew( *it, (const SwFrmFmt**)&pBoxFmt ) )
            break;
    if( pBoxFmt == pBox->GetFrmFmt() ) 
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pBoxFmt->GetItemState( RES_BOXATR_FORMULA, sal_False,
            &pItem ) && ((SwTblBoxFormula*)pItem)->IsIntrnlName() )
        {
            ((SwTblBoxFormula*)pItem)->PtrToBoxNm( pCT->pOldTable );
        }

        pBoxFmt = pCT->pDoc->MakeTableBoxFmt();
        pBoxFmt->CopyAttrs( *pBox->GetFrmFmt() );

        if( pBox->GetSttIdx() )
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

        pCT->rMapArr.push_back( _MapTblFrmFmt( pBox->GetFrmFmt(), pBoxFmt ) );
    }

    sal_uInt16 nLines = pBox->GetTabLines().size();
    SwTableBox* pNewBox;
    if( nLines )
        pNewBox = new SwTableBox( pBoxFmt, nLines, pCT->pInsLine );
    else
    {
        SwNodeIndex aNewIdx( *pCT->pTblNd,
                            pBox->GetSttIdx() - pCT->nOldTblSttIdx );
        OSL_ENSURE( aNewIdx.GetNode().IsStartNode(), "Index is not on the start node" );
        pNewBox = new SwTableBox( pBoxFmt, aNewIdx, pCT->pInsLine );
        pNewBox->setRowSpan( pBox->getRowSpan() );
    }

    pCT->pInsLine->GetTabBoxes().push_back( pNewBox );

    if( nLines )
    {
        _CopyTable aPara( *pCT );
        aPara.pInsBox = pNewBox;
        BOOST_FOREACH( const SwTableLine* pLine, pBox->GetTabLines() )
            lcl_CopyTblLine( pLine, &aPara );
    }
    else if( pNewBox->IsInHeadline( &pCT->pTblNd->GetTable() ))
        
        pNewBox->GetSttNd()->CheckSectionCondColl();
}

static void lcl_CopyTblLine( const SwTableLine* pLine, _CopyTable* pCT )
{
    SwTableLineFmt* pLineFmt = (SwTableLineFmt*)pLine->GetFrmFmt();
    for( _MapTblFrmFmts::const_iterator it = pCT->rMapArr.begin(); it != pCT->rMapArr.end(); ++it )
        if ( !lcl_SrchNew( *it, (const SwFrmFmt**)&pLineFmt ) )
            break;
    if( pLineFmt == pLine->GetFrmFmt() ) 
    {
        pLineFmt = pCT->pDoc->MakeTableLineFmt();
        pLineFmt->CopyAttrs( *pLine->GetFrmFmt() );
        pCT->rMapArr.push_back( _MapTblFrmFmt( pLine->GetFrmFmt(), pLineFmt ) );
    }
    SwTableLine* pNewLine = new SwTableLine( pLineFmt,
                            pLine->GetTabBoxes().size(), pCT->pInsBox );
    
    if( pCT->pInsBox )
    {
        pCT->pInsBox->GetTabLines().push_back( pNewLine );
    }
    else
    {
        pCT->pTblNd->GetTable().GetTabLines().push_back( pNewLine );
    }
    pCT->pInsLine = pNewLine;
    for( SwTableBoxes::iterator it = ((SwTableLine*)pLine)->GetTabBoxes().begin();
             it != ((SwTableLine*)pLine)->GetTabBoxes().end(); ++it)
        lcl_CopyTblBox(*it, pCT );
}

SwTableNode* SwTableNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    
    SwNodes& rNds = (SwNodes&)GetNodes();

    {
        if( rIdx < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
            rIdx >= pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() )
            return 0;
    }

    
    OUString sTblName( GetTable().GetFrmFmt()->GetName() );
    if( !pDoc->IsCopyIsMove() )
    {
        const SwFrmFmts& rTblFmts = *pDoc->GetTblFrmFmts();
        for( sal_uInt16 n = rTblFmts.size(); n; )
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
        
        
        pDDEType = ((SwDDETable&)GetTable()).GetDDEFldType();
        if( pDDEType->IsDeleted() )
            pDoc->InsDeletedFldType( *pDDEType );
        else
            pDDEType = (SwDDEFieldType*)pDoc->InsertFldType( *pDDEType );
        OSL_ENSURE( pDDEType, "unknown FieldType" );

        
        SwDDETable* pNewTable = new SwDDETable( pTblNd->GetTable(), pDDEType );
        pTblNd->SetNewTable( pNewTable, sal_False );
    }
    
    
    SwNodeRange aRg( *this, +1, *EndOfSectionNode() );

    
    
    
    
    
    pTblNd->GetTable().SetTableNode( pTblNd );
    rNds._Copy( aRg, aInsPos, sal_False );
    pTblNd->GetTable().SetTableNode( 0 );

    
    if( 1 == GetTable().GetTabSortBoxes().size() )
    {
        aRg.aStart.Assign( *pTblNd, 1 );
        aRg.aEnd.Assign( *pTblNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRg, SwTableBoxStartNode );
    }

    
    
    pTblNd->DelFrms();

    _MapTblFrmFmts aMapArr;
    _CopyTable aPara( pDoc, aMapArr, GetIndex(), *pTblNd, &GetTable() );

    BOOST_FOREACH(const SwTableLine* pLine, GetTable().GetTabLines() )
        lcl_CopyTblLine( pLine, &aPara );

    if( pDDEType )
        pDDEType->IncRefCnt();

    CHECK_TABLE( GetTable() );
    return pTblNd;
}

void SwTxtNode::CopyCollFmt( SwTxtNode& rDestNd )
{
    
    
    SwDoc* pDestDoc = rDestNd.GetDoc();
    SwAttrSet aPgBrkSet( pDestDoc->GetAttrPool(), aBreakSetRange );
    const SwAttrSet* pSet;

    if( 0 != ( pSet = rDestNd.GetpSwAttrSet() ) )
    {
        
        const SfxPoolItem* pAttr;
        if( SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, false, &pAttr ) )
            aPgBrkSet.Put( *pAttr );

        if( SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, false, &pAttr ) )
            aPgBrkSet.Put( *pAttr );
    }

    rDestNd.ChgFmtColl( pDestDoc->CopyTxtColl( *GetTxtColl() ));
    if( 0 != ( pSet = GetpSwAttrSet() ) )
        pSet->CopyToModify( rDestNd );

    if( aPgBrkSet.Count() )
        rDestNd.SetAttr( aPgBrkSet );
}



static sal_Bool lcl_ChkFlyFly( SwDoc* pDoc, sal_uLong nSttNd, sal_uLong nEndNd,
                        sal_uLong nInsNd )
{
    const SwFrmFmts& rFrmFmtTbl = *pDoc->GetSpzFrmFmts();

    for( sal_uInt16 n = 0; n < rFrmFmtTbl.size(); ++n )
    {
        SwFrmFmt const*const  pFmt = rFrmFmtTbl[n];
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
        if (pAPos &&
            ((FLY_AS_CHAR == pAnchor->GetAnchorId()) ||
             (FLY_AT_CHAR == pAnchor->GetAnchorId()) ||
             (FLY_AT_FLY  == pAnchor->GetAnchorId()) ||
             (FLY_AT_PARA == pAnchor->GetAnchorId())) &&
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
                
                return sal_True;

            if( lcl_ChkFlyFly( pDoc, pSNd->GetIndex(),
                        pSNd->EndOfSectionIndex(), nInsNd ) )
                
                return sal_True;
        }
    }

    return sal_False;
}

static void lcl_DeleteRedlines( const SwPaM& rPam, SwPaM& rCpyPam )
{
    const SwDoc* pSrcDoc = rPam.GetDoc();
    const SwRedlineTbl& rTbl = pSrcDoc->GetRedlineTbl();
    if( !rTbl.empty() )
    {
        SwDoc* pDestDoc = rCpyPam.GetDoc();
        SwPosition* pCpyStt = rCpyPam.Start(), *pCpyEnd = rCpyPam.End();
        SwPaM* pDelPam = 0;
        const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
        
        sal_uLong nDelCount = 0;
        SwNodeIndex aCorrIdx( pStt->nNode );

        sal_uInt16 n = 0;
        pSrcDoc->GetRedline( *pStt, &n );
        for( ; n < rTbl.size(); ++n )
        {
            const SwRangeRedline* pRedl = rTbl[ n ];
            if( nsRedlineType_t::REDLINE_DELETE == pRedl->GetType() && pRedl->IsVisible() )
            {
                const SwPosition *pRStt = pRedl->Start(), *pREnd = pRedl->End();

                SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );
                switch( eCmpPos )
                {
                case POS_COLLIDE_END:
                case POS_BEFORE:
                    
                    break;

                case POS_COLLIDE_START:
                case POS_BEHIND:
                    
                    n = rTbl.size();
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

            ::sw::UndoGuard const undoGuard(pDestDoc->GetIDocumentUndoRedo());

            do {
                pDestDoc->DeleteAndJoin( *(SwPaM*)pDelPam->GetNext() );
                if( pDelPam->GetNext() == pDelPam )
                    break;
                delete pDelPam->GetNext();
            } while( true );
            delete pDelPam;

            pDestDoc->SetRedlineMode_intern( eOld );
        }
    }
}

static void lcl_DeleteRedlines( const SwNodeRange& rRg, SwNodeRange& rCpyRg )
{
    SwDoc* pSrcDoc = rRg.aStart.GetNode().GetDoc();
    if( !pSrcDoc->GetRedlineTbl().empty() )
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPaM aCpyTmp( rCpyRg.aStart, rCpyRg.aEnd );
        lcl_DeleteRedlines( aRgTmp, aCpyTmp );
    }
}


bool
SwDoc::CopyRange( SwPaM& rPam, SwPosition& rPos, const bool bCopyAll ) const
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
    bool bColumnSel = pDoc->IsClipBoard() && pDoc->IsColumnSelection();

    
    if( !rPam.HasMark() || ( *pStt >= *pEnd && !bColumnSel ) )
        return false;

    
    if( pDoc == this )
    {
        
        sal_uLong nStt = pStt->nNode.GetIndex(),
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
        {
            return false;
        }
    }

    SwPaM* pRedlineRange = 0;
    if( pDoc->IsRedlineOn() ||
        (!pDoc->IsIgnoreRedline() && !pDoc->GetRedlineTbl().empty() ) )
        pRedlineRange = new SwPaM( rPos );

    RedlineMode_t eOld = pDoc->GetRedlineMode();

    bool bRet = false;

    if( pDoc != this )
    {   
        bRet = CopyImpl( rPam, rPos, true, bCopyAll, pRedlineRange );
    }
    else if( ! ( *pStt <= rPos && rPos < *pEnd &&
            ( pStt->nNode != pEnd->nNode ||
              !pStt->nNode.GetNode().IsTxtNode() )) )
    {
        
        
        bRet = CopyImpl( rPam, rPos, true, bCopyAll, pRedlineRange );
    }
    else
    {
        
        
        
        OSL_ENSURE( this == pDoc, " invalid copy branch!" );
        OSL_FAIL("mst: i thought this could be dead code;"
                "please tell me what you did to get here!");
        pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

        
        
        

        SwUndoCpyDoc* pUndo = 0;
        
        SwPaM aPam( rPos );
        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pDoc->GetIDocumentUndoRedo().ClearRedo();
            pUndo = new SwUndoCpyDoc( aPam );
        }

        {
            ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
            SwStartNode* pSttNd = pDoc->GetNodes().MakeEmptySection(
                                SwNodeIndex( GetNodes().GetEndOfAutotext() ));
            aPam.GetPoint()->nNode = *pSttNd->EndOfSectionNode();
            
            pDoc->CopyImpl( rPam, *aPam.GetPoint(), false, bCopyAll, 0 );

            aPam.GetPoint()->nNode = pDoc->GetNodes().GetEndOfAutotext();
            aPam.SetMark();
            SwCntntNode* pNode =
                pDoc->GetNodes().GoPrevious( &aPam.GetMark()->nNode );
            pNode->MakeEndIndex( &aPam.GetMark()->nContent );

            aPam.GetPoint()->nNode = *aPam.GetNode()->StartOfSectionNode();
            pNode = pDoc->GetNodes().GoNext( &aPam.GetPoint()->nNode );
            pNode->MakeStartIndex( &aPam.GetPoint()->nContent );
            
            pDoc->MoveRange( aPam, rPos, DOC_MOVEDEFAULT );

            pNode = aPam.GetCntntNode();
            *aPam.GetPoint() = rPos;      
            aPam.SetMark();               
            aPam.DeleteMark();            
            pDoc->DeleteSection( pNode ); 
        }

        
        if (pDoc->GetIDocumentUndoRedo().DoesUndo())
        {
            pUndo->SetInsertRange( aPam );
            pDoc->GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        if( pRedlineRange )
        {
            pRedlineRange->SetMark();
            *pRedlineRange->GetPoint() = *aPam.GetPoint();
            *pRedlineRange->GetMark() = *aPam.GetMark();
        }

        pDoc->SetModified();
        bRet = true;
    }

    pDoc->SetRedlineMode_intern( eOld );
    if( pRedlineRange )
    {
        if( pDoc->IsRedlineOn() )
            pDoc->AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pRedlineRange ), true);
        else
            pDoc->SplitRedline( *pRedlineRange );
        delete pRedlineRange;
    }

    return bRet;
}

static bool lcl_MarksWholeNode(const SwPaM & rPam)
{
    bool bResult = false;
    const SwPosition* pStt = rPam.Start();
    const SwPosition* pEnd = rPam.End();

    if (NULL != pStt && NULL != pEnd)
    {
        const SwTxtNode* pSttNd = pStt->nNode.GetNode().GetTxtNode();
        const SwTxtNode* pEndNd = pEnd->nNode.GetNode().GetTxtNode();

        if (NULL != pSttNd && NULL != pEndNd &&
            pStt->nContent.GetIndex() == 0 &&
            pEnd->nContent.GetIndex() == pEndNd->Len())
        {
            bResult = true;
        }
    }

    return bResult;
}


static bool lcl_ContainsOnlyParagraphsInList( const SwPaM& rPam )
{
    bool bRet = false;

    const SwTxtNode* pTxtNd = rPam.Start()->nNode.GetNode().GetTxtNode();
    const SwTxtNode* pEndTxtNd = rPam.End()->nNode.GetNode().GetTxtNode();
    if ( pTxtNd && pTxtNd->IsInList() &&
         pEndTxtNd && pEndTxtNd->IsInList() )
    {
        bRet = true;
        SwNodeIndex aIdx(rPam.Start()->nNode);

        do
        {
            ++aIdx;
            pTxtNd = aIdx.GetNode().GetTxtNode();

            if ( !pTxtNd || !pTxtNd->IsInList() )
            {
                bRet = false;
                break;
            }
        } while ( pTxtNd && pTxtNd != pEndTxtNd );
    }

    return bRet;
}

bool SwDoc::CopyImpl( SwPaM& rPam, SwPosition& rPos,
        const bool bMakeNewFrms, const bool bCopyAll,
        SwPaM *const pCpyRange ) const
{
    SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
    const bool bColumnSel = pDoc->IsClipBoard() && pDoc->IsColumnSelection();

    SwPosition* pStt = rPam.Start();
    SwPosition* pEnd = rPam.End();

    
    if( !rPam.HasMark() || ( *pStt >= *pEnd && !bColumnSel ) ||
        
        
        ( pDoc == this && *pStt <= rPos && rPos < *pEnd ))
    {
        return false;
    }

    const bool bEndEqualIns = pDoc == this && rPos == *pEnd;

    
    SwUndoCpyDoc* pUndo = 0;
    
    
    ::boost::scoped_ptr<SwUnoCrsr> const pCopyPam(pDoc->CreateUnoCrsr(rPos));

    SwTblNumFmtMerge aTNFM( *this, *pDoc );

    if (pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo = new SwUndoCpyDoc(*pCopyPam);
        pDoc->GetIDocumentUndoRedo().AppendUndo( pUndo );
    }

    RedlineMode_t eOld = pDoc->GetRedlineMode();
    pDoc->SetRedlineMode_intern((RedlineMode_t)(eOld | nsRedlineMode_t::REDLINE_IGNORE));

    
    
    pCopyPam->SetMark();
    bool bCanMoveBack = pCopyPam->Move(fnMoveBackward, fnGoCntnt);
    
    bool bAfterTable = false;
    if ((rPos.nNode.GetIndex() - pCopyPam->GetPoint()->nNode.GetIndex()) > 1)
    {
        
        pCopyPam->GetPoint()->nNode = rPos.nNode;
        pCopyPam->GetPoint()->nContent = rPos.nContent;

        bCanMoveBack = false;
        bAfterTable = true;
    }
    if( !bCanMoveBack )
        pCopyPam->GetPoint()->nNode--;

    SwNodeRange aRg( pStt->nNode, pEnd->nNode );
    SwNodeIndex aInsPos( rPos.nNode );
    const bool bOneNode = pStt->nNode == pEnd->nNode;
    SwTxtNode* pSttTxtNd = pStt->nNode.GetNode().GetTxtNode();
    SwTxtNode* pEndTxtNd = pEnd->nNode.GetNode().GetTxtNode();
    SwTxtNode* pDestTxtNd = aInsPos.GetNode().GetTxtNode();
    bool bCopyCollFmt = !pDoc->IsInsOnlyTextGlossary() &&
                        ( (pDestTxtNd && !pDestTxtNd->GetTxt().getLength()) ||
                          ( !bOneNode && !rPos.nContent.GetIndex() ) );
    bool bCopyBookmarks = true;
    sal_Bool bStartIsTxtNode = 0 != pSttTxtNd;

    
    if (pDoc->IsClipBoard() && GetOutlineNumRule())
    {
        pDoc->SetOutlineNumRule(*GetOutlineNumRule());
    }

    
    
    
    
    
    OUString aListIdToPropagate;
    const SwNumRule* pNumRuleToPropagate =
        pDoc->SearchNumRule( rPos, false, true, false, 0, aListIdToPropagate, true );
    if ( !pNumRuleToPropagate )
    {
        pNumRuleToPropagate =
            pDoc->SearchNumRule( rPos, false, false, false, 0, aListIdToPropagate, true );
    }
    
    
    
    
    if ( pNumRuleToPropagate &&
         pDestTxtNd && !pDestTxtNd->GetTxt().getLength() &&
         !pDestTxtNd->IsInList() &&
         !lcl_ContainsOnlyParagraphsInList( rPam ) )
    {
        pNumRuleToPropagate = 0;
    }

    
    do {
        if( pSttTxtNd )
        {
            
            if( !bCopyCollFmt || bColumnSel || pStt->nContent.GetIndex() )
            {
                SwIndex aDestIdx( rPos.nContent );
                bool bCopyOk = false;
                if( !pDestTxtNd )
                {
                    if( pStt->nContent.GetIndex() || bOneNode )
                        pDestTxtNd = pDoc->GetNodes().MakeTxtNode( aInsPos,
                            pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD));
                    else
                    {
                        pDestTxtNd = static_cast<SwTxtNode*>(pSttTxtNd->MakeCopy( pDoc, aInsPos ));
                        bCopyOk = true;
                    }
                    aDestIdx.Assign( pDestTxtNd, 0 );
                    bCopyCollFmt = true;
                }
                else if( !bOneNode || bColumnSel )
                {
                    const sal_Int32 nCntntEnd = pEnd->nContent.GetIndex();
                    {
                        ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                        pDoc->SplitNode( rPos, false );
                    }

                    if (bCanMoveBack && rPos == *pCopyPam->GetPoint())
                    {
                        
                        pCopyPam->Move( fnMoveBackward, fnGoCntnt );
                        pCopyPam->Move( fnMoveBackward, fnGoCntnt );
                    }

                    pDestTxtNd = pDoc->GetNodes()[ aInsPos.GetIndex()-1 ]->GetTxtNode();
                    aDestIdx.Assign(
                            pDestTxtNd, pDestTxtNd->GetTxt().getLength());

                    
                    if( bEndEqualIns )
                    {
                        bool bChg = pEnd != rPam.GetPoint();
                        if( bChg )
                            rPam.Exchange();
                        rPam.Move( fnMoveBackward, fnGoCntnt );
                        if( bChg )
                            rPam.Exchange();

                        aRg.aEnd = pEnd->nNode;
                        pEndTxtNd = pEnd->nNode.GetNode().GetTxtNode();
                    }
                    else if( rPos == *pEnd )
                    {
                        
                        pEnd->nNode--;
                        pEnd->nContent.Assign( pDestTxtNd, nCntntEnd );
                        aRg.aEnd = pEnd->nNode;
                        pEndTxtNd = pEnd->nNode.GetNode().GetTxtNode();
                    }
                }

                
                
                int aNumRuleState = SFX_ITEM_UNKNOWN;
                SwNumRuleItem aNumRuleItem;
                int aListIdState = SFX_ITEM_UNKNOWN;
                SfxStringItem aListIdItem( RES_PARATR_LIST_ID, OUString() );
                {
                    const SfxItemSet * pAttrSet = pDestTxtNd->GetpSwAttrSet();
                    if (pAttrSet != NULL)
                    {
                        const SfxPoolItem * pItem = NULL;
                        aNumRuleState = pAttrSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem);
                        if (SFX_ITEM_SET == aNumRuleState)
                            aNumRuleItem = *((SwNumRuleItem *) pItem);

                        aListIdState =
                            pAttrSet->GetItemState(RES_PARATR_LIST_ID, false, &pItem);
                        if (SFX_ITEM_SET == aListIdState)
                        {
                            aListIdItem.SetValue( static_cast<const SfxStringItem*>(pItem)->GetValue() );
                        }
                    }
                }

                if( !bCopyOk )
                {
                    const sal_Int32 nCpyLen = ( (bOneNode)
                                           ? pEnd->nContent.GetIndex()
                                           : pSttTxtNd->GetTxt().getLength())
                                         - pStt->nContent.GetIndex();
                    pSttTxtNd->CopyText( pDestTxtNd, aDestIdx,
                                            pStt->nContent, nCpyLen );
                    if( bEndEqualIns )
                        pEnd->nContent -= nCpyLen;
                }

                if( bOneNode )
                {
                    if( bCopyCollFmt )
                    {
                        pSttTxtNd->CopyCollFmt( *pDestTxtNd );

                        /* If only a part of one paragraph is copied
                           restore the numrule at the destination. */
                        
                        if ( !lcl_MarksWholeNode(rPam) )
                        {
                            if (SFX_ITEM_SET == aNumRuleState)
                            {
                                pDestTxtNd->SetAttr(aNumRuleItem);
                            }
                            else
                            {
                                pDestTxtNd->ResetAttr(RES_PARATR_NUMRULE);
                            }
                            if (SFX_ITEM_SET == aListIdState)
                            {
                                pDestTxtNd->SetAttr(aListIdItem);
                            }
                            else
                            {
                                pDestTxtNd->ResetAttr(RES_PARATR_LIST_ID);
                            }
                        }
                    }

                    break;
                }

                aRg.aStart++;
            }
        }
        else if( pDestTxtNd )
        {
            
            
            
            
            if( rPos.nContent.GetIndex() == pDestTxtNd->Len() )
            {    
                ++aInsPos; 
            }
            else if( rPos.nContent.GetIndex() )
            {   
                
                bStartIsTxtNode = sal_True;

                const sal_Int32 nCntntEnd = pEnd->nContent.GetIndex();
                {
                    ::sw::UndoGuard const ug(pDoc->GetIDocumentUndoRedo());
                    pDoc->SplitNode( rPos, false );
                }

                if (bCanMoveBack && rPos == *pCopyPam->GetPoint())
                {
                    
                    pCopyPam->Move( fnMoveBackward, fnGoCntnt );
                    pCopyPam->Move( fnMoveBackward, fnGoCntnt );
                }

                
                if( bEndEqualIns )
                    aRg.aEnd--;
                
                else if( rPos == *pEnd )
                {
                    rPos.nNode-=2;
                    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                                            nCntntEnd );
                    rPos.nNode++;
                    aRg.aEnd--;
                }
            }
            else if( bCanMoveBack )
            {   
                
                
                
                
                
                bCanMoveBack = false;
                pCopyPam->GetPoint()->nNode--;
            }
        }

        pDestTxtNd = aInsPos.GetNode().GetTxtNode();
        if( pEndTxtNd )
        {
            SwIndex aDestIdx( rPos.nContent );
            if( !pDestTxtNd )
            {
                pDestTxtNd = pDoc->GetNodes().MakeTxtNode( aInsPos,
                            pDoc->GetTxtCollFromPool(RES_POOLCOLL_STANDARD));
                aDestIdx.Assign( pDestTxtNd, 0  );
                aInsPos--;

                
                
                
                
                
                OSL_ENSURE( !bStartIsTxtNode, "Oops, undo may be instable now." );
                bStartIsTxtNode = sal_True;
            }

            
            
            int aNumRuleState = SFX_ITEM_UNKNOWN;
            SwNumRuleItem aNumRuleItem;
            int aListIdState = SFX_ITEM_UNKNOWN;
            SfxStringItem aListIdItem( RES_PARATR_LIST_ID, OUString() );
            {
                const SfxItemSet* pAttrSet = pDestTxtNd->GetpSwAttrSet();
                if (pAttrSet != NULL)
                {
                    const SfxPoolItem * pItem = NULL;

                    aNumRuleState =
                        pAttrSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem);
                    if (SFX_ITEM_SET == aNumRuleState)
                        aNumRuleItem = *((SwNumRuleItem *) pItem);

                    aListIdState =
                        pAttrSet->GetItemState(RES_PARATR_LIST_ID, false, &pItem);
                    if (SFX_ITEM_SET == aListIdState)
                        aListIdItem.SetValue( static_cast<const SfxStringItem*>(pItem)->GetValue() );
                }
            }

            const bool bEmptyDestNd = pDestTxtNd->GetTxt().isEmpty();
            pEndTxtNd->CopyText( pDestTxtNd, aDestIdx, SwIndex( pEndTxtNd ),
                            pEnd->nContent.GetIndex() );

            
            if( bCopyCollFmt && ( bOneNode || bEmptyDestNd ))
            {
                pEndTxtNd->CopyCollFmt( *pDestTxtNd );

                if ( bOneNode )
                {
                    /* If only a part of one paragraph is copied
                       restore the numrule at the destination. */
                    
                    if ( !lcl_MarksWholeNode(rPam) )
                    {
                        if (SFX_ITEM_SET == aNumRuleState)
                        {
                            pDestTxtNd->SetAttr(aNumRuleItem);
                        }
                        else
                        {
                            pDestTxtNd->ResetAttr(RES_PARATR_NUMRULE);
                        }
                        if (SFX_ITEM_SET == aListIdState)
                        {
                            pDestTxtNd->SetAttr(aListIdItem);
                        }
                        else
                        {
                            pDestTxtNd->ResetAttr(RES_PARATR_LIST_ID);
                        }
                    }
                }
            }
        }

        if( bCopyAll || aRg.aStart != aRg.aEnd )
        {
            SfxItemSet aBrkSet( pDoc->GetAttrPool(), aBreakSetRange );
            if( pSttTxtNd && bCopyCollFmt && pDestTxtNd->HasSwAttrSet() )
            {
                aBrkSet.Put( *pDestTxtNd->GetpSwAttrSet() );
                if( SFX_ITEM_SET == aBrkSet.GetItemState( RES_BREAK, false ) )
                    pDestTxtNd->ResetAttr( RES_BREAK );
                if( SFX_ITEM_SET == aBrkSet.GetItemState( RES_PAGEDESC, false ) )
                    pDestTxtNd->ResetAttr( RES_PAGEDESC );
            }

            if( aInsPos == pEnd->nNode )
            {
                SwNodeIndex aSaveIdx( aInsPos, -1 );
                CopyWithFlyInFly( aRg, 0,aInsPos, &rPam, bMakeNewFrms, sal_False );
                ++aSaveIdx;
                pEnd->nNode = aSaveIdx;
                pEnd->nContent.Assign( aSaveIdx.GetNode().GetTxtNode(), 0 );
            }
            else
                CopyWithFlyInFly( aRg, pEnd->nContent.GetIndex(), aInsPos, &rPam, bMakeNewFrms, sal_False );

            bCopyBookmarks = false;

            
            if( aBrkSet.Count() && 0 != ( pDestTxtNd = pDoc->GetNodes()[
                    pCopyPam->GetPoint()->nNode.GetIndex()+1 ]->GetTxtNode()))
            {
                pDestTxtNd->SetAttr( aBrkSet );
            }
        }
    } while( false );

    
    rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(),
                            rPos.nContent.GetIndex() );

    if( rPos.nNode != aInsPos )
    {
        pCopyPam->GetMark()->nNode = aInsPos;
        pCopyPam->GetMark()->nContent.Assign(pCopyPam->GetCntntNode(false), 0);
        rPos = *pCopyPam->GetMark();
    }
    else
        *pCopyPam->GetMark() = rPos;

    if ( !bAfterTable )
        pCopyPam->Move( fnMoveForward, bCanMoveBack ? fnGoCntnt : fnGoNode );
    else
    {
        
        pCopyPam->GetPoint()->nContent -= pCopyPam->GetPoint()->nContent;

        pCopyPam->GetPoint()->nNode++;
        
        
        if (pCopyPam->GetPoint()->nNode.GetNode().IsStartNode())
            pCopyPam->GetPoint()->nNode--;

    }
    pCopyPam->Exchange();

    
    if( bCopyBookmarks && getIDocumentMarkAccess()->getAllMarksCount() )
        lcl_CopyBookmarks( rPam, *pCopyPam );

    if( nsRedlineMode_t::REDLINE_DELETE_REDLINES & eOld )
    {
        assert(*pCopyPam->GetPoint() == rPos);
        
        rPos.nContent = SwIndex(0);
        lcl_DeleteRedlines(rPam, *pCopyPam);
        rPos = *pCopyPam->GetPoint(); 
    }

    
    if (pDoc->GetIDocumentUndoRedo().DoesUndo())
    {
        pUndo->SetInsertRange( *pCopyPam, sal_True, bStartIsTxtNode );
    }

    if( pCpyRange )
    {
        pCpyRange->SetMark();
        *pCpyRange->GetPoint() = *pCopyPam->GetPoint();
        *pCpyRange->GetMark() = *pCopyPam->GetMark();
    }

    if ( pNumRuleToPropagate )
    {
        
        pDoc->SetNumRule( *pCopyPam, *pNumRuleToPropagate, false,
                          aListIdToPropagate, true, true );
    }

    pDoc->SetRedlineMode_intern( eOld );
    pDoc->SetModified();

    return true;
}


void SwDoc::CopyWithFlyInFly(
    const SwNodeRange& rRg,
    const sal_Int32 nEndContentIndex,
    const SwNodeIndex& rInsPos,
    const SwPaM* pCopiedPaM,
    const sal_Bool bMakeNewFrms,
    const sal_Bool bDelRedlines,
    const sal_Bool bCopyFlyAtFly ) const
{
    SwDoc* pDest = rInsPos.GetNode().GetDoc();

    _SaveRedlEndPosForRestore aRedlRest( rInsPos, 0 );

    SwNodeIndex aSavePos( rInsPos, -1 );
    bool bEndIsEqualEndPos = rInsPos == rRg.aEnd;
    GetNodes()._CopyNodes( rRg, rInsPos, bMakeNewFrms, sal_True );
    ++aSavePos;
    if( bEndIsEqualEndPos )
        ((SwNodeIndex&)rRg.aEnd) = aSavePos;

    aRedlRest.Restore();

#if OSL_DEBUG_LEVEL > 0
    {
        
        
        
        const SwSectionNode* pSSectNd = rRg.aStart.GetNode().FindSectionNode();
        SwNodeIndex aTmpI( rRg.aEnd, -1 );
        const SwSectionNode* pESectNd = aTmpI.GetNode().FindSectionNode();
        if( pSSectNd == pESectNd &&
            !rRg.aStart.GetNode().IsSectionNode() &&
            !aTmpI.GetNode().IsEndNode() )
        {
            OSL_ENSURE( rInsPos.GetIndex() - aSavePos.GetIndex() ==
                    rRg.aEnd.GetIndex() - rRg.aStart.GetIndex(),
                    "An insufficient number of nodes were copied!" );
        }
    }
#endif

    {
        ::sw::UndoGuard const undoGuard(pDest->GetIDocumentUndoRedo());
        CopyFlyInFlyImpl( rRg, nEndContentIndex, aSavePos, bCopyFlyAtFly );
    }

    SwNodeRange aCpyRange( aSavePos, rInsPos );

    
    if( getIDocumentMarkAccess()->getAllMarksCount() )
    {
        SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
        SwPaM aCpyTmp( aCpyRange.aStart, aCpyRange.aEnd );

        lcl_CopyBookmarks(
            pCopiedPaM != NULL ? *pCopiedPaM : aRgTmp,
            aCpyTmp );
    }

    if( bDelRedlines && ( nsRedlineMode_t::REDLINE_DELETE_REDLINES & pDest->GetRedlineMode() ))
        lcl_DeleteRedlines( rRg, aCpyRange );

    pDest->GetNodes()._DelDummyNodes( aCpyRange );
}

static void lcl_ChainFmts( SwFlyFrmFmt *pSrc, SwFlyFrmFmt *pDest )
{
    SwFmtChain aSrc( pSrc->GetChain() );
    if ( !aSrc.GetNext() )
    {
        aSrc.SetNext( pDest );
        pSrc->SetFmtAttr( aSrc );
    }
    SwFmtChain aDest( pDest->GetChain() );
    if ( !aDest.GetPrev() )
    {
        aDest.SetPrev( pSrc );
        pDest->SetFmtAttr( aDest );
    }
}

void SwDoc::CopyFlyInFlyImpl(
    const SwNodeRange& rRg,
    const sal_Int32 nEndContentIndex,
    const SwNodeIndex& rStartIdx,
    const bool bCopyFlyAtFly ) const
{
    
    
    
    SwDoc *const pDest = rStartIdx.GetNode().GetDoc();
    ::std::set< _ZSortFly > aSet;
    sal_uInt16 nArrLen = GetSpzFrmFmts()->size();

    for ( sal_uInt16 n = 0; n < nArrLen; ++n )
    {
        SwFrmFmt const*const pFmt = (*GetSpzFrmFmts())[n];
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();
        bool bAtCntnt = (pAnchor->GetAnchorId() == FLY_AT_PARA);
        if ( pAPos &&
             ( bAtCntnt ||
              (pAnchor->GetAnchorId() == FLY_AT_FLY) ||
              (pAnchor->GetAnchorId() == FLY_AT_CHAR)) &&
             (( bCopyFlyAtFly && FLY_AT_FLY == pAnchor->GetAnchorId() )
                    ? rRg.aStart <= pAPos->nNode.GetIndex() + 1
                    : ( IsRedlineMove()
                            ? rRg.aStart < pAPos->nNode
                            : rRg.aStart <= pAPos->nNode )) &&
             pAPos->nNode <= rRg.aEnd )
        {
            
            
            
            
            
            bool bAdd = false;
            if( pAPos->nNode < rRg.aEnd )
                bAdd = true;
            if (!bAdd && !IsRedlineMove()) 
            {
                bool bEmptyNode = false;
                bool bLastNode = false;
                
                const SwNodes& rNodes = pAPos->nNode.GetNodes();
                SwTxtNode* pTxtNode;
                if( 0 != ( pTxtNode = pAPos->nNode.GetNode().GetTxtNode() ))
                {
                    bEmptyNode = pTxtNode->GetTxt().isEmpty();
                    if( bEmptyNode )
                    {
                        
                        SwNodeIndex aTmp( pAPos->nNode );
                        ++aTmp;
                        while (aTmp.GetNode().IsEndNode())
                        {
                            if( aTmp == rNodes.GetEndOfContent().GetIndex() )
                            {
                                bLastNode = true;
                                break;
                            }
                            ++aTmp;
                        }
                    }
                }
                bAdd = bLastNode && bEmptyNode;
                if( !bAdd )
                {
                    if( bAtCntnt )
                        bAdd = nEndContentIndex > 0;
                    else
                        bAdd = pAPos->nContent <= nEndContentIndex;
                }
            }
            if( bAdd )
                aSet.insert( _ZSortFly( pFmt, pAnchor, nArrLen + aSet.size() ));
        }
    }

    
    
    
    ::std::vector< SwFrmFmt* > aVecSwFrmFmt;
    ::std::set< _ZSortFly >::const_iterator it=aSet.begin();

    while (it != aSet.end())
    {
        
        
        SwFmtAnchor aAnchor( *(*it).GetAnchor() );
        SwPosition* pNewPos = (SwPosition*)aAnchor.GetCntntAnchor();
        
        
        
        
        
        
        
        if ((aAnchor.GetAnchorId() == FLY_AT_PARA) ||
            (aAnchor.GetAnchorId() == FLY_AT_CHAR) )
        {
            
            
            sal_uLong nAnchorTxtNdNumInRange( 0L );
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
                
                
                OSL_FAIL( "<SwDoc::_CopyFlyInFly(..)> - anchor text node in copied range not found" );
                nAnchorTxtNdNumInRange = 1;
            }
            
            
            
            aIdx = rStartIdx;
            SwNodeIndex aAnchorNdIdx( rStartIdx );
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
                
                
                OSL_FAIL( "<SwDoc::_CopyFlyInFly(..)> - found anchor node index isn't a text node" );
                aAnchorNdIdx = rStartIdx;
                while ( !aAnchorNdIdx.GetNode().IsTxtNode() )
                {
                    ++aAnchorNdIdx;
                }
            }
            
            pNewPos->nNode = aAnchorNdIdx;
        }
        else
        {
            long nOffset = pNewPos->nNode.GetIndex() - rRg.aStart.GetIndex();
            SwNodeIndex aIdx( rStartIdx, nOffset );
            pNewPos->nNode = aIdx;
        }
        
        if ((FLY_AT_CHAR == aAnchor.GetAnchorId()) &&
             pNewPos->nNode.GetNode().IsTxtNode() )
        {
            pNewPos->nContent.Assign( (SwTxtNode*)&pNewPos->nNode.GetNode(),
                                        pNewPos->nContent.GetIndex() );
        }
        else
        {
            pNewPos->nContent.Assign( 0, 0 );
        }

        
        bool bMakeCpy = true;
        if( pDest == this )
        {
            const SwFmtCntnt& rCntnt = (*it).GetFmt()->GetCntnt();
            const SwStartNode* pSNd;
            if( rCntnt.GetCntntIdx() &&
                0 != ( pSNd = rCntnt.GetCntntIdx()->GetNode().GetStartNode() ) &&
                pSNd->GetIndex() < rStartIdx.GetIndex() &&
                rStartIdx.GetIndex() < pSNd->EndOfSectionIndex() )
            {
                bMakeCpy = false;
                aSet.erase (it++);
                continue;
            }
        }

        
        if( bMakeCpy )
            aVecSwFrmFmt.push_back( pDest->CopyLayoutFmt( *(*it).GetFmt(),
                        aAnchor, false, true ) );
        ++it;
    }

    
    OSL_ENSURE( aSet.size() == aVecSwFrmFmt.size(), "Missing new Flys" );
    if ( aSet.size() == aVecSwFrmFmt.size() )
    {
        size_t n = 0;
        for (::std::set< _ZSortFly >::const_iterator nIt=aSet.begin() ; nIt != aSet.end(); ++nIt, ++n )
        {
            const SwFrmFmt *pFmtN = (*nIt).GetFmt();
            const SwFmtChain &rChain = pFmtN->GetChain();
            int nCnt = int(0 != rChain.GetPrev());
            nCnt += rChain.GetNext() ? 1: 0;
            size_t k = 0;
            for (::std::set< _ZSortFly >::const_iterator kIt=aSet.begin() ; kIt != aSet.end(); ++kIt, ++k )
            {
                const SwFrmFmt *pFmtK = (*kIt).GetFmt();
                if ( rChain.GetPrev() == pFmtK )
                {
                    ::lcl_ChainFmts( static_cast< SwFlyFrmFmt* >(aVecSwFrmFmt[k]),
                                     static_cast< SwFlyFrmFmt* >(aVecSwFrmFmt[n]) );
                    --nCnt;
                }
                else if ( rChain.GetNext() == pFmtK )
                {
                    ::lcl_ChainFmts( static_cast< SwFlyFrmFmt* >(aVecSwFrmFmt[n]),
                                     static_cast< SwFlyFrmFmt* >(aVecSwFrmFmt[k]) );
                    --nCnt;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
