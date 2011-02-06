/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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

#ifndef _OOX_EXPORT_CHART_HXX_
#define _OOX_EXPORT_CHART_HXX_

#include <oox/dllapi.h>
#include <com/sun/star/uno/XReference.hpp>
#include <oox/export/drawingml.hxx>
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
    Fraction            maFraction;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > mxChartModel;
    com::sun::star::uno::Reference< com::sun::star::chart::XDiagram > mxDiagram;
    com::sun::star::uno::Reference< com::sun::star::chart2::XDiagram > mxNewDiagram;

    rtl::OUString msTableName;
    rtl::OUStringBuffer msStringBuffer;
    rtl::OUString msString;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    sal_Bool mbHasSeriesLabels;
    sal_Bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false
    sal_Bool mbRowSourceColumns;
    rtl::OUString msChartAddress;
    rtl::OUString msTableNumberList;
    ::com::sun::star::uno::Sequence< sal_Int32 > maSequenceMapping;

    //::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes > mxAdditionalShapes;
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence > mxCategoriesValues;

    AxisVector          maAxes;
    sal_Bool            mbHasXAxis;
    sal_Bool            mbHasYAxis;
    sal_Bool            mbHasZAxis;
    sal_Bool            mbHasSecondaryXAxis;
    sal_Bool            mbHasSecondaryYAxis;
    sal_Bool            mbIs3DChart;


private:
    sal_Int32 getChartType(
         );

    rtl::OUString parseFormula( const rtl::OUString& rRange );
    void InitPlotArea();

    void _ExportContent();
    void exportChartSpace( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc,
                      sal_Bool bIncludeTable );
    void exportChart( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc );
    void exportLegend( com::sun::star::uno::Reference<
                          com::sun::star::chart::XChartDocument > rChartDoc );
    void exportTitle( com::sun::star::uno::Reference<
                          ::com::sun::star::drawing::XShape > xShape );
    void exportPlotArea( );

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

    void exportSeries( com::sun::star::uno::Reference< com::sun::star::chart2::XChartType > xChartType, sal_Int32& nAttachedAxis );
    void exportCandleStickSeries(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > > & aSeriesSeq,
        sal_Bool bJapaneseCandleSticks, sal_Int32& nAttachedAxis );
    void exportDataSeq(
        const com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSequence >& xValueSeq,
        sal_Int32 elementTokenId );
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
    void exportGrouping( sal_Bool isBar = sal_False );
    void exportMarker();
    void exportSmooth();
    void exportFirstSliceAng();

    void exportAxes( );
    void exportXAxis( AxisIdPair aAxisIdPair );
    void exportYAxis( AxisIdPair aAxisIdPair );
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

    sal_Int32           GetXmlNamespace() const;
    ChartExport&        SetXmlNamespace( sal_Int32 nXmlNamespace );
    sal_Int32           GetChartID( );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > getModel(){ return mxChartModel; }

    virtual ChartExport& WriteChartObj( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& xShape, sal_Int32 nChartCount );

    void ExportContent();
    void InitRangeSegmentationProperties(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );
};

}}

#endif /* ndef _OOX_EXPORT_CHART_HXX_ */
