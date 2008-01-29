/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colrowst.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:23:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "colrowst.hxx"

#include <string.h>

#include "document.hxx"
#include "root.hxx"

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif
#ifndef SC_XLTABLE_HXX
#include "xltable.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif

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
    maHeights( MAXROWCOUNT, 0 ),
    maRowFlags( MAXROWCOUNT, 0 ),
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
    DBG_ASSERT( (nScCol1 <= nScCol2) && ValidCol( nScCol2 ), "XclImpColRowSettings::SetColWidthRange - invalid column range" );
    nScCol2 = ::std::min( nScCol2, MAXCOL );
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
    DBG_ASSERT( (nScCol1 <= nScCol2) && ValidCol( nScCol2 ), "XclImpColRowSettings::HideColRange - invalid column range" );
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
    if( ValidRow( nScRow ) )
    {
        sal_uInt16 nRawHeight = nHeight & EXC_ROW_HEIGHTMASK;
        bool bDefHeight = ::get_flag( nHeight, EXC_ROW_FLAGDEFHEIGHT ) || (nRawHeight == 0);
        maHeights[ nScRow ] = nRawHeight;
        sal_uInt8& rnFlags = maRowFlags[ nScRow ];
        ::set_flag( rnFlags, EXC_COLROW_USED );
        if( !bDefHeight && (nRawHeight == 0) )
            ::set_flag( rnFlags, EXC_COLROW_HIDDEN );
        ::set_flag( rnFlags, EXC_COLROW_DEFAULT, bDefHeight );
        if( nScRow > mnLastScRow )
            mnLastScRow = nScRow;
    }
}

void XclImpColRowSettings::HideRow( SCROW nScRow )
{
    if( ValidRow( nScRow ) )
    {
        ::set_flag( maRowFlags[ nScRow ], static_cast< sal_uInt8 >( EXC_COLROW_USED | EXC_COLROW_HIDDEN ) );
        if( nScRow > mnLastScRow )
            mnLastScRow = nScRow;
    }
}

void XclImpColRowSettings::SetRowSettings( SCROW nScRow, sal_uInt16 nHeight, sal_uInt16 nFlags )
{
    if( ValidRow( nScRow ) )
    {
        SetHeight( nScRow, nHeight );
        sal_uInt8& rnFlags = maRowFlags[ nScRow ];
        if( ::get_flag( nFlags, EXC_ROW_UNSYNCED ) )
            ::set_flag( rnFlags, EXC_COLROW_MAN );
        if( ::get_flag( nFlags, EXC_ROW_HIDDEN ) )
            ::set_flag( rnFlags, EXC_COLROW_HIDDEN );
    }
}

void XclImpColRowSettings::SetDefaultXF( SCCOL nScCol1, SCCOL nScCol2, sal_uInt16 nXFIndex )
{
    /*  #109555# assign the default column formatting here to ensure that
        explicit cell formatting is not overwritten. */
    DBG_ASSERT( (nScCol1 <= nScCol2) && ValidCol( nScCol2 ), "XclImpColRowSettings::SetDefaultXF - invalid column index" );
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
    rDoc.IncSizeRecalcLevel( nScTab );

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
        rDoc.SetColWidth( nScCol, nScTab, nWidth );
    }

    // row heights ------------------------------------------------------------

    // #i54252# set default row height
    rDoc.SetRowHeightRange( 0, MAXROW, nScTab, mnDefHeight );
    if( ::get_flag( mnDefRowFlags, EXC_DEFROW_UNSYNCED ) )
        // first access to row flags, do not ask for old flags
        rDoc.SetRowFlags( 0, MAXROW, nScTab, CR_MANUALSIZE );
    bool bDefHideRow = ::get_flag( mnDefRowFlags, EXC_DEFROW_HIDDEN );

    SCROW nFirstScRow = -1;
    sal_uInt16 nLastHeight = 0;
    for( SCROW nScRow = 0; nScRow <= mnLastScRow ; ++nScRow )
    {
        // get height and hidden state from cached data
        sal_uInt8 nFlags = maRowFlags[ nScRow ];
        sal_uInt16 nHeight = 0;
        bool bHideRow = false;
        if( ::get_flag( nFlags, EXC_COLROW_USED ) )
        {
            if( ::get_flag( nFlags, EXC_COLROW_DEFAULT ) )
            {
                nHeight = mnDefHeight;
                bHideRow = bDefHideRow;
            }
            else
            {
                nHeight = maHeights[ nScRow ];
                if( nHeight == 0 )
                {
                    nHeight = mnDefHeight;
                    bHideRow = true;
                }
            }

            if( ::get_flag( nFlags, EXC_COLROW_MAN ) )
                rDoc.SetRowFlags( nScRow, nScTab, rDoc.GetRowFlags( nScRow, nScTab ) | CR_MANUALSIZE );
        }
        else
        {
            nHeight = mnDefHeight;
            bHideRow = bDefHideRow;
        }

        /*  Hidden rows: remember hidden state, but do not set hidden state in
            document here. Needed for #i11776#, no HIDDEN flags in the document,
            until filters and outlines are inserted. */
        if( bHideRow )
            ::set_flag( maRowFlags[ nScRow ], EXC_COLROW_HIDDEN );

        // set height range
        if( (nLastHeight != nHeight) || (nScRow == 0) )
        {
            DBG_ASSERT( (nScRow == 0) || (nFirstScRow >= 0), "XclImpColRowSettings::Convert - algorithm error" );
            if( nScRow > 0 )
                rDoc.SetRowHeightRange( nFirstScRow, nScRow - 1, nScTab, nLastHeight );

            nFirstScRow = nScRow;
            nLastHeight = nHeight;
        }
    }

    // set row height of last portion
    if( mnLastScRow >= 0 )
        rDoc.SetRowHeightRange( nFirstScRow, mnLastScRow, nScTab, nLastHeight );

    // ------------------------------------------------------------------------

    mbDirty = false;
    rDoc.DecSizeRecalcLevel( nScTab );
}

void XclImpColRowSettings::ConvertHiddenFlags( SCTAB nScTab )
{
    ScDocument& rDoc = GetDoc();

    // hide the columns
    for( SCCOL nScCol = 0; nScCol <= MAXCOL; ++nScCol )
        if( ::get_flag( maColFlags[ nScCol ], EXC_COLROW_HIDDEN ) )
            rDoc.ShowCol( nScCol, nScTab, FALSE );

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

    // hide the rows
    for( SCROW nScRow = 0; nScRow <= mnLastScRow; ++nScRow )
    {
        if( ::get_flag( maRowFlags[ nScRow ], EXC_COLROW_HIDDEN ) )
        {
            // hide the row
            rDoc.ShowRow( nScRow, nScTab, FALSE );
            // #i38093# rows hidden by filter need extra flag
            if( (nFirstFilterScRow <= nScRow) && (nScRow <= nLastFilterScRow) )
                rDoc.SetRowFlags( nScRow, nScTab, rDoc.GetRowFlags( nScRow, nScTab ) | CR_FILTERED );
        }
    }

    // #i47438# if default row format is hidden, hide remaining rows
    if( ::get_flag( mnDefRowFlags, EXC_DEFROW_HIDDEN ) && (mnLastScRow < MAXROW) )
        rDoc.ShowRows( mnLastScRow + 1, MAXROW, nScTab, FALSE );
}

