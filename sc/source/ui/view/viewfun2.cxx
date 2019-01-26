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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svl/zforlist.hxx>
#include <svx/srchdlg.hxx>
#include <svx/svdview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/waitobj.hxx>

#include <basic/sbstar.hxx>
#include <com/sun/star/container/XNameContainer.hpp>

#include <viewfunc.hxx>

#include <sc.hrc>
#include <globstr.hrc>
#include <scresid.hxx>

#include <attrib.hxx>
#include <autoform.hxx>
#include <formulacell.hxx>
#include <cellmergeoption.hxx>
#include <compiler.hxx>
#include <docfunc.hxx>
#include <docpool.hxx>
#include <docsh.hxx>
#include <docoptio.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <printfun.hxx>
#include <rangenam.hxx>
#include <rangeutl.hxx>
#include <refundo.hxx>
#include <table.hxx>
#include <tablink.hxx>
#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <undoblk.hxx>
#include <undocell.hxx>
#include <undotab.hxx>
#include <sizedev.hxx>
#include <editable.hxx>
#include <scmod.hxx>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <funcdesc.hxx>
#include <docuno.hxx>
#include <charthelper.hxx>
#include <tabbgcolor.hxx>
#include <clipparam.hxx>
#include <prnsave.hxx>
#include <searchresults.hxx>
#include <tokenarray.hxx>
#include <rowheightcontext.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <comphelper/lok.hxx>
#include <mergecellsdialog.hxx>
#include <sheetevents.hxx>
#include <columnspanset.hxx>

#include <vector>
#include <memory>
#include <boost/property_tree/json_parser.hpp>

#include <officecfg/Office/Calc.hxx>

using namespace com::sun::star;
using ::editeng::SvxBorderLine;

using ::std::vector;
using ::std::unique_ptr;

bool ScViewFunc::AdjustBlockHeight( bool bPaint, ScMarkData* pMarkData )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    if (!pMarkData)
        pMarkData = &GetViewData().GetMarkData();

    ScDocument& rDoc = pDocSh->GetDocument();
    std::vector<sc::ColRowSpan> aMarkedRows = pMarkData->GetMarkedRowSpans();

    if (aMarkedRows.empty())
    {
        SCROW nCurRow = GetViewData().GetCurY();
        aMarkedRows.emplace_back(nCurRow, nCurRow);
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        SCCOLROW nStart = aMarkedRows[0].mnStart;
        OnLOKSetWidthOrHeight(nStart, /*width: */ false);
    }

    double nPPTX = GetViewData().GetPPTX();
    double nPPTY = GetViewData().GetPPTY();
    Fraction aZoomX = GetViewData().GetZoomX();
    Fraction aZoomY = GetViewData().GetZoomY();

    ScSizeDeviceProvider aProv(pDocSh);
    if (aProv.IsPrinter())
    {
        nPPTX = aProv.GetPPTX();
        nPPTY = aProv.GetPPTY();
        aZoomX = aZoomY = Fraction( 1, 1 );
    }

    sc::RowHeightContext aCxt(nPPTX, nPPTY, aZoomX, aZoomY, aProv.GetDevice());
    bool bAnyChanged = false;
    for (const SCTAB& nTab : *pMarkData)
    {
        bool bChanged = false;
        SCROW nPaintY = 0;
        for (const auto& rRow : aMarkedRows)
        {
            SCROW nStartNo = rRow.mnStart;
            SCROW nEndNo = rRow.mnEnd;
            ScAddress aTopLeft(0, nStartNo, nTab);
            rDoc.UpdateScriptTypes(aTopLeft, MAXCOLCOUNT, nEndNo-nStartNo+1);
            if (rDoc.SetOptimalHeight(aCxt, nStartNo, nEndNo, nTab))
            {
                if (!bChanged)
                    nPaintY = nStartNo;
                bAnyChanged = bChanged = true;
            }
        }
        // tdf#76183: recalculate objects' positions
        if (bChanged)
            rDoc.SetDrawPageSize(nTab);
        if ( bPaint && bChanged )
            pDocSh->PostPaint( 0, nPaintY, nTab, MAXCOL, MAXROW, nTab,
                                                PaintPartFlags::Grid | PaintPartFlags::Left );
    }

    if ( bPaint && bAnyChanged )
        pDocSh->UpdateOle(&GetViewData());

    if (comphelper::LibreOfficeKit::isActive())
        ScTabViewShell::notifyAllViewsHeaderInvalidation(ROW_HEADER, GetViewData().GetTabNo());

    return bAnyChanged;
}

bool ScViewFunc::AdjustRowHeight( SCROW nStartRow, SCROW nEndRow )
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        OnLOKSetWidthOrHeight(nStartRow, /*width: */ false);
    }

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();
    double nPPTX = GetViewData().GetPPTX();
    double nPPTY = GetViewData().GetPPTY();
    Fraction aZoomX = GetViewData().GetZoomX();
    Fraction aZoomY = GetViewData().GetZoomY();
    sal_uInt16 nOldPixel = 0;
    if (nStartRow == nEndRow)
        nOldPixel = static_cast<sal_uInt16>(rDoc.GetRowHeight(nStartRow,nTab) * nPPTY);

    ScSizeDeviceProvider aProv(pDocSh);
    if (aProv.IsPrinter())
    {
        nPPTX = aProv.GetPPTX();
        nPPTY = aProv.GetPPTY();
        aZoomX = aZoomY = Fraction( 1, 1 );
    }
    sc::RowHeightContext aCxt(nPPTX, nPPTY, aZoomX, aZoomY, aProv.GetDevice());
    bool bChanged = rDoc.SetOptimalHeight(aCxt, nStartRow, nEndRow, nTab);

    // tdf#76183: recalculate objects' positions
    if (bChanged)
        rDoc.SetDrawPageSize(nTab);

    if (bChanged && ( nStartRow == nEndRow ))
    {
        sal_uInt16 nNewPixel = static_cast<sal_uInt16>(rDoc.GetRowHeight(nStartRow,nTab) * nPPTY);
        if ( nNewPixel == nOldPixel )
            bChanged = false;
    }

    if ( bChanged )
        pDocSh->PostPaint( 0, nStartRow, nTab, MAXCOL, MAXROW, nTab,
                                            PaintPartFlags::Grid | PaintPartFlags::Left );

    if (comphelper::LibreOfficeKit::isActive())
        ScTabViewShell::notifyAllViewsHeaderInvalidation(ROW_HEADER, GetViewData().GetTabNo());

    return bChanged;
}

enum ScAutoSum
{
    ScAutoSumNone = 0,
    ScAutoSumData,
    ScAutoSumSum
};

static ScAutoSum lcl_IsAutoSumData( ScDocument* pDoc, SCCOL nCol, SCROW nRow,
        SCTAB nTab, ScDirection eDir, SCCOLROW& nExtend )
{
    ScRefCellValue aCell(*pDoc, ScAddress(nCol, nRow, nTab));
    if (aCell.hasNumeric())
    {
        if (aCell.meType == CELLTYPE_FORMULA)
        {
            ScTokenArray* pCode = aCell.mpFormula->GetCode();
            if ( pCode && pCode->GetOuterFuncOpCode() == ocSum )
            {
                if ( pCode->GetAdjacentExtendOfOuterFuncRefs( nExtend,
                        ScAddress( nCol, nRow, nTab ), eDir ) )
                    return ScAutoSumSum;
            }
        }
        return ScAutoSumData;
    }
    return ScAutoSumNone;
}

#define SC_AUTOSUM_MAXCOUNT     20

static ScAutoSum lcl_SeekAutoSumData( ScDocument* pDoc, SCCOL& nCol, SCROW& nRow,
        SCTAB nTab, ScDirection eDir, SCCOLROW& nExtend )
{
    sal_uInt16 nCount = 0;
    while (nCount < SC_AUTOSUM_MAXCOUNT)
    {
        if ( eDir == DIR_TOP )
        {
            if (nRow > 0)
                --nRow;
            else
                return ScAutoSumNone;
        }
        else
        {
            if (nCol > 0)
                --nCol;
            else
                return ScAutoSumNone;
        }
        ScAutoSum eSum;
        if ( (eSum = lcl_IsAutoSumData(
                pDoc, nCol, nRow, nTab, eDir, nExtend )) != ScAutoSumNone )
            return eSum;
        ++nCount;
    }
    return ScAutoSumNone;
}

#undef SC_AUTOSUM_MAXCOUNT

static bool lcl_FindNextSumEntryInColumn( ScDocument* pDoc, SCCOL nCol, SCROW& nRow,
                                   SCTAB nTab, SCCOLROW& nExtend, SCROW nMinRow )
{
    const SCROW nTmp = nRow;
    ScAutoSum eSkip = ScAutoSumNone;
    while ( ( eSkip = lcl_IsAutoSumData( pDoc, nCol, nRow, nTab, DIR_TOP, nExtend ) ) == ScAutoSumData &&
            nRow > nMinRow )
    {
        --nRow;
    }
    return eSkip == ScAutoSumSum && nRow < nTmp;
}

static bool lcl_FindNextSumEntryInRow( ScDocument* pDoc, SCCOL& nCol, SCROW nRow,
                                SCTAB nTab, SCCOLROW& nExtend, SCROW nMinCol )
{
    const SCCOL nTmp = nCol;
    ScAutoSum eSkip = ScAutoSumNone;
    while ( ( eSkip = lcl_IsAutoSumData( pDoc, nCol, nRow, nTab, DIR_LEFT, nExtend ) ) == ScAutoSumData &&
            nCol > nMinCol )
    {
        --nCol;
    }
    return eSkip == ScAutoSumSum && nCol < nTmp;
}

static ScAutoSum lcl_GetAutoSumForColumnRange( ScDocument* pDoc, ScRangeList& rRangeList, const ScRange& rRange )
{
    const ScAddress aStart = rRange.aStart;
    const ScAddress aEnd = rRange.aEnd;
    if ( aStart.Col() != aEnd.Col() )
    {
        return ScAutoSumNone;
    }

    const SCTAB nTab = aEnd.Tab();
    const SCCOL nCol = aEnd.Col();
    SCROW nEndRow = aEnd.Row();
    SCROW nStartRow = nEndRow;
    SCCOLROW nExtend = 0;
    ScAutoSum eSum = lcl_IsAutoSumData( pDoc, nCol, nEndRow, nTab, DIR_TOP, nExtend /*out*/ );

    if ( eSum == ScAutoSumSum )
    {
        bool bContinue = false;
        do
        {
            rRangeList.push_back( ScRange( nCol, nStartRow, nTab, nCol, nEndRow, nTab ) );
            nEndRow = static_cast< SCROW >( nExtend );
            bContinue = lcl_FindNextSumEntryInColumn( pDoc, nCol, nEndRow /*inout*/, nTab, nExtend /*out*/, aStart.Row() );
            if ( bContinue )
            {
                nStartRow = nEndRow;
            }
        } while ( bContinue );
    }
    else
    {
        while ( nStartRow > aStart.Row() &&
                (eSum = lcl_IsAutoSumData( pDoc, nCol, nStartRow-1, nTab, DIR_TOP, nExtend /*out*/ )) != ScAutoSumSum )
        {
            --nStartRow;
        }
        rRangeList.push_back( ScRange( nCol, nStartRow, nTab, nCol, nEndRow, nTab ) );
        if (eSum == ScAutoSumNone)
            eSum = ScAutoSumData;
    }

    return eSum;
}

static ScAutoSum lcl_GetAutoSumForRowRange( ScDocument* pDoc, ScRangeList& rRangeList, const ScRange& rRange )
{
    const ScAddress aStart = rRange.aStart;
    const ScAddress aEnd = rRange.aEnd;
    if ( aStart.Row() != aEnd.Row() )
    {
        return ScAutoSumNone;
    }

    const SCTAB nTab = aEnd.Tab();
    const SCROW nRow = aEnd.Row();
    SCCOL nEndCol = aEnd.Col();
    SCCOL nStartCol = nEndCol;
    SCCOLROW nExtend = 0;
    ScAutoSum eSum = lcl_IsAutoSumData( pDoc, nEndCol, nRow, nTab, DIR_LEFT, nExtend /*out*/ );

    if ( eSum == ScAutoSumSum )
    {
        bool bContinue = false;
        do
        {
            rRangeList.push_back( ScRange( nStartCol, nRow, nTab, nEndCol, nRow, nTab ) );
            nEndCol = static_cast< SCCOL >( nExtend );
            bContinue = lcl_FindNextSumEntryInRow( pDoc, nEndCol /*inout*/, nRow, nTab, nExtend /*out*/, aStart.Col() );
            if ( bContinue )
            {
                nStartCol = nEndCol;
            }
        } while ( bContinue );
    }
    else
    {
        while ( nStartCol > aStart.Col() &&
                (eSum = lcl_IsAutoSumData( pDoc, nStartCol-1, nRow, nTab, DIR_LEFT, nExtend /*out*/ )) != ScAutoSumSum )
        {
            --nStartCol;
        }
        rRangeList.push_back( ScRange( nStartCol, nRow, nTab, nEndCol, nRow, nTab ) );
        if (eSum == ScAutoSumNone)
            eSum = ScAutoSumData;
    }

    return eSum;
}

bool ScViewFunc::GetAutoSumArea( ScRangeList& rRangeList )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();

    SCCOL nCol = GetViewData().GetCurX();
    SCROW nRow = GetViewData().GetCurY();

    SCCOL nStartCol = nCol;
    SCROW nStartRow = nRow;
    SCCOL nEndCol    = nCol;
    SCROW nEndRow    = nRow;
    SCCOL nSeekCol   = nCol;
    SCROW nSeekRow   = nRow;
    SCCOLROW nExtend;       // will become valid via reference for ScAutoSumSum

    bool bCol = false;
    bool bRow = false;

    ScAutoSum eSum;
    if ( nRow != 0
            && ((eSum = lcl_IsAutoSumData( pDoc, nCol, nRow-1, nTab,
                DIR_TOP, nExtend /*out*/ )) == ScAutoSumData )
            && ((eSum = lcl_IsAutoSumData( pDoc, nCol, nRow-1, nTab,
                DIR_LEFT, nExtend /*out*/ )) == ScAutoSumData )
        )
    {
        bRow = true;
        nSeekRow = nRow - 1;
    }
    else if ( nCol != 0 && (eSum = lcl_IsAutoSumData( pDoc, nCol-1, nRow, nTab,
            DIR_LEFT, nExtend /*out*/ )) == ScAutoSumData )
    {
        bCol = true;
        nSeekCol = nCol - 1;
    }
    else if ( (eSum = lcl_SeekAutoSumData( pDoc, nCol, nSeekRow, nTab, DIR_TOP, nExtend /*out*/ )) != ScAutoSumNone )
        bRow = true;
    else if (( eSum = lcl_SeekAutoSumData( pDoc, nSeekCol, nRow, nTab, DIR_LEFT, nExtend /*out*/ )) != ScAutoSumNone )
        bCol = true;

    if ( bCol || bRow )
    {
        if ( bRow )
        {
            nStartRow = nSeekRow;       // nSeekRow might be adjusted via reference
            if ( eSum == ScAutoSumSum )
                nEndRow = nStartRow;        // only sum sums
            else
                nEndRow = nRow - 1;     // maybe extend data area at bottom
        }
        else
        {
            nStartCol = nSeekCol;       // nSeekCol might be adjusted vie reference
            if ( eSum == ScAutoSumSum )
                nEndCol = nStartCol;        // only sum sums
            else
                nEndCol = nCol - 1;     // maybe extend data area to the right
        }
        bool bContinue = false;
        do
        {
            if ( eSum == ScAutoSumData )
            {
                if ( bRow )
                {
                    while ( nStartRow != 0 && lcl_IsAutoSumData(    pDoc, nCol,
                            nStartRow-1, nTab, DIR_TOP, nExtend /*out*/ ) == eSum )
                        --nStartRow;
                }
                else
                {
                    while ( nStartCol != 0 && lcl_IsAutoSumData( pDoc, nStartCol-1,
                            nRow, nTab, DIR_LEFT, nExtend /*out*/ ) == eSum )
                        --nStartCol;
                }
            }
            rRangeList.push_back(
                ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab ) );
            if ( eSum == ScAutoSumSum )
            {
                if ( bRow )
                {
                    nEndRow = static_cast< SCROW >( nExtend );
                    bContinue = lcl_FindNextSumEntryInColumn( pDoc, nCol, nEndRow /*inout*/, nTab, nExtend /*out*/, 0 );
                    if ( bContinue )
                    {
                        nStartRow = nEndRow;
                    }
                }
                else
                {
                    nEndCol = static_cast< SCCOL >( nExtend );
                    bContinue = lcl_FindNextSumEntryInRow( pDoc, nEndCol /*inout*/, nRow, nTab, nExtend /*out*/, 0 );
                    if ( bContinue )
                    {
                        nStartCol = nEndCol;
                    }
                }
            }
        } while ( bContinue );
        return true;
    }
    return false;
}

void ScViewFunc::EnterAutoSum(const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr)
{
    OUString aFormula = GetAutoSumFormula( rRangeList, bSubTotal, rAddr );
    EnterBlock( aFormula, nullptr );
}

bool ScViewFunc::AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    const SCTAB nTab = rRange.aStart.Tab();
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    const SCCOL nEndCol = rRange.aEnd.Col();
    const SCROW nEndRow = rRange.aEnd.Row();
    SCCOLROW nExtend = 0; // out parameter for lcl_IsAutoSumData

    // ignore rows at the top of the given range which don't contain autosum data
    bool bRowData = false;
    for ( SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow )
    {
        for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
        {
            if ( lcl_IsAutoSumData( pDoc, nCol, nRow, nTab, DIR_TOP, nExtend ) != ScAutoSumNone )
            {
                bRowData = true;
                break;
            }
        }
        if ( bRowData )
        {
            nStartRow = nRow;
            break;
        }
    }
    if ( !bRowData )
    {
        return false;
    }

    // ignore columns at the left of the given range which don't contain autosum data
    bool bColData = false;
    for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
    {
        for ( SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow )
        {
            if ( lcl_IsAutoSumData( pDoc, nCol, nRow, nTab, DIR_LEFT, nExtend ) != ScAutoSumNone )
            {
                bColData = true;
                break;
            }
        }
        if ( bColData )
        {
            nStartCol = nCol;
            break;
        }
    }
    if ( !bColData )
    {
        return false;
    }

    const bool bEndRowEmpty = pDoc->IsBlockEmpty( nTab, nStartCol, nEndRow, nEndCol, nEndRow );
    const bool bEndColEmpty = pDoc->IsBlockEmpty( nTab, nEndCol, nStartRow, nEndCol, nEndRow );
    bool bRow = ( nStartRow != nEndRow ) && ( bEndRowEmpty || !bEndColEmpty );
    bool bCol = ( nStartCol != nEndCol ) && ( bEndColEmpty || nStartRow == nEndRow );

    // find an empty row for entering the result
    SCROW nInsRow = nEndRow;
    if ( bRow && !bEndRowEmpty )
    {
        if ( nInsRow < MAXROW )
        {
            ++nInsRow;
            while ( !pDoc->IsBlockEmpty( nTab, nStartCol, nInsRow, nEndCol, nInsRow ) )
            {
                if ( nInsRow < MAXROW )
                {
                    ++nInsRow;
                }
                else
                {
                    bRow = false;
                    break;
                }
            }
        }
        else
        {
            bRow = false;
        }
    }

    // find an empty column for entering the result
    SCCOL nInsCol = nEndCol;
    if ( bCol && !bEndColEmpty )
    {
        if ( nInsCol < MAXCOL )
        {
            ++nInsCol;
            while ( !pDoc->IsBlockEmpty( nTab, nInsCol, nStartRow, nInsCol, nEndRow ) )
            {
                if ( nInsCol < MAXCOL )
                {
                    ++nInsCol;
                }
                else
                {
                    bCol = false;
                    break;
                }
            }
        }
        else
        {
            bCol = false;
        }
    }

    if ( !bRow && !bCol )
    {
        return false;
    }

    SCCOL nMarkEndCol = nEndCol;
    SCROW nMarkEndRow = nEndRow;
    ScAutoSum eSum = ScAutoSumNone;
    SCROW nColSums = 0;
    SCCOL nRowSums = 0;
    SCROW nColSumsStartRow = 0;
    SCCOL nRowSumsStartCol = 0;

    if ( bRow )
    {
        // calculate the row sums for all columns of the given range

        SCROW nSumEndRow = nEndRow;

        if ( bEndRowEmpty )
        {
            // the last row of the given range is empty;
            // don't take into account for calculating the autosum
            --nSumEndRow;
        }
        else
        {
            // increase mark range
            ++nMarkEndRow;
        }

        for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
        {
            if ( !pDoc->IsBlockEmpty( nTab, nCol, nStartRow, nCol, nSumEndRow ) )
            {
                ScRangeList aRangeList;
                // Include the originally selected start row.
                const ScRange aRange( nCol, rRange.aStart.Row(), nTab, nCol, nSumEndRow, nTab );
                if ( (eSum = lcl_GetAutoSumForColumnRange( pDoc, aRangeList, aRange )) != ScAutoSumNone )
                {
                    if (++nRowSums == 1)
                        nRowSumsStartCol = aRangeList[0].aStart.Col();
                    const OUString aFormula = GetAutoSumFormula(
                        aRangeList, bSubTotal, ScAddress(nCol, nInsRow, nTab));
                    EnterData( nCol, nInsRow, nTab, aFormula );
                }
            }
        }
    }

    if ( bCol )
    {
        // calculate the column sums for all rows of the given range

        SCCOL nSumEndCol = nEndCol;

        if ( bEndColEmpty )
        {
            // the last column of the given range is empty;
            // don't take into account for calculating the autosum
            --nSumEndCol;
        }
        else
        {
            // increase mark range
            ++nMarkEndCol;
        }

        for ( SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow )
        {
            if ( !pDoc->IsBlockEmpty( nTab, nStartCol, nRow, nSumEndCol, nRow ) )
            {
                ScRangeList aRangeList;
                // Include the originally selected start column.
                const ScRange aRange( rRange.aStart.Col(), nRow, nTab, nSumEndCol, nRow, nTab );
                if ( (eSum = lcl_GetAutoSumForRowRange( pDoc, aRangeList, aRange )) != ScAutoSumNone )
                {
                    if (++nColSums == 1)
                        nColSumsStartRow = aRangeList[0].aStart.Row();
                    const OUString aFormula = GetAutoSumFormula( aRangeList, bSubTotal, ScAddress(nInsCol, nRow, nTab) );
                    EnterData( nInsCol, nRow, nTab, aFormula );
                }
            }
        }
    }

    // Set new mark range and cursor position.
    // For sum of sums (and data until sum) mark the actual resulting range if
    // there is only one, or the data range if more than one. Otherwise use the
    // original selection. All extended by end column/row where the sum is put.
    const ScRange aMarkRange(
            (eSum == ScAutoSumSum ?
             (nRowSums == 1 ? nRowSumsStartCol : nStartCol) :
             rRange.aStart.Col()),
            (eSum == ScAutoSumSum ?
             (nColSums == 1 ? nColSumsStartRow : nStartRow) :
             rRange.aStart.Row()),
            nTab, nMarkEndCol, nMarkEndRow, nTab );
    MarkRange( aMarkRange, false, bContinue );
    if ( bSetCursor )
    {
        SetCursor( nMarkEndCol, nMarkEndRow );
    }

    return true;
}

OUString ScViewFunc::GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr )
{
    ScViewData& rViewData = GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    std::unique_ptr<ScTokenArray> pArray(new ScTokenArray);

    pArray->AddOpCode(bSubTotal ? ocSubTotal : ocSum);
    pArray->AddOpCode(ocOpen);

    if (bSubTotal)
    {
        pArray->AddDouble(9);
        pArray->AddOpCode(ocSep);
    }

    if(!rRangeList.empty())
    {
        ScRangeList aRangeList = rRangeList;
        size_t ListSize = aRangeList.size();
        for ( size_t i = 0; i < ListSize; ++i )
        {
            const ScRange & r = aRangeList[i];
            if (i != 0)
                pArray->AddOpCode(ocSep);
            ScComplexRefData aRef;
            aRef.InitRangeRel(r, rAddr);
            pArray->AddDoubleReference(aRef);
        }
    }

    pArray->AddOpCode(ocClose);

    ScCompiler aComp(pDoc, rAddr, *pArray, pDoc->GetGrammar());
    OUStringBuffer aBuf;
    aComp.CreateStringFromTokenArray(aBuf);
    OUString aFormula = aBuf.makeStringAndClear();
    aBuf.append('=');
    aBuf.append(aFormula);
    return aBuf.makeStringAndClear();
}

void ScViewFunc::EnterBlock( const OUString& rString, const EditTextObject* pData )
{
    //  test for multi selection

    SCCOL nCol = GetViewData().GetCurX();
    SCROW nRow = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScMarkData& rMark = GetViewData().GetMarkData();
    if ( rMark.IsMultiMarked() )
    {
        rMark.MarkToSimple();
        if ( rMark.IsMultiMarked() )
        {       // "Insert into multi selection not possible"
            ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);

            //  insert into single cell
            if ( pData )
                EnterData(nCol, nRow, nTab, *pData);
            else
                EnterData( nCol, nRow, nTab, rString );
            return;
        }
    }

    if (GetViewData().SelectionForbidsCellFill())
    {
        PaintArea(nCol, nRow, nCol, nRow);        // possibly the edit-engine is still painted there
        return;
    }

    ScDocument* pDoc = GetViewData().GetDocument();
    OUString aNewStr = rString;
    if ( pData )
    {
        const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
        ScTabEditEngine aEngine( *pOldPattern, pDoc->GetEnginePool(), pDoc );
        aEngine.SetText(*pData);

        ScEditAttrTester aTester( &aEngine );
        if (!aTester.NeedsObject())
        {
            aNewStr = aEngine.GetText();
            pData = nullptr;
        }
    }

    //  Insert via PasteFromClip

    WaitObject aWait( GetFrameWin() );

    ScAddress aPos( nCol, nRow, nTab );

    ScDocumentUniquePtr pInsDoc(new ScDocument( SCDOCMODE_CLIP ));
    pInsDoc->ResetClip( pDoc, nTab );

    if (aNewStr[0] == '=')                      // Formula ?
    {
        //  SetString not possible, because in Clipboard-Documents nothing will be compiled!
        pInsDoc->SetFormulaCell(aPos, new ScFormulaCell(pDoc, aPos, aNewStr));
    }
    else if ( pData )
    {
        // A copy of pData will be stored.
        pInsDoc->SetEditText(aPos, *pData, pDoc->GetEditPool());
    }
    else
        pInsDoc->SetString( nCol, nRow, nTab, aNewStr );

    pInsDoc->SetClipArea( ScRange(aPos) );
    // insert Block, with Undo etc.
    if ( PasteFromClip( InsertDeleteFlags::CONTENTS, pInsDoc.get(), ScPasteFunc::NONE, false, false,
            false, INS_NONE, InsertDeleteFlags::ATTRIB ) )
    {
        const SfxUInt32Item* pItem = pInsDoc->GetAttr(
            nCol, nRow, nTab, ATTR_VALUE_FORMAT );
        if ( pItem )
        {   // set number format if incompatible
            // MarkData was already MarkToSimple'ed in PasteFromClip
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            std::unique_ptr<ScPatternAttr> pPattern(new ScPatternAttr( pDoc->GetPool() ));
            pPattern->GetItemSet().Put( *pItem );
            SvNumFormatType nNewType = pDoc->GetFormatTable()->GetType( pItem->GetValue() );
            pDoc->ApplyPatternIfNumberformatIncompatible( aRange, rMark,
                *pPattern, nNewType );
        }
    }
}

//  manual page break

void ScViewFunc::InsertPageBreak( bool bColumn, bool bRecord, const ScAddress* pPos,
                                    bool bSetModified )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScAddress aCursor;
    if (pPos)
        aCursor = *pPos;
    else
        aCursor = ScAddress( GetViewData().GetCurX(), GetViewData().GetCurY(), nTab );

    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().
                        InsertPageBreak( bColumn, aCursor, bRecord, bSetModified );

    if ( bSuccess && bSetModified )
        UpdatePageBreakData( true );    // for PageBreak-Mode
}

void ScViewFunc::DeletePageBreak( bool bColumn, bool bRecord, const ScAddress* pPos,
                                    bool bSetModified )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScAddress aCursor;
    if (pPos)
        aCursor = *pPos;
    else
        aCursor = ScAddress( GetViewData().GetCurX(), GetViewData().GetCurY(), nTab );

    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().
                        RemovePageBreak( bColumn, aCursor, bRecord, bSetModified );

    if ( bSuccess && bSetModified )
        UpdatePageBreakData( true );    // for PageBreak-Mode
}

void ScViewFunc::RemoveManualBreaks()
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData().GetTabNo();
    bool bUndo(rDoc.IsUndoEnabled());

    if (bUndo)
    {
        ScDocumentUniquePtr pUndoDoc(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nTab, nTab, true, true );
        rDoc.CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, InsertDeleteFlags::NONE, false, *pUndoDoc );
        pDocSh->GetUndoManager()->AddUndoAction(
                                std::make_unique<ScUndoRemoveBreaks>( pDocSh, nTab, std::move(pUndoDoc) ) );
    }

    rDoc.RemoveManualBreaks(nTab);
    rDoc.UpdatePageBreaks(nTab);

    UpdatePageBreakData( true );
    pDocSh->SetDocumentModified();
    pDocSh->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PaintPartFlags::Grid );
}

void ScViewFunc::SetPrintZoom(sal_uInt16 nScale)
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    SCTAB nTab = GetViewData().GetTabNo();
    pDocSh->SetPrintZoom( nTab, nScale, 0/*nPages*/ );
}

void ScViewFunc::AdjustPrintZoom()
{
    ScRange aRange;
    if ( GetViewData().GetSimpleArea( aRange ) != SC_MARK_SIMPLE )
        GetViewData().GetMarkData().GetMultiMarkArea( aRange );
    GetViewData().GetDocShell()->AdjustPrintZoom( aRange );
}

void ScViewFunc::SetPrintRanges( bool bEntireSheet, const OUString* pPrint,
                                const OUString* pRepCol, const OUString* pRepRow,
                                bool bAddPrint )
{
    //  on all selected tables

    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    ScDocument& rDoc    = pDocSh->GetDocument();
    ScMarkData& rMark   = GetViewData().GetMarkData();
    bool bUndo (rDoc.IsUndoEnabled());

    std::unique_ptr<ScPrintRangeSaver> pOldRanges = rDoc.CreatePrintRangeSaver();

    ScAddress::Details aDetails(rDoc.GetAddressConvention(), 0, 0);

    for (const SCTAB& nTab : rMark)
    {
        ScRange aRange( 0,0,nTab );

        //  print ranges

        if( !bAddPrint )
            rDoc.ClearPrintRanges( nTab );

        if( bEntireSheet )
        {
            rDoc.SetPrintEntireSheet( nTab );
        }
        else if ( pPrint )
        {
            if ( !pPrint->isEmpty() )
            {
                const sal_Unicode sep = ScCompiler::GetNativeSymbolChar(ocSep);
                sal_Int32 nPos = 0;
                do
                {
                    const OUString aToken = pPrint->getToken(0, sep, nPos);
                    if ( aRange.ParseAny( aToken, &rDoc, aDetails ) & ScRefFlags::VALID )
                        rDoc.AddPrintRange( nTab, aRange );
                }
                while (nPos >= 0);
            }
        }
        else    // NULL = use selection (print range is always set), use empty string to delete all ranges
        {
            if ( GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
            {
                rDoc.AddPrintRange( nTab, aRange );
            }
            else if ( rMark.IsMultiMarked() )
            {
                rMark.MarkToMulti();
                ScRangeListRef pList( new ScRangeList );
                rMark.FillRangeListWithMarks( pList.get(), false );
                for (size_t i = 0, n = pList->size(); i < n; ++i)
                {
                    const ScRange & rR = (*pList)[i];
                    rDoc.AddPrintRange(nTab, rR);
                }
            }
        }

        //  repeat columns

        if ( pRepCol )
        {
            if ( pRepCol->isEmpty() )
                rDoc.SetRepeatColRange( nTab, nullptr );
            else
                if ( aRange.ParseAny( *pRepCol, &rDoc, aDetails ) & ScRefFlags::VALID )
                    rDoc.SetRepeatColRange( nTab, std::unique_ptr<ScRange>(new ScRange(aRange)) );
        }

        //  repeat rows

        if ( pRepRow )
        {
            if ( pRepRow->isEmpty() )
                rDoc.SetRepeatRowRange( nTab, nullptr );
            else
                if ( aRange.ParseAny( *pRepRow, &rDoc, aDetails ) & ScRefFlags::VALID )
                    rDoc.SetRepeatRowRange( nTab, std::unique_ptr<ScRange>(new ScRange(aRange)) );
        }
    }

    //  undo (for all tables)
    if (bUndo)
    {
        SCTAB nCurTab = GetViewData().GetTabNo();
        std::unique_ptr<ScPrintRangeSaver> pNewRanges = rDoc.CreatePrintRangeSaver();
        pDocSh->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoPrintRange>( pDocSh, nCurTab, std::move(pOldRanges), std::move(pNewRanges) ) );
    }
    else
        pOldRanges.reset();

    //  update page breaks

    for (const auto& rTab : rMark)
        ScPrintFunc( pDocSh, pDocSh->GetPrinter(), rTab ).UpdatePages();

    SfxBindings& rBindings = GetViewData().GetBindings();
    rBindings.Invalidate( SID_DELETE_PRINTAREA );

    pDocSh->SetDocumentModified();
}

//  Merge cells

bool ScViewFunc::TestMergeCells()           // pre-test (for menu)
{
    //  simple test: true if there's a selection but no multi selection and not filtered

    const ScMarkData& rMark = GetViewData().GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        ScRange aRange;
        bool bMergable = ( GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE );
        bMergable = bMergable && ( aRange.aStart.Col() != aRange.aEnd.Col() ||
                                   aRange.aStart.Row() != aRange.aEnd.Row() );
        return bMergable;
    }
    else
        return false;
}

bool ScViewFunc::MergeCells( bool bApi, bool& rDoContents, bool bCenter )
{
    //  Editable- and Being-Nested- test must be at the beginning (in DocFunc too),
    //  so that the Contents-QueryBox won't appear
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return false;
    }

    ScMarkData& rMark = GetViewData().GetMarkData();
    rMark.MarkToSimple();
    if (!rMark.IsMarked())
    {
        ErrorMessage(STR_NOMULTISELECT);
        return false;
    }

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();

    ScRange aMarkRange;
    rMark.GetMarkArea( aMarkRange );
    SCCOL nStartCol = aMarkRange.aStart.Col();
    SCROW nStartRow = aMarkRange.aStart.Row();
    SCTAB nStartTab = aMarkRange.aStart.Tab();
    SCCOL nEndCol = aMarkRange.aEnd.Col();
    SCROW nEndRow = aMarkRange.aEnd.Row();
    SCTAB nEndTab = aMarkRange.aEnd.Tab();
    if ( nStartCol == nEndCol && nStartRow == nEndRow )
    {
        // nothing to do
        return true;
    }

    if ( rDoc.HasAttrib( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                            HasAttrFlags::Merged | HasAttrFlags::Overlapped ) )
    {       // "Don't nest merging  !"
        ErrorMessage(STR_MSSG_MERGECELLS_0);
        return false;
    }

    // Check for the contents of all selected tables.
    bool bAskDialog = false;
    ScCellMergeOption aMergeOption(nStartCol, nStartRow, nEndCol, nEndRow, bCenter);
    for (const SCTAB& i : rMark)
    {
        aMergeOption.maTabs.insert(i);

        sc::MultiDataCellState aState = rDoc.HasMultipleDataCells(aMergeOption.getSingleRange(i));
        switch (aState.meState)
        {
            case sc::MultiDataCellState::HasMultipleCells:
            {
                // this range contains multiple data cells.
                bAskDialog = true;
                break;
            }
            case sc::MultiDataCellState::HasOneCell:
            {
                // this range contains only one data cell.
                if (nStartCol != aState.mnCol1 || nStartRow != aState.mnRow1)
                    rDoContents = true; // move the value to the top-left.
                break;
            }
            default:
                ;
        }
    }

    bool bOk = true;
    bool bEmptyMergedCells = officecfg::Office::Calc::Compatibility::MergeCells::EmptyMergedCells::get();

    if (bAskDialog)
    {
        bool bShowDialog = officecfg::Office::Calc::Compatibility::MergeCells::ShowDialog::get();
        if (!bApi && bShowDialog)
        {
            vcl::Window* pWin = GetViewData().GetDialogParent();
            ScMergeCellsDialog aBox(pWin ? pWin->GetFrameWeld() : nullptr);
            sal_uInt16 nRetVal = aBox.run();

            if ( nRetVal == RET_OK )
            {
                switch (aBox.GetMergeCellsOption())
                {
                    case MoveContentHiddenCells:
                        rDoContents = true;
                        break;
                    case KeepContentHiddenCells:
                        bEmptyMergedCells = false;
                        break;
                    case EmptyContentHiddenCells:
                        bEmptyMergedCells = true;
                        break;
                    default:
                        assert(!"Unknown option for merge cells.");
                        break;
                }
            }
            else if ( nRetVal == RET_CANCEL )
                bOk = false;
        }
    }

    if (bOk)
    {
        bOk = pDocSh->GetDocFunc().MergeCells( aMergeOption, rDoContents, true/*bRecord*/, bApi, bEmptyMergedCells );

        if (bOk)
        {
            SetCursor( nStartCol, nStartRow );
            //DoneBlockMode( sal_False);
            Unmark();

            pDocSh->UpdateOle(&GetViewData());
            UpdateInputLine();
        }
    }

    return bOk;
}

bool ScViewFunc::TestRemoveMerge()
{
    bool bMerged = false;
    ScRange aRange;
    if (GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData().GetDocument();
        if ( pDoc->HasAttrib( aRange, HasAttrFlags::Merged ) )
            bMerged = true;
    }
    return bMerged;
}

static bool lcl_extendMergeRange(ScCellMergeOption& rOption, const ScRange& rRange)
{
    bool bExtended = false;
    if (rOption.mnStartCol > rRange.aStart.Col())
    {
        rOption.mnStartCol = rRange.aStart.Col();
        bExtended = true;
    }
    if (rOption.mnStartRow > rRange.aStart.Row())
    {
        rOption.mnStartRow = rRange.aStart.Row();
        bExtended = true;
    }
    if (rOption.mnEndCol < rRange.aEnd.Col())
    {
        rOption.mnEndCol = rRange.aEnd.Col();
        bExtended = true;
    }
    if (rOption.mnEndRow < rRange.aEnd.Row())
    {
        rOption.mnEndRow = rRange.aEnd.Row();
        bExtended = true;
    }
    return bExtended;
}

bool ScViewFunc::RemoveMerge()
{
    ScRange aRange;
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return false;
    }
    else if (GetViewData().GetSimpleArea( aRange ) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData().GetDocument();
        ScRange aExtended( aRange );
        pDoc->ExtendMerge( aExtended );
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        const ScMarkData& rMark = GetViewData().GetMarkData();
        ScCellMergeOption aOption(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row());
        bool bExtended = false;
        do
        {
            bExtended = false;
            for (const SCTAB& i : rMark)
            {
                aOption.maTabs.insert(i);
                aExtended.aStart.SetTab(i);
                aExtended.aEnd.SetTab(i);
                pDoc->ExtendMerge(aExtended);
                pDoc->ExtendOverlapped(aExtended);

                // Expand the current range to be inclusive of all merged
                // areas on all sheets.
                bExtended = lcl_extendMergeRange(aOption, aExtended);
            }
        }
        while (bExtended);

        bool bOk = pDocSh->GetDocFunc().UnmergeCells(aOption, true/*bRecord*/, nullptr);
        aExtended = aOption.getFirstSingleRange();
        MarkRange( aExtended );

        if (bOk)
            pDocSh->UpdateOle(&GetViewData());
    }
    return true;        //! bOk ??
}

void ScViewFunc::FillSimple( FillDir eDir )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        const ScMarkData& rMark = GetViewData().GetMarkData();
        bool bSuccess = pDocSh->GetDocFunc().FillSimple( aRange, &rMark, eDir, false );
        if (bSuccess)
        {
            pDocSh->UpdateOle(&GetViewData());
            UpdateScrollBars();

            bool bDoAutoSpell = pDocSh->GetDocument().GetDocOptions().IsAutoSpell();
            if ( bDoAutoSpell )
            {
                // Copy AutoSpellData from above(left/right/below) if no selection.
                switch (eDir)
                {
                    case FILL_TO_BOTTOM:
                        if (aRange.aStart.Row() > 0 && aRange.aStart.Row() == aRange.aEnd.Row())
                            aRange.aStart.IncRow(-1);
                    break;
                    case FILL_TO_TOP:
                        if (aRange.aEnd.Row() < MAXROW && aRange.aStart.Row() == aRange.aEnd.Row())
                            aRange.aEnd.IncRow(1);
                    break;
                    case FILL_TO_RIGHT:
                        if (aRange.aStart.Col() > 0 && aRange.aStart.Col() == aRange.aEnd.Col())
                            aRange.aStart.IncCol(-1);
                    break;
                    case FILL_TO_LEFT:
                        if (aRange.aEnd.Col() < MAXCOL && aRange.aStart.Col() == aRange.aEnd.Col())
                            aRange.aEnd.IncCol(1);
                    break;
                }
                CopyAutoSpellData(eDir, aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(),
                        ::std::numeric_limits<sal_uLong>::max());
            }

            // Invalidate cell slots and update input line with new content.
            CellContentChanged();
        }
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

void ScViewFunc::FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                             double fStart, double fStep, double fMax )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        const ScMarkData& rMark = GetViewData().GetMarkData();
        bool bSuccess = pDocSh->GetDocFunc().
                        FillSeries( aRange, &rMark, eDir, eCmd, eDateCmd,
                                    fStart, fStep, fMax, false );
        if (bSuccess)
        {
            pDocSh->UpdateOle(&GetViewData());
            UpdateScrollBars();

            HelperNotifyChanges::NotifyIfChangesListeners(*pDocSh, aRange);
        }
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

void ScViewFunc::FillAuto( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                            SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount )
{
    SCTAB nTab = GetViewData().GetTabNo();
    ScRange aRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab );
    ScRange aSourceRange( aRange );
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    const ScMarkData& rMark = GetViewData().GetMarkData();
    bool bSuccess = pDocSh->GetDocFunc().
                    FillAuto( aRange, &rMark, eDir, nCount, false );
    if (bSuccess)
    {
        MarkRange( aRange, false );         // aRange was modified in FillAuto
        pDocSh->UpdateOle(&GetViewData());
        UpdateScrollBars();

        bool bDoAutoSpell = pDocSh->GetDocument().GetDocOptions().IsAutoSpell();
        if ( bDoAutoSpell )
            CopyAutoSpellData(eDir, nStartCol, nStartRow, nEndCol, nEndRow, nCount);

        if (ScModelObj* pModelObj = HelperNotifyChanges::getMustPropagateChangesModel(*pDocSh))
        {
            ScRangeList aChangeRanges;
            ScRange aChangeRange( aRange );
            switch (eDir)
            {
                case FILL_TO_BOTTOM:
                    aChangeRange.aStart.SetRow( aSourceRange.aEnd.Row() + 1 );
                    break;
                case FILL_TO_TOP:
                    aChangeRange.aEnd.SetRow( aSourceRange.aStart.Row() - 1 );
                    break;
                case FILL_TO_RIGHT:
                    aChangeRange.aStart.SetCol( aSourceRange.aEnd.Col() + 1 );
                    break;
                case FILL_TO_LEFT:
                    aChangeRange.aEnd.SetCol( aSourceRange.aStart.Col() - 1 );
                    break;
                default:
                    break;
            }
            aChangeRanges.push_back( aChangeRange );
            HelperNotifyChanges::Notify(*pModelObj, aChangeRanges);
        }
    }
}

void ScViewFunc::CopyAutoSpellData( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                                   SCCOL nEndCol, SCROW nEndRow, sal_uLong nCount )
{
    ScGridWindow* pWin = GetActiveWin();
    if ( pWin->InsideVisibleRange(nStartCol, nStartRow) && pWin->InsideVisibleRange(nEndCol, nEndRow) )
    {
        if ( nCount == ::std::numeric_limits<sal_uLong>::max() )
        {
            switch( eDir )
            {
                case FILL_TO_BOTTOM:
                    for ( SCCOL nColItr = nStartCol; nColItr <= nEndCol; ++nColItr )
                    {
                        const std::vector<editeng::MisspellRanges>* pRanges = pWin->GetAutoSpellData(nColItr, nStartRow);
                        if ( !pRanges )
                            continue;
                        for ( SCROW nRowItr = nStartRow + 1; nRowItr <= nEndRow; ++nRowItr )
                            pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                    break;
                case FILL_TO_TOP:
                    for ( SCCOL nColItr = nStartCol; nColItr <= nEndCol; ++nColItr )
                    {
                        const std::vector<editeng::MisspellRanges>* pRanges = pWin->GetAutoSpellData(nColItr, nEndRow);
                        if ( !pRanges )
                            continue;
                        for ( SCROW nRowItr = nEndRow - 1; nRowItr >= nStartRow; --nRowItr )
                            pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                    break;
                case FILL_TO_RIGHT:
                    for ( SCROW nRowItr = nStartRow; nRowItr <= nEndRow; ++nRowItr )
                    {
                        const std::vector<editeng::MisspellRanges>* pRanges = pWin->GetAutoSpellData(nStartCol, nRowItr);
                        if ( !pRanges )
                            continue;
                        for ( SCCOL nColItr = nStartCol + 1; nColItr <= nEndCol; ++nColItr )
                            pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                    break;
                case FILL_TO_LEFT:
                    for ( SCROW nRowItr = nStartRow; nRowItr <= nEndRow; ++nRowItr )
                    {
                        const std::vector<editeng::MisspellRanges>* pRanges = pWin->GetAutoSpellData(nEndCol, nRowItr);
                        if ( !pRanges )
                            continue;
                        for ( SCCOL nColItr = nEndCol - 1; nColItr >= nStartCol; --nColItr )
                            pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                    break;
            }
            return;
        }

        typedef const std::vector<editeng::MisspellRanges>* MisspellRangesType;
        SCROW nRowRepeatSize = nEndRow - nStartRow + 1;
        SCCOL nColRepeatSize = nEndCol - nStartCol + 1;
        SCROW nTillRow = 0;
        SCCOL nTillCol = 0;
        std::vector<std::vector<MisspellRangesType>> aSourceSpellRanges(nRowRepeatSize, std::vector<MisspellRangesType>(nColRepeatSize));

        for ( SCROW nRowIdx = 0; nRowIdx < nRowRepeatSize; ++nRowIdx )
            for ( SCROW nColIdx = 0; nColIdx < nColRepeatSize; ++nColIdx )
                aSourceSpellRanges[nRowIdx][nColIdx] = pWin->GetAutoSpellData( nStartCol + nColIdx, nStartRow + nRowIdx );

        switch( eDir )
        {
            case FILL_TO_BOTTOM:
                nTillRow = nEndRow + nCount;
                for ( SCCOL nColItr = nStartCol; nColItr <= nEndCol; ++nColItr )
                {
                    for ( SCROW nRowItr = nEndRow + 1; nRowItr <= nTillRow; ++nRowItr )
                    {
                        size_t nSourceRowIdx = ( nRowItr - nEndRow - 1 ) % nRowRepeatSize;
                        MisspellRangesType pRanges = aSourceSpellRanges[nSourceRowIdx][nColItr - nStartCol];
                        if ( !pRanges )
                            continue;
                        pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                }
                break;

            case FILL_TO_TOP:
                nTillRow = nStartRow - nCount;
                for ( SCCOL nColItr = nStartCol; nColItr <= nEndCol; ++nColItr )
                {
                    for ( SCROW nRowItr = nStartRow - 1; nRowItr >= nTillRow; --nRowItr )
                    {
                        size_t nSourceRowIdx = nRowRepeatSize - 1 - ( ( nStartRow - 1 - nRowItr ) % nRowRepeatSize );
                        MisspellRangesType pRanges = aSourceSpellRanges[nSourceRowIdx][nColItr - nStartCol];
                        if ( !pRanges )
                            continue;
                        pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                }
                break;

            case FILL_TO_RIGHT:
                nTillCol = nEndCol + nCount;
                for ( SCCOL nColItr = nEndCol + 1; nColItr <= nTillCol; ++nColItr )
                {
                    size_t nSourceColIdx = ( nColItr - nEndCol - 1 ) % nColRepeatSize;
                    for ( SCROW nRowItr = nStartRow; nRowItr <= nEndRow; ++nRowItr )
                    {
                        MisspellRangesType pRanges = aSourceSpellRanges[nRowItr - nStartRow][nSourceColIdx];
                        if ( !pRanges )
                            continue;
                        pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                }
                break;

            case FILL_TO_LEFT:
                nTillCol = nStartCol - nCount;
                for ( SCCOL nColItr = nStartCol - 1; nColItr >= nTillCol; --nColItr )
                {
                    size_t nSourceColIdx = nColRepeatSize - 1 - ( ( nStartCol - 1 - nColItr ) % nColRepeatSize );
                    for ( SCROW nRowItr = nStartRow; nRowItr <= nEndRow; ++nRowItr )
                    {
                        MisspellRangesType pRanges = aSourceSpellRanges[nRowItr - nStartRow][nSourceColIdx];
                        if ( !pRanges )
                            continue;
                        pWin->SetAutoSpellData(nColItr, nRowItr, pRanges);
                    }
                }
                break;
        }
    }
    else
        pWin->ResetAutoSpell();

}

void ScViewFunc::FillTab( InsertDeleteFlags nFlags, ScPasteFunc nFunction, bool bSkipEmpty, bool bAsLink )
{
    //! allow source sheet to be protected
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    SCTAB nTab = GetViewData().GetTabNo();
    bool bUndo(rDoc.IsUndoEnabled());

    ScRange aMarkRange;
    rMark.MarkToSimple();
    bool bMulti = rMark.IsMultiMarked();
    if (bMulti)
        rMark.GetMultiMarkArea( aMarkRange );
    else if (rMark.IsMarked())
        rMark.GetMarkArea( aMarkRange );
    else
        aMarkRange = ScRange( GetViewData().GetCurX(), GetViewData().GetCurY(), nTab );

    ScDocumentUniquePtr pUndoDoc;

    if (bUndo)
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndo( &rDoc, nTab, nTab );

        for (const SCTAB& i : rMark)
            if (i != nTab )
            {
                pUndoDoc->AddUndoTab( i, i );
                aMarkRange.aStart.SetTab( i );
                aMarkRange.aEnd.SetTab( i );
                rDoc.CopyToDocument( aMarkRange, InsertDeleteFlags::ALL, bMulti, *pUndoDoc );
            }
    }

    if (bMulti)
        rDoc.FillTabMarked( nTab, rMark, nFlags, nFunction, bSkipEmpty, bAsLink );
    else
    {
        aMarkRange.aStart.SetTab( nTab );
        aMarkRange.aEnd.SetTab( nTab );
        rDoc.FillTab( aMarkRange, rMark, nFlags, nFunction, bSkipEmpty, bAsLink );
    }

    if (bUndo)
    {   //! for ChangeTrack not until the end
        pDocSh->GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoFillTable>( pDocSh, rMark,
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nTab,
                                aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab,
                                std::move(pUndoDoc), bMulti, nTab, nFlags, nFunction, bSkipEmpty, bAsLink ) );
    }

    pDocSh->PostPaintGridAll();
    pDocSh->PostDataChanged();
}

/** Downward fill of selected cell(s) by double-clicking cross-hair cursor

    Extends a current selection down to the last non-empty cell of an adjacent
    column when the lower-right corner of the selection is double-clicked.  It
    uses a left-adjoining non-empty column as a guide if such is available,
    otherwise a right-adjoining non-empty column is used.

    @return No return value

    @see #i12313#
*/
void ScViewFunc::FillCrossDblClick()
{
    ScRange aRange;
    GetViewData().GetSimpleArea( aRange );
    aRange.PutInOrder();

    SCTAB nTab = GetViewData().GetCurPos().Tab();
    SCCOL nStartX = aRange.aStart.Col();
    SCROW nStartY = aRange.aStart.Row();
    SCCOL nEndX   = aRange.aEnd.Col();
    SCROW nEndY   = aRange.aEnd.Row();

    ScDocument* pDoc = GetViewData().GetDocument();

    // Make sure the selection is not empty
    if ( pDoc->IsBlockEmpty( nTab, nStartX, nStartY, nEndX, nEndY ) )
        return;

    if ( nEndY < MAXROW )
    {
        if ( nStartX > 0 )
        {
            SCCOL nMovX = nStartX - 1;
            SCROW nMovY = nStartY;

            if ( pDoc->HasData( nMovX, nStartY, nTab ) &&
                 pDoc->HasData( nMovX, nStartY + 1, nTab ) )
            {
                pDoc->FindAreaPos( nMovX, nMovY, nTab, SC_MOVE_DOWN );

                if ( nMovY > nEndY )
                {
                    FillAuto( FILL_TO_BOTTOM, nStartX, nStartY, nEndX, nEndY,
                              nMovY - nEndY );
                    return;
                }
            }
        }

        if ( nEndX < MAXCOL )
        {
            SCCOL nMovX = nEndX + 1;
            SCROW nMovY = nStartY;

            if ( pDoc->HasData( nMovX, nStartY, nTab ) &&
                 pDoc->HasData( nMovX, nStartY + 1, nTab ) )
            {
                pDoc->FindAreaPos( nMovX, nMovY, nTab, SC_MOVE_DOWN );

                if ( nMovY > nEndY )
                {
                    FillAuto( FILL_TO_BOTTOM, nStartX, nStartY, nEndX, nEndY,
                              nMovY - nEndY );
                    return;
                }
            }
        }
    }
}

void ScViewFunc::ConvertFormulaToValue()
{
    ScRange aRange;
    GetViewData().GetSimpleArea(aRange);
    aRange.PutInOrder();

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    pDocSh->GetDocFunc().ConvertFormulaToValue(aRange, true);
    pDocSh->PostPaint(aRange, PaintPartFlags::Grid);
}

void ScViewFunc::TransliterateText( TransliterationFlags nType )
{
    ScMarkData aFuncMark = GetViewData().GetMarkData();
    if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
    {
        //  no selection -> use cursor position

        ScAddress aCursor( GetViewData().GetCurX(), GetViewData().GetCurY(), GetViewData().GetTabNo() );
        aFuncMark.SetMarkArea( ScRange( aCursor ) );
    }

    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().
                        TransliterateText( aFuncMark, nType, false );
    if (bSuccess)
    {
        GetViewData().GetViewShell()->UpdateInputHandler();
    }
}

//  AutoFormat

ScAutoFormatData* ScViewFunc::CreateAutoFormatData()
{
    ScAutoFormatData* pData = nullptr;
    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;
    if (GetViewData().GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
    {
        if ( nEndCol-nStartCol >= 3 && nEndRow-nStartRow >= 3 )
        {
            ScDocument* pDoc = GetViewData().GetDocument();
            pData = new ScAutoFormatData;
            pDoc->GetAutoFormatData( nStartTab, nStartCol,nStartRow,nEndCol,nEndRow, *pData );
        }
    }
    return pData;
}

void ScViewFunc::AutoFormat( sal_uInt16 nFormatNo )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScMarkData& rMark = GetViewData().GetMarkData();

        bool bSuccess = pDocSh->GetDocFunc().AutoFormat( aRange, &rMark, nFormatNo, false );
        if (bSuccess)
            pDocSh->UpdateOle(&GetViewData());
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

// Search & Replace

bool ScViewFunc::SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        bool bAddUndo, bool bIsApi )
{
    SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::Empty);
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData().GetMarkData();
    if (bAddUndo && !rDoc.IsUndoEnabled())
        bAddUndo = false;

    if ( !rMark.IsMarked() && !rMark.IsMultiMarked() && (pSearchItem->HasStartPoint()) )
    {
        // No selection -> but we have a start point (top left corner of the
        // current view), start searching from there, not from the current
        // cursor position.
        SCCOL nPosX;
        SCROW nPosY;

        int nPixelX = pSearchItem->GetStartPointX() * GetViewData().GetPPTX();
        int nPixelY = pSearchItem->GetStartPointY() * GetViewData().GetPPTY();

        GetViewData().GetPosFromPixel(nPixelX, nPixelY, GetViewData().GetActivePart(), nPosX, nPosY);

        AlignToCursor( nPosX, nPosY, SC_FOLLOW_JUMP );
        SetCursor( nPosX, nPosY, true );
    }

    SCCOL nCol, nOldCol;
    SCROW nRow, nOldRow;
    SCTAB nTab, nOldTab;
    nCol = nOldCol = GetViewData().GetCurX();
    nRow = nOldRow = GetViewData().GetCurY();
    nTab = nOldTab = GetViewData().GetTabNo();

    SvxSearchCmd nCommand = pSearchItem->GetCommand();
    bool bAllTables = pSearchItem->IsAllTables();
    std::set<SCTAB> aOldSelectedTables;
    SCTAB nLastTab = rDoc.GetTableCount() - 1;
    SCTAB nStartTab, nEndTab;
    if ( bAllTables )
    {
        nStartTab = 0;
        nEndTab = nLastTab;
        std::set<SCTAB> aTmp(rMark.begin(), rMark.end());
        aOldSelectedTables.swap(aTmp);
    }
    else
    {   //! at least one is always selected
        nStartTab = rMark.GetFirstSelected();
        nEndTab = rMark.GetLastSelected();
    }

    if (   nCommand == SvxSearchCmd::FIND
        || nCommand == SvxSearchCmd::FIND_ALL)
        bAddUndo = false;

    //!     account for bAttrib during Undo !!!

    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScMarkData> pUndoMark;
    OUString aUndoStr;
    if (bAddUndo)
    {
        pUndoMark.reset(new ScMarkData(rMark));                // Mark is being modified
        if ( nCommand == SvxSearchCmd::REPLACE_ALL )
        {
            pUndoDoc.reset(new ScDocument(SCDOCMODE_UNDO));
            pUndoDoc->InitUndo( &rDoc, nStartTab, nEndTab );
        }
    }

    if ( bAllTables )
    {   //! select all, after pUndoMark has been created
        for ( SCTAB j = nStartTab; j <= nEndTab; j++ )
        {
            rMark.SelectTable( j, true );
        }
    }

    DoneBlockMode(true);                // don't delete mark
    InitOwnBlockMode();

    //  If search starts at the beginning don't ask again whether it shall start at the beginning
    bool bFirst = true;
    if ( nCol == 0 && nRow == 0 && nTab == nStartTab && !pSearchItem->GetBackward()  )
        bFirst = false;

    bool bFound = false;
    while (true)
    {
        GetFrameWin()->EnterWait();
        ScRangeList aMatchedRanges;
        if (rDoc.SearchAndReplace(*pSearchItem, nCol, nRow, nTab, rMark, aMatchedRanges, aUndoStr, pUndoDoc.get()))
        {
            bFound = true;
            if (bAddUndo)
            {
                GetViewData().GetDocShell()->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoReplace>( GetViewData().GetDocShell(), *pUndoMark,
                                        nCol, nRow, nTab,
                                        aUndoStr, std::move(pUndoDoc), pSearchItem ) );
            }

            if (nCommand == SvxSearchCmd::FIND_ALL || nCommand == SvxSearchCmd::REPLACE_ALL)
            {
                SfxViewFrame* pViewFrm = SfxViewFrame::Current();
                if (pViewFrm && !comphelper::LibreOfficeKit::isActive())
                {
                    pViewFrm->ShowChildWindow(sc::SearchResultsDlgWrapper::GetChildWindowId());
                    SfxChildWindow* pWnd = pViewFrm->GetChildWindow(sc::SearchResultsDlgWrapper::GetChildWindowId());
                    if (pWnd)
                    {
                        sc::SearchResultsDlg* pDlg = static_cast<sc::SearchResultsDlg*>(pWnd->GetWindow());
                        if (pDlg)
                            pDlg->FillResults(&rDoc, aMatchedRanges,
                                    pSearchItem->GetCellType() == SvxSearchCellType::NOTE);
                    }
                }

                rMark.ResetMark();
                for (size_t i = 0, n = aMatchedRanges.size(); i < n; ++i)
                {
                    const ScRange& r = aMatchedRanges[i];
                    if (r.aStart.Tab() == nTab)
                        rMark.SetMultiMarkArea(r);
                }
            }

            break;                  // break 'while (TRUE)'
        }
        else if ( bFirst && (nCommand == SvxSearchCmd::FIND ||
                nCommand == SvxSearchCmd::REPLACE) )
        {
            bFirst = false;
            GetFrameWin()->LeaveWait();
            if (!bIsApi)
            {
                if ( nStartTab == nEndTab )
                    SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::EndSheet);
                else
                    SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::End);

                ScDocument::GetSearchAndReplaceStart( *pSearchItem, nCol, nRow );
                if (pSearchItem->GetBackward())
                    nTab = nEndTab;
                else
                    nTab = nStartTab;
            }
            else
            {
                break;                  // break 'while (TRUE)'
            }
        }
        else                            // nothing found
        {
            if ( nCommand == SvxSearchCmd::FIND_ALL || nCommand == SvxSearchCmd::REPLACE_ALL )
            {
                pDocSh->PostPaintGridAll();                             // Mark
            }

            GetFrameWin()->LeaveWait();
            if (!bIsApi)
            {
                GetViewData().GetViewShell()->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_NOT_FOUND, pSearchItem->GetSearchString().toUtf8().getStr());
                SvxSearchDialogWrapper::SetSearchLabel(SearchLabel::NotFound);
            }

            break;                      // break 'while (TRUE)'
        }
    }                               // of while true

    if (!aOldSelectedTables.empty())
    {
        // restore originally selected table
        for (SCTAB i = 0; i <= nEndTab; ++i)
            rMark.SelectTable(i, false);

        for (const auto& rTab : aOldSelectedTables)
            rMark.SelectTable(rTab, true);

        if ( bFound )
        {   // if a table is selected as a "match" it remains selected.
            rMark.SelectTable( nTab, true );
            // It's a swap if only one table was selected before
            //! otherwise now one table more might be selected
            if ( aOldSelectedTables.size() == 1 && nTab != nOldTab )
                rMark.SelectTable( nOldTab, false );
        }
    }

    // Avoid LOK selection notifications before we have all the results.
    GetViewData().GetViewShell()->setTiledSearching(true);
    MarkDataChanged();
    GetViewData().GetViewShell()->setTiledSearching(false);

    if ( bFound )
    {
        if ( nTab != GetViewData().GetTabNo() )
            SetTabNo( nTab );

        //  if nothing is marked, DoneBlockMode, then marking can start
        //  directly from this place via Shift-Cursor
        if (!rMark.IsMarked() && !rMark.IsMultiMarked())
            DoneBlockMode(true);

        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
        SetCursor( nCol, nRow, true );

        if (comphelper::LibreOfficeKit::isActive())
        {
            Point aCurPos = GetViewData().GetScrPos(nCol, nRow, GetViewData().GetActivePart());

            // just update the cell selection
            ScGridWindow* pGridWindow = GetViewData().GetActiveWin();
            // Don't move cell selection handles for find-all: selection of all but the first result would be lost.
            if (pGridWindow && nCommand == SvxSearchCmd::FIND)
            {
                // move the cell selection handles
                pGridWindow->SetCellSelectionPixel(LOK_SETTEXTSELECTION_RESET, aCurPos.X(), aCurPos.Y());
                pGridWindow->SetCellSelectionPixel(LOK_SETTEXTSELECTION_START, aCurPos.X(), aCurPos.Y());
                pGridWindow->SetCellSelectionPixel(LOK_SETTEXTSELECTION_END, aCurPos.X(), aCurPos.Y());
            }

            if (pGridWindow)
            {
                std::vector<tools::Rectangle> aLogicRects;
                pGridWindow->GetCellSelection(aLogicRects);

                boost::property_tree::ptree aTree;
                aTree.put("searchString", pSearchItem->GetSearchString().toUtf8().getStr());
                aTree.put("highlightAll", nCommand == SvxSearchCmd::FIND_ALL);

                boost::property_tree::ptree aSelections;
                for (const tools::Rectangle& rLogicRect : aLogicRects)
                {
                    boost::property_tree::ptree aSelection;
                    aSelection.put("part", OString::number(nTab).getStr());
                    aSelection.put("rectangles", rLogicRect.toString().getStr());
                    aSelections.push_back(std::make_pair("", aSelection));
                }
                aTree.add_child("searchResultSelection", aSelections);

                std::stringstream aStream;
                boost::property_tree::write_json(aStream, aTree);
                OString aPayload = aStream.str().c_str();
                SfxViewShell* pViewShell = GetViewData().GetViewShell();
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_SEARCH_RESULT_SELECTION, aPayload.getStr());

                // Trigger LOK_CALLBACK_TEXT_SELECTION now.
                MarkDataChanged();
            }
        }

        if (   nCommand == SvxSearchCmd::REPLACE
            || nCommand == SvxSearchCmd::REPLACE_ALL )
        {
            if ( nCommand == SvxSearchCmd::REPLACE )
            {
                pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PaintPartFlags::Grid );

                // jump to next cell if we replaced everything in the cell
                // where the cursor was positioned (but avoid switching tabs)
                if ( nCol == nOldCol && nRow == nOldRow && nTab == nOldTab )
                {
                    SvxSearchItem aSearchItem = ScGlobal::GetSearchItem();
                    aSearchItem.SetCommand(SvxSearchCmd::FIND);
                    aSearchItem.SetWhich(SID_SEARCH_ITEM);

                    ScRangeList aMatchedRanges;
                    ScTable::UpdateSearchItemAddressForReplace( aSearchItem, nCol, nRow );
                    if ( rDoc.SearchAndReplace( aSearchItem, nCol, nRow, nTab, rMark, aMatchedRanges, aUndoStr ) &&
                            ( nTab == nOldTab ) &&
                            ( nCol != nOldCol || nRow != nOldRow ) )
                    {
                        AlignToCursor(nCol, nRow, SC_FOLLOW_JUMP);
                        SetCursor( nCol, nRow, true );
                    }
                }
            }
            else
                pDocSh->PostPaintGridAll();
            pDocSh->SetDocumentModified();
        }
        else if ( nCommand == SvxSearchCmd::FIND_ALL )
            pDocSh->PostPaintGridAll();                             // mark
        GetFrameWin()->LeaveWait();
    }
    return bFound;
}

// Goal Seek

void ScViewFunc::Solve( const ScSolveParam& rParam )
{
    ScDocument* pDoc = GetViewData().GetDocument();

    if ( pDoc )
    {
        SCCOL nDestCol = rParam.aRefVariableCell.Col();
        SCROW nDestRow = rParam.aRefVariableCell.Row();
        SCTAB nDestTab = rParam.aRefVariableCell.Tab();

        ScEditableTester aTester( pDoc, nDestTab, nDestCol,nDestRow, nDestCol,nDestRow );
        if (!aTester.IsEditable())
        {
            ErrorMessage(aTester.GetMessageId());
            return;
        }

        OUString  aTargetValStr;
        if ( rParam.pStrTargetVal )
            aTargetValStr = *rParam.pStrTargetVal;

        OUString  aMsgStr;
        OUString  aResStr;
        double  nSolveResult;

        GetFrameWin()->EnterWait();

        bool    bExact =
                    pDoc->Solver(
                        rParam.aRefFormulaCell.Col(),
                        rParam.aRefFormulaCell.Row(),
                        rParam.aRefFormulaCell.Tab(),
                        nDestCol, nDestRow, nDestTab,
                        aTargetValStr,
                        nSolveResult );

        GetFrameWin()->LeaveWait();

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        sal_uLong nFormat = 0;
        const ScPatternAttr* pPattern = pDoc->GetPattern( nDestCol, nDestRow, nDestTab );
        if ( pPattern )
            nFormat = pPattern->GetNumberFormat( pFormatter );
        Color* p;
        pFormatter->GetOutputString( nSolveResult, nFormat, aResStr, &p );

        if ( bExact )
        {
            aMsgStr += ScResId( STR_MSSG_SOLVE_0 );
            aMsgStr += aResStr;
            aMsgStr += ScResId( STR_MSSG_SOLVE_1 );
        }
        else
        {
            aMsgStr  = ScResId( STR_MSSG_SOLVE_2 );
            aMsgStr += ScResId( STR_MSSG_SOLVE_3 );
            aMsgStr += aResStr ;
            aMsgStr += ScResId( STR_MSSG_SOLVE_4 );
        }

        vcl::Window* pWin = GetViewData().GetDialogParent();
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                  VclMessageType::Question, VclButtonsType::YesNo, aMsgStr));
        xBox->set_title(ScResId(STR_MSSG_DOSUBTOTALS_0));
        xBox->set_default_response(RET_NO);
        if (xBox->run() == RET_YES)
            EnterValue( nDestCol, nDestRow, nDestTab, nSolveResult );

        GetViewData().GetViewShell()->UpdateInputHandler( true );
    }
}

//  multi operation

void ScViewFunc::TabOp( const ScTabOpParam& rParam, bool bRecord )
{
    ScRange aRange;
    if (GetViewData().GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData().GetDocShell();
        ScMarkData& rMark = GetViewData().GetMarkData();
        pDocSh->GetDocFunc().TabOp( aRange, &rMark, rParam, bRecord, false );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

void ScViewFunc::MakeScenario( const OUString& rName, const OUString& rComment,
                                    const Color& rColor, ScScenarioFlags nFlags )
{
    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    ScMarkData& rMark   = GetViewData().GetMarkData();
    SCTAB       nTab    = GetViewData().GetTabNo();

    SCTAB nNewTab = pDocSh->MakeScenario( nTab, rName, rComment, rColor, nFlags, rMark );
    if (nFlags & ScScenarioFlags::CopyAll)
        SetTabNo( nNewTab, true );          // ScScenarioFlags::CopyAll -> visible
    else
    {
        SfxBindings& rBindings = GetViewData().GetBindings();
        rBindings.Invalidate( SID_STATUS_DOCPOS );      // Statusbar
        rBindings.Invalidate( SID_ROWCOL_SELCOUNT );    // Statusbar
        rBindings.Invalidate( SID_TABLES_COUNT );
        rBindings.Invalidate( SID_SELECT_SCENARIO );
        rBindings.Invalidate( FID_TABLE_SHOW );
    }
}

void ScViewFunc::ExtendScenario()
{
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

        //  Undo: apply attributes

    ScDocument* pDoc = GetViewData().GetDocument();
    ScPatternAttr aPattern( pDoc->GetPool() );
    aPattern.GetItemSet().Put( ScMergeFlagAttr( ScMF::Scenario ) );
    aPattern.GetItemSet().Put( ScProtectionAttr( true ) );
    ApplySelectionPattern(aPattern);
}

void ScViewFunc::UseScenario( const OUString& rName )
{
    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    SCTAB       nTab    = GetViewData().GetTabNo();

    DoneBlockMode();
    InitOwnBlockMode();
    pDocSh->UseScenario( nTab, rName );
}

//  Insert table

bool ScViewFunc::InsertTable( const OUString& rName, SCTAB nTab, bool bRecord )
{
    //  Order Tabl/Name is inverted for DocFunc
    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().
                        InsertTable( nTab, rName, bRecord, false );
    if (bSuccess)
        SetTabNo( nTab, true );

    return bSuccess;
}

//  Insert tables

void ScViewFunc::InsertTables(std::vector<OUString>& aNames, SCTAB nTab,
                                            SCTAB nCount, bool bRecord )
{
    ScDocShell* pDocSh    = GetViewData().GetDocShell();
    ScDocument& rDoc     = pDocSh->GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    WaitObject aWait( GetFrameWin() );

    if (bRecord)
    {
        rDoc.BeginDrawUndo();                            //    InsertTab creates a SdrUndoNewPage
    }

    bool bFlag=false;

    if(aNames.empty())
    {
        rDoc.CreateValidTabNames(aNames, nCount);
    }
    if (rDoc.InsertTabs(nTab, aNames))
    {
        pDocSh->Broadcast( ScTablesHint( SC_TABS_INSERTED, nTab, nCount ) );
        bFlag = true;
    }

    if (bFlag)
    {
        if (bRecord)
            pDocSh->GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoInsertTables>( pDocSh, nTab, aNames));

        //    Update views

        SetTabNo( nTab, true );
        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
    }
}

bool ScViewFunc::AppendTable( const OUString& rName, bool bRecord )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc   = pDocSh->GetDocument();
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;

    WaitObject aWait( GetFrameWin() );

    if (bRecord)
        rDoc.BeginDrawUndo();                          //  InsertTab creates a SdrUndoNewPage

    if (rDoc.InsertTab( SC_TAB_APPEND, rName ))
    {
        SCTAB nTab = rDoc.GetTableCount()-1;
        if (bRecord)
            pDocSh->GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoInsertTab>( pDocSh, nTab, true, rName));
        GetViewData().InsertTab( nTab );
        SetTabNo( nTab, true );
        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
        return true;
    }
    else
    {
        return false;
    }
}

void ScViewFunc::DeleteTable( SCTAB nTab, bool bRecord )
{
    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    ScDocument& rDoc    = pDocSh->GetDocument();

    bool bSuccess = pDocSh->GetDocFunc().DeleteTable( nTab, bRecord );
    if (bSuccess)
    {
        SCTAB nNewTab = nTab;
        if ( nNewTab >= rDoc.GetTableCount() )
            --nNewTab;
        SetTabNo( nNewTab, true );
    }
}

//only use this method for undo for now, all sheets must be connected
//this method doesn't support undo for now, merge it when it with the other method later
void ScViewFunc::DeleteTables( const SCTAB nTab, SCTAB nSheets )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc    = pDocSh->GetDocument();
    bool bVbaEnabled = rDoc.IsInVBAMode();
    SCTAB       nNewTab = nTab;
    WaitObject aWait( GetFrameWin() );

    while ( nNewTab > 0 && !rDoc.IsVisible( nNewTab ) )
        --nNewTab;

    if (rDoc.DeleteTabs(nTab, nSheets))
    {
        if( bVbaEnabled )
        {
            for (SCTAB aTab = 0; aTab < nSheets; ++aTab)
            {
                OUString sCodeName;
                bool bHasCodeName = rDoc.GetCodeName( nTab + aTab, sCodeName );
                if ( bHasCodeName )
                    VBA_DeleteModule( *pDocSh, sCodeName );
            }
        }

        pDocSh->Broadcast( ScTablesHint( SC_TABS_DELETED, nTab, nSheets ) );
        if ( nNewTab >= rDoc.GetTableCount() )
            nNewTab = rDoc.GetTableCount() - 1;
        SetTabNo( nNewTab, true );

        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();

        SfxApplication* pSfxApp = SfxGetpApp();                                // Navigator
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );
    }
}

bool ScViewFunc::DeleteTables(const vector<SCTAB> &TheTabs, bool bRecord )
{
    ScDocShell* pDocSh  = GetViewData().GetDocShell();
    ScDocument& rDoc    = pDocSh->GetDocument();
    bool bVbaEnabled = rDoc.IsInVBAMode();
    SCTAB       nNewTab = TheTabs.front();
    WaitObject aWait( GetFrameWin() );
    if (bRecord && !rDoc.IsUndoEnabled())
        bRecord = false;
    if ( bVbaEnabled )
        bRecord = false;

    while ( nNewTab > 0 && !rDoc.IsVisible( nNewTab ) )
        --nNewTab;

    bool bWasLinked = false;
    ScDocumentUniquePtr pUndoDoc;
    std::unique_ptr<ScRefUndoData> pUndoData;
    if (bRecord)
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        SCTAB nCount = rDoc.GetTableCount();

        OUString aOldName;
        for(size_t i=0; i<TheTabs.size(); ++i)
        {
            SCTAB nTab = TheTabs[i];
            if (i==0)
                pUndoDoc->InitUndo( &rDoc, nTab,nTab, true,true );   // incl. column/fow flags
            else
                pUndoDoc->AddUndoTab( nTab,nTab, true,true );       // incl. column/fow flags

            rDoc.CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, InsertDeleteFlags::ALL,false, *pUndoDoc );
            rDoc.GetName( nTab, aOldName );
            pUndoDoc->RenameTab( nTab, aOldName );
            if (rDoc.IsLinked(nTab))
            {
                bWasLinked = true;
                pUndoDoc->SetLink( nTab, rDoc.GetLinkMode(nTab), rDoc.GetLinkDoc(nTab),
                                    rDoc.GetLinkFlt(nTab), rDoc.GetLinkOpt(nTab),
                                    rDoc.GetLinkTab(nTab),
                                    rDoc.GetLinkRefreshDelay(nTab) );
            }
            if ( rDoc.IsScenario(nTab) )
            {
                pUndoDoc->SetScenario( nTab, true );
                OUString aComment;
                Color  aColor;
                ScScenarioFlags nScenFlags;
                rDoc.GetScenarioData( nTab, aComment, aColor, nScenFlags );
                pUndoDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
                bool bActive = rDoc.IsActiveScenario( nTab );
                pUndoDoc->SetActiveScenario( nTab, bActive );
            }
            pUndoDoc->SetVisible( nTab, rDoc.IsVisible( nTab ) );
            pUndoDoc->SetTabBgColor( nTab, rDoc.GetTabBgColor(nTab) );
            auto pSheetEvents = rDoc.GetSheetEvents( nTab );
            pUndoDoc->SetSheetEvents( nTab, std::unique_ptr<ScSheetEvents>(pSheetEvents ? new ScSheetEvents(*pSheetEvents) : nullptr) );
            pUndoDoc->SetLayoutRTL( nTab, rDoc.IsLayoutRTL( nTab ) );

            if ( rDoc.IsTabProtected( nTab ) )
                pUndoDoc->SetTabProtection(nTab, rDoc.GetTabProtection(nTab));

            //  Drawing-Layer is responsible for its Undo  !!!
            //      pUndoDoc->TransferDrawPage(pDoc, nTab,nTab);
        }

        pUndoDoc->AddUndoTab( 0, nCount-1 );            //  all Tabs for references

        rDoc.BeginDrawUndo();                          //  DeleteTab creates a SdrUndoDelPage

        pUndoData.reset(new ScRefUndoData( &rDoc ));
    }

    bool bDelDone = false;

    for(int i=TheTabs.size()-1; i>=0; --i)
    {
        OUString sCodeName;
        bool bHasCodeName = rDoc.GetCodeName( TheTabs[i], sCodeName );
        if (rDoc.DeleteTab(TheTabs[i]))
        {
            bDelDone = true;
            if( bVbaEnabled && bHasCodeName )
            {
                VBA_DeleteModule( *pDocSh, sCodeName );
            }
            pDocSh->Broadcast( ScTablesHint( SC_TAB_DELETED, TheTabs[i] ) );
        }
    }
    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
                    std::make_unique<ScUndoDeleteTab>( GetViewData().GetDocShell(), TheTabs,
                                            std::move(pUndoDoc), std::move(pUndoData) ));
    }

    if (bDelDone)
    {
        if ( nNewTab >= rDoc.GetTableCount() )
            nNewTab = rDoc.GetTableCount() - 1;

        SetTabNo( nNewTab, true );

        if (bWasLinked)
        {
            pDocSh->UpdateLinks();              // update Link-Manager
            GetViewData().GetBindings().Invalidate(SID_LINKS);
        }

        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();

        SfxApplication* pSfxApp = SfxGetpApp();                                // Navigator
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScDbAreasChanged ) );
        pSfxApp->Broadcast( SfxHint( SfxHintId::ScAreaLinksChanged ) );
    }
    return bDelDone;
}

bool ScViewFunc::RenameTable( const OUString& rName, SCTAB nTab )
{
    //  order Table/Name is inverted for DocFunc
    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().
                        RenameTable( nTab, rName, true, false );
    if (bSuccess)
    {
        //  the table name might be part of a formula
        GetViewData().GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}

bool ScViewFunc::SetTabBgColor( const Color& rColor, SCTAB nTab )
{
    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().SetTabBgColor( nTab, rColor, true, false );
    if (bSuccess)
    {
        GetViewData().GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}

bool ScViewFunc::SetTabBgColor( ScUndoTabColorInfo::List& rUndoSetTabBgColorInfoList )
{
    bool bSuccess = GetViewData().GetDocShell()->GetDocFunc().SetTabBgColor( rUndoSetTabBgColorInfoList, false );
    if (bSuccess)
    {
        GetViewData().GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}

void ScViewFunc::InsertAreaLink( const OUString& rFile,
                                    const OUString& rFilter, const OUString& rOptions,
                                    const OUString& rSource )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    SCCOL nPosX = GetViewData().GetCurX();
    SCROW nPosY = GetViewData().GetCurY();
    SCTAB nTab = GetViewData().GetTabNo();
    ScAddress aPos( nPosX, nPosY, nTab );

    pDocSh->GetDocFunc().InsertAreaLink( rFile, rFilter, rOptions, rSource, aPos, 0/*nRefresh*/, false, false );
}

void ScViewFunc::InsertTableLink( const OUString& rFile,
                                    const OUString& rFilter, const OUString& rOptions,
                                    const OUString& rTabName )
{
    OUString aFilterName = rFilter;
    OUString aOpt = rOptions;
    ScDocumentLoader aLoader( rFile, aFilterName, aOpt );
    if (!aLoader.IsError())
    {
        ScDocShell* pSrcSh = aLoader.GetDocShell();
        ScDocument& rSrcDoc = pSrcSh->GetDocument();
        SCTAB nTab = MAXTAB+1;
        if (rTabName.isEmpty())                // no name given -> first table
            nTab = 0;
        else
        {
            OUString aTemp;
            SCTAB nCount = rSrcDoc.GetTableCount();
            for (SCTAB i=0; i<nCount; i++)
            {
                rSrcDoc.GetName( i, aTemp );
                if ( aTemp == rTabName )
                    nTab = i;
            }
        }

        if ( nTab <= MAXTAB )
            ImportTables( pSrcSh, 1, &nTab, true,
                        GetViewData().GetTabNo() );
    }
}

//  Copy/link tables from another document

void ScViewFunc::ImportTables( ScDocShell* pSrcShell,
                                SCTAB nCount, const SCTAB* pSrcTabs, bool bLink,SCTAB nTab )
{
    ScDocument& rSrcDoc = pSrcShell->GetDocument();

    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());

    bool bError = false;
    bool bRefs = false;
    bool bName = false;

    if (rSrcDoc.GetDrawLayer())
        pDocSh->MakeDrawLayer();

    if (bUndo)
        rDoc.BeginDrawUndo();          // drawing layer must do its own undo actions

    SCTAB nInsCount = 0;
    SCTAB i;
    for( i=0; i<nCount; i++ )
    {   // insert sheets first and update all references
        OUString aName;
        rSrcDoc.GetName( pSrcTabs[i], aName );
        rDoc.CreateValidTabName( aName );
        if ( !rDoc.InsertTab( nTab+i, aName ) )
        {
            bError = true;      // total error
            break;  // for
        }
        ++nInsCount;
    }
    for (i=0; i<nCount && !bError; i++)
    {
        SCTAB nSrcTab = pSrcTabs[i];
        SCTAB nDestTab1=nTab+i;
        sal_uLong nErrVal = pDocSh->TransferTab( *pSrcShell, nSrcTab, nDestTab1,
            false, false );     // no insert

        switch (nErrVal)
        {
            case 0:                     // internal error or full of errors
                bError = true;
                break;
            case 2:
                bRefs = true;
                break;
            case 3:
                bName = true;
                break;
            case 4:
                bRefs = bName = true;
                break;
        }

    }

    if (bLink)
    {
        sfx2::LinkManager* pLinkManager = rDoc.GetLinkManager();

        SfxMedium* pMed = pSrcShell->GetMedium();
        OUString aFileName = pMed->GetName();
        OUString aFilterName;
        if (pMed->GetFilter())
            aFilterName = pMed->GetFilter()->GetFilterName();
        OUString aOptions = ScDocumentLoader::GetOptions(*pMed);

        bool bWasThere = rDoc.HasLink( aFileName, aFilterName, aOptions );

        sal_uLong nRefresh = 0;
        OUString aTabStr;
        for (i=0; i<nInsCount; i++)
        {
            rSrcDoc.GetName( pSrcTabs[i], aTabStr );
            rDoc.SetLink( nTab+i, ScLinkMode::NORMAL,
                        aFileName, aFilterName, aOptions, aTabStr, nRefresh );
        }

        if (!bWasThere)         // Insert link only once per source document
        {
            ScTableLink* pLink = new ScTableLink( pDocSh, aFileName, aFilterName, aOptions, nRefresh );
            pLink->SetInCreate( true );
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName, &aFilterName );
            pLink->Update();
            pLink->SetInCreate( false );

            SfxBindings& rBindings = GetViewData().GetBindings();
            rBindings.Invalidate( SID_LINKS );
        }
    }

    if (bUndo)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
                std::make_unique<ScUndoImportTab>( pDocSh, nTab, nCount ) );
    }

    for (i=0; i<nInsCount; i++)
        GetViewData().InsertTab(nTab);
    SetTabNo(nTab,true);
    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                PaintPartFlags::Grid | PaintPartFlags::Top | PaintPartFlags::Left | PaintPartFlags::Extras );

    SfxApplication* pSfxApp = SfxGetpApp();
    pSfxApp->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
    pSfxApp->Broadcast( SfxHint( SfxHintId::ScAreasChanged ) );

    pDocSh->PostPaintExtras();
    pDocSh->PostPaintGridAll();
    pDocSh->SetDocumentModified();

    if (bRefs)
        ErrorMessage(STR_ABSREFLOST);
    if (bName)
        ErrorMessage(STR_NAMECONFLICT);
}

//  Move/Copy table to another document

void ScViewFunc::MoveTable(
    sal_uInt16 nDestDocNo, SCTAB nDestTab, bool bCopy, const OUString* pNewTabName )
{
    ScDocument* pDoc       = GetViewData().GetDocument();
    ScDocShell* pDocShell  = GetViewData().GetDocShell();
    ScDocument* pDestDoc   = nullptr;
    ScDocShell* pDestShell = nullptr;
    ScTabViewShell* pDestViewSh = nullptr;
    bool bUndo (pDoc->IsUndoEnabled());
    bool bRename = pNewTabName && !pNewTabName->isEmpty();

    bool bNewDoc = (nDestDocNo == SC_DOC_NEW);
    if ( bNewDoc )
    {
        nDestTab = 0;           // firstly insert

        //  execute without SfxCallMode::RECORD, because already contained in move command

        SfxStringItem aItem( SID_FILE_NAME, "private:factory/" STRING_SCAPP );
        SfxStringItem aTarget( SID_TARGETNAME, OUString("_blank") );

        const SfxPoolItem* pRetItem = GetViewData().GetDispatcher().ExecuteList(
                    SID_OPENDOC, SfxCallMode::API|SfxCallMode::SYNCHRON,
                    { &aItem, &aTarget });
        if ( pRetItem )
        {
            if ( auto pObjectItem = dynamic_cast<const SfxObjectItem*>(pRetItem) )
                pDestShell = dynamic_cast<ScDocShell*>( pObjectItem->GetShell()  );
            else if ( auto pViewFrameItem = dynamic_cast<const SfxViewFrameItem*>( pRetItem) )
            {
                SfxViewFrame* pFrm = pViewFrameItem->GetFrame();
                if (pFrm)
                    pDestShell = dynamic_cast<ScDocShell*>( pFrm->GetObjectShell()  );
            }
            if (pDestShell)
                pDestViewSh = pDestShell->GetBestViewShell();
        }
    }
    else
        pDestShell = ScDocShell::GetShellByNum( nDestDocNo );

    if (!pDestShell)
    {
        OSL_FAIL("Destination document not found !!!");
        return;
    }

    ScMarkData& rMark = GetViewData().GetMarkData();
    if (bRename && rMark.GetSelectCount() != 1)
    {
        // Custom sheet name is provided, but more than one sheet is selected.
        // We don't support this scenario at the moment.
        return;
    }

    pDestDoc = &pDestShell->GetDocument();

    if (pDestDoc != pDoc)
    {
        if (bNewDoc)
        {
            while (pDestDoc->GetTableCount() > 1)
                pDestDoc->DeleteTab(0);
            pDestDoc->RenameTab( 0, "______42_____" );
        }

        SCTAB       nTabCount   = pDoc->GetTableCount();
        SCTAB       nTabSelCount = rMark.GetSelectCount();

        vector<SCTAB> TheTabs;

        for(SCTAB i=0; i<nTabCount; ++i)
        {
            if(rMark.GetTableSelect(i))
            {
                OUString aTabName;
                pDoc->GetName( i, aTabName);
                TheTabs.push_back(i);
                for(SCTAB j=i+1;j<nTabCount;j++)
                {
                    if((!pDoc->IsVisible(j)) && pDoc->IsScenario(j))
                    {
                        pDoc->GetName( j, aTabName);
                        TheTabs.push_back(j);
                        i=j;
                    }
                    else break;
                }
            }
        }

        GetFrameWin()->EnterWait();

        if (pDoc->GetDrawLayer())
            pDestShell->MakeDrawLayer();

        if (!bNewDoc && bUndo)
            pDestDoc->BeginDrawUndo();      // drawing layer must do its own undo actions

        sal_uLong nErrVal =1;
        if(nDestTab==SC_TAB_APPEND)
            nDestTab=pDestDoc->GetTableCount();
        SCTAB nDestTab1=nDestTab;
        ScClipParam aParam;
        for( size_t j=0; j<TheTabs.size(); ++j, ++nDestTab1 )
        {   // insert sheets first and update all references
            OUString aName;
            if (bRename)
                aName = *pNewTabName;
            else
                pDoc->GetName( TheTabs[j], aName );

            pDestDoc->CreateValidTabName( aName );
            if ( !pDestDoc->InsertTab( nDestTab1, aName ) )
            {
                nErrVal = 0;        // total error
                break;  // for
            }
            ScRange aRange( 0, 0, TheTabs[j], MAXCOL, MAXROW, TheTabs[j] );
            aParam.maRanges.push_back(aRange);
        }
        pDoc->SetClipParam(aParam);
        if ( nErrVal > 0 )
        {
            nDestTab1 = nDestTab;
            for(SCTAB nTab : TheTabs)
            {
                nErrVal = pDestShell->TransferTab( *pDocShell, nTab, nDestTab1, false, false );
                nDestTab1++;
            }
        }
        OUString sName;
        if (!bNewDoc && bUndo)
        {
            pDestDoc->GetName(nDestTab, sName);
            pDestShell->GetUndoManager()->AddUndoAction(
                            std::make_unique<ScUndoImportTab>( pDestShell, nDestTab,
                                static_cast<SCTAB>(TheTabs.size())));

        }
        else
        {
            pDestShell->GetUndoManager()->Clear();
        }

        GetFrameWin()->LeaveWait();
        switch (nErrVal)
        {
            case 0:                     // internal error or full of errors
            {
                ErrorMessage(STR_TABINSERT_ERROR);
                return;
            }
            case 2:
                ErrorMessage(STR_ABSREFLOST);
            break;
            case 3:
                ErrorMessage(STR_NAMECONFLICT);
            break;
            case 4:
            {
                ErrorMessage(STR_ABSREFLOST);
                ErrorMessage(STR_NAMECONFLICT);
            }
            break;
            default:
            break;
        }

        if (!bCopy)
        {
            if(nTabCount!=nTabSelCount)
                DeleteTables(TheTabs); // incl. Paint & Undo
            else
                ErrorMessage(STR_TABREMOVE_ERROR);
        }

        if (bNewDoc)
        {
            //  ChartListenerCollection must be updated before DeleteTab
            if ( pDestDoc->IsChartListenerCollectionNeedsUpdate() )
                pDestDoc->UpdateChartListenerCollection();

            SCTAB nNumTabsInserted = static_cast<SCTAB>(TheTabs.size());
            pDestShell->Broadcast( ScTablesHint( SC_TABS_INSERTED, 0, nNumTabsInserted ) );

            pDestDoc->DeleteTab( nNumTabsInserted );   // old first table
            pDestShell->Broadcast( ScTablesHint( SC_TAB_DELETED, nNumTabsInserted ) );

            if (pDestViewSh)
            {
                // Make sure to clear the cached page view after sheet
                // deletion, which still points to the sdr page belonging to
                // the deleted sheet.
                SdrView* pSdrView = pDestViewSh->GetSdrView();
                if (pSdrView)
                    pSdrView->ClearPageView();

                pDestViewSh->TabChanged();      // pages on the drawing layer
            }
            pDestShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                    PaintPartFlags::Grid | PaintPartFlags::Top | PaintPartFlags::Left |
                                    PaintPartFlags::Extras | PaintPartFlags::Size );
            //  PaintPartFlags::Size for outline
        }
        else
        {
            pDestShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, nDestTab ) );
            pDestShell->PostPaintExtras();
            pDestShell->PostPaintGridAll();
        }

        TheTabs.clear();

        pDestShell->SetDocumentModified();
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
    }
    else
    {
        // Move or copy within the same document.
        SCTAB       nTabCount   = pDoc->GetTableCount();

        unique_ptr< vector<SCTAB> >    pSrcTabs(new vector<SCTAB>);
        unique_ptr< vector<SCTAB> >    pDestTabs(new vector<SCTAB>);
        unique_ptr< vector<OUString> > pTabNames(new vector<OUString>);
        unique_ptr< vector<OUString> > pDestNames;
        pSrcTabs->reserve(nTabCount);
        pDestTabs->reserve(nTabCount);
        pTabNames->reserve(nTabCount);
        OUString aDestName;

        for(SCTAB i=0;i<nTabCount;i++)
        {
            if(rMark.GetTableSelect(i))
            {
                OUString aTabName;
                pDoc->GetName( i, aTabName);
                pTabNames->push_back(aTabName);

                for(SCTAB j=i+1;j<nTabCount;j++)
                {
                    if((!pDoc->IsVisible(j)) && pDoc->IsScenario(j))
                    {
                        pDoc->GetName( j, aTabName);
                        pTabNames->push_back(aTabName);
                        i=j;
                    }
                    else break;
                }
            }
        }

        if (bCopy && bUndo)
            pDoc->BeginDrawUndo();          // drawing layer must do its own undo actions

        pDoc->GetName( nDestTab, aDestName);
        SCTAB nDestTab1=nDestTab;
        SCTAB nMovTab=0;
        for (size_t j = 0, n = pTabNames->size(); j < n; ++j)
        {
            nTabCount   = pDoc->GetTableCount();
            const OUString& rStr = (*pTabNames)[j];
            if(!pDoc->GetTable(rStr,nMovTab))
            {
                nMovTab=nTabCount;
            }
            if(!pDoc->GetTable(aDestName,nDestTab1))
            {
                nDestTab1=nTabCount;
            }
            pDocShell->MoveTable( nMovTab, nDestTab1, bCopy, false );   // Undo is here

            if(bCopy && pDoc->IsScenario(nMovTab))
            {
                OUString aComment;
                Color  aColor;
                ScScenarioFlags nFlags;

                pDoc->GetScenarioData(nMovTab, aComment,aColor, nFlags);
                pDoc->SetScenario(nDestTab1,true);
                pDoc->SetScenarioData(nDestTab1,aComment,aColor,nFlags);
                bool bActive = pDoc->IsActiveScenario(nMovTab );
                pDoc->SetActiveScenario( nDestTab1, bActive );
                bool bVisible=pDoc->IsVisible(nMovTab);
                pDoc->SetVisible(nDestTab1,bVisible );
            }

            pSrcTabs->push_back(nMovTab);

            if(!bCopy)
            {
                if(!pDoc->GetTable(rStr,nDestTab1))
                {
                    nDestTab1=nTabCount;
                }
            }

            pDestTabs->push_back(nDestTab1);
        }

        // Rename must be done after all sheets have been moved.
        if (bRename)
        {
            pDestNames.reset(new vector<OUString>);
            size_t n = pDestTabs->size();
            pDestNames->reserve(n);
            for (size_t j = 0; j < n; ++j)
            {
                SCTAB nRenameTab = (*pDestTabs)[j];
                OUString aTabName = *pNewTabName;
                pDoc->CreateValidTabName( aTabName );
                pDestNames->push_back(aTabName);
                pDoc->RenameTab(nRenameTab, aTabName);
            }
        }
        else
            // No need to keep this around when we are not renaming.
            pTabNames.reset();

        SCTAB nTab = GetViewData().GetTabNo();

        if (bUndo)
        {
            if (bCopy)
            {
                pDocShell->GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoCopyTab>(
                            pDocShell, std::move(pSrcTabs), std::move(pDestTabs), std::move(pDestNames)));
            }
            else
            {
                pDocShell->GetUndoManager()->AddUndoAction(
                        std::make_unique<ScUndoMoveTab>(
                            pDocShell, std::move(pSrcTabs), std::move(pDestTabs), std::move(pTabNames), std::move(pDestNames)));
            }
        }

        SCTAB nNewTab = nDestTab;
        if (nNewTab == SC_TAB_APPEND)
            nNewTab = pDoc->GetTableCount()-1;
        else if (!bCopy && nTab<nDestTab)
            nNewTab--;

        SetTabNo( nNewTab, true );

        //#i29848# adjust references to data on the copied sheet
        if( bCopy )
            ScChartHelper::AdjustRangesOfChartsOnDestinationPage( pDoc, pDestDoc, nTab, nNewTab );
    }
}

void ScViewFunc::ShowTable( const std::vector<OUString>& rNames )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());

    std::vector<SCTAB> undoTabs;
    SCTAB nPos = 0;

    bool bFound(false);

    for (const OUString& aName : rNames)
    {
        if (rDoc.GetTable(aName, nPos))
        {
            rDoc.SetVisible( nPos, true );
            SetTabNo( nPos, true );
            SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
            if (!bFound)
                bFound = true;
            if (bUndo)
                undoTabs.push_back(nPos);
        }
    }
    if (bFound)
    {
        if (bUndo)
        {
            pDocSh->GetUndoManager()->AddUndoAction( std::make_unique<ScUndoShowHideTab>( pDocSh, undoTabs, true ) );
        }
        pDocSh->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Extras);
        pDocSh->SetDocumentModified();
    }
}

void ScViewFunc::HideTable( const ScMarkData& rMark )
{
    ScDocShell* pDocSh = GetViewData().GetDocShell();
    ScDocument& rDoc = pDocSh->GetDocument();
    bool bUndo(rDoc.IsUndoEnabled());
    SCTAB nVisible = 0;
    SCTAB nTabCount = rDoc.GetTableCount();

    SCTAB nTabSelCount = rMark.GetSelectCount();

    // check to make sure we won't hide all sheets. we need at least one visible at all times.
    for ( SCTAB i=0; i < nTabCount && nVisible <= nTabSelCount ; i++ )
        if (rDoc.IsVisible(i))
            ++nVisible;

    if (nVisible > nTabSelCount)
    {
        std::vector<SCTAB> undoTabs;

        // need to take a copy of selectedtabs since it is modified in the loop
        const ScMarkData::MarkedTabsType selectedTabs = rMark.GetSelectedTabs();
        for (const SCTAB& nTab : selectedTabs)
        {
            if (rDoc.IsVisible( nTab ))
            {
                rDoc.SetVisible( nTab, false );
                // Update views
                pDocSh->Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );
                SetTabNo( nTab, true );
                // Store for undo
                if (bUndo)
                    undoTabs.push_back(nTab);
            }
        }
        if (bUndo)
        {
            pDocSh->GetUndoManager()->AddUndoAction( std::make_unique<ScUndoShowHideTab>( pDocSh, undoTabs, false ) );
        }

        //  Update views
        SfxGetpApp()->Broadcast( SfxHint( SfxHintId::ScTablesChanged ) );
        pDocSh->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PaintPartFlags::Extras);
        pDocSh->SetDocumentModified();
    }
}

void ScViewFunc::InsertSpecialChar( const OUString& rStr, const vcl::Font& rFont )
{
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    const sal_Unicode* pChar    = rStr.getStr();
    ScTabViewShell* pViewShell  = GetViewData().GetViewShell();
    SvxFontItem     aFontItem( rFont.GetFamilyType(),
                               rFont.GetFamilyName(),
                               rFont.GetStyleName(),
                               rFont.GetPitch(),
                               rFont.GetCharSet(),
                               ATTR_FONT );

    //  if string contains WEAK characters, set all fonts
    SvtScriptType nScript;
    ScDocument* pDoc = GetViewData().GetDocument();
    if ( pDoc->HasStringWeakCharacters( rStr ) )
        nScript = SvtScriptType::LATIN | SvtScriptType::ASIAN | SvtScriptType::COMPLEX;
    else
        nScript = pDoc->GetStringScriptType( rStr );

    SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, pViewShell->GetPool() );
    aSetItem.PutItemForScriptType( nScript, aFontItem );
    ApplyUserItemSet( aSetItem.GetItemSet() );

    while ( *pChar )
        pViewShell->TabKeyInput( KeyEvent( *(pChar++), vcl::KeyCode() ) );
}

void ScViewFunc::UpdateLineAttrs( SvxBorderLine&       rLine,
                                  const SvxBorderLine* pDestLine,
                                  const SvxBorderLine* pSrcLine,
                                  bool                 bColor )
{
    if ( pSrcLine && pDestLine )
    {
        if ( bColor )
        {
            rLine.SetColor      ( pSrcLine->GetColor() );
            rLine.SetBorderLineStyle(pDestLine->GetBorderLineStyle());
            rLine.SetWidth      ( pDestLine->GetWidth() );
        }
        else
        {
            rLine.SetColor      ( pDestLine->GetColor() );
            rLine.SetBorderLineStyle(pSrcLine->GetBorderLineStyle());
            rLine.SetWidth      ( pSrcLine->GetWidth() );
        }
    }
}

#define SET_LINE_ATTRIBUTES(LINE,BOXLINE) \
    pBoxLine = aBoxItem.Get##LINE();                                \
    if ( pBoxLine )                                                 \
    {                                                               \
        if ( pLine )                                                \
        {                                                           \
            UpdateLineAttrs( aLine, pBoxLine, pLine, bColorOnly );  \
            aBoxItem.SetLine( &aLine, BOXLINE );                    \
        }                                                           \
        else                                                        \
            aBoxItem.SetLine( nullptr, BOXLINE );                      \
    }

void ScViewFunc::SetSelectionFrameLines( const SvxBorderLine* pLine,
                                         bool bColorOnly )
{
    // Not editable only due to a matrix? Attribute is ok anyhow.
    bool bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocument*             pDoc = GetViewData().GetDocument();
    ScMarkData aFuncMark( GetViewData().GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
    ScDocShell*             pDocSh = GetViewData().GetDocShell();
    const ScPatternAttr*    pSelAttrs = GetSelectionPattern();
    const SfxItemSet&       rSelItemSet = pSelAttrs->GetItemSet();

    const SfxPoolItem*      pBorderAttr = nullptr;
    SfxItemState            eItemState = rSelItemSet.GetItemState( ATTR_BORDER, true, &pBorderAttr );

    const SfxPoolItem*      pTLBRItem = nullptr;
    SfxItemState            eTLBRState = rSelItemSet.GetItemState( ATTR_BORDER_TLBR, true, &pTLBRItem );

    const SfxPoolItem*      pBLTRItem = nullptr;
    SfxItemState            eBLTRState = rSelItemSet.GetItemState( ATTR_BORDER_BLTR, true, &pBLTRItem );

    // any of the lines visible?
    if( (eItemState != SfxItemState::DEFAULT) || (eTLBRState != SfxItemState::DEFAULT) || (eBLTRState != SfxItemState::DEFAULT) )
    {
        // none of the lines don't care?
        if( (eItemState != SfxItemState::DONTCARE) && (eTLBRState != SfxItemState::DONTCARE) && (eBLTRState != SfxItemState::DONTCARE) )
        {
            std::unique_ptr<SfxItemSet> pOldSet(new SfxItemSet(
                                            *(pDoc->GetPool()),
                                            svl::Items<ATTR_PATTERN_START,
                                            ATTR_PATTERN_END>{} ));
            std::unique_ptr<SfxItemSet> pNewSet(new SfxItemSet(
                                            *(pDoc->GetPool()),
                                            svl::Items<ATTR_PATTERN_START,
                                            ATTR_PATTERN_END>{} ));

            SvxBorderLine           aLine;

            if( pBorderAttr )
            {
                const SvxBorderLine*    pBoxLine = nullptr;
                SvxBoxItem      aBoxItem( *static_cast<const SvxBoxItem*>(pBorderAttr) );
                SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );

                // here pBoxLine is used
                SET_LINE_ATTRIBUTES(Top,SvxBoxItemLine::TOP)
                SET_LINE_ATTRIBUTES(Bottom,SvxBoxItemLine::BOTTOM)
                SET_LINE_ATTRIBUTES(Left,SvxBoxItemLine::LEFT)
                SET_LINE_ATTRIBUTES(Right,SvxBoxItemLine::RIGHT)

                aBoxInfoItem.SetLine( aBoxItem.GetTop(), SvxBoxInfoItemLine::HORI );
                aBoxInfoItem.SetLine( aBoxItem.GetLeft(), SvxBoxInfoItemLine::VERT );
                aBoxInfoItem.ResetFlags(); // set Lines to Valid

                pOldSet->Put( *pBorderAttr );
                pNewSet->Put( aBoxItem );
                pNewSet->Put( aBoxInfoItem );
            }

            if( pTLBRItem && static_cast<const SvxLineItem*>(pTLBRItem)->GetLine() )
            {
                SvxLineItem aTLBRItem( *static_cast<const SvxLineItem*>(pTLBRItem) );
                UpdateLineAttrs( aLine, aTLBRItem.GetLine(), pLine, bColorOnly );
                aTLBRItem.SetLine( &aLine );
                pOldSet->Put( *pTLBRItem );
                pNewSet->Put( aTLBRItem );
            }

            if( pBLTRItem && static_cast<const SvxLineItem*>(pBLTRItem)->GetLine() )
            {
                SvxLineItem aBLTRItem( *static_cast<const SvxLineItem*>(pBLTRItem) );
                UpdateLineAttrs( aLine, aBLTRItem.GetLine(), pLine, bColorOnly );
                aBLTRItem.SetLine( &aLine );
                pOldSet->Put( *pBLTRItem );
                pNewSet->Put( aBLTRItem );
            }

            ApplyAttributes( pNewSet.get(), pOldSet.get() );
        }
        else // if ( eItemState == SfxItemState::DONTCARE )
        {
            aFuncMark.MarkToMulti();
            pDoc->ApplySelectionLineStyle( aFuncMark, pLine, bColorOnly );
        }

        ScRange aMarkRange;
        aFuncMark.GetMultiMarkArea( aMarkRange );
        SCCOL nStartCol = aMarkRange.aStart.Col();
        SCROW nStartRow = aMarkRange.aStart.Row();
        SCTAB nStartTab = aMarkRange.aStart.Tab();
        SCCOL nEndCol = aMarkRange.aEnd.Col();
        SCROW nEndRow = aMarkRange.aEnd.Row();
        SCTAB nEndTab = aMarkRange.aEnd.Tab();
        pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                           nEndCol, nEndRow, nEndTab,
                           PaintPartFlags::Grid, SC_PF_LINES | SC_PF_TESTMERGE );

        pDocSh->UpdateOle( &GetViewData() );
        pDocSh->SetDocumentModified();
    }
}

#undef SET_LINE_ATTRIBUTES

void ScViewFunc::SetValidation( const ScValidationData& rNew )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    sal_uLong nIndex = pDoc->AddValidationEntry(rNew);      // for it there is no Undo
    SfxUInt32Item aItem( ATTR_VALIDDATA, nIndex );

    ApplyAttr( aItem );         // with Paint and Undo...
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
