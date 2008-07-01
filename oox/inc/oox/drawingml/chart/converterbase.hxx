/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: converterbase.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef OOX_DRAWINGML_CHART_CONVERTERBASE_HXX
#define OOX_DRAWINGML_CHART_CONVERTERBASE_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/drawingml/chart/chartcontextbase.hxx"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace chart2 { class XChartDocument; }
} } }

namespace oox { namespace core {
    class XmlFilterBase;
} }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

/** Enumerates different object types for specific automatic formatting behaviour. */
enum ObjectType
{
    OBJECTTYPE_CHARTSPACE,              /// Chart background.
    OBJECTTYPE_CHARTTITLE,              /// Chart title.
    OBJECTTYPE_LEGEND,                  /// Legend.
    OBJECTTYPE_PLOTAREA2D,              /// Plot area containing axes and data series in 2D charts.
    OBJECTTYPE_PLOTAREA3D,              /// Plot area containing axes and data series in 3D charts.
    OBJECTTYPE_WALL,                    /// Background and side wall in 3D charts.
    OBJECTTYPE_FLOOR,                   /// Floor in 3D charts.
    OBJECTTYPE_AXIS,                    /// Axis line, labels, tick marks.
    OBJECTTYPE_AXISTITLE,               /// Axis title.
    OBJECTTYPE_AXISUNIT,                /// Axis unit label.
    OBJECTTYPE_GRIDLINE,                /// Axis grid line.
    OBJECTTYPE_LINEARSERIES2D,          /// Linear series in 2D line/radarline/scatter charts.
    OBJECTTYPE_FILLEDSERIES2D,          /// Filled series in 2D bar/area/radararea/bubble/pie/surface charts.
    OBJECTTYPE_FILLEDSERIES3D,          /// Filled series in 3D charts.
    OBJECTTYPE_DATALABEL,               /// Labels for data points.
    OBJECTTYPE_TRENDLINE,               /// Data series trend line.
    OBJECTTYPE_TRENDLINELABEL,          /// Trend line label.
    OBJECTTYPE_ERRORBAR,                /// Data series error indicator line.
    OBJECTTYPE_SERLINE,                 /// Data point connector lines.
    OBJECTTYPE_LEADERLINE,              /// Leader lines between pie slice and data label.
    OBJECTTYPE_DROPLINE,                /// Drop lines between data points and X axis.
    OBJECTTYPE_HILOLINE,                /// High/low lines in line/stock charts.
    OBJECTTYPE_UPBAR,                   /// Up-bar in line/stock charts.
    OBJECTTYPE_DOWNBAR,                 /// Down-bar in line/stock charts.
    OBJECTTYPE_DATATABLE                /// Data table.
};

// ============================================================================

class ChartConverter;
struct ConverterData;

class ConverterRoot
{
public:
    explicit            ConverterRoot(
                            ::oox::core::XmlFilterBase& rFilter,
                            ChartConverter& rChartConverter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc );
    virtual             ~ConverterRoot();

    /** Creates an instance for the passed service name, using the passed service factory. */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        createInstance(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory,
                            const ::rtl::OUString& rServiceName );

    /** Creates an instance for the passed service name, using the process service factory. */
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        createInstance( const ::rtl::OUString& rServiceName );

protected:
    /** Returns the filter object of the imported/exported document. */
    ::oox::core::XmlFilterBase& getFilter() const;
    /** Returns the chart converter. */
    ChartConverter&     getChartConverter() const;
    /** Returns the API chart document model. */
    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >
                        getChartDocument() const;

    /** Initializes the automatic formatting information. */
    void                initAutoFormats( sal_Int32 nStyle );

    /** Sets auto formatting properties to the passed property set. */
    void                convertAutoFormats( PropertySet& rPropSet, ObjectType eObjType ) const;

private:
    ::boost::shared_ptr< ConverterData > mxData;
};

// ============================================================================

template< typename ModelType >
class ConverterBase : public ConverterRoot
{
public:
    inline const ModelType& getModel() const { return mrModel; }

protected:
    inline explicit     ConverterBase( const ConverterRoot& rParent, ModelType& rModel ) :
                            ConverterRoot( rParent ), mrModel( rModel ) {}
    virtual             ~ConverterBase() {}

protected:
    ModelType&          mrModel;
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

