/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeview.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:05:34 $
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

#ifndef SC_XEVIEW_HXX
#include "xeview.hxx"
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCEXTOPT_HXX
#include "scextopt.hxx"
#endif
#ifndef SC_VIEWOPTI_HXX
#include "viewopti.hxx"
#endif

#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif

// Workbook view settings records =============================================

XclExpWindow1::XclExpWindow1( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_WINDOW1, 18 ),
    mnFlags( 0 ),
    mnTabBarSize( 600 )
{
    const ScViewOptions& rViewOpt = rRoot.GetDoc().GetViewOptions();
    ::set_flag( mnFlags, EXC_WIN1_HOR_SCROLLBAR,    rViewOpt.GetOption( VOPT_HSCROLL ) );
    ::set_flag( mnFlags, EXC_WIN1_VER_SCROLLBAR,    rViewOpt.GetOption( VOPT_VSCROLL ) );
    ::set_flag( mnFlags, EXC_WIN1_TABBAR,           rViewOpt.GetOption( VOPT_TABCONTROLS ) );

    double fTabBarWidth = rRoot.GetExtDocOptions().GetDocSettings().mfTabBarWidth;
    if( (0.0 <= fTabBarWidth) && (fTabBarWidth <= 1.0) )
        mnTabBarSize = static_cast< sal_uInt16 >( fTabBarWidth * 1000.0 + 0.5 );
}

void XclExpWindow1::WriteBody( XclExpStream& rStrm )
{
    const XclExpTabInfo& rTabInfo = rStrm.GetRoot().GetTabInfo();

    rStrm   << sal_uInt16( 0 )              // X position of the window
            << sal_uInt16( 0 )              // Y position of the window
            << sal_uInt16( 0x4000 )         // width of the window
            << sal_uInt16( 0x2000 )         // height of the window
            << mnFlags
            << rTabInfo.GetDisplayedXclTab()
            << rTabInfo.GetFirstVisXclTab()
            << rTabInfo.GetXclSelectedCount()
            << mnTabBarSize;
}

// Sheet view settings records ================================================

XclExpWindow2::XclExpWindow2( const XclExpRoot& rRoot,
        const XclTabViewData& rData, sal_uInt32 nGridColorId ) :
    XclExpRecord( EXC_ID_WINDOW2, (rRoot.GetBiff() == EXC_BIFF8) ? 18 : 10 ),
    maGridColor( rData.maGridColor ),
    mnGridColorId( nGridColorId ),
    mnFlags( 0 ),
    maFirstXclPos( rData.maFirstXclPos ),
    mnNormalZoom( rData.mnNormalZoom ),
    mnPageZoom( rData.mnPageZoom )
{
    ::set_flag( mnFlags, EXC_WIN2_SHOWFORMULAS,     rData.mbShowFormulas );
    ::set_flag( mnFlags, EXC_WIN2_SHOWGRID,         rData.mbShowGrid );
    ::set_flag( mnFlags, EXC_WIN2_SHOWHEADINGS,     rData.mbShowHeadings );
    ::set_flag( mnFlags, EXC_WIN2_FROZEN,           rData.mbFrozenPanes );
    ::set_flag( mnFlags, EXC_WIN2_SHOWZEROS,        rData.mbShowZeros );
    ::set_flag( mnFlags, EXC_WIN2_DEFGRIDCOLOR,     rData.mbDefGridColor );
    ::set_flag( mnFlags, EXC_WIN2_MIRRORED,         rData.mbMirrored );
    ::set_flag( mnFlags, EXC_WIN2_SHOWOUTLINE,      rData.mbShowOutline );
    ::set_flag( mnFlags, EXC_WIN2_FROZENNOSPLIT,    rData.mbFrozenPanes );
    ::set_flag( mnFlags, EXC_WIN2_SELECTED,         rData.mbSelected );
    ::set_flag( mnFlags, EXC_WIN2_DISPLAYED,        rData.mbDisplayed );
    ::set_flag( mnFlags, EXC_WIN2_PAGEBREAKMODE,    rData.mbPageMode );
}

void XclExpWindow2::WriteBody( XclExpStream& rStrm )
{
    const XclExpRoot& rRoot = rStrm.GetRoot();

    rStrm   << mnFlags
            << maFirstXclPos;

    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            rStrm   << maGridColor;
        break;
        case EXC_BIFF8:
            rStrm   << rRoot.GetPalette().GetColorIndex( mnGridColorId )
                    << sal_uInt16( 0 )
                    << mnPageZoom
                    << mnNormalZoom
                    << sal_uInt32( 0 );
        break;
        default:    DBG_ERROR_BIFF();
    }
}

// ----------------------------------------------------------------------------

XclExpScl::XclExpScl( sal_uInt16 nZoom ) :
    XclExpRecord( EXC_ID_SCL, 4 ),
    mnNum( nZoom ),
    mnDenom( 100 )
{
    Shorten( 2 );
    Shorten( 5 );
}

void XclExpScl::Shorten( sal_uInt16 nFactor )
{
    while( (mnNum % nFactor == 0) && (mnDenom % nFactor == 0) )
    {
        mnNum /= nFactor;
        mnDenom /= nFactor;
    }
}

void XclExpScl::WriteBody( XclExpStream& rStrm )
{
    DBG_ASSERT_BIFF( rStrm.GetRoot().GetBiff() >= EXC_BIFF4 );
    rStrm << mnNum << mnDenom;
}

// ----------------------------------------------------------------------------

XclExpPane::XclExpPane( const XclExpRoot& rRoot, const XclTabViewData& rData ) :
    XclExpRecord( EXC_ID_PANE, 10 ),
    mnSplitX( rData.mnSplitX ),
    mnSplitY( rData.mnSplitY ),
    maSecondXclPos( rData.maSecondXclPos ),
    mnActivePane( rData.mnActivePane )
{
    DBG_ASSERT( rData.IsSplit(), "XclExpPane::XclExpPane - no PANE record for unsplit view" );
}

void XclExpPane::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnSplitX
            << mnSplitY
            << maSecondXclPos
            << mnActivePane;
    if( rStrm.GetRoot().GetBiff() >= EXC_BIFF5 )
        rStrm << sal_uInt8( 0 );
}

// ----------------------------------------------------------------------------

XclExpSelection::XclExpSelection( const XclTabViewData& rData, sal_uInt8 nPane ) :
    XclExpRecord( EXC_ID_SELECTION, 15 ),
    mnPane( nPane )
{
    if( const XclSelectionData* pSelData = rData.GetSelectionData( nPane ) )
        maSelData = *pSelData;

    // find the cursor position in the selection list (or add it)
    XclRangeList& rXclSel = maSelData.maXclSelection;
    bool bFound = false;
    for( XclRangeList::const_iterator aIt = rXclSel.begin(), aEnd = rXclSel.end(); !bFound && (aIt != aEnd); ++aIt )
        if( (bFound = aIt->Contains( maSelData.maXclCursor )) == true )
            maSelData.mnCursorIdx = static_cast< sal_uInt16 >( aIt - rXclSel.begin() );
    /*  Cursor cell not found in list? (e.g. inactive pane, or removed in
        ConvertRangeList(), because Calc cursor on invalid pos)
        -> insert the valid Excel cursor. */
    if( !bFound )
    {
        maSelData.mnCursorIdx = static_cast< sal_uInt16 >( rXclSel.size() );
        rXclSel.push_back( XclRange( maSelData.maXclCursor ) );
    }
}

void XclExpSelection::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnPane                   // pane for this selection
            << maSelData.maXclCursor    // cell cursor
            << maSelData.mnCursorIdx;   // index to range containing cursor
    maSelData.maXclSelection.Write( rStrm, false );
}

// Sheet view settings ========================================================

namespace {

/** Converts a Calc zoom factor into an Excel zoom factor. Returns 0 for a default zoom value. */
sal_uInt16 lclGetXclZoom( long nScZoom, sal_uInt16 nDefXclZoom )
{
    sal_uInt16 nXclZoom = limit_cast< sal_uInt16 >( nScZoom, EXC_ZOOM_MIN, EXC_ZOOM_MAX );
    return (nXclZoom == nDefXclZoom) ? 0 : nXclZoom;
}

} // namespace

// ----------------------------------------------------------------------------

XclExpTabViewSettings::XclExpTabViewSettings( const XclExpRoot& rRoot, SCTAB nScTab ) :
    XclExpRoot( rRoot ),
    mnGridColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT ) )
{
    // *** sheet flags ***

    const XclExpTabInfo& rTabInfo = GetTabInfo();
    maData.mbSelected       = rTabInfo.IsSelectedTab( nScTab );
    maData.mbDisplayed      = rTabInfo.IsDisplayedTab( nScTab );
    maData.mbMirrored       = rTabInfo.IsMirroredTab( nScTab );

    const ScViewOptions& rViewOpt = GetDoc().GetViewOptions();
    maData.mbShowFormulas   = rViewOpt.GetOption( VOPT_FORMULAS );
    maData.mbShowGrid       = rViewOpt.GetOption( VOPT_GRID );
    maData.mbShowHeadings   = rViewOpt.GetOption( VOPT_HEADER );
    maData.mbShowZeros      = rViewOpt.GetOption( VOPT_NULLVALS );
    maData.mbShowOutline    = rViewOpt.GetOption( VOPT_OUTLINER );

    // *** sheet options: cursor, selection, splits, grid color, zoom ***

    if( const ScExtTabSettings* pTabSett = GetExtDocOptions().GetTabSettings( nScTab ) )
    {
        const ScExtTabSettings& rTabSett = *pTabSett;
        XclExpAddressConverter& rAddrConv = GetAddressConverter();

        // first visible cell in top-left pane
        if( (rTabSett.maFirstVis.Col() >= 0) && (rTabSett.maFirstVis.Row() >= 0) )
            maData.maFirstXclPos = rAddrConv.CreateValidAddress( rTabSett.maFirstVis, false );

        // first visible cell in additional pane(s)
        if( (rTabSett.maSecondVis.Col() >= 0) && (rTabSett.maSecondVis.Row() >= 0) )
            maData.maSecondXclPos = rAddrConv.CreateValidAddress( rTabSett.maSecondVis, false );

        // active pane
        switch( rTabSett.meActivePane )
        {
            case SCEXT_PANE_TOPLEFT:        maData.mnActivePane = EXC_PANE_TOPLEFT;     break;
            case SCEXT_PANE_TOPRIGHT:       maData.mnActivePane = EXC_PANE_TOPRIGHT;    break;
            case SCEXT_PANE_BOTTOMLEFT:     maData.mnActivePane = EXC_PANE_BOTTOMLEFT;  break;
            case SCEXT_PANE_BOTTOMRIGHT:    maData.mnActivePane = EXC_PANE_BOTTOMRIGHT; break;
        }

        // freeze/split position
        maData.mbFrozenPanes = rTabSett.mbFrozenPanes;
        if( maData.mbFrozenPanes )
        {
            /*  Frozen panes: handle split position as row/column positions.
                #i35812# Excel uses number of visible rows/columns, Calc uses position of freeze. */
            SCCOL nFreezeScCol = rTabSett.maFreezePos.Col();
            if( (0 < nFreezeScCol) && (nFreezeScCol <= GetXclMaxPos().Col()) )
                maData.mnSplitX = static_cast< sal_uInt16 >( nFreezeScCol ) - maData.maFirstXclPos.mnCol;
            SCROW nFreezeScRow = rTabSett.maFreezePos.Row();
            if( (0 < nFreezeScRow) && (nFreezeScRow <= GetXclMaxPos().Row()) )
                maData.mnSplitY = static_cast< sal_uInt16 >( nFreezeScRow ) - maData.maFirstXclPos.mnRow;
            // if both splits are left out (address overflow), remove the frozen flag
            maData.mbFrozenPanes = maData.IsSplit();

            // #i20671# frozen panes: mostright/mostbottom pane is active regardless of cursor position
            if( maData.HasPane( EXC_PANE_BOTTOMRIGHT ) )
                maData.mnActivePane = EXC_PANE_BOTTOMRIGHT;
            else if( maData.HasPane( EXC_PANE_TOPRIGHT ) )
                maData.mnActivePane = EXC_PANE_TOPRIGHT;
            else if( maData.HasPane( EXC_PANE_BOTTOMLEFT ) )
                maData.mnActivePane = EXC_PANE_BOTTOMLEFT;
        }
        else
        {
            // split window: position is in twips
            maData.mnSplitX = ulimit_cast< sal_uInt16 >( rTabSett.maSplitPos.X() );
            maData.mnSplitY = ulimit_cast< sal_uInt16 >( rTabSett.maSplitPos.Y() );
        }

        // selection
        CreateSelectionData( EXC_PANE_TOPLEFT,     rTabSett.maCursor, rTabSett.maSelection );
        CreateSelectionData( EXC_PANE_TOPRIGHT,    rTabSett.maCursor, rTabSett.maSelection );
        CreateSelectionData( EXC_PANE_BOTTOMLEFT,  rTabSett.maCursor, rTabSett.maSelection );
        CreateSelectionData( EXC_PANE_BOTTOMRIGHT, rTabSett.maCursor, rTabSett.maSelection );

        // grid color
        const Color& rGridColor = rTabSett.maGridColor;
        maData.mbDefGridColor = rGridColor.GetColor() == COL_AUTO;
        if( !maData.mbDefGridColor )
        {
            if( GetBiff() == EXC_BIFF8 )
                mnGridColorId = GetPalette().InsertColor( rGridColor, EXC_COLOR_GRID );
            else
                maData.maGridColor = rGridColor;
        }

        // view mode and zoom
        maData.mbPageMode       = (GetBiff() == EXC_BIFF8) && rTabSett.mbPageMode;
        maData.mnNormalZoom     = lclGetXclZoom( rTabSett.mnNormalZoom, EXC_WIN2_NORMALZOOM_DEF );
        maData.mnPageZoom       = lclGetXclZoom( rTabSett.mnPageZoom, EXC_WIN2_PAGEZOOM_DEF );
        maData.mnCurrentZoom    = maData.mbPageMode ? maData.mnPageZoom : maData.mnNormalZoom;
    }
}

void XclExpTabViewSettings::Save( XclExpStream& rStrm )
{
    WriteWindow2( rStrm );
    WriteScl( rStrm );
    WritePane( rStrm );
    WriteSelection( rStrm, EXC_PANE_TOPLEFT );
    WriteSelection( rStrm, EXC_PANE_TOPRIGHT );
    WriteSelection( rStrm, EXC_PANE_BOTTOMLEFT );
    WriteSelection( rStrm, EXC_PANE_BOTTOMRIGHT );
}

// private --------------------------------------------------------------------

void XclExpTabViewSettings::CreateSelectionData( sal_uInt8 nPane,
        const ScAddress& rCursor, const ScRangeList& rSelection )
{
    if( maData.HasPane( nPane ) )
    {
        XclSelectionData& rSelData = maData.CreateSelectionData( nPane );

        // first step: use top-left visible cell as cursor
        rSelData.maXclCursor.mnCol = ((nPane == EXC_PANE_TOPLEFT) || (nPane == EXC_PANE_BOTTOMLEFT)) ?
            maData.maFirstXclPos.mnCol : maData.maSecondXclPos.mnCol;
        rSelData.maXclCursor.mnRow = ((nPane == EXC_PANE_TOPLEFT) || (nPane == EXC_PANE_TOPRIGHT)) ?
            maData.maFirstXclPos.mnRow : maData.maSecondXclPos.mnRow;

        // second step, active pane: create actual selection data with current cursor position
        if( nPane == maData.mnActivePane )
        {
            XclExpAddressConverter& rAddrConv = GetAddressConverter();
            // cursor position (keep top-left pane position from above, if rCursor is invalid)
            if( (rCursor.Col() >= 0) && (rCursor.Row() >= 0) )
                rSelData.maXclCursor = rAddrConv.CreateValidAddress( rCursor, false );
            // selection
            rAddrConv.ConvertRangeList( rSelData.maXclSelection, rSelection, false );
        }
    }
}

void XclExpTabViewSettings::WriteWindow2( XclExpStream& rStrm ) const
{
//  #i43553# GCC 3.3 parse error
//    XclExpWindow2( GetRoot(), maData, mnGridColorId ).Save( rStrm );
    XclExpWindow2 aWindow2( GetRoot(), maData, mnGridColorId );
    aWindow2.Save( rStrm );
}

void XclExpTabViewSettings::WriteScl( XclExpStream& rStrm ) const
{
    if( maData.mnCurrentZoom != 0 )
        XclExpScl( maData.mnCurrentZoom ).Save( rStrm );
}

void XclExpTabViewSettings::WritePane( XclExpStream& rStrm ) const
{
    if( maData.IsSplit() )
//  #i43553# GCC 3.3 parse error
//        XclExpPane( GetRoot(), maData ).Save( rStrm );
    {
        XclExpPane aPane( GetRoot(), maData );
        aPane.Save( rStrm );
    }
}

void XclExpTabViewSettings::WriteSelection( XclExpStream& rStrm, sal_uInt8 nPane ) const
{
    if( maData.HasPane( nPane ) )
        XclExpSelection( maData, nPane ).Save( rStrm );
}

// ============================================================================

