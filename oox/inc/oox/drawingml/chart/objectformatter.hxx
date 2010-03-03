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

#ifndef OOX_DRAWINGML_CHART_OBJECTFORMATTER_HXX
#define OOX_DRAWINGML_CHART_OBJECTFORMATTER_HXX

#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/chart/modelbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XChartDocument; }
} } }

namespace oox { namespace core { class XmlFilterBase; } }

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
    OBJECTTYPE_MAJORGRIDLINE,           /// Axis major grid line.
    OBJECTTYPE_MINORGRIDLINE,           /// Axis minor grid line.
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

struct ChartSpaceModel;
struct ObjectFormatterData;
struct PictureOptionsModel;

class ObjectFormatter
{
public:
    explicit            ObjectFormatter(
                            const ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument >& rxChartDoc,
                            const ChartSpaceModel& rChartSpace );
                        ~ObjectFormatter();

    /** Sets the maximum series index used for color cycling/fading. */
    void                setMaxSeriesIndex( sal_Int32 nMaxSeriesIdx );
    /** Returns the current maximum series index used for color cycling/fading. */
    sal_Int32           getMaxSeriesIndex() const;

    /** Sets frame formatting properties to the passed property set. */
    void                convertFrameFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< Shape >& rxShapeProp,
                            ObjectType eObjType,
                            sal_Int32 nSeriesIdx = -1 );

    /** Sets frame formatting properties to the passed property set. */
    void                convertFrameFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< Shape >& rxShapeProp,
                            const PictureOptionsModel& rPicOptions,
                            ObjectType eObjType,
                            sal_Int32 nSeriesIdx = -1 );

    /** Sets text formatting properties to the passed property set. */
    void                convertTextFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< TextBody >& rxTextProp,
                            ObjectType eObjType );

    /** Sets frame/text formatting properties to the passed property set. */
    void                convertFormatting(
                            PropertySet& rPropSet,
                            const ModelRef< Shape >& rxShapeProp,
                            const ModelRef< TextBody >& rxTextProp,
                            ObjectType eObjType );

    /** Sets text formatting properties to the passed property set. */
    void                convertTextFormatting(
                            PropertySet& rPropSet,
                            const TextCharacterProperties& rTextProps,
                            ObjectType eObjType );

    /** Sets text rotation properties to the passed property set. */
    void                convertTextRotation(
                            PropertySet& rPropSet,
                            const ModelRef< TextBody >& rxTextProp,
                            bool bSupportsStacked );

    /** Sets number format properties to the passed property set. */
    void                convertNumberFormat(
                            PropertySet& rPropSet,
                            const NumberFormat& rNumberFormat,
                            bool bPercentFormat = false );

    /** Sets automatic line properties to the passed property set. */
    void                convertAutomaticLine(
                            PropertySet& rPropSet,
                            ObjectType eObjType,
                            sal_Int32 nSeriesIdx = -1 );

    /** Sets automatic fill properties to the passed property set. */
    void                convertAutomaticFill(
                            PropertySet& rPropSet,
                            ObjectType eObjType,
                            sal_Int32 nSeriesIdx = -1 );

    /** Returns true, if the passed shape properties have automatic line mode. */
    static bool         isAutomaticLine( const ModelRef< Shape >& rxShapeProp );
    /** Returns true, if the passed shape properties have automatic fill mode. */
    static bool         isAutomaticFill( const ModelRef< Shape >& rxShapeProp );

private:
    ::boost::shared_ptr< ObjectFormatterData > mxData;
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

