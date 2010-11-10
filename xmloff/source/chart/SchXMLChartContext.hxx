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
#ifndef _SCH_XMLCHARTCONTEXT_HXX_
#define _SCH_XMLCHARTCONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/awt/Size.hpp>

#include "transporttypes.hxx"

#include <list>
#include <map>

class SchXMLImport;
class SchXMLImportHelper;

namespace com { namespace sun { namespace star {
    namespace chart {
        class XChartDocument;
        class XDiagram;
        struct ChartSeriesAddress;
    }
    namespace util {
        class XStringMapping;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    }}
    namespace drawing {
        class XShapes;
    }
}}}

// ----------------------------------------

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
    ::std::list< DataRowPointStyle > maSeriesStyleList;
};

class SchXMLChartContext : public SvXMLImportContext
{
public:
    SchXMLChartContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const rtl::OUString& rLocalName );
    virtual ~SchXMLChartContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                                     com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<
            com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    SchXMLTable maTable;
    SchXMLImportHelper& mrImportHelper;

    ::rtl::OUString maMainTitle, maSubTitle;
    com::sun::star::awt::Point maMainTitlePos, maSubTitlePos, maLegendPos;
    ::rtl::OUString m_aXLinkHRefAttributeToIndicateDataProvider;
    bool m_bHasRangeAtPlotArea;
    bool m_bHasTableElement;
    sal_Bool mbAllRangeAddressesAvailable;
    sal_Bool mbColHasLabels;
    sal_Bool mbRowHasLabels;
    ::com::sun::star::chart::ChartDataRowSource meDataRowSource;
    bool mbIsStockChart;

    com::sun::star::uno::Sequence< com::sun::star::chart::ChartSeriesAddress > maSeriesAddresses;
    ::rtl::OUString msCategoriesAddress;
    ::rtl::OUString msChartAddress;

    SeriesDefaultsAndStyles maSeriesDefaultsAndStyles;
    tSchXMLLSequencesPerIndex maLSequencesPerIndex;

    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxDrawPage;
    ::rtl::OUString msColTrans;
    ::rtl::OUString msRowTrans;
    ::rtl::OUString maChartTypeServiceName;

    ::com::sun::star::awt::Size maChartSize;

    /** @descr  This method bundles some settings to the chart model and executes them with
            a locked controller.  This includes setting the chart type.
        @param  aServiceName The name of the service the diagram is initialized with.
        @param  bSetWitchData   Indicates wether the data set takes it's data series from
            rows or from columns.
    */
    void    InitChart   (const ::rtl::OUString & rChartTypeServiceName,
                        sal_Bool bSetSwitchData);

    void MergeSeriesForStockChart();
};

// ----------------------------------------

class SchXMLTitleContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;
    rtl::OUString& mrTitle;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxTitleShape;
    rtl::OUString msAutoStyleName;

public:
    SchXMLTitleContext( SchXMLImportHelper& rImpHelper,
                        SvXMLImport& rImport, const rtl::OUString& rLocalName,
                        rtl::OUString& rTitle,
                        com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xTitleShape );
    virtual ~SchXMLTitleContext();

    virtual void StartElement( const com::sun::star::uno::Reference<
                               com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ----------------------------------------

class SchXMLLegendContext : public SvXMLImportContext
{
private:
    SchXMLImportHelper& mrImportHelper;

public:
    SchXMLLegendContext( SchXMLImportHelper& rImpHelper,
                         SvXMLImport& rImport, const rtl::OUString& rLocalName );
    virtual ~SchXMLLegendContext();

    virtual void StartElement( const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  // _SCH_XMLCHARTCONTEXT_HXX_
