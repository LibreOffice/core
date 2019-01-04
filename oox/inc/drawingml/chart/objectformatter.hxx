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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_OBJECTFORMATTER_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_OBJECTFORMATTER_HXX

#include <oox/helper/propertyset.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/chart/modelbase.hxx>

namespace com { namespace sun { namespace star {
    namespace chart2 { class XChartDocument; }
} } }

namespace oox { namespace core { class XmlFilterBase; } }

namespace oox {
namespace drawingml {
namespace chart {

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

struct ChartSpaceModel;
struct ObjectFormatterData;
struct PictureOptionsModel;

class ObjectFormatter
{
public:
    explicit            ObjectFormatter(
                            const ::oox::core::XmlFilterBase& rFilter,
                            const css::uno::Reference< css::chart2::XChartDocument >& rxChartDoc,
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
    static void         convertTextRotation(
                            PropertySet& rPropSet,
                            const ModelRef< TextBody >& rxTextProp,
                            bool bSupportsStacked, sal_Int32 nDefaultRotation  = 0);

    /** Sets text wrap properties to the passed property set. */
    static void         convertTextWrap(
                            PropertySet& rPropSet,
                            const ModelRef< TextBody >& rxTextProp);

    /** Sets number format properties to the passed property set. */
    void                convertNumberFormat(
                            PropertySet& rPropSet,
                            const NumberFormat& rNumberFormat,
                            bool bAxis,
                            bool bShowPercent = false );

    /** Sets automatic fill properties to the passed property set. */
    void                convertAutomaticFill(
                            PropertySet& rPropSet,
                            ObjectType eObjType,
                            sal_Int32 nSeriesIdx );

    /** Returns true, if the passed shape properties have automatic fill mode. */
    static bool         isAutomaticFill( const ModelRef< Shape >& rxShapeProp );

    /** Returns true, if the X Axis label rotation is 0 degree. */
    static bool         getTextRotation(
                            const ModelRef< TextBody >& rxTextProp,
                            sal_Int32 nDefaultRotation = 0 );

private:
    std::shared_ptr< ObjectFormatterData > mxData;
};

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
