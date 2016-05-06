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

#include <set>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <oox/dllapi.h>
#include <oox/export/drawingml.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/tokens.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sax/fshelper.hxx>

namespace com { namespace sun { namespace star {
    namespace beans {
        class XPropertySet;
    }
    namespace chart {
        class XDiagram;
        class XChartDocument;
    }
    namespace chart2 {
        struct RelativePosition;
        struct RelativeSize;
        class XDiagram;
        class XChartDocument;
        class XDataSeries;
        class XChartType;
        namespace data
        {
            class XDataSequence;
        }
    }
    namespace drawing {
        class XShape;
    }
    namespace frame {
        class XModel;
    }
}}}

namespace oox {
namespace core {
    class XmlFilterBase;
}}

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

    void ExportContent_();
    void exportChartSpace( const css::uno::Reference<
                           css::chart::XChartDocument >& rChartDoc,
                           bool bIncludeTable );
    void exportChart( const css::uno::Reference<
                          css::chart::XChartDocument >& rChartDoc );
    void exportExternalData( const css::uno::Reference<
                              css::chart::XChartDocument >& rChartDoc );
    void exportLegend( const css::uno::Reference<
                          css::chart::XChartDocument >& rChartDoc );
    void exportTitle( const css::uno::Reference<
                          css::drawing::XShape >& xShape );
    void exportPlotArea( );
    void exportPlotAreaShapeProps( const css::uno::Reference< css::beans::XPropertySet >& xPropSet  );
    void exportFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportGradientFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportBitmapFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportDataTable( );

    void exportAreaChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportBarChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportBubbleChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportDoughnutChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportLineChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportPieChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportRadarChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportScatterChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportStockChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportSurfaceChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportHiLowLines();
    void exportUpDownBars(const css::uno::Reference< css::chart2::XChartType >& xChartType );

    void exportAllSeries(const css::uno::Reference<css::chart2::XChartType>& xChartType, bool& rPrimaryAxes);
    void exportSeries(const css::uno::Reference< css::chart2::XChartType >& xChartType,
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
    void exportShapeProps( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportTextProps(const css::uno::Reference< css::beans::XPropertySet >& xPropSet);
    void exportDataPoints(
        const css::uno::Reference< css::beans::XPropertySet >& xSeriesProperties,
        sal_Int32 nSeriesLength );
    void exportDataLabels( const css::uno::Reference<css::chart2::XDataSeries>& xSeries, sal_Int32 nSeriesLength, sal_Int32 eChartType );
    void exportGrouping( bool isBar = false );
    void exportTrendlines( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );
    void exportMarker( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );
    void exportSmooth();
    void exportFirstSliceAng();

    void exportErrorBar(const css::uno::Reference< css::beans::XPropertySet >& xErrorBarProps,
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

    void exportMissingValueTreatment(const css::uno::Reference<css::beans::XPropertySet>& xPropSet);

    OUString getNumberFormatCode(sal_Int32 nKey) const;

public:

    ChartExport( sal_Int32 nXmlNamespace, ::sax_fastparser::FSHelperPtr pFS, css::uno::Reference< css::frame::XModel >& xModel, ::oox::core::XmlFilterBase* pFB = nullptr, DocumentType eDocumentType = DOCUMENT_PPTX );
    virtual ~ChartExport() {}

    sal_Int32           GetChartID( );
    const css::uno::Reference< css::frame::XModel >& getModel(){ return mxChartModel; }

    void WriteChartObj( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nChartCount );

    void ExportContent();
    void InitRangeSegmentationProperties(
        const css::uno::Reference<
            css::chart2::XChartDocument > & xChartDoc );
};

}}

#endif // INCLUDED_OOX_EXPORT_CHARTEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
