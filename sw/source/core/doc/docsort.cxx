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

#include <boost/ptr_container/ptr_set.hpp>

#include <hintids.hxx>
#include <rtl/math.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <comphelper/processfactory.hxx>
#include <editeng/unolingu.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <swundo.hxx>
#include <sortopt.hxx>
#include <docsort.hxx>
#include <UndoSort.hxx>
#include <UndoRedline.hxx>
#include <hints.hxx>
#include <tblsel.hxx>
#include <cellatr.hxx>
#include <redline.hxx>
#include <node2lay.hxx>
#include <unochart.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

SwSortOptions*      SwSortElement::pOptions = 0;
SwDoc*              SwSortElement::pDoc = 0;
const FlatFndBox*   SwSortElement::pBox = 0;
CollatorWrapper*    SwSortElement::pSortCollator = 0;
lang::Locale*       SwSortElement::pLocale = 0;
OUString*           SwSortElement::pLastAlgorithm = 0;
LocaleDataWrapper*  SwSortElement::pLclData = 0;


typedef SwSortElement*      SwSortElementPtr;

typedef ::boost::ptr_multiset<SwSortTxtElement> SwSortTxtElements;
typedef ::boost::ptr_multiset<SwSortBoxElement> SwSortBoxElements;


void SwSortElement::Init( SwDoc* pD, const SwSortOptions& rOpt,
                            FlatFndBox* pFltBx )
{
    OSL_ENSURE( !pDoc && !pOptions && !pBox, "Who forgot to call Finit?" );
    pDoc = pD;
    pOptions = new SwSortOptions( rOpt );
    pBox = pFltBx;

    LanguageType nLang = rOpt.nLanguage;
    switch ( nLang )
    {
    case LANGUAGE_NONE:
    case LANGUAGE_DONTKNOW:
        nLang = GetAppLanguage();
        break;
    }
    pLocale = new lang::Locale( LanguageTag::convertToLocale( nLang ) );

    pSortCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
}

void SwSortElement::Finit()
{
    delete pOptions, pOptions = 0;
    delete pLocale, pLocale = 0;
    delete pLastAlgorithm, pLastAlgorithm = 0;
    delete pSortCollator, pSortCollator = 0;
    delete pLclData, pLclData = 0;
    pDoc = 0;
    pBox = 0;
}

SwSortElement::~SwSortElement()
{
}

double SwSortElement::StrToDouble( const OUString& rStr ) const
{
    if( !pLclData )
        pLclData = new LocaleDataWrapper( LanguageTag( *pLocale ));

    rtl_math_ConversionStatus eStatus;
    sal_Int32 nEnd;
    double nRet = ::rtl::math::stringToDouble( rStr,
                                    pLclData->getNumDecimalSep()[0],
                                    pLclData->getNumThousandSep()[0],
                                    &eStatus, &nEnd );

    if( rtl_math_ConversionStatus_Ok != eStatus || nEnd == 0 )
        nRet = 0.0;
    return nRet;
}

int SwSortElement::keycompare(const SwSortElement& rCmp, sal_uInt16 nKey) const
{
    int nCmp = 0;
    
    const SwSortElement *pOrig, *pCmp;

    const SwSortKey* pSrtKey = pOptions->aKeys[ nKey ];
    if( pSrtKey->eSortOrder == SRT_ASCENDING )
        pOrig = this, pCmp = &rCmp;
    else
        pOrig = &rCmp, pCmp = this;

    if( pSrtKey->bIsNumeric )
    {
        double n1 = pOrig->GetValue( nKey );
        double n2 = pCmp->GetValue( nKey );

        nCmp = n1 < n2 ? -1 : n1 == n2 ? 0 : 1;
    }
    else
    {
        if( !pLastAlgorithm || *pLastAlgorithm != pSrtKey->sSortType )
        {
            if( pLastAlgorithm )
                *pLastAlgorithm = pSrtKey->sSortType;
            else
                pLastAlgorithm = new OUString( pSrtKey->sSortType );
            pSortCollator->loadCollatorAlgorithm( *pLastAlgorithm,
                    *pLocale,
                    pOptions->bIgnoreCase ? SW_COLLATOR_IGNORES : 0 );
        }

        nCmp = pSortCollator->compareString(
                    pOrig->GetKey( nKey ), pCmp->GetKey( nKey ));
    }
    return nCmp;
}

bool SwSortElement::operator==(const SwSortElement& ) const
{
    return false;
}

bool SwSortElement::operator<(const SwSortElement& rCmp) const
{
    
    for(sal_uInt16 nKey = 0; nKey < pOptions->aKeys.size(); ++nKey)
    {
        int nCmp = keycompare(rCmp, nKey);

        if (nCmp == 0)
            continue;

        return nCmp < 0;
    }

    return false;
}

double SwSortElement::GetValue( sal_uInt16 nKey ) const
{
    return StrToDouble( GetKey( nKey ));
}


SwSortTxtElement::SwSortTxtElement(const SwNodeIndex& rPos)
    : nOrg(rPos.GetIndex()), aPos(rPos)
{
}

SwSortTxtElement::~SwSortTxtElement()
{
}

OUString SwSortTxtElement::GetKey(sal_uInt16 nId) const
{
    SwTxtNode* pTxtNd = aPos.GetNode().GetTxtNode();
    if( !pTxtNd )
        return OUString();

    
    const OUString& rStr = pTxtNd->GetTxt();

    sal_Unicode nDeli = pOptions->cDeli;
    sal_uInt16 nDCount = pOptions->aKeys[nId]->nColumnId, i = 1;
    sal_Int32 nStart = 0;

    
    while( nStart != -1 && i < nDCount)
        if( -1 != ( nStart = rStr.indexOf( nDeli, nStart ) ) )
        {
            nStart++;
            i++;
        }

    
    
    sal_Int32 nEnd = rStr.indexOf( nDeli, nStart+1 );
    if (nEnd == -1)
        return rStr.copy( nStart );
    return rStr.copy( nStart, nEnd-nStart );
}


SwSortBoxElement::SwSortBoxElement( sal_uInt16 nRC )
    : nRow( nRC )
{
}

SwSortBoxElement::~SwSortBoxElement()
{
}


OUString SwSortBoxElement::GetKey(sal_uInt16 nKey) const
{
    const _FndBox* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         
    else
        pFndBox = pBox->GetBox(nRow, nCol);         

    
    OUString aRetStr;
    if( pFndBox )
    {   
        const SwTableBox* pMyBox = pFndBox->GetBox();
        OSL_ENSURE(pMyBox, "No atomic Box");

        if( pMyBox->GetSttNd() )
        {
            
            const SwNode *pNd = 0, *pEndNd = pMyBox->GetSttNd()->EndOfSectionNode();
            for( sal_uLong nIdx = pMyBox->GetSttIdx() + 1; pNd != pEndNd; ++nIdx )
                if( ( pNd = pDoc->GetNodes()[ nIdx ])->IsTxtNode() )
                    aRetStr += ((SwTxtNode*)pNd)->GetTxt();
        }
    }
    return aRetStr;
}

double SwSortBoxElement::GetValue( sal_uInt16 nKey ) const
{
    const _FndBox* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         
    else
        pFndBox = pBox->GetBox(nRow, nCol);         


    double nVal;
    if( pFndBox )
    {
        const SwFmt *pFmt = pFndBox->GetBox()->GetFrmFmt();
        if (pFmt->GetTblBoxNumFmt().GetValue() & NUMBERFORMAT_TEXT)
            nVal = SwSortElement::GetValue( nKey );
        else
            nVal = pFmt->GetTblBoxValue().GetValue();
    }
    else
        nVal = 0;

    return nVal;
}


bool SwDoc::SortText(const SwPaM& rPaM, const SwSortOptions& rOpt)
{
    
    const SwPosition *pStart = rPaM.Start(), *pEnd = rPaM.End();

    
    for ( sal_uInt16 n = 0; n < GetSpzFrmFmts()->size(); ++n )
    {
        SwFrmFmt *const pFmt = static_cast<SwFrmFmt*>((*GetSpzFrmFmts())[n]);
        SwFmtAnchor const*const pAnchor = &pFmt->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetCntntAnchor();

        if (pAPos && (FLY_AT_PARA == pAnchor->GetAnchorId()) &&
            pStart->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode )
            return false;
    }

    
    {
        sal_uLong nStart = pStart->nNode.GetIndex(),
                        nEnd = pEnd->nNode.GetIndex();
        while( nStart <= nEnd )
            
            if( !GetNodes()[ nStart++ ]->IsTxtNode() )
                return false;
    }

    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if( bUndo )
    {
        GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
    }

    SwPaM* pRedlPam = 0;
    SwUndoRedlineSort* pRedlUndo = 0;
    SwUndoSort* pUndoSort = 0;

    
    if( IsRedlineOn() || (!IsIgnoreRedline() && !mpRedlineTbl->empty() ))
    {
        pRedlPam = new SwPaM( pStart->nNode, pEnd->nNode, -1, 1 );
        SwCntntNode* pCNd = pRedlPam->GetCntntNode( false );
        if( pCNd )
            pRedlPam->GetMark()->nContent = pCNd->Len();

        if( IsRedlineOn() && !IsShowOriginal( GetRedlineMode() ) )
        {
            if( bUndo )
            {
                pRedlUndo = new SwUndoRedlineSort( *pRedlPam,rOpt );
                GetIDocumentUndoRedo().DoUndo(false);
            }
            
            SwNodeIndex aEndIdx( pEnd->nNode, 1 );
            SwNodeRange aRg( pStart->nNode, aEndIdx );
            GetNodes()._Copy( aRg, aEndIdx );

            
            DeleteRedline( *pRedlPam, true, USHRT_MAX );

            pRedlPam->GetMark()->nNode.Assign( pEnd->nNode.GetNode(), 1 );
            pCNd = pRedlPam->GetCntntNode( false );
            pRedlPam->GetMark()->nContent.Assign( pCNd, 0 );

            pRedlPam->GetPoint()->nNode.Assign( aEndIdx.GetNode() );
            pCNd = pRedlPam->GetCntntNode( true );
            sal_Int32 nCLen = 0;
            if( !pCNd &&
                0 != (pCNd = GetNodes()[ aEndIdx.GetIndex()-1 ]->GetCntntNode()))
            {
                nCLen = pCNd->Len();
                pRedlPam->GetPoint()->nNode.Assign( *pCNd );
            }
            pRedlPam->GetPoint()->nContent.Assign( pCNd, nCLen );

            if( pRedlUndo )
                pRedlUndo->SetValues( rPaM );
        }
        else
        {
            DeleteRedline( *pRedlPam, true, USHRT_MAX );
            delete pRedlPam, pRedlPam = 0;
        }
    }

    SwNodeIndex aStart(pStart->nNode);
    SwSortElement::Init( this, rOpt );
    SwSortTxtElements aSortSet;
    while( aStart <= pEnd->nNode )
    {
        
        SwSortTxtElement* pSE = new SwSortTxtElement( aStart );
        aSortSet.insert(pSE);
        ++aStart;
    }

    
    sal_uLong nBeg = pStart->nNode.GetIndex();
    SwNodeRange aRg( aStart, aStart );

    if( bUndo && !pRedlUndo )
    {
        pUndoSort = new SwUndoSort(rPaM, rOpt);
        GetIDocumentUndoRedo().AppendUndo(pUndoSort);
    }

    GetIDocumentUndoRedo().DoUndo(false);

    size_t n = 0;
    for (SwSortTxtElements::const_iterator it = aSortSet.begin();
            it != aSortSet.end(); ++it, ++n)
    {
        aStart      = nBeg + n;
        aRg.aStart  = it->aPos.GetIndex();
        aRg.aEnd    = aRg.aStart.GetIndex() + 1;

        
        MoveNodeRange( aRg, aStart,
            IDocumentContentOperations::DOC_MOVEDEFAULT );

        
        if(pUndoSort)
        {
            pUndoSort->Insert(it->nOrg, nBeg + n);
        }
    }
    
    aSortSet.clear();
    SwSortElement::Finit();

    if( pRedlPam )
    {
        if( pRedlUndo )
        {
            pRedlUndo->SetSaveRange( *pRedlPam );
            
            GetIDocumentUndoRedo().DoUndo(true);
            GetIDocumentUndoRedo().AppendUndo( pRedlUndo );
            GetIDocumentUndoRedo().DoUndo(false);
        }

        
        SwNodeIndex aSttIdx( GetNodes(), nBeg );

        
        SwRangeRedline *const pDeleteRedline(
            new SwRangeRedline( nsRedlineType_t::REDLINE_DELETE, *pRedlPam ));

        
        
        pRedlPam->GetPoint()->nNode = aSttIdx;
        SwCntntNode* pCNd = aSttIdx.GetNode().GetCntntNode();
        pRedlPam->GetPoint()->nContent.Assign( pCNd, 0 );

        AppendRedline(pDeleteRedline, true);

        
        AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pRedlPam ), true);

        if( pRedlUndo )
        {
            SwNodeIndex aInsEndIdx( pRedlPam->GetMark()->nNode, -1 );
            pRedlPam->GetMark()->nNode = aInsEndIdx;
            SwCntntNode *const pPrevNode =
                pRedlPam->GetMark()->nNode.GetNode().GetCntntNode();
            pRedlPam->GetMark()->nContent.Assign( pPrevNode, pPrevNode->Len() );

            pRedlUndo->SetValues( *pRedlPam );
        }

        if( pRedlUndo )
            pRedlUndo->SetOffset( aSttIdx );

        delete pRedlPam, pRedlPam = 0;
    }
    GetIDocumentUndoRedo().DoUndo( bUndo );
    if( bUndo )
    {
        GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }

    return true;
}


bool SwDoc::SortTbl(const SwSelBoxes& rBoxes, const SwSortOptions& rOpt)
{
    
    OSL_ENSURE( !rBoxes.empty(), "no valid Box list" );
    SwTableNode* pTblNd = (SwTableNode*)rBoxes[0]->GetSttNd()->FindTableNode();
    if( !pTblNd )
        return false;

    
    
    _FndBox aFndBox( 0, 0 );
    {
        _FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTblNd->GetTable().GetTabLines(), &aPara );
    }

    if(aFndBox.GetLines().empty())
        return false;

    if( !IsIgnoreRedline() && !GetRedlineTbl().empty() )
        DeleteRedline( *pTblNd, true, USHRT_MAX );

    sal_uInt16 nStart = 0;
    if( pTblNd->GetTable().GetRowsToRepeat() > 0 && rOpt.eDirection == SRT_ROWS )
    {
        
        _FndLines& rLines = aFndBox.GetLines();

        while( nStart < rLines.size() )
        {
            
            
            SwTableLine* pLine = rLines[nStart].GetLine();
            while ( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTblNd->GetTable().IsHeadline( *pLine ) )
                nStart++;
            else
                break;
        }
        
        if( nStart == rLines.size() )
            nStart = 0;
    }

    
    SwTableFmlUpdate aMsgHnt( &pTblNd->GetTable() );
    aMsgHnt.eFlags = TBL_RELBOXNAME;
    UpdateTblFlds( &aMsgHnt );

    
    FlatFndBox aFlatBox(this, aFndBox);

    if(!aFlatBox.IsSymmetric())
        return false;

    
    pTblNd->GetTable().SetHTMLTableLayout( 0 );

    
    
    SwNode2Layout aNode2Layout( *pTblNd );

    
    pTblNd->DelFrms();
    

    SwUndoSort* pUndoSort = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndoSort = new SwUndoSort( rBoxes[0]->GetSttIdx(),
                                    rBoxes.back()->GetSttIdx(),
                                   *pTblNd, rOpt, aFlatBox.HasItemSets() );
        GetIDocumentUndoRedo().AppendUndo(pUndoSort);
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    
    sal_uInt16 nCount = (rOpt.eDirection == SRT_ROWS) ?
                    aFlatBox.GetRows() : aFlatBox.GetCols();

    
    SwSortElement::Init( this, rOpt, &aFlatBox );
    SwSortBoxElements aSortList;

    
    sal_uInt16 i;

    for( i = nStart; i < nCount; ++i)
    {
        SwSortBoxElement* pEle = new SwSortBoxElement( i );
        aSortList.insert(pEle);
    }

    
    SwMovedBoxes aMovedList;
    i = 0;
    for (SwSortBoxElements::const_iterator it = aSortList.begin();
            it != aSortList.end(); ++i, ++it)
    {
        if(rOpt.eDirection == SRT_ROWS)
        {
            MoveRow(this, aFlatBox, it->nRow, i+nStart, aMovedList, pUndoSort);
        }
        else
        {
            MoveCol(this, aFlatBox, it->nRow, i+nStart, aMovedList, pUndoSort);
        }
    }

    
    
    
    const sal_uLong nIdx = pTblNd->GetIndex();
    aNode2Layout.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );

    
    UpdateCharts( pTblNd->GetTable().GetFrmFmt()->GetName() );

    
    aSortList.clear();
    SwSortElement::Finit();

    SetModified();
    return true;
}


void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for( sal_uInt16 i=0; i < rBox.GetCols(); ++i )
    {   
        const _FndBox* pSource = rBox.GetBox(i, nS);

        
        const _FndBox* pTarget = rBox.GetBox(i, nT);

        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;

        
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.push_back(pS);

        if( pS != pT )
        {
            SwFrmFmt* pTFmt = (SwFrmFmt*)pT->GetFrmFmt();
            const SfxItemSet* pSSet = rBox.GetItemSet( i, nS );

            if( pSSet ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMAT ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMULA ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_VALUE ) )
            {
                pTFmt = ((SwTableBox*)pT)->ClaimFrmFmt();
                pTFmt->LockModify();
                if( pTFmt->ResetFmtAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE ) )
                    pTFmt->ResetFmtAttr( RES_VERT_ORIENT );

                if( pSSet )
                    pTFmt->SetFmtAttr( *pSSet );
                pTFmt->UnlockModify();
            }
        }
    }
}


void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for(sal_uInt16 i=0; i < rBox.GetRows(); ++i)
    {   
        const _FndBox* pSource = rBox.GetBox(nS, i);

        
        const _FndBox* pTarget = rBox.GetBox(nT, i);

        
        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        
        bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.push_back(pS);

        if( pS != pT )
        {
            SwFrmFmt* pTFmt = (SwFrmFmt*)pT->GetFrmFmt();
            const SfxItemSet* pSSet = rBox.GetItemSet( nS, i );

            if( pSSet ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMAT ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_FORMULA ) ||
                SFX_ITEM_SET == pTFmt->GetItemState( RES_BOXATR_VALUE ) )
            {
                pTFmt = ((SwTableBox*)pT)->ClaimFrmFmt();
                pTFmt->LockModify();
                if( pTFmt->ResetFmtAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE ) )
                    pTFmt->ResetFmtAttr( RES_VERT_ORIENT );

                if( pSSet )
                    pTFmt->SetFmtAttr( *pSSet );
                pTFmt->UnlockModify();
            }
        }
    }
}


void MoveCell(SwDoc* pDoc, const SwTableBox* pSource, const SwTableBox* pTar,
              bool bMovedBefore, SwUndoSort* pUD)
{
    OSL_ENSURE(pSource && pTar,"Source or target missing");

    if(pSource == pTar)
        return;

    if(pUD)
        pUD->Insert( pSource->GetName(), pTar->GetName() );

    
    SwNodeRange aRg( *pSource->GetSttNd(), 0, *pSource->GetSttNd() );
    SwNode* pNd = pDoc->GetNodes().GoNext( &aRg.aStart );

    
    
    
    if( pNd->StartOfSectionNode() == pSource->GetSttNd() )
        pNd = pDoc->GetNodes().MakeTxtNode( aRg.aStart,
                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    aRg.aEnd = *pNd->EndOfSectionNode();

    
    
    SwNodeIndex aTar( *pTar->GetSttNd() );
    pNd = pDoc->GetNodes().GoNext( &aTar );     
    sal_uLong nCount = pNd->EndOfSectionIndex() - pNd->StartOfSectionIndex();

    bool bDelFirst = false;
    if( nCount == 2 )
    {
        OSL_ENSURE( pNd->GetCntntNode(), "No ContentNode");
        bDelFirst = !pNd->GetCntntNode()->Len() && bMovedBefore;
    }

    if(!bDelFirst)
    {   
        SwNodeRange aRgTar( aTar.GetNode(), 0, *pNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRgTar );
    }

    
    SwNodeIndex aIns( *pTar->GetSttNd()->EndOfSectionNode() );
    pDoc->MoveNodeRange( aRg, aIns,
        IDocumentContentOperations::DOC_MOVEDEFAULT );

    
    if(bDelFirst)
        pDoc->GetNodes().Delete( aTar, 1 );
}


FlatFndBox::FlatFndBox(SwDoc* pDocPtr, const _FndBox& rBox) :
    pDoc(pDocPtr),
    rBoxRef(rBox),
    pArr(0),
    ppItemSets(0),
    nRow(0),
    nCol(0)
{ 
    if( (bSym = CheckLineSymmetry(rBoxRef)) )
    {
        
        nCols = GetColCount(rBoxRef);
        nRows = GetRowCount(rBoxRef);

        
        pArr = new const _FndBox*[ nRows * nCols ];
        _FndBox** ppTmp = (_FndBox**)pArr;
        memset( ppTmp, 0, sizeof(const _FndBox*) * nRows * nCols );

        FillFlat( rBoxRef );
    }
}

FlatFndBox::~FlatFndBox()
{
    _FndBox** ppTmp = (_FndBox**)pArr;
    delete [] ppTmp;

    if( ppItemSets )
        delete [] ppItemSets;
}


bool FlatFndBox::CheckLineSymmetry(const _FndBox& rBox)
{
    const _FndLines &rLines = rBox.GetLines();
    sal_uInt16 nBoxes(0);

    for(sal_uInt16 i=0; i < rLines.size(); ++i)
    {
        const _FndLine* pLn = &rLines[i];
        const _FndBoxes& rBoxes = pLn->GetBoxes();

        
        if( i  && nBoxes != rBoxes.size())
            return false;

        nBoxes = rBoxes.size();
        if( !CheckBoxSymmetry( *pLn ) )
            return false;
    }
    return true;
}


bool FlatFndBox::CheckBoxSymmetry(const _FndLine& rLn)
{
    const _FndBoxes &rBoxes = rLn.GetBoxes();
    sal_uInt16 nLines(0);

    for(sal_uInt16 i=0; i < rBoxes.size(); ++i)
    {
        _FndBox const*const pBox = &rBoxes[i];
        const _FndLines& rLines = pBox->GetLines();

        
        if( i && nLines != rLines.size() )
            return false;

        nLines = rLines.size();
        if( nLines && !CheckLineSymmetry( *pBox ) )
            return false;
    }
    return true;
}


sal_uInt16 FlatFndBox::GetColCount(const _FndBox& rBox)
{
    const _FndLines& rLines = rBox.GetLines();
    
    if( rLines.empty() )
        return 1;

    sal_uInt16 nSum = 0;
    for( sal_uInt16 i=0; i < rLines.size(); ++i )
    {
        
        sal_uInt16 nCount = 0;
        const _FndBoxes& rBoxes = rLines[i].GetBoxes();
        for( sal_uInt16 j=0; j < rBoxes.size(); ++j )
            
            nCount += (rBoxes[j].GetLines().size())
                        ? GetColCount(rBoxes[j]) : 1;

        if( nSum < nCount )
            nSum = nCount;
    }
    return nSum;
}


sal_uInt16 FlatFndBox::GetRowCount(const _FndBox& rBox)
{
    const _FndLines& rLines = rBox.GetLines();
    if( rLines.empty() )
        return 1;

    sal_uInt16 nLines = 0;
    for(sal_uInt16 i=0; i < rLines.size(); ++i)
    {   
        const _FndBoxes& rBoxes = rLines[i].GetBoxes();
        sal_uInt16 nLn = 1;
        for(sal_uInt16 j=0; j < rBoxes.size(); ++j)
            if (rBoxes[j].GetLines().size())
                
                nLn = std::max(GetRowCount(rBoxes[j]), nLn);

        nLines = nLines + nLn;
    }
    return nLines;
}


void FlatFndBox::FillFlat(const _FndBox& rBox, bool bLastBox)
{
    bool bModRow = false;
    const _FndLines& rLines = rBox.GetLines();

    
    sal_uInt16 nOldRow = nRow;
    for( sal_uInt16 i=0; i < rLines.size(); ++i )
    {
        
        const _FndBoxes& rBoxes = rLines[i].GetBoxes();
        sal_uInt16 nOldCol = nCol;
        for( sal_uInt16 j = 0; j < rBoxes.size(); ++j )
        {
            
            const _FndBox *const pBox = &rBoxes[j];

            if( !pBox->GetLines().size() )
            {
                
                sal_uInt16 nOff = nRow * nCols + nCol;
                *(pArr + nOff) = pBox;

                
                const SwFrmFmt* pFmt = pBox->GetBox()->GetFrmFmt();
                if( SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMAT ) ||
                    SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_FORMULA ) ||
                    SFX_ITEM_SET == pFmt->GetItemState( RES_BOXATR_VALUE ) )
                {
                    SfxItemSet* pSet = new SfxItemSet( pDoc->GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
                    pSet->Put( pFmt->GetAttrSet() );
                    if( !ppItemSets )
                    {
                        ppItemSets = new SfxItemSet*[ nRows * nCols ];
                        memset( ppItemSets, 0, sizeof(SfxItemSet*) * nRows * nCols );
                    }
                    *(ppItemSets + nOff ) = pSet;
                }

                bModRow = true;
            }
            else
            {
                
                FillFlat( *pBox, ( j == rBoxes.size()-1 ) );
            }
            nCol++;
        }
        if(bModRow)
            nRow++;
        nCol = nOldCol;
    }
    if(!bLastBox)
        nRow = nOldRow;
}


const _FndBox* FlatFndBox::GetBox(sal_uInt16 n_Col, sal_uInt16 n_Row) const
{
    sal_uInt16 nOff = n_Row * nCols + n_Col;
    const _FndBox* pTmp = *(pArr + nOff);

    OSL_ENSURE(n_Col < nCols && n_Row < nRows && pTmp, "invalid array access");
    return pTmp;
}

const SfxItemSet* FlatFndBox::GetItemSet(sal_uInt16 n_Col, sal_uInt16 n_Row) const
{
    OSL_ENSURE( !ppItemSets || ( n_Col < nCols && n_Row < nRows), "invalid array access");

    return ppItemSets ? *(ppItemSets + (n_Row * nCols + n_Col )) : 0;
}

sal_uInt16 SwMovedBoxes::GetPos(const SwTableBox* pTableBox) const
{
    const_iterator it = std::find(begin(), end(), pTableBox);
    return it == end() ? USHRT_MAX : it - begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
