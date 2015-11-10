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

#ifndef INCLUDED_OOX_EXPORT_CHARTEXPORT_HXX
#define INCLUDED_OOX_EXPORT_CHARTEXPORT_HXX

#include <oox/dllapi.h>
#include <com/sun/star/uno/XReference.hpp>
#include <oox/export/drawingml.hxx>
#include <oox/token/tokens.hxx>
#include <sax/fshelper.hxx>
#include <vcl/mapmod.hxx>

#include <com/sun/star/chart2/RelativePosition.hpp>
#include <com/sun/star/chart2/RelativeSize.hpp>

#include <set>

namespace com { namespace sun { namespace star {
    namespace chart {
        class XDiagram;
        class XChartDocument;
        class XChartDataArray;
        struct ChartSeriesAddress;
    }
    namespace chart2 {
        class XDiagram;
        class XChartDocument;
        class XDataSeries;
        class XChartType;
        namespace data
        {
            class XDataProvider;
            class XDataSequence;
        }
    }
    namespace drawing {
        class XShape;
        class XShapes;
    }
    namespace task {
        class XStatusIndicator;
    }
    namespace frame {
        class XModel;
    }
}}}

namespace oox { namespace drawingml {

enum AxesType
{
    AXIS_PRIMARY_X = 1,
    AXIS_PRIMARY_Y = 2,
    AXIS_PRIMARY_Z = 3,
    AXIS_SECONDARY_X = 4,
    AXIS_SECONDARY_Y = 5
};

struct AxisIdPair{
    AxesType nAxisType;
    sal_Int32 nAxisId;
    sal_Int32 nCrossAx;

    AxisIdPair(AxesType nType, sal_Int32 nId, sal_Int32 nAx)
        : nAxisType(nType)
        , nAxisId(nId)
        , nCrossAx(nAx)
    {}
};

class OOX_DLLPUBLIC ChartExport : public DrawingML {

public:
    // first: data sequence for label, second: data sequence for values.
    typedef ::std::vector< AxisIdPair > AxisVector;

private:
    sal_Int32           mnXmlNamespace;
    sal_Int32           mnSeriesCount;
    css::uno::Reference< css::frame::XModel > mxChartModel;
    css::uno::Reference< css::chart::XDiagram > mxDiagram;
    css::uno::Reference< css::chart2::XDiagram > mxNewDiagram;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false
    OUString msChartAddress;
    css::uno::Sequence< sal_Int32 > maSequenceMapping;

    //css::uno::Reference< css::drawing::XShapes > mxAdditionalShapes;
    css::uno::Reference< css::chart2::data::XDataSequence > mxCategoriesValues;

    AxisVector          maAxes;
    bool                mbHasZAxis;
    bool                mbIs3DChart;
    bool                mbStacked;
    bool                mbPercent;

    std::set<sal_Int32> maExportedAxis;

private:
    sal_Int32 getChartType();

    OUString parseFormula( const OUString& rRange );
    void InitPlotArea();

    void _ExportContent();
    void exportChartSpace( css::uno::Reference<
                           css::chart::XChartDocument > rChartDoc,
                           bool bIncludeTable );
    void exportChart( css::uno::Reference<
                          css::chart::XChartDocument > rChartDoc );
    void exportExternalData( css::uno::Reference<
                              css::chart::XChartDocument > rChartDoc );
    void exportLegend( css::uno::Reference<
                          css::chart::XChartDocument > rChartDoc );
    void exportTitle( css::uno::Reference<
                          css::drawing::XShape > xShape );
    void exportPlotArea( );
    void exportPlotAreaShapeProps( css::uno::Reference< css::beans::XPropertySet > xPropSet  );
    void exportFill( css::uno::Reference< css::beans::XPropertySet > xPropSet );
    void exportGradientFill( css::uno::Reference< css::beans::XPropertySet > xPropSet );
    void exportBitmapFill( css::uno::Reference< css::beans::XPropertySet > xPropSet );
    void exportDataTable( );

    void exportAreaChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportBarChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportBubbleChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportDoughnutChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportLineChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportPieChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportRadarChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportScatterChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportStockChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportSurfaceChart( css::uno::Reference< css::chart2::XChartType > xChartType );
    void exportHiLowLines();
    void exportUpDownBars(css::uno::Reference< css::chart2::XChartType > xChartType );

    void exportAllSeries(css::uno::Reference<css::chart2::XChartType> xChartType, bool& rPrimaryAxes);
    void exportSeries(css::uno::Reference< css::chart2::XChartType > xChartType,
            css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries> >& rSeriesSeq, bool& rPrimaryAxes);
    void exportCandleStickSeries(
        const css::uno::Sequence<
            css::uno::Reference<
                css::chart2::XDataSeries > > & aSeriesSeq,
        bool bJapaneseCandleSticks, bool& rPrimaryAxes );
    void exportSeriesText(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xValueSeq );
    void exportSeriesCategory(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xValueSeq );
    void exportSeriesValues(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xValueSeq, sal_Int32 nValueType = XML_val );
    void exportShapeProps( css::uno::Reference< css::beans::XPropertySet > xPropSet );
    void exportTextProps(css::uno::Reference< css::beans::XPropertySet > xPropSet);
    void exportDataPoints(
        const css::uno::Reference< css::beans::XPropertySet >& xSeriesProperties,
        sal_Int32 nSeriesLength );
    void exportDataLabels( const css::uno::Reference<css::chart2::XDataSeries>& xSeries, sal_Int32 nSeriesLength, sal_Int32 eChartType );
    void exportGrouping( bool isBar = false );
    void exportTrendlines( css::uno::Reference< css::chart2::XDataSeries > xSeries );
    void exportMarker( css::uno::Reference< css::chart2::XDataSeries > xSeries );
    void exportSmooth();
    void exportFirstSliceAng();

    void exportErrorBar(css::uno::Reference< css::beans::XPropertySet > xErrorBarProps,
            bool bYError);

    void exportManualLayout(const css::chart2::RelativePosition& rPos, const css::chart2::RelativeSize& rSize);

    void exportAxes( );
    void exportAxis(const AxisIdPair& rAxisIdPair);
    void _exportAxis(
        const css::uno::Reference< css::beans::XPropertySet >& xAxisProp,
        const css::uno::Reference< css::drawing::XShape >& xAxisTitle,
        const css::uno::Reference< css::beans::XPropertySet >& xMajorGrid,
        const css::uno::Reference< css::beans::XPropertySet >& xMinorGrid,
        sal_Int32 nAxisType,
        const char* sAxisPos,
        const AxisIdPair& rAxisIdPair );
    void exportAxesId(bool bPrimaryAxes);
    void exportView3D();
    bool isDeep3dChart();

    void exportMissingValueTreatment(css::uno::Reference<
            css::beans::XPropertySet> xPropSet);

    OUString getNumberFormatCode(sal_Int32 nKey) const;

public:

    ChartExport( sal_Int32 nXmlNamespace, ::sax_fastparser::FSHelperPtr pFS, css::uno::Reference< css::frame::XModel >& xModel, ::oox::core::XmlFilterBase* pFB = nullptr, DocumentType eDocumentType = DOCUMENT_PPTX );
    virtual ~ChartExport() {}

    sal_Int32           GetChartID( );
    css::uno::Reference< css::frame::XModel > getModel(){ return mxChartModel; }

    ChartExport& WriteChartObj( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nChartCount );

    void ExportContent();
    void InitRangeSegmentationProperties(
        const css::uno::Reference<
            css::chart2::XChartDocument > & xChartDoc );
};

}}

#endif // INCLUDED_OOX_EXPORT_CHARTEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
