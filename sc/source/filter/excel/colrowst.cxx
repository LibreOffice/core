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

#include <colrowst.hxx>

#include <document.hxx>
#include <ftools.hxx>
#include <xltable.hxx>
#include <xistyle.hxx>
#include <excimp8.hxx>
#include <table.hxx>

XclImpColRowSettings::XclImpColRowSettings( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maColWidths(0, MAXCOLCOUNT, 0),
    maColFlags(0, MAXCOLCOUNT, ExcColRowFlags::NONE),
    maRowHeights(0, MAXROWCOUNT, 0),
    maRowFlags(0, MAXROWCOUNT, ExcColRowFlags::NONE),
    maHiddenRows(0, MAXROWCOUNT, false),
    mnLastScRow( -1 ),
    mnDefWidth( STD_COL_WIDTH ),
    mnDefHeight( ScGlobal::nStdRowHeight ),
    mnDefRowFlags( EXC_DEFROW_DEFAULTFLAGS ),
    mbHasStdWidthRec( false ),
    mbHasDefHeight( false ),
    mbDirty( true )
{
}

XclImpColRowSettings::~XclImpColRowSettings()
{
}

void XclImpColRowSettings::SetDefWidth( sal_uInt16 nDefWidth, bool bStdWidthRec )
{
    if( bStdWidthRec )
    {
        // STANDARDWIDTH record overrides DEFCOLWIDTH record
        mnDefWidth = nDefWidth;
        mbHasStdWidthRec = true;
    }
    else if( !mbHasStdWidthRec )
    {
        // use DEFCOLWIDTH record only, if no STANDARDWIDTH record exists
        mnDefWidth = nDefWidth;
    }
}

void XclImpColRowSettings::SetWidthRange( SCCOL nCol1, SCCOL nCol2, sal_uInt16 nWidth )
{
    nCol2 = ::std::min( nCol2, MAXCOL );
    if (nCol2 == 256)
        // In BIFF8, the column range is 0-255, and the use of 256 probably
        // means the range should extend to the max column if the loading app
        // support columns beyond 255.
        nCol2 = MAXCOL;

    nCol1 = ::std::min( nCol1, nCol2 );
    maColWidths.insert_back(nCol1, nCol2+1, nWidth);

    // We need to apply flag values individually since all flag values are aggregated for each column.
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        ApplyColFlag(nCol, ExcColRowFlags::Used);
}

void XclImpColRowSettings::HideCol( SCCOL nCol )
{
    if (!ValidCol(nCol))
        return;

    ApplyColFlag(nCol, ExcColRowFlags::Hidden);
}

void XclImpColRowSettings::HideColRange( SCCOL nCol1, SCCOL nCol2 )
{
    nCol2 = ::std::min( nCol2, MAXCOL );
    nCol1 = ::std::min( nCol1, nCol2 );

    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        ApplyColFlag(nCol, ExcColRowFlags::Hidden);
}

void XclImpColRowSettings::SetDefHeight( sal_uInt16 nDefHeight, sal_uInt16 nFlags )
{
    mnDefHeight = nDefHeight;
    mnDefRowFlags = nFlags;
    if( mnDefHeight == 0 )
    {
        mnDefHeight = ScGlobal::nStdRowHeight;
        ::set_flag( mnDefRowFlags, EXC_DEFROW_HIDDEN );
    }
    mbHasDefHeight = true;
}

void XclImpColRowSettings::SetHeight( SCROW nScRow, sal_uInt16 nHeight )
{
    if (!ValidRow(nScRow))
        return;

    sal_uInt16 nRawHeight = nHeight & EXC_ROW_HEIGHTMASK;
    bool bDefHeight = ::get_flag( nHeight, EXC_ROW_FLAGDEFHEIGHT ) || (nRawHeight == 0);
    maRowHeights.insert_back(nScRow, nScRow+1, nRawHeight);
    ExcColRowFlags nFlagVal = ExcColRowFlags::NONE;
    if (!maRowFlags.search(nScRow, nFlagVal).second)
        return;

    ::set_flag(nFlagVal, ExcColRowFlags::Used);
    ::set_flag(nFlagVal, ExcColRowFlags::Default, bDefHeight);

    maRowFlags.insert_back(nScRow, nScRow+1, nFlagVal);

    if (nScRow > mnLastScRow)
        mnLastScRow = nScRow;
}

void XclImpColRowSettings::SetRowSettings( SCROW nScRow, sal_uInt16 nHeight, sal_uInt16 nFlags )
{
    if (!ValidRow(nScRow))
        return;

    SetHeight(nScRow, nHeight);

    ExcColRowFlags nFlagVal = ExcColRowFlags::NONE;
    if (!maRowFlags.search(nScRow, nFlagVal).second)
        return;

    if (::get_flag(nFlags, EXC_ROW_UNSYNCED))
        ::set_flag(nFlagVal, ExcColRowFlags::Man);

    maRowFlags.insert_back(nScRow, nScRow+1, nFlagVal);

    if (::get_flag(nFlags, EXC_ROW_HIDDEN))
        maHiddenRows.insert_back(nScRow, nScRow+1, true);
}

void XclImpColRowSettings::SetManualRowHeight( SCROW nScRow )
{
    if (!ValidRow(nScRow))
        return;

    ExcColRowFlags nFlagVal = ExcColRowFlags::NONE;
    if (!maRowFlags.search(nScRow, nFlagVal).second)
        return;

    nFlagVal |= ExcColRowFlags::Man;
    maRowFlags.insert_back(nScRow, nScRow+1, nFlagVal);
}

void XclImpColRowSettings::SetDefaultXF( SCCOL nCol1, SCCOL nCol2, sal_uInt16 nXFIndex )
{
    /*  assign the default column formatting here to ensure that
        explicit cell formatting is not overwritten. */
    OSL_ENSURE( (nCol1 <= nCol2) && ValidCol( nCol2 ), "XclImpColRowSettings::SetDefaultXF - invalid column index" );
    nCol2 = ::std::min( nCol2, MAXCOL );
    nCol1 = ::std::min( nCol1, nCol2 );
    XclImpXFRangeBuffer& rXFRangeBuffer = GetXFRangeBuffer();
    for( SCCOL nCol = nCol1; nCol <= nCol2; ++nCol )
        rXFRangeBuffer.SetColumnDefXF( nCol, nXFIndex );
}

void XclImpColRowSettings::Convert( SCTAB nScTab )
{
    if( !mbDirty )
        return;

    ScDocument& rDoc = GetDoc();

    // column widths ----------------------------------------------------------

    maColWidths.build_tree();
    for( SCCOL nCol : rDoc.GetColumnsRange(nScTab, 0, MAXCOL) )
    {
        sal_uInt16 nWidth = mnDefWidth;
        if (GetColFlag(nCol, ExcColRowFlags::Used))
        {
            sal_uInt16 nTmp;
            if (maColWidths.search_tree(nCol, nTmp).second)
                nWidth = nTmp;
        }

        /*  Hidden columns: remember hidden state, but do not set hidden state
            in document here. Needed for #i11776#, no HIDDEN flags in the
            document, until filters and outlines are inserted. */
        if( nWidth == 0 )
        {
            ApplyColFlag(nCol, ExcColRowFlags::Hidden);
            nWidth = mnDefWidth;
        }
        rDoc.SetColWidthOnly( nCol, nScTab, nWidth );
    }

    // row heights ------------------------------------------------------------

    // #i54252# set default row height
    rDoc.SetRowHeightOnly( 0, MAXROW, nScTab, mnDefHeight );
    if( ::get_flag( mnDefRowFlags, EXC_DEFROW_UNSYNCED ) )
        // first access to row flags, do not ask for old flags
        rDoc.SetRowFlags( 0, MAXROW, nScTab, CRFlags::ManualSize );

    maRowHeights.build_tree();
    if (!maRowHeights.is_tree_valid())
        return;

    SCROW nPrevRow = -1;
    ExcColRowFlags nPrevFlags = ExcColRowFlags::NONE;
    for (const auto& [nRow, nFlags] : maRowFlags)
    {
        if (nPrevRow >= 0)
        {
            sal_uInt16 nHeight = 0;

            if (nPrevFlags & ExcColRowFlags::Used)
            {
                if (nPrevFlags & ExcColRowFlags::Default)
                {
                    nHeight = mnDefHeight;
                    rDoc.SetRowHeightOnly(nPrevRow, nRow-1, nScTab, nHeight);
                }
                else
                {
                    for (SCROW i = nPrevRow; i <= nRow - 1; ++i)
                    {
                        SCROW nLast;
                        if (!maRowHeights.search_tree(i, nHeight, nullptr, &nLast).second)
                        {
                            // search failed for some reason
                            return;
                        }

                        if (nLast > nRow)
                            nLast = nRow;

                        rDoc.SetRowHeightOnly(i, nLast-1, nScTab, nHeight);
                        i = nLast-1;
                    }
                }

                if (nPrevFlags & ExcColRowFlags::Man)
                    rDoc.SetManualHeight(nPrevRow, nRow-1, nScTab, true);
            }
            else
            {
                nHeight = mnDefHeight;
                rDoc.SetRowHeightOnly(nPrevRow, nRow-1, nScTab, nHeight);
            }
        }

        nPrevRow = nRow;
        nPrevFlags = nFlags;
    }

    mbDirty = false;
}

void XclImpColRowSettings::ConvertHiddenFlags( SCTAB nScTab )
{
    ScDocument& rDoc = GetDoc();

    // hide the columns
    for( SCCOL nCol : rDoc.GetColumnsRange(nScTab, 0, MAXCOL) )
        if (GetColFlag(nCol, ExcColRowFlags::Hidden))
            rDoc.ShowCol( nCol, nScTab, false );

    // #i38093# rows hidden by filter need extra flag
    SCROW nFirstFilterScRow = SCROW_MAX;
    SCROW nLastFilterScRow = SCROW_MAX;
    if( GetBiff() == EXC_BIFF8 )
    {
        const XclImpAutoFilterData* pFilter = GetFilterManager().GetByTab( nScTab );
        // #i70026# use IsFiltered() to set the CRFlags::Filtered flag for active filters only
        if( pFilter && pFilter->IsActive() && pFilter->IsFiltered() )
        {
            nFirstFilterScRow = pFilter->StartRow();
            nLastFilterScRow = pFilter->EndRow();
        }
    }

    // In case the excel row limit is lower than calc's, use the visibility of
    // the last row and extend it to calc's last row.
    SCROW nLastXLRow = GetRoot().GetXclMaxPos().Row();
    if (nLastXLRow < MAXROW)
    {
        bool bHidden = false;
        if (!maHiddenRows.search(nLastXLRow, bHidden).second)
            return;

        maHiddenRows.insert_back(nLastXLRow, MAXROWCOUNT, bHidden);
    }

    SCROW nPrevRow = -1;
    bool bPrevHidden = false;
    for (const auto& [nRow, bHidden] : maHiddenRows)
    {
        if (nPrevRow >= 0)
        {
            if (bPrevHidden)
            {
                rDoc.SetRowHidden(nPrevRow, nRow-1, nScTab, true);
                // #i38093# rows hidden by filter need extra flag
                if (nFirstFilterScRow <= nPrevRow && nPrevRow <= nLastFilterScRow)
                {
                    SCROW nLast = ::std::min(nRow-1, nLastFilterScRow);
                    rDoc.SetRowFiltered(nPrevRow, nLast, nScTab, true);
                }
            }
        }

        nPrevRow = nRow;
        bPrevHidden = bHidden;
    }

    // #i47438# if default row format is hidden, hide remaining rows
    if( ::get_flag( mnDefRowFlags, EXC_DEFROW_HIDDEN ) && (mnLastScRow < MAXROW) )
        rDoc.ShowRows( mnLastScRow + 1, MAXROW, nScTab, false );
}

void XclImpColRowSettings::ApplyColFlag(SCCOL nCol, ExcColRowFlags nNewVal)
{
    // Get the original flag value.
    ExcColRowFlags nFlagVal = ExcColRowFlags::NONE;
    std::pair<ColRowFlagsType::const_iterator,bool> r = maColFlags.search(nCol, nFlagVal);
    if (!r.second)
        // Search failed.
        return;

    ::set_flag(nFlagVal, nNewVal);

    // Re-insert the flag value.
    maColFlags.insert(r.first, nCol, nCol+1, nFlagVal);
}

bool XclImpColRowSettings::GetColFlag(SCCOL nCol, ExcColRowFlags nMask) const
{
    ExcColRowFlags nFlagVal = ExcColRowFlags::NONE;
    if (!maColFlags.search(nCol, nFlagVal).second)
        return false;
        // Search failed.

    return bool(nFlagVal & nMask);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
