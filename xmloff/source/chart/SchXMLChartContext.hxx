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
#pragma once

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/awt/Size.hpp>

#include "transporttypes.hxx"

#include <vector>

class SchXMLImportHelper;

namespace com::sun::star {
    namespace chart {
        class XChartDocument;
        struct ChartSeriesAddress;
    }
    namespace xml::sax {
        class XAttributeList;
    }
    namespace drawing {
        class XShapes;
    }
}

struct SeriesDefaultsAndStyles
{
    //default values for series:
    css::uno::Any    maSymbolTypeDefault;
    css::uno::Any    maDataCaptionDefault;

    css::uno::Any    maErrorIndicatorDefault;
    css::uno::Any    maErrorCategoryDefault;
    css::uno::Any    maConstantErrorLowDefault;
    css::uno::Any    maConstantErrorHighDefault;
    css::uno::Any    maPercentageErrorDefault;
    css::uno::Any    maErrorMarginDefault;

    css::uno::Any    maMeanValueDefault;
    css::uno::Any    maRegressionCurvesDefault;

    css::uno::Any    maStackedDefault;
    css::uno::Any    maPercentDefault;
    css::uno::Any    maDeepDefault;
    css::uno::Any    maStackedBarsConnectedDefault;

    //additional information
    css::uno::Any    maLinesOnProperty;

    //styles for series and datapoints
    ::std::vector< DataRowPointStyle > maSeriesStyleVector;
    ::std::vector< RegressionStyle >   maRegressionStyleVector;
};

class SchXMLChartContext : public SvXMLImportContext
{
public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport );
    virtual ~SchXMLChartContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    OUString maMainTitle, maSubTitle;
    OUString m_aXLinkHRefAttributeToIndicateDataProvider;
    bool m_bHasRangeAtPlotArea;
    bool m_bHasTableElement;
    bool mbAllRangeAddressesAvailable;
    bool mbColHasLabels;
    bool mbRowHasLabels;
    css::chart::ChartDataRowSource meDataRowSource;
    bool mbIsStockChart;

    OUString msCategoriesAddress;
    OUString msChartAddress;

    OUString msDataPilotSource;

    SeriesDefaultsAndStyles maSeriesDefaultsAndStyles;
    tSchXMLLSequencesPerIndex maLSequencesPerIndex;

    css::uno::Reference< css::drawing::XShapes > mxDrawPage;
    OUString msColTrans;
    OUString msRowTrans;
    OUString maChartTypeServiceName;

    css::awt::Size maChartSize;

    /** @descr  This method bundles some settings to the chart model and executes them with
            a locked controller.  This includes setting the chart type.
        @param  aServiceName The name of the service the diagram is initialized with.
    */
    void    InitChart   (const OUString & rChartTypeServiceName);

    void MergeSeriesForStockChart();
};

class SchXMLTitleContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    OUString& mrTitle;
    css::uno::Reference< css::drawing::XShape > mxTitleShape;
    OUString msAutoStyleName;

public:
    SchXMLTitleContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport,
                        OUString& rTitle,
                        css::uno::Reference< css::drawing::XShape > const & xTitleShape );
    virtual ~SchXMLTitleContext() override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
