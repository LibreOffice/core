/*************************************************************************
 *
 *  $RCSfile: xlview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 13:35:52 $
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

#ifndef SC_XLVIEW_HXX
#include "xlview.hxx"
#endif

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif

// View settings ==============================================================

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

