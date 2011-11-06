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

#include "dlg_ChartType.hxx"
#include "dlg_ChartType.hrc"
#include "ResId.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "tp_ChartType.hxx"
#include "macros.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

ChartTypeDialog::ChartTypeDialog( Window* pParent
                , const uno::Reference< frame::XModel >& xChartModel
                , const uno::Reference< uno::XComponentContext >& xContext )
                : ModalDialog( pParent, SchResId( DLG_DIAGRAM_TYPE ))
                , m_aFL( this, SchResId( FL_BUTTONS ) )
                , m_aBtnOK( this, SchResId( BTN_OK ) )
                , m_aBtnCancel( this, SchResId( BTN_CANCEL ) )
                , m_aBtnHelp( this, SchResId( BTN_HELP ) )
                , m_pChartTypeTabPage(0)
                , m_xChartModel(xChartModel)
                , m_xCC( xContext )
{
    FreeResource();

    this->SetText(String(SchResId(STR_PAGE_CHARTTYPE)));

    //don't create the tabpages before FreeResource, otherwise the help ids are not matched correctly
    m_pChartTypeTabPage = new ChartTypeTabPage(this,uno::Reference< XChartDocument >::query(m_xChartModel),m_xCC,true/*live update*/,true/*hide title description*/);
    m_pChartTypeTabPage->initializePage();
    m_pChartTypeTabPage->Show();
 }

ChartTypeDialog::~ChartTypeDialog()
{
    delete m_pChartTypeTabPage;
}

//.............................................................................
} //namespace chart
//.............................................................................
