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
#include "xiview.hxx"
#include "document.hxx"
#include "scextopt.hxx"
#include "viewopti.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xistyle.hxx"

// Document view settings =====================================================

XclImpDocViewSettings::XclImpDocViewSettings( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpDocViewSettings::ReadWindow1( XclImpStream& rStrm )
{
    rStrm   >> maData.mnWinX
            >> maData.mnWinY
            >> maData.mnWinWidth
            >> maData.mnWinHeight
            >> maData.mnFlags;
    if( GetBiff() >= EXC_BIFF5 )
    {
        rStrm   >> maData.mnDisplXclTab
                >> maData.mnFirstVisXclTab
                >> maData.mnXclSelectCnt
                >> maData.mnTabBarWidth;
    }
}

SCTAB XclImpDocViewSettings::GetDisplScTab() const
{
    /*  Simply cast Excel index to Calc index.
        TODO: This may fail if the document contains scenarios. */
    sal_uInt16 nMaxXclTab = static_cast< sal_uInt16 >( GetMaxPos().Tab() );
    return static_cast< SCTAB >( (maData.mnDisplXclTab <= nMaxXclTab) ? maData.mnDisplXclTab : 0 );
}

void XclImpDocViewSettings::Finalize()
{
    ScViewOptions aViewOpt( GetDoc().GetViewOptions() );
    aViewOpt.SetOption( VOPT_HSCROLL,       ::get_flag( maData.mnFlags, EXC_WIN1_HOR_SCROLLBAR ) );
    aViewOpt.SetOption( VOPT_VSCROLL,       ::get_flag( maData.mnFlags, EXC_WIN1_VER_SCROLLBAR ) );
    aViewOpt.SetOption( VOPT_TABCONTROLS,   ::get_flag( maData.mnFlags, EXC_WIN1_TABBAR ) );
    GetDoc().SetViewOptions( aViewOpt );

    // displayed sheet
    GetExtDocOptions().GetDocSettings().mnDisplTab = GetDisplScTab();

    // width of the tabbar with sheet names
    if( maData.mnTabBarWidth <= 1000 )
        GetExtDocOptions().GetDocSettings().mfTabBarWidth = static_cast< double >( maData.mnTabBarWidth ) / 1000.0;
}

// Sheet view settings ========================================================

namespace {

long lclGetScZoom( sal_uInt16 nXclZoom, sal_uInt16 nDefZoom )
{
    return static_cast< long >( nXclZoom ? nXclZoom : nDefZoom );
}

} // namespace

// ----------------------------------------------------------------------------

XclImpTabViewSettings::XclImpTabViewSettings( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
    Initialize();
}

void XclImpTabViewSettings::Initialize()
{
    maData.SetDefaults();
}

void XclImpTabViewSettings::ReadTabBgColor( XclImpStream& rStrm, XclImpPalette& rPal )
{
    DBG_ASSERT_BIFF( GetBiff() >= EXC_BIFF8 );
    if( GetBiff() < EXC_BIFF8 )
        return;

    sal_uInt8 ColorIndex;
    Color TabBgColor;

    rStrm.Ignore( 16 );
    ColorIndex = rStrm.ReaduInt8() & EXC_SHEETEXT_TABCOLOR; //0x7F
    if ( ColorIndex >= 8 && ColorIndex <= 63 ) //only accept valid index values
    {
        TabBgColor = rPal.GetColor( ColorIndex );
        maData.maTabBgColor = TabBgColor;
    }
}

void XclImpTabViewSettings::ReadWindow2( XclImpStream& rStrm, bool bChart )
{
    if( GetBiff() == EXC_BIFF2 )
    {
        maData.mbShowFormulas   = rStrm.ReaduInt8() != 0;
        maData.mbShowGrid       = rStrm.ReaduInt8() != 0;
        maData.mbShowHeadings   = rStrm.ReaduInt8() != 0;
        maData.mbFrozenPanes    = rStrm.ReaduInt8() != 0;
        maData.mbShowZeros      = rStrm.ReaduInt8() != 0;
        rStrm >> maData.maFirstXclPos;
        maData.mbDefGridColor   = rStrm.ReaduInt8() != 0;
        rStrm >> maData.maGridColor;
    }
    else
    {
        sal_uInt16 nFlags;
        rStrm >> nFlags >> maData.maFirstXclPos;

        // #i59590# real life: Excel ignores some view settings in chart sheets
        maData.mbSelected       = ::get_flag( nFlags, EXC_WIN2_SELECTED );
        maData.mbDisplayed      = ::get_flag( nFlags, EXC_WIN2_DISPLAYED );
        maData.mbMirrored       = !bChart && ::get_flag( nFlags, EXC_WIN2_MIRRORED );
        maData.mbFrozenPanes    = !bChart && ::get_flag( nFlags, EXC_WIN2_FROZEN );
        maData.mbPageMode       = !bChart && ::get_flag( nFlags, EXC_WIN2_PAGEBREAKMODE );
        maData.mbDefGridColor   = bChart || ::get_flag( nFlags, EXC_WIN2_DEFGRIDCOLOR );
        maData.mbShowFormulas   = !bChart && ::get_flag( nFlags, EXC_WIN2_SHOWFORMULAS );
        maData.mbShowGrid       = bChart || ::get_flag( nFlags, EXC_WIN2_SHOWGRID );
        maData.mbShowHeadings   = bChart || ::get_flag( nFlags, EXC_WIN2_SHOWHEADINGS );
        maData.mbShowZeros      = bChart || ::get_flag( nFlags, EXC_WIN2_SHOWZEROS );
        maData.mbShowOutline    = bChart || ::get_flag( nFlags, EXC_WIN2_SHOWOUTLINE );

        switch( GetBiff() )
        {
            case EXC_BIFF3:
            case EXC_BIFF4:
            case EXC_BIFF5:
                rStrm >> maData.maGridColor;
            break;
            case EXC_BIFF8:
            {
                sal_uInt16 nGridColorIdx;
                rStrm >> nGridColorIdx;
                // zoom data not included in chart sheets
                if( rStrm.GetRecLeft() >= 6 )
                {
                    rStrm.Ignore( 2 );
                    rStrm >> maData.mnPageZoom >> maData.mnNormalZoom;
                }

                if( !maData.mbDefGridColor )
                    maData.maGridColor = GetPalette().GetColor( nGridColorIdx );
            }
            break;
            default:    DBG_ERROR_BIFF();
        }
    }

    // do not scroll chart sheets
    if( bChart )
        maData.maFirstXclPos.Set( 0, 0 );
}

void XclImpTabViewSettings::ReadScl( XclImpStream& rStrm )
{
    sal_uInt16 nNum, nDenom;
    rStrm >> nNum >> nDenom;
    DBG_ASSERT( nDenom > 0, "XclImpPageSettings::ReadScl - invalid denominator" );
    if( nDenom > 0 )
        maData.mnCurrentZoom = limit_cast< sal_uInt16 >( (nNum * 100) / nDenom );
}

void XclImpTabViewSettings::ReadPane( XclImpStream& rStrm )
{
    rStrm   >> maData.mnSplitX
            >> maData.mnSplitY
            >> maData.maSecondXclPos
            >> maData.mnActivePane;
}

void XclImpTabViewSettings::ReadSelection( XclImpStream& rStrm )
{
    // pane of this selection
    sal_uInt8 nPane;
    rStrm >> nPane;
    XclSelectionData& rSelData = maData.CreateSelectionData( nPane );
    // cursor position and selection
    rStrm >> rSelData.maXclCursor >> rSelData.mnCursorIdx;
    rSelData.maXclSelection.Read( rStrm, false );
}

void XclImpTabViewSettings::Finalize()
{
    SCTAB nScTab = GetCurrScTab();
    ScDocument& rDoc = GetDoc();
    XclImpAddressConverter& rAddrConv = GetAddressConverter();
    ScExtTabSettings& rTabSett = GetExtDocOptions().GetOrCreateTabSettings( nScTab );
    bool bDisplayed = GetDocViewSettings().GetDisplScTab() == nScTab;

    // *** sheet options: cursor, selection, splits, zoom ***

    // sheet flags
    if( maData.mbMirrored )
        // do not call this function with FALSE, it would mirror away all drawing objects
        rDoc.SetLayoutRTL( nScTab, TRUE );
    rTabSett.mbSelected = maData.mbSelected || bDisplayed;

    // first visible cell in top-left pane and in additional pane(s)
    rTabSett.maFirstVis = rAddrConv.CreateValidAddress( maData.maFirstXclPos, nScTab, false );
    rTabSett.maSecondVis = rAddrConv.CreateValidAddress( maData.maSecondXclPos, nScTab, false );

    // cursor position and selection
    if( const XclSelectionData* pSelData = maData.GetSelectionData( maData.mnActivePane ) )
    {
        rTabSett.maCursor = rAddrConv.CreateValidAddress( pSelData->maXclCursor, nScTab, false );
        rAddrConv.ConvertRangeList( rTabSett.maSelection, pSelData->maXclSelection, nScTab, false );
    }

    // active pane
    switch( maData.mnActivePane )
    {
        case EXC_PANE_TOPLEFT:      rTabSett.meActivePane = SCEXT_PANE_TOPLEFT;     break;
        case EXC_PANE_TOPRIGHT:     rTabSett.meActivePane = SCEXT_PANE_TOPRIGHT;    break;
        case EXC_PANE_BOTTOMLEFT:   rTabSett.meActivePane = SCEXT_PANE_BOTTOMLEFT;  break;
        case EXC_PANE_BOTTOMRIGHT:  rTabSett.meActivePane = SCEXT_PANE_BOTTOMRIGHT; break;
    }

    // freeze/split position
    rTabSett.mbFrozenPanes = maData.mbFrozenPanes;
    if( maData.mbFrozenPanes )
    {
        /*  Frozen panes: handle split position as row/column positions.
            #i35812# Excel uses number of visible rows/columns, Calc uses position of freeze. */
        if( (maData.mnSplitX > 0) && (maData.maFirstXclPos.mnCol + maData.mnSplitX <= GetScMaxPos().Col()) )
            rTabSett.maFreezePos.SetCol( static_cast< SCCOL >( maData.maFirstXclPos.mnCol + maData.mnSplitX ) );
        if( (maData.mnSplitY > 0) && (maData.maFirstXclPos.mnRow + maData.mnSplitY <= GetScMaxPos().Row()) )
            rTabSett.maFreezePos.SetRow( static_cast< SCROW >( maData.maFirstXclPos.mnRow + maData.mnSplitY ) );
    }
    else
    {
        // split window: position is in twips
        rTabSett.maSplitPos.X() = static_cast< long >( maData.mnSplitX );
        rTabSett.maSplitPos.Y() = static_cast< long >( maData.mnSplitY );
    }

    // grid color
    if( maData.mbDefGridColor )
        rTabSett.maGridColor.SetColor( COL_AUTO );
    else
        rTabSett.maGridColor = maData.maGridColor;

    // show grid option
    rTabSett.mbShowGrid      = maData.mbShowGrid;

    // view mode and zoom
    if( maData.mnCurrentZoom != 0 )
        (maData.mbPageMode ? maData.mnPageZoom : maData.mnNormalZoom) = maData.mnCurrentZoom;
    rTabSett.mbPageMode      = maData.mbPageMode;
    rTabSett.mnNormalZoom    = lclGetScZoom( maData.mnNormalZoom, EXC_WIN2_NORMALZOOM_DEF );
    rTabSett.mnPageZoom      = lclGetScZoom( maData.mnPageZoom, EXC_WIN2_PAGEZOOM_DEF );

    // *** additional handling for displayed sheet ***

    if( bDisplayed )
    {
        // set Excel sheet settings globally at Calc document, take settings from displayed sheet
        ScViewOptions aViewOpt( rDoc.GetViewOptions() );
        aViewOpt.SetOption( VOPT_FORMULAS, maData.mbShowFormulas );
        aViewOpt.SetOption( VOPT_HEADER,   maData.mbShowHeadings );
        aViewOpt.SetOption( VOPT_NULLVALS, maData.mbShowZeros );
        aViewOpt.SetOption( VOPT_OUTLINER, maData.mbShowOutline );
        rDoc.SetViewOptions( aViewOpt );
    }

    // *** set tab bg color
    if ( !maData.IsDefaultTabBgColor() )
        rDoc.SetTabBgColor(nScTab, maData.maTabBgColor);
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
