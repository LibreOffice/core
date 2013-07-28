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

using ::com::sun::star::uno::Reference;
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart
{

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
    ObjectType objType = eType == ErrorBarResources::ERROR_BAR_Y ? OBJECTTYPE_DATA_ERRORS_Y : OBJECTTYPE_DATA_ERRORS_X;

    this->SetText( ObjectNameProvider::getName_ObjectForAllSeries(objType) );

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

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
