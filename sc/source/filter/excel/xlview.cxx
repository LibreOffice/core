/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xlview.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:23:51 $
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

#ifndef SC_XLVIEW_HXX
#include "xlview.hxx"
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

// Structs ====================================================================

XclDocViewData::XclDocViewData() :
    mnWinX( 0 ),
    mnWinY( 0 ),
    mnWinWidth( 0 ),
    mnWinHeight( 0 ),
    mnFlags( EXC_WIN1_HOR_SCROLLBAR | EXC_WIN1_VER_SCROLLBAR | EXC_WIN1_TABBAR ),
    mnDisplXclTab( 0 ),
    mnFirstVisXclTab( 0 ),
    mnXclSelectCnt( 1 ),
    mnTabBarWidth( 600 )
{
}

// ----------------------------------------------------------------------------

XclTabViewData::XclTabViewData() :
    maFirstXclPos( ScAddress::UNINITIALIZED ),
    maSecondXclPos( ScAddress::UNINITIALIZED )
{
    SetDefaults();
}

XclTabViewData::~XclTabViewData()
{
}

void XclTabViewData::SetDefaults()
{
    maSelMap.clear();
    maGridColor.SetColor( COL_AUTO );
    maFirstXclPos.Set( 0, 0 );
    maSecondXclPos.Set( 0, 0 );
    mnSplitX = mnSplitY = 0;
    mnNormalZoom = EXC_WIN2_NORMALZOOM_DEF;
    mnPageZoom = EXC_WIN2_PAGEZOOM_DEF;
    mnCurrentZoom = 0;  // default to mnNormalZoom or mnPageZoom
    mnActivePane = EXC_PANE_TOPLEFT;
    mbSelected = mbDisplayed = false;
    mbMirrored = false;
    mbFrozenPanes = false;
    mbPageMode = false;
    mbDefGridColor = true;
    mbShowFormulas = false;
    mbShowGrid = mbShowHeadings = mbShowZeros = mbShowOutline = true;
}

bool XclTabViewData::IsSplit() const
{
    return (mnSplitX > 0) || (mnSplitY > 0);
}

bool XclTabViewData::HasPane( sal_uInt8 nPaneId ) const
{
    switch( nPaneId )
    {
        case EXC_PANE_BOTTOMRIGHT:  return (mnSplitX > 0) && (mnSplitY > 0);
        case EXC_PANE_TOPRIGHT:     return mnSplitX > 0;
        case EXC_PANE_BOTTOMLEFT:   return mnSplitY > 0;
        case EXC_PANE_TOPLEFT:      return true;
    }
    DBG_ERRORFILE( "XclExpPane::HasPane - wrong pane ID" );
    return false;
}

const XclSelectionData* XclTabViewData::GetSelectionData( sal_uInt8 nPane ) const
{
    XclSelectionMap::const_iterator aIt = maSelMap.find( nPane );
    return (aIt == maSelMap.end()) ? 0 : aIt->second.get();
}

XclSelectionData& XclTabViewData::CreateSelectionData( sal_uInt8 nPane )
{
    XclSelectionDataRef& rxSelData = maSelMap[ nPane ];
    if( !rxSelData )
        rxSelData.reset( new XclSelectionData );
    return *rxSelData;
}

// ============================================================================

