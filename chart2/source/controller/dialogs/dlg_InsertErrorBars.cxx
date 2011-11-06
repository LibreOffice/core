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

#include "dlg_InsertErrorBars.hxx"
#include "dlg_InsertErrorBars.hrc"
#include "res_ErrorBar.hxx"
#include "ResourceIds.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "chartview/ExplicitValueProvider.hxx"
#include "ChartModelHelper.hxx"
#include "ObjectIdentifier.hxx"
#include "DiagramHelper.hxx"
#include "AxisHelper.hxx"
#include "ObjectNameProvider.hxx"

#include <com/sun/star/chart2/XAxis.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//.............................................................................
namespace chart
{
//.............................................................................

InsertErrorBarsDialog::InsertErrorBarsDialog(
    Window* pParent, const SfxItemSet& rMyAttrs,
    const uno::Reference< chart2::XChartDocument > & xChartDocument,
    ErrorBarResources::tErrorBarType eType /* = ErrorBarResources::ERROR_BAR_Y */ ) :
        ModalDialog( pParent, SchResId( DLG_DATA_YERRORBAR )),
        rInAttrs( rMyAttrs ),
        aBtnOK( this, SchResId( BTN_OK )),
        aBtnCancel( this, SchResId( BTN_CANCEL )),
        aBtnHelp( this, SchResId( BTN_HELP )),
        m_apErrorBarResources( new ErrorBarResources(
                                   this, this, rInAttrs,
                                   /* bNoneAvailable = */ true, eType ))
{
    FreeResource();
    this->SetText( ObjectNameProvider::getName_ObjectForAllSeries( OBJECTTYPE_DATA_ERRORS ) );

    m_apErrorBarResources->SetChartDocumentForRangeChoosing( xChartDocument );
}

InsertErrorBarsDialog::~InsertErrorBarsDialog()
{
}

void InsertErrorBarsDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apErrorBarResources->FillItemSet(rOutAttrs);
}

void InsertErrorBarsDialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    ModalDialog::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        m_apErrorBarResources->FillValueSets();
}

void InsertErrorBarsDialog::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    m_apErrorBarResources->SetAxisMinorStepWidthForErrorBarDecimals( fMinorStepWidth );
}

double InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals(
    const Reference< frame::XModel >& xChartModel,
    const Reference< uno::XInterface >& xChartView,
    const OUString& rSelectedObjectCID )
{
    double fStepWidth = 0.001;

    ExplicitValueProvider* pExplicitValueProvider( ExplicitValueProvider::getExplicitValueProvider(xChartView) );
    if( pExplicitValueProvider )
    {
        Reference< XAxis > xAxis;
        Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
        Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rSelectedObjectCID, xChartModel );
        xAxis = DiagramHelper::getAttachedAxis( xSeries, xDiagram );
        if(!xAxis.is())
            xAxis = AxisHelper::getAxis( 1/*nDimensionIndex*/, true/*bMainAxis*/, xDiagram );
        if(xAxis.is())
        {
            ExplicitScaleData aExplicitScale;
            ExplicitIncrementData aExplicitIncrement;
            pExplicitValueProvider->getExplicitValuesForAxis( xAxis,aExplicitScale, aExplicitIncrement );

            fStepWidth = aExplicitIncrement.Distance;
            if( !aExplicitIncrement.SubIncrements.empty() && aExplicitIncrement.SubIncrements[0].IntervalCount>0 )
                fStepWidth=fStepWidth/double(aExplicitIncrement.SubIncrements[0].IntervalCount);
            else
                fStepWidth/=10;
        }
    }

    return fStepWidth;
}

//.............................................................................
} //namespace chart
//.............................................................................
