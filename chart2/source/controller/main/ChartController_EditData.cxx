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
#include "ChartController.hxx"
#include "macros.hxx"

#include "dlg_DataEditor.hxx"
#include "DataSourceHelper.hxx"
#include "DiagramHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "UndoGuard.hxx"
#include "ResId.hxx"
#include "Strings.hrc"

// for RET_OK
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

void ChartController::executeDispatch_EditData()
{
    Reference< chart2::XChartDocument > xChartDoc( getModel(), uno::UNO_QUERY );
    if( xChartDoc.is())
    {
        Window* pParent( NULL );

        Reference< ::com::sun::star::chart2::data::XDataProvider > xDataProvider( xChartDoc->getDataProvider());

        {
            // /--
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
            // using assignment for broken gcc 3.3
            UndoLiveUpdateGuardWithData aUndoGuard = UndoLiveUpdateGuardWithData(
                String( SchResId( STR_ACTION_EDIT_CHART_DATA )),
                m_xUndoManager );
            DataEditor aDataEditorDialog( pParent, xChartDoc, m_xCC );
            // the dialog has no OK/Cancel
            aDataEditorDialog.Execute();
            aUndoGuard.commit();
            // \--
        }
    }
}

} //  namespace chart
