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
#include <array>
#include <utility>
#include <vector>
#include <algorithm>

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
#include <rootfrm.hxx>
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
#include <calbck.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/string.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <swtable.hxx>

using namespace ::com::sun::star;
using ::editeng::SvxBorderLine;

namespace
{
    template<typename Tcoretype, typename Tunotype>
    struct FindUnoInstanceHint final : SfxHint
    {
        FindUnoInstanceHint(Tcoretype* pCore) : m_pCore(pCore), m_pResult(nullptr) {};
        const Tcoretype* const m_pCore;
        mutable Tunotype* m_pResult;
    };
    SwFrameFormat* lcl_EnsureCoreConnected(SwFrameFormat* pFormat, cppu::OWeakObject* pObject)
    {
        if(!pFormat)
            throw uno::RuntimeException("Lost connection to core objects", pObject);
        return pFormat;
    }
    SwTable* lcl_EnsureTableNotComplex(SwTable* pTable, cppu::OWeakObject* pObject)
    {
        if(pTable->IsTableComplex())
            throw uno::RuntimeException("Table too complex", pObject);
        return pTable;
    }
}

#define UNO_TABLE_COLUMN_SUM    10000

static void lcl_SendChartEvent(::cppu::OWeakObject & rSource,
                               ::cppu::OInterfaceContainerHelper & rListeners)
{
    if (!rListeners.getLength())
        return;
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

    return rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
}

static void lcl_SetSpecialProperty(SwFrameFormat* pFormat,
                                   const SfxItemPropertySimpleEntry* pEntry,
                                   const uno::Any& aValue)
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
{
    // special treatment for "non-items"
    switch(pEntry->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        case  FN_TABLE_HEADLINE_COUNT:
        {
            {
                SwTable* pTable = SwTable::FindTable( pFormat );
                UnoActionContext aAction(pFormat->GetDoc());
                if( pEntry->nWID == FN_TABLE_HEADLINE_REPEAT)
                {
                    pFormat->GetDoc()->SetRowsToRepeat( *pTable, aValue.get<bool>() ? 1 : 0 );
                }
                else
                {
                    sal_Int32 nRepeat = 0;
                    aValue >>= nRepeat;
                    if( nRepeat >= 0 && nRepeat < USHRT_MAX )
                        pFormat->GetDoc()->SetRowsToRepeat( *pTable, (sal_uInt16) nRepeat );
                }
            }
        }
        break;

        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            SwFormatFrmSize aSz( pFormat->GetFrmSize() );
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
                if(!aValue.get<bool>())
                    aSz.SetWidthPercent(0);
                else
                {
                    lang::IllegalArgumentException aExcept;
                    aExcept.Message = "relative width cannot be switched on with this property";
                    throw aExcept;
                }
            }
            pFormat->GetDoc()->SetAttr(aSz, *pFormat);
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
                pDesc = SwPageDesc::GetByName(*pFormat->GetDoc(), sPageStyle);
            }
            SwFormatPageDesc aDesc( pDesc );
            pFormat->GetDoc()->SetAttr(aDesc, *pFormat);
        }
        break;

        default:
            throw lang::IllegalArgumentException();
    }
}

static uno::Any lcl_GetSpecialProperty(SwFrameFormat* pFormat, const SfxItemPropertySimpleEntry* pEntry )
{
    switch(pEntry->nWID)
    {
        case  FN_TABLE_HEADLINE_REPEAT:
        case  FN_TABLE_HEADLINE_COUNT:
        {
            SwTable* pTable = SwTable::FindTable( pFormat );
            const sal_uInt16 nRepeat = pTable->GetRowsToRepeat();
            if(pEntry->nWID == FN_TABLE_HEADLINE_REPEAT)
                return uno::makeAny<bool>(nRepeat > 0);
            return uno::makeAny<sal_Int32>(nRepeat);
        }

        case  FN_TABLE_WIDTH:
        case  FN_TABLE_IS_RELATIVE_WIDTH:
        case  FN_TABLE_RELATIVE_WIDTH:
        {
            uno::Any aRet;
            const SwFormatFrmSize& rSz = pFormat->GetFrmSize();
            if(FN_TABLE_WIDTH == pEntry->nWID)
                rSz.QueryValue(aRet, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            else if(FN_TABLE_RELATIVE_WIDTH == pEntry->nWID)
                rSz.QueryValue(aRet, MID_FRMSIZE_REL_WIDTH);
            else
                aRet = uno::makeAny<bool>(0 != rSz.GetWidthPercent());
            return aRet;
        }

        case RES_PAGEDESC:
        {
            const SfxItemSet& rSet = pFormat->GetAttrSet();
            const SfxPoolItem* pItem;
            if(SfxItemState::SET == rSet.GetItemState(RES_PAGEDESC, false, &pItem))
            {
                const SwPageDesc* pDsc = static_cast<const SwFormatPageDesc*>(pItem)->GetPageDesc();
                if(pDsc)
                    return uno::makeAny<OUString>(SwStyleNameMapper::GetProgName(pDsc->GetName(), nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC ));
            }
            return uno::makeAny(OUString());
        }

        case RES_ANCHOR:
            return uno::makeAny(text::TextContentAnchorType_AT_PARAGRAPH);

        case FN_UNO_ANCHOR_TYPES:
        {
            uno::Sequence<text::TextContentAnchorType> aTypes{text::TextContentAnchorType_AT_PARAGRAPH};
            return uno::makeAny(aTypes);
        }

        case FN_UNO_WRAP :
            return uno::makeAny(text::WrapTextMode_NONE);

        case FN_PARAM_LINK_DISPLAY_NAME :
            return uno::makeAny(pFormat->GetName());

        case FN_UNO_REDLINE_NODE_START:
        case FN_UNO_REDLINE_NODE_END:
        {
            SwTable* pTable = SwTable::FindTable( pFormat );
            SwNode* pTableNode = pTable->GetTableNode();
            if(FN_UNO_REDLINE_NODE_END == pEntry->nWID)
                pTableNode = pTableNode->EndOfSectionNode();
            for(const SwRangeRedline* pRedline : pFormat->GetDoc()->getIDocumentRedlineAccess().GetRedlineTable())
            {
                const SwNode& rRedPointNode = pRedline->GetNode();
                const SwNode& rRedMarkNode = pRedline->GetNode(false);
                if(&rRedPointNode == pTableNode || &rRedMarkNode == pTableNode)
                {
                    const SwNode& rStartOfRedline = SwNodeIndex(rRedPointNode) <= SwNodeIndex(rRedMarkNode) ?
                        rRedPointNode : rRedMarkNode;
                    bool bIsStart = &rStartOfRedline == pTableNode;
                    return uno::makeAny(SwXRedlinePortion::CreateRedlineProperties(*pRedline, bIsStart));
                }
            }
        }
    }
    return uno::Any();
}

/** get position of a cell with a given name
 *
 * If everything was OK, the indices for column and row are changed (both >= 0).
 * In case of errors, at least one of them is < 0.
 *
 * Also since the implementations of tables does not really have columns using
 * this function is appropriate only for tables that are not complex (i.e.
 * where IsTableComplex() returns false).
 *
 * @param rCellName e.g. A1..Z1, a1..z1, AA1..AZ1, Aa1..Az1, BA1..BZ1, Ba1..Bz1, ...
 * @param [IN,OUT] o_rColumn (0-based)
 * @param [IN,OUT] o_rRow (0-based)
 */
//TODO: potential for throwing proper exceptions instead of having every caller to check for errors
void SwXTextTable::GetCellPosition(const OUString& rCellName, sal_Int32& o_rColumn, sal_Int32& o_rRow)
{
    o_rColumn = o_rRow = -1;    // default return values indicating failure
    const sal_Int32 nLen = rCellName.getLength();
    if(!nLen)
    {
        SAL_WARN("sw.uno", "failed to get column or row index");
        return;
    }
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

        o_rColumn = nColIdx;
        o_rRow    = rCellName.copy(nRowPos).toInt32() - 1; // - 1 because indices ought to be 0 based
    }
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
    SwXTextTable::GetCellPosition( rCellName1, nCol1, nRow1 );
    SwXTextTable::GetCellPosition( rCellName2, nCol2, nRow2 );

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
    SwXTextTable::GetCellPosition( rCellName1, nCol1, nRow1 );
    SwXTextTable::GetCellPosition( rCellName2, nCol2, nRow2 );

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
    if (nColumn < 0 || nRow < 0)
        return OUString();
    OUString sCellName;
    sw_GetTableBoxColStr( static_cast< sal_uInt16 >(nColumn), sCellName );
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
    const SwTableLines* pLines(&rTableLines);
    while(true)
    {
        assert(!pLines->empty());
        if(pLines->empty())
            return nullptr;
        const SwTableLine* pLine(i_bTopLeft ? pLines->front() : pLines->back());
        assert(pLine);
        const SwTableBoxes& rBoxes(pLine->GetTabBoxes());
        assert(rBoxes.size() != 0);
        const SwTableBox* pBox = i_bTopLeft ? rBoxes.front() : rBoxes.back();
        assert(pBox);
        if (pBox->GetSttNd())
            return pBox;
        pLines = &pBox->GetTabLines();
    }
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
    SwXTextTable::GetCellPosition( rCell1, nCol1, nRow1 );
    SwXTextTable::GetCellPosition( rCell2, nCol2, nRow2 );
    if (nCol2 < nCol1 || nRow2 < nRow1)
    {
        rCell1  = sw_GetCellName( std::min(nCol1, nCol2), std::min(nRow1, nRow2) );
        rCell2  = sw_GetCellName( std::max(nCol1, nCol2), std::max(nRow1, nRow2) );
    }
}

void SwRangeDescriptor::Normalize()
{
    if (nTop > nBottom)
        std::swap(nBottom, nTop);
    if (nLeft > nRight)
        std::swap(nLeft, nRight);
}

static SwXCell* lcl_CreateXCell(SwFrameFormat* pFormat, sal_Int32 nColumn, sal_Int32 nRow)
{
    const OUString sCellName = sw_GetCellName(nColumn, nRow);
    SwTable* pTable = SwTable::FindTable(pFormat);
    SwTableBox* pBox = const_cast<SwTableBox*>(pTable->GetTableBox(sCellName));
    if(!pBox)
        return nullptr;
    return SwXCell::CreateXCell(pFormat, pBox, pTable);
}

static void lcl_InspectLines(SwTableLines& rLines, std::vector<OUString>& rAllNames)
{
    for(auto pLine : rLines)
    {
        for(auto pBox : pLine->GetTabBoxes())
        {
            if(!pBox->GetName().isEmpty() && pBox->getRowSpan() > 0)
                rAllNames.push_back(pBox->GetName());
            SwTableLines& rBoxLines = pBox->GetTabLines();
            if(!rBoxLines.empty())
                lcl_InspectLines(rBoxLines, rAllNames);
        }
    }
}

static bool lcl_FormatTable(SwFrameFormat* pTableFormat)
{
    bool bHasFrames = false;
    SwIterator<SwFrm,SwFormat> aIter( *pTableFormat );
    for(SwFrm* pFrm = aIter.First(); pFrm; pFrm = aIter.Next())
    {
        vcl::RenderContext* pRenderContext = pFrm->getRootFrm()->GetCurrShell()->GetOut();
        // mba: no TYPEINFO for SwTabFrm
        if(!pFrm->IsTabFrm())
            continue;
        SwTabFrm* pTabFrm = static_cast<SwTabFrm*>(pFrm);
        if(pTabFrm->IsValid())
            pTabFrm->InvalidatePos();
        pTabFrm->SetONECalcLowers();
        pTabFrm->Calc(pRenderContext);
        bHasFrames = true;
    }
    return bHasFrames;
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

static void lcl_GetTableSeparators(uno::Any& rRet, SwTable* pTable, SwTableBox* pBox, bool bRow)
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
        rRet.setValue(&aColSeq, cppu::UnoType<uno::Sequence< text::TableColumnSeparator>>::get());

}

static void lcl_SetTableSeparators(const uno::Any& rVal, SwTable* pTable, SwTableBox* pBox, bool bRow, SwDoc* pDoc)
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
                static_cast<uno::Sequence< text::TableColumnSeparator> const *>(rVal.getValue());
    if(!pSepSeq || static_cast<size_t>(pSepSeq->getLength()) != nOldCount)
        return;
    SwTabCols aCols(aOldCols);
    const text::TableColumnSeparator* pArray = pSepSeq->getConstArray();
    long nLastValue = 0;
    //sal_Int32 nTableWidth = aCols.GetRight() - aCols.GetLeft();
    for(size_t i = 0; i < nOldCount; ++i)
    {
        aCols[i] = pArray[i].Position;
        if(bool(pArray[i].IsVisible) == aCols.IsHidden(i) ||
                (!bRow && aCols.IsHidden(i)) ||
                aCols[i] < nLastValue ||
                UNO_TABLE_COLUMN_SUM < aCols[i] )
            return; // probably this should assert()
        nLastValue = aCols[i];
    }
    pDoc->SetTabCols(*pTable, aCols, aOldCols, pBox, bRow );
}

/* non UNO function call to set string in SwXCell */
void sw_setString( SwXCell &rCell, const OUString &rText,
        bool bKeepNumberFormat = false )
{
    if(rCell.IsValid())
    {
        SwFrameFormat* pBoxFormat = rCell.pBox->ClaimFrameFormat();
        pBoxFormat->LockModify();
        pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMULA );
        pBoxFormat->ResetFormatAttr( RES_BOXATR_VALUE );
        if (!bKeepNumberFormat)
            pBoxFormat->SetFormatAttr( SwTableBoxNumFormat(css::util::NumberFormat::TEXT) );
        pBoxFormat->UnlockModify();
    }
    rCell.SwXText::setString(rText);
}


/* non UNO function call to set value in SwXCell */
void sw_setValue( SwXCell &rCell, double nVal )
{
    if(!rCell.IsValid())
        return;
    // first this text (maybe) needs to be deleted
    sal_uLong nNdPos = rCell.pBox->IsValidNumTextNd();
    if(ULONG_MAX != nNdPos)
        sw_setString( rCell, OUString(), true );   // true == keep number format
    SwDoc* pDoc = rCell.GetDoc();
    UnoActionContext aAction(pDoc);
    SwFrameFormat* pBoxFormat = rCell.pBox->ClaimFrameFormat();
    SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE);
    const SfxPoolItem* pItem;

    //!! do we need to set a new number format? Yes, if
    // - there is no current number format
    // - the current number format is not a number format according to the number formatter, but rather a text format
    // - the current number format is not even a valid number formatter number format, but rather Writer's own 'special' text number format
    if(SfxItemState::SET != pBoxFormat->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, true, &pItem)
        ||  pDoc->GetNumberFormatter()->IsTextFormat(static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue())
        ||  static_cast<sal_Int16>(static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue()) == css::util::NumberFormat::TEXT)
    {
        aSet.Put(SwTableBoxNumFormat(0));
    }

    SwTableBoxValue aVal(nVal);
    aSet.Put(aVal);
    pDoc->SetTableBoxFormulaAttrs( *rCell.pBox, aSet );
    // update table
    SwTableFormulaUpdate aTableUpdate( SwTable::FindTable( rCell.GetFrameFormat() ));
    pDoc->getIDocumentFieldsAccess().UpdateTableFields( &aTableUpdate );
}

TYPEINIT1(SwXCell, SwClient);

SwXCell::SwXCell(SwFrameFormat* pTableFormat, SwTableBox* pBx, size_t const nPos) :
    SwXText(pTableFormat->GetDoc(), CURSOR_TBLTEXT),
    SwClient(pTableFormat),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_CELL)),
    pBox(pBx),
    pStartNode(nullptr),
    nFndPos(nPos)
{
}

SwXCell::SwXCell(SwFrameFormat* pTableFormat, const SwStartNode& rStartNode) :
    SwXText(pTableFormat->GetDoc(), CURSOR_TBLTEXT),
    SwClient(pTableFormat),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_CELL)),
    pBox(nullptr),
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

    if(aRetTypes.getLength())
        return aRetTypes;
    const auto& rCellTypes = SwXCellBaseClass::getTypes();
    const auto& rTextTypes = SwXText::getTypes();
    aRetTypes = uno::Sequence<uno::Type>(rCellTypes.getLength() + rTextTypes.getLength());
    std::copy_n(rCellTypes.begin(), rCellTypes.getLength(), aRetTypes.begin());
    std::copy_n(rTextTypes.begin(), rTextTypes.getLength(), aRetTypes.begin()+rCellTypes.getLength());
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
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXText::queryInterface(aType);
    return aRet;
}

const SwStartNode *SwXCell::GetStartNode() const
{
    const SwStartNode* pSttNd = nullptr;

    if( pStartNode || IsValid() )
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
    SwFrameFormat* pTableFormat = pBox ? GetFrameFormat() : nullptr;
    if(!pTableFormat)
    {
        const_cast<SwXCell*>(this)->pBox = nullptr;
    }
    else
    {
        SwTable* pTable = SwTable::FindTable( pTableFormat );
        SwTableBox const*const pFoundBox =
            const_cast<SwXCell*>(this)->FindBox(pTable, pBox);
        if (!pFoundBox)
        {
            const_cast<SwXCell*>(this)->pBox = nullptr;
        }
    }
    return nullptr != pBox;
}

OUString SwXCell::getFormula() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        return OUString();
    SwTableBoxFormula aFormula( pBox->GetFrameFormat()->GetTableBoxFormula() );
    SwTable* pTable = SwTable::FindTable( GetFrameFormat() );
    aFormula.PtrToBoxNm( pTable );
    return aFormula.GetFormula();
}

///@see sw_setValue (TODO: seems to be copy and paste programming here)
void SwXCell::setFormula(const OUString& rFormula) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        return;
    // first this text (maybe) needs to be deleted
    sal_uInt32 nNdPos = pBox->IsValidNumTextNd();
    if(USHRT_MAX == nNdPos)
        sw_setString( *this, OUString(), true );
    OUString sFormula(comphelper::string::stripStart(rFormula, ' '));
    if( !sFormula.isEmpty() && '=' == sFormula[0] )
                sFormula = sFormula.copy( 1 );
    SwTableBoxFormula aFormula( sFormula );
    SwDoc* pMyDoc = GetDoc();
    UnoActionContext aAction(pMyDoc);
    SfxItemSet aSet(pMyDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_FORMULA);
    const SfxPoolItem* pItem;
    SwFrameFormat* pBoxFormat = pBox->GetFrameFormat();
    if(SfxItemState::SET != pBoxFormat->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, true, &pItem)
        ||  pMyDoc->GetNumberFormatter()->IsTextFormat(static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue()))
    {
        aSet.Put(SwTableBoxNumFormat(0));
    }
    aSet.Put(aFormula);
    GetDoc()->SetTableBoxFormulaAttrs( *pBox, aSet );
    // update table
    SwTableFormulaUpdate aTableUpdate( SwTable::FindTable( GetFrameFormat() ));
    pMyDoc->getIDocumentFieldsAccess().UpdateTableFields( &aTableUpdate );
}

double SwXCell::getValue() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    // #i112652# a table cell may contain NaN as a value, do not filter that
    double fRet;
    if(IsValid() && !getString().isEmpty())
        fRet = pBox->GetFrameFormat()->GetTableBoxValue().GetValue();
    else
        ::rtl::math::setNan( &fRet );
    return fRet;
}

void SwXCell::setValue(double rValue) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    sw_setValue( *this, rValue );
}

table::CellContentType SwXCell::getType() throw( uno::RuntimeException, std::exception )
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

sal_Int32 SwXCell::getError() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    OUString sContent = getString();
    return sal_Int32(sContent.equals(SwViewShell::GetShellRes()->aCalc_Error));
}

uno::Reference<text::XTextCursor> SwXCell::createTextCursor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!pStartNode && !IsValid())
        throw uno::RuntimeException();
    const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    SwXTextCursor* const pXCursor =
        new SwXTextCursor(*GetDoc(), this, CURSOR_TBLTEXT, aPos);
    auto& rUnoCrsr(pXCursor->GetCursor());
    rUnoCrsr.Move(fnMoveForward, fnGoNode);
    return static_cast<text::XWordCursor*>(pXCursor);
}

uno::Reference<text::XTextCursor> SwXCell::createTextCursorByRange(const uno::Reference< text::XTextRange > & xTextPosition)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoInternalPaM aPam(*GetDoc());
    if((!pStartNode && !IsValid()) || !::sw::XTextRangeToSwPaM(aPam, xTextPosition))
        throw uno::RuntimeException();
    const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
    // skip sections
    SwStartNode* p1 = aPam.GetNode().StartOfSectionNode();
    while(p1->IsSectionNode())
        p1 = p1->StartOfSectionNode();
    if( p1 != pSttNd )
        return nullptr;
    return static_cast<text::XWordCursor*>(
        new SwXTextCursor(*GetDoc(), this, CURSOR_TBLTEXT,
        *aPam.GetPoint(), aPam.GetMark()));
}

uno::Reference< beans::XPropertySetInfo >  SwXCell::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXCell::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        return;
    // Hack to support hidden property to transfer textDirection
    if(rPropertyName == "FRMDirection")
    {
        SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
        try
        {
            const std::array<SvxFrameDirection, 3> vDirs = { FRMDIR_HORI_LEFT_TOP,  FRMDIR_HORI_RIGHT_TOP, FRMDIR_VERT_TOP_RIGHT };
            eDir = vDirs.at(aValue.get<sal_Int32>());
        } catch(std::out_of_range) {
            SAL_WARN("sw.uno", "unknown direction code, maybe it's a bitfield");
        }
        SvxFrameDirectionItem aItem(eDir, RES_FRAMEDIR);
        pBox->GetFrameFormat()->SetFormatAttr(aItem);
    }
    else if(rPropertyName == "TableRedlineParams")
    {
        // Get the table row properties
        uno::Sequence<beans::PropertyValue> tableCellProperties;
        tableCellProperties = aValue.get< uno::Sequence< beans::PropertyValue > >();
        comphelper::SequenceAsHashMap aPropMap(tableCellProperties);
        uno::Any sRedlineTypeValue = aPropMap.getUnpackedValueOrDefault("RedlineType", uno::Any());
        if(!sRedlineTypeValue.has<OUString>())
            throw beans::UnknownPropertyException("No redline type property: ", static_cast<cppu::OWeakObject*>(this));
        // Create a 'Table Cell Redline' object
        SwUnoCursorHelper::makeTableCellRedline(*pBox, sRedlineTypeValue.get<OUString>(), tableCellProperties);
    }
    else
    {
        auto pEntry(m_pPropSet->getPropertyMap().getByName(rPropertyName));
        if(!pEntry)
            throw beans::UnknownPropertyException(rPropertyName, static_cast<cppu::OWeakObject*>(this));
        if(pEntry->nWID != FN_UNO_CELL_ROW_SPAN)
        {
            SwFrameFormat* pBoxFormat = pBox->ClaimFrameFormat();
            SwAttrSet aSet(pBoxFormat->GetAttrSet());
            m_pPropSet->setPropertyValue(rPropertyName, aValue, aSet);
            pBoxFormat->GetDoc()->SetAttr(aSet, *pBoxFormat);
        }
        else if(aValue.isExtractableTo(cppu::UnoType<sal_Int32>::get()))
            pBox->setRowSpan(aValue.get<sal_Int32>());
    }
}

uno::Any SwXCell::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        return uno::Any();
    auto pEntry(m_pPropSet->getPropertyMap().getByName(rPropertyName));
    if(!pEntry)
        throw beans::UnknownPropertyException(rPropertyName, static_cast<cppu::OWeakObject*>(this));
    switch(pEntry->nWID)
    {
        case FN_UNO_CELL_ROW_SPAN:
            return uno::makeAny(pBox->getRowSpan());
        break;
        case FN_UNO_TEXT_SECTION:
        {
            SwFrameFormat* pTableFormat = GetFrameFormat();
            SwTable* pTable = SwTable::FindTable(pTableFormat);
            SwTableNode* pTableNode = pTable->GetTableNode();
            SwSectionNode* pSectionNode = pTableNode->FindSectionNode();
            if(!pSectionNode)
                return uno::Any();
            SwSection& rSect = pSectionNode->GetSection();
            return uno::makeAny(SwXTextSections::GetObject(*rSect.GetFormat()));
        }
        break;
        case FN_UNO_CELL_NAME:
            return uno::makeAny(pBox->GetName());
        break;
        case FN_UNO_REDLINE_NODE_START:
        case FN_UNO_REDLINE_NODE_END:
        {
            //redline can only be returned if it's a living object
            return SwXText::getPropertyValue(rPropertyName);
        }
        break;
        default:
        {
            const SwAttrSet& rSet = pBox->GetFrameFormat()->GetAttrSet();
            uno::Any aResult;
            m_pPropSet->getPropertyValue(rPropertyName, rSet, aResult);
            return aResult;
        }
    }
}

void SwXCell::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXCell::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXCell::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXCell::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

uno::Reference<container::XEnumeration> SwXCell::createEnumeration() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(!IsValid())
        return uno::Reference<container::XEnumeration>();
    const SwStartNode* pSttNd = pBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    auto pUnoCursor(GetDoc()->CreateUnoCrsr(aPos));
    pUnoCursor->Move(fnMoveForward, fnGoNode);
    // remember table and start node for later travelling
    // (used in export of tables in tables)
    SwTable const*const pTable(&pSttNd->FindTableNode()->GetTable());
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CURSOR_TBLTEXT, pSttNd, pTable);
}

uno::Type SAL_CALL SwXCell::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SwXCell::hasElements() throw( uno::RuntimeException, std::exception )
{
    return sal_True;
}

void SwXCell::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

void SwXCell::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(typeid(FindUnoInstanceHint<SwTableBox, SwXCell>) == typeid(rHint))
    {
        auto* pFindHint(static_cast<const FindUnoInstanceHint<SwTableBox, SwXCell>* >(&rHint));
        if(!pFindHint->m_pCore && pFindHint->m_pCore == GetTableBox())
            pFindHint->m_pResult = this;
    }
    else
        SwClient::SwClientNotify(rModify, rHint);
}

SwXCell* SwXCell::CreateXCell(SwFrameFormat* pTableFormat, SwTableBox* pBox, SwTable *pTable )
{
    if(!pTableFormat || !pBox)
        return nullptr;
    if(!pTable)
        pTable = SwTable::FindTable(pTableFormat);
    SwTableSortBoxes::const_iterator it = pTable->GetTabSortBoxes().find(pBox);
    if(it == pTable->GetTabSortBoxes().end())
        return nullptr;
    size_t const nPos = it - pTable->GetTabSortBoxes().begin();
    FindUnoInstanceHint<SwTableBox, SwXCell> aHint{pBox};
    pTableFormat->CallSwClientNotify(aHint);
    return aHint.m_pResult ? aHint.m_pResult : new SwXCell(pTableFormat, pBox, nPos);
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
    return nullptr;
}

double SwXCell::GetForcedNumericalValue() const
{
    if(table::CellContentType_TEXT != getType())
        return getValue();
    // now we'll try to get a useful numerical value
    // from the text in the cell...
    sal_uInt32 nFIndex;
    SvNumberFormatter* pNumFormatter(const_cast<SvNumberFormatter*>(GetDoc()->GetNumberFormatter()));
    // look for SwTableBoxNumFormat value in parents as well
    const SfxPoolItem* pItem;
    auto pBoxFormat(GetTableBox()->GetFrameFormat());
    SfxItemState eState = pBoxFormat->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, true, &pItem);

    if (eState == SfxItemState::SET)
    {
        // please note that the language of the numberformat
        // is implicitly coded into the below value as well
        nFIndex = static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue();

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
    if (!pNumFormatter->IsNumberFormat(const_cast<SwXCell*>(this)->getString(), nFIndex, fTmp))
        ::rtl::math::setNan(&fTmp);
    return fTmp;
}

uno::Any SwXCell::GetAny() const
{
    if(!pBox)
        throw uno::RuntimeException();
    // check if table box value item is set
    auto pBoxFormat(pBox->GetFrameFormat());
    const bool bIsNum = pBoxFormat->GetItemState(RES_BOXATR_VALUE, false) == SfxItemState::SET;
    return bIsNum ? uno::makeAny(getValue()) : uno::makeAny(const_cast<SwXCell*>(this)->getString());
}

OUString SwXCell::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXCell"); }

sal_Bool SwXCell::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

uno::Sequence< OUString > SwXCell::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
    { return {"com.sun.star.text.CellProperties"}; }

OUString SwXTextTableRow::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXTextTableRow"); }

sal_Bool SwXTextTableRow::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

uno::Sequence< OUString > SwXTextTableRow::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
    { return {"com.sun.star.text.TextTableRow"}; }

TYPEINIT1(SwXTextTableRow, SwClient);

SwXTextTableRow::SwXTextTableRow(SwFrameFormat* pFormat, SwTableLine* pLn) :
    SwClient(pFormat),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE_ROW)),
    pLine(pLn)
{ }

SwXTextTableRow::~SwXTextTableRow()
{ }

uno::Reference< beans::XPropertySetInfo > SwXTextTableRow::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
{
    static uno::Reference<beans::XPropertySetInfo> xRef = m_pPropSet->getPropertySetInfo();
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
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    SwTable* pTable = SwTable::FindTable( pFormat );
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
            SwDoc* pDoc = pFormat->GetDoc();
            if (!pEntry)
                throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pEntry->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    SwFormatFrmSize aFrmSize(pLn->GetFrameFormat()->GetFrmSize());
                    if(FN_UNO_ROW_AUTO_HEIGHT== pEntry->nWID)
                    {
                        bool bSet = *static_cast<sal_Bool const *>(aValue.getValue());
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
                    pDoc->SetAttr(aFrmSize, *pLn->ClaimFrameFormat());
                }
                break;

                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    UnoActionContext aContext(pDoc);
                    SwTable* pTable2 = SwTable::FindTable( pFormat );
                    lcl_SetTableSeparators(aValue, pTable2, pLine->GetTabBoxes()[0], true, pDoc);
                }
                break;

                default:
                {
                    SwFrameFormat* pLnFormat = pLn->ClaimFrameFormat();
                    SwAttrSet aSet(pLnFormat->GetAttrSet());
                    m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                    pDoc->SetAttr(aSet, *pLnFormat);
                }
            }
        }
    }
}

uno::Any SwXTextTableRow::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    SwTable* pTable = SwTable::FindTable( pFormat );
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
                const SwFormatFrmSize& rSize = pLn->GetFrameFormat()->GetFrmSize();
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
                lcl_GetTableSeparators(aRet, pTable, pLine->GetTabBoxes()[0], true);
            }
            break;

            default:
            {
                const SwAttrSet& rSet = pLn->GetFrameFormat()->GetAttrSet();
                m_pPropSet->getPropertyValue(*pEntry, rSet, aRet);
            }
        }
    }
    return aRet;
}

void SwXTextTableRow::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableRow::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableRow::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableRow::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableRow::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
    { ClientModify(this, pOld, pNew); }

void SwXTextTableRow::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(typeid(FindUnoInstanceHint<SwTableLine, SwXTextTableRow>) == typeid(rHint))
    {
        auto* pFindHint(static_cast<const FindUnoInstanceHint<SwTableLine,SwXTextTableRow>* >(&rHint));
        if(!pFindHint->m_pCore && pFindHint->m_pCore == GetTableRow())
            pFindHint->m_pResult = this;
    }
    else
        SwClient::SwClientNotify(rModify, rHint);
}

SwTableLine* SwXTextTableRow::FindLine(SwTable* pTable, SwTableLine* pLine)
{
    for(auto& pCurrentLine : pTable->GetTabLines())
        if(pCurrentLine == pLine)
            return pCurrentLine;
    return nullptr;
}

// SwXTextTableCursor

OUString SwXTextTableCursor::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXTextTableCursor"); }

sal_Bool SwXTextTableCursor::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

IMPLEMENT_FORWARD_XINTERFACE2(SwXTextTableCursor,SwXTextTableCursor_Base,OTextCursorHelper)
const SwPaM*        SwXTextTableCursor::GetPaM() const  { return &GetCrsr(); }
SwPaM*              SwXTextTableCursor::GetPaM()        { return &GetCrsr(); }
const SwDoc*        SwXTextTableCursor::GetDoc() const  { return GetFrameFormat()->GetDoc(); }
SwDoc*              SwXTextTableCursor::GetDoc()        { return GetFrameFormat()->GetDoc(); }
const SwUnoCrsr&    SwXTextTableCursor::GetCrsr() const { return *m_pUnoCrsr; }
SwUnoCrsr&          SwXTextTableCursor::GetCrsr()       { return *m_pUnoCrsr; }

uno::Sequence<OUString> SwXTextTableCursor::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
    { return {"com.sun.star.text.TextTableCursor"}; }

SwXTextTableCursor::SwXTextTableCursor(SwFrameFormat* pFormat, SwTableBox* pBox) :
    SwClient(pFormat),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE_CURSOR))
{
    SwDoc* pDoc = pFormat->GetDoc();
    const SwStartNode* pSttNd = pBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    m_pUnoCrsr = pDoc->CreateUnoCrsr(aPos, true);
    m_pUnoCrsr->Move( fnMoveForward, fnGoNode );
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(*m_pUnoCrsr);
    rTableCrsr.MakeBoxSels();
}

SwXTextTableCursor::SwXTextTableCursor(SwFrameFormat& rTableFormat, const SwTableCursor* pTableSelection) :
    SwClient(&rTableFormat),
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE_CURSOR))
{
    m_pUnoCrsr = pTableSelection->GetDoc()->CreateUnoCrsr(*pTableSelection->GetPoint(), true);
    if(pTableSelection->HasMark())
    {
        m_pUnoCrsr->SetMark();
        *m_pUnoCrsr->GetMark() = *pTableSelection->GetMark();
    }
    const SwSelBoxes& rBoxes = pTableSelection->GetSelectedBoxes();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(*m_pUnoCrsr);
    for(auto pBox : rBoxes)
        rTableCrsr.InsertBox(*pBox);
    rTableCrsr.MakeBoxSels();
}

OUString SwXTextTableCursor::getRangeName()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr* pTableCrsr = dynamic_cast<SwUnoTableCrsr*>(&rUnoCrsr);
    //!! see also SwChartDataSequence::getSourceRangeRepresentation
    if(!pTableCrsr)
        return OUString();
    pTableCrsr->MakeBoxSels();
    const SwStartNode* pNode = pTableCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
    const SwTable* pTable = SwTable::FindTable(GetFrameFormat());
    const SwTableBox* pEndBox = pTable->GetTableBox(pNode->GetIndex());
    if(pTableCrsr->HasMark())
    {
        pNode = pTableCrsr->GetMark()->nNode.GetNode().FindTableBoxStartNode();
        const SwTableBox* pStartBox = pTable->GetTableBox(pNode->GetIndex());
        if(pEndBox != pStartBox)
        {
            // need to switch start and end?
            if(*pTableCrsr->GetPoint() < *pTableCrsr->GetMark())
                std::swap(pStartBox, pEndBox);
            return pStartBox->GetName() + ":" + pEndBox->GetName();
        }
    }
    return pEndBox->GetName();
}

sal_Bool SwXTextTableCursor::gotoCellByName(const OUString& sCellName, sal_Bool bExpand)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    auto& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    return rTableCrsr.GotoTableBox(sCellName);
}

sal_Bool SwXTextTableCursor::goLeft(sal_Int16 Count, sal_Bool bExpand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    return rTableCrsr.Left(Count, CRSR_SKIP_CHARS, false, false);
}

sal_Bool SwXTextTableCursor::goRight(sal_Int16 Count, sal_Bool bExpand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    return rTableCrsr.Right(Count, CRSR_SKIP_CHARS, false, false);
}

sal_Bool SwXTextTableCursor::goUp(sal_Int16 Count, sal_Bool bExpand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    return rTableCrsr.UpDown(true, Count, 0, 0);
}

sal_Bool SwXTextTableCursor::goDown(sal_Int16 Count, sal_Bool bExpand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    return rTableCrsr.UpDown(false, Count, 0, 0);
}

void SwXTextTableCursor::gotoStart(sal_Bool bExpand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    rTableCrsr.MoveTable(fnTableCurr, fnTableStart);
}

void SwXTextTableCursor::gotoEnd(sal_Bool bExpand) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    lcl_CrsrSelect(rTableCrsr, bExpand);
    rTableCrsr.MoveTable(fnTableCurr, fnTableEnd);
}

sal_Bool SwXTextTableCursor::mergeRange()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();

    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    {
        // HACK: remove pending actions for selecting old style tables
        UnoActionRemoveContext aRemoveContext(rTableCrsr);
    }
    rTableCrsr.MakeBoxSels();
    bool bResult;
    {
        UnoActionContext aContext(rUnoCrsr.GetDoc());
        bResult = TBLMERGE_OK == rTableCrsr.GetDoc()->MergeTable(rTableCrsr);
    }
    if(bResult)
    {
        size_t nCount = rTableCrsr.GetSelectedBoxesCount();
        while (nCount--)
            rTableCrsr.DeleteBox(nCount);
    }
    rTableCrsr.MakeBoxSels();
    return bResult;
}

sal_Bool SwXTextTableCursor::splitRange(sal_Int16 Count, sal_Bool Horizontal)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (Count <= 0)
        throw uno::RuntimeException("Illegal first argument: needs to be > 0", static_cast<cppu::OWeakObject*>(this));
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    {
        // HACK: remove pending actions for selecting old style tables
        UnoActionRemoveContext aRemoveContext(rTableCrsr);
    }
    rTableCrsr.MakeBoxSels();
    bool bResult;
    {
        UnoActionContext aContext(rUnoCrsr.GetDoc());
        bResult = rTableCrsr.GetDoc()->SplitTable(rTableCrsr.GetSelectedBoxes(), !Horizontal, Count);
    }
    rTableCrsr.MakeBoxSels();
    return bResult;
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTableCursor::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
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
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    auto pEntry(m_pPropSet->getPropertyMap().getByName(rPropertyName));
    if(!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast<cppu::OWeakObject*>(this));
    if(pEntry->nFlags & beans::PropertyAttribute::READONLY)
        throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast<cppu::OWeakObject*>(this));
    {
        auto pSttNode = rUnoCrsr.GetNode().StartOfSectionNode();
        const SwTableNode* pTableNode = pSttNode->FindTableNode();
        lcl_FormatTable(pTableNode->GetTable().GetFrameFormat());
    }
    auto& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    rTableCrsr.MakeBoxSels();
    SwDoc* pDoc = rUnoCrsr.GetDoc();
    switch(pEntry->nWID)
    {
        case FN_UNO_TABLE_CELL_BACKGROUND:
        {
            SvxBrushItem aBrush(RES_BACKGROUND);
            SwDoc::GetBoxAttr(rUnoCrsr, aBrush);
            aBrush.PutValue(aValue, pEntry->nMemberId);
            pDoc->SetBoxAttr(rUnoCrsr, aBrush);

        }
        break;
        case RES_BOXATR_FORMAT:
        {
            SfxUInt32Item aNumberFormat(RES_BOXATR_FORMAT);
            aNumberFormat.PutValue(aValue, 0);
            pDoc->SetBoxAttr(rUnoCrsr, aNumberFormat);
        }
        break;
        case FN_UNO_PARA_STYLE:
            SwUnoCursorHelper::SetTextFormatColl(aValue, rUnoCrsr);
        break;
        default:
        {
            SfxItemSet aItemSet(pDoc->GetAttrPool(), pEntry->nWID, pEntry->nWID);
            SwUnoCursorHelper::GetCrsrAttr(rTableCrsr.GetSelRing(),
                    aItemSet);

            if (!SwUnoCursorHelper::SetCursorPropertyValue(
                    *pEntry, aValue, rTableCrsr.GetSelRing(), aItemSet))
            {
                m_pPropSet->setPropertyValue(*pEntry, aValue, aItemSet);
            }
            SwUnoCursorHelper::SetCrsrAttr(rTableCrsr.GetSelRing(),
                    aItemSet, SetAttrMode::DEFAULT, true);
        }
    }
}

uno::Any SwXTextTableCursor::getPropertyValue(const OUString& rPropertyName)
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr& rUnoCrsr = GetCrsr();
    {
        auto pSttNode = rUnoCrsr.GetNode().StartOfSectionNode();
        const SwTableNode* pTableNode = pSttNode->FindTableNode();
        lcl_FormatTable(pTableNode->GetTable().GetFrameFormat());
    }
    SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(rUnoCrsr);
    auto pEntry(m_pPropSet->getPropertyMap().getByName(rPropertyName));
    if(!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast<cppu::OWeakObject*>(this));
    rTableCrsr.MakeBoxSels();
    uno::Any aResult;
    switch(pEntry->nWID)
    {
        case FN_UNO_TABLE_CELL_BACKGROUND:
        {
            SvxBrushItem aBrush(RES_BACKGROUND);
            if (SwDoc::GetBoxAttr(rUnoCrsr, aBrush))
                aBrush.QueryValue(aResult, pEntry->nMemberId);
        }
        break;
        case RES_BOXATR_FORMAT:
            // TODO: GetAttr for table selections in a Doc is missing
            throw uno::RuntimeException("Unknown property: " + rPropertyName, static_cast<cppu::OWeakObject*>(this));
        break;
        case FN_UNO_PARA_STYLE:
        {
            auto pFormat(SwUnoCursorHelper::GetCurTextFormatColl(rUnoCrsr, false));
            if(pFormat)
                aResult = uno::makeAny(pFormat->GetName());
        }
        break;
        default:
        {
            SfxItemSet aSet(rTableCrsr.GetDoc()->GetAttrPool(),
                RES_CHRATR_BEGIN, RES_FRMATR_END-1,
                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                0L);
            SwUnoCursorHelper::GetCrsrAttr(rTableCrsr.GetSelRing(), aSet);
            m_pPropSet->getPropertyValue(*pEntry, aSet, aResult);
        }
    }
    return aResult;
}

void SwXTextTableCursor::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableCursor::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableCursor::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableCursor::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("not implemented", static_cast<cppu::OWeakObject*>(this)); };

void SwXTextTableCursor::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
    { ClientModify(this, pOld, pNew); }

class SwXTextTable::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::cppu::OMultiTypeInterfaceContainerHelper m_Listeners;

    Impl() : m_Listeners(m_Mutex) { }

    // note: lock mutex before calling this to avoid concurrent update
    static std::pair<sal_uInt16, sal_uInt16> ThrowIfComplex(SwXTextTable &rThis)
    {
        sal_uInt16 const nRowCount(rThis.getRowCount());
        sal_uInt16 const nColCount(rThis.getColumnCount());
        if (!nRowCount || !nColCount)
        {
            throw uno::RuntimeException("Table too complex",
                    static_cast<cppu::OWeakObject*>(&rThis));
        }
        return std::make_pair(nRowCount, nColCount);
    }
};

class SwTableProperties_Impl
{
    SwUnoCursorHelper::SwAnyMapHelper aAnyMap;
public:
    SwTableProperties_Impl();
    ~SwTableProperties_Impl();

    void SetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any& aVal);
    bool GetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& rpAny);
    template<typename Tpoolitem>
    inline void AddItemToSet(SfxItemSet& rSet, std::function<Tpoolitem()> aItemFactory, sal_uInt16 nWhich, std::initializer_list<sal_uInt16> vMember, bool bAddTwips = false);

    void ApplyTableAttr(const SwTable& rTable, SwDoc& rDoc);
};

SwTableProperties_Impl::SwTableProperties_Impl()
    { }

SwTableProperties_Impl::~SwTableProperties_Impl()
    { }

void SwTableProperties_Impl::SetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any& rVal)
    { aAnyMap.SetValue( nWhichId, nMemberId, rVal ); }

bool SwTableProperties_Impl::GetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& rpAny )
    { return aAnyMap.FillValue( nWhichId, nMemberId, rpAny ); }

template<typename Tpoolitem>
void SwTableProperties_Impl::AddItemToSet(SfxItemSet& rSet, std::function<Tpoolitem()> aItemFactory, sal_uInt16 nWhich, std::initializer_list<sal_uInt16> vMember, bool bAddTwips)
{
    std::list< std::pair<sal_uInt16, const uno::Any* > > vMemberAndAny;
    for(sal_uInt16 nMember : vMember)
    {
        const uno::Any* pAny = nullptr;
        GetProperty(nWhich, nMember, pAny);
        if(pAny)
            vMemberAndAny.push_back(std::make_pair(nMember, pAny));
    }
    if(!vMemberAndAny.empty())
    {
        Tpoolitem aItem = aItemFactory();
        for(auto& aMemberAndAny : vMemberAndAny)
            aItem.PutValue(*aMemberAndAny.second, aMemberAndAny.first | (bAddTwips ? CONVERT_TWIPS : 0) );
        rSet.Put(aItem);
    }
}
void SwTableProperties_Impl::ApplyTableAttr(const SwTable& rTable, SwDoc& rDoc)
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
    const SwFrameFormat &rFrameFormat = *rTable.GetFrameFormat();
    if(GetProperty(FN_TABLE_HEADLINE_REPEAT, 0xff, pRepHead ))
    {
        bool bVal(pRepHead->get<bool>());
        const_cast<SwTable&>(rTable).SetRowsToRepeat( bVal ? 1 : 0 );  // TODO: MULTIHEADER
    }

    AddItemToSet<SvxBrushItem>(aSet, [&rFrameFormat]() { return rFrameFormat.makeBackgroundBrushItem(); }, RES_BACKGROUND, {
        MID_BACK_COLOR,
        MID_GRAPHIC_TRANSPARENT,
        MID_GRAPHIC_POSITION,
        MID_GRAPHIC_URL,
        MID_GRAPHIC_FILTER });

    bool bPutBreak = true;
    const uno::Any* pPage;
    if(GetProperty(FN_UNO_PAGE_STYLE, 0, pPage) || GetProperty(RES_PAGEDESC, 0xff, pPage))
    {
        OUString sPageStyle = pPage->get<OUString>();
        if(!sPageStyle.isEmpty())
        {
            SwStyleNameMapper::FillUIName(sPageStyle, sPageStyle, nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC, true);
            const SwPageDesc* pDesc = SwPageDesc::GetByName(rDoc, sPageStyle);
            if(pDesc)
            {
                SwFormatPageDesc aDesc(pDesc);
                const uno::Any* pPgNo;
                if(GetProperty(RES_PAGEDESC, MID_PAGEDESC_PAGENUMOFFSET, pPgNo))
                {
                    aDesc.SetNumOffset(pPgNo->get<sal_Int16>());
                }
                aSet.Put(aDesc);
                bPutBreak = false;
            }

        }
    }

    if(bPutBreak)
        AddItemToSet<SvxFormatBreakItem>(aSet, [&rFrameFormat]() { return rFrameFormat.GetBreak(); }, RES_BREAK, {0});
    AddItemToSet<SvxShadowItem>(aSet, [&rFrameFormat]() { return rFrameFormat.GetShadow(); }, RES_SHADOW, {0}, true);
    AddItemToSet<SvxFormatKeepItem>(aSet, [&rFrameFormat]() { return rFrameFormat.GetKeep(); }, RES_KEEP, {0});
    AddItemToSet<SwFormatHoriOrient>(aSet, [&rFrameFormat]() { return rFrameFormat.GetHoriOrient(); }, RES_HORI_ORIENT, {MID_HORIORIENT_ORIENT}, true);

    const uno::Any* pSzRel(nullptr);
    GetProperty(FN_TABLE_IS_RELATIVE_WIDTH, 0xff, pSzRel);
    const uno::Any* pRelWidth(nullptr);
    GetProperty(FN_TABLE_RELATIVE_WIDTH, 0xff, pRelWidth);
    const uno::Any* pWidth(nullptr);
    GetProperty(FN_TABLE_WIDTH, 0xff, pWidth);

    bool bPutSize = pWidth != nullptr;
    SwFormatFrmSize aSz(ATT_VAR_SIZE);
    if(pWidth)
    {
        aSz.PutValue(*pWidth, MID_FRMSIZE_WIDTH);
        bPutSize = true;
    }
    if(pSzRel && pSzRel->get<bool>() && pRelWidth)
    {
        aSz.PutValue(*pRelWidth, MID_FRMSIZE_REL_WIDTH|CONVERT_TWIPS);
        bPutSize = true;
    }
    if(bPutSize)
    {
        if(!aSz.GetWidth())
            aSz.SetWidth(MINLAY);
        aSet.Put(aSz);
    }
    AddItemToSet<SvxLRSpaceItem>(aSet, [&rFrameFormat]() { return rFrameFormat.GetLRSpace(); }, RES_LR_SPACE, {
        MID_L_MARGIN|CONVERT_TWIPS,
        MID_R_MARGIN|CONVERT_TWIPS });
    AddItemToSet<SvxULSpaceItem>(aSet, [&rFrameFormat]() { return rFrameFormat.GetULSpace(); }, RES_UL_SPACE, {
        MID_UP_MARGIN|CONVERT_TWIPS,
        MID_LO_MARGIN|CONVERT_TWIPS });
    const::uno::Any* pSplit(nullptr);
    if(GetProperty(RES_LAYOUT_SPLIT, 0, pSplit))
    {
        SwFormatLayoutSplit aSp(pSplit->get<bool>());
        aSet.Put(aSp);
    }
    if(aSet.Count())
    {
        rDoc.SetAttr(aSet, *rTable.GetFrameFormat());
    }
}

namespace
{
    class theSwXTextTableUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXTextTableUnoTunnelId > {};
}

const uno::Sequence< sal_Int8 > & SwXTextTable::getUnoTunnelId()
    { return theSwXTextTableUnoTunnelId::get().getSeq(); }

sal_Int64 SAL_CALL SwXTextTable::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if(rId.getLength() == 16
            && 0 == memcmp(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16))
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
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
    m_bFirstRowAsLabel(false),
    m_bFirstColumnAsLabel(false)
{ }

SwXTextTable::SwXTextTable(SwFrameFormat& rFrameFormat)
    : SwClient( &rFrameFormat )
    , m_pImpl(new Impl)
    ,
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE)),
    pTableProps(0),
    bIsDescriptor(false),
    nRows(0),
    nColumns(0),
    m_bFirstRowAsLabel(false),
    m_bFirstColumnAsLabel(false)
{ }

SwXTextTable::~SwXTextTable()
    { delete pTableProps; }

uno::Reference<text::XTextTable> SwXTextTable::CreateXTextTable(SwFrameFormat* const pFrameFormat)
{
    uno::Reference<text::XTextTable> xTable;
    if(pFrameFormat)
        xTable.set(pFrameFormat->GetXObject(), uno::UNO_QUERY); // cached?
    if(xTable.is())
        return xTable;
    SwXTextTable* const pNew( (pFrameFormat) ? new SwXTextTable(*pFrameFormat) : new SwXTextTable());
    xTable.set(pNew);
    if(pFrameFormat)
        pFrameFormat->SetXObject(xTable);
    // need a permanent Reference to initialize m_wThis
    pNew->m_pImpl->m_wThis = xTable;
    return xTable;
}

void SwXTextTable::initialize(sal_Int32 nR, sal_Int32 nC) throw( uno::RuntimeException, std::exception )
{
    if(!bIsDescriptor || nR <= 0 || nC <= 0 || nR >= USHRT_MAX || nC >= USHRT_MAX )
        throw uno::RuntimeException();
    nRows = static_cast<sal_uInt16>(nR);
    nColumns = static_cast<sal_uInt16>(nC);
}

uno::Reference< table::XTableRows >  SwXTextTable::getRows() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XTableRows> xResult(m_xRows);
    if(xResult.is())
        return xResult;
    if(SwFrameFormat* pFormat = GetFrameFormat())
        m_xRows = xResult = new SwXTableRows(*pFormat);
    if(!xResult.is())
        throw uno::RuntimeException();
    return xResult;
}

uno::Reference< table::XTableColumns >  SwXTextTable::getColumns() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference<table::XTableColumns> xResult(m_xColumns);
    if(xResult.is())
        return xResult;
    if(SwFrameFormat* pFormat = GetFrameFormat())
        m_xColumns = xResult = new SwXTableColumns(*pFormat);
    if(!xResult.is())
        throw uno::RuntimeException();
    return xResult;
}

uno::Reference<table::XCell> SwXTextTable::getCellByName(const OUString& sCellName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    SwTable* pTable = SwTable::FindTable(pFormat);
    SwTableBox* pBox = const_cast<SwTableBox*>(pTable->GetTableBox(sCellName));
    if(!pBox)
        return nullptr;
    return SwXCell::CreateXCell(pFormat, pBox);
}

uno::Sequence<OUString> SwXTextTable::getCellNames() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat(GetFrameFormat());
    if(!pFormat)
        return {};
    SwTable* pTable = SwTable::FindTable(pFormat);
    // exists at the table and at all boxes
    SwTableLines& rTableLines = pTable->GetTabLines();
    std::vector<OUString> aAllNames;
    lcl_InspectLines(rTableLines, aAllNames);
    return comphelper::containerToSequence<OUString>(aAllNames);
}

uno::Reference<text::XTextTableCursor> SwXTextTable::createCursorByCellName(const OUString& sCellName)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    uno::Reference<text::XTextTableCursor> xRet;
    SwTable* pTable = SwTable::FindTable(pFormat);
    SwTableBox* pBox = const_cast<SwTableBox*>(pTable->GetTableBox(sCellName));
    if(!pBox || pBox->getRowSpan() == 0)
        throw uno::RuntimeException();
    return new SwXTextTableCursor(pFormat, pBox);
}

void SwXTextTable::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    // attachToRange must only be called once
    if(!bIsDescriptor)  /* already attached ? */
        throw uno::RuntimeException("SwXTextTable: already attached to range.", static_cast<cppu::OWeakObject*>(this));

    uno::Reference<XUnoTunnel> xRangeTunnel(xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange(nullptr);
    OTextCursorHelper* pCursor(nullptr);
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast<SwXTextRange*>(
                sal::static_int_cast<sal_IntPtr>(xRangeTunnel->getSomething(SwXTextRange::getUnoTunnelId())));
        pCursor = reinterpret_cast<OTextCursorHelper*>(
                sal::static_int_cast<sal_IntPtr>(xRangeTunnel->getSomething(OTextCursorHelper::getUnoTunnelId())));
    }
    SwDoc* pDoc = pRange ? &pRange->GetDoc() : pCursor ? pCursor->GetDoc() : nullptr;
    if(!pDoc || !nRows || !nColumns)
        throw lang::IllegalArgumentException();
    SwUnoInternalPaM aPam(*pDoc);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aPam, xTextRange);
    {
        UnoActionContext aCont(pDoc);

        pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
        const SwTable* pTable(nullptr);
        if( 0 != aPam.Start()->nContent.GetIndex() )
        {
            pDoc->getIDocumentContentOperations().SplitNode(*aPam.Start(), false);
        }
        //TODO: if it is the last paragraph than add another one!
        if(aPam.HasMark())
        {
            pDoc->getIDocumentContentOperations().DeleteAndJoin(aPam);
            aPam.DeleteMark();
        }
        pTable = pDoc->InsertTable(SwInsertTableOptions( tabopts::HEADLINE | tabopts::DEFAULT_BORDER | tabopts::SPLIT_LAYOUT, 0 ),
                *aPam.GetPoint(),
                nRows,
                nColumns,
                text::HoriOrientation::FULL);
        if(pTable)
        {
            // here, the properties of the descriptor need to be analyzed
            pTableProps->ApplyTableAttr(*pTable, *pDoc);
            SwFrameFormat* pTableFormat(pTable->GetFrameFormat());
            lcl_FormatTable(pTableFormat);

            pTableFormat->Add(this);
            if(!m_sTableName.isEmpty())
            {
                sal_uInt16 nIndex = 1;
                OUString sTmpNameIndex(m_sTableName);
                while(pDoc->FindTableFormatByName(sTmpNameIndex, true) && nIndex < USHRT_MAX)
                {
                    sTmpNameIndex = m_sTableName + OUString::number(nIndex++);
                }
                pDoc->SetTableName( *pTableFormat, sTmpNameIndex);
            }

            const::uno::Any* pName;
            if(pTableProps->GetProperty(FN_UNO_TABLE_NAME, 0, pName))
                setName(pName->get<OUString>());
            bIsDescriptor = false;
            DELETEZ(pTableProps);
        }
        pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
}

void SwXTextTable::attach(const uno::Reference< text::XTextRange > & xTextRange)
        throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    attachToRange(xTextRange);
}

uno::Reference<text::XTextRange>  SwXTextTable::getAnchor()
        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    return new SwXTextRange(*pFormat);
}

void SwXTextTable::dispose() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    SwTable* pTable = SwTable::FindTable(pFormat);
    SwSelBoxes aSelBoxes;
    for(auto& rBox : pTable->GetTabSortBoxes() )
        aSelBoxes.insert(rBox);
    pFormat->GetDoc()->DeleteRowCol(aSelBoxes);
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

uno::Reference<table::XCell>  SwXTextTable::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat(GetFrameFormat());
    // sheet is unimportant
    if(nColumn >= 0 && nRow >= 0 && nColumn < USHRT_MAX && nRow < USHRT_MAX && pFormat)
    {
        auto pXCell = lcl_CreateXCell(pFormat, nColumn, nRow);
        if(pXCell)
            return pXCell;
    }
    throw lang::IndexOutOfBoundsException();
}

uno::Reference<table::XCellRange>  SwXTextTable::GetRangeByName(SwFrameFormat* pFormat, SwTable* pTable,
        const OUString& rTLName, const OUString& rBRName,
        SwRangeDescriptor& rDesc)
{
    SolarMutexGuard aGuard;
    const SwTableBox* pTLBox = pTable->GetTableBox(rTLName);
    if(!pTLBox)
        return nullptr;
    const SwStartNode* pSttNd = pTLBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    // set cursor to the upper-left cell of the range
    auto pUnoCrsr(pFormat->GetDoc()->CreateUnoCrsr(aPos, true));
    pUnoCrsr->Move(fnMoveForward, fnGoNode);
    pUnoCrsr->SetRemainInSection(false);
    const SwTableBox* pBRBox(pTable->GetTableBox(rBRName));
    if(!pBRBox)
        return nullptr;
    pUnoCrsr->SetMark();
    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
    pUnoCrsr->Move( fnMoveForward, fnGoNode );
    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr.get());
    // HACK: remove pending actions for selecting old style tables
    UnoActionRemoveContext aRemoveContext(*pCrsr);
    pCrsr->MakeBoxSels();
    // pUnoCrsr will be provided and will not be deleted
    return new SwXCellRange(pUnoCrsr, *pFormat, rDesc);
}

uno::Reference<table::XCellRange>  SwXTextTable::getCellRangeByPosition(sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom)
    throw(uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat(GetFrameFormat());
    if(pFormat && nRight < USHRT_MAX && nBottom < USHRT_MAX &&
            nLeft <= nRight && nTop <= nBottom &&
            nLeft >= 0 && nRight >= 0 && nTop >= 0 && nBottom >= 0 )
    {
        SwTable* pTable = SwTable::FindTable(pFormat);
        if(!pTable->IsTableComplex())
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
            return GetRangeByName(pFormat, pTable, sTLName, sBRName, aDesc);
        }
    }
    throw lang::IndexOutOfBoundsException();
}

uno::Reference<table::XCellRange> SwXTextTable::getCellRangeByName(const OUString& sRange)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRef;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    SwTable* pTable = lcl_EnsureTableNotComplex(SwTable::FindTable(pFormat), static_cast<cppu::OWeakObject*>(this));
    sal_Int32 nPos = 0;
    const OUString sTLName(sRange.getToken(0, ':', nPos));
    const OUString sBRName(sRange.getToken(0, ':', nPos));
    if(sTLName.isEmpty() || sBRName.isEmpty())
        throw uno::RuntimeException();
    SwRangeDescriptor aDesc;
    aDesc.nTop = aDesc.nLeft = aDesc.nBottom = aDesc.nRight = -1;
    SwXTextTable::GetCellPosition(sTLName, aDesc.nLeft, aDesc.nTop );
    SwXTextTable::GetCellPosition(sBRName, aDesc.nRight, aDesc.nBottom );

    // we should normalize the range now (e.g. A5:C1 will become A1:C5)
    // since (depending on what is done later) it will be troublesome
    // elsewhere when the cursor in the implementation does not
    // point to the top-left and bottom-right cells
    aDesc.Normalize();
    return GetRangeByName(pFormat, pTable, sTLName, sBRName, aDesc);
}

uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXTextTable::getDataArray()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<sheet::XCellRangeData> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    return xAllRange->getDataArray();
}

void SAL_CALL SwXTextTable::setDataArray(const uno::Sequence< uno::Sequence< uno::Any > >& rArray)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<sheet::XCellRangeData> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    return xAllRange->setDataArray(rArray);
}

uno::Sequence< uno::Sequence< double > > SwXTextTable::getData()
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<chart::XChartDataArray> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    static_cast<SwXCellRange*>(xAllRange.get())->SetLabels(m_bFirstRowAsLabel, m_bFirstColumnAsLabel);
    return xAllRange->getData();
}

void SwXTextTable::setData(const uno::Sequence< uno::Sequence< double > >& rData)
                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<chart::XChartDataArray> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    static_cast<SwXCellRange*>(xAllRange.get())->SetLabels(m_bFirstRowAsLabel, m_bFirstColumnAsLabel);
    xAllRange->setData(rData);
    // this is rather inconsistent: setData on XTextTable sends events, but e.g. CellRanges do not
    lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
}

uno::Sequence<OUString> SwXTextTable::getRowDescriptions()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<chart::XChartDataArray> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    static_cast<SwXCellRange*>(xAllRange.get())->SetLabels(m_bFirstRowAsLabel, m_bFirstColumnAsLabel);
    return xAllRange->getRowDescriptions();
}

void SwXTextTable::setRowDescriptions(const uno::Sequence<OUString>& rRowDesc)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<chart::XChartDataArray> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    static_cast<SwXCellRange*>(xAllRange.get())->SetLabels(m_bFirstRowAsLabel, m_bFirstColumnAsLabel);
    xAllRange->setRowDescriptions(rRowDesc);
}

uno::Sequence<OUString> SwXTextTable::getColumnDescriptions()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<chart::XChartDataArray> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    static_cast<SwXCellRange*>(xAllRange.get())->SetLabels(m_bFirstRowAsLabel, m_bFirstColumnAsLabel);
    return xAllRange->getColumnDescriptions();
}

void SwXTextTable::setColumnDescriptions(const uno::Sequence<OUString>& rColumnDesc)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    std::pair<sal_uInt16, sal_uInt16> const RowsAndColumns(SwXTextTable::Impl::ThrowIfComplex(*this));
    uno::Reference<chart::XChartDataArray> const xAllRange(
        getCellRangeByPosition(0, 0, RowsAndColumns.second-1, RowsAndColumns.first-1),
        uno::UNO_QUERY);
    static_cast<SwXCellRange*>(xAllRange.get())->SetLabels(m_bFirstRowAsLabel, m_bFirstColumnAsLabel);
    return xAllRange->setColumnDescriptions(rColumnDesc);
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

double SwXTextTable::getNotANumber() throw( uno::RuntimeException, std::exception )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return DBL_MIN;
}

uno::Sequence< beans::PropertyValue > SwXTextTable::createSortDescriptor()
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
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat &&
        SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
    {
        SwTable* pTable = SwTable::FindTable( pFormat );
        SwSelBoxes aBoxes;
        const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
        for (size_t n = 0; n < rTBoxes.size(); ++n)
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.insert( pBox );
        }
        UnoActionContext aContext( pFormat->GetDoc() );
        pFormat->GetDoc()->SortTable(aBoxes, aSortOpt);
    }
}

void SwXTextTable::autoFormat(const OUString& sAutoFormatName)
    throw (lang::IllegalArgumentException, uno::RuntimeException,
           std::exception)
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    SwTable* pTable = lcl_EnsureTableNotComplex(SwTable::FindTable(pFormat), static_cast<cppu::OWeakObject*>(this));
    SwTableAutoFormatTable aAutoFormatTable;
    aAutoFormatTable.Load();
    for (size_t i = aAutoFormatTable.size(); i;)
        if( sAutoFormatName == aAutoFormatTable[ --i ].GetName() )
        {
            SwSelBoxes aBoxes;
            const SwTableSortBoxes& rTBoxes = pTable->GetTabSortBoxes();
            for (size_t n = 0; n < rTBoxes.size(); ++n)
            {
                SwTableBox* pBox = rTBoxes[ n ];
                aBoxes.insert( pBox );
            }
            UnoActionContext aContext( pFormat->GetDoc() );
            pFormat->GetDoc()->SetTableAutoFormat( aBoxes, aAutoFormatTable[i] );
            break;
        }
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTable::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
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
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!aValue.hasValue())
        throw lang::IllegalArgumentException();
    const SfxItemPropertySimpleEntry* pEntry =
                                m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if( !pEntry )
        throw lang::IllegalArgumentException();
    if(pFormat)
    {
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        if(0xFF == pEntry->nMemberId)
        {
            lcl_SetSpecialProperty(pFormat, pEntry, aValue);
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
                    bool bTmp = *static_cast<sal_Bool const *>(aValue.getValue());
                    if(m_bFirstRowAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
                        m_bFirstRowAsLabel = bTmp;
                    }
                }
                break;

                case FN_UNO_RANGE_COL_LABEL:
                {
                    bool bTmp = *static_cast<sal_Bool const *>(aValue.getValue());
                    if(m_bFirstColumnAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
                        m_bFirstColumnAsLabel = bTmp;
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
                    SwDoc* pDoc = pFormat->GetDoc();
                    if(!lcl_FormatTable(pFormat))
                        break;
                    SwTable* pTable = SwTable::FindTable( pFormat );
                    SwTableLines &rLines = pTable->GetTabLines();

                    const SwTableBox* pTLBox = lcl_FindCornerTableBox(rLines, true);
                    const SwStartNode* pSttNd = pTLBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    // set cursor to top left cell
                    auto pUnoCrsr(pDoc->CreateUnoCrsr(aPos, true));
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    pUnoCrsr->SetRemainInSection( false );

                    const SwTableBox* pBRBox = lcl_FindCornerTableBox(rLines, false);
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr& rCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
                    // HACK: remove pending actions for selecting old style tables
                    UnoActionRemoveContext aRemoveContext(rCrsr);
                    rCrsr.MakeBoxSels();

                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);

                    SvxBoxItem aBox( RES_BOX );
                    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );

                    aBox.SetLine(aTopLine.isEmpty() ? 0 : &aTopLine, SvxBoxItemLine::TOP);
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::TOP, aBorder.IsTopLineValid);

                    aBox.SetLine(aBottomLine.isEmpty() ? 0 : &aBottomLine, SvxBoxItemLine::BOTTOM);
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::BOTTOM, aBorder.IsBottomLineValid);

                    aBox.SetLine(aLeftLine.isEmpty() ? 0 : &aLeftLine, SvxBoxItemLine::LEFT);
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::LEFT, aBorder.IsLeftLineValid);

                    aBox.SetLine(aRightLine.isEmpty() ? 0 : &aRightLine, SvxBoxItemLine::RIGHT);
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::RIGHT, aBorder.IsRightLineValid);

                    aBoxInfo.SetLine(aHoriLine.isEmpty() ? 0 : &aHoriLine, SvxBoxInfoItemLine::HORI);
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::HORI, aBorder.IsHorizontalLineValid);

                    aBoxInfo.SetLine(aVertLine.isEmpty() ? 0 : &aVertLine, SvxBoxInfoItemLine::VERT);
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::VERT, aBorder.IsVerticalLineValid);

                    aBox.SetDistance((sal_uInt16)convertMm100ToTwip(aBorder.Distance));
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::DISTANCE, aBorder.IsDistanceValid);

                    aSet.Put(aBox);
                    aSet.Put(aBoxInfo);

                    pDoc->SetTabBorders(rCrsr, aSet);
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
                    SwDoc* pDoc = pFormat->GetDoc();
                    SwTable* pTable = SwTable::FindTable( pFormat );
                    SwTableLines &rLines = pTable->GetTabLines();
                    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_START, NULL);
                    for(size_t i = 0; i < rLines.size(); ++i)
                    {
                        SwTableLine* pLine = rLines[i];
                        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                        for(size_t k = 0; k < rBoxes.size(); ++k)
                        {
                            SwTableBox* pBox = rBoxes[k];
                            const SwFrameFormat* pBoxFormat = pBox->GetFrameFormat();
                            const SvxBoxItem& rBox = pBoxFormat->GetBox();
                            if(
                                (aTableBorderDistances.IsLeftDistanceValid && nLeftDistance !=   rBox.GetDistance( SvxBoxItemLine::LEFT )) ||
                                (aTableBorderDistances.IsRightDistanceValid && nRightDistance !=  rBox.GetDistance( SvxBoxItemLine::RIGHT )) ||
                                (aTableBorderDistances.IsTopDistanceValid && nTopDistance !=    rBox.GetDistance( SvxBoxItemLine::TOP )) ||
                                (aTableBorderDistances.IsBottomDistanceValid && nBottomDistance != rBox.GetDistance( SvxBoxItemLine::BOTTOM )))
                            {
                                SvxBoxItem aSetBox( rBox );
                                SwFrameFormat* pSetBoxFormat = pBox->ClaimFrameFormat();
                                if( aTableBorderDistances.IsLeftDistanceValid )
                                    aSetBox.SetDistance( nLeftDistance, SvxBoxItemLine::LEFT );
                                if( aTableBorderDistances.IsRightDistanceValid )
                                    aSetBox.SetDistance( nRightDistance, SvxBoxItemLine::RIGHT );
                                if( aTableBorderDistances.IsTopDistanceValid )
                                    aSetBox.SetDistance( nTopDistance, SvxBoxItemLine::TOP );
                                if( aTableBorderDistances.IsBottomDistanceValid )
                                    aSetBox.SetDistance( nBottomDistance, SvxBoxItemLine::BOTTOM );
                                pDoc->SetAttr( aSetBox, *pSetBoxFormat );
                            }
                        }
                    }
                    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_END, NULL);
                }
                break;

                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    UnoActionContext aContext(pFormat->GetDoc());
                    SwTable* pTable = SwTable::FindTable( pFormat );
                    lcl_SetTableSeparators(aValue, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], false, pFormat->GetDoc());
                }
                break;

                case FN_UNO_TABLE_COLUMN_RELATIVE_SUM:/*_readonly_*/ break;

                default:
                {
                    SwAttrSet aSet(pFormat->GetAttrSet());
                    m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                    pFormat->GetDoc()->SetAttr(aSet, *pFormat);
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
    SwFrameFormat* pFormat = GetFrameFormat();
    const SfxItemPropertySimpleEntry* pEntry =
                                m_pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException("Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pFormat)
    {
        if(0xFF == pEntry->nMemberId)
        {
            aRet = lcl_GetSpecialProperty(pFormat, pEntry );
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
                    aRet <<= m_bFirstRowAsLabel;
                }
                break;

                case FN_UNO_RANGE_COL_LABEL:
                    aRet <<= m_bFirstColumnAsLabel;
                break;

                case FN_UNO_TABLE_BORDER:
                case FN_UNO_TABLE_BORDER2:
                {
                    SwDoc* pDoc = pFormat->GetDoc();
                    // tables without layout (invisible header/footer?)
                    if(!lcl_FormatTable(pFormat))
                        break;
                    SwTable* pTable = SwTable::FindTable( pFormat );
                    SwTableLines &rLines = pTable->GetTabLines();

                    const SwTableBox* pTLBox = lcl_FindCornerTableBox(rLines, true);
                    const SwStartNode* pSttNd = pTLBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    // set cursor to top left cell
                    auto pUnoCrsr(pDoc->CreateUnoCrsr(aPos, true));
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    pUnoCrsr->SetRemainInSection( false );

                    const SwTableBox* pBRBox = lcl_FindCornerTableBox(rLines, false);
                    pUnoCrsr->SetMark();
                    const SwStartNode* pLastNd = pBRBox->GetSttNd();
                    pUnoCrsr->GetPoint()->nNode = *pLastNd;

                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr& rCrsr = dynamic_cast<SwUnoTableCrsr&>(*pUnoCrsr);
                    // HACK: remove pending actions for selecting old style tables
                    UnoActionRemoveContext aRemoveContext(rCrsr);
                    rCrsr.MakeBoxSels();

                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                    SwDoc::GetTabBorders(rCrsr, aSet);
                    const SvxBoxInfoItem& rBoxInfoItem =
                        static_cast<const SvxBoxInfoItem&>(aSet.Get(SID_ATTR_BORDER_INNER));
                    const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX));

                    if (FN_UNO_TABLE_BORDER == pEntry->nWID)
                    {
                        table::TableBorder aTableBorder;
                        aTableBorder.TopLine                = SvxBoxItem::SvxLineToLine(rBox.GetTop(), true);
                        aTableBorder.IsTopLineValid         = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::TOP);
                        aTableBorder.BottomLine             = SvxBoxItem::SvxLineToLine(rBox.GetBottom(), true);
                        aTableBorder.IsBottomLineValid      = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::BOTTOM);
                        aTableBorder.LeftLine               = SvxBoxItem::SvxLineToLine(rBox.GetLeft(), true);
                        aTableBorder.IsLeftLineValid        = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::LEFT);
                        aTableBorder.RightLine              = SvxBoxItem::SvxLineToLine(rBox.GetRight(), true);
                        aTableBorder.IsRightLineValid       = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::RIGHT );
                        aTableBorder.HorizontalLine         = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetHori(), true);
                        aTableBorder.IsHorizontalLineValid  = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::HORI);
                        aTableBorder.VerticalLine           = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetVert(), true);
                        aTableBorder.IsVerticalLineValid    = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::VERT);
                        aTableBorder.Distance               = convertTwipToMm100( rBox.GetDistance() );
                        aTableBorder.IsDistanceValid        = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::DISTANCE);
                        aRet <<= aTableBorder;
                    }
                    else
                    {
                        table::TableBorder2 aTableBorder;
                        aTableBorder.TopLine                = SvxBoxItem::SvxLineToLine(rBox.GetTop(), true);
                        aTableBorder.IsTopLineValid         = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::TOP);
                        aTableBorder.BottomLine             = SvxBoxItem::SvxLineToLine(rBox.GetBottom(), true);
                        aTableBorder.IsBottomLineValid      = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::BOTTOM);
                        aTableBorder.LeftLine               = SvxBoxItem::SvxLineToLine(rBox.GetLeft(), true);
                        aTableBorder.IsLeftLineValid        = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::LEFT);
                        aTableBorder.RightLine              = SvxBoxItem::SvxLineToLine(rBox.GetRight(), true);
                        aTableBorder.IsRightLineValid       = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::RIGHT );
                        aTableBorder.HorizontalLine         = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetHori(), true);
                        aTableBorder.IsHorizontalLineValid  = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::HORI);
                        aTableBorder.VerticalLine           = SvxBoxItem::SvxLineToLine(rBoxInfoItem.GetVert(), true);
                        aTableBorder.IsVerticalLineValid    = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::VERT);
                        aTableBorder.Distance               = convertTwipToMm100( rBox.GetDistance() );
                        aTableBorder.IsDistanceValid        = rBoxInfoItem.IsValid(SvxBoxInfoItemValidFlags::DISTANCE);
                        aRet <<= aTableBorder;
                    }
                }
                break;

                case FN_UNO_TABLE_BORDER_DISTANCES :
                {
                    table::TableBorderDistances aTableBorderDistances( 0, sal_True, 0, sal_True, 0, sal_True, 0, sal_True ) ;
                    SwTable* pTable = SwTable::FindTable( pFormat );
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
                            SwFrameFormat* pBoxFormat = pBox->GetFrameFormat();
                            const SvxBoxItem& rBox = pBoxFormat->GetBox();
                            if( bFirst )
                            {
                                nLeftDistance =     convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::LEFT   ));
                                nRightDistance =    convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::RIGHT  ));
                                nTopDistance =      convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::TOP    ));
                                nBottomDistance =   convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::BOTTOM ));
                                bFirst = false;
                            }
                            else
                            {
                                if( aTableBorderDistances.IsLeftDistanceValid &&
                                    nLeftDistance != convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::LEFT   )))
                                    aTableBorderDistances.IsLeftDistanceValid = sal_False;
                                if( aTableBorderDistances.IsRightDistanceValid &&
                                    nRightDistance != convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::RIGHT   )))
                                    aTableBorderDistances.IsRightDistanceValid = sal_False;
                                if( aTableBorderDistances.IsTopDistanceValid &&
                                    nTopDistance != convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::TOP   )))
                                    aTableBorderDistances.IsTopDistanceValid = sal_False;
                                if( aTableBorderDistances.IsBottomDistanceValid &&
                                    nBottomDistance != convertTwipToMm100( rBox.GetDistance( SvxBoxItemLine::BOTTOM   )))
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
                    SwTable* pTable = SwTable::FindTable( pFormat );
                    lcl_GetTableSeparators(aRet, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], false);
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
                    SwTable* pTable = SwTable::FindTable( pFormat );
                    SwTableNode* pTableNode = pTable->GetTableNode();
                    SwSectionNode* pSectionNode =  pTableNode->FindSectionNode();
                    if(pSectionNode)
                    {
                        SwSection& rSect = pSectionNode->GetSection();
                        uno::Reference< text::XTextSection >  xSect =
                                        SwXTextSections::GetObject( *rSect.GetFormat() );
                        aRet <<= xSect;
                    }
                }
                break;

                default:
                {
                    const SwAttrSet& rSet = pFormat->GetAttrSet();
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
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

void SwXTextTable::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

void SwXTextTable::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

void SwXTextTable::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

OUString SwXTextTable::getName() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!pFormat && !bIsDescriptor)
        throw uno::RuntimeException();
    if(pFormat)
    {
        return pFormat->GetName();
    }
    return m_sTableName;
}

void SwXTextTable::setName(const OUString& rName) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if((!pFormat && !bIsDescriptor) ||
       rName.isEmpty() ||
       rName.indexOf('.')>=0 ||
       rName.indexOf(' ')>=0 )
        throw uno::RuntimeException();

    if(pFormat)
    {
        const OUString aOldName( pFormat->GetName() );
        const SwFrameFormats* pFrameFormats = pFormat->GetDoc()->GetTableFrameFormats();
        for (size_t i = pFrameFormats->size(); i;)
        {
            const SwFrameFormat* pTmpFormat = (*pFrameFormats)[--i];
            if( !pTmpFormat->IsDefault() &&
                pTmpFormat->GetName() == rName &&
                            pFormat->GetDoc()->IsUsed( *pTmpFormat ))
            {
                throw uno::RuntimeException();
            }
        }

        pFormat->SetName( rName );

        SwStartNode *pStNd;
        SwNodeIndex aIdx( *pFormat->GetDoc()->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
        {
            ++aIdx;
            SwNode *const pNd = & aIdx.GetNode();
            if ( pNd->IsOLENode() &&
                aOldName == static_cast<const SwOLENode*>(pNd)->GetChartTableName() )
            {
                const_cast<SwOLENode*>(static_cast<const SwOLENode*>(pNd))->SetChartTableName( rName );

                static_cast<SwOLENode*>(pNd)->GetOLEObj();

                SwTable* pTable = SwTable::FindTable( pFormat );
                //TL_CHART2: chart needs to be notfied about name changes
                pFormat->GetDoc()->UpdateCharts( pTable->GetFrameFormat()->GetName() );
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
        pFormat->GetDoc()->getIDocumentState().SetModified();
    }
    else
        m_sTableName = rName;
}

sal_uInt16 SwXTextTable::getRowCount()
{
    SolarMutexGuard aGuard;
    sal_uInt16 nRet = 0;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwTable* pTable = SwTable::FindTable( pFormat );
        if(!pTable->IsTableComplex())
        {
            nRet = pTable->GetTabLines().size();
        }
    }
    return nRet;
}

sal_uInt16 SwXTextTable::getColumnCount()
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    sal_uInt16 nRet = 0;
    if(pFormat)
    {
        SwTable* pTable = SwTable::FindTable( pFormat );
        if(!pTable->IsTableComplex())
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
        static_cast<void*>(GetRegisteredIn()) == static_cast<const SwPtrMsgPoolItem *>(pOld)->pObject )
            GetRegisteredIn()->Remove(this);
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

OUString SAL_CALL SwXTextTable::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXTextTable"); }

sal_Bool SwXTextTable::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

uno::Sequence<OUString> SwXTextTable::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    return {
        "com.sun.star.document.LinkTarget",
        "com.sun.star.text.TextTable",
        "com.sun.star.text.TextContent",
        "com.sun.star.text.TextSortable" };
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

OUString SwXCellRange::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXCellRange"); }

sal_Bool SwXCellRange::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

uno::Sequence<OUString> SwXCellRange::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    return {
        "com.sun.star.text.CellRange",
        "com.sun.star.style.CharacterProperties",
        "com.sun.star.style.CharacterPropertiesAsian",
        "com.sun.star.style.CharacterPropertiesComplex",
        "com.sun.star.style.ParagraphProperties",
        "com.sun.star.style.ParagraphPropertiesAsian",
        "com.sun.star.style.ParagraphPropertiesComplex" };
}

SwXCellRange::SwXCellRange(sw::UnoCursorPointer pCrsr, SwFrameFormat& rFrameFormat,
    SwRangeDescriptor& rDesc)
    : SwClient(&rFrameFormat)
    , m_ChartListeners(m_Mutex)
    , aRgDesc(rDesc)
    , m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TABLE_RANGE))
    , m_pTableCrsr(pCrsr)
    , m_bFirstRowAsLabel(false)
    , m_bFirstColumnAsLabel(false)
{
    aRgDesc.Normalize();
}

std::vector< uno::Reference< table::XCell > > SwXCellRange::GetCells()
{
    SwFrameFormat* const pFormat = GetFrameFormat();
    const sal_Int32 nRowCount(getRowCount());
    const sal_Int32 nColCount(getColumnCount());
    std::vector< uno::Reference< table::XCell > > vResult;
    vResult.reserve(static_cast<size_t>(nRowCount)*static_cast<size_t>(nColCount));
    for(sal_Int32 nRow = 0; nRow < nRowCount; ++nRow)
        for(sal_Int32 nCol = 0; nCol < nColCount; ++nCol)
            vResult.push_back(uno::Reference< table::XCell >(lcl_CreateXCell(pFormat, aRgDesc.nLeft + nCol, aRgDesc.nTop + nRow)));
    return vResult;
}

uno::Reference< table::XCell >  SwXCellRange::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  aRet;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        if(nColumn >= 0 && nRow >= 0 &&
             getColumnCount() > nColumn && getRowCount() > nRow )
        {
            SwXCell* pXCell = lcl_CreateXCell(pFormat,
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
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat && getColumnCount() > nRight && getRowCount() > nBottom &&
        nLeft <= nRight && nTop <= nBottom
        && nLeft >= 0 && nRight >= 0 && nTop >= 0 && nBottom >= 0 )
    {
        SwTable* pTable = SwTable::FindTable( pFormat );
        if(!pTable->IsTableComplex())
        {
            SwRangeDescriptor aNewDesc;
            aNewDesc.nTop    = nTop + aRgDesc.nTop;
            aNewDesc.nBottom = nBottom + aRgDesc.nTop;
            aNewDesc.nLeft   = nLeft + aRgDesc.nLeft;
            aNewDesc.nRight  = nRight + aRgDesc.nLeft;
            aNewDesc.Normalize();
            const OUString sTLName = sw_GetCellName(aNewDesc.nLeft, aNewDesc.nTop);
            const OUString sBRName = sw_GetCellName(aNewDesc.nRight, aNewDesc.nBottom);
            const SwTableBox* pTLBox = pTable->GetTableBox( sTLName );
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // set cursor in the upper-left cell of the range
                auto pUnoCrsr(pFormat->GetDoc()->CreateUnoCrsr(aPos, true));
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( false );
                const SwTableBox* pBRBox = pTable->GetTableBox( sBRName );
                if(pBRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr.get());
                    // HACK: remove pending actions for selecting old style tables
                    UnoActionRemoveContext aRemoveContext(*pCrsr);
                    pCrsr->MakeBoxSels();
                    // pUnoCrsr will be provided and will not be deleted
                    SwXCellRange* pCellRange = new SwXCellRange(pUnoCrsr, *pFormat, aNewDesc);
                    aRet = pCellRange;
                }
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
    SwXTextTable::GetCellPosition( sTLName, aDesc.nLeft, aDesc.nTop );
    SwXTextTable::GetCellPosition( sBRName, aDesc.nRight, aDesc.nBottom );
    aDesc.Normalize();
    return getCellRangeByPosition(aDesc.nLeft - aRgDesc.nLeft, aDesc.nTop - aRgDesc.nTop,
                aDesc.nRight - aRgDesc.nLeft, aDesc.nBottom - aRgDesc.nTop);
}

uno::Reference< beans::XPropertySetInfo >  SwXCellRange::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
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
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            SwDoc* pDoc = m_pTableCrsr->GetDoc();
            SwUnoTableCrsr& rCrsr = dynamic_cast<SwUnoTableCrsr&>(*m_pTableCrsr);
            {
                // HACK: remove pending actions for selecting old style tables
                UnoActionRemoveContext aRemoveContext(rCrsr);
            }
            rCrsr.MakeBoxSels();
            switch(pEntry->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    SwDoc::GetBoxAttr( *m_pTableCrsr, aBrush );
                    ((SfxPoolItem&)aBrush).PutValue(aValue, pEntry->nMemberId);
                    pDoc->SetBoxAttr( *m_pTableCrsr, aBrush );

                }
                break;
                case RES_BOX :
                {
                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
                    aBoxInfo.SetValid(SvxBoxInfoItemValidFlags::ALL, false);
                    SvxBoxInfoItemValidFlags nValid = SvxBoxInfoItemValidFlags::NONE;
                    switch(pEntry->nMemberId & ~CONVERT_TWIPS)
                    {
                        case  LEFT_BORDER :             nValid = SvxBoxInfoItemValidFlags::LEFT; break;
                        case  RIGHT_BORDER:             nValid = SvxBoxInfoItemValidFlags::RIGHT; break;
                        case  TOP_BORDER  :             nValid = SvxBoxInfoItemValidFlags::TOP; break;
                        case  BOTTOM_BORDER:            nValid = SvxBoxInfoItemValidFlags::BOTTOM; break;
                        case  LEFT_BORDER_DISTANCE :
                        case  RIGHT_BORDER_DISTANCE:
                        case  TOP_BORDER_DISTANCE  :
                        case  BOTTOM_BORDER_DISTANCE:
                            nValid = SvxBoxInfoItemValidFlags::DISTANCE;
                        break;
                    }
                    aBoxInfo.SetValid(nValid);

                    aSet.Put(aBoxInfo);
                    SwDoc::GetTabBorders(rCrsr, aSet);

                    aSet.Put(aBoxInfo);
                    SvxBoxItem aBoxItem(static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX)));
                    ((SfxPoolItem&)aBoxItem).PutValue(aValue, pEntry->nMemberId);
                    aSet.Put(aBoxItem);
                    pDoc->SetTabBorders( *m_pTableCrsr, aSet );
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
                    bool bTmp = *static_cast<sal_Bool const *>(aValue.getValue());
                    if(m_bFirstRowAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_ChartListeners);
                        m_bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    bool bTmp = *static_cast<sal_Bool const *>(aValue.getValue());
                    if(m_bFirstColumnAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_ChartListeners);
                        m_bFirstColumnAsLabel = bTmp;
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
                            aItemSet, SetAttrMode::DEFAULT, true);
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
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
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
                    if(SwDoc::GetBoxAttr( *m_pTableCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pEntry->nMemberId);

                }
                break;
                case RES_BOX :
                {
                    SwDoc* pDoc = m_pTableCrsr->GetDoc();
                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                    SwDoc::GetTabBorders(*m_pTableCrsr, aSet);
                    const SvxBoxItem& rBoxItem = static_cast<const SvxBoxItem&>(aSet.Get(RES_BOX));
                    rBoxItem.QueryValue(aRet, pEntry->nMemberId);
                }
                break;
                case RES_BOXATR_FORMAT:
                    OSL_FAIL("not implemented");
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFormatColl *const pTmpFormat =
                        SwUnoCursorHelper::GetCurTextFormatColl(*m_pTableCrsr, false);
                    OUString sRet;
                    if(pFormat)
                        sRet = pTmpFormat->GetName();
                    aRet <<= sRet;
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                    aRet <<= m_bFirstRowAsLabel;
                break;
                case FN_UNO_RANGE_COL_LABEL:
                    aRet <<= m_bFirstColumnAsLabel;
                break;
                default:
                {
                    SfxItemSet aSet(m_pTableCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // first look at the attributes of the cursor
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(&(*m_pTableCrsr));
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
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

void SwXCellRange::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

void SwXCellRange::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

void SwXCellRange::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

///@see SwXCellRange::getData
uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXCellRange::getDataArray()
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const sal_Int32 nRowCount = getRowCount();
    const sal_Int32 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    uno::Sequence< uno::Sequence< uno::Any > > aRowSeq(nRowCount);
    auto vCells(GetCells());
    auto pCurrentCell(vCells.begin());
    for(auto& rRow : aRowSeq)
    {
        rRow = uno::Sequence< uno::Any >(nColCount);
        for(auto& rCellAny : rRow)
        {
            auto pCell(static_cast<SwXCell*>(pCurrentCell->get()));
            if(!pCell)
                throw uno::RuntimeException();
            rCellAny = pCell->GetAny();
            ++pCurrentCell;
        }
    }
    return aRowSeq;
}

///@see SwXCellRange::setData
void SAL_CALL SwXCellRange::setDataArray(const uno::Sequence< uno::Sequence< uno::Any > >& rArray) throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    const sal_Int32 nRowCount = getRowCount();
    const sal_Int32 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!pFormat)
        return;
    if(rArray.getLength() != nRowCount)
        throw uno::RuntimeException("Row count mismatch. expected: " + OUString::number(nRowCount) + " got: " + OUString::number(rArray.getLength()), static_cast<cppu::OWeakObject*>(this));
    auto vCells(GetCells());
    auto pCurrentCell(vCells.begin());
    for(const auto& rColSeq : rArray)
    {
        if(rColSeq.getLength() != nColCount)
            throw uno::RuntimeException("Column count mismatch. expected: " + OUString::number(nColCount) + " got: " + OUString::number(rColSeq.getLength()), static_cast<cppu::OWeakObject*>(this));
        for(const auto& aValue : rColSeq)
        {
            auto pCell(static_cast<SwXCell*>(pCurrentCell->get()));
            if(!pCell || !pCell->GetTableBox())
                throw uno::RuntimeException("Box for cell missing", static_cast<cppu::OWeakObject*>(this));
            if(aValue.isExtractableTo(cppu::UnoType<OUString>::get()))
                sw_setString(*pCell, aValue.get<OUString>());
            else if(aValue.isExtractableTo(cppu::UnoType<double>::get()))
                sw_setValue(*pCell, aValue.get<double>());
            else
                sw_setString(*pCell, OUString(), true);
            ++pCurrentCell;
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXCellRange::getData() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_Int32 nRowCount = getRowCount();
    const sal_Int32 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    if(m_bFirstColumnAsLabel || m_bFirstRowAsLabel)
    {
        uno::Reference<chart::XChartDataArray> xDataRange(getCellRangeByPosition(m_bFirstColumnAsLabel ? 1 : 0, m_bFirstRowAsLabel ? 1 : 0,
            nColCount-1, nRowCount-1), uno::UNO_QUERY);
        return xDataRange->getData();
    }
    uno::Sequence< uno::Sequence< double > > vRows(nRowCount);
    auto vCells(GetCells());
    auto pCurrentCell(vCells.begin());
    for(auto& rRow : vRows)
    {
        rRow = uno::Sequence<double>(nColCount);
        for(auto& rValue : rRow)
        {
            rValue = (*pCurrentCell)->getValue();
            ++pCurrentCell;
        }
    }
    return vRows;
}

void SwXCellRange::setData(const uno::Sequence< uno::Sequence< double > >& rData)
    throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    const sal_Int32 nRowCount = getRowCount();
    const sal_Int32 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    if(m_bFirstColumnAsLabel || m_bFirstRowAsLabel)
    {
        uno::Reference<chart::XChartDataArray> xDataRange(getCellRangeByPosition(m_bFirstColumnAsLabel ? 1 : 0, m_bFirstRowAsLabel ? 1 : 0,
            nColCount-1, nRowCount-1), uno::UNO_QUERY);
        return xDataRange->setData(rData);
    }
    lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    if(rData.getLength() != nRowCount)
        throw uno::RuntimeException("Row count mismatch. expected: " + OUString::number(nRowCount) + " got: " + OUString::number(rData.getLength()), static_cast<cppu::OWeakObject*>(this));
    auto vCells(GetCells());
    auto pCurrentCell(vCells.begin());
    for(const auto& rRow : rData)
    {
        if(rRow.getLength() != nColCount)
            throw uno::RuntimeException("Column count mismatch. expected: " + OUString::number(nColCount) + " got: " + OUString::number(rRow.getLength()), static_cast<cppu::OWeakObject*>(this));
        for(const auto& rValue : rRow)
        {
            uno::Reference<table::XCell>(*pCurrentCell, uno::UNO_QUERY)->setValue(rValue);
            ++pCurrentCell;
        }
    }
}

std::tuple<sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32> SwXCellRange::getLabelCoordinates(bool bRow)
{
    sal_uInt32 nLeft, nTop, nRight, nBottom;
    nLeft = nTop = nRight = nBottom = 0;
    if(bRow)
    {
        nTop = m_bFirstRowAsLabel ? 1 : 0;
        nBottom = getRowCount()-1;
    }
    else
    {
        nLeft = m_bFirstColumnAsLabel ? 1 : 0;
        nRight = getColumnCount()-1;
    }
    return std::make_tuple(nLeft, nTop, nRight, nBottom);
}

uno::Sequence<OUString> SwXCellRange::getLabelDescriptions(bool bRow)
{
    SolarMutexGuard aGuard;
    sal_uInt32 nLeft, nTop, nRight, nBottom;
    std::tie(nLeft, nTop, nRight, nBottom) = getLabelCoordinates(bRow);
    if(!nRight && !nBottom)
        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    if(!(bRow ? m_bFirstColumnAsLabel : m_bFirstRowAsLabel))
        return {};  // without labels we have no descriptions
    auto xLabelRange(getCellRangeByPosition(nLeft, nTop, nRight, nBottom));
    auto vCells(static_cast<SwXCellRange*>(xLabelRange.get())->GetCells());
    uno::Sequence<OUString> vResult(vCells.size());
    std::transform(vCells.begin(), vCells.end(), vResult.begin(),
        [](uno::Reference<table::XCell> xCell) -> OUString { return uno::Reference<text::XText>(xCell, uno::UNO_QUERY_THROW)->getString(); });
    return vResult;
}

uno::Sequence<OUString> SwXCellRange::getRowDescriptions()
    throw( uno::RuntimeException, std::exception )
{ return getLabelDescriptions(true); }

uno::Sequence<OUString> SwXCellRange::getColumnDescriptions()
    throw(uno::RuntimeException, std::exception)
{ return getLabelDescriptions(false); }

void SwXCellRange::setLabelDescriptions(const uno::Sequence<OUString>& rDesc, bool bRow)
{
    SolarMutexGuard aGuard;
    lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    if(!(bRow ? m_bFirstColumnAsLabel : m_bFirstRowAsLabel))
        return; // if there are no labels we cannot set descriptions
    sal_uInt32 nLeft, nTop, nRight, nBottom;
    std::tie(nLeft, nTop, nRight, nBottom) = getLabelCoordinates(bRow);
    if(!nRight && !nBottom)
        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    auto xLabelRange(getCellRangeByPosition(nLeft, nTop, nRight, nBottom));
    auto vCells(static_cast<SwXCellRange*>(xLabelRange.get())->GetCells());
    if (sal::static_int_cast<sal_uInt32>(rDesc.getLength()) != vCells.size())
        throw uno::RuntimeException("Too few or too many descriptions", static_cast<cppu::OWeakObject*>(this));
    auto pDescIterator(rDesc.begin());
    for(auto xCell : vCells)
        uno::Reference<text::XText>(xCell, uno::UNO_QUERY_THROW)->setString(*pDescIterator++);
}
void SwXCellRange::setRowDescriptions(const uno::Sequence<OUString>& rRowDesc)
    throw(uno::RuntimeException, std::exception)
{ setLabelDescriptions(rRowDesc, true); }

void SwXCellRange::setColumnDescriptions(const uno::Sequence<OUString>& rColumnDesc)
    throw(uno::RuntimeException, std::exception)
{ setLabelDescriptions(rColumnDesc, false); }

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
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

double SwXCellRange::getNotANumber() throw( uno::RuntimeException, std::exception )
    { throw uno::RuntimeException("Not implemented", static_cast<cppu::OWeakObject*>(this)); }

uno::Sequence< beans::PropertyValue > SwXCellRange::createSortDescriptor() throw( uno::RuntimeException, std::exception )
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
    SwFrameFormat* pFormat(GetFrameFormat());
    if(pFormat && SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
    {
        SwUnoTableCrsr& rTableCrsr = dynamic_cast<SwUnoTableCrsr&>(*m_pTableCrsr);
        rTableCrsr.MakeBoxSels();
        UnoActionContext aContext(pFormat->GetDoc());
        pFormat->GetDoc()->SortTable(rTableCrsr.GetSelectedBoxes(), aSortOpt);
    }
}

sal_uInt16 SwXCellRange::getColumnCount()
    { return static_cast<sal_uInt16>(aRgDesc.nRight - aRgDesc.nLeft + 1); }

sal_uInt16 SwXCellRange::getRowCount()
    { return static_cast<sal_uInt16>(aRgDesc.nBottom - aRgDesc.nTop + 1); }

const SwUnoCrsr* SwXCellRange::GetTableCrsr() const
{
    SwFrameFormat* pFormat = GetFrameFormat();
    return pFormat ? &(*m_pTableCrsr) : nullptr;
}

void SwXCellRange::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew );
    if(!GetRegisteredIn() || !m_pTableCrsr)
    {
        m_pTableCrsr.reset(nullptr);
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
        m_ChartListeners.disposeAndClear(ev);
    }
    else
    {
        lcl_SendChartEvent(*this, m_ChartListeners);
    }
}

//  SwXTableRows

OUString SwXTableRows::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXTableRows"); }

sal_Bool SwXTableRows::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

uno::Sequence< OUString > SwXTableRows::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
    { return { "com.sun.star.text.TableRows" }; }

TYPEINIT1(SwXTableRows, SwClient);

SwXTableRows::SwXTableRows(SwFrameFormat& rFrameFormat) :
    SwClient(&rFrameFormat)
{ }

SwXTableRows::~SwXTableRows()
{ }

sal_Int32 SwXTableRows::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFrameFormat = GetFrameFormat();
    if(!pFrameFormat)
        throw uno::RuntimeException();
    SwTable* pTable = SwTable::FindTable(pFrameFormat);
    return pTable->GetTabLines().size();
}

///@see SwXCell::CreateXCell (TODO: seems to be copy and paste programming here)
uno::Any SwXTableRows::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFrameFormat(lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this)));
    if(nIndex < 0)
        throw lang::IndexOutOfBoundsException();
    SwTable* pTable = SwTable::FindTable( pFrameFormat );
    if(static_cast<size_t>(nIndex) >= pTable->GetTabLines().size())
        throw lang::IndexOutOfBoundsException();
    SwTableLine* pLine = pTable->GetTabLines()[nIndex];
    FindUnoInstanceHint<SwTableLine,SwXTextTableRow> aHint{pLine};
    pFrameFormat->CallSwClientNotify(aHint);
    if(!aHint.m_pResult)
        aHint.m_pResult = new SwXTextTableRow(pFrameFormat, pLine);
    uno::Reference<beans::XPropertySet> xRet = static_cast<beans::XPropertySet*>(aHint.m_pResult);
    return uno::makeAny(xRet);
}

uno::Type SAL_CALL SwXTableRows::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool SwXTableRows::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFrameFormat = GetFrameFormat();
    if(!pFrameFormat)
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
    SwFrameFormat* pFrameFormat(lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this)));
    SwTable* pTable = lcl_EnsureTableNotComplex(SwTable::FindTable(pFrameFormat), static_cast<cppu::OWeakObject*>(this));
    const size_t nRowCount = pTable->GetTabLines().size();
    if (nCount <= 0 || !(0 <= nIndex && static_cast<size_t>(nIndex) <= nRowCount))
        throw uno::RuntimeException("Illegal arguments", static_cast<cppu::OWeakObject*>(this));
    const OUString sTLName = sw_GetCellName(0, nIndex);
    const SwTableBox* pTLBox = pTable->GetTableBox(sTLName);
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
    if(!pTLBox)
        throw uno::RuntimeException("Illegal arguments", static_cast<cppu::OWeakObject*>(this));
    const SwStartNode* pSttNd = pTLBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    // set cursor to the upper-left cell of the range
    UnoActionContext aAction(pFrameFormat->GetDoc());
    std::shared_ptr<SwUnoTableCrsr> const pUnoCrsr(
            std::dynamic_pointer_cast<SwUnoTableCrsr>(
                pFrameFormat->GetDoc()->CreateUnoCrsr(aPos, true)));
    pUnoCrsr->Move( fnMoveForward, fnGoNode );
    {
        // remove actions - TODO: why?
        UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
    }
    pFrameFormat->GetDoc()->InsertRow(*pUnoCrsr, (sal_uInt16)nCount, bAppend);
}

void SwXTableRows::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrameFormat* pFrameFormat(lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this)));
    if(nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    SwTable* pTable = lcl_EnsureTableNotComplex(SwTable::FindTable(pFrameFormat), static_cast<cppu::OWeakObject*>(this));
    OUString sTLName = sw_GetCellName(0, nIndex);
    const SwTableBox* pTLBox = pTable->GetTableBox(sTLName);
    if(!pTLBox)
        throw uno::RuntimeException("Illegal arguments", static_cast<cppu::OWeakObject*>(this));
    const SwStartNode* pSttNd = pTLBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    // set cursor to the upper-left cell of the range
    auto pUnoCrsr(pFrameFormat->GetDoc()->CreateUnoCrsr(aPos, true));
    pUnoCrsr->Move(fnMoveForward, fnGoNode);
    pUnoCrsr->SetRemainInSection( false );
    const OUString sBLName = sw_GetCellName(0, nIndex + nCount - 1);
    const SwTableBox* pBLBox = pTable->GetTableBox( sBLName );
    if(!pBLBox)
        throw uno::RuntimeException("Illegal arguments", static_cast<cppu::OWeakObject*>(this));
    pUnoCrsr->SetMark();
    pUnoCrsr->GetPoint()->nNode = *pBLBox->GetSttNd();
    pUnoCrsr->Move(fnMoveForward, fnGoNode);
    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr.get());
    {
        // HACK: remove pending actions for selecting old style tables
        UnoActionRemoveContext aRemoveContext(*pCrsr);
    }
    pCrsr->MakeBoxSels();
    {   // these braces are important
        UnoActionContext aAction(pFrameFormat->GetDoc());
        pFrameFormat->GetDoc()->DeleteRow(*pUnoCrsr);
        pUnoCrsr.reset();
    }
    {
        // invalidate all actions - TODO: why?
        UnoActionRemoveContext aRemoveContext(pFrameFormat->GetDoc());
    }
}

void SwXTableRows::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
    { ClientModify(this, pOld, pNew); }

// SwXTableColumns

OUString SwXTableColumns::getImplementationName() throw( uno::RuntimeException, std::exception )
    { return OUString("SwXTableColumns"); }

sal_Bool SwXTableColumns::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
    { return cppu::supportsService(this, rServiceName); }

uno::Sequence< OUString > SwXTableColumns::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
    { return { "com.sun.star.text.TableColumns"}; }

TYPEINIT1(SwXTableColumns, SwClient);

SwXTableColumns::SwXTableColumns(SwFrameFormat& rFrameFormat) :
    SwClient(&rFrameFormat)
{ }

SwXTableColumns::~SwXTableColumns()
{ }

sal_Int32 SwXTableColumns::getCount() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFrameFormat(lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this)));
    SwTable* pTable = SwTable::FindTable( pFrameFormat );
//    if(!pTable->IsTableComplex())
//        throw uno::RuntimeException("Table too complex", static_cast<cppu::OWeakObject*>(this));
    SwTableLines& rLines = pTable->GetTabLines();
    SwTableLine* pLine = rLines.front();
    return pLine->GetTabBoxes().size();
}

uno::Any SwXTableColumns::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    if(nIndex < 0 || getCount() <= nIndex)
        throw lang::IndexOutOfBoundsException();
    return uno::makeAny(uno::Reference<uno::XInterface>()); // i#21699 not supported
}

uno::Type SAL_CALL SwXTableColumns::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<uno::XInterface>::get();
}

sal_Bool SwXTableColumns::hasElements() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this));
    return sal_True;
}

///@see SwXTableRows::insertByIndex (TODO: seems to be copy and paste programming here)
void SwXTableColumns::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrameFormat* pFrameFormat(lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this)));
    SwTable* pTable = lcl_EnsureTableNotComplex(SwTable::FindTable(pFrameFormat), static_cast<cppu::OWeakObject*>(this));
    SwTableLines& rLines = pTable->GetTabLines();
    SwTableLine* pLine = rLines.front();
    const size_t nColCount = pLine->GetTabBoxes().size();
    if (nCount <= 0 || !(0 <= nIndex && static_cast<size_t>(nIndex) <= nColCount))
        throw uno::RuntimeException("Illegal arguments", static_cast<cppu::OWeakObject*>(this));
    const OUString sTLName = sw_GetCellName(nIndex, 0);
    const SwTableBox* pTLBox = pTable->GetTableBox( sTLName );
    bool bAppend = false;
    if(!pTLBox)
    {
        bAppend = true;
        // to append at the end the cursor must be in the last line
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        pTLBox = rBoxes.back();
    }
    if(!pTLBox)
        throw uno::RuntimeException("Illegal arguments", static_cast<cppu::OWeakObject*>(this));
    const SwStartNode* pSttNd = pTLBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    UnoActionContext aAction(pFrameFormat->GetDoc());
    auto pUnoCrsr(pFrameFormat->GetDoc()->CreateUnoCrsr(aPos, true));
    pUnoCrsr->Move(fnMoveForward, fnGoNode);

    {
        // remove actions - TODO: why?
        UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
    }

    pFrameFormat->GetDoc()->InsertCol(*pUnoCrsr, (sal_uInt16)nCount, bAppend);
}

///@see SwXTableRows::removeByIndex (TODO: seems to be copy and paste programming here)
void SwXTableColumns::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount)
    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (nCount == 0)
        return;
    SwFrameFormat* pFrameFormat(lcl_EnsureCoreConnected(GetFrameFormat(), static_cast<cppu::OWeakObject*>(this)));
    if(nIndex < 0 || nCount <=0 )
        throw uno::RuntimeException();
    SwTable* pTable = lcl_EnsureTableNotComplex(SwTable::FindTable(pFrameFormat), static_cast<cppu::OWeakObject*>(this));
    const OUString sTLName = sw_GetCellName(nIndex, 0);
    const SwTableBox* pTLBox = pTable->GetTableBox( sTLName );
    if(!pTLBox)
        throw uno::RuntimeException("Cell not found", static_cast<cppu::OWeakObject*>(this));
    const SwStartNode* pSttNd = pTLBox->GetSttNd();
    SwPosition aPos(*pSttNd);
    // set cursor to the upper-left cell of the range
    auto pUnoCrsr(pFrameFormat->GetDoc()->CreateUnoCrsr(aPos, true));
    pUnoCrsr->Move(fnMoveForward, fnGoNode);
    pUnoCrsr->SetRemainInSection(false);
    const OUString sTRName = sw_GetCellName(nIndex + nCount - 1, 0);
    const SwTableBox* pTRBox = pTable->GetTableBox(sTRName);
    if(!pTRBox)
        throw uno::RuntimeException("Cell not found", static_cast<cppu::OWeakObject*>(this));
    pUnoCrsr->SetMark();
    pUnoCrsr->GetPoint()->nNode = *pTRBox->GetSttNd();
    pUnoCrsr->Move(fnMoveForward, fnGoNode);
    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr.get());
    {
        // HACK: remove pending actions for selecting old style tables
        UnoActionRemoveContext aRemoveContext(*pCrsr);
    }
    pCrsr->MakeBoxSels();
    {   // these braces are important
        UnoActionContext aAction(pFrameFormat->GetDoc());
        pFrameFormat->GetDoc()->DeleteCol(*pUnoCrsr);
        pUnoCrsr.reset();
    }
    {
        // invalidate all actions - TODO: why?
        UnoActionRemoveContext aRemoveContext(pFrameFormat->GetDoc());
    }
}

void SwXTableColumns::Modify(const SfxPoolItem* pOld, const SfxPoolItem *pNew)
    { ClientModify(this, pOld, pNew); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
