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

#ifndef INCLUDED_OOX_DRAWINGML_CHART_TYPEGROUPCONVERTER_HXX
#define INCLUDED_OOX_DRAWINGML_CHART_TYPEGROUPCONVERTER_HXX

#include <drawingml/chart/converterbase.hxx>
#include <com/sun/star/chart2/PieChartSubType.hpp>

namespace com::sun::star {
    namespace chart2 { class XChartType; }
    namespace chart2 { class XCoordinateSystem; }
    namespace chart2 { class XDataSeries; }
    namespace chart2 { class XDiagram; }
    namespace chart2::data { class XLabeledDataSequence; }
}

namespace oox::drawingml::chart {

/** Enumerates different chart types. */
enum TypeId
{
    TYPEID_BAR,                     /// Vertical bar chart.
    TYPEID_HORBAR,                  /// Horizontal bar chart.
    TYPEID_LINE,                    /// Line chart.
    TYPEID_AREA,                    /// Area chart.
    TYPEID_STOCK,                   /// Stock chart.
    TYPEID_RADARLINE,               /// Linear radar chart.
    TYPEID_RADARAREA,               /// Filled radar chart.
    TYPEID_PIE,                     /// Pie chart.
    TYPEID_DOUGHNUT,                /// Doughnut (ring) chart.
    TYPEID_OFPIE,                   /// Pie-to-pie or pie-to-bar chart.
    TYPEID_SCATTER,                 /// Scatter (XY) chart.
    TYPEID_BUBBLE,                  /// Bubble chart.
    TYPEID_SURFACE,                 /// Surface chart.
    TYPEID_UNKNOWN                  /// Default for unknown chart types.
};

/** Enumerates different categories of similar chart types. */
enum TypeCategory
{
    TYPECATEGORY_BAR,               /// Bar charts (horizontal or vertical).
    TYPECATEGORY_LINE,              /// Line charts (line, area, stock charts).
    TYPECATEGORY_RADAR,             /// Radar charts (linear or filled).
    TYPECATEGORY_PIE,               /// Pie and donut charts.
    TYPECATEGORY_SCATTER,           /// Scatter and bubble charts.
    TYPECATEGORY_SURFACE            /// Surface charts.
};

/** Enumerates modes for varying point colors in a series. */
enum VarPointMode
{
    VARPOINTMODE_NONE,              /// No varied colors supported.
    VARPOINTMODE_SINGLE,            /// Only supported, if type group contains only one series.
    VARPOINTMODE_MULTI              /// Supported for multiple series in a chart type group.
};

/** Contains info for a chart type related to the OpenOffice.org chart module. */
struct TypeGroupInfo
{
    TypeId              meTypeId;               /// Unique chart type identifier.
    TypeCategory        meTypeCategory;         /// Category this chart type belongs to.
    const char*         mpcServiceName;         /// Service name of the type.
    VarPointMode        meVarPointMode;         /// Mode for varying point colors.
    sal_Int32           mnDefLabelPos;          /// Default data label position (API constant).
    bool                mbPolarCoordSystem;     /// True = polar, false = cartesian.
    bool                mbSeriesIsFrame2d;      /// True = 2D type series with area formatting.
    bool                mbSingleSeriesVis;      /// True = only first series visible (e.g. pie charts).
    bool                mbCategoryAxis;         /// True = X axis contains categories.
    bool                mbSwappedAxesSet;       /// True = X axis and Y axis are swapped.
    bool                mbSupportsStacking;     /// True = data points can be stacked on each other.
    bool                mbPictureOptions;       /// True = bitmaps support options from c:pictureOptions.
};

const TypeGroupInfo& GetTypeGroupInfo( TypeId eType );

struct UpDownBarsModel;

class UpDownBarsConverter final : public ConverterBase< UpDownBarsModel >
{
public:
    explicit            UpDownBarsConverter( const ConverterRoot& rParent, UpDownBarsModel& rModel );
    virtual             ~UpDownBarsConverter() override;

    /** Converts the OOXML up/down bars. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XChartType >& rxChartType );
};

struct TypeGroupModel;
struct View3DModel;

class TypeGroupConverter final : public ConverterBase< TypeGroupModel >
{
public:
    explicit            TypeGroupConverter( const ConverterRoot& rParent, TypeGroupModel& rModel );
    virtual             ~TypeGroupConverter() override;

    /** Returns the type info struct that describes this chart type group. */
    const TypeGroupInfo& getTypeInfo() const { return maTypeInfo; }

    /** Returns true, if the series in this chart type group are stacked on each other (no percentage). */
    bool                isStacked() const;
    /** Returns true, if the series in this chart type group are stacked on each other as percentage. */
    bool                isPercent() const;
    /** Returns true, if the chart is three-dimensional. */
    bool                is3dChart() const { return mb3dChart;}
    /** Returns true, if chart type supports wall and floor format in 3D mode. */
    bool                isWall3dChart() const;
    /** Returns true, if the series in this chart type group are ordered on the Z axis. */
    bool                isDeep3dChart() const;

    /** Returns true, if this chart type supports area formatting for its series. */
    bool                isSeriesFrameFormat() const;
    /** Returns the object type for a series depending on the chart type. */
    ObjectType          getSeriesObjectType() const;

    /** Returns series title, if the chart type group contains only one single series. */
    OUString            getSingleSeriesTitle() const;

    /** Returns true, if the chart contains only one series and have title textbox (even empty). */
    bool                isSingleSeriesTitle() const;

    /** Creates a coordinate system according to the contained chart type. */
    css::uno::Reference< css::chart2::XCoordinateSystem >
                        createCoordinateSystem();
    /** Creates a labeled data sequence object for axis categories. */
    css::uno::Reference< css::chart2::data::XLabeledDataSequence >
                        createCategorySequence();

    /** Converts the OOXML type group model into a chart2 coordinate system. */
    void                convertFromModel(
                            const css::uno::Reference< css::chart2::XDiagram >& rxDiagram,
                            const css::uno::Reference< css::chart2::XCoordinateSystem >& rxCoordSystem,
                            sal_Int32 nAxesSetIdx, bool bSupportsVaryColorsByPoint );

    /** Sets the passed OOXML marker style at the passed property set. */
    void                convertMarker( PropertySet& rPropSet, sal_Int32 nOoxSymbol, sal_Int32 nOoxSize,
                                       const ModelRef< Shape >& xShapeProps ) const;
    /** Sets the passed OOXML line smoothing at the passed property set. */
    void                convertLineSmooth( PropertySet& rPropSet, bool bOoxSmooth ) const;
    /** Sets the passed OOXML bar 3D geometry at the passed property set. */
    void                convertBarGeometry( PropertySet& rPropSet, sal_Int32 nOoxShape ) const;
    /** Sets the passed OOXML pie rotation at the passed property set. */
    void                convertPieRotation( PropertySet& rPropSet, sal_Int32 nOoxAngle ) const;
    /** Sets the passed OOXML pie explosion at the passed property set. */
    void                convertPieExplosion( PropertySet& rPropSet, sal_Int32 nOoxExplosion ) const;
    /** Converts of-pie types */
    css::chart2::PieChartSubType convertOfPieType(sal_Int32 nOoxOfPieType ) const;

private:
    /** Inserts the passed series into the chart type. Adds additional properties to the series. */
    void                insertDataSeries(
                            const css::uno::Reference< css::chart2::XChartType >& rxChartType,
                            const css::uno::Reference< css::chart2::XDataSeries >& rxSeries,
                            sal_Int32 nAxesSetIdx );

private:
    TypeGroupInfo       maTypeInfo;         /// Extended type info for contained chart type model.
    bool                mb3dChart;          /// True = type is a 3D chart type.
};

} // namespace oox::drawingml::chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
