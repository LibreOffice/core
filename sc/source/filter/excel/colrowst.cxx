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

#include "colrowst.hxx"

#include <string.h>

#include "document.hxx"
#include "root.hxx"
#include "ftools.hxx"
#include "xltable.hxx"
#include "xistream.hxx"
#include "xistyle.hxx"
#include "queryparam.hxx"

// for filter manager
#include "excimp8.hxx"

// ============================================================================

const sal_uInt8 EXC_COLROW_USED         = 0x01;
const sal_uInt8 EXC_COLROW_DEFAULT      = 0x02;
const sal_uInt8 EXC_COLROW_HIDDEN       = 0x04;
const sal_uInt8 EXC_COLROW_MAN          = 0x08;

// ============================================================================

XclImpColRowSettings::XclImpColRowSettings( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maWidths( MAXCOLCOUNT, 0 ),
    maColFlags( MAXCOLCOUNT, 0 ),
    maRowHeights(0, MAXROWCOUNT, 0),
    maRowFlags(0, MAXROWCOUNT, 0),
    maHiddenRows(0, MAXROWCOUNT, false),
    mnLastScRow( -1 ),
    mnDefWidth( STD_COL_WIDTH ),
    mnDefHeight( static_cast< sal_uInt16 >( STD_ROW_HEIGHT ) ),
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

void XclImpColRowSettings::SetWidthRange( SCCOL nScCol1, SCCOL nScCol2, sal_uInt16 nWidth )
{
    OSL_ENSURE( (nScCol1 <= nScCol2) && ValidCol( nScCol2 ), "XclImpColRowSettings::SetColWidthRange - invalid column range" );
    nScCol2 = ::std::min( nScCol2, MAXCOL );
    if (nScCol2 == 256)
        // In BIFF8, the column range is 0-255, and the use of 256 probably
        // means the range should extend to the max column if the loading app
        // support columns beyond 255.
        nScCol2 = MAXCOL;

    nScCol1 = ::std::min( nScCol1, nScCol2 );
    ::std::fill( maWidths.begin() + nScCol1, maWidths.begin() + nScCol2 + 1, nWidth );
    for( ScfUInt8Vec::iterator aIt = maColFlags.begin() + nScCol1, aEnd = maColFlags.begin() + nScCol2 + 1; aIt != aEnd; ++aIt )
        ::set_flag( *aIt, EXC_COLROW_USED );
}

void XclImpColRowSettings::HideCol( SCCOL nScCol )
{
    if( ValidCol( nScCol ) )
        ::set_flag( maColFlags[ nScCol ], EXC_COLROW_HIDDEN );
}

void XclImpColRowSettings::HideColRange( SCCOL nScCol1, SCCOL nScCol2 )
{
    OSL_ENSURE( (nScCol1 <= nScCol2) && ValidCol( nScCol2 ), "XclImpColRowSettings::HideColRange - invalid column range" );
    nScCol2 = ::std::min( nScCol2, MAXCOL );
    nScCol1 = ::std::min( nScCol1, nScCol2 );
    for( ScfUInt8Vec::iterator aIt = maColFlags.begin() + nScCol1, aEnd = maColFlags.begin() + nScCol2 + 1; aIt != aEnd; ++aIt )
        ::set_flag( *aIt, EXC_COLROW_HIDDEN );
}

void XclImpColRowSettings::SetDefHeight( sal_uInt16 nDefHeight, sal_uInt16 nFlags )
{
    mnDefHeight = nDefHeight;
    mnDefRowFlags = nFlags;
    if( mnDefHeight == 0 )
    {
        mnDefHeight = static_cast< sal_uInt16 >( STD_ROW_HEIGHT );
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
    sal_uInt8 nFlagVal = 0;
    if (!maRowFlags.search(nScRow, nFlagVal).second)
        return;

    ::set_flag(nFlagVal, EXC_COLROW_USED);
    ::set_flag(nFlagVal, EXC_COLROW_DEFAULT, bDefHeight);

    if (!bDefHeight && nRawHeight == 0)
        maHiddenRows.insert_back(nScRow, nScRow+1, true);

    maRowFlags.insert_back(nScRow, nScRow+1, nFlagVal);

    if (nScRow > mnLastScRow)
        mnLastScRow = nScRow;
}

void XclImpColRowSettings::SetRowSettings( SCROW nScRow, sal_uInt16 nHeight, sal_uInt16 nFlags )
{
    if (!ValidRow(nScRow))
        return;

    SetHeight(nScRow, nHeight);

    sal_uInt8 nFlagVal = 0;
    if (!maRowFlags.search(nScRow, nFlagVal).second)
        return;

    if (::get_flag(nFlags, EXC_ROW_UNSYNCED))
        ::set_flag(nFlagVal, EXC_COLROW_MAN);

    maRowFlags.insert_back(nScRow, nScRow+1, nFlagVal);

    if (::get_flag(nFlags, EXC_ROW_HIDDEN))
        maHiddenRows.insert_back(nScRow, nScRow+1, true);
}

void XclImpColRowSettings::SetManualRowHeight( SCROW nScRow )
{
    if (!ValidRow(nScRow))
        return;

    sal_uInt8 nFlagVal = 0;
    if (!maRowFlags.search(nScRow, nFlagVal).second)
        return;

    ::set_flag(nFlagVal, EXC_COLROW_MAN);
    maRowFlags.insert_back(nScRow, nScRow+1, nFlagVal);
}

void XclImpColRowSettings::SetDefaultXF( SCCOL nScCol1, SCCOL nScCol2, sal_uInt16 nXFIndex )
{
    /*  assign the default column formatting here to ensure that
        explicit cell formatting is not overwritten. */
    OSL_ENSURE( (nScCol1 <= nScCol2) && ValidCol( nScCol2 ), "XclImpColRowSettings::SetDefaultXF - invalid column index" );
    nScCol2 = ::std::min( nScCol2, MAXCOL );
    nScCol1 = ::std::min( nScCol1, nScCol2 );
    XclImpXFRangeBuffer& rXFRangeBuffer = GetXFRangeBuffer();
    for( SCCOL nScCol = nScCol1; nScCol <= nScCol2; ++nScCol )
        rXFRangeBuffer.SetColumnDefXF( nScCol, nXFIndex );
}

void XclImpColRowSettings::Convert( SCTAB nScTab )
{
    if( !mbDirty )
        return;

    ScDocument& rDoc = GetDoc();

    // column widths ----------------------------------------------------------

    for( SCCOL nScCol = 0; nScCol <= MAXCOL; ++nScCol )
    {
        sal_uInt16 nWidth = ::get_flag( maColFlags[ nScCol ], EXC_COLROW_USED ) ? maWidths[ nScCol ] : mnDefWidth;
        /*  Hidden columns: remember hidden state, but do not set hidden state
            in document here. Needed for #i11776#, no HIDDEN flags in the
            document, until filters and outlines are inserted. */
        if( nWidth == 0 )
        {
            ::set_flag( maColFlags[ nScCol ], EXC_COLROW_HIDDEN );
            nWidth = mnDefWidth;
        }
        rDoc.SetColWidthOnly( nScCol, nScTab, nWidth );
    }

    // row heights ------------------------------------------------------------

    // #i54252# set default row height
    rDoc.SetRowHeightOnly( 0, MAXROW, nScTab, mnDefHeight );
    if( ::get_flag( mnDefRowFlags, EXC_DEFROW_UNSYNCED ) )
        // first access to row flags, do not ask for old flags
        rDoc.SetRowFlags( 0, MAXROW, nScTab, CR_MANUALSIZE );

    maRowHeights.build_tree();
    if (!maRowHeights.is_tree_valid())
        return;

    RowFlagsType::const_iterator itrFlags = maRowFlags.begin(), itrFlagsEnd = maRowFlags.end();
    SCROW nPrevRow = -1;
    sal_uInt8 nPrevFlags = 0;
    for (; itrFlags != itrFlagsEnd; ++itrFlags)
    {
        SCROW nRow = itrFlags->first;
        sal_uInt8 nFlags = itrFlags->second;
        if (nPrevRow >= 0)
        {
            sal_uInt16 nHeight = 0;

            if (::get_flag(nPrevFlags, EXC_COLROW_USED))
            {
                if (::get_flag(nPrevFlags, EXC_COLROW_DEFAULT))
                {
                    nHeight = mnDefHeight;
                    rDoc.SetRowHeightOnly(nPrevRow, nRow-1, nScTab, nHeight);
                }
                else
                {
                    for (SCROW i = nPrevRow; i <= nRow - 1; ++i)
                    {
                        SCROW nLast;
                        if (!maRowHeights.search_tree(i, nHeight, NULL, &nLast))
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

                if (::get_flag(nPrevFlags, EXC_COLROW_MAN))
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

    // ------------------------------------------------------------------------

    mbDirty = false;
}

void XclImpColRowSettings::ConvertHiddenFlags( SCTAB nScTab )
{
    ScDocument& rDoc = GetDoc();

    // hide the columns
    for( SCCOL nScCol = 0; nScCol <= MAXCOL; ++nScCol )
        if( ::get_flag( maColFlags[ nScCol ], EXC_COLROW_HIDDEN ) )
            rDoc.ShowCol( nScCol, nScTab, false );

    // #i38093# rows hidden by filter need extra flag
    SCROW nFirstFilterScRow = SCROW_MAX;
    SCROW nLastFilterScRow = SCROW_MAX;
    if( GetBiff() == EXC_BIFF8 )
    {
        const XclImpAutoFilterData* pFilter = GetFilterManager().GetByTab( nScTab );
        // #i70026# use IsFiltered() to set the CR_FILTERED flag for active filters only
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
    RowHiddenType::const_iterator itr = maHiddenRows.begin(), itrEnd = maHiddenRows.end();
    for (; itr != itrEnd; ++itr)
    {
        SCROW nRow = itr->first;
        bool bHidden = itr->second;
        if (nPrevRow >= 0)
        {
            if (bPrevHidden)
            {
                rDoc.SetRowHidden(nPrevRow, nRow-1, nScTab, true);        // #i116460# SetRowHidden instead of ShowRow
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
