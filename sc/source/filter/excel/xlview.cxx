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
#include "xlview.hxx"
#include "ftools.hxx"

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
    maTabBgColor.SetColor( COL_AUTO );
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

