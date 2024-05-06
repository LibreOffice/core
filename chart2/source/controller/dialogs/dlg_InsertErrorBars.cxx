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

#include <dlg_InsertErrorBars.hxx>
#include <res_ErrorBar.hxx>
#include <chartview/ExplicitScaleValues.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <ChartModel.hxx>
#include <ChartView.hxx>
#include <ObjectIdentifier.hxx>
#include <Diagram.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <ObjectNameProvider.hxx>
#include <DataSeries.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart
{

InsertErrorBarsDialog::InsertErrorBarsDialog(
    weld::Window* pParent, const SfxItemSet& rMyAttrs,
    const rtl::Reference<::chart::ChartModel> & xChartDocument,
    ErrorBarResources::tErrorBarType eType /* = ErrorBarResources::ERROR_BAR_Y */ )
        : GenericDialogController(pParent, u"modules/schart/ui/dlg_InsertErrorBars.ui"_ustr, u"dlg_InsertErrorBars"_ustr)
        , m_apErrorBarResources( new ErrorBarResources(
                                   m_xBuilder.get(), this, rMyAttrs,
                                   /* bNoneAvailable = */ true, eType ))
{
    ObjectType objType = eType == ErrorBarResources::ERROR_BAR_Y ? OBJECTTYPE_DATA_ERRORS_Y : OBJECTTYPE_DATA_ERRORS_X;

    m_xDialog->set_title(ObjectNameProvider::getName_ObjectForAllSeries(objType));

    m_apErrorBarResources->SetChartDocumentForRangeChoosing( xChartDocument );
}

void InsertErrorBarsDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apErrorBarResources->FillItemSet(rOutAttrs);
}

void InsertErrorBarsDialog::SetAxisMinorStepWidthForErrorBarDecimals( double fMinorStepWidth )
{
    m_apErrorBarResources->SetAxisMinorStepWidthForErrorBarDecimals( fMinorStepWidth );
}

double InsertErrorBarsDialog::getAxisMinorStepWidthForErrorBarDecimals(
    const rtl::Reference<::chart::ChartModel>& xChartModel,
    const rtl::Reference<::chart::ChartView>& xChartView,
    std::u16string_view rSelectedObjectCID )
{
    double fStepWidth = 0.001;

    ExplicitValueProvider* pExplicitValueProvider( xChartView.get() );
    if( pExplicitValueProvider )
    {
        rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
        rtl::Reference< DataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( rSelectedObjectCID, xChartModel );
        rtl::Reference< Axis > xAxis = xDiagram->getAttachedAxis( xSeries );
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
