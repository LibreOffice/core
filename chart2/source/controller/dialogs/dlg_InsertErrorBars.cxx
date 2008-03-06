/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertErrorBars.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:33:13 $
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
    this->SetText( String( SchResId( STR_PAGE_YERROR_BARS )));
    m_apErrorBarResources->SetChartDocumentForRangeChoosing( xChartDocument );
}

InsertErrorBarsDialog::~InsertErrorBarsDialog()
{
}

void InsertErrorBarsDialog::Reset()
{
    m_apErrorBarResources->Reset(rInAttrs);
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

//static
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
            if( aExplicitIncrement.SubIncrements.getLength()  && aExplicitIncrement.SubIncrements[0].IntervalCount>0 )
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
