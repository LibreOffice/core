/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
