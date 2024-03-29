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
#include <com/sun/star/chart2/FormattedString.hpp>
#include <oox/dllapi.h>
#include <oox/export/drawingml.hxx>
#include <oox/export/shapes.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/tokens.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sax/fshelper.hxx>

namespace com::sun::star {
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
            class XLabeledDataSequence;
        }
    }
    namespace drawing {
        class XShape;
    }
    namespace frame {
        class XModel;
    }
}

namespace oox {
namespace core {
    class XmlFilterBase;
}}

namespace oox::drawingml {

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

/**
 A helper container class to collect the chart data point labels and the address
 of the cell[range] from which the labels are sourced if that is the case. This
 is then used to write the label texts under the extension tag <c15:datalabelsRange>.

 @since LibreOffice 7.3.0
 */
class DataLabelsRange
{
public:

    /// type of the internal container that stores the indexed label text.
    typedef std::map<sal_Int32, OUString> LabelsRangeMap;

    /// Returns whether the container is empty or not.
    bool empty() const;
    /// Returns the count of labels stored.
    size_t count() const;
    /// Indicates whether the container has a label with index specified by nIndex.
    bool hasLabel(sal_Int32 nIndex) const;
    /// Returns the address of the cell[range] from which label contents are sourced.
    const OUString & getRange() const;

    /// Sets the address of the cell[range] from which label contents are sourced.
    void setRange(const OUString& rRange);
    /// Adds a new indexed label text.
    void setLabel(sal_Int32 nIndex, const OUString& rText);

    LabelsRangeMap::const_iterator begin() const;
    LabelsRangeMap::const_iterator end() const;

private:
    OUString         maRange;
    LabelsRangeMap   maLabels;
};


class ChartExport final : public DrawingML {

public:
    // first: data sequence for label, second: data sequence for values.
    typedef ::std::vector< AxisIdPair > AxisVector;

private:
    sal_Int32           mnXmlNamespace;
    sal_Int32           mnSeriesCount;
    css::uno::Reference< css::frame::XModel > mxChartModel;
    css::uno::Reference< css::chart::XDiagram > mxDiagram;
    css::uno::Reference< css::chart2::XDiagram > mxNewDiagram;
    std::shared_ptr<URLTransformer> mpURLTransformer;

    // members filled by InitRangeSegmentationProperties (retrieved from DataProvider)
    bool mbHasCategoryLabels; //if the categories are only automatically generated this will be false

    //css::uno::Reference< css::drawing::XShapes > mxAdditionalShapes;
    css::uno::Reference< css::chart2::data::XDataSequence > mxCategoriesValues;

    AxisVector          maAxes;
    bool                mbHasZAxis;
    bool                mbIs3DChart;
    bool                mbStacked;
    bool                mbPercent;
    bool                mbHasDateCategories;

    std::set<sal_Int32> maExportedAxis;

private:
    sal_Int32 getChartType();

    css::uno::Sequence< css::uno::Sequence< rtl::OUString > > getSplitCategoriesList(const OUString& rRange);

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
    void exportTitle( const css::uno::Reference< css::drawing::XShape >& xShape,
                          const css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >& xFormattedSubTitle =
                          css::uno::Sequence< css::uno::Reference< css::chart2::XFormattedString > >() );
    void exportPlotArea( const css::uno::Reference<
                             css::chart::XChartDocument >& rChartDoc );
    void exportAdditionalShapes( const css::uno::Reference<css::chart::XChartDocument >& rChartDoc );
    void exportFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportSolidFill(const css::uno::Reference<css::beans::XPropertySet>& xPropSet);
    void exportGradientFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportBitmapFill( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportHatch(const css::uno::Reference<css::beans::XPropertySet>& xPropSet);
    void exportDataTable( );

    void exportAreaChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportBarChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportBubbleChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportDoughnutChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportLineChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportOfPieChart( const css::uno::Reference< css::chart2::XChartType >&
            xChartType, const char* s_subtype );
    void exportPieChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportRadarChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportScatterChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportScatterChartSeries( const css::uno::Reference< css::chart2::XChartType >& xChartType,
            const css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries>>* pSeries);
    void exportStockChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportSurfaceChart( const css::uno::Reference< css::chart2::XChartType >& xChartType );
    void exportHiLowLines();
    void exportUpDownBars(const css::uno::Reference< css::chart2::XChartType >& xChartType );

    void exportAllSeries(const css::uno::Reference<css::chart2::XChartType>& xChartType, bool& rPrimaryAxes);
    void exportSeries(const css::uno::Reference< css::chart2::XChartType >& xChartType,
            const css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries> >& rSeriesSeq, bool& rPrimaryAxes);

    void exportVaryColors(const css::uno::Reference<css::chart2::XChartType>& xChartType);
    void exportCandleStickSeries(
        const css::uno::Sequence<
            css::uno::Reference<
                css::chart2::XDataSeries > > & aSeriesSeq,
        bool& rPrimaryAxes );
    void exportSeriesText(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xValueSeq );
    void exportSeriesCategory(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xValueSeq, sal_Int32 nValueType = XML_cat );
    void exportSeriesValues(
        const css::uno::Reference< css::chart2::data::XDataSequence >& xValueSeq, sal_Int32 nValueType = XML_val );
    void exportShapeProps( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportDataPoints(
        const css::uno::Reference< css::beans::XPropertySet >& xSeriesProperties,
        sal_Int32 nSeriesLength, sal_Int32 eChartType );
    void exportDataLabels( const css::uno::Reference<css::chart2::XDataSeries>& xSeries, sal_Int32 nSeriesLength,
        sal_Int32 eChartType, DataLabelsRange& rDLblsRange );
    void exportGrouping( bool isBar = false );
    void exportTrendlines( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );
    void exportMarker( const css::uno::Reference< css::beans::XPropertySet >& xPropSet );
    void exportSmooth();
    void exportFirstSliceAng();

    void exportErrorBar(const css::uno::Reference< css::beans::XPropertySet >& xErrorBarProps,
            bool bYError);

    void exportManualLayout(const css::chart2::RelativePosition& rPos, const css::chart2::RelativeSize& rSize, const bool bIsExcludingDiagramPositioning);

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
    void exportAxesId(bool bPrimaryAxes, bool bCheckCombinedAxes = false);
    void exportView3D();
    bool isDeep3dChart();

    void exportMissingValueTreatment(const css::uno::Reference<css::beans::XPropertySet>& xPropSet);

    OUString getNumberFormatCode(sal_Int32 nKey) const;

public:

    OOX_DLLPUBLIC ChartExport( sal_Int32 nXmlNamespace, ::sax_fastparser::FSHelperPtr pFS, css::uno::Reference< css::frame::XModel > const & xModel,
                 ::oox::core::XmlFilterBase* pFB, DocumentType eDocumentType );
    virtual ~ChartExport() {}

    OOX_DLLPUBLIC void SetURLTranslator(const std::shared_ptr<URLTransformer>& pTransformer);

    const css::uno::Reference< css::frame::XModel >& getModel() const { return mxChartModel; }

    OOX_DLLPUBLIC void WriteChartObj( const css::uno::Reference< css::drawing::XShape >& xShape, sal_Int32 nID, sal_Int32 nChartCount );
    void exportTextProps(const css::uno::Reference< css::beans::XPropertySet >& xPropSet);

    OOX_DLLPUBLIC void ExportContent();
    void InitRangeSegmentationProperties(
        const css::uno::Reference<
            css::chart2::XChartDocument > & xChartDoc );
};

}

#endif // INCLUDED_OOX_EXPORT_CHARTEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
