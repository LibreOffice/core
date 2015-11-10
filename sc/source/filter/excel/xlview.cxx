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

#include "xlview.hxx"
#include "ftools.hxx"
#include <osl/diagnose.h>

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
    mnTabBgColorId = 0;
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
    OSL_FAIL( "XclExpPane::HasPane - wrong pane ID" );
    return false;
}

const XclSelectionData* XclTabViewData::GetSelectionData( sal_uInt8 nPane ) const
{
    XclSelectionMap::const_iterator aIt = maSelMap.find( nPane );
    return (aIt == maSelMap.end()) ? nullptr : aIt->second.get();
}

XclSelectionData& XclTabViewData::CreateSelectionData( sal_uInt8 nPane )
{
    XclSelectionDataRef& rxSelData = maSelMap[ nPane ];
    if( !rxSelData )
        rxSelData.reset( new XclSelectionData );
    return *rxSelData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
