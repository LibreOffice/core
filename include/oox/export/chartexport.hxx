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
#include <boost/unordered_map.hpp>
#include <map>

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

const sal_Int32  AXIS_PRIMARY_X = 1;
const sal_Int32  AXIS_PRIMARY_Y = 2;
const sal_Int32  AXIS_PRIMARY_Z = 3;
const sal_Int32  AXIS_SECONDARY_X = 4;
const sal_Int32  AXIS_SECONDARY_Y = 5;

struct AxisIdPair{
    sal_Int32 nAxisType;
    sal_Int32 nAxisId;
    sal_Int32 nCrossAx;

    AxisIdPair( sal_Int32 nType, sal_Int32 nId, sal_Int32 nAx ): nAxisType( nType ),nAxisId( nId ),nCrossAx( nAx ) {}
};

class OOX_DLLPUBLIC ChartExport : public DrawingML {

public:
    // first: data sequence for label, second: data sequence for values.
    typedef ::std::vector< AxisIdPair > AxisVector;

private:
    sal_Int32           mnXmlNamespace;
    sal_Int32           mnSeriesCount;
    Fraction            maFraction;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxChartModel;
    com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > mxDiagram;
    com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > mxNewDiagram;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    sal_Bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false
    OUString msChartAddress;
    ::com::sun::star::uno::Sequence< sal_Int32 > maSequenceMapping;

    //::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxAdditionalShapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > mxCategoriesValues;

    AxisVector          maAxes;
    sal_Bool            mbHasZAxis;
    sal_Bool            mbIs3DChart;


private:
    sal_Int32 getChartType();

    OUString parseFormula( const OUString& rRange );
    void InitPlotArea();

    void _ExportContent();
    void exportChartSpace( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc,
                      sal_Bool bIncludeTable );
    void exportChart( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc );
    void exportExternalData( com::sun::star::uno::Reference<
                              com::sun::star::chart::XChartDocument > rChartDoc );
    void exportLegend( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc );
    void exportTitle( com::sun::star::uno::Reference<
                          ::com::sun::star::drawing::XShape > xShape );
    void exportPlotArea( );
    void exportPlotAreaShapeProps( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet  );
    void exportFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet );
    void exportGradientFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet );
    void exportBitmapFill( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet );
    void exportDataTable( );

    void exportAreaChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportBarChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportBubbleChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportDoughnutChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportLineChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportOfPieChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportPieChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportRadarChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportScatterChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportStockChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportSuffaceChart( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );
    void exportHiLowLines();
    void exportUpDownBars(com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType );

    void exportSeries( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType, sal_Int32& nAttachedAxis );
    void exportCandleStickSeries(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > > & aSeriesSeq,
        sal_Bool bJapaneseCandleSticks, sal_Int32& nAttachedAxis );
    void exportSeriesText(
        const com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xValueSeq );
    void exportSeriesCategory(
        const com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xValueSeq );
    void exportSeriesValues(
        const com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xValueSeq, sal_Int32 nValueType = XML_val );
    void exportShapeProps( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet );
    void exportDataPoints(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesProperties,
        sal_Int32 nSeriesLength );
    void exportDataLabels(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xSeriesProperties,
        sal_Int32 nSeriesLength );
    void exportGrouping( sal_Bool isBar = sal_False );
    void exportTrendlines( ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > xSeries );
    void exportMarker();
    void exportSmooth();
    void exportFirstSliceAng();

    void exportErrorBar(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xErrorBarProps,
            bool bYError);

    void exportAxes( );
    void exportAxis( AxisIdPair aAxisIdPair );
    void _exportAxis(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xAxisProp,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xAxisTitle,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xMajorGrid,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xMinorGrid,
        sal_Int32 nAxisType,
        const char* sAxisPos,
        AxisIdPair aAxisIdPair );
    void exportAxesId( sal_Int32 nAttachedAxis );
    void exportView3D();
    sal_Bool isDeep3dChart();

public:

    ChartExport( sal_Int32 nXmlNamespace, ::sax_fastparser::FSHelperPtr pFS, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel, ::oox::core::XmlFilterBase* pFB = NULL, DocumentType eDocumentType = DOCUMENT_PPTX );
    virtual ~ChartExport() {}

    sal_Int32           GetChartID( );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getModel(){ return mxChartModel; }

    virtual ChartExport& WriteChartObj( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, sal_Int32 nChartCount );

    void ExportContent();
    void InitRangeSegmentationProperties(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );
};

}}

#endif /* ndef INCLUDED_OOX_EXPORT_CHARTEXPORT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
