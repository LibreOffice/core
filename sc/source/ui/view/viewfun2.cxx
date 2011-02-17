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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <sfx2/app.hxx>
#define _SVSTDARR_STRINGS
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
#include <svl/svstdarr.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>
#include <vcl/waitobj.hxx>

#include <basic/sbstar.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>

#include "viewfunc.hxx"

#include "sc.hrc"
#include "globstr.hrc"

#include "attrib.hxx"
#include "autoform.hxx"
#include "cell.hxx"                 // EnterAutoSum
#include "cellmergeoption.hxx"
#include "compiler.hxx"
#include "docfunc.hxx"
#include "docpool.hxx"
#include "docsh.hxx"
#include "global.hxx"
#include "patattr.hxx"
#include "printfun.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "refundo.hxx"
#include "tablink.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "undoblk.hxx"
#include "undocell.hxx"
#include "undotab.hxx"
#include "sizedev.hxx"
#include "editable.hxx"
#include "scmod.hxx"
#include "inputhdl.hxx"
#include "inputwin.hxx"
#include "funcdesc.hxx"
#include "docuno.hxx"
#include "charthelper.hxx"
#include "tabbgcolor.hxx"

#include <basic/sbstar.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <memory>

using namespace com::sun::star;
using ::rtl::OUStringBuffer;
using ::rtl::OUString;

using ::std::vector;
using ::std::auto_ptr;

// helper func defined in docfunc.cxx
void VBA_DeleteModule( ScDocShell& rDocSh, String& sModuleName );

// STATIC DATA ---------------------------------------------------------------


//----------------------------------------------------------------------------

BOOL ScViewFunc::AdjustBlockHeight( BOOL bPaint, ScMarkData* pMarkData )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    if (!pMarkData)
        pMarkData = &GetViewData()->GetMarkData();

    ScDocument* pDoc = pDocSh->GetDocument();
    SCCOLROW* pRanges = new SCCOLROW[MAXCOLROWCOUNT];
    SCCOLROW nRangeCnt = pMarkData->GetMarkRowRanges( pRanges );
    if (nRangeCnt == 0)
    {
        pRanges[0] = pRanges[1] = GetViewData()->GetCurY();
        nRangeCnt = 1;
    }

    double nPPTX = GetViewData()->GetPPTX();
    double nPPTY = GetViewData()->GetPPTY();
    Fraction aZoomX = GetViewData()->GetZoomX();
    Fraction aZoomY = GetViewData()->GetZoomY();

    ScSizeDeviceProvider aProv(pDocSh);
    if (aProv.IsPrinter())
    {
        nPPTX = aProv.GetPPTX();
        nPPTY = aProv.GetPPTY();
        aZoomX = aZoomY = Fraction( 1, 1 );
    }

    BOOL bAnyChanged = FALSE;
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
    {
        if (pMarkData->GetTableSelect(nTab))
        {
            SCCOLROW* pOneRange = pRanges;
            BOOL bChanged = FALSE;
            SCROW nPaintY = 0;
            for (SCROW nRangeNo=0; nRangeNo<nRangeCnt; nRangeNo++)
            {
                SCROW nStartNo = *(pOneRange++);
                SCROW nEndNo = *(pOneRange++);
                if (pDoc->SetOptimalHeight( nStartNo, nEndNo, nTab, 0, aProv.GetDevice(),
                                            nPPTX, nPPTY, aZoomX, aZoomY, FALSE ))
                {
                    if (!bChanged)
                        nPaintY = nStartNo;
                    bAnyChanged = bChanged = TRUE;
                }
            }
            if ( bPaint && bChanged )
                pDocSh->PostPaint( 0, nPaintY, nTab, MAXCOL, MAXROW, nTab,
                                                    PAINT_GRID | PAINT_LEFT );
        }
    }
    delete[] pRanges;

    if ( bPaint && bAnyChanged )
        pDocSh->UpdateOle(GetViewData());

    return bAnyChanged;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, BOOL bPaint )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    double nPPTX = GetViewData()->GetPPTX();
    double nPPTY = GetViewData()->GetPPTY();
    Fraction aZoomX = GetViewData()->GetZoomX();
    Fraction aZoomY = GetViewData()->GetZoomY();
    USHORT nOldPixel = 0;
    if (nStartRow == nEndRow)
        nOldPixel = (USHORT) (pDoc->GetRowHeight(nStartRow,nTab) * nPPTY);

    ScSizeDeviceProvider aProv(pDocSh);
    if (aProv.IsPrinter())
    {
        nPPTX = aProv.GetPPTX();
        nPPTY = aProv.GetPPTY();
        aZoomX = aZoomY = Fraction( 1, 1 );
    }
    BOOL bChanged = pDoc->SetOptimalHeight( nStartRow, nEndRow, nTab, 0, aProv.GetDevice(),
                                            nPPTX, nPPTY, aZoomX, aZoomY, FALSE );

    if (bChanged && ( nStartRow == nEndRow ))
    {
        USHORT nNewPixel = (USHORT) (pDoc->GetRowHeight(nStartRow,nTab) * nPPTY);
        if ( nNewPixel == nOldPixel )
            bChanged = FALSE;
    }

    if ( bPaint && bChanged )
        pDocSh->PostPaint( 0, nStartRow, nTab, MAXCOL, MAXROW, nTab,
                                            PAINT_GRID | PAINT_LEFT );

    return bChanged;
}


//----------------------------------------------------------------------------

enum ScAutoSum
{
    ScAutoSumNone = 0,
    ScAutoSumData,
    ScAutoSumSum
};


ScAutoSum lcl_IsAutoSumData( ScDocument* pDoc, SCCOL nCol, SCROW nRow,
        SCTAB nTab, ScDirection eDir, SCCOLROW& nExtend )
{
    ScBaseCell* pCell;
    pDoc->GetCell( nCol, nRow, nTab, pCell );
    if ( pCell && pCell->HasValueData() )
    {
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
        {
            ScTokenArray* pCode = ((ScFormulaCell*)pCell)->GetCode();
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


//----------------------------------------------------------------------------

#define SC_AUTOSUM_MAXCOUNT     20

ScAutoSum lcl_SeekAutoSumData( ScDocument* pDoc, SCCOL& nCol, SCROW& nRow,
        SCTAB nTab, ScDirection eDir, SCCOLROW& nExtend )
{
    USHORT nCount = 0;
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

//----------------------------------------------------------------------------

bool lcl_FindNextSumEntryInColumn( ScDocument* pDoc, SCCOL nCol, SCROW& nRow,
                                   SCTAB nTab, SCCOLROW& nExtend, SCROW nMinRow )
{
    const SCROW nTmp = nRow;
    ScAutoSum eSkip = ScAutoSumNone;
    while ( ( eSkip = lcl_IsAutoSumData( pDoc, nCol, nRow, nTab, DIR_TOP, nExtend ) ) == ScAutoSumData &&
            nRow > nMinRow )
    {
        --nRow;
    }
    if ( eSkip == ScAutoSumSum && nRow < nTmp )
    {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------

bool lcl_FindNextSumEntryInRow( ScDocument* pDoc, SCCOL& nCol, SCROW nRow,
                                SCTAB nTab, SCCOLROW& nExtend, SCROW nMinCol )
{
    const SCCOL nTmp = nCol;
    ScAutoSum eSkip = ScAutoSumNone;
    while ( ( eSkip = lcl_IsAutoSumData( pDoc, nCol, nRow, nTab, DIR_LEFT, nExtend ) ) == ScAutoSumData &&
            nCol > nMinCol )
    {
        --nCol;
    }
    if ( eSkip == ScAutoSumSum && nCol < nTmp )
    {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------

bool lcl_GetAutoSumForColumnRange( ScDocument* pDoc, ScRangeList& rRangeList, const ScRange& rRange )
{
    const ScAddress aStart = rRange.aStart;
    const ScAddress aEnd = rRange.aEnd;
    if ( aStart.Col() != aEnd.Col() )
    {
        return false;
    }

    const SCTAB nTab = aEnd.Tab();
    const SCCOL nCol = aEnd.Col();
    SCROW nEndRow = aEnd.Row();
    SCROW nStartRow = nEndRow;
    SCCOLROW nExtend = 0;
    const ScAutoSum eSum = lcl_IsAutoSumData( pDoc, nCol, nEndRow, nTab, DIR_TOP, nExtend /*out*/ );

    if ( eSum == ScAutoSumSum )
    {
        bool bContinue = false;
        do
        {
            rRangeList.Append( ScRange( nCol, nStartRow, nTab, nCol, nEndRow, nTab ) );
            nEndRow = static_cast< SCROW >( nExtend );
            if ( ( bContinue = lcl_FindNextSumEntryInColumn( pDoc, nCol, nEndRow /*inout*/, nTab, nExtend /*out*/, aStart.Row() ) ) == true )
            {
                nStartRow = nEndRow;
            }
        } while ( bContinue );
    }
    else
    {
        while ( nStartRow > aStart.Row() &&
                lcl_IsAutoSumData( pDoc, nCol, nStartRow-1, nTab, DIR_TOP, nExtend /*out*/ ) != ScAutoSumSum )
        {
            --nStartRow;
        }
        rRangeList.Append( ScRange( nCol, nStartRow, nTab, nCol, nEndRow, nTab ) );
    }

    return true;
}

//----------------------------------------------------------------------------

bool lcl_GetAutoSumForRowRange( ScDocument* pDoc, ScRangeList& rRangeList, const ScRange& rRange )
{
    const ScAddress aStart = rRange.aStart;
    const ScAddress aEnd = rRange.aEnd;
    if ( aStart.Row() != aEnd.Row() )
    {
        return false;
    }

    const SCTAB nTab = aEnd.Tab();
    const SCROW nRow = aEnd.Row();
    SCCOL nEndCol = aEnd.Col();
    SCCOL nStartCol = nEndCol;
    SCCOLROW nExtend = 0;
    const ScAutoSum eSum = lcl_IsAutoSumData( pDoc, nEndCol, nRow, nTab, DIR_LEFT, nExtend /*out*/ );

    if ( eSum == ScAutoSumSum )
    {
        bool bContinue = false;
        do
        {
            rRangeList.Append( ScRange( nStartCol, nRow, nTab, nEndCol, nRow, nTab ) );
            nEndCol = static_cast< SCCOL >( nExtend );
            if ( ( bContinue = lcl_FindNextSumEntryInRow( pDoc, nEndCol /*inout*/, nRow, nTab, nExtend /*out*/, aStart.Col() ) ) == true )
            {
                nStartCol = nEndCol;
            }
        } while ( bContinue );
    }
    else
    {
        while ( nStartCol > aStart.Col() &&
                lcl_IsAutoSumData( pDoc, nStartCol-1, nRow, nTab, DIR_LEFT, nExtend /*out*/ ) != ScAutoSumSum )
        {
            --nStartCol;
        }
        rRangeList.Append( ScRange( nStartCol, nRow, nTab, nEndCol, nRow, nTab ) );
    }

    return true;
}

//----------------------------------------------------------------------------

BOOL ScViewFunc::GetAutoSumArea( ScRangeList& rRangeList )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();

    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();

    SCCOL nStartCol = nCol;
    SCROW nStartRow = nRow;
    SCCOL nEndCol    = nCol;
    SCROW nEndRow    = nRow;
    SCCOL nSeekCol   = nCol;
    SCROW nSeekRow   = nRow;
    SCCOLROW nExtend;       // wird per Reference gueltig bei ScAutoSumSum

    BOOL bCol = FALSE;
    BOOL bRow = FALSE;

    ScAutoSum eSum;
    if ( nRow != 0
            && ((eSum = lcl_IsAutoSumData( pDoc, nCol, nRow-1, nTab,
                DIR_TOP, nExtend /*out*/ )) == ScAutoSumData )
            && ((eSum = lcl_IsAutoSumData( pDoc, nCol, nRow-1, nTab,
                DIR_LEFT, nExtend /*out*/ )) == ScAutoSumData )
        )
    {
        bRow = TRUE;
        nSeekRow = nRow - 1;
    }
    else if ( nCol != 0 && (eSum = lcl_IsAutoSumData( pDoc, nCol-1, nRow, nTab,
            DIR_LEFT, nExtend /*out*/ )) == ScAutoSumData )
    {
        bCol = TRUE;
        nSeekCol = nCol - 1;
    }
    else if ( (eSum = lcl_SeekAutoSumData( pDoc, nCol, nSeekRow, nTab, DIR_TOP, nExtend /*out*/ )) != ScAutoSumNone )
        bRow = TRUE;
    else if (( eSum = lcl_SeekAutoSumData( pDoc, nSeekCol, nRow, nTab, DIR_LEFT, nExtend /*out*/ )) != ScAutoSumNone )
        bCol = TRUE;

    if ( bCol || bRow )
    {
        if ( bRow )
        {
            nStartRow = nSeekRow;       // nSeekRow evtl. per Reference angepasst
            if ( eSum == ScAutoSumSum )
                nEndRow = nStartRow;        // nur Summen summieren
            else
                nEndRow = nRow - 1;     // Datenbereich evtl. nach unten erweitern
        }
        else
        {
            nStartCol = nSeekCol;       // nSeekCol evtl. per Reference angepasst
            if ( eSum == ScAutoSumSum )
                nEndCol = nStartCol;        // nur Summen summieren
            else
                nEndCol = nCol - 1;     // Datenbereich evtl. nach rechts erweitern
        }
        BOOL bContinue = FALSE;
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
            rRangeList.Append(
                ScRange( nStartCol, nStartRow, nTab, nEndCol, nEndRow, nTab ) );
            if ( eSum == ScAutoSumSum )
            {
                if ( bRow )
                {
                    nEndRow = static_cast< SCROW >( nExtend );
                    if ( ( bContinue = lcl_FindNextSumEntryInColumn( pDoc, nCol, nEndRow /*inout*/, nTab, nExtend /*out*/, 0 ) ) == true )
                    {
                        nStartRow = nEndRow;
                    }
                }
                else
                {
                    nEndCol = static_cast< SCCOL >( nExtend );
                    if ( ( bContinue = lcl_FindNextSumEntryInRow( pDoc, nEndCol /*inout*/, nRow, nTab, nExtend /*out*/, 0 ) ) == true )
                    {
                        nStartCol = nEndCol;
                    }
                }
            }
        } while ( bContinue );
        return TRUE;
    }
    return FALSE;
}

//----------------------------------------------------------------------------

void ScViewFunc::EnterAutoSum(const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr)
{
    String aFormula = GetAutoSumFormula( rRangeList, bSubTotal, rAddr );
    EnterBlock( aFormula, NULL );
}

//----------------------------------------------------------------------------

bool ScViewFunc::AutoSum( const ScRange& rRange, bool bSubTotal, bool bSetCursor, bool bContinue )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
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
    bool bRow = ( ( nStartRow != nEndRow ) && ( bEndRowEmpty || ( !bEndRowEmpty && !bEndColEmpty ) ) );
    bool bCol = ( ( nStartCol != nEndCol ) && ( bEndColEmpty || nStartRow == nEndRow ) );

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
                const ScRange aRange( nCol, nStartRow, nTab, nCol, nSumEndRow, nTab );
                if ( lcl_GetAutoSumForColumnRange( pDoc, aRangeList, aRange ) )
                {
                    const String aFormula = GetAutoSumFormula(
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
                const ScRange aRange( nStartCol, nRow, nTab, nSumEndCol, nRow, nTab );
                if ( lcl_GetAutoSumForRowRange( pDoc, aRangeList, aRange ) )
                {
                    const String aFormula = GetAutoSumFormula( aRangeList, bSubTotal, ScAddress(nInsCol, nRow, nTab) );
                    EnterData( nInsCol, nRow, nTab, aFormula );
                }
            }
        }
    }

    // set new mark range and cursor position
    const ScRange aMarkRange( nStartCol, nStartRow, nTab, nMarkEndCol, nMarkEndRow, nTab );
    MarkRange( aMarkRange, FALSE, bContinue );
    if ( bSetCursor )
    {
        SetCursor( nMarkEndCol, nMarkEndRow );
    }

    return true;
}

//----------------------------------------------------------------------------

String ScViewFunc::GetAutoSumFormula( const ScRangeList& rRangeList, bool bSubTotal, const ScAddress& rAddr )
{
    ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();
    ::boost::scoped_ptr<ScTokenArray> pArray(new ScTokenArray);

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
        const ScRange* pFirst = aRangeList.front();
        size_t ListSize = aRangeList.size();
        for ( size_t i = 0; i < ListSize; ++i )
        {
            const ScRange* p = aRangeList[i];
            if (p != pFirst)
                pArray->AddOpCode(ocSep);
            ScComplexRefData aRef;
            aRef.InitRangeRel(*p, rAddr);
            pArray->AddDoubleReference(aRef);
        }
    }

    pArray->AddOpCode(ocClose);

    ScCompiler aComp(pDoc, rAddr, *pArray);
    aComp.SetGrammar(pDoc->GetGrammar());
    OUStringBuffer aBuf;
    aComp.CreateStringFromTokenArray(aBuf);
    OUString aFormula = aBuf.makeStringAndClear();
    aBuf.append(sal_Unicode('='));
    aBuf.append(aFormula);
    return aBuf.makeStringAndClear();
}

//----------------------------------------------------------------------------

void ScViewFunc::EnterBlock( const String& rString, const EditTextObject* pData )
{
    //  Mehrfachselektion vorher abfragen...

    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( rMark.IsMultiMarked() )
    {
        rMark.MarkToSimple();
        if ( rMark.IsMultiMarked() )
        {       // "Einfuegen auf Mehrfachselektion nicht moeglich"
            ErrorMessage(STR_MSSG_PASTEFROMCLIP_0);

            //  insert into single cell
            if ( pData )
                EnterData( nCol, nRow, nTab, pData );
            else
                EnterData( nCol, nRow, nTab, rString );
            return;
        }
    }

    ScDocument* pDoc = GetViewData()->GetDocument();
    String aNewStr = rString;
    if ( pData )
    {
        const ScPatternAttr* pOldPattern = pDoc->GetPattern( nCol, nRow, nTab );
        ScTabEditEngine aEngine( *pOldPattern, pDoc->GetEnginePool() );
        aEngine.SetText(*pData);

        ScEditAttrTester aTester( &aEngine );
        if (!aTester.NeedsObject())
        {
            aNewStr = aEngine.GetText();
            pData = NULL;
        }
    }

    //  Einfuegen per PasteFromClip

    WaitObject aWait( GetFrameWin() );

    ScAddress aPos( nCol, nRow, nTab );

    ScDocument* pInsDoc = new ScDocument( SCDOCMODE_CLIP );
    pInsDoc->ResetClip( pDoc, nTab );

    if (aNewStr.GetChar(0) == '=')                      // Formel ?
    {
        //  SetString geht nicht, weil in Clipboard-Dokumenten nicht kompiliert wird!
        ScFormulaCell* pFCell = new ScFormulaCell( pDoc, aPos, aNewStr );
        pInsDoc->PutCell( nCol, nRow, nTab, pFCell );
    }
    else if ( pData )
        pInsDoc->PutCell( nCol, nRow, nTab, new ScEditCell( pData, pDoc, NULL ) );
    else
        pInsDoc->SetString( nCol, nRow, nTab, aNewStr );

    pInsDoc->SetClipArea( ScRange(aPos) );
    // auf Block einfuegen, mit Undo etc.
    if ( PasteFromClip( IDF_CONTENTS, pInsDoc, PASTE_NOFUNC, FALSE, FALSE,
            FALSE, INS_NONE, IDF_ATTRIB ) )
    {
        const SfxUInt32Item* pItem = (SfxUInt32Item*) pInsDoc->GetAttr(
            nCol, nRow, nTab, ATTR_VALUE_FORMAT );
        if ( pItem )
        {   // Numberformat setzen wenn inkompatibel
            // MarkData wurde bereits in PasteFromClip MarkToSimple'ed
            ScRange aRange;
            rMark.GetMarkArea( aRange );
            ScPatternAttr* pPattern = new ScPatternAttr( pDoc->GetPool() );
            pPattern->GetItemSet().Put( *pItem );
            short nNewType = pDoc->GetFormatTable()->GetType( pItem->GetValue() );
            pDoc->ApplyPatternIfNumberformatIncompatible( aRange, rMark,
                *pPattern, nNewType );
            delete pPattern;
        }
    }

    delete pInsDoc;
}


//----------------------------------------------------------------------------
//  manueller Seitenumbruch

void ScViewFunc::InsertPageBreak( BOOL bColumn, BOOL bRecord, const ScAddress* pPos,
                                    BOOL bSetModified )
{
    SCTAB nTab = GetViewData()->GetTabNo();
    ScAddress aCursor;
    if (pPos)
        aCursor = *pPos;
    else
        aCursor = ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );

    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        InsertPageBreak( bColumn, aCursor, bRecord, bSetModified, FALSE );

    if ( bSuccess && bSetModified )
        UpdatePageBreakData( TRUE );    // fuer PageBreak-Modus
}


//----------------------------------------------------------------------------

void ScViewFunc::DeletePageBreak( BOOL bColumn, BOOL bRecord, const ScAddress* pPos,
                                    BOOL bSetModified )
{
    SCTAB nTab = GetViewData()->GetTabNo();
    ScAddress aCursor;
    if (pPos)
        aCursor = *pPos;
    else
        aCursor = ScAddress( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );

    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        RemovePageBreak( bColumn, aCursor, bRecord, bSetModified, FALSE );

    if ( bSuccess && bSetModified )
        UpdatePageBreakData( TRUE );    // fuer PageBreak-Modus
}

//----------------------------------------------------------------------------

void ScViewFunc::RemoveManualBreaks()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTab = GetViewData()->GetTabNo();
    BOOL bUndo(pDoc->IsUndoEnabled());

    if (bUndo)
    {
        ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab, TRUE, TRUE );
        pDoc->CopyToDocument( 0,0,nTab, MAXCOL,MAXROW,nTab, IDF_NONE, FALSE, pUndoDoc );
        pDocSh->GetUndoManager()->AddUndoAction(
                                new ScUndoRemoveBreaks( pDocSh, nTab, pUndoDoc ) );
    }

    pDoc->RemoveManualBreaks(nTab);
    pDoc->UpdatePageBreaks(nTab);

    UpdatePageBreakData( TRUE );
    pDocSh->SetDocumentModified();
    pDocSh->PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
}

//----------------------------------------------------------------------------

void ScViewFunc::SetPrintZoom(USHORT nScale, USHORT nPages)
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    SCTAB nTab = GetViewData()->GetTabNo();
    pDocSh->SetPrintZoom( nTab, nScale, nPages );
}

void ScViewFunc::AdjustPrintZoom()
{
    ScRange aRange;
    if ( GetViewData()->GetSimpleArea( aRange ) != SC_MARK_SIMPLE )
        GetViewData()->GetMarkData().GetMultiMarkArea( aRange );
    GetViewData()->GetDocShell()->AdjustPrintZoom( aRange );
}

//----------------------------------------------------------------------------

void ScViewFunc::SetPrintRanges( BOOL bEntireSheet, const String* pPrint,
                                const String* pRepCol, const String* pRepRow,
                                BOOL bAddPrint )
{
    //  on all selected tables

    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    ScMarkData& rMark   = GetViewData()->GetMarkData();
    SCTAB nTab;
    BOOL bUndo (pDoc->IsUndoEnabled());

    ScPrintRangeSaver* pOldRanges = pDoc->CreatePrintRangeSaver();

    ScAddress::Details aDetails(pDoc->GetAddressConvention(), 0, 0);

    for (nTab=0; nTab<nTabCount; nTab++)
        if (rMark.GetTableSelect(nTab))
        {
            ScRange aRange( 0,0,nTab );

            //  print ranges

            if( !bAddPrint )
                pDoc->ClearPrintRanges( nTab );

            if( bEntireSheet )
            {
                pDoc->SetPrintEntireSheet( nTab );
            }
            else if ( pPrint )
            {
                if ( pPrint->Len() )
                {
                    const sal_Unicode sep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
                    USHORT nTCount = pPrint->GetTokenCount(sep);
                    for (USHORT i=0; i<nTCount; i++)
                    {
                        String aToken = pPrint->GetToken(i, sep);
                        if ( aRange.ParseAny( aToken, pDoc, aDetails ) & SCA_VALID )
                            pDoc->AddPrintRange( nTab, aRange );
                    }
                }
            }
            else    // NULL = use selection (print range is always set), use empty string to delete all ranges
            {
                if ( GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE )
                {
                    pDoc->AddPrintRange( nTab, aRange );
                }
                else if ( rMark.IsMultiMarked() )
                {
                    rMark.MarkToMulti();
                    ScRangeListRef pList( new ScRangeList );
                    rMark.FillRangeListWithMarks( pList, FALSE );
                    for (size_t i = 0, n = pList->size(); i < n; ++i)
                    {
                        ScRange* pR = (*pList)[i];
                        pDoc->AddPrintRange(nTab, *pR);
                    }
                }
            }

            //  repeat columns

            if ( pRepCol )
            {
                if ( !pRepCol->Len() )
                    pDoc->SetRepeatColRange( nTab, NULL );
                else
                    if ( aRange.ParseAny( *pRepCol, pDoc, aDetails ) & SCA_VALID )
                        pDoc->SetRepeatColRange( nTab, &aRange );
            }

            //  repeat rows

            if ( pRepRow )
            {
                if ( !pRepRow->Len() )
                    pDoc->SetRepeatRowRange( nTab, NULL );
                else
                    if ( aRange.ParseAny( *pRepRow, pDoc, aDetails ) & SCA_VALID )
                        pDoc->SetRepeatRowRange( nTab, &aRange );
            }
        }

    //  undo (for all tables)
    if (bUndo)
    {
        SCTAB nCurTab = GetViewData()->GetTabNo();
        ScPrintRangeSaver* pNewRanges = pDoc->CreatePrintRangeSaver();
        pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoPrintRange( pDocSh, nCurTab, pOldRanges, pNewRanges ) );
    }

    //  update page breaks

    for (nTab=0; nTab<nTabCount; nTab++)
        if (rMark.GetTableSelect(nTab))
            ScPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab ).UpdatePages();

    SfxBindings& rBindings = GetViewData()->GetBindings();
    rBindings.Invalidate( SID_DELETE_PRINTAREA );

    pDocSh->SetDocumentModified();
}

//----------------------------------------------------------------------------
//  Zellen zusammenfassen

BOOL ScViewFunc::TestMergeCells()           // Vorab-Test (fuer Menue)
{
    //  simple test: TRUE if there's a selection but no multi selection and not filtered

    const ScMarkData& rMark = GetViewData()->GetMarkData();
    if ( rMark.IsMarked() || rMark.IsMultiMarked() )
    {
        ScRange aDummy;
        return GetViewData()->GetSimpleArea( aDummy) == SC_MARK_SIMPLE;
    }
    else
        return FALSE;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::MergeCells( BOOL bApi, BOOL& rDoContents, BOOL bRecord, BOOL bCenter )
{
    //  Editable- und Verschachtelungs-Abfrage muss vorneweg sein (auch in DocFunc),
    //  damit dann nicht die Inhalte-QueryBox kommt
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return FALSE;
    }

    ScMarkData& rMark = GetViewData()->GetMarkData();
    rMark.MarkToSimple();
    if (!rMark.IsMarked())
    {
        ErrorMessage(STR_NOMULTISELECT);
        return FALSE;
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();

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
        // nichts zu tun
        return TRUE;
    }

    if ( pDoc->HasAttrib( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                            HASATTR_MERGED | HASATTR_OVERLAPPED ) )
    {       // "Zusammenfassen nicht verschachteln !"
        ErrorMessage(STR_MSSG_MERGECELLS_0);
        return FALSE;
    }

    // Check for the contents of all selected tables.
    bool bAskDialog = false;
    SCTAB nTabCount = pDoc->GetTableCount();
    ScCellMergeOption aMergeOption(nStartCol, nStartRow, nEndCol, nEndRow, bCenter);
    for (SCTAB i = 0; i < nTabCount; ++i)
    {
        if (!rMark.GetTableSelect(i))
            // this table is not selected.
            continue;

        aMergeOption.maTabs.insert(i);

        if (!pDoc->IsBlockEmpty(i, nStartCol, nStartRow+1, nStartCol, nEndRow) ||
            !pDoc->IsBlockEmpty(i, nStartCol+1, nStartRow, nEndCol, nEndRow))
            bAskDialog = true;
    }

    BOOL bOk = TRUE;

    if (bAskDialog)
    {
        if (!bApi)
        {
            MessBox aBox( GetViewData()->GetDialogParent(),
                            WinBits(WB_YES_NO_CANCEL | WB_DEF_NO),
                            ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                            ScGlobal::GetRscString( STR_MERGE_NOTEMPTY ) );
            USHORT nRetVal = aBox.Execute();

            if ( nRetVal == RET_YES )
                rDoContents = TRUE;
            else if ( nRetVal == RET_CANCEL )
                bOk = FALSE;
        }
    }

    if (bOk)
    {
        HideCursor();
        bOk = pDocSh->GetDocFunc().MergeCells( aMergeOption, rDoContents, bRecord, bApi );
        ShowCursor();

        if (bOk)
        {
            SetCursor( nStartCol, nStartRow );
            //DoneBlockMode( FALSE);
            Unmark();

            pDocSh->UpdateOle(GetViewData());
            UpdateInputLine();
        }
    }

    return bOk;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::TestRemoveMerge()
{
    BOOL bMerged = FALSE;
    ScRange aRange;
    if (GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        if ( pDoc->HasAttrib( aRange, HASATTR_MERGED ) )
            bMerged = TRUE;
    }
    return bMerged;
}


//----------------------------------------------------------------------------

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

BOOL ScViewFunc::RemoveMerge( BOOL bRecord )
{
    ScRange aRange;
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return FALSE;
    }
    else if (GetViewData()->GetSimpleArea( aRange ) == SC_MARK_SIMPLE)
    {
        ScDocument* pDoc = GetViewData()->GetDocument();
        ScRange aExtended( aRange );
        pDoc->ExtendMerge( aExtended );
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        SCTAB nTabCount = pDoc->GetTableCount();
        ScCellMergeOption aOption(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row());
        bool bExtended = false;
        do
        {
            bExtended = false;
            for (SCTAB i = 0; i < nTabCount; ++i)
            {
                if (!rMark.GetTableSelect(i))
                    // This table is not selected.
                    continue;

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

        HideCursor();
        BOOL bOk = pDocSh->GetDocFunc().UnmergeCells(aOption, bRecord, FALSE );
        aExtended = aOption.getFirstSingleRange();
        MarkRange( aExtended );
        ShowCursor();

        if (bOk)
            pDocSh->UpdateOle(GetViewData());
    }
    return TRUE;        //! bOk ??
}

//----------------------------------------------------------------------------

void ScViewFunc::FillSimple( FillDir eDir, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        BOOL bSuccess = pDocSh->GetDocFunc().FillSimple( aRange, &rMark, eDir, bRecord, FALSE );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            UpdateScrollBars();
        }
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//----------------------------------------------------------------------------

void ScViewFunc::FillSeries( FillDir eDir, FillCmd eCmd, FillDateCmd eDateCmd,
                             double fStart, double fStep, double fMax, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        const ScMarkData& rMark = GetViewData()->GetMarkData();
        BOOL bSuccess = pDocSh->GetDocFunc().
                        FillSeries( aRange, &rMark, eDir, eCmd, eDateCmd,
                                    fStart, fStep, fMax, bRecord, FALSE );
        if (bSuccess)
        {
            pDocSh->UpdateOle(GetViewData());
            UpdateScrollBars();

            // #i97876# Spreadsheet data changes are not notified
            ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
            if ( pModelObj && pModelObj->HasChangesListeners() )
            {
                ScRangeList aChangeRanges;
                aChangeRanges.Append( aRange );
                pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
            }
        }
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}

//----------------------------------------------------------------------------

void ScViewFunc::FillAuto( FillDir eDir, SCCOL nStartCol, SCROW nStartRow,
                            SCCOL nEndCol, SCROW nEndRow, ULONG nCount, BOOL bRecord )
{
    SCTAB nTab = GetViewData()->GetTabNo();
    ScRange aRange( nStartCol,nStartRow,nTab, nEndCol,nEndRow,nTab );
    ScRange aSourceRange( aRange );
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    const ScMarkData& rMark = GetViewData()->GetMarkData();
    BOOL bSuccess = pDocSh->GetDocFunc().
                    FillAuto( aRange, &rMark, eDir, nCount, bRecord, FALSE );
    if (bSuccess)
    {
        MarkRange( aRange, FALSE );         // aRange ist in FillAuto veraendert worden
        pDocSh->UpdateOle(GetViewData());
        UpdateScrollBars();

        // #i97876# Spreadsheet data changes are not notified
        ScModelObj* pModelObj = ScModelObj::getImplementation( pDocSh->GetModel() );
        if ( pModelObj && pModelObj->HasChangesListeners() )
        {
            ScRangeList aChangeRanges;
            ScRange aChangeRange( aRange );
            switch ( eDir )
            {
                case FILL_TO_BOTTOM:
                    {
                        aChangeRange.aStart.SetRow( aSourceRange.aEnd.Row() + 1 );
                    }
                    break;
                case FILL_TO_TOP:
                    {
                        aChangeRange.aEnd.SetRow( aSourceRange.aStart.Row() - 1 );
                    }
                    break;
                case FILL_TO_RIGHT:
                    {
                        aChangeRange.aStart.SetCol( aSourceRange.aEnd.Col() + 1 );
                    }
                    break;
                case FILL_TO_LEFT:
                    {
                        aChangeRange.aEnd.SetCol( aSourceRange.aStart.Col() - 1 );
                    }
                    break;
                default:
                    {

                    }
                    break;
            }
            aChangeRanges.Append( aChangeRange );
            pModelObj->NotifyChanges( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cell-change" ) ), aChangeRanges );
        }
    }
}

//----------------------------------------------------------------------------

void ScViewFunc::FillTab( USHORT nFlags, USHORT nFunction, BOOL bSkipEmpty, BOOL bAsLink )
{
    //! allow source sheet to be protected
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    SCTAB nTab = GetViewData()->GetTabNo();
    BOOL bUndo(pDoc->IsUndoEnabled());

    ScRange aMarkRange;
    rMark.MarkToSimple();
    BOOL bMulti = rMark.IsMultiMarked();
    if (bMulti)
        rMark.GetMultiMarkArea( aMarkRange );
    else if (rMark.IsMarked())
        rMark.GetMarkArea( aMarkRange );
    else
        aMarkRange = ScRange( GetViewData()->GetCurX(), GetViewData()->GetCurY(), nTab );

    ScDocument* pUndoDoc = NULL;

    if (bUndo)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        pUndoDoc->InitUndo( pDoc, nTab, nTab );

        SCTAB nTabCount = pDoc->GetTableCount();
        for (SCTAB i=0; i<nTabCount; i++)
            if (i != nTab && rMark.GetTableSelect(i))
            {
                pUndoDoc->AddUndoTab( i, i );
                aMarkRange.aStart.SetTab( i );
                aMarkRange.aEnd.SetTab( i );
                pDoc->CopyToDocument( aMarkRange, IDF_ALL, bMulti, pUndoDoc );
            }
    }

    if (bMulti)
        pDoc->FillTabMarked( nTab, rMark, nFlags, nFunction, bSkipEmpty, bAsLink );
    else
    {
        aMarkRange.aStart.SetTab( nTab );
        aMarkRange.aEnd.SetTab( nTab );
        pDoc->FillTab( aMarkRange, rMark, nFlags, nFunction, bSkipEmpty, bAsLink );
    }

    if (bUndo)
    {   //! fuer ChangeTrack erst zum Schluss
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoFillTable( pDocSh, rMark,
                                aMarkRange.aStart.Col(), aMarkRange.aStart.Row(), nTab,
                                aMarkRange.aEnd.Col(), aMarkRange.aEnd.Row(), nTab,
                                pUndoDoc, bMulti, nTab, nFlags, nFunction, bSkipEmpty, bAsLink ) );
    }

    pDocSh->PostPaintGridAll();
    pDocSh->PostDataChanged();
}

//----------------------------------------------------------------------------

/** Downward fill of selected cell(s) by double-clicking cross-hair cursor

    Extends a current selection down to the last non-empty cell of an adjacent
    column when the lower-right corner of the selection is double-clicked.  It
    uses a left-adjoining non-empty column as a guide if such is available,
    otherwise a right-adjoining non-empty column is used.

    @author Kohei Yoshida (kohei@openoffice.org)

    @return No return value

    @see #i12313#
*/
void ScViewFunc::FillCrossDblClick()
{
    ScRange aRange;
    GetViewData()->GetSimpleArea( aRange );
    aRange.Justify();

    SCTAB nTab = GetViewData()->GetCurPos().Tab();
    SCCOL nStartX = aRange.aStart.Col();
    SCROW nStartY = aRange.aStart.Row();
    SCCOL nEndX   = aRange.aEnd.Col();
    SCROW nEndY   = aRange.aEnd.Row();

    ScDocument* pDoc = GetViewData()->GetDocument();

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
                pDoc->FindAreaPos( nMovX, nMovY, nTab, 0, 1 );

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
                pDoc->FindAreaPos( nMovX, nMovY, nTab, 0, 1 );

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

//----------------------------------------------------------------------------

void ScViewFunc::TransliterateText( sal_Int32 nType )
{
    ScMarkData aFuncMark = GetViewData()->GetMarkData();
    if ( !aFuncMark.IsMarked() && !aFuncMark.IsMultiMarked() )
    {
        //  no selection -> use cursor position

        ScAddress aCursor( GetViewData()->GetCurX(), GetViewData()->GetCurY(), GetViewData()->GetTabNo() );
        aFuncMark.SetMarkArea( ScRange( aCursor ) );
    }

    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        TransliterateText( aFuncMark, nType, TRUE, FALSE );
    if (bSuccess)
    {
        GetViewData()->GetViewShell()->UpdateInputHandler();
    }
}

//----------------------------------------------------------------------------
//  AutoFormat

ScAutoFormatData* ScViewFunc::CreateAutoFormatData()
{
    ScAutoFormatData* pData = NULL;
    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;
    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
    {
        if ( nEndCol-nStartCol >= 3 && nEndRow-nStartRow >= 3 )
        {
            ScDocument* pDoc = GetViewData()->GetDocument();
            pData = new ScAutoFormatData;
            pDoc->GetAutoFormatData( nStartTab, nStartCol,nStartRow,nEndCol,nEndRow, *pData );
        }
    }
    return pData;
}


//----------------------------------------------------------------------------

void ScViewFunc::AutoFormat( USHORT nFormatNo, BOOL bRecord )
{
#if 1

    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScMarkData& rMark = GetViewData()->GetMarkData();

        BOOL bSuccess = pDocSh->GetDocFunc().AutoFormat( aRange, &rMark, nFormatNo, bRecord, FALSE );
        if (bSuccess)
            pDocSh->UpdateOle(GetViewData());
    }
    else
        ErrorMessage(STR_NOMULTISELECT);

#else

    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;

    if (GetViewData()->GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScDocument* pDoc = pDocSh->GetDocument();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        BOOL bSize = (*ScGlobal::GetAutoFormat())[nFormatNo]->GetIncludeWidthHeight();
        if (bRecord && !pDoc->IsUndoEnabled())
            bRecord = FALSE;

        ScDocument* pUndoDoc = NULL;
        if ( bRecord )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab, bSize, bSize );
            pDoc->CopyToDocument( nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab,
                                    IDF_ATTRIB, FALSE, pUndoDoc );
            if (bSize)
            {
                pDoc->CopyToDocument( nStartCol,0,nStartTab, nEndCol,MAXROW,nEndTab,
                                                            IDF_NONE, FALSE, pUndoDoc );
                pDoc->CopyToDocument( 0,nStartRow,nStartTab, MAXCOL,nEndRow,nEndTab,
                                                            IDF_NONE, FALSE, pUndoDoc );
            }
            pDoc->BeginDrawUndo();
        }

        GetFrameWin()->EnterWait();
        pDoc->AutoFormat( nStartCol, nStartRow, nEndCol, nEndRow, nFormatNo, rMark );
        GetFrameWin()->LeaveWait();

        if (bSize)
        {
            SetMarkedWidthOrHeight( TRUE, SC_SIZE_VISOPT, STD_EXTRA_WIDTH, FALSE, FALSE );
            SetMarkedWidthOrHeight( FALSE, SC_SIZE_VISOPT, 0, FALSE, FALSE );
            pDocSh->PostPaint( 0,0,nStartTab, MAXCOL,MAXROW,nStartTab,
                                    PAINT_GRID | PAINT_LEFT | PAINT_TOP );
        }
        else
        {
            BOOL bAdj = AdjustBlockHeight( FALSE );
            if (bAdj)
                pDocSh->PostPaint( 0,nStartRow,nStartTab, MAXCOL,MAXROW,nStartTab,
                                    PAINT_GRID | PAINT_LEFT );
            else
                pDocSh->PostPaint( nStartCol, nStartRow, nStartTab,
                                    nEndCol, nEndRow, nEndTab, PAINT_GRID );
        }

        if ( bRecord )      // Draw-Undo erst jetzt verfuegbar
        {
            pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoAutoFormat( pDocSh,
                        ScRange(nStartCol,nStartRow,nStartTab, nEndCol,nEndRow,nEndTab),
                        pUndoDoc, rMark, bSize, nFormatNo ) );
        }

        pDocSh->UpdateOle(GetViewData());
        pDocSh->SetDocumentModified();
    }
    else
        ErrorMessage(STR_NOMULTISELECT);

#endif
}


//----------------------------------------------------------------------------
//  Suchen & Ersetzen

void ScViewFunc::SearchAndReplace( const SvxSearchItem* pSearchItem,
                                        BOOL bAddUndo, BOOL bIsApi )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    ScMarkData& rMark = GetViewData()->GetMarkData();
    if (bAddUndo && !pDoc->IsUndoEnabled())
        bAddUndo = FALSE;

    SCCOL nCol = GetViewData()->GetCurX();
    SCROW nRow = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    USHORT nCommand = pSearchItem->GetCommand();
    BOOL bAllTables = pSearchItem->IsAllTables();
    BOOL* pOldSelectedTables = NULL;
    USHORT nOldSelectedCount = 0;
    SCTAB nOldTab = nTab;
    SCTAB nLastTab = pDoc->GetTableCount() - 1;
    SCTAB nStartTab, nEndTab;
    if ( bAllTables )
    {
        nStartTab = 0;
        nEndTab = nLastTab;
        pOldSelectedTables = new BOOL [ nEndTab + 1 ];
        for ( SCTAB j = 0; j <= nEndTab; j++ )
        {
            pOldSelectedTables[j] = rMark.GetTableSelect( j );
            if ( pOldSelectedTables[j] )
                ++nOldSelectedCount;
        }
    }
    else
    {   //! at least one is always selected
        nStartTab = nEndTab = rMark.GetFirstSelected();
        for ( SCTAB j = nStartTab + 1; j <= nLastTab; j++ )
        {
            if ( rMark.GetTableSelect( j ) )
                nEndTab = j;
        }
    }

    if (   nCommand == SVX_SEARCHCMD_REPLACE
        || nCommand == SVX_SEARCHCMD_REPLACE_ALL )
    {
        for ( SCTAB j = nStartTab; j <= nEndTab; j++ )
        {
            if ( (bAllTables || rMark.GetTableSelect( j )) &&
                    pDoc->IsTabProtected( j ) )
            {
                if ( pOldSelectedTables )
                    delete [] pOldSelectedTables;
                ErrorMessage(STR_PROTECTIONERR);
                return;
            }
        }
    }

    if (   nCommand == SVX_SEARCHCMD_FIND
        || nCommand == SVX_SEARCHCMD_FIND_ALL)
        bAddUndo = FALSE;

    //!     account for bAttrib during Undo !!!

    ScDocument* pUndoDoc = NULL;
    ScMarkData* pUndoMark = NULL;
    String aUndoStr;
    if (bAddUndo)
    {
        pUndoMark = new ScMarkData( rMark );                // Mark is being modified
        if ( nCommand == SVX_SEARCHCMD_REPLACE_ALL )
        {
            pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
            pUndoDoc->InitUndo( pDoc, nStartTab, nEndTab );
        }
    }

    if ( bAllTables )
    {   //! select all, after pUndoMark has been created
        for ( SCTAB j = nStartTab; j <= nEndTab; j++ )
        {
            rMark.SelectTable( j, TRUE );
        }
    }

    DoneBlockMode(TRUE);                // don't delete mark
    InitOwnBlockMode();

    //  If search starts at the beginning don't ask again whether it shall start at the beginning
    BOOL bFirst = TRUE;
    if ( nCol == 0 && nRow == 0 && nTab == nStartTab && !pSearchItem->GetBackward()  )
        bFirst = FALSE;

    BOOL bFound = FALSE;
    while (TRUE)
    {
        GetFrameWin()->EnterWait();
        if (pDoc->SearchAndReplace( *pSearchItem, nCol, nRow, nTab, rMark, aUndoStr, pUndoDoc ) )
        {
            bFound = TRUE;
            bFirst = TRUE;
            if (bAddUndo)
            {
                GetViewData()->GetDocShell()->GetUndoManager()->AddUndoAction(
                    new ScUndoReplace( GetViewData()->GetDocShell(), *pUndoMark,
                                        nCol, nRow, nTab,
                                        aUndoStr, pUndoDoc, pSearchItem ) );
                pUndoDoc = NULL;
            }

            break;                  // break 'while (TRUE)'
        }
        else if ( bFirst && (nCommand == SVX_SEARCHCMD_FIND ||
                nCommand == SVX_SEARCHCMD_REPLACE) )
        {
            bFirst = FALSE;
            USHORT nRetVal;
            GetFrameWin()->LeaveWait();
            if ( bIsApi )
                nRetVal = RET_NO;
            else
            {
                //  search dialog as parent (if available)
                Window* pParent = GetParentOrChild(SID_SEARCH_DLG);
                USHORT nStrId;
                if ( pSearchItem->GetBackward() )
                {
                    if ( nStartTab == nEndTab )
                        nStrId = STR_MSSG_SEARCHANDREPLACE_1;
                    else
                        nStrId = STR_MSSG_SEARCHANDREPLACE_4;
                }
                else
                {
                    if ( nStartTab == nEndTab )
                        nStrId = STR_MSSG_SEARCHANDREPLACE_2;
                    else
                        nStrId = STR_MSSG_SEARCHANDREPLACE_5;
                }
                MessBox aBox( pParent, WinBits(WB_YES_NO | WB_DEF_YES),
                                ScGlobal::GetRscString( STR_MSSG_SEARCHANDREPLACE_3 ),
                                ScGlobal::GetRscString( nStrId ) );
                nRetVal = aBox.Execute();
            }

            if ( nRetVal == RET_YES )
            {
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
            if ( nCommand == SVX_SEARCHCMD_FIND_ALL || nCommand == SVX_SEARCHCMD_REPLACE_ALL )
            {
                pDocSh->PostPaintGridAll();                             // Mark
            }

            GetFrameWin()->LeaveWait();
            if (!bIsApi)
            {
                //  search dialog as parent if available
                Window* pParent = GetParentOrChild(SID_SEARCH_DLG);
                // "nothing found"
                InfoBox aBox( pParent, ScGlobal::GetRscString( STR_MSSG_SEARCHANDREPLACE_0 ) );
                aBox.Execute();
            }

            break;                      // break 'while (TRUE)'
        }
    }                               // of while TRUE

    if ( pOldSelectedTables )
    {   // restore originally selected table
        for ( SCTAB j = nStartTab; j <= nEndTab; j++ )
        {
            rMark.SelectTable( j, pOldSelectedTables[j] );
        }
        if ( bFound )
        {   // durch Fundstelle neu selektierte Tabelle bleibt
            rMark.SelectTable( nTab, TRUE );
            // wenn vorher nur eine selektiert war, ist es ein Tausch
            //! wenn nicht, ist jetzt evtl. eine mehr selektiert
            if ( nOldSelectedCount == 1 && nTab != nOldTab )
                rMark.SelectTable( nOldTab, FALSE );
        }
        delete [] pOldSelectedTables;
    }

    MarkDataChanged();

    if ( bFound )
    {
        if ( nTab != GetViewData()->GetTabNo() )
            SetTabNo( nTab );

        //  wenn nichts markiert ist, DoneBlockMode, damit von hier aus
        //  direkt per Shift-Cursor markiert werden kann:
        if (!rMark.IsMarked() && !rMark.IsMultiMarked())
            DoneBlockMode(TRUE);

        AlignToCursor( nCol, nRow, SC_FOLLOW_JUMP );
        SetCursor( nCol, nRow, TRUE );

        if (   nCommand == SVX_SEARCHCMD_REPLACE
            || nCommand == SVX_SEARCHCMD_REPLACE_ALL )
        {
            if ( nCommand == SVX_SEARCHCMD_REPLACE )
                pDocSh->PostPaint( nCol,nRow,nTab, nCol,nRow,nTab, PAINT_GRID );
            else
                pDocSh->PostPaintGridAll();
            pDocSh->SetDocumentModified();
        }
        else if ( nCommand == SVX_SEARCHCMD_FIND_ALL )
            pDocSh->PostPaintGridAll();                             // Markierung
        GetFrameWin()->LeaveWait();
    }

    delete pUndoDoc;            // loeschen wenn nicht benutzt
    delete pUndoMark;           // kann immer geloescht werden
}


//----------------------------------------------------------------------------
//  Zielwertsuche

void ScViewFunc::Solve( const ScSolveParam& rParam )
{
    ScDocument* pDoc = GetViewData()->GetDocument();

    SCCOL nDestCol = rParam.aRefVariableCell.Col();
    SCROW nDestRow = rParam.aRefVariableCell.Row();
    SCTAB nDestTab = rParam.aRefVariableCell.Tab();

    ScEditableTester aTester( pDoc, nDestTab, nDestCol,nDestRow, nDestCol,nDestRow );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    if ( pDoc )
    {
        String  aTargetValStr;
        if ( rParam.pStrTargetVal != NULL )
            aTargetValStr = *(rParam.pStrTargetVal);

        String  aMsgStr;
        String  aResStr;
        double  nSolveResult;

        GetFrameWin()->EnterWait();

        BOOL    bExact =
                    pDoc->Solver(
                        rParam.aRefFormulaCell.Col(),
                        rParam.aRefFormulaCell.Row(),
                        rParam.aRefFormulaCell.Tab(),
                        nDestCol, nDestRow, nDestTab,
                        aTargetValStr,
                        nSolveResult );

        GetFrameWin()->LeaveWait();

        SvNumberFormatter* pFormatter = pDoc->GetFormatTable();
        ULONG nFormat = 0;
        const ScPatternAttr* pPattern = pDoc->GetPattern( nDestCol, nDestRow, nDestTab );
        if ( pPattern )
            nFormat = pPattern->GetNumberFormat( pFormatter );
        Color* p;
        pFormatter->GetOutputString( nSolveResult, nFormat, aResStr, &p );

        if ( bExact )
        {
            aMsgStr  = ScGlobal::GetRscString( STR_MSSG_SOLVE_0 );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_1 );
            aMsgStr += String( aResStr );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_2 );
        }
        else
        {
            aMsgStr  = ScGlobal::GetRscString( STR_MSSG_SOLVE_3 );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_4 );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_5 );
            aMsgStr += String( aResStr );
            aMsgStr += ScGlobal::GetRscString( STR_MSSG_SOLVE_6 );
        }

        MessBox aBox( GetViewData()->GetDialogParent(),
                        WinBits(WB_YES_NO | WB_DEF_NO),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ), aMsgStr );
        USHORT nRetVal = aBox.Execute();

        if ( RET_YES == nRetVal )
            EnterValue( nDestCol, nDestRow, nDestTab, nSolveResult );

        GetViewData()->GetViewShell()->UpdateInputHandler( TRUE );
    }
}


//----------------------------------------------------------------------------
//  Mehrfachoperation

void ScViewFunc::TabOp( const ScTabOpParam& rParam, BOOL bRecord )
{
    ScRange aRange;
    if (GetViewData()->GetSimpleArea(aRange) == SC_MARK_SIMPLE)
    {
        ScDocShell* pDocSh = GetViewData()->GetDocShell();
        ScMarkData& rMark = GetViewData()->GetMarkData();
        pDocSh->GetDocFunc().TabOp( aRange, &rMark, rParam, bRecord, FALSE );
    }
    else
        ErrorMessage(STR_NOMULTISELECT);
}


//----------------------------------------------------------------------------

void ScViewFunc::MakeScenario( const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScMarkData& rMark   = GetViewData()->GetMarkData();
    SCTAB       nTab    = GetViewData()->GetTabNo();

    SCTAB nNewTab = pDocSh->MakeScenario( nTab, rName, rComment, rColor, nFlags, rMark );
    if (nFlags & SC_SCENARIO_COPYALL)
        SetTabNo( nNewTab, TRUE );          // SC_SCENARIO_COPYALL -> sichtbar
    else
    {
        SfxBindings& rBindings = GetViewData()->GetBindings();
        rBindings.Invalidate( SID_STATUS_DOCPOS );      // Statusbar
        rBindings.Invalidate( SID_TABLES_COUNT );
        rBindings.Invalidate( SID_SELECT_SCENARIO );
        rBindings.Invalidate( FID_TABLE_SHOW );
    }
}


//----------------------------------------------------------------------------

void ScViewFunc::ExtendScenario()
{
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

        //  Undo: Attribute anwenden

    ScDocument* pDoc = GetViewData()->GetDocument();
    ScPatternAttr aPattern( pDoc->GetPool() );
    aPattern.GetItemSet().Put( ScMergeFlagAttr( SC_MF_SCENARIO ) );
    aPattern.GetItemSet().Put( ScProtectionAttr( TRUE ) );
    ApplySelectionPattern(aPattern);
}


//----------------------------------------------------------------------------

void ScViewFunc::UseScenario( const String& rName )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    SCTAB       nTab    = GetViewData()->GetTabNo();

    DoneBlockMode();
    InitOwnBlockMode();
    pDocSh->UseScenario( nTab, rName );
}


//----------------------------------------------------------------------------
//  Tabelle einfuegen

BOOL ScViewFunc::InsertTable( const String& rName, SCTAB nTab, BOOL bRecord )
{
    //  Reihenfolge Tabelle/Name ist bei DocFunc umgekehrt
    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        InsertTable( nTab, rName, bRecord, FALSE );
    if (bSuccess)
        SetTabNo( nTab, TRUE );

    return bSuccess;
}

//----------------------------------------------------------------------------
//  Tabellen einfuegen

BOOL ScViewFunc::InsertTables(SvStrings *pNames, SCTAB nTab,
                                            SCTAB nCount, BOOL bRecord )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    SvStrings *pNameList= NULL;

    WaitObject aWait( GetFrameWin() );

    if (bRecord)
    {
        pNameList= new SvStrings;
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage
    }

    BOOL bFlag=FALSE;

    String aValTabName;
    String *pStr;

    for(SCTAB i=0;i<nCount;i++)
    {
        if(pNames!=NULL)
        {
            pStr=pNames->GetObject(static_cast<USHORT>(i));
        }
        else
        {
            aValTabName.Erase();
            pDoc->CreateValidTabName( aValTabName);
            pStr=&aValTabName;
        }

        if(pDoc->InsertTab( nTab+i,*pStr))
        {
            bFlag=TRUE;
            pDocSh->Broadcast( ScTablesHint( SC_TAB_INSERTED, nTab+i ) );
        }
        else
        {
            break;
        }

        if(pNameList!=NULL)
            pNameList->Insert(new String(*pStr),pNameList->Count());

    }

    if (bFlag)
    {
        if (bRecord)
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTables( pDocSh, nTab, FALSE, pNameList));

        //  Views updaten:

        SetTabNo( nTab, TRUE );
        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::AppendTable( const String& rName, BOOL bRecord )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc   = pDocSh->GetDocument();
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;

    WaitObject aWait( GetFrameWin() );

    if (bRecord)
        pDoc->BeginDrawUndo();                          //  InsertTab erzeugt ein SdrUndoNewPage

    if (pDoc->InsertTab( SC_TAB_APPEND, rName ))
    {
        SCTAB nTab = pDoc->GetTableCount()-1;
        if (bRecord)
            pDocSh->GetUndoManager()->AddUndoAction(
                        new ScUndoInsertTab( pDocSh, nTab, TRUE, rName));
        GetViewData()->InsertTab( nTab );
        SetTabNo( nTab, TRUE );
        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::DeleteTable( SCTAB nTab, BOOL bRecord )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();

    BOOL bSuccess = pDocSh->GetDocFunc().DeleteTable( nTab, bRecord, FALSE );
    if (bSuccess)
    {
        SCTAB nNewTab = nTab;
        if ( nNewTab >= pDoc->GetTableCount() )
            --nNewTab;
        SetTabNo( nNewTab, TRUE );
    }
    return bSuccess;
}

BOOL ScViewFunc::DeleteTables(const vector<SCTAB> &TheTabs, BOOL bRecord )
{
    ScDocShell* pDocSh  = GetViewData()->GetDocShell();
    ScDocument* pDoc    = pDocSh->GetDocument();
    BOOL bVbaEnabled = pDoc ? pDoc->IsInVBAMode() : FALSE;
    SCTAB       nNewTab = TheTabs[0];
    WaitObject aWait( GetFrameWin() );
    if (bRecord && !pDoc->IsUndoEnabled())
        bRecord = FALSE;
    if ( bVbaEnabled )
        bRecord = FALSE;

    while ( nNewTab > 0 && !pDoc->IsVisible( nNewTab ) )
        --nNewTab;

    BOOL bWasLinked = FALSE;
    ScDocument* pUndoDoc = NULL;
    ScRefUndoData* pUndoData = NULL;
    if (bRecord)
    {
        pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
        SCTAB nCount = pDoc->GetTableCount();

        String aOldName;
        for(unsigned int i=0; i<TheTabs.size(); ++i)
        {
            SCTAB nTab = TheTabs[i];
            if (i==0)
                pUndoDoc->InitUndo( pDoc, nTab,nTab, TRUE,TRUE );   // incl. Spalten/Zeilenflags
            else
                pUndoDoc->AddUndoTab( nTab,nTab, TRUE,TRUE );       // incl. Spalten/Zeilenflags

            pDoc->CopyToDocument(0,0,nTab, MAXCOL,MAXROW,nTab, IDF_ALL,FALSE, pUndoDoc );
            pDoc->GetName( nTab, aOldName );
            pUndoDoc->RenameTab( nTab, aOldName, FALSE );
            if (pDoc->IsLinked(nTab))
            {
                bWasLinked = TRUE;
                pUndoDoc->SetLink( nTab, pDoc->GetLinkMode(nTab), pDoc->GetLinkDoc(nTab),
                                    pDoc->GetLinkFlt(nTab), pDoc->GetLinkOpt(nTab),
                                    pDoc->GetLinkTab(nTab),
                                    pDoc->GetLinkRefreshDelay(nTab) );
            }
            if ( pDoc->IsScenario(nTab) )
            {
                pUndoDoc->SetScenario( nTab, TRUE );
                String aComment;
                Color  aColor;
                USHORT nScenFlags;
                pDoc->GetScenarioData( nTab, aComment, aColor, nScenFlags );
                pUndoDoc->SetScenarioData( nTab, aComment, aColor, nScenFlags );
                BOOL bActive = pDoc->IsActiveScenario( nTab );
                pUndoDoc->SetActiveScenario( nTab, bActive );
            }
            pUndoDoc->SetVisible( nTab, pDoc->IsVisible( nTab ) );
            pUndoDoc->SetTabBgColor( nTab, pDoc->GetTabBgColor(nTab) );
            pUndoDoc->SetSheetEvents( nTab, pDoc->GetSheetEvents( nTab ) );

            if ( pDoc->IsTabProtected( nTab ) )
                pUndoDoc->SetTabProtection(nTab, pDoc->GetTabProtection(nTab));

            //  Drawing-Layer muss sein Undo selbst in der Hand behalten !!!
            //      pUndoDoc->TransferDrawPage(pDoc, nTab,nTab);
        }

        pUndoDoc->AddUndoTab( 0, nCount-1 );            //  alle Tabs fuer Referenzen

        pDoc->BeginDrawUndo();                          //  DeleteTab erzeugt ein SdrUndoDelPage

        pUndoData = new ScRefUndoData( pDoc );
    }

    BOOL bDelDone = FALSE;

    for(int i=TheTabs.size()-1; i>=0; --i)
    {
        String sCodeName;
        BOOL bHasCodeName = pDoc->GetCodeName( TheTabs[i], sCodeName );
        if (pDoc->DeleteTab( TheTabs[i], pUndoDoc ))
        {
            bDelDone = TRUE;
            if( bVbaEnabled )
            {
                if( bHasCodeName )
                {
                    VBA_DeleteModule( *pDocSh, sCodeName );
                }
            }
            pDocSh->Broadcast( ScTablesHint( SC_TAB_DELETED, TheTabs[i] ) );
        }
    }
    if (bRecord)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoDeleteTab( GetViewData()->GetDocShell(), TheTabs,
                                            pUndoDoc, pUndoData ));
    }


    if (bDelDone)
    {
        if ( nNewTab >= pDoc->GetTableCount() )
            nNewTab = pDoc->GetTableCount() - 1;

        SetTabNo( nNewTab, TRUE );

        if (bWasLinked)
        {
            pDocSh->UpdateLinks();              // Link-Manager updaten
            GetViewData()->GetBindings().Invalidate(SID_LINKS);
        }

        pDocSh->PostPaintExtras();
        pDocSh->SetDocumentModified();


        SfxApplication* pSfxApp = SFX_APP();                                // Navigator
        pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
        pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREALINKS_CHANGED ) );
    }
    else
    {
        delete pUndoDoc;
        delete pUndoData;
    }
    return bDelDone;
}


//----------------------------------------------------------------------------

BOOL ScViewFunc::RenameTable( const String& rName, SCTAB nTab )
{
    //  Reihenfolge Tabelle/Name ist bei DocFunc umgekehrt
    BOOL bSuccess = GetViewData()->GetDocShell()->GetDocFunc().
                        RenameTable( nTab, rName, TRUE, FALSE );
    if (bSuccess)
    {
        //  Der Tabellenname koennte in einer Formel vorkommen...
        GetViewData()->GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}


//----------------------------------------------------------------------------

bool ScViewFunc::SetTabBgColor( const Color& rColor, SCTAB nTab )
{
    bool bSuccess = GetViewData()->GetDocShell()->GetDocFunc().SetTabBgColor( nTab, rColor, TRUE, FALSE );
    if (bSuccess)
    {
        GetViewData()->GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}

bool ScViewFunc::SetTabBgColor( ScUndoTabColorInfo::List& rUndoSetTabBgColorInfoList )
{
    bool bSuccess = GetViewData()->GetDocShell()->GetDocFunc().SetTabBgColor( rUndoSetTabBgColorInfoList, TRUE, FALSE );
    if (bSuccess)
    {
        GetViewData()->GetViewShell()->UpdateInputHandler();
    }
    return bSuccess;
}

//----------------------------------------------------------------------------

void ScViewFunc::InsertAreaLink( const String& rFile,
                                    const String& rFilter, const String& rOptions,
                                    const String& rSource, ULONG nRefresh )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    SCCOL nPosX = GetViewData()->GetCurX();
    SCROW nPosY = GetViewData()->GetCurY();
    SCTAB nTab = GetViewData()->GetTabNo();
    ScAddress aPos( nPosX, nPosY, nTab );

    pDocSh->GetDocFunc().InsertAreaLink( rFile, rFilter, rOptions, rSource, aPos, nRefresh, FALSE, FALSE );
}


//----------------------------------------------------------------------------

void ScViewFunc::InsertTableLink( const String& rFile,
                                    const String& rFilter, const String& rOptions,
                                    const String& rTabName )
{
    String aFilterName = rFilter;
    String aOpt = rOptions;
    ScDocumentLoader aLoader( rFile, aFilterName, aOpt );
    if (!aLoader.IsError())
    {
        ScDocShell* pSrcSh = aLoader.GetDocShell();
        ScDocument* pSrcDoc = pSrcSh->GetDocument();
        SCTAB nTab = MAXTAB+1;
        if (!rTabName.Len())                // kein Name angegeben -> erste Tabelle
            nTab = 0;
        else
        {
            String aTemp;
            SCTAB nCount = pSrcDoc->GetTableCount();
            for (SCTAB i=0; i<nCount; i++)
            {
                pSrcDoc->GetName( i, aTemp );
                if ( aTemp == rTabName )
                    nTab = i;
            }
        }

        if ( nTab <= MAXTAB )
            ImportTables( pSrcSh, 1, &nTab, TRUE,
                        GetViewData()->GetTabNo() );
    }
}


//----------------------------------------------------------------------------
//  Tabellen aus anderem Dokument kopieren / linken

void ScViewFunc::ImportTables( ScDocShell* pSrcShell,
                                SCTAB nCount, const SCTAB* pSrcTabs, BOOL bLink,SCTAB nTab )
{
    ScDocument* pSrcDoc = pSrcShell->GetDocument();

    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());

    BOOL bError = FALSE;
    BOOL bRefs = FALSE;
    BOOL bName = FALSE;

    if (pSrcDoc->GetDrawLayer())
        pDocSh->MakeDrawLayer();

    if (bUndo)
        pDoc->BeginDrawUndo();          // drawing layer must do its own undo actions

    SCTAB nInsCount = 0;
    SCTAB i;
    for( i=0; i<nCount; i++ )
    {   // #63304# insert sheets first and update all references
        String aName;
        pSrcDoc->GetName( pSrcTabs[i], aName );
        pDoc->CreateValidTabName( aName );
        if ( !pDoc->InsertTab( nTab+i, aName ) )
        {
            bError = TRUE;      // total error
            break;  // for
        }
        ++nInsCount;
    }
    for (i=0; i<nCount && !bError; i++)
    {
        SCTAB nSrcTab = pSrcTabs[i];
        SCTAB nDestTab1=nTab+i;
        ULONG nErrVal = pDocSh->TransferTab( *pSrcShell, nSrcTab, nDestTab1,
            FALSE, FALSE );     // no insert

        switch (nErrVal)
        {
            case 0:                     // interner Fehler oder voll Fehler
                bError = TRUE;
                break;
            case 2:
                bRefs = TRUE;
                break;
            case 3:
                bName = TRUE;
                break;
            case 4:
                bRefs = bName = TRUE;
                break;
        }

    }

    if (bLink)
    {
        sfx2::LinkManager* pLinkManager = pDoc->GetLinkManager();

        SfxMedium* pMed = pSrcShell->GetMedium();
        String aFileName = pMed->GetName();
        String aFilterName;
        if (pMed->GetFilter())
            aFilterName = pMed->GetFilter()->GetFilterName();
        String aOptions = ScDocumentLoader::GetOptions(*pMed);

        BOOL bWasThere = pDoc->HasLink( aFileName, aFilterName, aOptions );

        ULONG nRefresh = 0;
        String aTabStr;
        for (i=0; i<nInsCount; i++)
        {
            pSrcDoc->GetName( pSrcTabs[i], aTabStr );
            pDoc->SetLink( nTab+i, SC_LINK_NORMAL,
                        aFileName, aFilterName, aOptions, aTabStr, nRefresh );
        }

        if (!bWasThere)         // Link pro Quelldokument nur einmal eintragen
        {
            ScTableLink* pLink = new ScTableLink( pDocSh, aFileName, aFilterName, aOptions, nRefresh );
            pLink->SetInCreate( TRUE );
            pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, aFileName, &aFilterName );
            pLink->Update();
            pLink->SetInCreate( FALSE );

            SfxBindings& rBindings = GetViewData()->GetBindings();
            rBindings.Invalidate( SID_LINKS );
        }
    }


    if (bUndo)
    {
        pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoImportTab( pDocSh, nTab, nCount, bLink ) );
    }

    for (i=0; i<nInsCount; i++)
        GetViewData()->InsertTab(nTab);
    SetTabNo(nTab,TRUE);
    pDocSh->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS );

    SfxApplication* pSfxApp = SFX_APP();
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    pSfxApp->Broadcast( SfxSimpleHint( SC_HINT_AREAS_CHANGED ) );

    pDocSh->PostPaintExtras();
    pDocSh->PostPaintGridAll();
    pDocSh->SetDocumentModified();

    if (bRefs)
        ErrorMessage(STR_ABSREFLOST);
    if (bName)
        ErrorMessage(STR_NAMECONFLICT);
}


//----------------------------------------------------------------------------
//  Tabelle in anderes Dokument verschieben / kopieren

void ScViewFunc::MoveTable( USHORT nDestDocNo, SCTAB nDestTab, BOOL bCopy, const String* pNewTabName )
{
    ScDocument* pDoc       = GetViewData()->GetDocument();
    ScDocShell* pDocShell  = GetViewData()->GetDocShell();
    ScDocument* pDestDoc   = NULL;
    ScDocShell* pDestShell = NULL;
    ScTabViewShell* pDestViewSh = NULL;
    BOOL bUndo (pDoc->IsUndoEnabled());
    bool bRename = pNewTabName && pNewTabName->Len();

    BOOL bNewDoc = ( nDestDocNo == SC_DOC_NEW );
    if ( bNewDoc )
    {
        nDestTab = 0;           // als erstes einfuegen

        //  ohne SFX_CALLMODE_RECORD ausfuehren, weil schon im Move-Befehl enthalten:

        String aUrl = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("private:factory/"));
        aUrl.AppendAscii(RTL_CONSTASCII_STRINGPARAM( STRING_SCAPP ));               // "scalc"
        SfxStringItem aItem( SID_FILE_NAME, aUrl );
        SfxStringItem aTarget( SID_TARGETNAME, String::CreateFromAscii("_blank") );

        const SfxPoolItem* pRetItem = GetViewData()->GetDispatcher().Execute(
                    SID_OPENDOC, SFX_CALLMODE_API|SFX_CALLMODE_SYNCHRON, &aItem, &aTarget, 0L );
        if ( pRetItem )
        {
            if ( pRetItem->ISA( SfxObjectItem ) )
                pDestShell = PTR_CAST( ScDocShell, ((const SfxObjectItem*)pRetItem)->GetShell() );
            else if ( pRetItem->ISA( SfxViewFrameItem ) )
            {
                SfxViewFrame* pFrm = ((const SfxViewFrameItem*)pRetItem)->GetFrame();
                if (pFrm)
                    pDestShell = PTR_CAST( ScDocShell, pFrm->GetObjectShell() );
            }
            if (pDestShell)
                pDestViewSh = pDestShell->GetBestViewShell();
        }
    }
    else
        pDestShell = ScDocShell::GetShellByNum( nDestDocNo );

    if (!pDestShell)
    {
        DBG_ERROR("Dest-Doc nicht gefunden !!!");
        return;
    }

    ScMarkData& rMark = GetViewData()->GetMarkData();
    if (bRename && rMark.GetSelectCount() != 1)
    {
        // Custom sheet name is provided, but more than one sheet is selected.
        // We don't support this scenario at the moment.
        return;
    }

    pDestDoc = pDestShell->GetDocument();

    SCTAB nTab = GetViewData()->GetTabNo();

    if (pDestDoc != pDoc)
    {
        if (bNewDoc)
        {
            while (pDestDoc->GetTableCount() > 1)
                pDestDoc->DeleteTab(0);
            pDestDoc->RenameTab( 0,
                        String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("______42_____")),
                        FALSE );
        }

        SCTAB       nTabCount   = pDoc->GetTableCount();
        SCTAB       nTabSelCount = rMark.GetSelectCount();

        vector<SCTAB> TheTabs;

        for(SCTAB i=0; i<nTabCount; ++i)
        {
            if(rMark.GetTableSelect(i))
            {
                String aTabName;
                pDoc->GetName( i, aTabName);
                TheTabs.push_back(i);
                for(SCTAB j=i+1;j<nTabCount;j++)
                {
                    if((!pDoc->IsVisible(j))&&(pDoc->IsScenario(j)))
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

        ULONG nErrVal =1;
        if(nDestTab==SC_TAB_APPEND)
            nDestTab=pDestDoc->GetTableCount();
        SCTAB nDestTab1=nDestTab;
        for( USHORT j=0; j<TheTabs.size(); ++j, ++nDestTab1 )
        {   // #63304# insert sheets first and update all references
            String aName;
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
        }
        if ( nErrVal > 0 )
        {
            nDestTab1 = nDestTab;
            for(USHORT i=0; i<TheTabs.size();++i)
            {
                nErrVal = pDestShell->TransferTab( *pDocShell, TheTabs[i], static_cast<SCTAB>(nDestTab1), FALSE, FALSE );
                nDestTab1++;
            }
        }
        String sName;
        if (!bNewDoc && bUndo)
        {
            pDestDoc->GetName(nDestTab, sName);
            pDestShell->GetUndoManager()->AddUndoAction(
                            new ScUndoImportTab( pDestShell, nDestTab,
                                static_cast<SCTAB>(TheTabs.size()), FALSE));

        }
        else
        {
            pDestShell->GetUndoManager()->Clear();
        }

        GetFrameWin()->LeaveWait();
        switch (nErrVal)
        {
            case 0:                     // interner Fehler oder voll Fehler
            {
                ErrorMessage(STR_TABINSERT_ERROR);
                return;
            }
            //break;
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
                DeleteTables(TheTabs);// incl. Paint & Undo
            else
                ErrorMessage(STR_TABREMOVE_ERROR);
        }

        if (bNewDoc)
        {
            //  ChartListenerCollection must be updated before DeleteTab
            if ( pDestDoc->IsChartListenerCollectionNeedsUpdate() )
                pDestDoc->UpdateChartListenerCollection();

            pDestDoc->DeleteTab(static_cast<SCTAB>(TheTabs.size()));   // alte erste Tabelle
            if (pDestViewSh)
                pDestViewSh->TabChanged();      // Pages auf dem Drawing-Layer
            pDestShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB,
                                    PAINT_GRID | PAINT_TOP | PAINT_LEFT |
                                    PAINT_EXTRAS | PAINT_SIZE );
            //  PAINT_SIZE fuer Gliederung
        }
        else
        {
            pDestShell->Broadcast( ScTablesHint( SC_TAB_INSERTED, nDestTab ) );
            pDestShell->PostPaintExtras();
            pDestShell->PostPaintGridAll();
        }

        TheTabs.clear();

        pDestShell->SetDocumentModified();
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
    }
    else
    {
        // Move or copy within the same document.
        SCTAB       nTabCount   = pDoc->GetTableCount();

        auto_ptr< vector<SCTAB> >    pSrcTabs(new vector<SCTAB>);
        auto_ptr< vector<SCTAB> >    pDestTabs(new vector<SCTAB>);
        auto_ptr< vector<OUString> > pTabNames(new vector<OUString>);
        auto_ptr< vector<OUString> > pDestNames(NULL);
        pSrcTabs->reserve(nTabCount);
        pDestTabs->reserve(nTabCount);
        pTabNames->reserve(nTabCount);
        String      aDestName;

        for(SCTAB i=0;i<nTabCount;i++)
        {
            if(rMark.GetTableSelect(i))
            {
                String aTabName;
                pDoc->GetName( i, aTabName);
                pTabNames->push_back(aTabName);

                for(SCTAB j=i+1;j<nTabCount;j++)
                {
                    if((!pDoc->IsVisible(j))&&(pDoc->IsScenario(j)))
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
            pDocShell->MoveTable( nMovTab, nDestTab1, bCopy, FALSE );   // Undo ist hier

            if(bCopy && pDoc->IsScenario(nMovTab))
            {
                String aComment;
                Color  aColor;
                USHORT nFlags;

                pDoc->GetScenarioData(nMovTab, aComment,aColor, nFlags);
                pDoc->SetScenario(nDestTab1,TRUE);
                pDoc->SetScenarioData(nDestTab1,aComment,aColor,nFlags);
                BOOL bActive = pDoc->IsActiveScenario(nMovTab );
                pDoc->SetActiveScenario( nDestTab1, bActive );
                BOOL bVisible=pDoc->IsVisible(nMovTab);
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
                String aTabName = *pNewTabName;
                pDoc->CreateValidTabName( aTabName );
                pDestNames->push_back(aTabName);
                pDoc->RenameTab(nRenameTab, aTabName);
            }
        }
        else
            // No need to keep this around when we are not renaming.
            pTabNames.reset();

        nTab = GetViewData()->GetTabNo();

        if (bUndo)
        {
            if (bCopy)
            {
                pDocShell->GetUndoManager()->AddUndoAction(
                        new ScUndoCopyTab(
                            pDocShell, pSrcTabs.release(), pDestTabs.release(), pDestNames.release()));
            }
            else
            {
                pDocShell->GetUndoManager()->AddUndoAction(
                        new ScUndoMoveTab(
                            pDocShell, pSrcTabs.release(), pDestTabs.release(), pTabNames.release(), pDestNames.release()));
            }
        }

        SCTAB nNewTab = nDestTab;
        if (nNewTab == SC_TAB_APPEND)
            nNewTab = pDoc->GetTableCount()-1;
        else if (!bCopy && nTab<nDestTab)
            nNewTab--;

        SetTabNo( nNewTab, TRUE );

        //#i29848# adjust references to data on the copied sheet
        if( bCopy )
            ScChartHelper::AdjustRangesOfChartsOnDestinationPage( pDoc, pDestDoc, nTab, nNewTab );
    }
}


//----------------------------------------------------------------------------

void ScViewFunc::ShowTable( const String& rName )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());
    BOOL bFound = FALSE;
    SCTAB nPos = 0;
    String aTabName;
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nCount; i++)
    {
        pDoc->GetName( i, aTabName );
        if ( aTabName == rName )
        {
            nPos = i;
            bFound = TRUE;
        }
    }

    if (bFound)
    {
        pDoc->SetVisible( nPos, TRUE );
        if (bUndo)
        {
            pDocSh->GetUndoManager()->AddUndoAction( new ScUndoShowHideTab( pDocSh, nPos, TRUE ) );
        }
        SetTabNo( nPos, TRUE );
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        pDocSh->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
        pDocSh->SetDocumentModified();
    }
    else
        Sound::Beep();
}


//----------------------------------------------------------------------------

void ScViewFunc::HideTable( SCTAB nTab )
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocument* pDoc = pDocSh->GetDocument();
    BOOL bUndo(pDoc->IsUndoEnabled());
    SCTAB nVisible = 0;
    SCTAB nCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nCount; i++)
    {
        if (pDoc->IsVisible(i))
            ++nVisible;
    }

    if (nVisible > 1)
    {
        pDoc->SetVisible( nTab, FALSE );
        if (bUndo)
        {
            pDocSh->GetUndoManager()->AddUndoAction( new ScUndoShowHideTab( pDocSh, nTab, FALSE ) );
        }

        //  Views updaten:
        pDocSh->Broadcast( ScTablesHint( SC_TAB_HIDDEN, nTab ) );

        SetTabNo( nTab, TRUE );
        SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );
        pDocSh->PostPaint(0,0,0,MAXCOL,MAXROW,MAXTAB, PAINT_EXTRAS);
        pDocSh->SetDocumentModified();
    }
    else
        Sound::Beep();
}


//----------------------------------------------------------------------------

void ScViewFunc::InsertSpecialChar( const String& rStr, const Font& rFont )
{
    ScEditableTester aTester( this );
    if (!aTester.IsEditable())
    {
        ErrorMessage(aTester.GetMessageId());
        return;
    }

    const sal_Unicode* pChar    = rStr.GetBuffer();
    ScTabViewShell* pViewShell  = GetViewData()->GetViewShell();
    SvxFontItem     aFontItem( rFont.GetFamily(),
                               rFont.GetName(),
                               rFont.GetStyleName(),
                               rFont.GetPitch(),
                               rFont.GetCharSet(),
                               ATTR_FONT );

    //  if string contains WEAK characters, set all fonts
    BYTE nScript;
    ScDocument* pDoc = GetViewData()->GetDocument();
    if ( pDoc->HasStringWeakCharacters( rStr ) )
        nScript = SCRIPTTYPE_LATIN | SCRIPTTYPE_ASIAN | SCRIPTTYPE_COMPLEX;
    else
        nScript = pDoc->GetStringScriptType( rStr );

    SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, pViewShell->GetPool() );
    aSetItem.PutItemForScriptType( nScript, aFontItem );
    ApplyUserItemSet( aSetItem.GetItemSet() );

    while ( *pChar )
        pViewShell->TabKeyInput( KeyEvent( *(pChar++), KeyCode() ) );
}


//----------------------------------------------------------------------------

void ScViewFunc::UpdateLineAttrs( SvxBorderLine&       rLine,
                                  const SvxBorderLine* pDestLine,
                                  const SvxBorderLine* pSrcLine,
                                  BOOL                 bColor )
{
    if ( pSrcLine && pDestLine )
    {
        if ( bColor )
        {
            rLine.SetColor      ( pSrcLine->GetColor() );
            rLine.SetOutWidth   ( pDestLine->GetOutWidth() );
            rLine.SetInWidth    ( pDestLine->GetInWidth() );
            rLine.SetDistance   ( pDestLine->GetDistance() );
            rLine.SetStyle      ( pDestLine->GetStyle() );
        }
        else
        {
            rLine.SetColor      ( pDestLine->GetColor() );
            rLine.SetOutWidth   ( pSrcLine->GetOutWidth() );
            rLine.SetInWidth    ( pSrcLine->GetInWidth() );
            rLine.SetDistance   ( pSrcLine->GetDistance() );
            rLine.SetStyle      ( pSrcLine->GetStyle() );
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
            aBoxItem.SetLine( NULL, BOXLINE );                      \
    }


//----------------------------------------------------------------------------

void ScViewFunc::SetSelectionFrameLines( const SvxBorderLine* pLine,
                                         BOOL bColorOnly )
{
    // nur wegen Matrix nicht editierbar? Attribute trotzdem ok
    BOOL bOnlyNotBecauseOfMatrix;
    if ( !SelectionEditable( &bOnlyNotBecauseOfMatrix ) && !bOnlyNotBecauseOfMatrix )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    ScDocument*             pDoc = GetViewData()->GetDocument();
    ScMarkData aFuncMark( GetViewData()->GetMarkData() );       // local copy for UnmarkFiltered
    ScViewUtil::UnmarkFiltered( aFuncMark, pDoc );
    ScDocShell*             pDocSh = GetViewData()->GetDocShell();
    const ScPatternAttr*    pSelAttrs = GetSelectionPattern();
    const SfxItemSet&       rSelItemSet = pSelAttrs->GetItemSet();

    const SfxPoolItem*      pBorderAttr = NULL;
    SfxItemState            eItemState = rSelItemSet.GetItemState( ATTR_BORDER, TRUE, &pBorderAttr );

    const SfxPoolItem*      pTLBRItem = 0;
    SfxItemState            eTLBRState = rSelItemSet.GetItemState( ATTR_BORDER_TLBR, TRUE, &pTLBRItem );

    const SfxPoolItem*      pBLTRItem = 0;
    SfxItemState            eBLTRState = rSelItemSet.GetItemState( ATTR_BORDER_BLTR, TRUE, &pBLTRItem );

    // any of the lines visible?
    if( (eItemState != SFX_ITEM_DEFAULT) || (eTLBRState != SFX_ITEM_DEFAULT) || (eBLTRState != SFX_ITEM_DEFAULT) )
    {
        // none of the lines don't care?
        if( (eItemState != SFX_ITEM_DONTCARE) && (eTLBRState != SFX_ITEM_DONTCARE) && (eBLTRState != SFX_ITEM_DONTCARE) )
        {
            SfxItemSet*     pOldSet = new SfxItemSet(
                                            *(pDoc->GetPool()),
                                            ATTR_PATTERN_START,
                                            ATTR_PATTERN_END );
            SfxItemSet*     pNewSet = new SfxItemSet(
                                            *(pDoc->GetPool()),
                                            ATTR_PATTERN_START,
                                            ATTR_PATTERN_END );

            //------------------------------------------------------------
            const SvxBorderLine*    pBoxLine = NULL;
            SvxBorderLine           aLine;

            // hier wird die pBoxLine benutzt:

            if( pBorderAttr )
            {
                SvxBoxItem      aBoxItem( *(const SvxBoxItem*)pBorderAttr );
                SvxBoxInfoItem  aBoxInfoItem( ATTR_BORDER_INNER );

                SET_LINE_ATTRIBUTES(Top,BOX_LINE_TOP)
                SET_LINE_ATTRIBUTES(Bottom,BOX_LINE_BOTTOM)
                SET_LINE_ATTRIBUTES(Left,BOX_LINE_LEFT)
                SET_LINE_ATTRIBUTES(Right,BOX_LINE_RIGHT)

                aBoxInfoItem.SetLine( aBoxItem.GetTop(), BOXINFO_LINE_HORI );
                aBoxInfoItem.SetLine( aBoxItem.GetLeft(), BOXINFO_LINE_VERT );
                aBoxInfoItem.ResetFlags(); // Lines auf Valid setzen

                pOldSet->Put( *pBorderAttr );
                pNewSet->Put( aBoxItem );
                pNewSet->Put( aBoxInfoItem );
            }

            if( pTLBRItem && ((const SvxLineItem*)pTLBRItem)->GetLine() )
            {
                SvxLineItem aTLBRItem( *(const SvxLineItem*)pTLBRItem );
                UpdateLineAttrs( aLine, aTLBRItem.GetLine(), pLine, bColorOnly );
                aTLBRItem.SetLine( &aLine );
                pOldSet->Put( *pTLBRItem );
                pNewSet->Put( aTLBRItem );
            }

            if( pBLTRItem && ((const SvxLineItem*)pBLTRItem)->GetLine() )
            {
                SvxLineItem aBLTRItem( *(const SvxLineItem*)pBLTRItem );
                UpdateLineAttrs( aLine, aBLTRItem.GetLine(), pLine, bColorOnly );
                aBLTRItem.SetLine( &aLine );
                pOldSet->Put( *pBLTRItem );
                pNewSet->Put( aBLTRItem );
            }

            ApplyAttributes( pNewSet, pOldSet );

            delete pOldSet;
            delete pNewSet;
        }
        else // if ( eItemState == SFX_ITEM_DONTCARE )
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
                           PAINT_GRID, SC_PF_LINES | SC_PF_TESTMERGE );

        pDocSh->UpdateOle( GetViewData() );
        pDocSh->SetDocumentModified();
    }
}

#undef SET_LINE_ATTRIBUTES


//----------------------------------------------------------------------------

void ScViewFunc::SetConditionalFormat( const ScConditionalFormat& rNew )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ULONG nIndex = pDoc->AddCondFormat(rNew);           // dafuer gibt's kein Undo
    SfxUInt32Item aItem( ATTR_CONDITIONAL, nIndex );

    ApplyAttr( aItem );         // mit Paint und Undo...
}


//----------------------------------------------------------------------------

void ScViewFunc::SetValidation( const ScValidationData& rNew )
{
    ScDocument* pDoc = GetViewData()->GetDocument();
    ULONG nIndex = pDoc->AddValidationEntry(rNew);      // dafuer gibt's kein Undo
    SfxUInt32Item aItem( ATTR_VALIDDATA, nIndex );

    ApplyAttr( aItem );         // mit Paint und Undo...
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
