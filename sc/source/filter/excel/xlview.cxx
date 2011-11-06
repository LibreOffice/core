/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

