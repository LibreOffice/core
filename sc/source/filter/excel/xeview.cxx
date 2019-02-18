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

#include <xeview.hxx>
#include <document.hxx>
#include <scextopt.hxx>
#include <viewopti.hxx>
#include <xelink.hxx>
#include <xestyle.hxx>
#include <xehelper.hxx>
#include <xltools.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>

using namespace ::oox;

// Workbook view settings records =============================================

XclExpWindow1::XclExpWindow1( const XclExpRoot& rRoot )
    : XclExpRecord(EXC_ID_WINDOW1, 18)
    , mnFlags(0)
    , mnTabBarSize(600)
{
    const ScViewOptions& rViewOpt = rRoot.GetDoc().GetViewOptions();
    ::set_flag( mnFlags, EXC_WIN1_HOR_SCROLLBAR,    rViewOpt.GetOption( VOPT_HSCROLL ) );
    ::set_flag( mnFlags, EXC_WIN1_VER_SCROLLBAR,    rViewOpt.GetOption( VOPT_VSCROLL ) );
    ::set_flag( mnFlags, EXC_WIN1_TABBAR,           rViewOpt.GetOption( VOPT_TABCONTROLS ) );

    double fTabBarWidth = rRoot.GetExtDocOptions().GetDocSettings().mfTabBarWidth;
    if( (0.0 <= fTabBarWidth) && (fTabBarWidth <= 1.0) )
        mnTabBarSize = static_cast< sal_uInt16 >( fTabBarWidth * 1000.0 + 0.5 );
}

void XclExpWindow1::SaveXml( XclExpXmlStream& rStrm )
{
    const XclExpTabInfo& rTabInfo = rStrm.GetRoot().GetTabInfo();

    rStrm.GetCurrentStream()->singleElement( XML_workbookView,
            // OOXTODO: XML_visibility, // ST_visibilty
            // OOXTODO: XML_minimized,  // bool
            XML_showHorizontalScroll,   ToPsz( ::get_flag( mnFlags, EXC_WIN1_HOR_SCROLLBAR ) ),
            XML_showVerticalScroll,     ToPsz( ::get_flag( mnFlags, EXC_WIN1_VER_SCROLLBAR ) ),
            XML_showSheetTabs,          ToPsz( ::get_flag( mnFlags, EXC_WIN1_TABBAR ) ),
            XML_xWindow,                "0",
            XML_yWindow,                "0",
            XML_windowWidth,            OString::number( 0x4000 ).getStr(),
            XML_windowHeight,           OString::number( 0x2000 ).getStr(),
            XML_tabRatio,               OString::number( mnTabBarSize ).getStr(),
            XML_firstSheet,             OString::number( rTabInfo.GetFirstVisXclTab() ).getStr(),
            XML_activeTab,              OString::number( rTabInfo.GetDisplayedXclTab() ).getStr(),
            // OOXTODO: XML_autoFilterDateGrouping,     // bool; AUTOFILTER12? 87Eh
            FSEND );
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
        mnNum = mnNum / nFactor;
        mnDenom = mnDenom / nFactor;
    }
}

void XclExpScl::WriteBody( XclExpStream& rStrm )
{
    OSL_ENSURE_BIFF( rStrm.GetRoot().GetBiff() >= EXC_BIFF4 );
    rStrm << mnNum << mnDenom;
}

XclExpPane::XclExpPane( const XclTabViewData& rData ) :
    XclExpRecord( EXC_ID_PANE, 10 ),
    mnSplitX( rData.mnSplitX ),
    mnSplitY( rData.mnSplitY ),
    maSecondXclPos( rData.maSecondXclPos ),
    mnActivePane( rData.mnActivePane ),
    mbFrozenPanes( rData.mbFrozenPanes )
{
    OSL_ENSURE( rData.IsSplit(), "XclExpPane::XclExpPane - no PANE record for unsplit view" );
}

static const char* lcl_GetActivePane( sal_uInt8 nActivePane )
{
    switch( nActivePane )
    {
        case EXC_PANE_TOPLEFT:      return "topLeft";
        case EXC_PANE_TOPRIGHT:     return "topRight";
        case EXC_PANE_BOTTOMLEFT:   return "bottomLeft";
        case EXC_PANE_BOTTOMRIGHT:  return "bottomRight";
    }
    return "**error: lcl_GetActivePane";
}

void XclExpPane::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->singleElement( XML_pane,
            XML_xSplit,         OString::number( mnSplitX ).getStr(),
            XML_ySplit,         OString::number( mnSplitY ).getStr(),
            XML_topLeftCell,    XclXmlUtils::ToOString( rStrm.GetRoot().GetStringBuf(), maSecondXclPos ).getStr(),
            XML_activePane,     lcl_GetActivePane( mnActivePane ),
            XML_state,          mbFrozenPanes ? "frozen" : "split",
            FSEND );
}

void XclExpPane::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnSplitX
            << static_cast<sal_uInt16>( mnSplitY )
            << maSecondXclPos
            << mnActivePane;
    if( rStrm.GetRoot().GetBiff() >= EXC_BIFF5 )
        rStrm << sal_uInt8( 0 );
}

XclExpSelection::XclExpSelection( const XclTabViewData& rData, sal_uInt8 nPane ) :
    XclExpRecord( EXC_ID_SELECTION, 15 ),
    mnPane( nPane )
{
    if( const XclSelectionData* pSelData = rData.GetSelectionData( nPane ) )
        maSelData = *pSelData;

    // find the cursor position in the selection list (or add it)
    XclRangeList& rXclSel = maSelData.maXclSelection;
    auto aIt = std::find_if(rXclSel.begin(), rXclSel.end(),
        [this](const XclRange& rRange) { return rRange.Contains(maSelData.maXclCursor); });
    if (aIt != rXclSel.end())
    {
        maSelData.mnCursorIdx = static_cast< sal_uInt16 >( std::distance(rXclSel.begin(), aIt) );
    }
    else
    {
        /*  Cursor cell not found in list? (e.g. inactive pane, or removed in
            ConvertRangeList(), because Calc cursor on invalid pos)
            -> insert the valid Excel cursor. */
        maSelData.mnCursorIdx = static_cast< sal_uInt16 >( rXclSel.size() );
        rXclSel.push_back( XclRange( maSelData.maXclCursor ) );
    }
}

void XclExpSelection::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->singleElement( XML_selection,
            XML_pane,           lcl_GetActivePane( mnPane ),
            XML_activeCell,     XclXmlUtils::ToOString( rStrm.GetRoot().GetStringBuf(), maSelData.maXclCursor ).getStr(),
            XML_activeCellId,   OString::number(  maSelData.mnCursorIdx ).getStr(),
            XML_sqref,          XclXmlUtils::ToOString( maSelData.maXclSelection ).getStr(),
            FSEND );
}

void XclExpSelection::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnPane                   // pane for this selection
            << maSelData.maXclCursor    // cell cursor
            << maSelData.mnCursorIdx;   // index to range containing cursor
    maSelData.maXclSelection.Write( rStrm, false );
}

XclExpTabBgColor::XclExpTabBgColor( const XclTabViewData& rTabViewData ) :
    XclExpRecord( EXC_ID_SHEETEXT, 18 ),
    mrTabViewData( rTabViewData )
{
}
//TODO Fix savexml...
/*void XclExpTabBgColor::SaveXml( XclExpXmlStream& rStrm )
{
}*/

void XclExpTabBgColor::WriteBody( XclExpStream& rStrm )
{
    if ( mrTabViewData.IsDefaultTabBgColor() )
        return;
    sal_uInt16 const rt = 0x0862; //rt
    sal_uInt16 const grbitFrt = 0x0000; //grbit must be set to 0
    sal_uInt32 unused = 0x00000000; //Use twice...
    sal_uInt32 const cb = 0x00000014; // Record Size, may be larger in future...
    sal_uInt16 const reserved = 0x0000; //trailing bits are 0
    sal_uInt16 TabBgColorIndex;
    XclExpPalette& rPal = rStrm.GetRoot().GetPalette();
    TabBgColorIndex = rPal.GetColorIndex(mrTabViewData.mnTabBgColorId);
    if (TabBgColorIndex < 8 || TabBgColorIndex > 63 ) // only numbers 8 - 63 are valid numbers
        TabBgColorIndex = 127; //Excel specs: 127 makes excel ignore tab color information.
    rStrm << rt << grbitFrt << unused << unused << cb << TabBgColorIndex << reserved;
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
                maData.mnSplitY = static_cast< sal_uInt32 >( nFreezeScRow ) - maData.maFirstXclPos.mnRow;
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
            maData.mnSplitX = static_cast<sal_uInt16>(rTabSett.maSplitPos.X());
            maData.mnSplitY = static_cast<sal_uInt32>(rTabSett.maSplitPos.Y());
        }

        // selection
        CreateSelectionData( EXC_PANE_TOPLEFT,     rTabSett.maCursor, rTabSett.maSelection );
        CreateSelectionData( EXC_PANE_TOPRIGHT,    rTabSett.maCursor, rTabSett.maSelection );
        CreateSelectionData( EXC_PANE_BOTTOMLEFT,  rTabSett.maCursor, rTabSett.maSelection );
        CreateSelectionData( EXC_PANE_BOTTOMRIGHT, rTabSett.maCursor, rTabSett.maSelection );

        // grid color
        const Color& rGridColor = rTabSett.maGridColor;
        maData.mbDefGridColor = rGridColor == COL_AUTO;
        if( !maData.mbDefGridColor )
        {
            if( GetBiff() == EXC_BIFF8 )
                mnGridColorId = GetPalette().InsertColor( rGridColor, EXC_COLOR_GRID );
            else
                maData.maGridColor = rGridColor;
        }
        maData.mbShowGrid       = rTabSett.mbShowGrid;

        // view mode and zoom
        maData.mbPageMode       = (GetBiff() == EXC_BIFF8) && rTabSett.mbPageMode;
        maData.mnNormalZoom     = lclGetXclZoom( rTabSett.mnNormalZoom, EXC_WIN2_NORMALZOOM_DEF );
        maData.mnPageZoom       = lclGetXclZoom( rTabSett.mnPageZoom, EXC_WIN2_PAGEZOOM_DEF );
        maData.mnCurrentZoom    = maData.mbPageMode ? maData.mnPageZoom : maData.mnNormalZoom;
    }

    // Tab Bg Color
    if ( GetBiff() == EXC_BIFF8 && !GetDoc().IsDefaultTabBgColor(nScTab) )
    {
        XclExpPalette& rPal = GetPalette();
        maData.maTabBgColor = GetDoc().GetTabBgColor(nScTab);
        maData.mnTabBgColorId = rPal.InsertColor(maData.maTabBgColor, EXC_COLOR_TABBG, EXC_COLOR_NOTABBG );
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
    WriteTabBgColor( rStrm );
}

static void lcl_WriteSelection( XclExpXmlStream& rStrm, const XclTabViewData& rData, sal_uInt8 nPane )
{
    if( rData.HasPane( nPane ) )
        XclExpSelection( rData, nPane ).SaveXml( rStrm );
}

static OString lcl_GetZoom( sal_uInt16 nZoom )
{
    if( nZoom )
        return OString::number( nZoom );
    return OString( "100" );
}

void XclExpTabViewSettings::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rWorksheet = rStrm.GetCurrentStream();
    rWorksheet->startElement( XML_sheetViews, FSEND );

    // handle missing viewdata at embedded XLSX OLE objects
    if (maData.mbSelected)
    {
        SCCOL nPosLeft = rStrm.GetRoot().GetDoc().GetPosLeft();
        SCROW nPosTop = rStrm.GetRoot().GetDoc().GetPosTop();
        if (nPosLeft > 0 || nPosTop > 0)
        {
            ScAddress aLeftTop(nPosLeft, nPosTop, 0);
            XclExpAddressConverter& rAddrConv = GetAddressConverter();
            maData.maFirstXclPos = rAddrConv.CreateValidAddress( aLeftTop, false );
        }
    }

    rWorksheet->startElement( XML_sheetView,
            // OOXTODO: XML_windowProtection,
            XML_showFormulas,               ToPsz( maData.mbShowFormulas ),
            XML_showGridLines,              ToPsz( maData.mbShowGrid ),
            XML_showRowColHeaders,          ToPsz( maData.mbShowHeadings ),
            XML_showZeros,                  ToPsz( maData.mbShowZeros ),
            XML_rightToLeft,                ToPsz( maData.mbMirrored ),
            XML_tabSelected,                ToPsz( maData.mbSelected ),
            // OOXTODO: XML_showRuler,
            XML_showOutlineSymbols,         ToPsz( maData.mbShowOutline ),
            XML_defaultGridColor,           mnGridColorId == XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT ) ? "true" : "false",
            // OOXTODO: XML_showWhiteSpace,
            XML_view,                       maData.mbPageMode ? "pageBreakPreview" : "normal",  // OOXTODO: pageLayout
            XML_topLeftCell,                XclXmlUtils::ToOString( rStrm.GetRoot().GetStringBuf(), maData.maFirstXclPos ).getStr(),
            XML_colorId,                    OString::number(  rStrm.GetRoot().GetPalette().GetColorIndex( mnGridColorId ) ).getStr(),
            XML_zoomScale,                  lcl_GetZoom( maData.mnCurrentZoom ).getStr(),
            XML_zoomScaleNormal,            lcl_GetZoom( maData.mnNormalZoom ).getStr(),
            // OOXTODO: XML_zoomScaleSheetLayoutView,
            XML_zoomScalePageLayoutView,    lcl_GetZoom( maData.mnPageZoom ).getStr(),
            XML_workbookViewId,             "0",    // OOXTODO? 0-based index of document(xl/workbook.xml)/workbook/bookviews/workbookView
                                                    //          should always be 0, as we only generate 1 such element.
            FSEND );
    if( maData.IsSplit() )
    {
        XclExpPane aPane( maData );
        aPane.SaveXml( rStrm );
    }
    lcl_WriteSelection( rStrm, maData, EXC_PANE_TOPLEFT );
    lcl_WriteSelection( rStrm, maData, EXC_PANE_TOPRIGHT );
    lcl_WriteSelection( rStrm, maData, EXC_PANE_BOTTOMLEFT );
    lcl_WriteSelection( rStrm, maData, EXC_PANE_BOTTOMRIGHT );
    rWorksheet->endElement( XML_sheetView );
    // OOXTODO: XML_extLst
    rWorksheet->endElement( XML_sheetViews );
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
        XclExpPane aPane( maData );
        aPane.Save( rStrm );
    }
}

void XclExpTabViewSettings::WriteSelection( XclExpStream& rStrm, sal_uInt8 nPane ) const
{
    if( maData.HasPane( nPane ) )
        XclExpSelection( maData, nPane ).Save( rStrm );
}

void XclExpTabViewSettings::WriteTabBgColor( XclExpStream& rStrm ) const
{
    if ( !maData.IsDefaultTabBgColor() )
        XclExpTabBgColor( maData ).Save( rStrm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
