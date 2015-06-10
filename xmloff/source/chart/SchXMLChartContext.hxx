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

#include <vector>

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
    ::com::sun::star::uno::Any    maSymbolTypeDefault;
    ::com::sun::star::uno::Any    maDataCaptionDefault;

    ::com::sun::star::uno::Any    maErrorIndicatorDefault;
    ::com::sun::star::uno::Any    maErrorCategoryDefault;
    ::com::sun::star::uno::Any    maConstantErrorLowDefault;
    ::com::sun::star::uno::Any    maConstantErrorHighDefault;
    ::com::sun::star::uno::Any    maPercentageErrorDefault;
    ::com::sun::star::uno::Any    maErrorMarginDefault;

    ::com::sun::star::uno::Any    maMeanValueDefault;
    ::com::sun::star::uno::Any    maRegressionCurvesDefault;

    ::com::sun::star::uno::Any    maStackedDefault;
    ::com::sun::star::uno::Any    maPercentDefault;
    ::com::sun::star::uno::Any    maDeepDefault;
    ::com::sun::star::uno::Any    maStackedBarsConnectedDefault;

    //additional information
    ::com::sun::star::uno::Any    maLinesOnProperty;

    //styles for series and datapoints
    ::std::vector< DataRowPointStyle > maSeriesStyleList;
    ::std::vector< RegressionStyle >   maRegressionStyleList;
};

class SchXMLChartContext : public SvXMLImportContext
{
public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const OUString& rLocalName );
    virtual ~SchXMLChartContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                                     com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;

private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    OUString maMainTitle, maSubTitle;
    com::sun::star::awt::Point maMainTitlePos, maSubTitlePos, maLegendPos;
    OUString m_aXLinkHRefAttributeToIndicateDataProvider;
    bool m_bHasRangeAtPlotArea;
    bool m_bHasTableElement;
    bool mbAllRangeAddressesAvailable;
    bool mbColHasLabels;
    bool mbRowHasLabels;
    ::com::sun::star::chart::ChartDataRowSource meDataRowSource;
    bool mbIsStockChart;

    OUString msCategoriesAddress;
    OUString msChartAddress;

    SeriesDefaultsAndStyles maSeriesDefaultsAndStyles;
    tSchXMLLSequencesPerIndex maLSequencesPerIndex;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxDrawPage;
    OUString msColTrans;
    OUString msRowTrans;
    OUString maChartTypeServiceName;

    ::com::sun::star::awt::Size maChartSize;

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
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxTitleShape;
    OUString msAutoStyleName;

public:
    SchXMLTitleContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const OUString& rLocalName,
                        OUString& rTitle,
                        com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xTitleShape );
    virtual ~SchXMLTitleContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                               com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
};

#endif // INCLUDED_XMLOFF_SOURCE_CHART_SCHXMLCHARTCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
