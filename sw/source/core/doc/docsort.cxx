/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>
#include <rtl/math.hxx>
#include <osl/diagnose.h>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/unolingu.hxx>
#include <docary.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
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

#include <set>
#include <utility>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

SwSortOptions*      SwSortElement::pOptions = nullptr;
SwDoc*              SwSortElement::pDoc = nullptr;
const FlatFndBox*   SwSortElement::pBox = nullptr;
CollatorWrapper*    SwSortElement::pSortCollator = nullptr;
lang::Locale*       SwSortElement::pLocale = nullptr;
OUString*           SwSortElement::pLastAlgorithm = nullptr;
LocaleDataWrapper*  SwSortElement::pLclData = nullptr;

// List of all sorted elements

typedef std::multiset<SwSortTextElement> SwSortTextElements;
typedef std::multiset<SwSortBoxElement> SwSortBoxElements;

/// Construct a SortElement for the Sort
void SwSortElement::Init( SwDoc* pD, const SwSortOptions& rOpt,
                            FlatFndBox const * pFltBx )
{
    OSL_ENSURE( !pDoc && !pOptions && !pBox, "Who forgot to call Finit?" );
    pDoc = pD;
    pOptions = new SwSortOptions( rOpt );
    pBox = pFltBx;

    LanguageType nLang = rOpt.nLanguage;
    if ( nLang.anyOf(
        LANGUAGE_NONE,
        LANGUAGE_DONTKNOW))
        nLang = GetAppLanguage();
    pLocale = new lang::Locale( LanguageTag::convertToLocale( nLang ) );

    pSortCollator = new CollatorWrapper( ::comphelper::getProcessComponentContext() );
}

void SwSortElement::Finit()
{
    delete pOptions;
    pOptions = nullptr;
    delete pLocale;
    pLocale = nullptr;
    delete pLastAlgorithm;
    pLastAlgorithm = nullptr;
    delete pSortCollator;
    pSortCollator = nullptr;
    delete pLclData;
    pLclData = nullptr;
    pDoc = nullptr;
    pBox = nullptr;
}

SwSortElement::~SwSortElement()
{
}

double SwSortElement::StrToDouble( const OUString& rStr )
{
    if( !pLclData )
        pLclData = new LocaleDataWrapper( LanguageTag( *pLocale ));

    rtl_math_ConversionStatus eStatus;
    sal_Int32 nEnd;
    double nRet = pLclData->stringToDouble( rStr, true, &eStatus, &nEnd );

    if( rtl_math_ConversionStatus_Ok != eStatus || nEnd == 0 )
        nRet = 0.0;
    return nRet;
}

int SwSortElement::keycompare(const SwSortElement& rCmp, sal_uInt16 nKey) const
{
    int nCmp = 0;
    // The actual comparison
    const SwSortElement *pOrig, *pCmp;

    const SwSortKey* pSrtKey = pOptions->aKeys[ nKey ].get();
    if( pSrtKey->eSortOrder == SRT_ASCENDING )
    {
        pOrig = this;
        pCmp = &rCmp;
    }
    else
    {
        pOrig = &rCmp;
        pCmp = this;
    }

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

bool SwSortElement::operator<(const SwSortElement& rCmp) const
{
    // The actual comparison
    for(size_t nKey = 0; nKey < pOptions->aKeys.size(); ++nKey)
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

/// SortingElement for Text
SwSortTextElement::SwSortTextElement(const SwNodeIndex& rPos)
    : nOrg(rPos.GetIndex()), aPos(rPos)
{
}

SwSortTextElement::~SwSortTextElement()
{
}

OUString SwSortTextElement::GetKey(sal_uInt16 nId) const
{
    SwTextNode* pTextNd = aPos.GetNode().GetTextNode();
    if( !pTextNd )
        return OUString();

    // for TextNodes
    const OUString& rStr = pTextNd->GetText();

    sal_Unicode nDeli = pOptions->cDeli;
    sal_uInt16 nDCount = pOptions->aKeys[nId]->nColumnId, i = 1;
    sal_Int32 nStart = 0;

    // Find the delimiter
    while( nStart != -1 && i < nDCount)
        if( -1 != ( nStart = rStr.indexOf( nDeli, nStart ) ) )
        {
            nStart++;
            i++;
        }

    // Found next delimiter or end of String
    // and copy
    sal_Int32 nEnd = rStr.indexOf( nDeli, nStart+1 );
    if (nEnd == -1)
        return rStr.copy( nStart );
    return rStr.copy( nStart, nEnd-nStart );
}

/// SortingElement for Tables
SwSortBoxElement::SwSortBoxElement( sal_uInt16 nRC )
    : nRow( nRC )
{
}

SwSortBoxElement::~SwSortBoxElement()
{
}

/// Get Key for a cell
OUString SwSortBoxElement::GetKey(sal_uInt16 nKey) const
{
    const FndBox_* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         // Sort rows
    else
        pFndBox = pBox->GetBox(nRow, nCol);         // Sort columns

    // Extract the Text
    OUStringBuffer aRetStr;
    if( pFndBox )
    {   // Get StartNode and skip it
        const SwTableBox* pMyBox = pFndBox->GetBox();
        OSL_ENSURE(pMyBox, "No atomic Box");

        if( pMyBox->GetSttNd() )
        {
            // Iterate over all the Box's TextNodes
            const SwNode *pNd = nullptr, *pEndNd = pMyBox->GetSttNd()->EndOfSectionNode();
            for( sal_uLong nIdx = pMyBox->GetSttIdx() + 1; pNd != pEndNd; ++nIdx )
                if( ( pNd = pDoc->GetNodes()[ nIdx ])->IsTextNode() )
                    aRetStr.append(pNd->GetTextNode()->GetText());
        }
    }
    return aRetStr.makeStringAndClear();
}

double SwSortBoxElement::GetValue( sal_uInt16 nKey ) const
{
    const FndBox_* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         // Sort rows
    else
        pFndBox = pBox->GetBox(nRow, nCol);         // Sort columns

    double nVal;
    if( pFndBox )
    {
        const SwFormat *pFormat = pFndBox->GetBox()->GetFrameFormat();
        if (pDoc->GetNumberFormatter()->IsTextFormat( pFormat->GetTableBoxNumFormat().GetValue()))
            nVal = SwSortElement::GetValue( nKey );
        else
            nVal = pFormat->GetTableBoxValue().GetValue();
    }
    else
        nVal = 0;

    return nVal;
}

/// Sort Text in the Document
bool SwDoc::SortText(const SwPaM& rPaM, const SwSortOptions& rOpt)
{
    // Check if Frame is in the Text
    const SwPosition *pStart = rPaM.Start(), *pEnd = rPaM.End();

    // Set index to the Selection's start
    for ( const auto *pFormat : *GetSpzFrameFormats() )
    {
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();

        if (pAPos && (RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId()) &&
            pStart->nNode <= pAPos->nNode && pAPos->nNode <= pEnd->nNode )
            return false;
    }

    // Check if only TextNodes are within the Selection
    {
        sal_uLong nStart = pStart->nNode.GetIndex(),
                        nEnd = pEnd->nNode.GetIndex();
        while( nStart <= nEnd )
            // Iterate over a selected range
            if( !GetNodes()[ nStart++ ]->IsTextNode() )
                return false;
    }

    bool const bUndo = GetIDocumentUndoRedo().DoesUndo();
    if( bUndo )
    {
        GetIDocumentUndoRedo().StartUndo( SwUndoId::START, nullptr );
    }

    SwPaM* pRedlPam = nullptr;
    SwUndoRedlineSort* pRedlUndo = nullptr;
    SwUndoSort* pUndoSort = nullptr;

    // To-Do - add 'SwExtraRedlineTable' also ?
    if( getIDocumentRedlineAccess().IsRedlineOn() || (!getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        pRedlPam = new SwPaM( pStart->nNode, pEnd->nNode, -1, 1 );
        SwContentNode* pCNd = pRedlPam->GetContentNode( false );
        if( pCNd )
            pRedlPam->GetMark()->nContent = pCNd->Len();

        if( getIDocumentRedlineAccess().IsRedlineOn() && !IDocumentRedlineAccess::IsShowOriginal( getIDocumentRedlineAccess().GetRedlineFlags() ) )
        {
            if( bUndo )
            {
                pRedlUndo = new SwUndoRedlineSort( *pRedlPam,rOpt );
                GetIDocumentUndoRedo().DoUndo(false);
            }
            // First copy the range
            SwNodeIndex aEndIdx( pEnd->nNode, 1 );
            SwNodeRange aRg( pStart->nNode, aEndIdx );
            GetNodes().Copy_( aRg, aEndIdx );

            // range is new from pEnd->nNode+1 to aEndIdx
            getIDocumentRedlineAccess().DeleteRedline( *pRedlPam, true, USHRT_MAX );

            pRedlPam->GetMark()->nNode.Assign( pEnd->nNode.GetNode(), 1 );
            pCNd = pRedlPam->GetContentNode( false );
            pRedlPam->GetMark()->nContent.Assign( pCNd, 0 );

            pRedlPam->GetPoint()->nNode.Assign( aEndIdx.GetNode() );
            pCNd = pRedlPam->GetContentNode();
            sal_Int32 nCLen = 0;
            if( !pCNd &&
                nullptr != (pCNd = GetNodes()[ aEndIdx.GetIndex()-1 ]->GetContentNode()))
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
            getIDocumentRedlineAccess().DeleteRedline( *pRedlPam, true, USHRT_MAX );
            delete pRedlPam;
            pRedlPam = nullptr;
        }
    }

    SwNodeIndex aStart(pStart->nNode);
    SwSortElement::Init( this, rOpt );
    SwSortTextElements aSortSet;
    while( aStart <= pEnd->nNode )
    {
        // Iterate over a selected range
        aSortSet.insert(SwSortTextElement(aStart));
        ++aStart;
    }

    // Now comes the tricky part: Move Nodes (and always keep Undo in mind)
    sal_uLong nBeg = pStart->nNode.GetIndex();
    SwNodeRange aRg( aStart, aStart );

    if( bUndo && !pRedlUndo )
    {
        pUndoSort = new SwUndoSort(rPaM, rOpt);
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndoSort));
    }

    GetIDocumentUndoRedo().DoUndo(false);

    size_t n = 0;
    for (const auto& rElem : aSortSet)
    {
        aStart      = nBeg + n;
        aRg.aStart  = rElem.aPos.GetIndex();
        aRg.aEnd    = aRg.aStart.GetIndex() + 1;

        // Move Nodes
        getIDocumentContentOperations().MoveNodeRange( aRg, aStart,
            SwMoveFlags::DEFAULT );

        // Insert Move in Undo
        if(pUndoSort)
        {
            pUndoSort->Insert(rElem.nOrg, nBeg + n);
        }
        ++n;
    }
    // Delete all elements from the SortArray
    aSortSet.clear();
    SwSortElement::Finit();

    if( pRedlPam )
    {
        if( pRedlUndo )
        {
            pRedlUndo->SetSaveRange( *pRedlPam );
            // UGLY: temp. enable Undo
            GetIDocumentUndoRedo().DoUndo(true);
            GetIDocumentUndoRedo().AppendUndo( std::unique_ptr<SwUndo>(pRedlUndo) );
            GetIDocumentUndoRedo().DoUndo(false);
        }

        // nBeg is start of sorted range
        SwNodeIndex aSttIdx( GetNodes(), nBeg );

        // the copied range is deleted
        SwRangeRedline *const pDeleteRedline(
            new SwRangeRedline( nsRedlineType_t::REDLINE_DELETE, *pRedlPam ));

        // pRedlPam points to nodes that may be deleted (hidden) by
        // AppendRedline, so adjust it beforehand to prevent ASSERT
        pRedlPam->GetPoint()->nNode = aSttIdx;
        SwContentNode* pCNd = aSttIdx.GetNode().GetContentNode();
        pRedlPam->GetPoint()->nContent.Assign( pCNd, 0 );

        getIDocumentRedlineAccess().AppendRedline(pDeleteRedline, true);

        // the sorted range is inserted
        getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, *pRedlPam ), true);

        if( pRedlUndo )
        {
            SwNodeIndex aInsEndIdx( pRedlPam->GetMark()->nNode, -1 );
            pRedlPam->GetMark()->nNode = aInsEndIdx;
            SwContentNode *const pPrevNode =
                pRedlPam->GetMark()->nNode.GetNode().GetContentNode();
            pRedlPam->GetMark()->nContent.Assign( pPrevNode, pPrevNode->Len() );

            pRedlUndo->SetValues( *pRedlPam );
        }

        delete pRedlPam;
        pRedlPam = nullptr;
    }
    GetIDocumentUndoRedo().DoUndo( bUndo );
    if( bUndo )
    {
        GetIDocumentUndoRedo().EndUndo( SwUndoId::END, nullptr );
    }

    return true;
}

/// Sort Table in the Document
bool SwDoc::SortTable(const SwSelBoxes& rBoxes, const SwSortOptions& rOpt)
{
    // Via SwDoc for Undo!
    OSL_ENSURE( !rBoxes.empty(), "no valid Box list" );
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rBoxes[0]->GetSttNd()->FindTableNode());
    if( !pTableNd )
        return false;

    // We begin sorting
    // Find all Boxes/Lines
    FndBox_ aFndBox( nullptr, nullptr );
    {
        FndPara aPara( rBoxes, &aFndBox );
        ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
    }

    if(aFndBox.GetLines().empty())
        return false;

    if( !getIDocumentRedlineAccess().IsIgnoreRedline() && !getIDocumentRedlineAccess().GetRedlineTable().empty() )
        getIDocumentRedlineAccess().DeleteRedline( *pTableNd, true, USHRT_MAX );

    FndLines_t::size_type nStart = 0;
    if( pTableNd->GetTable().GetRowsToRepeat() > 0 && rOpt.eDirection == SRT_ROWS )
    {
        // Uppermost selected Cell
        FndLines_t& rLines = aFndBox.GetLines();

        while( nStart < rLines.size() )
        {
            // Respect Split Merge nesting,
            // extract the upper most
            SwTableLine* pLine = rLines[nStart]->GetLine();
            while ( pLine->GetUpper() )
                pLine = pLine->GetUpper()->GetUpper();

            if( pTableNd->GetTable().IsHeadline( *pLine ) )
                nStart++;
            else
                break;
        }
        // Are all selected in the HeaderLine?  -> no Offset
        if( nStart == rLines.size() )
            nStart = 0;
    }

    // Switch to relative Formulas
    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.m_eFlags = TBL_RELBOXNAME;
    getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    // Table as a flat array structure
    FlatFndBox aFlatBox(this, aFndBox);

    if(!aFlatBox.IsSymmetric())
        return false;

    // Delete HTML layout
    pTableNd->GetTable().SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());

    // #i37739# A simple 'MakeFrames' after the node sorting
    // does not work if the table is inside a frame and has no prev/next.
    SwNode2LayoutSaveUpperFrames aNode2Layout(*pTableNd);

    // Delete the Table's Frames
    pTableNd->DelFrames();
    // ? TL_CHART2: ?

    SwUndoSort* pUndoSort = nullptr;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        pUndoSort = new SwUndoSort( rBoxes[0]->GetSttIdx(),
                                    rBoxes.back()->GetSttIdx(),
                                   *pTableNd, rOpt, aFlatBox.HasItemSets() );
        GetIDocumentUndoRedo().AppendUndo(std::unique_ptr<SwUndo>(pUndoSort));
    }
    ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

    // Insert KeyElements
    sal_uInt16 nCount = (rOpt.eDirection == SRT_ROWS) ?
                    aFlatBox.GetRows() : aFlatBox.GetCols();

    // Sort SortList by Key
    SwSortElement::Init( this, rOpt, &aFlatBox );
    SwSortBoxElements aSortList;

    // When sorting, do not include the first row if the HeaderLine is repeated
    for( sal_uInt16 i = static_cast<sal_uInt16>(nStart); i < nCount; ++i)
    {
        aSortList.insert(SwSortBoxElement(i));
    }

    // Move after Sorting
    SwMovedBoxes aMovedList;
    sal_uInt16 i = 0;
    for (const auto& rElem : aSortList)
    {
        if(rOpt.eDirection == SRT_ROWS)
        {
            MoveRow(this, aFlatBox, rElem.nRow, i+nStart, aMovedList, pUndoSort);
        }
        else
        {
            MoveCol(this, aFlatBox, rElem.nRow, i+nStart, aMovedList, pUndoSort);
        }
        ++i;
    }

    // Restore table frames:
    // #i37739# A simple 'MakeFrames' after the node sorting
    // does not work if the table is inside a frame and has no prev/next.
    const sal_uLong nIdx = pTableNd->GetIndex();
    aNode2Layout.RestoreUpperFrames( GetNodes(), nIdx, nIdx + 1 );

    // TL_CHART2: need to inform chart of probably changed cell names
    UpdateCharts( pTableNd->GetTable().GetFrameFormat()->GetName() );

    // Delete all Elements in the SortArray
    aSortList.clear();
    SwSortElement::Finit();

    getIDocumentState().SetModified();
    return true;
}

/// Move a row
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for( sal_uInt16 i=0; i < rBox.GetCols(); ++i )
    {   // Get old cell position and remember it
        const FndBox_* pSource = rBox.GetBox(i, nS);

        // new cell position
        const FndBox_* pTarget = rBox.GetBox(i, nT);

        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;

        // and move it
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.push_back(pS);

        if( pS != pT )
        {
            SwFrameFormat* pTFormat = pT->GetFrameFormat();
            const SfxItemSet* pSSet = rBox.GetItemSet( i, nS );

            if( pSSet ||
                SfxItemState::SET == pTFormat->GetItemState( RES_BOXATR_FORMAT ) ||
                SfxItemState::SET == pTFormat->GetItemState( RES_BOXATR_FORMULA ) ||
                SfxItemState::SET == pTFormat->GetItemState( RES_BOXATR_VALUE ) )
            {
                pTFormat = const_cast<SwTableBox*>(pT)->ClaimFrameFormat();
                pTFormat->LockModify();
                if( pTFormat->ResetFormatAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE ) )
                    pTFormat->ResetFormatAttr( RES_VERT_ORIENT );

                if( pSSet )
                    pTFormat->SetFormatAttr( *pSSet );
                pTFormat->UnlockModify();
            }
        }
    }
}

/// Move a column
void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for(sal_uInt16 i=0; i < rBox.GetRows(); ++i)
    {   // Get old cell position and remember it
        const FndBox_* pSource = rBox.GetBox(nS, i);

        // new cell position
        const FndBox_* pTarget = rBox.GetBox(nT, i);

        // and move it
        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        // and move it
        bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.push_back(pS);

        if( pS != pT )
        {
            SwFrameFormat* pTFormat = pT->GetFrameFormat();
            const SfxItemSet* pSSet = rBox.GetItemSet( nS, i );

            if( pSSet ||
                SfxItemState::SET == pTFormat->GetItemState( RES_BOXATR_FORMAT ) ||
                SfxItemState::SET == pTFormat->GetItemState( RES_BOXATR_FORMULA ) ||
                SfxItemState::SET == pTFormat->GetItemState( RES_BOXATR_VALUE ) )
            {
                pTFormat = const_cast<SwTableBox*>(pT)->ClaimFrameFormat();
                pTFormat->LockModify();
                if( pTFormat->ResetFormatAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE ) )
                    pTFormat->ResetFormatAttr( RES_VERT_ORIENT );

                if( pSSet )
                    pTFormat->SetFormatAttr( *pSSet );
                pTFormat->UnlockModify();
            }
        }
    }
}

/// Move a single Cell
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource, const SwTableBox* pTar,
              bool bMovedBefore, SwUndoSort* pUD)
{
    OSL_ENSURE(pSource && pTar,"Source or target missing");

    if(pSource == pTar)
        return;

    if(pUD)
        pUD->Insert( pSource->GetName(), pTar->GetName() );

    // Set Pam source to the first ContentNode
    SwNodeRange aRg( *pSource->GetSttNd(), 0, *pSource->GetSttNd() );
    SwNode* pNd = pDoc->GetNodes().GoNext( &aRg.aStart );

    // If the Cell (Source) wasn't moved
    // -> insert an empty Node and move the rest or the Mark
    // points to the first ContentNode
    if( pNd->StartOfSectionNode() == pSource->GetSttNd() )
        pNd = pDoc->GetNodes().MakeTextNode( aRg.aStart,
                pDoc->GetDfltTextFormatColl() );
    aRg.aEnd = *pNd->EndOfSectionNode();

    // If the Target is empty (there is one empty Node)
    // -> move and delete it
    SwNodeIndex aTar( *pTar->GetSttNd() );
    pNd = pDoc->GetNodes().GoNext( &aTar );     // next ContentNode
    sal_uLong nCount = pNd->EndOfSectionIndex() - pNd->StartOfSectionIndex();

    bool bDelFirst = false;
    if( nCount == 2 )
    {
        OSL_ENSURE( pNd->GetContentNode(), "No ContentNode");
        bDelFirst = !pNd->GetContentNode()->Len() && bMovedBefore;
    }

    if(!bDelFirst)
    {   // We already have Content -> old Content Section Down
        SwNodeRange aRgTar( aTar.GetNode(), 0, *pNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRgTar );
    }

    // Insert the Source
    SwNodeIndex aIns( *pTar->GetSttNd()->EndOfSectionNode() );
    pDoc->getIDocumentContentOperations().MoveNodeRange( aRg, aIns,
        SwMoveFlags::DEFAULT );

    // If first Node is empty -> delete it
    if(bDelFirst)
        pDoc->GetNodes().Delete( aTar );
}

/// Generate two-dimensional array of FndBoxes
FlatFndBox::FlatFndBox(SwDoc* pDocPtr, const FndBox_& rBoxRef) :
    pDoc(pDocPtr),
    nRow(0),
    nCol(0)
{ // If the array is symmetric
    bSym = CheckLineSymmetry(rBoxRef);
    if( bSym )
    {
        // Determine column/row count
        nCols = GetColCount(rBoxRef);
        nRows = GetRowCount(rBoxRef);

        // Create linear array
        size_t nCount = static_cast<size_t>(nRows) * nCols;
        pArr = std::make_unique<FndBox_ const *[]>(nCount);
        memset(pArr.get(), 0, sizeof(const FndBox_*) * nCount);

        FillFlat( rBoxRef );
    }
}

FlatFndBox::~FlatFndBox()
{
}

/// All Lines of a Box need to have same number of Boxes
bool FlatFndBox::CheckLineSymmetry(const FndBox_& rBox)
{
    const FndLines_t &rLines = rBox.GetLines();
    FndBoxes_t::size_type nBoxes {0};

    for (FndLines_t::size_type i=0; i < rLines.size(); ++i)
    {
        const FndLine_* pLn = rLines[i].get();
        const FndBoxes_t& rBoxes = pLn->GetBoxes();

        // Number of Boxes of all Lines is unequal -> no symmetry
        if( i  && nBoxes != rBoxes.size())
            return false;

        nBoxes = rBoxes.size();
        if( !CheckBoxSymmetry( *pLn ) )
            return false;
    }
    return true;
}

/// Check Box for symmetry (All Boxes of a Line need to have same number of Lines)
bool FlatFndBox::CheckBoxSymmetry(const FndLine_& rLn)
{
    const FndBoxes_t &rBoxes = rLn.GetBoxes();
    FndLines_t::size_type nLines {0};

    for (FndBoxes_t::size_type i = 0; i < rBoxes.size(); ++i)
    {
        FndBox_ const*const pBox = rBoxes[i].get();
        const FndLines_t& rLines = pBox->GetLines();

        // Number of Lines of all Boxes is unequal -> no symmetry
        if( i && nLines != rLines.size() )
            return false;

        nLines = rLines.size();
        if( nLines && !CheckLineSymmetry( *pBox ) )
            return false;
    }
    return true;
}

/// Maximum count of Columns (Boxes)
sal_uInt16 FlatFndBox::GetColCount(const FndBox_& rBox)
{
    const FndLines_t& rLines = rBox.GetLines();
    // Iterate over Lines
    if( rLines.empty() )
        return 1;

    sal_uInt16 nSum = 0;
    for (const auto & pLine : rLines)
    {
        // The Boxes of a Line
        sal_uInt16 nCount = 0;
        const FndBoxes_t& rBoxes = pLine->GetBoxes();
        for (const auto &rpB : rBoxes)
        {   // Iterate recursively over the Lines
            nCount += rpB->GetLines().empty() ? 1 : GetColCount(*rpB);
        }

        if( nSum < nCount )
            nSum = nCount;
    }
    return nSum;
}

/// Maximum count of Rows (Lines)
sal_uInt16 FlatFndBox::GetRowCount(const FndBox_& rBox)
{
    const FndLines_t& rLines = rBox.GetLines();
    if( rLines.empty() )
        return 1;

    sal_uInt16 nLines = 0;
    for (const auto & pLine : rLines)
    {   // The Boxes of a Line
        const FndBoxes_t& rBoxes = pLine->GetBoxes();
        sal_uInt16 nLn = 1;
        for (const auto &rpB : rBoxes)
        {
            if (!rpB->GetLines().empty())
            {   // Iterate recursively over the Lines
                nLn = std::max(GetRowCount(*rpB), nLn);
            }
        }

        nLines = nLines + nLn;
    }
    return nLines;
}

/// Create a linear array of atomic FndBoxes
void FlatFndBox::FillFlat(const FndBox_& rBox, bool bLastBox)
{
    bool bModRow = false;
    const FndLines_t& rLines = rBox.GetLines();

    // Iterate over Lines
    sal_uInt16 nOldRow = nRow;
    for (const auto & pLine : rLines)
    {
        // The Boxes of a Line
        const FndBoxes_t& rBoxes = pLine->GetBoxes();
        sal_uInt16 nOldCol = nCol;
        for( FndBoxes_t::size_type j = 0; j < rBoxes.size(); ++j )
        {
            // Check the Box if it's an atomic one
            const FndBox_ *const pBox = rBoxes[j].get();

            if( pBox->GetLines().empty() )
            {
                // save it
                sal_uInt16 nOff = nRow * nCols + nCol;
                pArr[nOff] = pBox;

                // Save the Formula/Format/Value values
                const SwFrameFormat* pFormat = pBox->GetBox()->GetFrameFormat();
                if( SfxItemState::SET == pFormat->GetItemState( RES_BOXATR_FORMAT ) ||
                    SfxItemState::SET == pFormat->GetItemState( RES_BOXATR_FORMULA ) ||
                    SfxItemState::SET == pFormat->GetItemState( RES_BOXATR_VALUE ) )
                {
                    auto pSet = std::make_unique<SfxItemSet>(
                        pDoc->GetAttrPool(),
                        svl::Items<
                            RES_VERT_ORIENT, RES_VERT_ORIENT,
                            RES_BOXATR_FORMAT, RES_BOXATR_VALUE>{});
                    pSet->Put( pFormat->GetAttrSet() );
                    if( ppItemSets.empty() )
                    {
                        size_t nCount = static_cast<size_t>(nRows) * nCols;
                        ppItemSets.resize(nCount);
                    }
                    ppItemSets[nOff] = std::move(pSet);
                }

                bModRow = true;
            }
            else
            {
                // Iterate recursively over the Lines of a Box
                FillFlat( *pBox, ( j+1 == rBoxes.size() ) );
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

/// Access a specific Cell
const FndBox_* FlatFndBox::GetBox(sal_uInt16 n_Col, sal_uInt16 n_Row) const
{
    sal_uInt16 nOff = n_Row * nCols + n_Col;
    const FndBox_* pTmp = pArr[nOff];

    OSL_ENSURE(n_Col < nCols && n_Row < nRows && pTmp, "invalid array access");
    return pTmp;
}

const SfxItemSet* FlatFndBox::GetItemSet(sal_uInt16 n_Col, sal_uInt16 n_Row) const
{
    OSL_ENSURE( ppItemSets.empty() || ( n_Col < nCols && n_Row < nRows), "invalid array access");

    return !ppItemSets.empty() ? ppItemSets[unsigned(n_Row * nCols) + n_Col].get() : nullptr;
}

sal_uInt16 SwMovedBoxes::GetPos(const SwTableBox* pTableBox) const
{
    std::vector<const SwTableBox*>::const_iterator it = std::find(mBoxes.begin(), mBoxes.end(), pTableBox);
    return it == mBoxes.end() ? USHRT_MAX : it - mBoxes.begin();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
