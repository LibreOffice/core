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



// STL includes
#include <list>
#include <vector>

#include <svx/svxids.hrc>
#include <editeng/memberids.hrc>
#include <float.h> // for DBL_MIN
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
#include <svl/zforlist.hxx>     // SvNumberFormatter
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

using namespace ::com::sun::star;
using ::editeng::SvxBorderLine;

// from swtable.cxx
extern void sw_GetTblBoxColStr( sal_uInt16 nCol, String& rNm );

#define UNO_TABLE_COLUMN_SUM    10000

static void
lcl_SendChartEvent(::cppu::OWeakObject & rSource,
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

static void
lcl_SendChartEvent(::cppu::OWeakObject & rSource,
        ::cppu::OMultiTypeInterfaceContainerHelper & rListeners)
{
    ::cppu::OInterfaceContainerHelper *const pContainer(rListeners.getContainer(
            chart::XChartDataChangeEventListener::static_type()));
    if (pContainer)
    {
        lcl_SendChartEvent(rSource, *pContainer);
    }
}

static bool lcl_LineToSvxLine(const table::BorderLine& rLine, SvxBorderLine& rSvxLine)
{
    rSvxLine.SetColor(Color(rLine.Color));

    rSvxLine.GuessLinesWidths( table::BorderLineStyle::NONE,
                                MM100_TO_TWIP( rLine.OuterLineWidth ),
                                MM100_TO_TWIP( rLine.InnerLineWidth ),
                                MM100_TO_TWIP( rLine.LineDistance ) );

    bool bRet = rLine.InnerLineWidth > 0 || rLine.OuterLineWidth > 0;
    return bRet;
}

static void lcl_SetSpecialProperty(SwFrmFmt* pFmt, const SfxItemPropertySimpleEntry* pEntry, const uno::Any& aValue)
    throw (lang::IllegalArgumentException)
{
    //Sonderbehandlung fuer "Nicht-Items"
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
                    sal_Bool bVal = *(sal_Bool*)aValue.getValue();
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
                aSz.SetWidth ( MM100_TO_TWIP ( nWidth ) );
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
                sal_Bool bPercent = *(sal_Bool*)aValue.getValue();
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
            sal_uInt16 nRepeat = pTable->GetRowsToRepeat();
            if(pEntry->nWID == FN_TABLE_HEADLINE_REPEAT)
            {
                sal_Bool bTemp = nRepeat > 0;
                aRet.setValue(&bTemp, ::getCppuBooleanType());
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
                sal_Bool bTemp = 0 != rSz.GetWidthPercent();
                aRet.setValue(&bTemp, ::getBooleanCppuType());
            }
        }
        break;
        case RES_PAGEDESC:
        {
            const SfxItemSet& rSet = pFmt->GetAttrSet();
            const SfxPoolItem* pItem;
            String sPDesc;
            if(SFX_ITEM_SET == rSet.GetItemState(RES_PAGEDESC, sal_False, &pItem))
            {
                const SwPageDesc* pDsc = ((const SwFmtPageDesc*)pItem)->GetPageDesc();
                if(pDsc)
                {
                   sPDesc = SwStyleNameMapper::GetProgName(pDsc->GetName(), nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC );
                }
            }
            aRet <<= OUString(sPDesc);
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
            const SwRedlineTbl& rRedTbl = pFmt->GetDoc()->GetRedlineTbl();
            for(sal_uInt16 nRed = 0; nRed < rRedTbl.size(); nRed++)
            {
                const SwRedline* pRedline = rRedTbl[nRed];
                const SwNode* pRedPointNode = pRedline->GetNode(sal_True);
                const SwNode* pRedMarkNode = pRedline->GetNode(sal_False);
                if(pRedPointNode == pTblNode || pRedMarkNode == pTblNode)
                {
                    const SwNode* pStartOfRedline = SwNodeIndex(*pRedPointNode) <= SwNodeIndex(*pRedMarkNode) ?
                        pRedPointNode : pRedMarkNode;
                    sal_Bool bIsStart = pStartOfRedline == pTblNode;
                    aRet <<= SwXRedlinePortion::CreateRedlineProperties(*pRedline, bIsStart);
                    break;
                }
            }
        }
        break;
    }
    return aRet;
}

// returns the position for the cell with the specified name
// (note that the indices rColumn and rRow are 0 based here)
// Also since the implementations of tables does not really have
// columns using this function is appropriate only for tables
// that are not complex (i.e. where IsTblComplex() returns false).
//
// returns: both indices for column and row (all >= 0) if everything was Ok.
//          At least one value < 0 if sth was wrong.
//
// Sample for naming scheme of cell in a single row (in groups a 26):
// A1..Z1, a1..z1, AA1..AZ1, Aa1..Az1, BA1..BZ1, Ba1..Bz1, ...
void sw_GetCellPosition( const String &rCellName,
        sal_Int32 &rColumn, sal_Int32 &rRow)
{
    rColumn = rRow = -1;    // default return values indicating failure
    xub_StrLen nLen = rCellName.Len();
    if (nLen)
    {
        const sal_Unicode *pBuf = rCellName.GetBuffer();
        const sal_Unicode *pEnd = pBuf + nLen;
        while (pBuf < pEnd && !('0' <= *pBuf && *pBuf <= '9'))
            ++pBuf;
        // start of number found?
        if (pBuf < pEnd && ('0' <= *pBuf && *pBuf <= '9'))
        {
            OUString aColTxt(rCellName.GetBuffer(), pBuf - rCellName.GetBuffer());
            OUString aRowTxt(pBuf, (rCellName.GetBuffer() + nLen - pBuf));
            if (!aColTxt.isEmpty() && !aRowTxt.isEmpty())
            {
                sal_Int32 nColIdx = 0;
                sal_Int32 nLength = aColTxt.getLength();
                for (sal_Int32 i = 0;  i < nLength;  ++i)
                {
                    nColIdx = 52 * nColIdx;
                    if (i < nLength - 1)
                        ++nColIdx;
                    sal_Unicode cChar = aColTxt[i];
                    if ('A' <= cChar && cChar <= 'Z')
                        nColIdx = nColIdx + (cChar - 'A');
                    else if ('a' <= cChar && cChar <= 'z')
                        nColIdx = nColIdx + (26 + cChar - 'a');
                    else
                    {
                        nColIdx = -1;   // sth failed
                        break;
                    }
                }

                rColumn = nColIdx;
                rRow    = aRowTxt.toInt32() - 1;    // - 1 because indices ought to be 0 based
            }
        }
    }
    OSL_ENSURE( rColumn != -1 && rRow != -1, "failed to get column or row index" );
}

// arguments: must be non-empty strings with valid cell names
//
// returns: -1 if first cell < second cell
//           0 if both cells are equal
//          +1 if the first cell > second cell
//
// Note: this function probably also make sense only
//      for cell names of non-complex tables
int sw_CompareCellsByRowFirst( const String &rCellName1, const String &rCellName2 )
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

// arguments: must be non-empty strings with valid cell names
//
// returns: -1 if first cell < second cell
//           0 if both cells are equal
//          +1 if the first cell > second cell
//
// Note: this function probably also make sense only
//      for cell names of non-complex tables
int sw_CompareCellsByColFirst( const String &rCellName1, const String &rCellName2 )
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

// arguments: must be non-empty strings with valid cell names
//
// returns: -1 if first cell range < second cell range
//           0 if both cell ranges are identical
//          +1 if the first cell range > second cell range
//
// Note: this function probably also make sense only
//      for cell names of non-complex tables
int sw_CompareCellRanges(
        const String &rRange1StartCell, const String &rRange1EndCell,
        const String &rRange2StartCell, const String &rRange2EndCell,
        sal_Bool bCmpColsFirst )
{
    int (*pCompareCells)( const String &, const String & ) =
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

// returns the cell name for the cell at the specified position
// (note that the indices nColumn and nRow are 0 based here)
String sw_GetCellName( sal_Int32 nColumn, sal_Int32 nRow )
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

    String sCellName;
    if (nColumn < 0 || nRow < 0)
        return sCellName;
    sw_GetTblBoxColStr( static_cast< sal_uInt16 >(nColumn), sCellName );
    sCellName += OUString::number( nRow + 1 );
    return sCellName;
}

/** Find the top left or bottom right corner box in given table.
  Consider nested lines when finding the box.

  @param i_pTable the table

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
        OSL_ASSERT(rLines.size() != 0);
        if (!rLines.empty())
        {
            const SwTableLine* pLine(i_bTopLeft ? rLines.front() : rLines.back());
            OSL_ASSERT(pLine);
            const SwTableBoxes& rBoxes(pLine->GetTabBoxes());
            OSL_ASSERT(rBoxes.size() != 0);
            pBox = i_bTopLeft ? rBoxes.front() : rBoxes.back();
            OSL_ASSERT(pBox);
        }
        else
        {
            pBox = 0;
        }
    } while (pBox && !pBox->GetSttNd());
    return pBox;
}

// start cell should be in the upper-left corner of the range and
// end cell in the lower-right.
// I.e. from the four possible representation
//      A1:C5, C5:A1, A5:C1, C1:A5
// only A1:C5 is the one to use
void sw_NormalizeRange(
    String &rCell1,     // will hold the upper-left cell of the range upon return
    String &rCell2 )    // will hold the lower-right cell of the range upon return
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
    String sCellName = sw_GetCellName(nColumn, nRow);
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
    for( sal_uInt16 i = 0; i < rLines.size(); i++ )
    {
        SwTableLine* pLine = rLines[i];
        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
        for(sal_uInt16 j = 0; j < rBoxes.size(); j++)
        {
            SwTableBox* pBox = rBoxes[j];
            if(pBox->GetName().Len() && pBox->getRowSpan() > 0 )
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
            ((SwTabFrm*)pFrm)->SetONECalcLowers();
            ((SwTabFrm*)pFrm)->Calc();
        }
    }
}

static void lcl_CrsrSelect(SwPaM* pCrsr, sal_Bool bExpand)
{
    if(bExpand)
    {
        if(!pCrsr->HasMark())
            pCrsr->SetMark();
    }
    else if(pCrsr->HasMark())
        pCrsr->DeleteMark();

}

static void lcl_GetTblSeparators(uno::Any& rRet, SwTable* pTable, SwTableBox* pBox, sal_Bool bRow)
{
    SwTabCols aCols;
    aCols.SetLeftMin ( 0 );
    aCols.SetLeft    ( 0 );
    aCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aCols, pBox, sal_False, bRow );

    sal_uInt16 nSepCount = aCols.Count();
    uno::Sequence< text::TableColumnSeparator> aColSeq(nSepCount);
     text::TableColumnSeparator* pArray = aColSeq.getArray();
    bool bError = false;
    for(sal_uInt16 i = 0; i < nSepCount; i++)
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

static void lcl_SetTblSeparators(const uno::Any& rVal, SwTable* pTable, SwTableBox* pBox, sal_Bool bRow, SwDoc* pDoc)
{
    SwTabCols aOldCols;

    aOldCols.SetLeftMin ( 0 );
    aOldCols.SetLeft    ( 0 );
    aOldCols.SetRight   ( UNO_TABLE_COLUMN_SUM );
    aOldCols.SetRightMax( UNO_TABLE_COLUMN_SUM );

    pTable->GetTabCols( aOldCols, pBox, sal_False, bRow );
    sal_uInt16 nOldCount = aOldCols.Count();
    //there's no use in setting tab cols if there's only one column
    if( !nOldCount )
        return;

    const uno::Sequence< text::TableColumnSeparator>* pSepSeq =
                (uno::Sequence< text::TableColumnSeparator>*) rVal.getValue();
    if(pSepSeq && pSepSeq->getLength() == nOldCount)
    {
        SwTabCols aCols(aOldCols);
        sal_Bool bError = sal_False;
        const text::TableColumnSeparator* pArray = pSepSeq->getConstArray();
        sal_Int32 nLastValue = 0;
        //sal_Int32 nTblWidth = aCols.GetRight() - aCols.GetLeft();
        for(sal_uInt16 i = 0; i < nOldCount; i++)
        {
            aCols[i] = pArray[i].Position;
            if(pArray[i].IsVisible == aCols.IsHidden(i) ||
                (!bRow && aCols.IsHidden(i)) ||
                long(aCols[i] - long(nLastValue)) < 0 ||
                UNO_TABLE_COLUMN_SUM < aCols[i] )
            {
                bError = sal_True;
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

/*  non UNO function call to set string in SwXCell */
void sw_setString( SwXCell &rCell, const OUString &rTxt,
        sal_Bool bKeepNumberFmt = sal_False )
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
        // Der Text mu? zunaechst (vielleicht) geloescht werden
        sal_uLong nNdPos = rCell.pBox->IsValidNumTxtNd( sal_True );
        if(ULONG_MAX != nNdPos)
            sw_setString( rCell, OUString(), sal_True );   // sal_True == keep number format
        SwDoc* pDoc = rCell.GetDoc();
        UnoActionContext aAction(pDoc);
        SwFrmFmt* pBoxFmt = rCell.pBox->ClaimFrmFmt();
        SfxItemSet aSet(pDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE);
        const SfxPoolItem* pItem;

        //!! do we need to set a new number format? Yes, if
        // - there is no current number format
        // - the current number format is not a number format according to the number formatter, but rather a text format
        // - the current number format is not even a valid number formatter number format, but rather Writer's own 'special' text number format
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue())
            ||  ((SwTblBoxNumFormat*)pItem)->GetValue() == NUMBERFORMAT_TEXT)
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }

        SwTblBoxValue aVal(nVal);
        aSet.Put(aVal);
        pDoc->SetTblBoxFormulaAttrs( *rCell.pBox, aSet );
        //Tabelle aktualisieren
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( rCell.GetFrmFmt() ));
        pDoc->UpdateTblFlds( &aTblUpdate );
    }
}

/******************************************************************
 * SwXCell
 ******************************************************************/
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
    throw(uno::RuntimeException)
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

uno::Sequence< uno::Type > SAL_CALL SwXCell::getTypes(  ) throw(uno::RuntimeException)
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

namespace
{
    class theSwXCellImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXCellImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwXCell::getImplementationId(  ) throw(uno::RuntimeException)
{
    return theSwXCellImplementationId::get().getSeq();
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
    throw (uno::RuntimeException)
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

OUString SwXCell::getFormula(void) throw( uno::RuntimeException )
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

void SwXCell::setFormula(const OUString& rFormula) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(IsValid())
    {
        // Der Text mu? zunaechst (vielleicht) geloescht werden
        sal_uInt32 nNdPos = pBox->IsValidNumTxtNd( sal_True );
        if(USHRT_MAX == nNdPos)
            sw_setString( *this, OUString(), sal_True );
        String sFml(comphelper::string::stripStart(rFormula, ' '));
        if( sFml.Len() && '=' == sFml.GetChar( 0 ) )
                    sFml.Erase( 0, 1 );
        SwTblBoxFormula aFml( sFml );
        SwDoc* pMyDoc = GetDoc();
        UnoActionContext aAction(pMyDoc);
        SfxItemSet aSet(pMyDoc->GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_FORMULA);
        const SfxPoolItem* pItem;
        SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
        if(SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
            ||  pMyDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue()))
        {
            aSet.Put(SwTblBoxNumFormat(0));
        }
        aSet.Put(aFml);
        GetDoc()->SetTblBoxFormulaAttrs( *pBox, aSet );
        //Tabelle aktualisieren
        SwTableFmlUpdate aTblUpdate( SwTable::FindTable( GetFrmFmt() ));
        pMyDoc->UpdateTblFlds( &aTblUpdate );
    }
}

double SwXCell::getValue(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    double const fRet = sw_getValue( *this );
    // #i112652# a table cell may contain NaN as a value, do not filter that
    return fRet;
}

void SwXCell::setValue(double rValue) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sw_setValue( *this, rValue );
}

table::CellContentType SwXCell::getType(void) throw( uno::RuntimeException )
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

void SwXCell::setString(const OUString& aString) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sw_setString( *this, aString );
}

sal_Int32 SwXCell::getError(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString sContent = getString();
    return sContent.equals(ViewShell::GetShellRes()->aCalc_Error);
}

uno::Reference< text::XTextCursor >  SwXCell::createTextCursor(void) throw( uno::RuntimeException )
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
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE | nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
//          throw( uno::RuntimeException() );
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< text::XTextCursor >  SwXCell::createTextCursorByRange(const uno::Reference< text::XTextRange > & xTextPosition)
                                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwUnoInternalPaM aPam(*GetDoc());
    if ((pStartNode || IsValid())
        && ::sw::XTextRangeToSwPaM(aPam, xTextPosition))
    {
        const SwStartNode* pSttNd = pStartNode ? pStartNode : pBox->GetSttNd();
        //skip sections
        SwStartNode* p1 = aPam.GetNode()->StartOfSectionNode();
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

uno::Reference< beans::XPropertySetInfo >  SwXCell::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXCell::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
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
            OSL_TRACE("FRMDirection val %d", nNum );
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
                    OSL_FAIL( "unknown direction code, maybe its a bitfield");
            }
            SvxFrameDirectionItem aItem( eDir, RES_FRAMEDIR);
            pBox->GetFrmFmt()->SetFmtAttr(aItem);
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
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
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
                aRet <<= OUString ( pBox->GetName() );
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

void SwXCell::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCell::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCell::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCell::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

uno::Reference< container::XEnumeration >  SwXCell::createEnumeration(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
    if(IsValid())
    {
        const SwStartNode* pSttNd = pBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        ::std::auto_ptr<SwUnoCrsr> pUnoCursor(
            GetDoc()->CreateUnoCrsr(aPos, false));
        pUnoCursor->Move(fnMoveForward, fnGoNode);

        // remember table and start node for later travelling
        // (used in export of tables in tables)
        SwTable const*const pTable( & pSttNd->FindTableNode()->GetTable() );
        SwXParagraphEnumeration *const pEnum =
            new SwXParagraphEnumeration(this, pUnoCursor, CURSOR_TBLTEXT,
                    pSttNd, pTable);

        aRef = pEnum;
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( nsSwCursorSelOverFlags::SELOVER_TOGGLE | nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ))
//          throw( uno::RuntimeException() );
    }
    return aRef;
}

uno::Type SAL_CALL SwXCell::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<text::XTextRange>*)0);

}

sal_Bool SwXCell::hasElements(void) throw( uno::RuntimeException )
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

        //wenn es die Box gibt, dann wird auch eine Zelle zurueckgegeben
        if( it != pTable->GetTabSortBoxes().end() )
        {
            size_t const nPos = it - pTable->GetTabSortBoxes().begin();
            SwIterator<SwXCell,SwFmt> aIter( *pTblFmt );
            SwXCell* pXCell = aIter.First();
            while( pXCell )
            {
                // gibt es eine passende Zelle bereits?
                if(pXCell->GetTblBox() == pBox)
                    break;
                pXCell = aIter.Next();
            }
            //sonst anlegen
            if(!pXCell)
            {
                pXCell = new SwXCell(pTblFmt, pBox, nPos);
            }
            pRet = pXCell;
        }
    }
    return pRet;
}

/* does box exist in given table? */
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

OUString SwXCell::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXCell");
}

sal_Bool SwXCell::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXCell::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.text.CellProperties";
    return aRet;
}

/******************************************************************
 * SwXTextTableRow
 ******************************************************************/
OUString SwXTextTableRow::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextTableRow");
}

sal_Bool SwXTextTableRow::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextTableRow::getSupportedServiceNames(void) throw( uno::RuntimeException )
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

uno::Reference< beans::XPropertySetInfo >  SwXTextTableRow::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXTextTableRow::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        SwTableLine* pLn = SwXTextTableRow::FindLine(pTable, pLine);
        if(pLn)
        {
            const SfxItemPropertySimpleEntry* pEntry =
                m_pPropSet->getPropertyMap().getByName(rPropertyName);
            SwDoc* pDoc = pFmt->GetDoc();
            if (!pEntry)
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
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
                        sal_Bool bSet = *(sal_Bool*)aValue.getValue();
                        aFrmSize.SetHeightSizeType(bSet ? ATT_VAR_SIZE : ATT_FIX_SIZE);
                    }
                    else
                    {
                        sal_Int32 nHeight = 0;
                        aValue >>= nHeight;
                         Size aSz(aFrmSize.GetSize());
                        aSz.Height() = MM100_TO_TWIP(nHeight);
                        aFrmSize.SetSize(aSz);
                    }
                    pDoc->SetAttr(aFrmSize, *pLn->ClaimFrmFmt());
                }
                break;
                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    UnoActionContext aContext(pDoc);
                    SwTable* pTable2 = SwTable::FindTable( pFmt );
                    lcl_SetTblSeparators(aValue, pTable2, pLine->GetTabBoxes()[0], sal_True, pDoc);
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

uno::Any SwXTextTableRow::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
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
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

            switch(pEntry->nWID)
            {
                case FN_UNO_ROW_HEIGHT:
                case FN_UNO_ROW_AUTO_HEIGHT:
                {
                    const SwFmtFrmSize& rSize = pLn->GetFrmFmt()->GetFrmSize();
                    if(FN_UNO_ROW_AUTO_HEIGHT== pEntry->nWID)
                    {
                        sal_Bool bTmp =  ATT_VAR_SIZE == rSize.GetHeightSizeType();
                        aRet.setValue(&bTmp, ::getCppuBooleanType());
                    }
                    else
                        aRet <<= (sal_Int32)(TWIP_TO_MM100(rSize.GetSize().Height()));
                }
                break;
                case FN_UNO_TABLE_COLUMN_SEPARATORS:
                {
                    lcl_GetTblSeparators(aRet, pTable, pLine->GetTabBoxes()[0], sal_True);
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

void SwXTextTableRow::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableRow::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
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
    for(sal_uInt16 i = 0; i < rLines.size(); i++)
        if(rLines[i] == pLine)
        {
            pRet = pLine;
            break;
        }
    return pRet;
}

/******************************************************************
 * SwXTextTableCursor
 ******************************************************************/
OUString SwXTextTableCursor::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextTableCursor");
}

sal_Bool SwXTextTableCursor::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

IMPLEMENT_FORWARD_XINTERFACE2(SwXTextTableCursor,SwXTextTableCursor_Base,OTextCursorHelper)
const SwPaM*        SwXTextTableCursor::GetPaM() const  { return GetCrsr(); }
SwPaM*              SwXTextTableCursor::GetPaM()        { return GetCrsr(); }
const SwDoc*        SwXTextTableCursor::GetDoc() const  { return GetFrmFmt()->GetDoc(); }
SwDoc*              SwXTextTableCursor::GetDoc()        { return GetFrmFmt()->GetDoc(); }
const SwUnoCrsr*    SwXTextTableCursor::GetCrsr() const { return (SwUnoCrsr*)aCrsrDepend.GetRegisteredIn(); }
SwUnoCrsr*          SwXTextTableCursor::GetCrsr()       { return (SwUnoCrsr*)aCrsrDepend.GetRegisteredIn(); }

uno::Sequence< OUString > SwXTextTableCursor::getSupportedServiceNames(void) throw( uno::RuntimeException )
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
    SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
    pTblCrsr->MakeBoxSels();
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
    SwTableCursor* pTableCrsr = dynamic_cast<SwTableCursor*>(pUnoCrsr);
    for (size_t i = 0; i < rBoxes.size(); i++)
    {
        pTableCrsr->InsertBox( *rBoxes[i] );
    }

    pUnoCrsr->Add(&aCrsrDepend);
    SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
    pTblCrsr->MakeBoxSels();
}

SwXTextTableCursor::~SwXTextTableCursor()
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    delete pUnoCrsr;
}

OUString SwXTextTableCursor::getRangeName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    OUString aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();

    //!! see also SwChartDataSequence::getSourceRangeRepresentation
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        pTblCrsr->MakeBoxSels();
        const SwStartNode* pNode = pTblCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();
        const SwTable* pTable = SwTable::FindTable( GetFrmFmt() );
        const SwTableBox* pEndBox = pTable->GetTblBox( pNode->GetIndex());
        String aTmp( pEndBox->GetName() );

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

                aTmp  = pStartBox->GetName();
                aTmp += ':';
                aTmp += pEndBox->GetName();
            }
        }
        aRet = aTmp;
    }
    return aRet;
}

sal_Bool SwXTextTableCursor::gotoCellByName(const OUString& CellName, sal_Bool Expand)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        String sCellName(CellName);
        bRet = pTblCrsr->GotoTblBox(sCellName);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goLeft(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->Left( Count,CRSR_SKIP_CHARS, sal_False, sal_False);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goRight(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->Right( Count, CRSR_SKIP_CHARS, sal_False, sal_False);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goUp(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->UpDown(sal_True, Count, 0, 0);
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::goDown(sal_Int16 Count, sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        bRet = pTblCrsr->UpDown(sal_False, Count, 0, 0);
    }
    return bRet;
}

void SwXTextTableCursor::gotoStart(sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        pTblCrsr->MoveTable(fnTableCurr, fnTableStart);
    }
}

void SwXTextTableCursor::gotoEnd(sal_Bool Expand) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        lcl_CrsrSelect( pTblCrsr, Expand );
        pTblCrsr->MoveTable(fnTableCurr, fnTableEnd);
    }
}

sal_Bool SwXTextTableCursor::mergeRange(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        {
            // hier muessen die Actions aufgehoben werden
            UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
        }
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        pTblCrsr->MakeBoxSels();

        {
            UnoActionContext aContext(pUnoCrsr->GetDoc());
            bRet = TBLMERGE_OK == pTblCrsr->GetDoc()->MergeTbl(*pTblCrsr);
            if(bRet)
            {
                size_t nCount = pTblCrsr->GetSelectedBoxesCount();
                while (--nCount)
                {
                    pTblCrsr->DeleteBox(nCount);
                }
            }
        }
        pTblCrsr->MakeBoxSels();
    }
    return bRet;
}

sal_Bool SwXTextTableCursor::splitRange(sal_Int16 Count, sal_Bool Horizontal) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if (Count <= 0)
        throw uno::RuntimeException("Illegal first argument: needs to be > 0", static_cast < cppu::OWeakObject * > ( this ) );
    sal_Bool bRet = sal_False;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        {
            // hier muessen die Actions aufgehoben werden
            UnoActionRemoveContext aRemoveContext(pUnoCrsr->GetDoc());
        }
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        pTblCrsr->MakeBoxSels();
        {
            UnoActionContext aContext(pUnoCrsr->GetDoc());
            bRet = pTblCrsr->GetDoc()->SplitTbl(
                    pTblCrsr->GetSelectedBoxes(), !Horizontal, Count);
        }
        pTblCrsr->MakeBoxSels();
    }
    return bRet;
}

uno::Reference< beans::XPropertySetInfo >  SwXTextTableCursor::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXTextTableCursor::setPropertyValue(const OUString& rPropertyName,
                                                        const uno::Any& aValue)
            throw( beans::UnknownPropertyException,
                        beans::PropertyVetoException,
                     lang::IllegalArgumentException,
                     lang::WrappedTargetException,
                     uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->StartOfSectionNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
                throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
            pTblCrsr->MakeBoxSels();
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
                    SwUnoCursorHelper::GetCrsrAttr(pTblCrsr->GetSelRing(),
                            aItemSet);

                    if (!SwUnoCursorHelper::SetCursorPropertyValue(
                            *pEntry, aValue, pTblCrsr->GetSelRing(), aItemSet))
                    {
                        m_pPropSet->setPropertyValue(*pEntry, aValue, aItemSet);
                    }
                    SwUnoCursorHelper::SetCrsrAttr(pTblCrsr->GetSelRing(),
                            aItemSet, nsSetAttrMode::SETATTR_DEFAULT, true);
                }
            }
        }
        else
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXTextTableCursor::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwUnoCrsr* pUnoCrsr = GetCrsr();
    if(pUnoCrsr)
    {
        SwStartNode* pSttNode = pUnoCrsr->GetNode()->StartOfSectionNode();
        const SwTableNode* pTblNode = pSttNode->FindTableNode();
        lcl_FormatTable((SwFrmFmt*)pTblNode->GetTable().GetFrmFmt());
        SwUnoTableCrsr* pTblCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
        const SfxItemPropertySimpleEntry* pEntry =
                                    m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            pTblCrsr->MakeBoxSels();
            switch(pEntry->nWID )
            {
                case FN_UNO_TABLE_CELL_BACKGROUND:
                {
                    SvxBrushItem aBrush( RES_BACKGROUND );
                    if(pTblCrsr->GetDoc()->GetBoxAttr( *pUnoCrsr, aBrush ))
                        aBrush.QueryValue(aRet, pEntry->nMemberId);

                }
                break;
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    OSL_FAIL("not implemented");
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl *const pFmt =
                        SwUnoCursorHelper::GetCurTxtFmtColl(*pUnoCrsr, sal_False);
                    OUString sRet;
                    if(pFmt)
                        sRet = pFmt->GetName();
                    aRet <<= sRet;
                }
                break;
                default:
                {
                    SfxItemSet aSet(pTblCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwUnoCursorHelper::GetCrsrAttr(pTblCrsr->GetSelRing(),
                            aSet);
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aRet);
                }
            }
        }
        else
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXTextTableCursor::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTableCursor::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
}

/******************************************************************
 * SwXTextTable
 ******************************************************************/

class SwXTextTable::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
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
    sal_Bool    GetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& rpAny);

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

sal_Bool SwTableProperties_Impl::GetProperty(sal_uInt16 nWhichId, sal_uInt16 nMemberId, const uno::Any*& rpAny )
{
    return aAnyMap.FillValue( nWhichId, nMemberId, rpAny );
}

void    SwTableProperties_Impl::ApplyTblAttr(const SwTable& rTbl, SwDoc& rDoc)
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
        sal_Bool bVal = *(sal_Bool*)pRepHead->getValue();
        ((SwTable&)rTbl).SetRowsToRepeat( bVal ? 1 : 0 );  // TODO MULTIHEADER
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
        SvxBrushItem aBrush ( rFrmFmt.GetBackground() );
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
    sal_Bool bTemp = pSzRel ? *(sal_Bool*)pSzRel->getValue() : sal_False;
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
        sal_Bool bTmp = *(sal_Bool*)pSplit->getValue();
        SwFmtLayoutSplit aSp(bTmp);
        aSet.Put(aSp);
    }

    //TODO: folgende Propertiers noch impl.
//  FN_UNO_RANGE_ROW_LABEL
//  FN_UNO_RANGE_COL_LABEL
//  FN_UNO_TABLE_BORDER

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
    throw(uno::RuntimeException)
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
    bIsDescriptor(sal_True),
    nRows(2),
    nColumns(2),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False)
{
}

SwXTextTable::SwXTextTable(SwFrmFmt& rFrmFmt)
    : SwClient( &rFrmFmt )
    , m_pImpl(new Impl)
    ,
    m_pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_TABLE)),
    pTableProps(0),
    bIsDescriptor(sal_False),
    nRows(0),
    nColumns(0),
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False)
{
}

SwXTextTable::~SwXTextTable()
{
    delete pTableProps;
}

void SwXTextTable::initialize(sal_Int32 nR, sal_Int32 nC) throw( uno::RuntimeException )
{
    if(!bIsDescriptor || nR <= 0 || nC <= 0 || nR >= USHRT_MAX || nC >= USHRT_MAX )
        throw uno::RuntimeException();
    else
    {
        nRows = (sal_uInt16)nR;
        nColumns = (sal_uInt16)nC;
    }
}

uno::Reference< table::XTableRows >  SwXTextTable::getRows(void) throw( uno::RuntimeException )
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

uno::Reference< table::XTableColumns >  SwXTextTable::getColumns(void) throw( uno::RuntimeException )
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

uno::Reference< table::XCell >  SwXTextTable::getCellByName(const OUString& CellName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        String sCellName(CellName);
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

uno::Sequence< OUString > SwXTextTable::getCellNames(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
          // gibts an der Tabelle und an allen Boxen
        SwTableLines& rTblLines = pTable->GetTabLines();
        std::vector<OUString*> aAllNames;
        lcl_InspectLines(rTblLines, aAllNames);
        uno::Sequence< OUString > aRet( static_cast<sal_uInt16>(aAllNames.size()) );
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

uno::Reference< text::XTextTableCursor >  SwXTextTable::createCursorByCellName(const OUString& CellName)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextTableCursor >  xRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        String sCellName(CellName);
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
        //das muss jetzt sal_True liefern
        ::sw::XTextRangeToSwPaM(aPam, xTextRange);

        {
            UnoActionContext aCont( pDoc );

            pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
            const SwTable *pTable = 0;
            if( 0 != aPam.Start()->nContent.GetIndex() )
            {
                pDoc->SplitNode(*aPam.Start(), false );
            }
            //TODO: wenn es der letzte Absatz ist, dann muss noch ein Absatz angehaengt werden!
            if( aPam.HasMark() )
            {
                pDoc->DeleteAndJoin(aPam);
                aPam.DeleteMark();
            }
            pTable = pDoc->InsertTable( SwInsertTableOptions( tabopts::HEADLINE | tabopts::DEFAULT_BORDER | tabopts::SPLIT_LAYOUT, 0 ),
                                        *aPam.GetPoint(),
                                        nRows,
                                        nColumns,
                                        text::HoriOrientation::FULL );
            if(pTable)
            {
                // hier muessen die Properties des Descriptors ausgewertet werden
                pTableProps->ApplyTblAttr(*pTable, *pDoc);
                SwFrmFmt* pTblFmt = pTable->GetFrmFmt();
                lcl_FormatTable( pTblFmt );

                pTblFmt->Add(this);
                if(m_sTableName.Len())
                {
                    sal_uInt16 nIndex = 1;
                    const String sTmpName(m_sTableName);
                    String sTmpNameIndex(sTmpName);
                    while(pDoc->FindTblFmtByName( sTmpNameIndex, sal_True ) && nIndex < USHRT_MAX)
                    {
                        sTmpNameIndex = sTmpName;
                        sTmpNameIndex += nIndex++;
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
                bIsDescriptor = sal_False;
                DELETEZ(pTableProps);
            }
            pDoc->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
        }
    }
    else
        throw lang::IllegalArgumentException();
}

void SwXTextTable::attach(const uno::Reference< text::XTextRange > & xTextRange)
        throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    attachToRange( xTextRange );
}

uno::Reference< text::XTextRange >  SwXTextTable::getAnchor(void)
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(!pFmt)
        throw uno::RuntimeException();
    uno::Reference< text::XTextRange >  xRet = new SwXTextRange(*pFmt);
    return xRet;
}

void SwXTextTable::dispose(void) throw( uno::RuntimeException )
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
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.addInterface(
            lang::XEventListener::static_type(), xListener);
}

void SAL_CALL SwXTextTable::removeEventListener(
        const uno::Reference< lang::XEventListener > & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.removeInterface(
            lang::XEventListener::static_type(), xListener);
}

uno::Reference< table::XCell >  SwXTextTable::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCell >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    // Sheet interessiert nicht
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
                    const String& rTLName, const String& rBRName,
                    SwRangeDescriptor& rDesc)
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRef;
    String sTLName(rTLName);
    String sBRName(rBRName);
    const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
    if(pTLBox)
    {
        // hier muessen die Actions aufgehoben werden
        UnoActionRemoveContext aRemoveContext(pFmt->GetDoc());
        const SwStartNode* pSttNd = pTLBox->GetSttNd();
        SwPosition aPos(*pSttNd);
        // Cursor in die obere linke Zelle des Ranges setzen
        SwUnoCrsr* pUnoCrsr = pFmt->GetDoc()->CreateUnoCrsr(aPos, true);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        pUnoCrsr->SetRemainInSection( sal_False );
        const SwTableBox* pBRBox = pTable->GetTblBox( sBRName );
        if(pBRBox)
        {
            pUnoCrsr->SetMark();
            pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
            pUnoCrsr->Move( fnMoveForward, fnGoNode );
            SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
            pCrsr->MakeBoxSels();
            // pUnoCrsr wird uebergeben und nicht geloescht
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
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
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
            String sTLName = sw_GetCellName(aDesc.nLeft, aDesc.nTop);
            String sBRName = sw_GetCellName(aDesc.nRight, aDesc.nBottom);

            // please note that according to the 'if' statement at the begin
            // sTLName:sBRName already denotes the normalized range string

            aRef = GetRangeByName(pFmt, pTable, sTLName, sBRName, aDesc);
        }
    }
    if(!aRef.is())
        throw lang::IndexOutOfBoundsException();
    return aRef;
}

uno::Reference< table::XCellRange >  SwXTextTable::getCellRangeByName(const OUString& aRange)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< table::XCellRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {
            String sRange(aRange);
            String sTLName(sRange.GetToken(0, ':'));
            String sBRName(sRange.GetToken(1, ':'));
            if(!sTLName.Len() || !sBRName.Len())
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
    throw (uno::RuntimeException)
{
    // see SwXTextTable::getData(...) also

    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
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
                    bool bIsNum = pBoxFmt->GetItemState( RES_BOXATR_VALUE, sal_False ) == SFX_ITEM_SET;
                    //const SfxPoolItem* pItem;
                    //SwDoc* pDoc = pXCell->GetDoc();
                    //sal_Bool bIsText = (SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
                    //          ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue())
                    //          ||  ((SwTblBoxNumFormat*)pItem)->GetValue() == NUMBERFORMAT_TEXT);

                    if(!bIsNum/*bIsText*/)
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
    throw (uno::RuntimeException)
{
    // see SwXTextTable::setData(...) also

    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();

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
                            sw_setString( *pXCell, OUString(), sal_True );

                    }
                }
            }
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXTextTable::getData(void)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    //
    SwFrmFmt* pFmt = GetFrmFmt();
    uno::Sequence< uno::Sequence< double > > aRowSeq(bFirstRowAsLabel ? nRowCount - 1 : nRowCount);
    if(pFmt)
    {
        uno::Sequence< double >* pArray = aRowSeq.getArray();

        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            uno::Sequence< double >  aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
            double* pColArray = aColSeq.getArray();
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
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
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    bool bChanged = false;

    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        if(rData.getLength() < nRowCount - nRowStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pRowArray = rData.getConstArray();
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< double >& rColSeq = pRowArray[nRow - nRowStart];
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
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

uno::Sequence< OUString > SwXTextTable::getRowDescriptions(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
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
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
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
            OSL_FAIL("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Int16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < (bFirstRowAsLabel ? nRowCount - 1 : nRowCount))
        {
            throw uno::RuntimeException();
        }
        const OUString* pArray = rRowDesc.getConstArray();
        if(bFirstColumnAsLabel)
        {
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
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
            OSL_FAIL("Wohin mit den Labels?");
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Sequence< OUString > SwXTextTable::getColumnDescriptions(void)
                                                throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
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
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
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
            OSL_FAIL("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXTextTable::setColumnDescriptions(const uno::Sequence< OUString >& rColumnDesc) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
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
        if(bFirstRowAsLabel && rColumnDesc.getLength() >= nColCount - bFirstColumnAsLabel ? 1 : 0)
        {
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
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
            OSL_FAIL("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
}

void SAL_CALL SwXTextTable::addChartDataChangeEventListener(
    const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.addInterface(
            chart::XChartDataChangeEventListener::static_type(), xListener);
}

void SAL_CALL SwXTextTable::removeChartDataChangeEventListener(
    const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_Listeners.removeInterface(
            chart::XChartDataChangeEventListener::static_type(), xListener);
}

sal_Bool SwXTextTable::isNotANumber(double nNumber) throw( uno::RuntimeException )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return ( nNumber == DBL_MIN );
}

double SwXTextTable::getNotANumber(void) throw( uno::RuntimeException )
{
    // We use DBL_MIN because starcalc does (which uses it because chart
    // wants it that way!)
    return DBL_MIN;
}

uno::Sequence< beans::PropertyValue > SwXTextTable::createSortDescriptor(void)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    return SwUnoCursorHelper::CreateSortDescriptor(true);
}

void SwXTextTable::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw( uno::RuntimeException )
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

void SwXTextTable::autoFormat(const OUString& aName) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwTable* pTable = SwTable::FindTable( pFmt );
        if(!pTable->IsTblComplex())
        {

            String sAutoFmtName(aName);
            SwTableAutoFmtTbl aAutoFmtTbl;
            aAutoFmtTbl.Load();
            for (sal_uInt16 i = aAutoFmtTbl.size(); i;)
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

uno::Reference< beans::XPropertySetInfo >  SwXTextTable::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXTextTable::setPropertyValue(const OUString& rPropertyName,
                                                    const uno::Any& aValue)
        throw( beans::UnknownPropertyException, beans::PropertyVetoException,
                lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
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
                case UNO_NAME_TABLE_NAME :
                {
                    OUString sName;
                    aValue >>= sName;
                    setName( sName );
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstRowAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
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


                    // hier muessen die Actions aufgehoben werden
                    UnoActionRemoveContext aRemoveContext(pDoc);
                    const SwTableBox* pTLBox = lcl_FindCornerTableBox(rLines, true);
                    const SwStartNode* pSttNd = pTLBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    // set cursor to top left cell
                    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, true);
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    pUnoCrsr->SetRemainInSection( sal_False );



                    const SwTableBox* pBRBox = lcl_FindCornerTableBox(rLines, false);
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();

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

                    aBox.SetDistance((sal_uInt16)MM100_TO_TWIP(aBorder.Distance));
                    aBoxInfo.SetValid(VALID_DISTANCE, aBorder.IsDistanceValid);

                    aSet.Put(aBox);
                    aSet.Put(aBoxInfo);

                    pDoc->SetTabBorders(*pCrsr, aSet);
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

                    sal_uInt16 nLeftDistance =     MM100_TO_TWIP_UNSIGNED( aTableBorderDistances.LeftDistance);
                    sal_uInt16 nRightDistance =    MM100_TO_TWIP_UNSIGNED( aTableBorderDistances.RightDistance);
                    sal_uInt16 nTopDistance =      MM100_TO_TWIP_UNSIGNED( aTableBorderDistances.TopDistance);
                    sal_uInt16 nBottomDistance =   MM100_TO_TWIP_UNSIGNED( aTableBorderDistances.BottomDistance);
                    SwDoc* pDoc = pFmt->GetDoc();
                    SwTable* pTable = SwTable::FindTable( pFmt );
                    SwTableLines &rLines = pTable->GetTabLines();
                    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_START, NULL);
                    for(sal_uInt16 i = 0; i < rLines.size(); i++)
                    {
                        SwTableLine* pLine = rLines[i];
                        SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                        for(sal_uInt16 k = 0; k < rBoxes.size(); k++)
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
                    lcl_SetTblSeparators(aValue, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], sal_False, pFmt->GetDoc());
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

uno::Any SwXTextTable::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    const SfxItemPropertySimpleEntry* pEntry =
                                m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if(pFmt)
    {
        if (!pEntry)
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        if(0xFF == pEntry->nMemberId)
        {
            aRet = lcl_GetSpecialProperty(pFmt, pEntry );
        }
        else
        {
            switch(pEntry->nWID)
            {
                case UNO_NAME_TABLE_NAME:
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
                    sal_Bool bTemp = bFirstRowAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTemp = bFirstColumnAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
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

                    // hier muessen die Actions aufgehoben werden
                    UnoActionRemoveContext aRemoveContext(pDoc);
                    const SwTableBox* pTLBox = lcl_FindCornerTableBox(rLines, true);
                    const SwStartNode* pSttNd = pTLBox->GetSttNd();
                    SwPosition aPos(*pSttNd);
                    // set cursor to top left cell
                    SwUnoCrsr* pUnoCrsr = pDoc->CreateUnoCrsr(aPos, true);
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    pUnoCrsr->SetRemainInSection( sal_False );

                    const SwTableBox* pBRBox = lcl_FindCornerTableBox(rLines, false);
                    pUnoCrsr->SetMark();
                    const SwStartNode* pLastNd = pBRBox->GetSttNd();
                    pUnoCrsr->GetPoint()->nNode = *pLastNd;

                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();

                    SfxItemSet aSet(pDoc->GetAttrPool(),
                                    RES_BOX, RES_BOX,
                                    SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                                    0);
                    aSet.Put(SvxBoxInfoItem( SID_ATTR_BORDER_INNER ));
                    pDoc->GetTabBorders(*pCrsr, aSet);
                    const SvxBoxInfoItem& rBoxInfoItem =
                        (const SvxBoxInfoItem&)aSet.Get(SID_ATTR_BORDER_INNER);
                    const SvxBoxItem& rBox = (const SvxBoxItem&)aSet.Get(RES_BOX);

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
                        aTableBorder.Distance               = TWIP_TO_MM100_UNSIGNED( rBox.GetDistance() );
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
                        aTableBorder.Distance               = TWIP_TO_MM100_UNSIGNED( rBox.GetDistance() );
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

                    for(sal_uInt16 i = 0; i < rLines.size(); i++)
                    {
                        const SwTableLine* pLine = rLines[i];
                        const SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                        for(sal_uInt16 k = 0; k < rBoxes.size(); k++)
                        {
                            const SwTableBox* pBox = rBoxes[k];
                            SwFrmFmt* pBoxFmt = pBox->GetFrmFmt();
                            const SvxBoxItem& rBox = pBoxFmt->GetBox();
                            if( bFirst )
                            {
                                nLeftDistance =     TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_LEFT   ));
                                nRightDistance =    TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_RIGHT  ));
                                nTopDistance =      TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_TOP    ));
                                nBottomDistance =   TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_BOTTOM ));
                                bFirst = false;
                            }
                            else
                            {
                                if( aTableBorderDistances.IsLeftDistanceValid &&
                                    nLeftDistance != TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_LEFT   )))
                                    aTableBorderDistances.IsLeftDistanceValid = sal_False;
                                if( aTableBorderDistances.IsRightDistanceValid &&
                                    nRightDistance != TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_RIGHT   )))
                                    aTableBorderDistances.IsRightDistanceValid = sal_False;
                                if( aTableBorderDistances.IsTopDistanceValid &&
                                    nTopDistance != TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_TOP   )))
                                    aTableBorderDistances.IsTopDistanceValid = sal_False;
                                if( aTableBorderDistances.IsBottomDistanceValid &&
                                    nBottomDistance != TWIP_TO_MM100_UNSIGNED( rBox.GetDistance( BOX_LINE_BOTTOM   )))
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
                    lcl_GetTblSeparators(aRet, pTable, pTable->GetTabLines()[0]->GetTabBoxes()[0], sal_False);
                }
                break;
                case FN_UNO_TABLE_COLUMN_RELATIVE_SUM:
                    aRet <<= (sal_Int16) UNO_TABLE_COLUMN_SUM;
                break;
                case RES_ANCHOR:
                    //AnchorType ist readonly und maybevoid und wird nicht geliefert
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

void SwXTextTable::addPropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTable::removePropertyChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTable::addVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXTextTable::removeVetoableChangeListener(const OUString& /*rPropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*xListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

OUString SwXTextTable::getName(void) throw( uno::RuntimeException )
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

void SwXTextTable::setName(const OUString& rName) throw( uno::RuntimeException )
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
        const String aOldName( pFmt->GetName() );
        SwFrmFmt* pTmpFmt;
        const SwFrmFmts* pTbl = pFmt->GetDoc()->GetTblFrmFmts();
        for( sal_uInt16 i = pTbl->size(); i; )
            if( !( pTmpFmt = (*pTbl)[ --i ] )->IsDefault() &&
                pTmpFmt->GetName() == rName &&
                            pFmt->GetDoc()->IsUsed( *pTmpFmt ))
            {
                throw uno::RuntimeException();
            }

        pFmt->SetName( rName );

        SwStartNode *pStNd;
        SwNodeIndex aIdx( *pFmt->GetDoc()->GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
        while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
        {
            ++aIdx;
            SwNode *const pNd = & aIdx.GetNode();
            if ( pNd->IsOLENode() &&
                aOldName == ((SwOLENode*)pNd)->GetChartTblName() )
            {
                ((SwOLENode*)pNd)->SetChartTblName( rName );

                ((SwOLENode*)pNd)->GetOLEObj();

                SwTable* pTable = SwTable::FindTable( pFmt );
                //TL_CHART2: chart needs to be notfied about name changes
                pFmt->GetDoc()->UpdateCharts( pTable->GetFrmFmt()->GetName() );
            }
            aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
        }
        pFmt->GetDoc()->SetModified();
    }
    else
        m_sTableName = rName;
}

sal_uInt16 SwXTextTable::getRowCount(void)
{
    SolarMutexGuard aGuard;
    sal_Int16 nRet = 0;
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
    sal_Int16 nRet = 0;
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
        (void*)GetRegisteredIn() == ((SwPtrMsgPoolItem *)pOld)->pObject )
            ((SwModify*)GetRegisteredIn())->Remove(this);
    else
        ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
        m_pImpl->m_Listeners.disposeAndClear(ev);
    }
    else
    {
        lcl_SendChartEvent(*this, m_pImpl->m_Listeners);
    }
}

OUString SAL_CALL SwXTextTable::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextTable");
}

sal_Bool SwXTextTable::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextTable::getSupportedServiceNames(void) throw( uno::RuntimeException )
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
    throw(uno::RuntimeException)
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

OUString SwXCellRange::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXCellRange");
}

sal_Bool SwXCellRange::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXCellRange::getSupportedServiceNames(void) throw( uno::RuntimeException )
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
    bFirstRowAsLabel(sal_False),
    bFirstColumnAsLabel(sal_False)
{
    aRgDesc.Normalize();
}

SwXCellRange::~SwXCellRange()
{
    SolarMutexGuard aGuard;
    delete pTblCrsr;
}

uno::Reference< table::XCell >  SwXCellRange::getCellByPosition(sal_Int32 nColumn, sal_Int32 nRow)
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
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
    throw( uno::RuntimeException, lang::IndexOutOfBoundsException )
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
            String sTLName = sw_GetCellName(aNewDesc.nLeft, aNewDesc.nTop);
            String sBRName = sw_GetCellName(aNewDesc.nRight, aNewDesc.nBottom);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                // hier muessen die Actions aufgehoben
                UnoActionRemoveContext aRemoveContext(pFmt->GetDoc());
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                SwUnoCrsr* pUnoCrsr = pFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                const SwTableBox* pBRBox = pTable->GetTblBox( sBRName );
                if(pBRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();
                    // pUnoCrsr wird uebergeben und nicht geloescht
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
        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    String sRange(rRange);
    String sTLName(sRange.GetToken(0, ':'));
    String sBRName(sRange.GetToken(1, ':'));
    if(!sTLName.Len() || !sBRName.Len())
        throw uno::RuntimeException();
    SwRangeDescriptor aDesc;
    aDesc.nTop = aDesc.nLeft = aDesc.nBottom = aDesc.nRight = -1;
    sw_GetCellPosition( sTLName, aDesc.nLeft, aDesc.nTop );
    sw_GetCellPosition( sBRName, aDesc.nRight, aDesc.nBottom );
    aDesc.Normalize();
    return getCellRangeByPosition(aDesc.nLeft - aRgDesc.nLeft, aDesc.nTop - aRgDesc.nTop,
                aDesc.nRight - aRgDesc.nLeft, aDesc.nBottom - aRgDesc.nTop);
}

uno::Reference< beans::XPropertySetInfo >  SwXCellRange::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRef = m_pPropSet->getPropertySetInfo();
    return xRef;
}

void SwXCellRange::setPropertyValue(const OUString& rPropertyName,
    const uno::Any& aValue) throw( beans::UnknownPropertyException,
        beans::PropertyVetoException, lang::IllegalArgumentException,
            lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        /* ASK OLIVER
        lcl_FormatTable(pFmt);*/
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
            SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
            pCrsr->MakeBoxSels();
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
                    aBoxInfo.SetValid(0xff, sal_False);
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
                    aBoxInfo.SetValid(nValid, sal_True);


                    aSet.Put(aBoxInfo);
                    pDoc->GetTabBorders(*pCrsr, aSet);

                    aSet.Put(aBoxInfo);
                    SvxBoxItem aBoxItem((const SvxBoxItem&)aSet.Get(RES_BOX));
                    ((SfxPoolItem&)aBoxItem).PutValue(aValue, pEntry->nMemberId);
                    aSet.Put(aBoxItem);
                    pDoc->SetTabBorders( *pTblCrsr, aSet );
                }
                break;
                case RES_BOXATR_FORMAT:
                {
                    SfxUInt32Item aNumberFormat(RES_BOXATR_FORMAT);
                    ((SfxPoolItem&)aNumberFormat).PutValue(aValue, 0);
                    pDoc->SetBoxAttr( *pCrsr, aNumberFormat);
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
                    if(bFirstRowAsLabel != bTmp)
                    {
                        lcl_SendChartEvent(*this, m_ChartListeners);
                        bFirstRowAsLabel = bTmp;
                    }
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTmp = *(sal_Bool*)aValue.getValue();
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
                    SwUnoCursorHelper::GetCrsrAttr(pCrsr->GetSelRing(),
                            aItemSet);

                    if (!SwUnoCursorHelper::SetCursorPropertyValue(
                            *pEntry, aValue, pCrsr->GetSelRing(), aItemSet))
                    {
                        m_pPropSet->setPropertyValue(*pEntry, aValue, aItemSet);
                    }
                    SwUnoCursorHelper::SetCrsrAttr(pCrsr->GetSelRing(),
                            aItemSet, nsSetAttrMode::SETATTR_DEFAULT, true);
                }
            }
        }
        else
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
}

uno::Any SwXCellRange::getPropertyValue(const OUString& rPropertyName) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        /* ASK OLIVER
        lcl_FormatTable(pFmt);*/
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
                    const SvxBoxItem& rBoxItem = ((const SvxBoxItem&)aSet.Get(RES_BOX));
                    rBoxItem.QueryValue(aRet, pEntry->nMemberId);
                }
                break;
                case RES_BOXATR_FORMAT:
                    //GetAttr fuer Tabellenselektion am Doc fehlt noch
                    OSL_FAIL("not implemented");
                break;
                case FN_UNO_PARA_STYLE:
                {
                    SwFmtColl *const pTmpFmt =
                        SwUnoCursorHelper::GetCurTxtFmtColl(*pTblCrsr, sal_False);
                    OUString sRet;
                    if(pFmt)
                        sRet = pTmpFmt->GetName();
                    aRet <<= sRet;
                }
                break;
                case FN_UNO_RANGE_ROW_LABEL:
                {
                    sal_Bool bTemp = bFirstRowAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                case FN_UNO_RANGE_COL_LABEL:
                {
                    sal_Bool bTemp = bFirstColumnAsLabel;
                    aRet.setValue(&bTemp, ::getCppuBooleanType());
                }
                break;
                default:
                {
                    SfxItemSet aSet(pTblCrsr->GetDoc()->GetAttrPool(),
                        RES_CHRATR_BEGIN,       RES_FRMATR_END -1,
                        RES_TXTATR_UNKNOWN_CONTAINER, RES_TXTATR_UNKNOWN_CONTAINER,
                        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                        0L);
                    // erstmal die Attribute des Cursors
                    SwUnoTableCrsr* pCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
                    SwUnoCursorHelper::GetCrsrAttr(pCrsr->GetSelRing(), aSet);
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aRet);
                }
            }
        }
        else
           throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    return aRet;
}

void SwXCellRange::addPropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::removePropertyChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::addVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::removeVetoableChangeListener(const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/) throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXCellRange::GetDataSequence(
        uno::Sequence< uno::Any >   *pAnySeq,   //-> first pointer != 0 is used
        uno::Sequence< OUString >   *pTxtSeq,   //-> as output sequence
        uno::Sequence< double >     *pDblSeq,   //-> (previous data gets overwritten)
        sal_Bool bForceNumberResults )          //-> when 'true' requires to make an
                                                // extra effort to return a value different
                                                // from 0 even if the cell is formatted to text
    throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    // compare to SwXCellRange::getDataArray (note different return types though)

    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //
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
                        bool bIsNum = pBox->GetFrmFmt()->GetItemState( RES_BOXATR_VALUE, sal_False ) == SFX_ITEM_SET;
                        //sal_uLong nNdPos = pBox->IsValidNumTxtNd( sal_True );
                        if (!bIsNum/* && ULONG_MAX == nNdPos*/)
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
                            SfxItemState eState = pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem);

                            if (eState == SFX_ITEM_SET)
                            {
                                // please note that the language of the numberformat
                                // is implicitly coded into the below value as well
                                nFIndex = ((SwTblBoxNumFormat*)pItem)->GetValue();

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
                                // it is at least conform to to what is used in
                                // SwTableShell::Execute when
                                // SID_ATTR_NUMBERFORMAT_VALUE is set...
                                LanguageType eLang = LANGUAGE_SYSTEM;
                                nFIndex = pNumFormatter->GetStandardIndex( eLang );
                            }

                            OUString aTxt( lcl_getString(*pXCell) );
                            double fTmp;
                            if (pNumFormatter->IsNumberFormat( aTxt, nFIndex, fTmp ))
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

uno::Sequence< uno::Sequence< uno::Any > > SAL_CALL SwXCellRange::getDataArray()
    throw (uno::RuntimeException)
{
    // see SwXCellRange::getData also
    // also see SwXCellRange::GetDataSequence

    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //
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
                    bool bIsNum = pBoxFmt->GetItemState( RES_BOXATR_VALUE, sal_False ) == SFX_ITEM_SET;
                    //const SfxPoolItem* pItem;
                    //SwDoc* pDoc = pXCell->GetDoc();
                    //sal_Bool bIsText = (SFX_ITEM_SET != pBoxFmt->GetAttrSet().GetItemState(RES_BOXATR_FORMAT, sal_True, &pItem)
                    //          ||  pDoc->GetNumberFormatter()->IsTextFormat(((SwTblBoxNumFormat*)pItem)->GetValue())
                    //          ||  ((SwTblBoxNumFormat*)pItem)->GetValue() == NUMBERFORMAT_TEXT);

                    if(!bIsNum/*bIsText*/)
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

void SAL_CALL SwXCellRange::setDataArray(
        const uno::Sequence< uno::Sequence< uno::Any > >& rArray )
    throw (uno::RuntimeException)
{
    // see SwXCellRange::setData also

    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
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
                            sw_setString( *pXCell, OUString(), sal_True );
                    }
                }
            }
        }
    }
}

uno::Sequence< uno::Sequence< double > > SwXCellRange::getData(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    //
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

        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            uno::Sequence< double > aColSeq(bFirstColumnAsLabel ? nColCount - 1 : nColCount);
            double * pArray = aColSeq.getArray();
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
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
                                                throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
    sal_Int16 nColCount = getColumnCount();
    if(!nRowCount || !nColCount)
    {
        uno::RuntimeException aRuntime;
        aRuntime.Message = "Table too complex";
        throw aRuntime;
    }
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt )
    {
        sal_uInt16 nRowStart = bFirstRowAsLabel ? 1 : 0;
        if(rData.getLength() < nRowCount - nRowStart)
        {
            throw uno::RuntimeException();
        }
        const uno::Sequence< double >* pRowArray = rData.getConstArray();
        for(sal_uInt16 nRow = nRowStart; nRow < nRowCount; nRow++)
        {
            const uno::Sequence< double >& rColSeq = pRowArray[nRow - nRowStart];
            sal_uInt16 nColStart = bFirstColumnAsLabel ? 1 : 0;
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

uno::Sequence< OUString > SwXCellRange::getRowDescriptions(void)
                                            throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nRowCount = getRowCount();
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
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
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
            OSL_FAIL("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXCellRange::setRowDescriptions(const uno::Sequence< OUString >& rRowDesc)
                                                    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        sal_Int16 nRowCount = getRowCount();
        if(!nRowCount || rRowDesc.getLength() < bFirstRowAsLabel ? nRowCount - 1 : nRowCount)
        {
            throw uno::RuntimeException();
        }
        const OUString* pArray = rRowDesc.getConstArray();
        if(bFirstColumnAsLabel)
        {
            sal_uInt16 nStart = bFirstRowAsLabel ? 1 : 0;
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
            OSL_FAIL("Wohin mit den Labels?");
        }
    }
}

uno::Sequence< OUString > SwXCellRange::getColumnDescriptions(void)
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
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
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
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
            OSL_FAIL("Wo kommen die Labels her?");
        }
    }
    else
        throw uno::RuntimeException();
    return aRet;
}

void SwXCellRange::setColumnDescriptions(const uno::Sequence< OUString >& ColumnDesc)
                                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    sal_Int16 nColCount = getColumnCount();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pArray = ColumnDesc.getConstArray();
        if(bFirstRowAsLabel && ColumnDesc.getLength() >= nColCount - bFirstColumnAsLabel ? 1 : 0)
        {
            sal_uInt16 nStart = bFirstColumnAsLabel ? 1 : 0;
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
            OSL_FAIL("Wo kommen die Labels her?");
        }
    }
}

void SAL_CALL SwXCellRange::addChartDataChangeEventListener(
        const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_ChartListeners.addInterface(xListener);
}

void SAL_CALL SwXCellRange::removeChartDataChangeEventListener(
        const uno::Reference<chart::XChartDataChangeEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_ChartListeners.removeInterface(xListener);
}

sal_Bool SwXCellRange::isNotANumber(double /*fNumber*/) throw( uno::RuntimeException )
{
    OSL_FAIL("not implemented");
    return sal_False;

}

double SwXCellRange::getNotANumber(void) throw( uno::RuntimeException )
{
    OSL_FAIL("not implemented");
    return 0.;
}

uno::Sequence< beans::PropertyValue > SwXCellRange::createSortDescriptor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    return SwUnoCursorHelper::CreateSortDescriptor(true);
}

void SAL_CALL SwXCellRange::sort(const uno::Sequence< beans::PropertyValue >& rDescriptor)
    throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwSortOptions aSortOpt;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt &&
        SwUnoCursorHelper::ConvertSortProperties(rDescriptor, aSortOpt))
    {
        SwUnoTableCrsr* pTableCrsr = dynamic_cast<SwUnoTableCrsr*>(pTblCrsr);
        pTableCrsr->MakeBoxSels();
        UnoActionContext aContext( pFmt->GetDoc() );
        pFmt->GetDoc()->SortTbl(pTableCrsr->GetSelectedBoxes(), aSortOpt);
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
         * is deleted in SwDoc and GPFs here when deleted again
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

/******************************************************************
 *  SwXTableRows
 ******************************************************************/
OUString SwXTableRows::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTableRows");
}

sal_Bool SwXTableRows::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTableRows::getSupportedServiceNames(void) throw( uno::RuntimeException )
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

sal_Int32 SwXTableRows::getCount(void) throw( uno::RuntimeException )
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

uno::Any SwXTableRows::getByIndex(sal_Int32 nIndex)
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt || nIndex < 0 )
        throw lang::IndexOutOfBoundsException();
    else
    {
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if( (sal_uInt16)pTable->GetTabLines().size() > nIndex)
        {
            SwTableLine* pLine = pTable->GetTabLines()[(sal_uInt16)nIndex];
            SwIterator<SwXTextTableRow,SwFmt> aIter( *pFrmFmt );
            SwXTextTableRow* pXRow = aIter.First();
            while( pXRow )
            {
                // gibt es eine passende Zelle bereits?
                if(pXRow->GetTblRow() == pLine)
                    break;
                pXRow = aIter.Next();
            }
            //sonst anlegen
            if(!pXRow)
                pXRow = new SwXTextTableRow(pFrmFmt, pLine);
            uno::Reference< beans::XPropertySet >  xRet =
                                    (beans::XPropertySet*)pXRow;
            aRet.setValue(&xRet, ::getCppuType((const uno::Reference<beans::XPropertySet>*)0));
        }
        else
            throw lang::IndexOutOfBoundsException();
    }
    return aRet;
}

uno::Type SAL_CALL SwXTableRows::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((const uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SwXTableRows::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    //es gibt keine Tabelle ohne Zeilen
    return sal_True;
}

void SwXTableRows::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
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
            sal_uInt16 nRowCount = pTable->GetTabLines().size();
            if (nCount <= 0 || !(0 <= nIndex && nIndex <= nRowCount))
            {
                uno::RuntimeException aExcept;
                aExcept.Message = "Illegal arguments";
                throw aExcept;
            }

            String sTLName = sw_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            bool bAppend = false;
            if(!pTLBox)
            {
                bAppend = true;
                // am Ende anfuegen, dazu muss der Cursor in die letzte Zeile!
                SwTableLines& rLines = pTable->GetTabLines();
                SwTableLine* pLine = rLines.back();
                SwTableBoxes& rBoxes = pLine->GetTabBoxes();
                pTLBox = rBoxes.front();
            }
            if(pTLBox)
            {
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
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

void SwXTableRows::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
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
            String sTLName = sw_GetCellName(0, nIndex);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                {
                    // hier muessen die Actions aufgehoben werden
                    UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                }
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                String sBLName = sw_GetCellName(0, nIndex + nCount - 1);
                const SwTableBox* pBLBox = pTable->GetTblBox( sBLName );
                if(pBLBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pBLBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr =
                        dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();
                    {   // Die Klammer ist wichtig
                        UnoActionContext aAction(pFrmFmt->GetDoc());
                        pFrmFmt->GetDoc()->DeleteRow(*pUnoCrsr);
                        delete pUnoCrsr;
                        bSuccess = true;
                    }
                    {
                        // hier muessen die Actions aufgehoben werden
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

/******************************************************************
 * SwXTableColumns
 ******************************************************************/
OUString SwXTableColumns::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTableColumns");
}

sal_Bool SwXTableColumns::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTableColumns::getSupportedServiceNames(void) throw( uno::RuntimeException )
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

sal_Int32 SwXTableColumns::getCount(void) throw( uno::RuntimeException )
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
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< uno::XInterface >  xRet;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    else
    {
        sal_uInt16 nCount = 0;
        SwTable* pTable = SwTable::FindTable( pFrmFmt );
        if(!pTable->IsTblComplex())
        {
            SwTableLines& rLines = pTable->GetTabLines();
            SwTableLine* pLine = rLines.front();
            nCount = pLine->GetTabBoxes().size();
        }
        if(nCount <= nIndex || nIndex < 0)
            throw lang::IndexOutOfBoundsException();
        xRet = uno::Reference<uno::XInterface>();   //!! writer tables do not have columns !!
    }
    return uno::Any(&xRet, ::getCppuType((const uno::Reference<uno::XInterface>*)0));
}

uno::Type SAL_CALL SwXTableColumns::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType((uno::Reference<uno::XInterface>*)0);
}

sal_Bool SwXTableColumns::hasElements(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFrmFmt = GetFrmFmt();
    if(!pFrmFmt)
        throw uno::RuntimeException();
    return sal_True;
}

void SwXTableColumns::insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
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
            sal_uInt16 nColCount = pLine->GetTabBoxes().size();
            if (nCount <= 0 || !(0 <= nIndex && nIndex <= nColCount))
            {
                uno::RuntimeException aExcept;
                aExcept.Message = "Illegal arguments";
                throw aExcept;
            }

            String sTLName = sw_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            bool bAppend = false;
            if(!pTLBox)
            {
                bAppend = true;
                // am Ende anfuegen, dazu muss der Cursor in die letzte Spalte!
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

void SwXTableColumns::removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( uno::RuntimeException )
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
            String sTLName = sw_GetCellName(nIndex, 0);
            const SwTableBox* pTLBox = pTable->GetTblBox( sTLName );
            if(pTLBox)
            {
                {
                    // hier muessen die Actions aufgehoben werden
                    UnoActionRemoveContext aRemoveContext(pFrmFmt->GetDoc());
                }
                const SwStartNode* pSttNd = pTLBox->GetSttNd();
                SwPosition aPos(*pSttNd);
                // Cursor in die obere linke Zelle des Ranges setzen
                SwUnoCrsr* pUnoCrsr = pFrmFmt->GetDoc()->CreateUnoCrsr(aPos, true);
                pUnoCrsr->Move( fnMoveForward, fnGoNode );
                pUnoCrsr->SetRemainInSection( sal_False );
                String sTRName = sw_GetCellName(nIndex + nCount - 1, 0);
                const SwTableBox* pTRBox = pTable->GetTblBox( sTRName );
                if(pTRBox)
                {
                    pUnoCrsr->SetMark();
                    pUnoCrsr->GetPoint()->nNode = *pTRBox->GetSttNd();
                    pUnoCrsr->Move( fnMoveForward, fnGoNode );
                    SwUnoTableCrsr* pCrsr =
                        dynamic_cast<SwUnoTableCrsr*>(pUnoCrsr);
                    pCrsr->MakeBoxSels();
                    {   // Die Klammer ist wichtig
                        UnoActionContext aAction(pFrmFmt->GetDoc());
                        pFrmFmt->GetDoc()->DeleteCol(*pUnoCrsr);
                        delete pUnoCrsr;
                        bSuccess = true;
                    }
                    {
                        // hier muessen die Actions aufgehoben werden
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
