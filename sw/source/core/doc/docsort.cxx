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
lang::Locale*             SwSortElement::pLocale = 0;
String*             SwSortElement::pLastAlgorithm = 0;
LocaleDataWrapper*  SwSortElement::pLclData = 0;

SV_IMPL_OP_PTRARR_SORT( SwSortElements, SwSortElementPtr );

/*--------------------------------------------------------------------
    Description: Construct a SortElement for the Sort
 --------------------------------------------------------------------*/
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
        nLang = (LanguageType)GetAppLanguage();
        break;
    }
    pLocale = new lang::Locale( SvxCreateLocale( nLang ) );

    pSortCollator = new CollatorWrapper(
                                ::comphelper::getProcessServiceFactory() );
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

double SwSortElement::StrToDouble( const String& rStr ) const
{
    if( !pLclData )
        pLclData = new LocaleDataWrapper(
                    ::comphelper::getProcessServiceFactory(), *pLocale );

    rtl_math_ConversionStatus eStatus;
    sal_Int32 nEnd;
    double nRet = ::rtl::math::stringToDouble( rStr,
                                    pLclData->getNumDecimalSep().GetChar(0),
                                    pLclData->getNumThousandSep().GetChar(0),
                                    &eStatus, &nEnd );

    if( rtl_math_ConversionStatus_Ok != eStatus || nEnd == 0 )
        nRet = 0.0;
    return nRet;
}

/*--------------------------------------------------------------------
    Description: Comparison operators
 --------------------------------------------------------------------*/
sal_Bool SwSortElement::operator==(const SwSortElement& )
{
    return sal_False;
}

/*--------------------------------------------------------------------
    Description: Less-than operator for sorting
 --------------------------------------------------------------------*/
sal_Bool SwSortElement::operator<(const SwSortElement& rCmp)
{

    // The actual comparison
    for(sal_uInt16 nKey = 0; nKey < pOptions->aKeys.Count(); ++nKey)
    {
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

            if( n1 == n2 )
                continue;

            return n1 < n2;
        }
        else
        {
            if( !pLastAlgorithm || *pLastAlgorithm != pSrtKey->sSortType )
            {
                if( pLastAlgorithm )
                    *pLastAlgorithm = pSrtKey->sSortType;
                else
                    pLastAlgorithm = new String( pSrtKey->sSortType );
                pSortCollator->loadCollatorAlgorithm( *pLastAlgorithm,
                        *pLocale,
                        pOptions->bIgnoreCase ? SW_COLLATOR_IGNORES : 0 );
            }

            sal_Int32 nCmp = pSortCollator->compareString(
                        pOrig->GetKey( nKey ), pCmp->GetKey( nKey ));
            if( 0 == nCmp )
                continue;

            return -1 == nCmp;
        }
    }
    return sal_False;
}

double SwSortElement::GetValue( sal_uInt16 nKey ) const
{
    return StrToDouble( GetKey( nKey ));
}

/*--------------------------------------------------------------------
    Description: SortingElement for Text
 --------------------------------------------------------------------*/
SwSortTxtElement::SwSortTxtElement(const SwNodeIndex& rPos)
    : nOrg(rPos.GetIndex()), aPos(rPos)
{
}

SwSortTxtElement::~SwSortTxtElement()
{
}

/*--------------------------------------------------------------------
    Description: Get Key
 --------------------------------------------------------------------*/
String SwSortTxtElement::GetKey(sal_uInt16 nId) const
{
    SwTxtNode* pTxtNd = aPos.GetNode().GetTxtNode();
    if( !pTxtNd )
        return aEmptyStr;

    // for TextNodes
    const String& rStr = pTxtNd->GetTxt();

    sal_Unicode nDeli = pOptions->cDeli;
    sal_uInt16 nDCount = pOptions->aKeys[nId]->nColumnId, i = 1;
    xub_StrLen nStart = 0;

    // Find the delimiter
    while( nStart != STRING_NOTFOUND && i < nDCount)
        if( STRING_NOTFOUND != ( nStart = rStr.Search( nDeli, nStart ) ) )
        {
            nStart++;
            i++;
        }

    // Found next delimiter or end of String
    // and copy
    xub_StrLen nEnd = rStr.Search( nDeli, nStart+1 );
    return rStr.Copy( nStart, nEnd-nStart );
}

/*--------------------------------------------------------------------
    Description: SortingElement for Tables
 --------------------------------------------------------------------*/
SwSortBoxElement::SwSortBoxElement( sal_uInt16 nRC )
    : nRow( nRC )
{
}

SwSortBoxElement::~SwSortBoxElement()
{
}

/*--------------------------------------------------------------------
    Description: Get Key for a cell
 --------------------------------------------------------------------*/
String SwSortBoxElement::GetKey(sal_uInt16 nKey) const
{
    const _FndBox* pFndBox;
    sal_uInt16 nCol = pOptions->aKeys[nKey]->nColumnId-1;

    if( SRT_ROWS == pOptions->eDirection )
        pFndBox = pBox->GetBox(nCol, nRow);         // Sort rows
    else
        pFndBox = pBox->GetBox(nRow, nCol);         // Sort columns

    // Extract the Text
    String aRetStr;
    if( pFndBox )
    {   // Get StartNode and skip it
        const SwTableBox* pMyBox = pFndBox->GetBox();
        OSL_ENSURE(pMyBox, "No atomic Box");

        if( pMyBox->GetSttNd() )
        {
            // Iterate over all the Box's TextNodes
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
        pFndBox = pBox->GetBox(nCol, nRow);         // Sort rows
    else
        pFndBox = pBox->GetBox(nRow, nCol);         // Sort columns


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

/*--------------------------------------------------------------------
    Description: Sort Text in the Document
 --------------------------------------------------------------------*/
/*--------------------------------------------------------------------
    Description: Move a row
 --------------------------------------------------------------------*/
void MoveRow(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for( sal_uInt16 i=0; i < rBox.GetCols(); ++i )
    {   // Get old cell position and remember it
        const _FndBox* pSource = rBox.GetBox(i, nS);

        // new cell position
        const _FndBox* pTarget = rBox.GetBox(i, nT);

        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        sal_Bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;

        // and move it
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.Insert(pS, rMovedList.Count() );

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

/*--------------------------------------------------------------------
    Description: Move a column
 --------------------------------------------------------------------*/
void MoveCol(SwDoc* pDoc, const FlatFndBox& rBox, sal_uInt16 nS, sal_uInt16 nT,
             SwMovedBoxes& rMovedList, SwUndoSort* pUD)
{
    for(sal_uInt16 i=0; i < rBox.GetRows(); ++i)
    {   // Get old cell position and remember it
        const _FndBox* pSource = rBox.GetBox(nS, i);

        // new cell position
        const _FndBox* pTarget = rBox.GetBox(nT, i);

        // and move it
        const SwTableBox* pT = pTarget->GetBox();
        const SwTableBox* pS = pSource->GetBox();

        // and move it
        sal_Bool bMoved = rMovedList.GetPos(pT) != USHRT_MAX;
        MoveCell(pDoc, pS, pT, bMoved, pUD);

        rMovedList.Insert(pS, rMovedList.Count() );

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

/*--------------------------------------------------------------------
    Description: Move a single Cell
 --------------------------------------------------------------------*/
void MoveCell(SwDoc* pDoc, const SwTableBox* pSource, const SwTableBox* pTar,
              sal_Bool bMovedBefore, SwUndoSort* pUD)
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
        pNd = pDoc->GetNodes().MakeTxtNode( aRg.aStart,
                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    aRg.aEnd = *pNd->EndOfSectionNode();

    // If the Target is empty (there is one empty Node)
    // -> delete it and move the Target
    SwNodeIndex aTar( *pTar->GetSttNd() );
    pNd = pDoc->GetNodes().GoNext( &aTar );     // next ContentNode
    sal_uLong nCount = pNd->EndOfSectionIndex() - pNd->StartOfSectionIndex();

    sal_Bool bDelFirst = sal_False;
    if( nCount == 2 )
    {
        OSL_ENSURE( pNd->GetCntntNode(), "No ContentNode");
        bDelFirst = !pNd->GetCntntNode()->Len() && bMovedBefore;
    }

    if(!bDelFirst)
    {   // We already have Content -> old Content Section Down
        SwNodeRange aRgTar( aTar.GetNode(), 0, *pNd->EndOfSectionNode() );
        pDoc->GetNodes().SectionDown( &aRgTar );
    }

    // Insert the Source
    SwNodeIndex aIns( *pTar->GetSttNd()->EndOfSectionNode() );
    pDoc->MoveNodeRange( aRg, aIns,
        IDocumentContentOperations::DOC_MOVEDEFAULT );

    // If first Node is empty -> delete it
    if(bDelFirst)
        pDoc->GetNodes().Delete( aTar, 1 );
}

/*--------------------------------------------------------------------
    Description: Generate two-dimensional array of FndBoxes
 --------------------------------------------------------------------*/
FlatFndBox::FlatFndBox(SwDoc* pDocPtr, const _FndBox& rBox) :
    pDoc(pDocPtr),
    rBoxRef(rBox),
    pArr(0),
    ppItemSets(0),
    nRow(0),
    nCol(0)
{ // If the array is symmetric
    if((bSym = CheckLineSymmetry(rBoxRef)) != 0)
    {
        // Determine column/row count
        nCols = GetColCount(rBoxRef);
        nRows = GetRowCount(rBoxRef);

        // Create linear array
        pArr = new _FndBoxPtr[ nRows * nCols ];
        _FndBox** ppTmp = (_FndBox**)pArr;
        memset( ppTmp, 0, sizeof(_FndBoxPtr) * nRows * nCols );


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

/*--------------------------------------------------------------------
    Description: All Lines of a Box need to have as many Boxes
 --------------------------------------------------------------------*/
sal_Bool FlatFndBox::CheckLineSymmetry(const _FndBox& rBox)
{
    const _FndLines &rLines = rBox.GetLines();
    sal_uInt16 nBoxes(0);

    // Iterate over Lines
    for(sal_uInt16 i=0; i < rLines.Count(); ++i)
    {   // A List's Box
        _FndLine* pLn = rLines[i];
        const _FndBoxes& rBoxes = pLn->GetBoxes();

        // Amount of Boxes of all Lines is uneven -> no symmetry
        if( i  && nBoxes != rBoxes.Count())
            return sal_False;

        nBoxes = rBoxes.Count();
        if( !CheckBoxSymmetry( *pLn ) )
            return sal_False;
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Description: Check Box for symmetry
                 All Boxes of a Line need to have as many Lines
 --------------------------------------------------------------------*/
sal_Bool FlatFndBox::CheckBoxSymmetry(const _FndLine& rLn)
{
    const _FndBoxes &rBoxes = rLn.GetBoxes();
    sal_uInt16 nLines(0);

    // Iterate over Boxes
    for(sal_uInt16 i=0; i < rBoxes.Count(); ++i)
    {   // The Boxes of a Line
        _FndBox* pBox = rBoxes[i];
        const _FndLines& rLines = pBox->GetLines();

        // Amount of Boxes of all Lines is uneven -> no symmetry
        if( i && nLines != rLines.Count() )
            return sal_False;

        nLines = rLines.Count();
        if( nLines && !CheckLineSymmetry( *pBox ) )
            return sal_False;
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Description: Maximum count of Columns (Boxes)
 --------------------------------------------------------------------*/
sal_uInt16 FlatFndBox::GetColCount(const _FndBox& rBox)
{
    const _FndLines& rLines = rBox.GetLines();
    // Iterate over Lines
    if( !rLines.Count() )
        return 1;

    sal_uInt16 nSum = 0;
    for( sal_uInt16 i=0; i < rLines.Count(); ++i )
    {
        // The Boxes of a Line
        sal_uInt16 nCount = 0;
        const _FndBoxes& rBoxes = rLines[i]->GetBoxes();
        for( sal_uInt16 j=0; j < rBoxes.Count(); ++j )
            // Iterate recursively over the Lines
            nCount += rBoxes[j]->GetLines().Count()
                        ? GetColCount(*rBoxes[j]) : 1;

        if( nSum < nCount )
            nSum = nCount;
    }
    return nSum;
}

/*--------------------------------------------------------------------
    Description: Maximum count of Rows (Lines)
 --------------------------------------------------------------------*/
sal_uInt16 FlatFndBox::GetRowCount(const _FndBox& rBox)
{
    const _FndLines& rLines = rBox.GetLines();
    if( !rLines.Count() )
        return 1;

    sal_uInt16 nLines = 0;
    for(sal_uInt16 i=0; i < rLines.Count(); ++i)
    {   // The Boxes of a Line
        const _FndBoxes& rBoxes = rLines[i]->GetBoxes();
        sal_uInt16 nLn = 1;
        for(sal_uInt16 j=0; j < rBoxes.Count(); ++j)
            if( rBoxes[j]->GetLines().Count() )
                // Iterate recursively over the Lines
                nLn = Max(GetRowCount(*rBoxes[j]), nLn);

        nLines = nLines + nLn;
    }
    return nLines;
}

/*--------------------------------------------------------------------
    Description: Create a linear array of atmoic FndBoxes
 --------------------------------------------------------------------*/
void FlatFndBox::FillFlat(const _FndBox& rBox, sal_Bool bLastBox)
{
    sal_Bool bModRow = sal_False;
    const _FndLines& rLines = rBox.GetLines();

    // Iterate over Lines
    sal_uInt16 nOldRow = nRow;
    for( sal_uInt16 i=0; i < rLines.Count(); ++i )
    {
        // The Boxes of a Line
        const _FndBoxes& rBoxes = rLines[i]->GetBoxes();
        sal_uInt16 nOldCol = nCol;
        for( sal_uInt16 j = 0; j < rBoxes.Count(); ++j )
        {
            // Check the Box if it's an atomic one
            const _FndBox*   pBox   = rBoxes[ j ];

            if( !pBox->GetLines().Count() )
            {
                // save it
                sal_uInt16 nOff = nRow * nCols + nCol;
                *(pArr + nOff) = pBox;

                // Save the Formula/Format/Value values
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

                bModRow = sal_True;
            }
            else
            {
                // Iterate recursively over the Lines of a Box
                FillFlat( *pBox, ( j == rBoxes.Count()-1 ) );
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

/*--------------------------------------------------------------------
    Description: Access a specific Cell
 --------------------------------------------------------------------*/
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
