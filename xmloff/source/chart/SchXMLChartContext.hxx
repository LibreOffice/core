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
#ifndef INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLCHARTCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLCHARTCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/awt/Size.hpp>

#include "transporttypes.hxx"

#include <list>

class SchXMLImportHelper;

namespace com { namespace sun { namespace star {
    namespace chart {
        class XChartDocument;
        struct ChartSeriesAddress;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    }}
    namespace drawing {
        class XShapes;
    }
}}}

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
    ::std::list< DataRowPointStyle > maSeriesStyleList;
    ::std::list< RegressionStyle >   maRegressionStyleList;
};

class SchXMLChartContext : public SvXMLImportContext
{
public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const OUString& rLocalName );
    virtual ~SchXMLChartContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    OUString maMainTitle, maSubTitle;
    css::awt::Point maMainTitlePos, maSubTitlePos, maLegendPos;
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
        @param  bSetWitchData   Indicates whether the data set takes it's data series from
            rows or from columns.
    */
    void    InitChart   (const OUString & rChartTypeServiceName,
                        bool bSetSwitchData);

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
                        SvXMLImport& rImport, const OUString& rLocalName,
                        OUString& rTitle,
                        css::uno::Reference< css::drawing::XShape >& xTitleShape );
    virtual ~SchXMLTitleContext();

    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLCHARTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
