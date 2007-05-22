/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertLegend.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:35:05 $
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
#include "precompiled_chart2.hxx"
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertLegend.hrc"
#include "res_LegendPosition.hxx"
#include "ObjectNameProvider.hxx"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

// header for enum SvxChartLegendPos
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
// header for class SfxItemPool
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;

SchLegendDlg::SchLegendDlg(Window* pWindow, const uno::Reference< uno::XComponentContext>& xCC )
    : ModalDialog(pWindow, SchResId(DLG_LEGEND))
    , m_apLegendPositionResources( new LegendPositionResources(this,xCC) )
    , aBtnOK(this, SchResId(BTN_OK))
    , aBtnCancel(this, SchResId(BTN_CANCEL))
    , aBtnHelp(this, SchResId(BTN_HELP))
{
    FreeResource();
    this->SetText( ObjectNameProvider::getName(OBJECTTYPE_LEGEND) );
}

SchLegendDlg::~SchLegendDlg()
{
}

void SchLegendDlg::init( const uno::Reference< frame::XModel >& xChartModel )
{
    m_apLegendPositionResources->writeToResources( xChartModel );
}

bool SchLegendDlg::writeToModel( const uno::Reference< frame::XModel >& xChartModel ) const
{
    m_apLegendPositionResources->writeToModel( xChartModel );
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
