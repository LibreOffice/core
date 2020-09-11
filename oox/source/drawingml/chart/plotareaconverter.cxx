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

#include <drawingml/chart/plotareaconverter.hxx>

#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart/XDiagramPositioning.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <osl/diagnose.h>
#include <drawingml/chart/axisconverter.hxx>
#include <drawingml/chart/plotareamodel.hxx>
#include <drawingml/chart/typegroupconverter.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <tools/helpers.hxx>

namespace oox::drawingml::chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::uno;

namespace {

/** Axes set model. This is a helper for the plot area converter collecting all
    type groups and axes of the primary or secondary axes set. */
struct AxesSetModel
{
    typedef ModelVector< TypeGroupModel >       TypeGroupVector;
    typedef ModelMap< sal_Int32, AxisModel >    AxisMap;

    TypeGroupVector     maTypeGroups;       /// All type groups containing data series.
    AxisMap             maAxes;             /// All axes mapped by API axis type.

    explicit     AxesSetModel() {}
};

/** Axes set converter. This is a helper class for the plot area converter. */
class AxesSetConverter : public ConverterBase< AxesSetModel >
{
public:
    explicit            AxesSetConverter( const ConverterRoot& rParent, AxesSetModel& rModel );

    /** Converts the axes set model to a chart2 diagram. Returns an automatic
        chart title from a single series title, if possible. */
    void                convertFromModel(
                            const Reference< XDiagram >& rxDiagram,
                            View3DModel& rView3DModel,
                            sal_Int32 nAxesSetIdx,
                            bool bSupportsVaryColorsByPoint );

    /** Returns the automatic chart title if the axes set contains only one series. */
    const OUString& getAutomaticTitle() const { return maAutoTitle; }
    /** Returns true, if the chart is three-dimensional. */
    bool         is3dChart() const { return mb3dChart; }
    /** Returns true, if chart type supports wall and floor format in 3D mode. */
    bool         isWall3dChart() const { return mbWall3dChart; }
    /** Returns true, if chart is a pie chart or doughnut chart. */
    bool         isPieChart() const { return mbPieChart; }

private:
    OUString     maAutoTitle;
    bool                mb3dChart;
    bool                mbWall3dChart;
    bool                mbPieChart;
};

AxesSetConverter::AxesSetConverter( const ConverterRoot& rParent, AxesSetModel& rModel ) :
    ConverterBase< AxesSetModel >( rParent, rModel ),
    mb3dChart( false ),
    mbWall3dChart( false ),
    mbPieChart( false )
{
}

ModelRef< AxisModel > lclGetOrCreateAxis( const AxesSetModel::AxisMap& rFromAxes, sal_Int32 nAxisIdx, sal_Int32 nDefTypeId, bool bMSO2007Doc )
{
    ModelRef< AxisModel > xAxis = rFromAxes.get( nAxisIdx );
    if( !xAxis )
        xAxis.create( nDefTypeId, bMSO2007Doc ).mbDeleted = true;  // missing axis is invisible
    return xAxis;
}

void AxesSetConverter::convertFromModel( const Reference< XDiagram >& rxDiagram,
        View3DModel& rView3DModel, sal_Int32 nAxesSetIdx, bool bSupportsVaryColorsByPoint )
{
    // create type group converter objects for all type groups
    typedef RefVector< TypeGroupConverter > TypeGroupConvVector;
    TypeGroupConvVector aTypeGroups;
    for (auto const& typeGroup : mrModel.maTypeGroups)
        aTypeGroups.push_back( std::make_shared<TypeGroupConverter>( *this, *typeGroup ) );

    OSL_ENSURE( !aTypeGroups.empty(), "AxesSetConverter::convertFromModel - no type groups in axes set" );
    if( aTypeGroups.empty() )
        return;

    try
    {
        // first type group needed for coordinate system and axis conversion
        TypeGroupConverter& rFirstTypeGroup = *aTypeGroups.front();

        // get automatic chart title, if there is only one type group
        if( aTypeGroups.size() == 1 )
            maAutoTitle = rFirstTypeGroup.getSingleSeriesTitle();

        /*  Create a coordinate system. For now, all type groups from all axes sets
            have to be inserted into one coordinate system. Later, chart2 should
            support using one coordinate system for each axes set. */
        Reference< XCoordinateSystem > xCoordSystem;
        Reference< XCoordinateSystemContainer > xCoordSystemCont( rxDiagram, UNO_QUERY_THROW );
        Sequence< Reference< XCoordinateSystem > > aCoordSystems = xCoordSystemCont->getCoordinateSystems();
        if( aCoordSystems.hasElements() )
        {
            OSL_ENSURE( aCoordSystems.getLength() == 1, "AxesSetConverter::convertFromModel - too many coordinate systems" );
            xCoordSystem = aCoordSystems[ 0 ];
            OSL_ENSURE( xCoordSystem.is(), "AxesSetConverter::convertFromModel - invalid coordinate system" );
        }
        else
        {
            xCoordSystem = rFirstTypeGroup.createCoordinateSystem();
            if( xCoordSystem.is() )
                xCoordSystemCont->addCoordinateSystem( xCoordSystem );
        }

        // 3D view settings
        mb3dChart = rFirstTypeGroup.is3dChart();
        mbWall3dChart = rFirstTypeGroup.isWall3dChart();
        mbPieChart = rFirstTypeGroup.getTypeInfo().meTypeCategory == TYPECATEGORY_PIE;
        if( mb3dChart )
        {
            View3DConverter aView3DConv( *this, rView3DModel );
            aView3DConv.convertFromModel( rxDiagram, rFirstTypeGroup );
        }

        /*  Convert all chart type groups. Each type group will add its series
            to the data provider attached to the chart document. */
        if( xCoordSystem.is() )
        {
            bool bMSO2007Doc = getFilter().isMSO2007Document();
            // convert all axes (create missing axis models)
            ModelRef< AxisModel > xXAxis = lclGetOrCreateAxis( mrModel.maAxes, API_X_AXIS, rFirstTypeGroup.getTypeInfo().mbCategoryAxis ? C_TOKEN( catAx ) : C_TOKEN( valAx ), bMSO2007Doc );
            ModelRef< AxisModel > xYAxis = lclGetOrCreateAxis( mrModel.maAxes, API_Y_AXIS, C_TOKEN( valAx ), bMSO2007Doc );

            AxisConverter aXAxisConv( *this, *xXAxis );
            aXAxisConv.convertFromModel( xCoordSystem, aTypeGroups, xYAxis.get(), nAxesSetIdx, API_X_AXIS );
            AxisConverter aYAxisConv( *this, *xYAxis );
            aYAxisConv.convertFromModel( xCoordSystem, aTypeGroups, xXAxis.get(), nAxesSetIdx, API_Y_AXIS );

            if( rFirstTypeGroup.isDeep3dChart() )
            {
                ModelRef< AxisModel > xZAxis = lclGetOrCreateAxis( mrModel.maAxes, API_Z_AXIS, C_TOKEN( serAx ), bMSO2007Doc );
                AxisConverter aZAxisConv( *this, *xZAxis );
                aZAxisConv.convertFromModel( xCoordSystem, aTypeGroups, nullptr, nAxesSetIdx, API_Z_AXIS );
            }

            // convert all chart type groups, this converts all series data and formatting
            for (auto const& typeGroup : aTypeGroups)
                typeGroup->convertFromModel( rxDiagram, xCoordSystem, nAxesSetIdx, bSupportsVaryColorsByPoint );
        }
    }
    catch( Exception& )
    {
    }
}

} // namespace

View3DConverter::View3DConverter( const ConverterRoot& rParent, View3DModel& rModel ) :
    ConverterBase< View3DModel >( rParent, rModel )
{
}

View3DConverter::~View3DConverter()
{
}

void View3DConverter::convertFromModel( const Reference< XDiagram >& rxDiagram, TypeGroupConverter const & rTypeGroup )
{
    namespace cssd = ::com::sun::star::drawing;
    PropertySet aPropSet( rxDiagram );

    sal_Int32 nRotationY = 0;
    sal_Int32 nRotationX = 0;
    bool bRightAngled = false;
    sal_Int32 nAmbientColor = 0;
    sal_Int32 nLightColor = 0;

    if( rTypeGroup.getTypeInfo().meTypeCategory == TYPECATEGORY_PIE )
    {
        // Y rotation used as 'first pie slice angle' in 3D pie charts
        rTypeGroup.convertPieRotation( aPropSet, mrModel.monRotationY.get( 0 ) );
        // X rotation a.k.a. elevation (map OOXML [0..90] to Chart2 [-90,0])
        nRotationX = getLimitedValue< sal_Int32, sal_Int32 >( mrModel.monRotationX.get( 15 ), 0, 90 ) - 90;
        // no right-angled axes in pie charts
        bRightAngled = false;
        // ambient color (Gray 30%)
        nAmbientColor = 0xB3B3B3;
        // light color (Gray 70%)
        nLightColor = 0x4C4C4C;
    }
    else // 3D bar/area/line charts
    {
        // Y rotation (OOXML [0..359], Chart2 [-179,180])
        nRotationY = mrModel.monRotationY.get( 20 );
        // X rotation a.k.a. elevation (OOXML [-90..90], Chart2 [-179,180])
        nRotationX = getLimitedValue< sal_Int32, sal_Int32 >( mrModel.monRotationX.get( 15 ), -90, 90 );
        // right-angled axes
        bRightAngled = mrModel.mbRightAngled;
        // ambient color (Gray 20%)
        nAmbientColor = 0xCCCCCC;
        // light color (Gray 60%)
        nLightColor = 0x666666;
    }

    // Y rotation (map OOXML [0..359] to Chart2 [-179,180])
    nRotationY = NormAngle180(nRotationY);
    /*  Perspective (map OOXML [0..200] to Chart2 [0,100]). Seems that MSO 2007 is
        buggy here, the XML plugin of MSO 2003 writes the correct perspective in
        the range from 0 to 100. We will emulate the wrong behaviour of MSO 2007. */
    sal_Int32 nPerspective = getLimitedValue< sal_Int32, sal_Int32 >( mrModel.mnPerspective / 2, 0, 100 );
    // projection mode (parallel axes, if right-angled, #i90360# or if perspective is at 0%)
    bool bParallel = bRightAngled || (nPerspective == 0);
    cssd::ProjectionMode eProjMode = bParallel ? cssd::ProjectionMode_PARALLEL : cssd::ProjectionMode_PERSPECTIVE;

    // set rotation properties
    aPropSet.setProperty( PROP_RightAngledAxes, bRightAngled );
    aPropSet.setProperty( PROP_RotationVertical, nRotationY );
    aPropSet.setProperty( PROP_RotationHorizontal, nRotationX );
    aPropSet.setProperty( PROP_Perspective, nPerspective );
    aPropSet.setProperty( PROP_D3DScenePerspective, eProjMode );

    // set light settings
    aPropSet.setProperty( PROP_D3DSceneShadeMode, cssd::ShadeMode_FLAT );
    aPropSet.setProperty( PROP_D3DSceneAmbientColor, nAmbientColor );
    aPropSet.setProperty( PROP_D3DSceneLightOn1, false );
    aPropSet.setProperty( PROP_D3DSceneLightOn2, true );
    aPropSet.setProperty( PROP_D3DSceneLightColor2, nLightColor );
    aPropSet.setProperty( PROP_D3DSceneLightDirection2, cssd::Direction3D( 0.2, 0.4, 1.0 ) );
}

WallFloorConverter::WallFloorConverter( const ConverterRoot& rParent, WallFloorModel& rModel ) :
    ConverterBase< WallFloorModel >( rParent, rModel )
{
}

WallFloorConverter::~WallFloorConverter()
{
}

void WallFloorConverter::convertFromModel( const Reference< XDiagram >& rxDiagram, ObjectType eObjType )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( rxDiagram.is() )
    {
        PropertySet aPropSet;
        switch( eObjType )
        {
            case OBJECTTYPE_FLOOR:  aPropSet.set( rxDiagram->getFloor() );  break;
            case OBJECTTYPE_WALL:   aPropSet.set( rxDiagram->getWall() );   break;
            default:                OSL_FAIL( "WallFloorConverter::convertFromModel - invalid object type" );
        }
        if( aPropSet.is() )
            getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(bMSO2007Doc), eObjType );
    }
}

DataTableConverter::DataTableConverter( const ConverterRoot& rParent, DataTableModel& rModel ) :
        ConverterBase< DataTableModel >( rParent, rModel )
{
}

DataTableConverter::~DataTableConverter()
{
}

void DataTableConverter::convertFromModel( const Reference< XDiagram >& rxDiagram )
{
    PropertySet aPropSet( rxDiagram );
    if (mrModel.mbShowHBorder)
        aPropSet.setProperty( PROP_DataTableHBorder, mrModel.mbShowHBorder );
    if (mrModel.mbShowVBorder)
        aPropSet.setProperty( PROP_DataTableVBorder, mrModel.mbShowVBorder);
    if (mrModel.mbShowOutline)
        aPropSet.setProperty( PROP_DataTableOutline, mrModel.mbShowOutline );
}

PlotAreaConverter::PlotAreaConverter( const ConverterRoot& rParent, PlotAreaModel& rModel ) :
    ConverterBase< PlotAreaModel >( rParent, rModel ),
    mb3dChart( false ),
    mbWall3dChart( false ),
    mbPieChart( false )
{
}

PlotAreaConverter::~PlotAreaConverter()
{
}

void PlotAreaConverter::convertFromModel( View3DModel& rView3DModel )
{
    /*  Create the diagram object and attach it to the chart document. One
        diagram is used to carry all coordinate systems and data series. */
    Reference< XDiagram > xDiagram;
    try
    {
        xDiagram.set( createInstance( "com.sun.star.chart2.Diagram" ), UNO_QUERY_THROW );
        getChartDocument()->setFirstDiagram( xDiagram );
    }
    catch( Exception& )
    {
    }

    // store all axis models in a map, keyed by axis identifier
    typedef ModelMap< sal_Int32, AxisModel > AxisMap;
    AxisMap aAxisMap;
    std::vector<sal_Int32>rValAxisIds;
    std::vector<sal_Int32>rRealValAxisIds;

    for (auto const& atypeGroup : mrModel.maTypeGroups)
    {
        if (atypeGroup->maAxisIds.size() > 1)
        {
            // let's collect which axId belongs to the Y Axis according to maTypeGroups
            rRealValAxisIds.push_back(atypeGroup->maAxisIds[1]);
        }
    }

    for (auto const& axis : mrModel.maAxes)
    {
        OSL_ENSURE( axis->mnAxisId >= 0, "PlotAreaConverter::convertFromModel - invalid axis identifier" );
        OSL_ENSURE( !aAxisMap.has( axis->mnAxisId ), "PlotAreaConverter::convertFromModel - axis identifiers not unique" );
        if( axis->mnAxisId != -1 )
            aAxisMap[ axis->mnAxisId ] = axis;

        if ( axis->mnAxisId != -1 && axis->mnTypeId == C_TOKEN(valAx) )
        {
            for (size_t i = 0; i < rRealValAxisIds.size(); i++)
            {
                if (axis->mnAxisId == rRealValAxisIds[i])
                {
                    // let's collect which axId belongs to the Y Axis according to maAxes
                    rValAxisIds.push_back(axis->mnAxisId);
                }
            }
        }
    }

    // group the type group models into different axes sets
    typedef ModelVector< AxesSetModel > AxesSetVector;
    AxesSetVector aAxesSets;
    sal_Int32 nMaxSeriesIdx = -1;
    for (auto const& typeGroup : mrModel.maTypeGroups)
    {
        if( !typeGroup->maSeries.empty() )
        {
            // try to find a compatible axes set for the type group
            AxesSetModel* pAxesSet = nullptr;
            for (auto const& axesSet : aAxesSets)
            {
                if( axesSet->maTypeGroups.front()->maAxisIds == typeGroup->maAxisIds )
                {
                    pAxesSet = axesSet.get();
                    if (pAxesSet)
                        break;
                }
            }

            // not possible to insert into an existing axes set -> start a new axes set
            if( !pAxesSet )
            {
                pAxesSet = &aAxesSets.create();
                // find axis models used by the type group
                const std::vector<sal_Int32>& rAxisIds = typeGroup->maAxisIds;
                if( !rAxisIds.empty() )
                    pAxesSet->maAxes[ API_X_AXIS ] = aAxisMap.get( rAxisIds[ 0 ] );
                if( rAxisIds.size() >= 2 )
                    pAxesSet->maAxes[ API_Y_AXIS ] = aAxisMap.get( rAxisIds[ 1 ] );
                if( rAxisIds.size() >= 3 )
                    pAxesSet->maAxes[ API_Z_AXIS ] = aAxisMap.get( rAxisIds[ 2 ] );
            }

            // insert the type group model
            pAxesSet->maTypeGroups.push_back( typeGroup );

            // collect the maximum series index for automatic series formatting
            for (auto const& elemSeries : typeGroup->maSeries)
                nMaxSeriesIdx = ::std::max( nMaxSeriesIdx, elemSeries->mnIndex );
        }
    }
    getFormatter().setMaxSeriesIndex( nMaxSeriesIdx );

    // varying point colors only for single series in single chart type
    bool bSupportsVaryColorsByPoint = mrModel.maTypeGroups.size() == 1;

    // convert all axes sets, and check which axis is attached to the first maTypeGroups
    sal_Int32 nStartAxesSetIdx = (rValAxisIds.size() > 1 && aAxesSets.size() > 0 && aAxesSets[0]->maAxes.count( API_Y_AXIS )
            && aAxesSets[0]->maAxes[ API_Y_AXIS ]->mnAxisId != rValAxisIds[0] ) ? 1 : 0;
    sal_Int32 nAxesSetIdx = nStartAxesSetIdx;

    for (auto const& axesSet : aAxesSets)
    {
        AxesSetConverter aAxesSetConv(*this, *axesSet);
        aAxesSetConv.convertFromModel( xDiagram, rView3DModel, nAxesSetIdx, bSupportsVaryColorsByPoint );
        if(nAxesSetIdx == nStartAxesSetIdx)
        {
            maAutoTitle = aAxesSetConv.getAutomaticTitle();
            mb3dChart = aAxesSetConv.is3dChart();
            mbWall3dChart = aAxesSetConv.isWall3dChart();
            mbPieChart = aAxesSetConv.isPieChart();
        }
        else
        {
            maAutoTitle.clear();
        }
        nAxesSetIdx = 1 - nAxesSetIdx;
    }

    DataTableConverter dataTableConverter (*this, mrModel.mxDataTable.getOrCreate());
    dataTableConverter.convertFromModel(xDiagram);
    // plot area formatting
    if( xDiagram.is() && !mb3dChart )
    {
        PropertySet aPropSet( xDiagram->getWall() );
        getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, OBJECTTYPE_PLOTAREA2D );
    }
}

void PlotAreaConverter::convertPositionFromModel()
{
    LayoutModel& rLayout = mrModel.mxLayout.getOrCreate();
    LayoutConverter aLayoutConv( *this, rLayout );
    awt::Rectangle aDiagramRect;
    if( !aLayoutConv.calcAbsRectangle( aDiagramRect ) )
        return;

    try
    {
        namespace cssc = ::com::sun::star::chart;
        Reference< cssc::XChartDocument > xChart1Doc( getChartDocument(), UNO_QUERY_THROW );
        Reference< cssc::XDiagramPositioning > xPositioning( xChart1Doc->getDiagram(), UNO_QUERY_THROW );
        // for pie charts, always set inner plot area size to exclude the data labels as Excel does
        sal_Int32 nTarget = (mbPieChart && (rLayout.mnTarget == XML_outer)) ? XML_inner : rLayout.mnTarget;
        switch( nTarget )
        {
            case XML_inner:
                xPositioning->setDiagramPositionExcludingAxes( aDiagramRect );
            break;
            case XML_outer:
                xPositioning->setDiagramPositionIncludingAxes( aDiagramRect );
            break;
            default:
                OSL_FAIL( "PlotAreaConverter::convertPositionFromModel - unknown positioning target" );
        }
    }
    catch( Exception& )
    {
    }
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
