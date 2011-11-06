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
#include "precompiled_chart2.hxx"
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertLegend.hrc"
#include "res_LegendPosition.hxx"
#include "ObjectNameProvider.hxx"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

// header for enum SvxChartLegendPos
#include <svx/chrtitem.hxx>
// header for class SfxItemPool
#include <svl/itempool.hxx>

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
