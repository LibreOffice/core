/*************************************************************************
 *
 *  $RCSfile: xiview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 13:34:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_XIVIEW_HXX
#include "xiview.hxx"
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

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif

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

void XclImpTabViewSettings::ReadWindow2( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> nFlags >> maData.maFirstXclPos;

    maData.mbSelected       = ::get_flag( nFlags, EXC_WIN2_SELECTED );
    maData.mbDisplayed      = ::get_flag( nFlags, EXC_WIN2_DISPLAYED );
    maData.mbMirrored       = ::get_flag( nFlags, EXC_WIN2_MIRRORED );
    maData.mbFrozenPanes    = ::get_flag( nFlags, EXC_WIN2_FROZEN );
    maData.mbPageMode       = ::get_flag( nFlags, EXC_WIN2_PAGEBREAKMODE );
    maData.mbDefGridColor   = ::get_flag( nFlags, EXC_WIN2_DEFGRIDCOLOR );
    maData.mbShowFormulas   = ::get_flag( nFlags, EXC_WIN2_SHOWFORMULAS );
    maData.mbShowGrid       = ::get_flag( nFlags, EXC_WIN2_SHOWGRID );
    maData.mbShowHeadings   = ::get_flag( nFlags, EXC_WIN2_SHOWHEADINGS );
    maData.mbShowZeros      = ::get_flag( nFlags, EXC_WIN2_SHOWZEROS );
    maData.mbShowOutline    = ::get_flag( nFlags, EXC_WIN2_SHOWOUTLINE );

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
            rStrm.Ignore( 2 );
            rStrm >> maData.mnPageZoom >> maData.mnNormalZoom;

            if( !maData.mbDefGridColor )
                maData.maGridColor = GetPalette().GetColor( nGridColorIdx );
        }
        break;
        default:    DBG_ERROR_BIFF();
    }
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

    // *** sheet options: cursor, selection, splits, zoom ***

    // sheet flags
    rDoc.SetLayoutRTL( nScTab, maData.mbMirrored );
    rTabSett.mbSelected = maData.mbSelected || maData.mbDisplayed;

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

    // view mode and zoom
    if( maData.mnCurrentZoom != 0 )
        (maData.mbPageMode ? maData.mnPageZoom : maData.mnNormalZoom) = maData.mnCurrentZoom;
    rTabSett.mbPageMode      = maData.mbPageMode;
    rTabSett.mnNormalZoom    = lclGetScZoom( maData.mnNormalZoom, EXC_WIN2_NORMALZOOM_DEF );
    rTabSett.mnPageZoom      = lclGetScZoom( maData.mnPageZoom, EXC_WIN2_PAGEZOOM_DEF );

    // *** additional handling for displayed sheet ***

    if( maData.mbDisplayed )
    {
        // displayed sheet
        GetExtDocOptions().GetDocSettings().mnDisplTab = nScTab;

        // set Excel sheet settings globally at Calc document, take settings from displayed sheet
        ScViewOptions aViewOpt( rDoc.GetViewOptions() );
        aViewOpt.SetOption( VOPT_FORMULAS, maData.mbShowFormulas );
        aViewOpt.SetOption( VOPT_GRID,     maData.mbShowGrid );
        aViewOpt.SetOption( VOPT_HEADER,   maData.mbShowHeadings );
        aViewOpt.SetOption( VOPT_NULLVALS, maData.mbShowZeros );
        aViewOpt.SetOption( VOPT_OUTLINER, maData.mbShowOutline );
        rDoc.SetViewOptions( aViewOpt );
    }
}

// ============================================================================

