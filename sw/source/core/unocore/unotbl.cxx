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

#include <list>
#include <utility>
#include <vector>

#include <svx/svxids.hrc>
#include <editeng/memberids.hrc>
#include <float.h>
#include <swtypes.hxx>
#include <cmdid.h>
#include <unotbl.hxx>
#include <unostyle.hxx>
#include <section.hxx>
#include <unocrsr.hxx>
#include <svx/unomid.hxx>
#include <hints.hxx>
#include <swtblfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <shellres.hxx>
#include <docary.hxx>
#include <ndole.hxx>
#include <frame.hxx>
#include <vcl/svapp.hxx>
#include <fmtfsize.hxx>
#include <tblafmt.hxx>
#include <tabcol.hxx>
#include <cellatr.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <viewsh.hxx>
#include <tabfrm.hxx>
#include <redline.hxx>
#include <unoport.hxx>
#include <unoprnms.hxx>
#include <unocrsrhelper.hxx>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/chart/XChartDataChangeEventListener.hpp>
#include <com/sun/star/chart/ChartDataChangeEvent.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unoparagraph.hxx>
#include <svl/zforlist.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <fmtornt.hxx>
#include <editeng/keepitem.hxx>
#include <fmtlsplt.hxx>
#include <swundo.hxx>
#include <osl/mutex.hxx>
#include <SwStyleNameMapper.hxx>
#include <frmatr.hxx>
#include <crsskip.hxx>
#include <unochart.hxx>
#include <sortopt.hxx>
#include <rtl/math.hxx>
#include <editeng/frmdiritem.hxx>
#include <switerator.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <swtable.hxx>

using namespace ::com::sun::star;
using ::editeng::SvxBorderLine;

#define UNO_TABLE_COLUMN_SUM    10000

static void lcl_SendChartEvent(::cppu::OWeakObject & rSource,
                               ::cppu::OInterfaceContainerHelper & rListeners)
{
    //TODO: find appropriate settings of the Event
    chart::ChartDataChangeEvent event;
    event.Source = & rSource;
    event.Type = chart::ChartDataChangeType_ALL;
    event.StartColumn = 0;
    event.EndColumn = 1;
    event.StartRow = 0;
    event.EndRow = 1;
    rListeners.notifyEach(
            & chart::XChartDataChangeEventListener::chartDataChanged, event);
}

static void lcl_SendChartEvent(::cppu::OWeakObject & rSource,
                               ::cppu::OMultiTypeInterfaceContainerHelper & rListeners)
{
    ::cppu::OInterfaceContainerHelper *const pContainer(rListeners.getContainer(
            cppu::UnoType<chart::XChartDataChangeEventListener>::get()));
    if (pContainer)
    {
        lcl_SendChartEvent(rSource, *pContainer);
    }
}

static bool lcl_LineToSvxLine(const table::BorderLine& rLine, SvxBorderLine& rSvxLine)
{
    rSvxLine.SetColor(Color(rLine.Color));

    rSvxLine.GuessLinesWidths( table::BorderLineStyle::NONE,
                                convertMm100ToTwip( rLine.OuterLineWidth ),
                                convertMm100ToTwip( rLine.InnerLineWidth ),
                                convertMm100ToTwip( rLine.LineDistance ) );

    bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
    return bRet;
}

static void lcl_SetSpecialProperty(SwFrmFmt* pFmt,
                                   const SfxItemPropertySimpleEntry* pEntry,
                                   const uno::Any& aValue)
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    // special treatment for "non-items"
    switch(pEntry->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        case  FN_TABLE_HEADLINE_COUNT:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            {
                UnoActionContext aAction(pFmt->GetDoc());
                if( pEntry->nWID == FN_TABLE_HEADLINE_REPEAT)
                {
                    bool bVal = *(sal_Bool*)aValue.getValue();
                    pFmt->GetDoc()->SetRowsToRepeat( *pTable, bVal ? 1 : 0 );
                }
                else
                {
                    sal_Int32 nRepeat = 0;
                    aValue >>= nRepeat;
                    if( nRepeat >= 0 && nRepeat < USHRT_MAX )
                        pFmt->GetDoc()->SetRowsToRepeat( *pTable, (sal_uInt16) nRepeat );
                }
            }
        }
        break;

        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            SwFmtFrmSize aSz( pFmt->GetFrmSize() );
            if(FN_TABLE_WIDTH == pEntry->nWID)
            {
                sal_Int32 nWidth = 0;
                aValue >>= nWidth;
                aSz.SetWidthPercent(0);
                aSz.SetWidth ( convertMm100ToTwip ( nWidth ) );
            }
            else if(FN_TABLE_RELATIVE_WIDTH == pEntry->nWID)
            {
                sal_Int16 nSet = 0;
                aValue >>= nSet;
                if(nSet && nSet <=100)
                    aSz.SetWidthPercent( (sal_uInt8)nSet );
            }
            else if(FN_TABLE_IS_RELATIVE_WIDTH == pEntry->nWID)
            {
                bool bPercent = *(sal_Bool*)aValue.getValue();
                if(!bPercent)
                    aSz.SetWidthPercent(0);
                else
                {
                    lang::IllegalArgumentException aExcept;
                    aExcept.Message = "relative width cannot be switched on with this property";
                    throw aExcept;
                }
            }
            pFmt->GetDoc()->SetAttr(aSz, *pFmt);
        }
        break;

        case RES_PAGEDESC:
        {
            OUString sPageStyle;
            aValue >>= sPageStyle;
            const SwPageDesc* pDesc = 0;
            if (!sPageStyle.isEmpty())
            {
                SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
                pDesc = SwPageDesc::GetByName(*pFmt->GetDoc(), sPageStyle);
            }
            SwFmtPageDesc aDesc( pDesc );
            pFmt->GetDoc()->SetAttr(aDesc, *pFmt);
        }
        break;

        default:
            throw lang::IllegalArgumentException();
    }
}

static uno::Any lcl_GetSpecialProperty(SwFrmFmt* pFmt, const SfxItemPropertySimpleEntry* pEntry )
{
    uno::Any aRet;
    switch(pEntry->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        case  FN_TABLE_HEADLINE_COUNT:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            const sal_uInt16 nRepeat = pTable->GetRowsToRepeat();
            if(pEntry->nWID == FN_TABLE_HEADLINE_REPEAT)
            {
                aRet <<= nRepeat > 0;
            }
            else
                aRet <<= (sal_Int32)nRepeat;
        }
        break;

        case  FN_TABLE_WIDTH:
        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            const SwFmtFrmSize& rSz = pFmt->GetFrmSize();
            if(FN_TABLE_WIDTH == pEntry->nWID)
                rSz.QueryValue(aRet, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            else if(FN_TABLE_RELATIVE_WIDTH == pEntry->nWID)
                rSz.QueryValue(aRet, MID_FRMSIZE_REL_WIDTH);
            else
            {
                aRet <<= 0 != rSz.GetWidthPercent();
            }
        }
        break;

        case RES_PAGEDESC:
        {
            const SfxItemSet& rSet = pFmt->GetAttrSet();
            const SfxPoolItem* pItem;
            OUString sPDesc;
            if(SfxItemState::SET == rSet.GetItemState(RES_PAGEDESC, false, &pItem))
            {
                const SwPageDesc* pDsc = static_cast<const SwFmtPageDesc*>(pItem)->GetPageDesc();
                if(pDsc)
                {
                   sPDesc = SwStyleNameMapper::GetProgName(pDsc->GetName(), nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
                }
            }
            aRet <<= sPDesc;
        }
        break;

        case RES_ANCHOR :
            aRet <<= text::TextContentAnchorType_AT_PARAGRAPH;
        break;

        case FN_UNO_ANCHOR_TYPES :
        {
            uno::Sequence<text::TextContentAnchorType> aTypes(1);
             text::TextContentAnchorType* pArray = aTypes.getArray();
            pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
            aRet <<= aTypes;
        }
        break;

        case FN_UNO_WRAP :
        {
            aRet <<= text::WrapTextMode_NONE;
        }
        break;

        case FN_PARAM_LINK_DISPLAY_NAME :
            aRet <<= pFmt->GetName();
        break;

        case FN_UNO_REDLINE_NODE_START:
        case FN_UNO_REDLINE_NODE_END:
        {
            SwTable* pTable = SwTable::FindTable( pFmt );
            SwNode* pTblNode = pTable->GetTableNode();
            if(FN_UNO_REDLINE_NODE_END == pEntry->nWID)
                pTblNode = pTblNode->EndOfSectionNode();
            const SwRedlineTbl& rRedTbl = pFmt->GetDoc()->getIDocumentRedlineAccess().GetRedlineTbl();
            for(size_t nRed = 0; nRed < rRedTbl.size(); ++nRed)
            {
                const SwRangeRedline* pRedline = rRedTbl[nRed];
                const SwNode& rRedPointNode = pRedline->GetNode(true);
                const SwNode& rRedMarkNode = pRedline->GetNode(false);
                if(&rRedPointNode == pTblNode || &rRedMarkNode == pTblNode)
                {
                    const SwNode& rStartOfRedline = SwNodeIndex(rRedPointNode) <= SwNodeIndex(rRedMarkNode) ?
                        rRedPointNode : rRedMarkNode;
                    bool bIsStart = &rStartOfRedline == pTblNode;
                    aRet <<= SwXRedlinePortion::CreateRedlineProperties(*pRedline, bIsStart);
                    break;
                }
            }
        }
        break;
    }
    return aRet;
}

/** get position of a cell with a given name
 *
 * If everything was OK, the indices for column and row are changed (both >= 0).
 * In case of errors, at least one of them is < 0.
 *
 * Also since the implementations of tables does not really have columns using
 * this function is appropriate only for tables that are not complex (i.e.
 * where IsTblComplex() returns false).
 *
 * @param rCellName e.g. A1..Z1, a1..z1, AA1..AZ1, Aa1..Az1, BA1..BZ1, Ba1..Bz1, ...
 * @param [IN,OUT] rColumn (0-based)
 * @param [IN,OUT] rRow (0-based)
 */
//TODO: potential for throwing proper exceptions instead of having every caller to check for errors
void sw_GetCellPosition(const OUString &rCellName,
                        sal_Int32 &rColumn, sal_Int32 &rRow)
{
    rColumn = rRow = -1;    // default return values indicating failure
    const sal_Int32 nLen = rCellName.getLength();
    if (nLen)
    {
        sal_Int32 nRowPos = 0;
        while (nRowPos<nLen)
        {
            if (rCellName[nRowPos]>='0' && rCellName[nRowPos]<='9')
            {
                break;
            }
            ++nRowPos;
        }
        if (nRowPos>0 && nRowPos<nLen)
        {
            sal_Int32 nColIdx = 0;
            for (sal_Int32 i = 0;  i < nRowPos;  ++i)
            {
                nColIdx *= 52;
                if (i < nRowPos - 1)
                    ++nColIdx;
                const sal_Unicode cChar = rCellName[i];
                if ('A' <= cChar && cChar <= 'Z')
                    nColIdx += cChar - 'A';
                else if ('a' <= cChar && cChar <= 'z')
                    nColIdx += 26 + cChar - 'a';
                else
                {
                    nColIdx = -1;   // sth failed
                    break;
                }
            }

            rColumn = nColIdx;
            rRow    = rCellName.copy(nRowPos).toInt32() - 1; // - 1 because indices ought to be 0 based
        }
    }
    OSL_ENSURE( rColumn >= 0 && rRow >= 0, "failed to get column or row index" );
}

/** compare position of two cells (check rows first)
 *
 * @note this function probably also make sense only
 *       for cell names of non-complex tables
 *
 * @param rCellName1 e.g. "A1" (non-empty string with valid cell name)
 * @param rCellName2 e.g. "A1" (non-empty string with valid cell name)
 * @return -1 if cell_1 < cell_2; 0 if both cells are equal; +1 if cell_1 > cell_2
 */
int sw_CompareCellsByRowFirst( const OUString &rCellName1, const OUString &rCellName2 )
{
    sal_Int32 nCol1 = -1, nRow1 = -1, nCol2 = -1, nRow2 = -1;
    sw_GetCellPosition( rCellName1, nCol1, nRow1 );
    sw_GetCellPosition( rCellName2, nCol2, nRow2 );

    if (nRow1 < nRow2 || (nRow1 == nRow2 && nCol1 < nCol2))
        return -1;
    else if (nCol1 == nCol2 && nRow1 == nRow2)
        return 0;
    else
        return +1;
}

/** compare position of two cells (check columns first)
 *
 * @note this function probably also make sense only
 *       for cell names of non-complex tables
 *
 * @param rCellName1 e.g. "A1" (non-empty string with valid cell name)
 * @param rCellName2 e.g. "A1" (non-empty string with valid cell name)
 * @return -1 if cell_1 < cell_2; 0 if both cells are equal; +1 if cell_1 > cell_2
 */
int sw_CompareCellsByColFirst( const OUString &rCellName1, const OUString &rCellName2 )
{
    sal_Int32 nCol1 = -1, nRow1 = -1, nCol2 = -1, nRow2 = -1;
    sw_GetCellPosition( rCellName1, nCol1, nRow1 );
    sw_GetCellPosition( rCellName2, nCol2, nRow2 );

    if (nCol1 < nCol2 || (nCol1 == nCol2 && nRow1 < nRow2))
        return -1;
    else if (nRow1 == nRow2 && nCol1 == nCol2)
        return 0;
    else
        return +1;
}

/** compare position of two cell ranges
 *
 * @note this function probably also make sense only
 *       for cell names of non-complex tables
 *
 * @param rRange1StartCell e.g. "A1" (non-empty string with valid cell name)
 * @param rRange1EndCell   e.g. "A1" (non-empty string with valid cell name)
 * @param rRange2StartCell e.g. "A1" (non-empty string with valid cell name)
 * @param rRange2EndCell   e.g. "A1" (non-empty string with valid cell name)
 * @param bCmpColsFirst    if <true> position in columns will be compared first before rows
 *
 * @return -1 if cell_range_1 < cell_range_2; 0 if both cell ranges are equal; +1 if cell_range_1 > cell_range_2
 */
int sw_CompareCellRanges(
        const OUString &rRange1StartCell, const OUString &rRange1EndCell,
        const OUString &rRange2StartCell, const OUString &rRange2EndCell,
        bool bCmpColsFirst )
{
    int (*pCompareCells)( const OUString &, const OUString & ) =
            bCmpColsFirst ? &sw_CompareCellsByColFirst : &sw_CompareCellsByRowFirst;

    int nCmpResStartCells = pCompareCells( rRange1StartCell, rRange2StartCell );
    if ((-1 == nCmpResStartCells ) ||
         ( 0 == nCmpResStartCells &&
          -1 == pCompareCells( rRange1EndCell, rRange2EndCell ) ))
        return -1;
    else if (0 == nCmpResStartCells &&
             0 == pCompareCells( rRange1EndCell, rRange2EndCell ))
        return 0;
    else
        return +1;
}

/** get cell name at a specified coordinate
 *
 * @param nColumn column index (0-based)
 * @param nRow row index (0-based)
 * @return the cell name
 */
OUString sw_GetCellName( sal_Int32 nColumn, sal_Int32 nRow )
{
#if OSL_DEBUG_LEVEL > 0
    {
        sal_Int32 nCol, nRow2;
        sw_GetCellPosition( OUString("z1"), nCol, nRow2);
        OSL_ENSURE( nCol == 51, "sw_GetCellPosition failed" );
        sw_GetCellPosition( OUString("AA1"), nCol, nRow2);
        OSL_ENSURE( nCol == 52, "sw_GetCellPosition failed" );
        sw_GetCellPosition( OUString("AB1"), nCol, nRow2);
        OSL_ENSURE( nCol == 53, "sw_GetCellPosition failed" );
        sw_GetCellPosition( OUString("BB1"), nCol, nRow2);
        OSL_ENSURE( nCol == 105, "sw_GetCellPosition failed" );
    }
#endif

    if (nColumn < 0 || nRow < 0)
        return OUString();
    OUString sCellName;
    sw_GetTblBoxColStr( static_cast< sal_uInt16 >(nColumn), sCellName );
    return sCellName + OUString::number( nRow + 1 );
}

/** Find the top left or bottom right corner box in given table.
  Consider nested lines when finding the box.

  @param rTableLines the table
  @param i_bTopLeft if true, find top left box, otherwise find bottom
         right box
 */
const SwTableBox* lcl_FindCornerTableBox(const SwTableLines& rTableLines, const bool i_bTopLeft)
{
    bool bFirst = true;
    const SwTableBox* pBox = 0;
    do
    {
        const SwTableLines& rLines(bFirst ? rTableLines : pBox->GetTabLines());
        bFirst = false;
        assert(rLines.size() != 0);
        if (!rLines.empty())
        {
            const SwTableLine* pLine(i_bTopLeft ? rLines.front() : rLines.back());
            assert(pLine);
            const SwTableBoxes& rBoxes(pLine->GetTabBoxes());
            assert(rBoxes.size() != 0);
            pBox = i_bTopLeft ? rBoxes.front() : rBoxes.back();
            assert(pBox);
        }
        else
        {
            pBox = 0;
        }
    } while (pBox && !pBox->GetSttNd());
    return pBox;
}

/** cleanup order in a range
 *
 * Sorts the input to a uniform format. I.e. for the four possible representation
 *      A1:C5, C5:A1, A5:C1, C1:A5
 * the result will be always A1:C5.
 *
 * @param [IN,OUT] rCell1 cell name (will be modified to upper-left corner), e.g. "A1" (non-empty string with valid cell name)
 * @param [IN,OUT] rCell2 cell name (will be modified to lower-right corner), e.g. "A1" (non-empty string with valid cell name)
 */
void sw_NormalizeRange(OUString &rCell1, OUString &rCell2)
{
    sal_Int32 nCol1 = -1, nRow1 = -1, nCol2 = -1, nRow2 = -1;
    sw_GetCellPosition( rCell1, nCol1, nRow1 );
    sw_GetCellPosition( rCell2, nCol2, nRow2 );
    if (nCol2 < nCol1 || nRow2 < nRow1)
    {
        rCell1  = sw_GetCellName( std::min(nCol1, nCol2), std::min(nRow1, nRow2) );
        rCell2  = sw_GetCellName( std::max(nCol1, nCol2), std::max(nRow1, nRow2) );
    }
}

void SwRangeDescriptor::Normalize()
{
    if (nTop > nBottom)
    {
        sal_Int32 nTmp = nTop;
        nTop = nBottom;
        nBottom = nTmp;
    }
    if (nLeft > nRight)
    {
        sal_Int32 nTmp = nLeft;
        nLeft = nRight;
        nRight = nTmp;
    }
}

static SwXCell* lcl_CreateXCell(SwFrmFmt* pFmt, sal_Int32 nColumn, sal_Int32 nRow)
{
    SwXCell* pXCell = 0;
    const OUString sCellName = sw_GetCellName(nColumn, nRow);
    SwTable* pTable = SwTable::FindTable( pFmt );
    SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
    if(pBox)
    {
        pXCell = SwXCell::CreateXCell(pFmt, pBox, pTable);
    }
    return pXCell;
}

static void lcl_InspectLines(SwTableLines& rLines, std::vector<OUString*>& rAllNames)
{
    for( size_t i = 0; i < rLines.size(); ++i )
    {
        SwTableLine* pLine = rLines[i];
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for(size_t j = 0; j < rBoxes.size(); ++j)
        {
            SwTableBox* pBox = rBoxes[j];
            if(!pBox->GetName().isEmpty() && pBox->getRowSpan() > 0 )
                rAllNames.push_back( new OUString(pBox->GetName()) );
            SwTableLines& rBoxLines = pBox->GetTabLines();
            if(!rBoxLines.empty())
            {
                lcl_InspectLines(rBoxLines, rAllNames);
            }
        }
    }
}

static void lcl_FormatTable(SwFrmFmt* pTblFmt)
{
    SwIterator<SwFrm,SwFmt> aIter( *pTblFmt );
    for( SwFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next() )
    {
        // mba: no TYPEINFO for SwTabFrm
        if( pFrm->IsTabFrm() )
        {
            if(pFrm->IsValid())
                pFrm->InvalidatePos();
            static_cast<SwTabFrm*>(pFrm)->SetONECalcLowers();
            static_cast<SwTabFrm*>(pFrm)->Calc();
        }
    }
}

static void lcl_CrsrSelect(SwPaM& rCrsr, bool bExpand)
{
    if(bExpand)
    {
        if(!rCrsr.HasMark())
            rCrsr.SetMark();
    }
    else if(rCrsr.HasMark())
        rCrsr.DeleteMark();
}

static void lcl_GetTblSeparators(uno::Any& rRet, SwTable* pTable, SwTableBox* pBox, bool bRow)
{
    SwTabCols aCols;
    aCols.SetLeftMin ( 0 );
    aCols.SetLeft    ( 0 );
    aCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aCols, pBox, false, bRow );

    const size_t nSepCount = aCols.Count();
    uno::Sequence< text::TableColumnSeparator> aColSeq(nSepCount);
    text::TableColumnSeparator* pArray = aColSeq.getArray();
    bool bError = false;
    for(size_t i = 0; i < nSepCount; ++i)
    {
        pArray[i].Position = static_cast< sal_Int16 >(aCols[i]);
        pArray[i].IsVisible = !aCols.IsHidden(i);
        if(!bRow && !pArray[i].IsVisible)
        {
            bError = true;
            break;
        }
    }
    if(!bError)
        rRet.setValue(&aColSeq, ::getCppuType((uno::Sequence< text::TableColumnSeparator>*)0));

}

static void lcl_SetTblSeparators(const uno::Any& rVal, SwTable* pTable, SwTableBox* pBox, bool bRow, SwDoc* pDoc)
{
    SwTabCols aOldCols;

    aOldCols.SetLeftMin ( 0 );
    aOldCols.SetLeft    ( 0 );
    aOldCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aOldCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aOldCols, pBox, false, bRow );
    const size_t nOldCount = aOldCols.Count();
    // there is no use in setting tab cols if there is only one column
    if( !nOldCount )
        return;

    const uno::Sequence< text::TableColumnSeparator>* pSepSeq =
                (uno::Sequence< text::TableColumnSeparator>*) rVal.getValue();
    if(pSepSeq && static_cast<size_t>(pSepSeq->getLength()) == nOldCount)
    {
        SwTabCols aCols(aOldCols);
        bool bError = false;
        const text::TableColumnSeparator* pArray = pSepSeq->getConstArray();
        long nLastValue = 0;
        //sal_Int32 nTblWidth = aCols.GetRight() - aCols.GetLeft();
        for(size_t i = 0; i < nOldCount; ++i)
        {
            aCols[i] = pArray[i].Position;
            if(pArray[i].IsVisible == (aCols.IsHidden(i) ? 1 : 0) ||
                (!bRow && aCols.IsHidden(i)) ||
                aCols[i] < nLastValue ||
                UNO_TABLE_COLUMN_SUM < aCols[i] )
            {
                bError = true;
                break;
            }
            nLastValue = aCols[i];
        }
        if(!bError)
        {
            pDoc->SetTabCols(*pTable, aCols, aOldCols, pBox, bRow );
        }
    }
}

static inline OUString lcl_getString( SwXCell &rCell )
{
    // getString is a member function of the base class...
    return rCell.getString();
}

/* non UNO function call to set string in SwXCell */
void sw_setString( SwXCell &rCell, const OUString &rTxt,
        bool bKeepNumberFmt = false )
{
    if(rCell.IsValid())
    {
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        pBoxFmt->LockModify();
        pBoxFmt->ResetFmtAttr( RES_BOXATR_FORMULA );
        pBoxFmt->ResetFmtAttr( RES_BOXATR_VALUE );
        if (!bKeepNumberFmt)
            pBoxFmt->SetFmtAttr( SwTblBoxNumFormat(NUMBERFORMAT_TEXT) );
        pBoxFmt->UnlockModify();
    }
    rCell.SwXText::setString(rTxt);
}

/* non UNO function call to get value from SwXCell */
double sw_getValue( SwXCell &rCell )
{
    double fRet;
    if(rCell.IsValid() && !rCell.getString().isEmpty())
        fRet = rCell.pBox->GetFrmFmt()->GetTblBoxValue().GetValue();
    else
        ::rtl::math::setNan( &fRet );
    return fRet;
}

/* non UNO function call to set value in SwXCell */
void sw_setValue( SwXCell &rCell, double nVal )
{
    if(rCell.IsValid())
    {
        // first this text (maybe) needs to be deleted
        sal_uLong nNdPos = rCell.pBox->IsValidNumTxtNd( true );
        if(ULONG_MAX != nNdPos)
            sw_setString( rCell, OUString(), true );   // true == keep number format
        SwDoc* pDoc = rCell.GetDoc();
        UnoActionContext aAction(pDoc);
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE);
        const SfxPoolItem* pItem;

        //!! do we need to set a new number format? Yes, if
        // - there is no current number format
        // - the current number format is not a number format according to the number formatter, but rather a text format
        // - the current number format is not even a valid number formatter number format, but rather Writer's own 'special' text number format
        if(SfxItemState::SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, true, &pItem)
            ||  pDoc->GetNumberFormatter()->IsTextFormat(static_cast<const SwTblBoxNumFormat*>(pItem)->GetValue())
            ||  static_cast<const SwTblBoxNumFormat*>(pItem)->GetValue() == NUMBERFORMAT_TEXT)
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }

        SwTblBoxValue aVal(nVal);
        aSet.Put(aVal);
        pDoc->SetTblBoxFormulaAttrs( *rCell.pBox, aSet );
        // update table
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( rCell.GetFrmFmt() ));
        pDoc->getIDocumentFieldsAccess().UpdateTblFlds( &aTblUpdate );
    }
}

TYPEINIT1(SwXCell, SwClient);

SwXCell::SwXCell(SwFrmFmt* pTblFmt, SwTableBox* pBx, size_t const nPos) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    SwClient(pTblFmt),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_CELL)),
    pBox(pBx),
    pStartNode(0),
    nFndPos(nPos)
{
}

SwXCell::SwXCell(SwFrmFmt* pTblFmt, const SwStartNode& rStartNode) :
    SwXText(pTblFmt->GetDoc(), CURSOR_TBLTEXT),
    SwClient(pTblFmt),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_CELL)),
    pBox(0),
    pStartNode(&rStartNode),
    nFndPos(NOTFOUND)
{
}

SwXCell::~SwXCell()
{
}

namespace
{
    class theSwXCellUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXCellUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXCell::getUnoTunnelId()
{
    return theSwXCellUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXCell::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    else
        return SwXText::getSomething(rId);
}

uno::Sequence< uno::Type > SAL_CALL SwXCell::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    static uno::Sequence< uno::Type > aRetTypes;
    if(!aRetTypes.getLength())
    {
        aRetTypes = SwXCellBaseClass::getTypes();
        uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

        long nIndex = aRetTypes.getLength();
        aRetTypes.realloc(
            aRetTypes.getLength() +
            aTextTypes.getLength());

        uno::Type* pRetTypes = aRetTypes.getArray();

        const uno::Type* pTextTypes = aTextTypes.getConstArray();
        for(long nPos = 0; nPos <aTextTypes.getLength(); nPos++)
            pRetTypes[nIndex++] = pTextTypes[nPos];
    }
    return aRetTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXCell::getImplementationId(  ) throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SwXCell::acquire(  ) throw()
{
    SwXCellBaseClass::acquire();
}

void SAL_CALL SwXCell::release(  ) throw()
{
    SwXCellBaseClass::release();
}

uno::Any SAL_CALL SwXCell::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException, std::exception)
{
    uno::Any aRet = SwXCellBaseClass::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXText::queryInterface(aType);
    return aRet;
}

const SwStartNode *SwXCell::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;

    if( pStartNode || ((SwXCell *)this)->IsValid() )
        pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();

    return pSttNd;
}

uno::Reference< text::XTextCursor >
SwXCell::CreateCursor() throw (uno::RuntimeException)
{
    return createTextCursor();
}

bool SwXCell::IsValid() const
{
    // FIXME: this is now a const method, to make SwXText::IsValid invisible
    // but the const_cast here are still ridiculous. TODO: find a better way.
    SwFrmFmt* pTblFmt = pBox ? GetFrmFmt() : 0;
    if(!pTblFmt)
    {
        const_cast<SwXCell*>(this)->pBox = 0;
    }
    else
    {
        SwTable* pTable = SwTable::FindTable( pTblFmt );
        SwTableBox const*const pFoundBox =
            const_cast<SwXCell*>(this)->FindBox(pTable, pBox);
        if (!pFoundBox)
        {
            const_cast<SwXCell*>(this)->pBox = 0;
        }
    }
    return 0 != pBox;
}

OUString SwXCell::getFormula(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString sRet;
    if(IsValid())
    {
        SwTblBoxFormula aFormula( pBox->GetFrmFmt()->GetTblBoxFormula() );
        SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        aFormula.PtrToBoxNm( pTable );
        sRet = aFormula.GetFormula();
    }
    return sRet;
}

///@see sw_setValue (TODO: seems to be copy and paste programming here)
void SwXCell::setFormula(const OUString& rFormula) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        // first this text (maybe) needs to be deleted
        sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( true );
        if(USHRT_MAX == nNdPos)
            sw_setString( *this, OUString(), true );
        OUString sFml(comphelper::string::stripStart(rFormula, ' '));
        if( !sFml.isEmpty() && '=' == sFml[0] )
                    sFml = sFml.copy( 1 );
        SwTblBoxFormula aFml( sFml );
        SwDoc* pMyDoc = GetDoc();
        UnoActionContext aAction(pMyDoc);
        SfxItemSet aSet(pMyDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_FORMULA);
        const SfxPoolItem* pItem;
        SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
        if(SfxItemState::SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, true, &pItem)
            ||  pMyDoc->GetNumberFormatter()->IsTextFormat(static_cast<const SwTblBoxNumFormat*>(pItem)->GetValue()))
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }
        aSet.Put(aFml);
        GetDoc()->SetTblBoxFormulaAttrs( *pBox, aSet );
        // update table
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( GetFrmFmt() ));
        pMyDoc->getIDocumentFieldsAccess().UpdateTblFlds( &aTblUpdate );
    }
}

double SwXCell::getValue(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    double const fRet = sw_getValue( *this );
    // #i112652# a table cell may contain NaN as a value, do not filter that
    return fRet;
}

void SwXCell::setValue(double rValue) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sw_setValue( *this, rValue );
}

table::CellContentType SwXCell::getType(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    table::CellContentType nRes = table::CellContentType_EMPTY;
    sal_uInt32 nNdPos = pBox->IsFormulaOrValueBox();
    switch (nNdPos)
    {
        case 0 :                    nRes = table::CellContentType_TEXT; break;
        case USHRT_MAX :            nRes = table::CellContentType_EMPTY; break;
        case RES_BOXATR_VALUE :     nRes = table::CellContentType_VALUE; break;
        case RES_BOXATR_FORMULA :   nRes = table::CellContentType_FORMULA; break;
        default :
            OSL_FAIL( "unexpected case" );
    }
    return  nRes;
}

void SwXCell::setString(const OUString& aString) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sw_setString( *this, aString );
}

sal_Int32 SwXCell::getError(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString sContent = getString();
    return sal_Int32(sContent.equals(SwViewShell::GetShellRes()->aCalc_Error));
}

uno::Reference< text::XTextCursor >  SwXCell::createTextCursor(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >     aRef;
    if(pStartNode || IsValid())
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        SwXTextCursor *const pXCursor =
            new SwXTextCursor(*GetDoc(), this, CURSOR_TBLTEXT, aPos);
        SwUnoCrsr *const pUnoCrsr = pXCursor->GetCursor();
        pUnoCrsr->Move(fnMoveForward, fnGoNode);
        aRef =  static_cast<text::XWordCursor*>(pXCursor);
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< text::XTextCursor >  SwXCell::createTextCursorByRange(const uno::Reference< text::XTextRange > & xTextPosition)
                                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if ((pStartNode || IsValid())
        && ::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        // skip sections
        SwStartNode* p1 = aPam.GetNode().StartOfSectionNode();
        while(p1->IsSectionNode())
            p1 = p1->StartOfSectionNode();

        if( p1 == pSttNd )
        {
            aRef = static_cast<text::XWordCursor*>(
                    new SwXTextCursor(*GetDoc(), this, CURSOR_TBLTEXT,
                        *aPam.GetPoint(), aPam.GetMark()));
        }
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< beans::XPropertySetInfo >  SwXCell::getPropertySetInfo(void) throw( uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXCell::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        // Hack to support hidden property to transfer textDirection
        if  ( rPropertyName == "FRMDirection" )
        {
            SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
            sal_Int16 nNum = 0;
            aValue >>= nNum;
            SAL_INFO("sw.uno", "FRMDirection val " << nNum);
            switch (nNum)
            {
                case 0:
                    eDir = FRMDIR_HORI_LEFT_TOP;
                    break;
                case 1:
                    eDir = FRMDIR_HORI_RIGHT_TOP;
                    break;
                case 2:
                    eDir = FRMDIR_VERT_TOP_RIGHT;
                    break;
                default:
                    OSL_FAIL( "unknown direction code, maybe it's a bitfield");
            }
            SvxFrameDirectionItem aItem( eDir, RES_FRAMEDIR);
            pBox->GetFrmFmt()->SetFmtAttr(aItem);
        }
        else if  ( rPropertyName == "TableRedlineParams" )
        {
            // Get the table row properties
            uno::Sequence< beans::PropertyValue > tableCellProperties;
            tableCellProperties = aValue.get< uno::Sequence< beans::PropertyValue > >();
            comphelper::SequenceAsHashMap aPropMap( tableCellProperties );
            OUString sRedlineType;
            uno::Any sRedlineTypeValue;
            sRedlineTypeValue = aPropMap.getUnpackedValueOrDefault("RedlineType", sRedlineTypeValue);
            if( sRedlineTypeValue >>= sRedlineType )
            {
                // Create a 'Table Cell Redline' object
                SwUnoCursorHelper::makeTableCellRedline( *pBox, sRedlineType, tableCellProperties);
            }
            else
            {
                throw beans::UnknownPropertyException("No redline type property: ", static_cast < cppu::OWeakObject * > ( this ) );
            }
        }
        else
        {
            const SfxItemPropertySimpleEntry* pEntry =
                m_pPropSet->getPropertyMap().getByName(rPropertyName);
            if( !pEntry )
            {
                beans::UnknownPropertyException aEx;
                aEx.Message = rPropertyName;
                throw( aEx );
            }
            if( pEntry->nWID == FN_UNO_CELL_ROW_SPAN )
            {
                sal_Int32 nRowSpan = 0;
                if( aValue >>= nRowSpan )
                    pBox->setRowSpan( nRowSpan );
            }
            else
            {
                SwFrmFmt* pBoxFmt = pBox->ClaimFrmFmt();
                SwAttrSet aSet(pBoxFmt->GetAttrSet());
                m_pPropSet->setPropertyValue(rPropertyName, aValue, aSet);
                pBoxFmt->GetDoc()->SetAttr(aSet, *pBoxFmt);
            }
        }
    }
}

uno::Any SwXCell::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    if(IsValid())
    {
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if( !pEntry )
        {
            beans::UnknownPropertyException aEx;
            aEx.Message = rPropertyName;
            throw( aEx );
        }
        switch( pEntry->nWID )
        {
            case FN_UNO_CELL_ROW_SPAN:
                aRet <<= pBox->getRowSpan();
            break;
            case FN_UNO_TEXT_SECTION:
            {
                SwFrmFmt* pTblFmt = GetFrmFmt();
                SwTable* pTable = SwTable::FindTable( pTblFmt );
                SwTableNode* pTblNode = pTable->GetTableNode();
                SwSectionNode* pSectionNode =  pTblNode->FindSectionNode();
                if(pSectionNode)
                {
                    const SwSection& rSect = pSectionNode->GetSection();
                    uno::Reference< text::XTextSection >  xSect =
                                    SwXTextSections::GetObject( *rSect.GetFmt() );
                    aRet <<= xSect;
                }
            }
            break;
            case FN_UNO_CELL_NAME:
                aRet <<= pBox->GetName();
            break;
            case FN_UNO_REDLINE_NODE_START:
            case FN_UNO_REDLINE_NODE_END:
            {
                //redline can only be returned if it's a living object
                aRet = SwXText::getPropertyValue(rPropertyName);
            }
            break;
            default:
            {
                const SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                const SwAttrSet& rSet = pBoxFmt->GetAttrSet();
                m_pPropSet->getPropertyValue(rPropertyName, rSet, aRet);
            }
        }
    }
    return aRet;
}

void SwXCell::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCell::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCell::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCell::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

uno::Reference< container::XEnumeration >  SwXCell::createEnumeration(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
    if(IsValid())
    {
        const SwStartNode* pSttNd = pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        ::std::unique_ptr<SwUnoCrsr> pUnoCursor(
            GetDoc()->CreateUnoCrsr(aPos, false));
        pUnoCursor->Move(fnMoveForward, fnGoNode);

        // remember table and start node for later travelling
        // (used in export of tables in tables)
        SwTable const*const pTable( & pSttNd->FindTableNode()->GetTable() );
        SwXParagraphEnumeration *const pEnum =
            new SwXParagraphEnumeration(this, std::move(pUnoCursor), CURSOR_TBLTEXT,
                    pSttNd, pTable);

        aRef = pEnum;
    }
    return aRef;
}

uno::Type SAL_CALL SwXCell::getElementType(void) throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SwXCell::hasElements(void) throw( uno::RuntimeException, std::exception )
{
    return sal_True;
}

void SwXCell::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

SwXCell* SwXCell::CreateXCell(SwFrmFmt* pTblFmt, SwTableBox* pBox, SwTable *pTable )
{
    SwXCell* pRet = 0;
    if(pTblFmt && pBox)
    {
        if( !pTable )
            pTable = SwTable::FindTable( pTblFmt );
        SwTableSortBoxes::const_iterator it = pTable->GetTabSortBoxes().find( pBox );

        // if the box exists, then return a cell
        if( it != pTable->GetTabSortBoxes().end() )
        {
            size_t const nPos = it - pTable->GetTabSortBoxes().begin();
            SwIterator<SwXCell,SwFmt> aIter( *pTblFmt );
            SwXCell* pXCell = aIter.First();
            while( pXCell )
            {
                // is there already a proper cell?
                if(pXCell->GetTblBox() == pBox)
                    break;
                pXCell = aIter.Next();
            }
            // otherwise create it
            if(!pXCell)
            {
                pXCell = new SwXCell(pTblFmt, pBox, nPos);
            }
            pRet = pXCell;
        }
    }
    return pRet;
}

/** search if a box exists in a table
 *
 * @param pTable the table to search in
 * @param pBox2 box model to find
 * @return the box if existent in pTable, 0 (!!!) if not found
 */
SwTableBox* SwXCell::FindBox(SwTable* pTable, SwTableBox* pBox2)
{
    // check if nFndPos happens to point to the right table box
    if( nFndPos < pTable->GetTabSortBoxes().size() &&
        pBox2 == pTable->GetTabSortBoxes()[ nFndPos ] )
        return pBox2;

    // if not, seek the entry (and return, if successful)
    SwTableSortBoxes::const_iterator it = pTable->GetTabSortBoxes().find( pBox2 );
    if( it != pTable->GetTabSortBoxes().end() )
    {
        nFndPos = it - pTable->GetTabSortBoxes().begin();
        return pBox2;
    }

    // box not found: reset nFndPos pointer
    nFndPos = NOTFOUND;
    return 0;
}

OUString SwXCell::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXCell");
}

sal_Bool SwXCell::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXCell::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.CellProperties";
    return aRet;
}

OUString SwXTextTableRow::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextTableRow");
}

sal_Bool SwXTextTableRow::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextTableRow::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextTableRow";
    return aRet;
}
TYPEINIT1(SwXTextTableRow, SwClient);

SwXTextTableRow::SwXTextTableRow(SwFrmFmt* pFmt, SwTableLine* pLn) :
    SwClient(pFmt),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE_ROW)),
    pLine(pLn)
{
}

SwXTextTableRow::~SwXTextTableRow()
{
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTableRow::getPropertySetInfo(void) throw( uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXTextTableRow::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw (beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            // Check for a specific property
            if  ( rPropertyName == "TableRedlineParams" )
            {
                // Get the table row properties
                uno::Sequence< beans::PropertyValue > tableRowProperties;
                tableRowProperties = aValue.get< uno::Sequence< beans::PropertyValue > >();
                comphelper::SequenceAsHashMap aPropMap( tableRowProperties );
                OUString sRedlineType;
                uno::Any sRedlineTypeValue;
                sRedlineTypeValue = aPropMap.getUnpackedValueOrDefault("RedlineType", sRedlineTypeValue);
                if( sRedlineTypeValue >>= sRedlineType )
                {
                    // Create a 'Table Row Redline' object
                    SwUnoCursorHelper::makeTableRowRedline( *pLn, sRedlineType, tableRowProperties);
                }
                else
                {
                    throw beans::UnknownPropertyException("No redline type property: ", static_cast < cppu::OWeakObject * > ( this ) );
                }
            }
            else
            {
                const SfxItemPropertySimpleEntry* pEntry =
                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
                SwDoc* pDoc = pFmt->GetDoc();
                if (!pEntry)
                    throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
                if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                    throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

                switch(pEntry->nWID)
                {
                    case FN_UNO_ROW_HEIGHT:
                    case FN_UNO_ROW_AUTO_HEIGHT:
                    {
                        SwFmtFrmSize aFrmSize(pLn->GetFrmFmt()->GetFrmSize());
                        if(FN_UNO_ROW_AUTO_HEIGHT== pEntry->nWID)
                        {
                            bool bSet = *(sal_Bool*)aValue.getValue();
                            aFrmSize.SetHeightSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
                        }
                        else
                        {
                            sal_Int32 nHeight = 0;
                            aValue >>= nHeight;
                             Size aSz(aFrmSize.GetSize());
                            aSz.Height() = convertMm100ToTwip(nHeight);
                            aFrmSize.SetSize(aSz);
                        }
                        pDoc->SetAttr(aFrmSize, *pLn->ClaimFrmFmt());
                    }
                    break;

                    case FN_UNO_TABLE_COLUMN_SEPARATORS:
                    {
                        UnoActionContext aContext(pDoc);
                        SwTable* pTable2 = SwTable::FindTable( pFmt );
                        lcl_SetTblSeparators(aValue, pTable2, pLine->GetTabBoxes()[0], true, pDoc);
                    }
                    break;

                    default:
                    {
                        SwFrmFmt* pLnFmt = pLn->ClaimFrmFmt();
                        SwAttrSet aSet(pLnFmt->GetAttrSet());
                        m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                        pDoc->SetAttr(aSet, *pLnFmt);
                    }
                }
            }
        }
    }
}

uno::Any SwXTextTableRow::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
            if (!pEntry)
                throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pEntry->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    const SwFmtFrmSize& rSize = pLn->GetFrmFmt()->GetFrmSize();
                    if(FN_UNO_ROW_AUTO_HEIGHT== pEntry->nWID)
                    {
                        aRet <<= ATT_VAR_SIZE == rSize.GetHeightSizeType();
                    }
                    else
                        aRet <<= (sal_Int32)(convertTwipToMm100(rSize.GetSize().Height()));
                }
                break;

                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    lcl_GetTblSeparators(aRet, pTable, pLine->GetTabBoxes()[0], true);
                }
                break;

                default:
                {
                    const SwAttrSet& rSet = pLn->GetFrmFmt()->GetAttrSet();
                    m_pPropSet->getPropertyValue(*pEntry, rSet, aRet);
                }
            }
        }
    }
    return aRet;
}

void SwXTextTableRow::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

SwTableLine* SwXTextTableRow::FindLine(SwTable* pTable, SwTableLine* pLine)
{
    SwTableLine* pRet = 0;
    SwTableLines &rLines = pTable->GetTabLines();
    for(size_t i = 0; i < rLines.size(); ++i)
        if(rLines[i] == pLine)
        {
            pRet = pLine;
            break;
        }
    return pRet;
}

// SwXTextTableCursor

OUString SwXTextTableCursor::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextTableCursor");
}

sal_Bool SwXTextTableCursor::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

IMPLEMENT_FORWARD_XINTERFACE2(SwXTextTableCursor,SwXTextTableCursor_Base,OTextCursorHelper)
const SwPaM*        SwXTextTableCursor::GetPaM() const  { return GetCrsr(); }
SwPaM*              SwXTextTableCursor::GetPaM()        { return GetCrsr(); }
const SwDoc*        SwXTextTableCursor::GetDoc() const  { return GetFrmFmt()->GetDoc(); }
SwDoc*              SwXTextTableCursor::GetDoc()        { return GetFrmFmt()->GetDoc(); }
const SwUnoCrsr*    SwXTextTableCursor::GetCrsr() const { return static_cast<const SwUnoCrsr*>(aCrsrDepend.GetRegisteredIn()); }
SwUnoCrsr*          SwXTextTableCursor::GetCrsr()       { return static_cast<SwUnoCrsr*>(aCrsrDepend.GetRegisteredIn()); }

uno::Sequence< OUString > SwXTextTableCursor::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TextTableCursor";
    return aRet;
}

SwXTextTableCursor::SwXTextTableCursor(SwFrmFmt* pFmt, SwTableBox* pBox) :
    SwClient(pFmt),
    aCrsrDepend(this, 0),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE_CURSOR))
{
    SwDoc* pDoc = pFmt->GetDoc();
    const SwStartNode* pSttNd = pBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, true);
    pUnoCrsr->Move( fnMoveForward, fnGoNode );
    pUnoCrsr->Add(&aCrsrDepend);
    SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
    rTblCrsr.MakeBoxSels();
}

SwXTextTableCursor::SwXTextTableCursor(SwFrmFmt& rTableFmt, const SwTableCursor* pTableSelection) :
    SwClient(&rTableFmt),
    aCrsrDepend(this, 0),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE_CURSOR))
{
    SwUnoCrsr* pUnoCrsr = pTableSelection->GetDoc()->CreateUnoCrsr(*pTableSelection->GetPoint(), true);
    if(pTableSelection->HasMark())
    {
        pUnoCrsr->SetMark();
        *pUnoCrsr->GetMark() = *pTableSelection->GetMark();
    }
    const SwSelBoxes& rBoxes = pTableSelection->GetSelectedBoxes();
    SwTableCursor& rTableCrsr = dynamic_cast<SwTableCursor&>(*pUnoCrsr);
    for (size_t i = 0; i < rBoxes.size(); i++)
    {
        rTableCrsr.InsertBox( *rBoxes[i] );
    }

    pUnoCrsr->Add(&aCrsrDepend);
    SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
    rTblCrsr.MakeBoxSels();
}

SwXTextTableCursor::~SwXTextTableCursor()
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
}

OUString SwXTextTableCursor::getRangeName()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    OUString aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
    //!! see also SwChartDataSequence::getSourceRangeRepresentation
    if (pTblCrsr)
    {
        pTblCrsr->MakeBoxSels();
        const SwStartNode* pNode = pTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
        const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        const SwTableBox* pEndBox = pTable->GetTblBox( pNode->GetIndex());
        aRet = pEndBox->GetName();

        if(pTblCrsr->HasMark())
        {
            pNode = pTblCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableBox* pStartBox = pTable->GetTblBox( pNode->GetIndex());
            if(pEndBox != pStartBox)
            {
                // need to switch start and end?
                if (*pTblCrsr->GetPoint() < *pTblCrsr->GetMark())
                {
                    const SwTableBox* pTmpBox = pStartBox;
                    pStartBox = pEndBox;
                    pEndBox = pTmpBox;
                }

                aRet = pStartBox->GetName() + ":" + pEndBox->GetName();
            }
        }
    }
    return aRet;
}

sal_Bool SwXTextTableCursor::gotoCellByName(const OUString& sCellName, sal_Bool Expand)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        bRet = rTblCrsr.GotoTblBox(sCellName);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goLeft(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        bRet = rTblCrsr.Left(Count, CRSR_SKIP_CHARS, false, false);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goRight(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        bRet = rTblCrsr.Right(Count, CRSR_SKIP_CHARS, false, false);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goUp(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        bRet = rTblCrsr.UpDown(true, Count, 0, 0);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goDown(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        bRet = rTblCrsr.UpDown(false, Count, 0, 0);
    }
    return bRet;
}

void SwXTextTableCursor::gotoStart(sal_Bool Expand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        rTblCrsr.MoveTable(fnTableCurr, fnTableStart);
    }
}

void SwXTextTableCursor::gotoEnd(sal_Bool Expand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        lcl_CrsrSelect(rTblCrsr, Expand);
        rTblCrsr.MoveTable(fnTableCurr, fnTableEnd);
    }
}

sal_Bool SwXTextTableCursor::mergeRange()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        {
            // The Actions need to be revoked here
            UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
        }
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        rTblCrsr.MakeBoxSels();

        {
            UnoActionContext aContext(pUnoCrsr->GetDoc());
            bRet = TBLMERGE_OK == rTblCrsr.GetDoc()->MergeTbl(rTblCrsr);
            if(bRet)
            {
                size_t nCount = rTblCrsr.GetSelectedBoxesCount();
                while (nCount--)
                {
                    rTblCrsr.DeleteBox(nCount);
                }
            }
        }
        rTblCrsr.MakeBoxSels();
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::splitRange(sal_Int16 Count, sal_Bool Horizontal)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (Count <= 0)
        throw uno::RuntimeException("Illegal first argument: needs to be > 0", static_cast < cppu::OWeakObject * > ( this ) );
    bool bRet = false;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        {
            // here, all actions need to be revoked
            UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
        }
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        rTblCrsr.MakeBoxSels();
        {
            UnoActionContext aContext(pUnoCrsr->GetDoc());
            bRet = rTblCrsr.GetDoc()->SplitTbl(
                    rTblCrsr.GetSelectedBoxes(), !Horizontal, Count);
        }
        rTblCrsr.MakeBoxSels();
    }
    return bRet;
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTableCursor::getPropertySetInfo(void) throw( uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXTextTableCursor::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw (beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode().StartOfSectionNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            rTblCrsr.MakeBoxSels();
            SwDoc* pDoc = pUnoCrsr->GetDoc();
            switch(pEntry->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    pDoc->GetBoxAttr( *pUnoCrsr, aBrush );
                    aBrush.PutValue(aValue, pEntry->nMemberId);
                    pDoc->SetBoxAttr( *pUnoCrsr, aBrush );

                }
                break;
                case RES_BOXATR_FORMAT:
                {
                    SfxUInt32Item aNumberFormat(RES_BOXATR_FORMAT);
                    aNumberFormat.PutValue(aValue, 0);
                    pDoc->SetBoxAttr( *pUnoCrsr, aNumberFormat);
                }
                break;
                case FN_UNO_PARA_STYLE:
                    SwUnoCursorHelper::SetTxtFmtColl(aValue, *pUnoCrsr);
                break;
                default:
                {
                    SfxItemSet aItemSet( pDoc->GetAttrPool(), pEntry->nWID, pEntry->nWID );
                    SwUnoCursorHelper::GetCrsrAttr(rTblCrsr.GetSelRing(),
                            aItemSet);

                    if (!SwUnoCursorHelper::SetCursorPropertyValue(
                            *pEntry, aValue, rTblCrsr.GetSelRing(), aItemSet))
                    {
                        m_pPropSet->setPropertyValue(*pEntry, aValue, aItemSet);
                    }
                    SwUnoCursorHelper::SetCrsrAttr(rTblCrsr.GetSelRing(),
                            aItemSet, nsSetAttrMode::SETATTR_DEFAULT, true);
                }
            }
        }
        else
            throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXTextTableCursor::getPropertyValue(const OUString& rPropertyName)
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode().StartOfSectionNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr& rTblCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            rTblCrsr.MakeBoxSels();
            switch(pEntry->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    if (rTblCrsr.GetDoc()->GetBoxAttr( *pUnoCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pEntry->nMemberId);

                }
                break;
                case RES_BOXATR_FORMAT:
                    // TODO: GetAttr for table selections in a Doc is missing
                    OSL_FAIL("not implemented");
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl *const pFmt =
                        SwUnoCursorHelper::GetCurTxtFmtColl(*pUnoCrsr, false);
                    OUString sRet;
                    if(pFmt)
                        sRet = pFmt->GetName();
                    aRet <<= sRet;
                }
                break;
                default:
                {
                    SfxItemSet aSet(rTblCrsr.GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    SwUnoCursorHelper::GetCrsrAttr(rTblCrsr.GetSelRing(),
                            aSet);
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aRet);
                }
            }
        }
        else
            throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXTextTableCursor::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

class SwXTextTable::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::cppu::OMultiTypeInterfaceContainerHelper m_Listeners;

    Impl() : m_Listeners(m_Mutex) { }
};

class SwTableProperties_Impl
{
    SwUnoCursorHelper::SwAnyMapHelper aAnyMap;
public:
    SwTableProperties_Impl();
    ~SwTableProperties_Impl();

    void        SetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any& aVal);
    bool    GetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& rpAny);

    void        ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc);
};

SwTableProperties_Impl::SwTableProperties_Impl()
{
}

SwTableProperties_Impl::~SwTableProperties_Impl()
{
}

void SwTableProperties_Impl::SetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any& rVal)
{
    aAnyMap.SetValue( nWhichId, nMemberId, rVal );
}

bool SwTableProperties_Impl::GetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& rpAny )
{
    return aAnyMap.FillValue( nWhichId, nMemberId, rpAny );
}

void SwTableProperties_Impl::ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc)
{
    SfxItemSet aSet(rDoc.GetAttrPool(),
        RES_LAYOUT_SPLIT,   RES_LAYOUT_SPLIT,
        RES_BACKGROUND,     RES_BACKGROUND,
        RES_FRM_SIZE,       RES_UL_SPACE,
        RES_HORI_ORIENT,    RES_HORI_ORIENT,
        RES_BREAK,          RES_BREAK,
        RES_KEEP,           RES_KEEP,
        RES_SHADOW,         RES_SHADOW,
        RES_PAGEDESC,       RES_PAGEDESC,
        0
        );
    const uno::Any* pRepHead;
    const SwFrmFmt &rFrmFmt = *rTbl.GetFrmFmt();
    if(GetProperty(FN_TABLE_HEADLINE_REPEAT, 0xff, pRepHead ))
    {
        bool bVal = *(sal_Bool*)pRepHead->getValue();
        ((SwTable&)rTbl).SetRowsToRepeat( bVal ? 1 : 0 );  // TODO: MULTIHEADER
    }

    const uno::Any* pBackColor   = 0;
    GetProperty(RES_BACKGROUND, MID_BACK_COLOR, pBackColor );
    const uno::Any* pBackTrans  = 0;
    GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENT, pBackTrans );
    const uno::Any* pGrLoc      = 0;
    GetProperty(RES_BACKGROUND, MID_GRAPHIC_POSITION, pGrLoc    );
    const uno::Any* pGrURL      = 0;
    GetProperty(RES_BACKGROUND, MID_GRAPHIC_URL, pGrURL     );
    const uno::Any* pGrFilter   = 0;
    GetProperty(RES_BACKGROUND, MID_GRAPHIC_FILTER, pGrFilter     );

    if(pBackColor||pBackTrans||pGrURL||pGrFilter||pGrLoc)
    {
        SvxBrushItem aBrush(rFrmFmt.makeBackgroundBrushItem());
        if(pBackColor)
            aBrush.PutValue(*pBackColor, MID_BACK_COLOR);
        if(pBackTrans)
            aBrush.PutValue(*pBackTrans, MID_GRAPHIC_TRANSPARENT);
        if(pGrURL)
            aBrush.PutValue(*pGrURL, MID_GRAPHIC_URL);
        if(pGrFilter)
            aBrush.PutValue(*pGrFilter, MID_GRAPHIC_FILTER);
        if(pGrLoc)
            aBrush.PutValue(*pGrLoc, MID_GRAPHIC_POSITION);
        aSet.Put(aBrush);
    }

    bool bPutBreak = true;
    const uno::Any* pPage;
    if(GetProperty(FN_UNO_PAGE_STYLE, 0, pPage) || GetProperty(RES_PAGEDESC, 0xff, pPage))
    {
        OUString sPageStyle;
        (*pPage) >>= sPageStyle;
        if (!sPageStyle.isEmpty())
        {
            SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true );
            const SwPageDesc* pDesc = SwPageDesc::GetByName(rDoc, sPageStyle);
            if(pDesc)
            {
                SwFmtPageDesc aDesc( pDesc );
                const uno::Any* pPgNo;
                if(GetProperty(RES_PAGEDESC, MID_PAGEDESC_PAGENUMOFFSET, pPgNo ))
                {
                    sal_Int16 nTmp = 0;
                    (*pPgNo) >>= nTmp;
                    aDesc.SetNumOffset( nTmp );
                }
                aSet.Put(aDesc);
                bPutBreak = false;
            }

        }
    }
    const uno::Any* pBreak;
    if(bPutBreak && GetProperty(RES_BREAK, 0, pBreak))
    {
        SvxFmtBreakItem aBreak ( rFrmFmt.GetBreak() );
        aBreak.PutValue(*pBreak, 0);
        aSet.Put(aBreak);
    }
    const uno::Any* pShadow;
    if(GetProperty(RES_SHADOW, 0, pShadow))
    {
        SvxShadowItem aShd ( rFrmFmt.GetShadow() );
        aShd.PutValue(*pShadow, CONVERT_TWIPS);
        aSet.Put(aShd);
    }
    const uno::Any* pKeep;
    if(GetProperty(RES_KEEP, 0, pKeep))
    {
        SvxFmtKeepItem aKeep( rFrmFmt.GetKeep() );
        aKeep.PutValue(*pKeep, 0);
        aSet.Put(aKeep);
    }

    const uno::Any* pHOrient;
    if(GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_ORIENT, pHOrient))
    {
        SwFmtHoriOrient aOrient ( rFrmFmt.GetHoriOrient() );
        ((SfxPoolItem&)aOrient).PutValue(*pHOrient, MID_HORIORIENT_ORIENT|CONVERT_TWIPS);
        aSet.Put(aOrient);
    }

    const uno::Any* pSzRel       = 0;
    GetProperty(FN_TABLE_IS_RELATIVE_WIDTH, 0xff, pSzRel  );
    const uno::Any* pRelWidth   = 0;
    GetProperty(FN_TABLE_RELATIVE_WIDTH, 0xff, pRelWidth);
    const uno::Any* pWidth      = 0;
    GetProperty(FN_TABLE_WIDTH, 0xff, pWidth  );

    bool bPutSize = pWidth != 0;
    SwFmtFrmSize aSz( ATT_VAR_SIZE);
    if(pWidth)
    {
        ((SfxPoolItem&)aSz).PutValue(*pWidth, MID_FRMSIZE_WIDTH);
        bPutSize = true;
    }
    bool bTemp = pSzRel && *(sal_Bool*)pSzRel->getValue();
    if(pSzRel && bTemp && pRelWidth)
    {
        ((SfxPoolItem&)aSz).PutValue(*pRelWidth, MID_FRMSIZE_REL_WIDTH|CONVERT_TWIPS);
        bPutSize = true;
    }
    if(bPutSize)
    {
        if(!aSz.GetWidth())
            aSz.SetWidth(MINLAY);
        aSet.Put(aSz);
    }
    const uno::Any* pL      = 0;
    GetProperty(RES_LR_SPACE, MID_L_MARGIN|CONVERT_TWIPS, pL);
    const uno::Any* pR      = 0;
    GetProperty(RES_LR_SPACE, MID_R_MARGIN|CONVERT_TWIPS, pR);
    if(pL||pR)
    {
        SvxLRSpaceItem aLR ( rFrmFmt.GetLRSpace() );
        if(pL)
            ((SfxPoolItem&)aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            ((SfxPoolItem&)aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        aSet.Put(aLR);
    }
    const uno::Any* pU      = 0;
    GetProperty(RES_UL_SPACE, MID_UP_MARGIN|CONVERT_TWIPS, pU);
    const uno::Any* pLo     = 0;
    GetProperty(RES_UL_SPACE, MID_LO_MARGIN|CONVERT_TWIPS, pLo);
    if(pU||pLo)
    {
        SvxULSpaceItem aUL ( rFrmFmt.GetULSpace() );
        if(pU)
            ((SfxPoolItem&)aUL).PutValue(*pU, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pLo)
            ((SfxPoolItem&)aUL).PutValue(*pLo, MID_LO_MARGIN|CONVERT_TWIPS);
        aSet.Put(aUL);
    }
    const::uno::Any* pSplit;
    if(GetProperty(RES_LAYOUT_SPLIT, 0, pSplit ))
    {
        bool bTmp = *(sal_Bool*)pSplit->getValue();
        SwFmtLayoutSplit aSp(bTmp);
        aSet.Put(aSp);
    }

    if(aSet.Count())
    {
        rDoc.SetAttr( aSet, *rTbl.GetFrmFmt() );
    }
}

namespace
{
    class theSwXTextTableUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextTableUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextTable::getUnoTunnelId()
{
    return theSwXTextTableUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXTextTable::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

TYPEINIT1(SwXTextTable, SwClient)

SwXTextTable::SwXTextTable()
    : m_pImpl(new Impl)
    ,
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE)),
    pTableProps(new SwTableProperties_Impl),
    bIsDescriptor(true),
    nRows(2),
    nColumns(2),
    bFirstRowAsLabel(false),
    bFirstColumnAsLabel(false)
{
}

SwXTextTable::SwXTextTable(SwFrmFmt& rFrmFmt)
    : SwClient( &rFrmFmt )
    , m_pImpl(new Impl)
    ,
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE)),
    pTableProps(0),
    bIsDescriptor(false),
    nRows(0),
    nColumns(0),
    bFirstRowAsLabel(false),
    bFirstColumnAsLabel(false)
{
}

SwXTextTable::~SwXTextTable()
{
    delete pTableProps;
}

uno::Reference<text::XTextTable>
SwXTextTable::CreateXTextTable(SwFrmFmt *const pFrmFmt)
{
    uno::Reference<text::XTextTable> xTable;
    if (pFrmFmt)
    {
        xTable.set(pFrmFmt->GetXObject(), uno::UNO_QUERY); // cached?
    }
    if (!xTable.is())
    {
        SwXTextTable *const pNew(
            (pFrmFmt) ? new SwXTextTable(*pFrmFmt) : new SwXTextTable());
        xTable.set(pNew);
        if (pFrmFmt)
        {
            pFrmFmt->SetXObject(xTable);
        }
        // need a permanent Reference to initialize m_wThis
        pNew->m_pImpl->m_wThis = xTable;
    }
    return xTable;
}

void SwXTextTable::initialize(sal_Int32 nR, sal_Int32 nC) throw( uno::RuntimeException, std::exception )
{
    if(!bIsDescriptor || nR <= 0 || nC <= 0 || nR >= USHRT_MAX || nC >= USHRT_MAX )
        throw uno::RuntimeException();
    else
    {
        nRows = (sal_uInt16)nR;
        nColumns = (sal_uInt16)nC;
    }
}

uno::Reference< table::XTableRows >  SwXTextTable::getRows(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XTableRows >  xRet;
    if (SwFrmFmt* pFmt = GetFrmFmt())
    {
        SwXTableRows* pRows = SwIterator<SwXTableRows,SwFmt>::FirstElement(*pFmt);
        if (!pRows)
            pRows = new SwXTableRows(*pFmt);
        xRet = pRows;
    }
    if (!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< table::XTableColumns >  SwXTextTable::getColumns(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XTableColumns >  xRet;
    if (SwFrmFmt* pFmt = GetFrmFmt())
    {
        SwXTableColumns* pCols = SwIterator<SwXTableColumns,SwFmt>::FirstElement(*pFmt);
        if (!pCols)
            pCols = new SwXTableColumns(*pFmt);
        xRet = pCols;
    }
    if (!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}

uno::Reference< table::XCell > SwXTextTable::getCellByName(const OUString& sCellName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
        if(pBox)
        {
            xRet = SwXCell::CreateXCell(pFmt, pBox);
        }
    }
    else
        throw uno::RuntimeException();
    return xRet;
}

uno::Sequence< OUString > SwXTextTable::getCellNames(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        // exists at the table and at all boxes
        SwTableLines& rTblLines = pTable->GetTabLines();
        std::vector<OUString*> aAllNames;
        lcl_InspectLines(rTblLines, aAllNames);
        uno::Sequence< OUString > aRet( static_cast<sal_Int32>(aAllNames.size()) );
        OUString* pArray = aRet.getArray();
        for( size_t i = 0; i < aAllNames.size(); ++i)
        {
            pArray[i] = *aAllNames[i];
            delete aAllNames[i];
        }
        return aRet;
    }
    return uno::Sequence< OUString >();
}

uno::Reference< text::XTextTableCursor > SwXTextTable::createCursorByCellName(const OUString& sCellName)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextTableCursor >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableBox* pBox = (SwTableBox*)pTable->GetTblBox( sCellName );
        if(pBox && pBox->getRowSpan() > 0 )
        {
            xRet = new SwXTextTableCursor(pFmt, pBox);
        }
    }
    if(!xRet.is())
        throw uno::RuntimeException();
    return xRet;
}

void SwXTextTable::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    // attachToRange must only be called once
    if(!bIsDescriptor)  /* already attached ? */
        throw uno::RuntimeException("SwXTextTable: already attached to range.", static_cast < cppu::OWeakObject * > ( this ) );

    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }
    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc && nRows && nColumns)
    {
        SwUnoInternalPaM aPam(*pDoc);
        // this now needs to return TRUE
        ::sw::XTextRangeToSwPaM(aPam, xTextRange);

        {
            UnoActionContext aCont( pDoc );

            pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
            const SwTable *pTable = 0;
            if( 0 != aPam.Start()->nContent.GetIndex() )
            {
                pDoc->getIDocumentContentOperations().SplitNode(*aPam.Start(), false );
            }
            //TODO: if it is the last paragraph than add another one!
            if( aPam.HasMark() )
            {
                pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
                aPam.DeleteMark();
            }
            pTable = pDoc->InsertTable( SwInsertTableOptions( tabopts::HEADLINE | tabopts::DEFAULT_BORDER | tabopts::SPLIT_LAYOUT, 0 ),
                                        *aPam.GetPoint(),
                                        nRows,
                                        nColumns,
                                        text::HoriOrientation::FULL );
            if(pTable)
            {
                // here, the properties of the descriptor need to be analyzed
                pTableProps->ApplyTblAttr(*pTable, *pDoc);
                SwFrmFmt* pTblFmt = pTable->GetFrmFmt();
                lcl_FormatTable( pTblFmt );

                pTblFmt->Add(this);
                if(!m_sTableName.isEmpty())
                {
                    sal_uInt16 nIndex = 1;
                    OUString sTmpNameIndex(m_sTableName);
                    while(pDoc->FindTblFmtByName( sTmpNameIndex, true ) && nIndex < USHRT_MAX)
                    {
                        sTmpNameIndex = m_sTableName + OUString::number(nIndex++);
                    }
                    pDoc->SetTableName( *pTblFmt, sTmpNameIndex);
                }

                const::uno::Any* pName;
                if(pTableProps->GetProperty(FN_UNO_TABLE_NAME, 0, pName))
                {
                    OUString sTmp;
                    (*pName) >>= sTmp;
                    setName(sTmp);
                }
                bIsDescriptor = false;
                DELETEZ(pTableProps);
            }
            pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
        }
    }
    else
        throw lang::IllegalArgumentException();
}

void SwXTextTable::attach(const uno::Reference< text::XTextRange > & xTextRange)
        throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange >  SwXTextTable::getAnchor(void)
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt)
        throw uno::RuntimeException();
    uno::Reference< text::XTextRange >  xRet = new SwXTextRange(*pFmt);
    return xRet;
}

void SwXTextTable::dispose(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableSortBoxes& rBoxes = pTable->GetTabSortBoxes();
        SwSelBoxes aSelBoxes;
        for(SwTableSortBoxes::const_iterator it = rBoxes.begin(); it != rBoxes.end(); ++it )
            aSelBoxes.insert( *it );
        pFmt->GetDoc()->DeleteRowCol(aSelBoxes);
    }
    else
        throw uno::RuntimeException();
}

void SAL_CALL SwXTextTable::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.addInterface(
            cppu::UnoType<lang::XEventListener>::get(), xListener);
}

void SAL_CALL SwXTextTable::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.removeInterface(
            cppu::UnoType<lang::XEventListener>::get(), xListener);
}

uno::Reference< table::XCell >  SwXTextTable::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    // sheet is unimportant
    if(nColumn >= 0 && nRow >= 0 && nColumn < USHRT_MAX && nRow < USHRT_MAX && pFmt)
    {
        SwXCell* pXCell = lcl_CreateXCell(pFmt, nColumn, nRow);
        if(pXCell)
            aRef = pXCell;
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;
}

uno::Reference< table::XCellRange >  SwXTextTable::GetRangeByName(SwFrmFmt* pFmt, SwTable* pTable,
                    const OUString& rTLName, const OUString& rBRName,
                    SwRangeDescriptor& rDesc)
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRef;
    const SwTableBox* pTLBox = pTable->GetTblBox( rTLName );
    if(pTLBox)
    {
        // invalidate all actions
        UnoActionRemoveContext aRemoveContext(pFmt->GetDoc());
        const SwStartNode* pSttNd = pTLBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        // set cursor to the upper-left cell of the range
        SwUnoCrsr* pUnoCrsr = pFmt->GetDoc()->CreateUnoCrsr(aPos, true);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        pUnoCrsr->SetRemainInSection( false );
        const SwTableBox* pBRBox = pTable->GetTblBox( rBRName );
        if(pBRBox)
        {
            pUnoCrsr->SetMark();
            pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
            pUnoCrsr->Move( fnMoveForward, fnGoNode );
            SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
            pCrsr->MakeBoxSels();
            // pUnoCrsr will be provided and will not be deleted
            SwXCellRange* pCellRange = new SwXCellRange(pUnoCrsr, *pFmt, rDesc);
            aRef = pCellRange;
        }
        else
            delete pUnoCrsr;
    }
    return aRef;
}

uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByPosition(sal_Int32 nLeft, sal_Int32 nTop,
                sal_Int32 nRight, sal_Int32 nBottom)
    throw (uno::RuntimeException,
           lang::IndexOutOfBoundsException,
           std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt && nRight < USHRT_MAX && nBottom < USHRT_MAX &&
        nLeft <= nRight && nTop <= nBottom &&
            nLeft >= 0 && nRight >= 0 && nTop >= 0 && nBottom >= 0 )
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aDesc;
            aDesc.nTop    = nTop;
            aDesc.nBottom = nBottom;
            aDesc.nLeft   = nLeft;
            aDesc.nRight  = nRight;
            const OUString sTLName = sw_GetCellName(aDesc.nLeft, aDesc.nTop);
            const OUString sBRName = sw_GetCellName(aDesc.nRight, aDesc.nBottom);

            // please note that according to the 'if' statement at the begin
            // sTLName:sBRName already denotes the normalized range string

            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;
}

uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByName(const OUString& sRange)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            sal_Int32 nPos = 0;
            const OUString sTLName(sRange.getToken(0, ':', nPos));
            const OUString sBRName(sRange.getToken(0, ':', nPos));
            if(sTLName.isEmpty() || sBRName.isEmpty())
                throw uno::RuntimeException();
            SwRangeDescriptor aDesc;
            aDesc.nTop = aDesc.nLeft = aDesc.nBottom = aDesc.nRight = -1;
            sw_GetCellPosition(sTLName, aDesc.nLeft, aDesc.nTop );
            sw_GetCellPosition(sBRName, aDesc.nRight, aDesc.nBottom );

            // we should normalize the range now (e.g. A5:C1 will become A1:C5)
            // since (depending on what is done later) it will be troublesome
            // elsewhere when the cursor in the implementation does not
            // point to the top-left and bottom-right cells
            aDesc.Normalize();

            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw uno::RuntimeException();
    return aRef;
}

uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXTextTable::getDataArray()
    throw (uno::RuntimeException, std::exception)
{
    // see SwXTextTable::getData(...) also

    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    uno::Sequence< uno::Sequence< uno::Any > > aRowSeq(nRowCount);
    if(pFmt)
    {
        uno::Sequence< uno::Any > * pRowArray = aRowSeq.getArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence< uno::Any >  aColSeq(nColCount);
            uno::Any * pColArray = aColSeq.getArray();
            uno::Reference< table::XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell* pXCell = lcl_CreateXCell(pFmt, nCol, nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    // check if table box value item is set
                    SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                    bool bIsNum = pBoxFmt->GetItemState( RES_BOXATR_VALUE, false ) == SfxItemState::SET;
                    if(!bIsNum)
                        pColArray[nCol] <<= lcl_getString(*pXCell);
                    else
                        pColArray[nCol] <<= sw_getValue(*pXCell);
                }
            }
            pRowArray[nRow] = aColSeq;
        }
    }
    else
        throw uno::RuntimeException();
    return aRowSeq;
}

void SAL_CALL SwXTextTable::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException, std::exception)
{
    // see SwXTextTable::setData(...) also

    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();

    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(pTable->IsTblComplex())
        {
            uno::RuntimeException aRuntime;
            aRuntime.Message = "Table too complex";
            throw aRuntime;
        }

        if(rArray.getLength() != nRowCount)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< uno::Any >* pRowArray = rArray.getConstArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< uno::Any >& rColSeq = pRowArray[nRow];
            if(rColSeq.getLength() != nColCount)
            {
                throw uno::RuntimeException();
            }
            const uno::Any * pColArray = rColSeq.getConstArray();
            uno::Reference< table::XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell* pXCell = lcl_CreateXCell(pFmt, nCol, nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    const uno::Any &rAny = pColArray[nCol];
                    if (uno::TypeClass_STRING == rAny.getValueTypeClass())
                        sw_setString( *pXCell, *(OUString *) rAny.getValue() );
                    else
                    {
                        double d = 0;
                        // #i20067# don't throw exception just do nothing if
                        // there is no value set
                        if( (rAny >>= d) )
                            sw_setValue( *pXCell, d );
                        else
                            sw_setString( *pXCell, OUString(), true );

                    }
                }
            }
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXTextTable::getData(void)
                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }

    SwFrmFmt* pFmt = GetFrmFmt();
    uno::Sequence< uno::Sequence< double > > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
    if(pFmt)
    {
        uno::Sequence< double >* pArray = aRowSeq.getArray();

        const sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            uno::Sequence< double >  aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
            double* pColArray = aColSeq.getArray();
            const sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                pColArray[nCol - nColStart] = xCell->getValue();
            }
            pArray[nRow - nRowStart] = aColSeq;
        }
    }
    else
        throw uno::RuntimeException();
    return aRowSeq;
}

void SwXTextTable::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();

    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }

    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        bool bChanged = false;

        const sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        if(rData.getLength() < nRowCount - nRowStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pRowArray = rData.getConstArray();
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< double >& rColSeq = pRowArray[nRow - nRowStart];
            const sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            if(rColSeq.getLength() < nColCount - nColStart)
            {
                throw uno::RuntimeException();
            }
            const double * pColArray = rColSeq.getConstArray();
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                xCell->setValue(pColArray[nCol - nColStart]);
                bChanged=true;
            }
        }
        if ( bChanged )
        {
            lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
        }
    }
}

uno::Sequence< OUString > SwXTextTable::getRowDescriptions(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    if(!nRowCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstColumnAsLabel ? nRowCount - 1 : nRowCount);

    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstColumnAsLabel)
        {
            const sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    //exception ...
                    break;
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            OSL_FAIL("Where do these labels come from?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const sal_uInt16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < (bFirstRowAsLabel ? nRowCount - 1 : nRowCount))
        {
            throw uno::RuntimeException();
        }
        const OUString* pArray = rRowDesc.getConstArray();
        if(bFirstColumnAsLabel)
        {
            const sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            OSL_FAIL("Where to put theses labels?");
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Sequence< OUString > SwXTextTable::getColumnDescriptions(void)
                                                throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nColCount = getColumnCount();
    if(!nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstRowAsLabel ? nColCount - 1 : nColCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstRowAsLabel)
        {
            const sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);

                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            OSL_FAIL("Where do these labels come from?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::setColumnDescriptions(const uno::Sequence< OUString >& rColumnDesc) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nColCount = getColumnCount();
    if(!nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pArray = rColumnDesc.getConstArray();
        if(bFirstRowAsLabel && rColumnDesc.getLength() >= nColCount - (bFirstColumnAsLabel ? 1 : 0))
        {
            const sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            OSL_FAIL("Where do these labels come from?");
        }
    }
    else
        throw uno::RuntimeException();
}

void SAL_CALL SwXTextTable::addChartDataChangeEventListener(
    const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.addInterface(
            cppu::UnoType<chart::XChartDataChangeEventListener>::get(), xListener);
}

void SAL_CALL SwXTextTable::removeChartDataChangeEventListener(
    const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.removeInterface(
            cppu::UnoType<chart::XChartDataChangeEventListener>::get(), xListener);
}

sal_Bool SwXTextTable::isNotANumber(double nNumber) throw( uno::RuntimeException, std::exception )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return ( nNumber == DBL_MIN );
}

double SwXTextTable::getNotANumber(void) throw( uno::RuntimeException, std::exception )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return DBL_MIN;
}

uno::Sequence< beans::PropertyValue > SwXTextTable::createSortDescriptor(void)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    return SwUnoCursorHelper::CreateSortDescriptor(true);
}

void SwXTextTable::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwSortOptions aSortOpt;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt &&
        SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwSelBoxes aBoxes;
        const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
        for (size_t n = 0; n < rTBoxes.size(); ++n)
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.insert( pBox );
        }
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(aBoxes, aSortOpt);
    }
}

void SwXTextTable::autoFormat(const OUString& sAutoFmtName)
    throw (lang::IllegalArgumentException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableAutoFmtTbl aAutoFmtTbl;
            aAutoFmtTbl.Load();
            for (size_t i = aAutoFmtTbl.size(); i;)
                if( sAutoFmtName == aAutoFmtTbl[ --i ].GetName() )
                {
                    SwSelBoxes aBoxes;
                    const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
                    for (size_t n = 0; n < rTBoxes.size(); ++n)
                    {
                        SwTableBox* pBox = rTBoxes[ n ];
                        aBoxes.insert( pBox );
                    }
                    UnoActionContext aContext( pFmt->GetDoc() );
                    pFmt->GetDoc()->SetTableAutoFmt( aBoxes, aAutoFmtTbl[i] );
                    break;
                }
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTable::getPropertySetInfo(void) throw( uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXTextTable::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
          lang::IllegalArgumentException, lang::WrappedTargetException,
          uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!aValue.hasValue())
        throw lang::IllegalArgumentException();
    const SfxItemPropertySimpleEntry* pEntry =
                                m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if( !pEntry )
        throw lang::IllegalArgumentException();
    if(pFmt)
    {
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        if(0xFF == pEntry->nMemberId)
        {
            lcl_SetSpecialProperty(pFmt, pEntry, aValue);
        }
        else
        {
            switch(pEntry->nWID)
            {
                case FN_UNO_TABLE_NAME :
                {
                    OUString sName;
                    aValue >>= sName;
                    setName( sName );
                }
                break;

                case FN_UNO_RANGE_ROW_LABEL:
                {
                    bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstRowAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;

                case FN_UNO_RANGE_COL_LABEL:
                {
                    bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstColumnAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
                        bFirstColumnAsLabel = bTmp;
                    }
                }
                break;

                case FN_UNO_TABLE_BORDER:
                case FN_UNO_TABLE_BORDER2:
                {
                    table::TableBorder oldBorder;
                    table::TableBorder2 aBorder;
                    SvxBorderLine aTopLine;
                    SvxBorderLine aBottomLine;
                    SvxBorderLine aLeftLine;
                    SvxBorderLine aRightLine;
                    SvxBorderLine aHoriLine;
                    SvxBorderLine aVertLine;
                    if (aValue >>= oldBorder)
                    {
                        aBorder.IsTopLineValid = oldBorder.IsTopLineValid;
                        aBorder.IsBottomLineValid = oldBorder.IsBottomLineValid;
                        aBorder.IsLeftLineValid = oldBorder.IsLeftLineValid;
                        aBorder.IsRightLineValid = oldBorder.IsRightLineValid;
                        aBorder.IsHorizontalLineValid = oldBorder.IsHorizontalLineValid;
                        aBorder.IsVerticalLineValid = oldBorder.IsVerticalLineValid;
                        aBorder.Distance = oldBorder.Distance;
                        aBorder.IsDistanceValid = oldBorder.IsDistanceValid;
                        lcl_LineToSvxLine(
                                oldBorder.TopLine, aTopLine);
                        lcl_LineToSvxLine(
                                oldBorder.BottomLine, aBottomLine);
                        lcl_LineToSvxLine(
                                oldBorder.LeftLine, aLeftLine);
                        lcl_LineToSvxLine(
                                oldBorder.RightLine, aRightLine);
                        lcl_LineToSvxLine(
                                oldBorder.HorizontalLine, aHoriLine);
                        lcl_LineToSvxLine(
                                oldBorder.VerticalLine, aVertLine);
                    }
                    else if (aValue >>= aBorder)
                    {
                        SvxBoxItem::LineToSvxLine(
                                aBorder.TopLine, aTopLine, true);
                        SvxBoxItem::LineToSvxLine(
                                aBorder.BottomLine, aBottomLine, true);
                        SvxBoxItem::LineToSvxLine(
                                aBorder.LeftLine, aLeftLine, true);
                        SvxBoxItem::LineToSvxLine(
                                aBorder.RightLine, aRightLine, true);
                        SvxBoxItem::LineToSvxLine(
                                aBorder.HorizontalLine, aHoriLine, true);
                        SvxBoxItem::LineToSvxLine(
                                aBorder.VerticalLine, aVertLine, true);
                    }
                    else
                    {
                        break; // something else
                    }
                    SwDoc* pDoc = pFmt->GetDoc();
                    SwFrm* pFrm = SwIterator<SwFrm,SwFmt>::FirstElement( *pFmt );
                    // tables without layout (invisible header/footer?)
                    if (!pFrm)
                    {
                        break;
                    }
                    lcl_FormatTable(pFmt);
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableLines &rLines = pTable->GetTabLines();

                    // invalidate all actions
                    UnoActionRemoveContext aRemoveContext(pDoc);
                    const SwTableBox* pTLBox = lcl_FindCornerTableBox(rLines, true);
                    const SwStartNode* pSttNd = pTLBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    // set cursor to top left cell
                    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, true);
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    pUnoCrsr->SetRemainInSection( false );

                    const SwTableBox* pBRBox = lcl_FindCornerTableBox(rLines, false);
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr& rCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
                    rCrsr.MakeBoxSels();

                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);

                    SvxBoxItem aBox( RES_BOX );
                    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );

                    aBox.SetLine(aTopLine.isEmpty() ? 0 : &aTopLine, BOX_LINE_TOP);
                    aBoxInfo.SetValid(VALID_TOP, aBorder.IsTopLineValid);

                    aBox.SetLine(aBottomLine.isEmpty() ? 0 : &aBottomLine, BOX_LINE_BOTTOM);
                    aBoxInfo.SetValid(VALID_BOTTOM, aBorder.IsBottomLineValid);

                    aBox.SetLine(aLeftLine.isEmpty() ? 0 : &aLeftLine, BOX_LINE_LEFT);
                    aBoxInfo.SetValid(VALID_LEFT, aBorder.IsLeftLineValid);

                    aBox.SetLine(aRightLine.isEmpty() ? 0 : &aRightLine, BOX_LINE_RIGHT);
                    aBoxInfo.SetValid(VALID_RIGHT, aBorder.IsRightLineValid);

                    aBoxInfo.SetLine(aHoriLine.isEmpty() ? 0 : &aHoriLine, BOXINFO_LINE_HORI);
                    aBoxInfo.SetValid(VALID_HORI, aBorder.IsHorizontalLineValid);

                    aBoxInfo.SetLine(aVertLine.isEmpty() ? 0 : &aVertLine, BOXINFO_LINE_VERT);
                    aBoxInfo.SetValid(VALID_VERT, aBorder.IsVerticalLineValid);

                    aBox.SetDistance((sal_uInt16)convertMm100ToTwip(aBorder.Distance));
                    aBoxInfo.SetValid(VALID_DISTANCE, aBorder.IsDistanceValid);

                    aSet.Put(aBox);
                    aSet.Put(aBoxInfo);

                    pDoc->SetTabBorders(rCrsr, aSet);
                    delete pUnoCrsr;
                }
                break;

                case FN_UNO_TABLE_BORDER_DISTANCES:
                {
                    table::TableBorderDistances aTableBorderDistances;
                    if( !(aValue >>= aTableBorderDistances) ||
                        (!aTableBorderDistances.IsLeftDistanceValid &&
                        !aTableBorderDistances.IsRightDistanceValid &&
                        !aTableBorderDistances.IsTopDistanceValid &&
                        !aTableBorderDistances.IsBottomDistanceValid ))
                        break;

                    const sal_uInt16 nLeftDistance =   convertMm100ToTwip(aTableBorderDistances.LeftDistance);
                    const sal_uInt16 nRightDistance =  convertMm100ToTwip(aTableBorderDistances.RightDistance);
                    const sal_uInt16 nTopDistance =    convertMm100ToTwip(aTableBorderDistances.TopDistance);
                    const sal_uInt16 nBottomDistance = convertMm100ToTwip(aTableBorderDistances.BottomDistance);
                    SwDoc* pDoc = pFmt->GetDoc();
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableLines &rLines = pTable->GetTabLines();
                    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_START, NULL);
                    for(size_t i = 0; i < rLines.size(); ++i)
                    {
                        SwTableLine* pLine = rLines[i];
                        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                        for(size_t k = 0; k < rBoxes.size(); ++k)
                        {
                            SwTableBox* pBox = rBoxes[k];
                            const SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                            const SvxBoxItem& rBox = pBoxFmt->GetBox();
                            if(
                                (aTableBorderDistances.IsLeftDistanceValid && nLeftDistance !=   rBox.GetDistance( BOX_LINE_LEFT )) ||
                                (aTableBorderDistances.IsRightDistanceValid && nRightDistance !=  rBox.GetDistance( BOX_LINE_RIGHT )) ||
                                (aTableBorderDistances.IsTopDistanceValid && nTopDistance !=    rBox.GetDistance( BOX_LINE_TOP )) ||
                                (aTableBorderDistances.IsBottomDistanceValid && nBottomDistance != rBox.GetDistance( BOX_LINE_BOTTOM )))
                            {
                                SvxBoxItem aSetBox( rBox );
                                SwFrmFmt* pSetBoxFmt = pBox->ClaimFrmFmt();
                                if( aTableBorderDistances.IsLeftDistanceValid )
                                    aSetBox.SetDistance( nLeftDistance, BOX_LINE_LEFT );
                                if( aTableBorderDistances.IsRightDistanceValid )
                                    aSetBox.SetDistance( nRightDistance, BOX_LINE_RIGHT );
                                if( aTableBorderDistances.IsTopDistanceValid )
                                    aSetBox.SetDistance( nTopDistance, BOX_LINE_TOP );
                                if( aTableBorderDistances.IsBottomDistanceValid )
                                    aSetBox.SetDistance( nBottomDistance, BOX_LINE_BOTTOM );
                                pDoc->SetAttr( aSetBox, *pSetBoxFmt );
                            }
                        }
                    }
                    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, NULL);
                }
                break;

                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    UnoActionContext aContext(pFmt->GetDoc());
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    lcl_SetTblSeparators(aValue, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], false, pFmt->GetDoc());
                }
                break;

                case FN_UNO_TABLE_COLUMN_RELATIVE_SUM:/*_readonly_*/ break;

                default:
                {
                    SwAttrSet aSet(pFmt->GetAttrSet());
                    m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                    pFmt->GetDoc()->SetAttr(aSet, *pFmt);
                }
            }
        }
    }
    else if(bIsDescriptor)
    {
        pTableProps->SetProperty( pEntry->nWID, pEntry->nMemberId, aValue);
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXTextTable::getPropertyValue(const OUString& rPropertyName)
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    const SfxItemPropertySimpleEntry* pEntry =
                                m_pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pFmt)
    {
        if(0xFF == pEntry->nMemberId)
        {
            aRet = lcl_GetSpecialProperty(pFmt, pEntry );
        }
        else
        {
            switch(pEntry->nWID)
            {
                case FN_UNO_TABLE_NAME:
                {
                    aRet <<= getName();
                }
                break;

                case  FN_UNO_ANCHOR_TYPES:
                case  FN_UNO_TEXT_WRAP:
                case  FN_UNO_ANCHOR_TYPE:
                    ::sw::GetDefaultTextContentValue(
                            aRet, OUString(), pEntry->nWID);
                break;

                case FN_UNO_RANGE_ROW_LABEL:
                {
                    aRet <<= bFirstRowAsLabel;
                }
                break;

                case FN_UNO_RANGE_COL_LABEL:
                    aRet <<= bFirstColumnAsLabel;
                break;

                case FN_UNO_TABLE_BORDER:
                case FN_UNO_TABLE_BORDER2:
                {
                    SwDoc* pDoc = pFmt->GetDoc();
                    SwFrm* pFrm = SwIterator<SwFrm,SwFmt>::FirstElement( *pFmt );
                    // tables without layout (invisible header/footer?)
                    if (!pFrm)
                    {
                        break;
                    }
                    lcl_FormatTable(pFmt);
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableLines &rLines = pTable->GetTabLines();

                    // invalidate all actions
                    UnoActionRemoveContext aRemoveContext(pDoc);
                    const SwTableBox* pTLBox = lcl_FindCornerTableBox(rLines, true);
                    const SwStartNode* pSttNd = pTLBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    // set cursor to top left cell
                    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, true);
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    pUnoCrsr->SetRemainInSection( false );

                    const SwTableBox* pBRBox = lcl_FindCornerTableBox(rLines, false);
                    pUnoCrsr->SetMark();
                    const SwStartNode* pLastNd = pBRBox->GetSttNd();
                    pUnoCrsr->GetPoint()->nNode = *pLastNd;

                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr& rCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
                    rCrsr.MakeBoxSels();

                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                    pDoc->GetTabBorders(rCrsr, aSet);
                    const SvxBoxInfoItem& rBoxInfoItem =
                        static_cast<const SvxBoxInfoItem&>(aSet.Get(SID_ATTR_BORDER_INNER));
                    const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX));

                    if (FN_UNO_TABLE_BORDER == pEntry->nWID)
                    {
                        table::TableBorder aTableBorder;
                        aTableBorder.TopLine                = SvxBoxItem::SvxLineToLine(rBox.GetTop(), true);
                        aTableBorder.IsTopLineValid         = rBoxInfoItem.IsValid(VALID_TOP);
                        aTableBorder.BottomLine             = SvxBoxItem::SvxLineToLine(rBox.GetBottom(), true);
                        aTableBorder.IsBottomLineValid      = rBoxInfoItem.IsValid(VALID_BOTTOM);
                        aTableBorder.LeftLine               = SvxBoxItem::SvxLineToLine(rBox.GetLeft(), true);
                        aTableBorder.IsLeftLineValid        = rBoxInfoItem.IsValid(VALID_LEFT);
                        aTableBorder.RightLine              = SvxBoxItem::SvxLineToLine(rBox.GetRight(), true);
                        aTableBorder.IsRightLineValid       = rBoxInfoItem.IsValid(VALID_RIGHT );
                        aTableBorder.HorizontalLine         = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetHori(), true);
                        aTableBorder.IsHorizontalLineValid  = rBoxInfoItem.IsValid(VALID_HORI);
                        aTableBorder.VerticalLine           = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetVert(), true);
                        aTableBorder.IsVerticalLineValid    = rBoxInfoItem.IsValid(VALID_VERT);
                        aTableBorder.Distance               = convertTwipToMm100( rBox.GetDistance() );
                        aTableBorder.IsDistanceValid        = rBoxInfoItem.IsValid(VALID_DISTANCE);
                        aRet <<= aTableBorder;
                    }
                    else
                    {
                        table::TableBorder2 aTableBorder;
                        aTableBorder.TopLine                = SvxBoxItem::SvxLineToLine(rBox.GetTop(), true);
                        aTableBorder.IsTopLineValid         = rBoxInfoItem.IsValid(VALID_TOP);
                        aTableBorder.BottomLine             = SvxBoxItem::SvxLineToLine(rBox.GetBottom(), true);
                        aTableBorder.IsBottomLineValid      = rBoxInfoItem.IsValid(VALID_BOTTOM);
                        aTableBorder.LeftLine               = SvxBoxItem::SvxLineToLine(rBox.GetLeft(), true);
                        aTableBorder.IsLeftLineValid        = rBoxInfoItem.IsValid(VALID_LEFT);
                        aTableBorder.RightLine              = SvxBoxItem::SvxLineToLine(rBox.GetRight(), true);
                        aTableBorder.IsRightLineValid       = rBoxInfoItem.IsValid(VALID_RIGHT );
                        aTableBorder.HorizontalLine         = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetHori(), true);
                        aTableBorder.IsHorizontalLineValid  = rBoxInfoItem.IsValid(VALID_HORI);
                        aTableBorder.VerticalLine           = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetVert(), true);
                        aTableBorder.IsVerticalLineValid    = rBoxInfoItem.IsValid(VALID_VERT);
                        aTableBorder.Distance               = convertTwipToMm100( rBox.GetDistance() );
                        aTableBorder.IsDistanceValid        = rBoxInfoItem.IsValid(VALID_DISTANCE);
                        aRet <<= aTableBorder;
                    }
                    delete pUnoCrsr;
                }
                break;

                case FN_UNO_TABLE_BORDER_DISTANCES :
                {
                    table::TableBorderDistances aTableBorderDistances( 0, sal_True, 0, sal_True, 0, sal_True, 0, sal_True ) ;
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    const SwTableLines &rLines = pTable->GetTabLines();
                    bool bFirst = true;
                    sal_uInt16 nLeftDistance = 0;
                    sal_uInt16 nRightDistance = 0;
                    sal_uInt16 nTopDistance = 0;
                    sal_uInt16 nBottomDistance = 0;

                    for(size_t i = 0; i < rLines.size(); ++i)
                    {
                        const SwTableLine* pLine = rLines[i];
                        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                        for(size_t k = 0; k < rBoxes.size(); ++k)
                        {
                            const SwTableBox* pBox = rBoxes[k];
                            SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                            const SvxBoxItem& rBox = pBoxFmt->GetBox();
                            if( bFirst )
                            {
                                nLeftDistance =     convertTwipToMm100( rBox.GetDistance( BOX_LINE_LEFT   ));
                                nRightDistance =    convertTwipToMm100( rBox.GetDistance( BOX_LINE_RIGHT  ));
                                nTopDistance =      convertTwipToMm100( rBox.GetDistance( BOX_LINE_TOP    ));
                                nBottomDistance =   convertTwipToMm100( rBox.GetDistance( BOX_LINE_BOTTOM ));
                                bFirst = false;
                            }
                            else
                            {
                                if( aTableBorderDistances.IsLeftDistanceValid &&
                                    nLeftDistance != convertTwipToMm100( rBox.GetDistance( BOX_LINE_LEFT   )))
                                    aTableBorderDistances.IsLeftDistanceValid = sal_False;
                                if( aTableBorderDistances.IsRightDistanceValid &&
                                    nRightDistance != convertTwipToMm100( rBox.GetDistance( BOX_LINE_RIGHT   )))
                                    aTableBorderDistances.IsRightDistanceValid = sal_False;
                                if( aTableBorderDistances.IsTopDistanceValid &&
                                    nTopDistance != convertTwipToMm100( rBox.GetDistance( BOX_LINE_TOP   )))
                                    aTableBorderDistances.IsTopDistanceValid = sal_False;
                                if( aTableBorderDistances.IsBottomDistanceValid &&
                                    nBottomDistance != convertTwipToMm100( rBox.GetDistance( BOX_LINE_BOTTOM   )))
                                    aTableBorderDistances.IsBottomDistanceValid = sal_False;
                            }

                        }
                        if( !aTableBorderDistances.IsLeftDistanceValid &&
                                !aTableBorderDistances.IsRightDistanceValid &&
                                !aTableBorderDistances.IsTopDistanceValid &&
                                !aTableBorderDistances.IsBottomDistanceValid )
                            break;
                    }
                    if( aTableBorderDistances.IsLeftDistanceValid)
                        aTableBorderDistances.LeftDistance = nLeftDistance;
                    if( aTableBorderDistances.IsRightDistanceValid)
                        aTableBorderDistances.RightDistance  = nRightDistance;
                    if( aTableBorderDistances.IsTopDistanceValid)
                        aTableBorderDistances.TopDistance    = nTopDistance;
                    if( aTableBorderDistances.IsBottomDistanceValid)
                        aTableBorderDistances.BottomDistance = nBottomDistance;

                    aRet <<= aTableBorderDistances;
                }
                break;

                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    lcl_GetTblSeparators(aRet, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], false);
                }
                break;

                case FN_UNO_TABLE_COLUMN_RELATIVE_SUM:
                    aRet <<= (sal_Int16) UNO_TABLE_COLUMN_SUM;
                break;

                case RES_ANCHOR:
                    // AnchorType is readonly and might be void (no return value)
                break;

                case FN_UNO_TEXT_SECTION:
                {
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableNode* pTblNode = pTable->GetTableNode();
                    SwSectionNode* pSectionNode =  pTblNode->FindSectionNode();
                    if(pSectionNode)
                    {
                        const SwSection& rSect = pSectionNode->GetSection();
                        uno::Reference< text::XTextSection >  xSect =
                                        SwXTextSections::GetObject( *rSect.GetFmt() );
                        aRet <<= xSect;
                    }
                }
                break;

                default:
                {
                    const SwAttrSet& rSet = pFmt->GetAttrSet();
                    m_pPropSet->getPropertyValue(*pEntry, rSet, aRet);
                }
            }
        }
    }
    else if(bIsDescriptor)
    {
        const uno::Any* pAny = 0;
        if(!pTableProps->GetProperty(pEntry->nWID, pEntry->nMemberId, pAny))
            throw lang::IllegalArgumentException();
        else if(pAny)
            aRet = *pAny;
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTable::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTable::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXTextTable::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

OUString SwXTextTable::getName(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt && !bIsDescriptor)
        throw uno::RuntimeException();
    if(pFmt)
    {
        return pFmt->GetName();
    }
    return m_sTableName;
}

void SwXTextTable::setName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if((!pFmt && !bIsDescriptor) ||
       rName.isEmpty() ||
       rName.indexOf('.')>=0 ||
       rName.indexOf(' ')>=0 )
        throw uno::RuntimeException();

    if(pFmt)
    {
        const OUString aOldName( pFmt->GetName() );
        const SwFrmFmts* pTbl = pFmt->GetDoc()->GetTblFrmFmts();
        for( size_t i = pTbl->size(); i; )
        {
            const SwFrmFmt* pTmpFmt = (*pTbl)[ --i ] ;
            if( !pTmpFmt->IsDefault() &&
                pTmpFmt->GetName() == rName &&
                            pFmt->GetDoc()->IsUsed( *pTmpFmt ))
            {
                throw uno::RuntimeException();
            }
        }

        pFmt->SetName( rName );

        SwStartNode *pStNd;
        SwNodeIndex aIdx( *pFmt->GetDoc()->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
        {
            ++aIdx;
            SwNode *const pNd = & aIdx.GetNode();
            if ( pNd->IsOLENode() &&
                aOldName == static_cast<const SwOLENode*>(pNd)->GetChartTblName() )
            {
                const_cast<SwOLENode*>(static_cast<const SwOLENode*>(pNd))->SetChartTblName( rName );

                static_cast<SwOLENode*>(pNd)->GetOLEObj();

                SwTable* pTable = SwTable::FindTable( pFmt );
                //TL_CHART2: chart needs to be notfied about name changes
                pFmt->GetDoc()->UpdateCharts( pTable->GetFrmFmt()->GetName() );
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
        pFmt->GetDoc()->getIDocumentState().SetModified();
    }
    else
        m_sTableName = rName;
}

sal_uInt16 SwXTextTable::getRowCount(void)
{
    SolarMutexGuard aGuard;
    sal_uInt16 nRet = 0;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            nRet = pTable->GetTabLines().size();
        }
    }
    return nRet;
}

sal_uInt16 SwXTextTable::getColumnCount(void)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    sal_uInt16 nRet = 0;
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.front();
            nRet = pLine->GetTabBoxes().size();
        }
    }
    return nRet;
}

void SwXTextTable::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    if(pOld && pOld->Which() == RES_REMOVE_UNO_OBJECT &&
        (void*)GetRegisteredIn() == static_cast<const SwPtrMsgPoolItem *>(pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
    else
        ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        uno::Reference<uno::XInterface> const xThis(m_pImpl->m_wThis);
        if (!xThis.is())
        {   // fdo#72695: if UNO object is already dead, don't revive it with event
            return;
        }
        lang::EventObject const ev(xThis);
        m_pImpl->m_Listeners.disposeAndClear(ev);
    }
    else
    {
        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
    }
}

OUString SAL_CALL SwXTextTable::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextTable");
}

sal_Bool SwXTextTable::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextTable::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(4);
    OUString* pArr = aRet.getArray();
    pArr[0] = "com.sun.star.document.LinkTarget";
    pArr[1] = "com.sun.star.text.TextTable";
    pArr[2] = "com.sun.star.text.TextContent";
    pArr[2] = "com.sun.star.text.TextSortable";
    return aRet;
}

namespace
{
    class theSwXCellRangeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXCellRangeUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXCellRange::getUnoTunnelId()
{
    return theSwXCellRangeUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXCellRange::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

TYPEINIT1(SwXCellRange, SwClient);

OUString SwXCellRange::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXCellRange");
}

sal_Bool SwXCellRange::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXCellRange::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(7);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.CellRange";
    pArray[1] = "com.sun.star.style.CharacterProperties";
    pArray[2] = "com.sun.star.style.CharacterPropertiesAsian";
    pArray[3] = "com.sun.star.style.CharacterPropertiesComplex";
    pArray[4] = "com.sun.star.style.ParagraphProperties";
    pArray[5] = "com.sun.star.style.ParagraphPropertiesAsian";
    pArray[6] = "com.sun.star.style.ParagraphPropertiesComplex";
    return aRet;
}

SwXCellRange::SwXCellRange(SwUnoCrsr* pCrsr, SwFrmFmt& rFrmFmt,
    SwRangeDescriptor& rDesc)
    : SwClient(&rFrmFmt)
    , aCursorDepend(this, pCrsr)
    , m_ChartListeners(m_Mutex)
    ,
    aRgDesc(rDesc),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_RANGE)),
    pTblCrsr(pCrsr),
    bFirstRowAsLabel(false),
    bFirstColumnAsLabel(false)
{
    aRgDesc.Normalize();
}

SwXCellRange::~SwXCellRange()
{
    SolarMutexGuard aGuard;
    delete pTblCrsr;
}

uno::Reference< table::XCell >  SwXCellRange::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        if(nColumn >= 0 && nRow >= 0 &&
             getColumnCount() > nColumn && getRowCount() > nRow )
        {
            SwXCell* pXCell = lcl_CreateXCell(pFmt,
                    aRgDesc.nLeft + nColumn, aRgDesc.nTop + nRow);
            if(pXCell)
                aRet = pXCell;
        }
    }
    if(!aRet.is())
        throw lang::IndexOutOfBoundsException();
    return aRet;
}

uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByPosition(
        sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom)
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException,
           std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt && getColumnCount() > nRight && getRowCount() > nBottom &&
        nLeft <= nRight && nTop <= nBottom
        && nLeft >= 0 && nRight >= 0 && nTop >= 0 && nBottom >= 0 )
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            SwRangeDescriptor aNewDesc;
            aNewDesc.nTop    = nTop + aRgDesc.nTop;
            aNewDesc.nBottom = nBottom + aRgDesc.nTop;
            aNewDesc.nLeft   = nLeft + aRgDesc.nLeft;
            aNewDesc.nRight  = nRight + aRgDesc.nLeft;
            aNewDesc.Normalize();
            const OUString sTLName = sw_GetCellName(aNewDesc.nLeft, aNewDesc.nTop);
            const OUString sBRName = sw_GetCellName(aNewDesc.nRight, aNewDesc.nBottom);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                // invalidate all actions
                UnoActionRemoveContext aRemoveContext(pFmt->GetDoc());
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // set cursor in the upper-left cell of the range
                SwUnoCrsr* pUnoCrsr = pFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( false );
                const SwTableBox* pBRBox = pTable->GetTblBox( sBRName );
                if(pBRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();
                    // pUnoCrsr will be provided and will not be deleted
                    SwXCellRange* pCellRange = new SwXCellRange(pUnoCrsr, *pFmt, aNewDesc);
                    aRet = pCellRange;
                }
                else
                    delete pUnoCrsr;
            }
        }
    }
    if(!aRet.is())
        throw lang::IndexOutOfBoundsException();
    return aRet;
}

uno::Reference< table::XCellRange >  SwXCellRange::getCellRangeByName(const OUString& rRange)
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sal_Int32 nPos = 0;
    const OUString sTLName(rRange.getToken(0, ':', nPos));
    const OUString sBRName(rRange.getToken(0, ':', nPos));
    if(sTLName.isEmpty() || sBRName.isEmpty())
        throw uno::RuntimeException();
    SwRangeDescriptor aDesc;
    aDesc.nTop = aDesc.nLeft = aDesc.nBottom = aDesc.nRight = -1;
    sw_GetCellPosition( sTLName, aDesc.nLeft, aDesc.nTop );
    sw_GetCellPosition( sBRName, aDesc.nRight, aDesc.nBottom );
    aDesc.Normalize();
    return getCellRangeByPosition(aDesc.nLeft - aRgDesc.nLeft, aDesc.nTop - aRgDesc.nTop,
                aDesc.nRight - aRgDesc.nLeft, aDesc.nBottom - aRgDesc.nTop);
}

uno::Reference< beans::XPropertySetInfo >  SwXCellRange::getPropertySetInfo(void) throw( uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXCellRange::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw (beans::UnknownPropertyException,
           beans::PropertyVetoException,
           lang::IllegalArgumentException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            SwDoc* pDoc = pTblCrsr->GetDoc();
            {
                // remove actions to enable box selection
                UnoActionRemoveContext aRemoveContext(pDoc);
            }
            SwUnoTableCrsr& rCrsr = dynamic_cast<SwUnoTableCrsr&>(*pTblCrsr);
            rCrsr.MakeBoxSels();
            switch(pEntry->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    pDoc->GetBoxAttr( *pTblCrsr, aBrush );
                    ((SfxPoolItem&)aBrush).PutValue(aValue, pEntry->nMemberId);
                    pDoc->SetBoxAttr( *pTblCrsr, aBrush );

                }
                break;
                case RES_BOX :
                {
                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                    aBoxInfo.SetValid(0xff, false);
                    sal_uInt8 nValid = 0;
                    switch(pEntry->nMemberId & ~CONVERT_TWIPS)
                    {
                        case  LEFT_BORDER :             nValid = VALID_LEFT; break;
                        case  RIGHT_BORDER:             nValid = VALID_RIGHT; break;
                        case  TOP_BORDER  :             nValid = VALID_TOP; break;
                        case  BOTTOM_BORDER:            nValid = VALID_BOTTOM; break;
                        case  LEFT_BORDER_DISTANCE :
                        case  RIGHT_BORDER_DISTANCE:
                        case  TOP_BORDER_DISTANCE  :
                        case  BOTTOM_BORDER_DISTANCE:
                            nValid = VALID_DISTANCE;
                        break;
                    }
                    aBoxInfo.SetValid(nValid, true);

                    aSet.Put(aBoxInfo);
                    pDoc->GetTabBorders(rCrsr, aSet);

                    aSet.Put(aBoxInfo);
                    SvxBoxItem aBoxItem(static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX)));
                    ((SfxPoolItem&)aBoxItem).PutValue(aValue, pEntry->nMemberId);
                    aSet.Put(aBoxItem);
                    pDoc->SetTabBorders( *pTblCrsr, aSet );
                }
                break;
                case RES_BOXATR_FORMAT:
                {
                    SfxUInt32Item aNumberFormat(RES_BOXATR_FORMAT);
                    ((SfxPoolItem&)aNumberFormat).PutValue(aValue, 0);
                    pDoc->SetBoxAttr(rCrsr, aNumberFormat);
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstRowAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_ChartListeners);
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstColumnAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_ChartListeners);
                        bFirstColumnAsLabel = bTmp;
                    }
                }
                break;
                default:
                {
                    SfxItemSet aItemSet( pDoc->GetAttrPool(), pEntry->nWID, pEntry->nWID );
                    SwUnoCursorHelper::GetCrsrAttr(rCrsr.GetSelRing(),
                            aItemSet);

                    if (!SwUnoCursorHelper::SetCursorPropertyValue(
                            *pEntry, aValue, rCrsr.GetSelRing(), aItemSet))
                    {
                        m_pPropSet->setPropertyValue(*pEntry, aValue, aItemSet);
                    }
                    SwUnoCursorHelper::SetCrsrAttr(rCrsr.GetSelRing(),
                            aItemSet, nsSetAttrMode::SETATTR_DEFAULT, true);
                }
            }
        }
        else
            throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXCellRange::getPropertyValue(const OUString& rPropertyName)
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            switch(pEntry->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    if(pTblCrsr->GetDoc()->GetBoxAttr( *pTblCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pEntry->nMemberId);

                }
                break;
                case RES_BOX :
                {
                    SwDoc* pDoc = pTblCrsr->GetDoc();
                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                    pDoc->GetTabBorders(*pTblCrsr, aSet);
                    const SvxBoxItem& rBoxItem = static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX));
                    rBoxItem.QueryValue(aRet, pEntry->nMemberId);
                }
                break;
                case RES_BOXATR_FORMAT:
                    OSL_FAIL("not implemented");
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl *const pTmpFmt =
                        SwUnoCursorHelper::GetCurTxtFmtColl(*pTblCrsr, false);
                    OUString sRet;
                    if(pFmt)
                        sRet = pTmpFmt->GetName();
                    aRet <<= sRet;
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                    aRet <<= bFirstRowAsLabel;
                break;
                case FN_UNO_RANGE_COL_LABEL:
                    aRet <<= bFirstColumnAsLabel;
                break;
                default:
                {
                    SfxItemSet aSet(pTblCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // first look at the attributes of the cursor
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
                    SwUnoCursorHelper::GetCrsrAttr(pCrsr->GetSelRing(), aSet);
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aRet);
                }
            }
        }
        else
           throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXCellRange::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::GetDataSequence(
        uno::Sequence< uno::Any >   *pAnySeq,   //-> first pointer != 0 is used
        uno::Sequence< OUString >   *pTxtSeq,   //-> as output sequence
        uno::Sequence< double >     *pDblSeq,   //-> (previous data gets overwritten)
        bool bForceNumberResults )              //-> when 'true' requires to make an
                                                // extra effort to return a value different
                                                // from 0 even if the cell is formatted to text
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // compare to SwXCellRange::getDataArray (note different return types though)

    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();

    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }

    sal_Int32 nSize = nRowCount * nColCount;
    if (pAnySeq)
        pAnySeq->realloc( nSize );
    else if (pTxtSeq)
        pTxtSeq->realloc( nSize );
    else if (pDblSeq)
        pDblSeq->realloc( nSize );
    else
    {
        OSL_FAIL( "argument missing" );
        return;
    }
    uno::Any   *pAnyData = pAnySeq ? pAnySeq->getArray() : 0;
    OUString   *pTxtData = pTxtSeq ? pTxtSeq->getArray() : 0;
    double     *pDblData = pDblSeq ? pDblSeq->getArray() : 0;

    sal_Int32 nDtaCnt = 0;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        double fNan;
        ::rtl::math::setNan( & fNan );

        uno::Reference< table::XCell > xCellRef;
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    aRgDesc.nLeft + nCol,
                                    aRgDesc.nTop + nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    if (pAnyData)
                    {
                        // check if table box value item is set
                        bool bIsNum = pBox->GetFrmFmt()->GetItemState( RES_BOXATR_VALUE, false ) == SfxItemState::SET;
                        if (!bIsNum)
                            pAnyData[nDtaCnt++] <<= lcl_getString(*pXCell);
                        else
                            pAnyData[nDtaCnt++] <<= sw_getValue(*pXCell);
                    }
                    else if (pTxtData)
                        pTxtData[nDtaCnt++] = lcl_getString(*pXCell);
                    else if (pDblData)
                    {
                        double fVal = fNan;
                        if (!bForceNumberResults || table::CellContentType_TEXT != pXCell->getType())
                            fVal = sw_getValue(*pXCell);
                        else
                        {
                            OSL_ENSURE( table::CellContentType_TEXT == pXCell->getType(),
                                    "this branch of 'if' is only for text formatted cells" );

                            // now we'll try to get a useful numerical value
                            // from the text in the cell...

                            sal_uInt32 nFIndex;
                            SvNumberFormatter* pNumFormatter = pTblCrsr->GetDoc()->GetNumberFormatter();

                            // look for SwTblBoxNumFormat value in parents as well
                            const SfxPoolItem* pItem;
                            SwFrmFmt *pBoxFmt = pXCell->GetTblBox()->GetFrmFmt();
                            SfxItemState eState = pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, true, &pItem);

                            if (eState == SfxItemState::SET)
                            {
                                // please note that the language of the numberformat
                                // is implicitly coded into the below value as well
                                nFIndex = static_cast<const SwTblBoxNumFormat*>(pItem)->GetValue();

                                // since the current value indicates a text format but the call
                                // to 'IsNumberFormat' below won't work for text formats
                                // we need to get rid of the part that indicates the text format.
                                // According to ER this can be done like this:
                                nFIndex -= (nFIndex % SV_COUNTRY_LANGUAGE_OFFSET);
                            }
                            else
                            {
                                // system language is probably not the best possible choice
                                // but since we have to guess anyway (because the language of at
                                // the text is NOT the one used for the number format!)
                                // it is at least conform to what is used in
                                // SwTableShell::Execute when
                                // SID_ATTR_NUMBERFORMAT_VALUE is set...
                                LanguageType eLang = LANGUAGE_SYSTEM;
                                nFIndex = pNumFormatter->GetStandardIndex( eLang );
                            }

                            double fTmp;
                            if (pNumFormatter->IsNumberFormat( lcl_getString(*pXCell), nFIndex, fTmp ))
                                fVal = fTmp;
                        }
                        pDblData[nDtaCnt++] = fVal;
                    }
                    else {
                        OSL_FAIL( "output sequence missing" );
                    }
                }
            }
        }
    }
    OSL_ENSURE( nDtaCnt == nSize, "size mismatch. Invalid cell range?" );
    if (pAnySeq)
        pAnySeq->realloc( nDtaCnt );
    else if (pTxtSeq)
        pTxtSeq->realloc( nDtaCnt );
    else if (pDblSeq)
        pDblSeq->realloc( nDtaCnt );
}

///@see SwXCellRange::getData
///@see SwXCellRange::GetDataSequence
uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXCellRange::getDataArray()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();

    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    uno::Sequence< uno::Sequence< uno::Any > > aRowSeq(nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< uno::Any >* pRowArray = aRowSeq.getArray();
        uno::Reference< table::XCell > xCellRef;
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            uno::Sequence< uno::Any > aColSeq(nColCount);
            uno::Any * pColArray = aColSeq.getArray();
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    aRgDesc.nLeft + nCol,
                                    aRgDesc.nTop + nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    // check if table box value item is set
                    SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                    bool bIsNum = pBoxFmt->GetItemState( RES_BOXATR_VALUE, false ) == SfxItemState::SET;
                    if(!bIsNum)
                        pColArray[nCol] <<= lcl_getString(*pXCell);
                    else
                        pColArray[nCol] <<= sw_getValue(*pXCell);
                }
            }
            pRowArray[nRow] = aColSeq;
        }
    }
    return aRowSeq;
}

///@see SwXCellRange::setData
void SAL_CALL SwXCellRange::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        if(rArray.getLength() != nRowCount)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< uno::Any >* pRowArray = rArray.getConstArray();
        for(sal_uInt16 nRow = 0; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< uno::Any >& rColSeq = pRowArray[nRow];
            if(rColSeq.getLength() != nColCount)
            {
                throw uno::RuntimeException();
            }
            const uno::Any * pColArray = rColSeq.getConstArray();
            uno::Reference< table::XCell > xCellRef;
            for(sal_uInt16 nCol = 0; nCol < nColCount; nCol++)
            {
                SwXCell * pXCell = lcl_CreateXCell(pFmt,
                                    aRgDesc.nLeft + nCol,
                                    aRgDesc.nTop + nRow);
                //! keep (additional) reference to object to prevent implicit destruction
                //! in following UNO calls (when object will get referenced)
                xCellRef = pXCell;
                SwTableBox * pBox = pXCell ? pXCell->GetTblBox() : 0;
                if(!pBox)
                {
                    throw uno::RuntimeException();
                }
                else
                {
                    const uno::Any &rAny = pColArray[nCol];
                    if (uno::TypeClass_STRING == rAny.getValueTypeClass())
                        sw_setString( *pXCell, *(OUString *) rAny.getValue() );
                    else
                    {
                        double d = 0;
                        // #i20067# don't throw exception just do nothing if
                        // there is no value set
                        if( (rAny >>= d) )
                            sw_setValue( *pXCell, d );
                        else
                            sw_setString( *pXCell, OUString(), true );
                    }
                }
            }
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXCellRange::getData(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();

    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    uno::Sequence< uno::Sequence< double > > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        uno::Sequence< double >* pRowArray = aRowSeq.getArray();

        const sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            uno::Sequence< double > aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
            double * pArray = aColSeq.getArray();
            const sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                pArray[nCol - nColStart] = xCell->getValue();
            }
            pRowArray[nRow - nRowStart] = aColSeq;
        }
    }
    return aRowSeq;
}

void SwXCellRange::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                                throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    const sal_uInt16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        const sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        if(rData.getLength() < nRowCount - nRowStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pRowArray = rData.getConstArray();
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< double >& rColSeq = pRowArray[nRow - nRowStart];
            const sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
            if(rColSeq.getLength() < nColCount - nColStart)
            {
                throw uno::RuntimeException();
            }
            const double * pColArray = rColSeq.getConstArray();
            for(sal_uInt16 nCol = nColStart; nCol < nColCount; nCol++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(nCol, nRow);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                xCell->setValue(pColArray[nCol - nColStart]);
            }
        }
    }
}

///@see SwXTextTable::getRowDescriptions (TODO: seems to be copy and paste programming here)
uno::Sequence< OUString > SwXCellRange::getRowDescriptions(void)
                                            throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nRowCount = getRowCount();
    if(!nRowCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstColumnAsLabel ? nRowCount - 1 : nRowCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstColumnAsLabel)
        {
            const sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            OSL_FAIL("Where do these labels come from?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

///@see SwXTextTable::setRowDescriptions (TODO: seems to be copy and paste programming here)
void SwXCellRange::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc)
                                                    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const sal_uInt16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < (bFirstRowAsLabel ? nRowCount - 1 : nRowCount))
        {
            throw uno::RuntimeException();
        }
        const OUString* pArray = rRowDesc.getConstArray();
        if(bFirstColumnAsLabel)
        {
            const sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nRowCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(0, i);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            OSL_FAIL("Where to put theses labels?");
        }
    }
}

///@see SwXTextTable::setColumnDescriptions (TODO: seems to be copy and paste programming here)
uno::Sequence< OUString > SwXCellRange::getColumnDescriptions(void)
                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nColCount = getColumnCount();
    if(!nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    uno::Sequence< OUString > aRet(bFirstRowAsLabel ? nColCount - 1 : nColCount);
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        OUString* pArray = aRet.getArray();
        if(bFirstRowAsLabel)
        {
            const sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);
                pArray[i - nStart] = xText->getString();
            }
        }
        else
        {
            OSL_FAIL("Where do these labels come from?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

///@see SwXTextTable::setColumnDescriptions (TODO: seems to be copy and paste programming here)
void SwXCellRange::setColumnDescriptions(const uno::Sequence< OUString >& ColumnDesc)
                                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_uInt16 nColCount = getColumnCount();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pArray = ColumnDesc.getConstArray();
        if(bFirstRowAsLabel && ColumnDesc.getLength() >= nColCount - (bFirstColumnAsLabel ? 1 : 0))
        {
            const sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
            for(sal_uInt16 i = nStart; i < nColCount; i++)
            {
                uno::Reference< table::XCell >  xCell = getCellByPosition(i, 0);
                if(!xCell.is())
                {
                    throw uno::RuntimeException();
                }
                uno::Reference< text::XText >  xText(xCell, uno::UNO_QUERY);

                xText->setString(pArray[i - nStart]);
            }
        }
        else
        {
            OSL_FAIL("Where to put theses labels?");
        }
    }
}

void SAL_CALL SwXCellRange::addChartDataChangeEventListener(
        const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_ChartListeners.addInterface(xListener);
}

void SAL_CALL SwXCellRange::removeChartDataChangeEventListener(
        const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_ChartListeners.removeInterface(xListener);
}

sal_Bool SwXCellRange::isNotANumber(double /*fNumber*/) throw( uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
    return sal_False;
}

double SwXCellRange::getNotANumber(void) throw( uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
    return 0.;
}

uno::Sequence< beans::PropertyValue > SwXCellRange::createSortDescriptor(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    return SwUnoCursorHelper::CreateSortDescriptor(true);
}

void SAL_CALL SwXCellRange::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw (uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwSortOptions aSortOpt;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt &&
        SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
    {
        SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(*pTblCrsr);
        rTableCrsr.MakeBoxSels();
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(rTableCrsr.GetSelectedBoxes(), aSortOpt);
    }
}

sal_uInt16 SwXCellRange::getColumnCount(void)
{
    return static_cast< sal_uInt16 >(aRgDesc.nRight - aRgDesc.nLeft + 1);
}

sal_uInt16 SwXCellRange::getRowCount(void)
{
    return static_cast< sal_uInt16 >(aRgDesc.nBottom - aRgDesc.nTop + 1);
}

const SwUnoCrsr* SwXCellRange::GetTblCrsr() const
{
    const SwUnoCrsr* pRet = 0;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        pRet = pTblCrsr;
    return pRet;
}

void SwXCellRange::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );
    if(!GetRegisteredIn() || !aCursorDepend.GetRegisteredIn())
    {
        /*
         * Not sure if this will cause a memory leak - this pTblCrsr
         * is deleted in SwDoc and segfaults here when deleted again
         * if(!aCursorDepend.GetRegisteredIn())
            delete pTblCrsr;
         */
        pTblCrsr = 0;
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
        m_ChartListeners.disposeAndClear(ev);
    }
    else
    {
        lcl_SendChartEvent(*this, m_ChartListeners);
    }
}

//  SwXTableRows

OUString SwXTableRows::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTableRows");
}

sal_Bool SwXTableRows::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTableRows::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TableRows";
    return aRet;
}
TYPEINIT1(SwXTableRows, SwClient);

SwXTableRows::SwXTableRows(SwFrmFmt& rFrmFmt) :
    SwClient(&rFrmFmt)
{
}

SwXTableRows::~SwXTableRows()
{
}

sal_Int32 SwXTableRows::getCount(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        nRet = pTable->GetTabLines().size();
    }
    return nRet;
}

///@see SwXCell::CreateXCell (TODO: seems to be copy and paste programming here)
uno::Any SwXTableRows::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 )
        throw lang::IndexOutOfBoundsException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if( pTable->GetTabLines().size() > static_cast<size_t>(nIndex))
        {
            SwTableLine* pLine = pTable->GetTabLines()[nIndex];
            SwIterator<SwXTextTableRow,SwFmt> aIter( *pFrmFmt );
            SwXTextTableRow* pXRow = aIter.First();
            while( pXRow )
            {
                // is there already a proper cell?
                if(pXRow->GetTblRow() == pLine)
                    break;
                pXRow = aIter.Next();
            }
            // otherwise create it
            if(!pXRow)
                pXRow = new SwXTextTableRow(pFrmFmt, pLine);
            uno::Reference< beans::XPropertySet >  xRet =
                                    (beans::XPropertySet*)pXRow;
            aRet.setValue(&xRet, cppu::UnoType<beans::XPropertySet>::get());
        }
        else
            throw lang::IndexOutOfBoundsException();
    }
    return aRet;
}

uno::Type SAL_CALL SwXTableRows::getElementType(void) throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SwXTableRows::hasElements(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    // a table always has rows
    return sal_True;
}

void SwXTableRows::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            const size_t nRowCount = pTable->GetTabLines().size();
            if (nCount <= 0 || !(0 <= nIndex && static_cast<size_t>(nIndex) <= nRowCount))
            {
                uno::RuntimeException aExcept;
                aExcept.Message = "Illegal arguments";
                throw aExcept;
            }

            const OUString sTLName = sw_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            bool bAppend = false;
            if(!pTLBox)
            {
                bAppend = true;
                // to append at the end the cursor must be in the last line
                SwTableLines& rLines = pTable->GetTabLines();
                SwTableLine* pLine = rLines.back();
                SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                pTLBox = rBoxes.front();
            }
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // set cursor to the upper-left cell of the range
                UnoActionContext aAction(pFrmFmt->GetDoc());
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );

                {
                    // remove actions
                    UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
                }

                pFrmFmt->GetDoc()->InsertRow(*pUnoCrsr, (sal_uInt16)nCount, bAppend);
                delete pUnoCrsr;
            }
        }
    }
}

void SwXTableRows::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        bool bSuccess = false;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            OUString sTLName = sw_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                {
                    // invalidate all actions
                    UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                }
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // set cursor to the upper-left cell of the range
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( false );
                const OUString sBLName = sw_GetCellName(0, nIndex + nCount - 1);
                const SwTableBox* pBLBox = pTable->GetTblBox( sBLName );
                if(pBLBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBLBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr =
                        dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();
                    {   // these braces are important
                        UnoActionContext aAction(pFrmFmt->GetDoc());
                        pFrmFmt->GetDoc()->DeleteRow(*pUnoCrsr);
                        delete pUnoCrsr;
                        bSuccess = true;
                    }
                    {
                        // invalidate all actions
                        UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                    }
                }
            }
        }
        if(!bSuccess)
        {
            uno::RuntimeException aExcept;
            aExcept.Message = "Illegal arguments";
            throw aExcept;
        }
    }
}

void SwXTableRows::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

// SwXTableColumns

OUString SwXTableColumns::getImplementationName(void) throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTableColumns");
}

sal_Bool SwXTableColumns::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTableColumns::getSupportedServiceNames(void) throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.TableColumns";
    return aRet;
}

TYPEINIT1(SwXTableColumns, SwClient);

SwXTableColumns::SwXTableColumns(SwFrmFmt& rFrmFmt) :
    SwClient(&rFrmFmt)
{
}

SwXTableColumns::~SwXTableColumns()
{
}

sal_Int32 SwXTableColumns::getCount(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sal_Int32 nRet = 0;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.front();
            nRet = pLine->GetTabBoxes().size();
        }
    }
    return nRet;
}

uno::Any SwXTableColumns::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  xRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        size_t nCount = 0;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.front();
            nCount = pLine->GetTabBoxes().size();
        }
        if(nIndex < 0 || nCount <= static_cast<size_t>(nIndex))
            throw lang::IndexOutOfBoundsException();
        xRet = uno::Reference<uno::XInterface>();   //!! writer tables do not have columns !!
    }
    return uno::Any(&xRet, cppu::UnoType<uno::XInterface>::get());
}

uno::Type SAL_CALL SwXTableColumns::getElementType(void) throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<uno::XInterface>::get();
}

sal_Bool SwXTableColumns::hasElements(void) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    return sal_True;
}

///@see SwXTableRows::insertByIndex (TODO: seems to be copy and paste programming here)
void SwXTableColumns::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.front();
            const size_t nColCount = pLine->GetTabBoxes().size();
            if (nCount <= 0 || !(0 <= nIndex && static_cast<size_t>(nIndex) <= nColCount))
            {
                uno::RuntimeException aExcept;
                aExcept.Message = "Illegal arguments";
                throw aExcept;
            }

            const OUString sTLName = sw_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            bool bAppend = false;
            if(!pTLBox)
            {
                bAppend = true;
                // to append at the end the cursor must be in the last line
                SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                pTLBox = rBoxes.back();
            }
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                UnoActionContext aAction(pFrmFmt->GetDoc());
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );

                {
                    // remove actions
                    UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
                }

                pFrmFmt->GetDoc()->InsertCol(*pUnoCrsr, (sal_uInt16)nCount, bAppend);
                delete pUnoCrsr;
            }
        }
    }
}

///@see SwXTableRows::removeByIndex (TODO: seems to be copy and paste programming here)
void SwXTableColumns::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt|| nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    else
    {
        bool bSuccess = false;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            const OUString sTLName = sw_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                {
                    // invalidate all actions
                    UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                }
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // set cursor to the upper-left cell of the range
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( false );
                const OUString sTRName = sw_GetCellName(nIndex + nCount - 1, 0);
                const SwTableBox* pTRBox = pTable->GetTblBox( sTRName );
                if(pTRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pTRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr =
                        dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();
                    {   // these braces are important
                        UnoActionContext aAction(pFrmFmt->GetDoc());
                        pFrmFmt->GetDoc()->DeleteCol(*pUnoCrsr);
                        delete pUnoCrsr;
                        bSuccess = true;
                    }
                    {
                        // invalidate all actions
                        UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                    }
                }
            }
        }
        if(!bSuccess)
        {
            uno::RuntimeException aExcept;
            aExcept.Message = "Illegal arguments";
            throw aExcept;
        }
    }
}

void SwXTableColumns::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
