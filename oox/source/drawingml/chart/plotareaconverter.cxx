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

#include "oox/drawingml/chart/plotareaconverter.hxx"
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include "oox/drawingml/chart/axisconverter.hxx"
#include "oox/drawingml/chart/plotareamodel.hxx"
#include "oox/drawingml/chart/typegroupconverter.hxx"
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::chart2::XCoordinateSystem;
using ::com::sun::star::chart2::XCoordinateSystemContainer;
using ::com::sun::star::chart2::XDiagram;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

/** Axes set model. This is a helper class for the plot area converter. */
struct AxesSetModel
{
    typedef ModelVector< TypeGroupModel >       TypeGroupVector;
    typedef ModelMap< sal_Int32, AxisModel >    AxisMap;

    TypeGroupVector     maTypeGroups;
    AxisMap             maAxes;

    inline explicit     AxesSetModel() {}
    inline              ~AxesSetModel() {}
};

// ============================================================================

/** Axes set converter. This is a helper class for the plot area converter. */
class AxesSetConverter : public ConverterBase< AxesSetModel >
{
public:
    explicit            AxesSetConverter( const ConverterRoot& rParent, AxesSetModel& rModel );
    virtual             ~AxesSetConverter();

    /** Converts the axes set model to a chart2 diagram. Returns an automatic
        chart title from a single series title, if possible. */
    void                convertFromModel(
                            const Reference< XDiagram >& rxDiagram,
                            View3DModel& rView3DModel,
                            sal_Int32 nAxesSetIdx,
                            bool bSupportsVaryColorsByPoint );

    /** Returns the automatic chart title if the axes set contains only one series. */
    inline const ::rtl::OUString& getAutomaticTitle() const { return maAutoTitle; }
    /** Returns true, if the chart is three-dimensional. */
    inline bool         is3dChart() const { return mb3dChart; }
    /** Returns true, if chart type supports wall and floor format in 3D mode. */
    inline bool         isWall3dChart() const { return mbWall3dChart; }

private:
    ::rtl::OUString     maAutoTitle;
    bool                mb3dChart;
    bool                mbWall3dChart;
};

// ----------------------------------------------------------------------------

AxesSetConverter::AxesSetConverter( const ConverterRoot& rParent, AxesSetModel& rModel ) :
    ConverterBase< AxesSetModel >( rParent, rModel ),
    mb3dChart( false ),
    mbWall3dChart( false )
{
}

AxesSetConverter::~AxesSetConverter()
{
}

ModelRef< AxisModel > lclGetOrCreateAxis( const AxesSetModel::AxisMap& rFromAxes, sal_Int32 nAxisIdx, sal_Int32 nDefTypeId )
{
    ModelRef< AxisModel > xAxis = rFromAxes.get( nAxisIdx );
    if( !xAxis )
        xAxis.create( nDefTypeId ).mbDeleted = true;  // missing axis is invisible
    return xAxis;
}

void AxesSetConverter::convertFromModel( const Reference< XDiagram >& rxDiagram,
        View3DModel& rView3DModel, sal_Int32 nAxesSetIdx, bool bSupportsVaryColorsByPoint )
{
    // create type group converter objects for all type groups
    typedef RefVector< TypeGroupConverter > TypeGroupConvVector;
    TypeGroupConvVector aTypeGroups;
    for( AxesSetModel::TypeGroupVector::iterator aIt = mrModel.maTypeGroups.begin(), aEnd = mrModel.maTypeGroups.end(); aIt != aEnd; ++aIt )
        aTypeGroups.push_back( TypeGroupConvVector::value_type( new TypeGroupConverter( *this, **aIt ) ) );

    OSL_ENSURE( !aTypeGroups.empty(), "AxesSetConverter::convertFromModel - no type groups in axes set" );
    if( !aTypeGroups.empty() ) try
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
        if( mb3dChart )
        {
            View3DConverter aView3DConv( *this, rView3DModel );
            aView3DConv.convertFromModel( rxDiagram, rFirstTypeGroup );
        }

        /*  Convert all chart type groups. Each type group will add its series
            to the data provider attached to the chart document. */
        if( xCoordSystem.is() )
        {
            // convert all axes (create missing axis models)
            ModelRef< AxisModel > xXAxis = lclGetOrCreateAxis( mrModel.maAxes, API_X_AXIS, rFirstTypeGroup.getTypeInfo().mbCategoryAxis ? C_TOKEN( catAx ) : C_TOKEN( valAx ) );
            ModelRef< AxisModel > xYAxis = lclGetOrCreateAxis( mrModel.maAxes, API_Y_AXIS, C_TOKEN( valAx ) );

            AxisConverter aXAxisConv( *this, *xXAxis );
            aXAxisConv.convertFromModel( xCoordSystem, rFirstTypeGroup, xYAxis.get(), nAxesSetIdx, API_X_AXIS );
            AxisConverter aYAxisConv( *this, *xYAxis );
            aYAxisConv.convertFromModel( xCoordSystem, rFirstTypeGroup, xXAxis.get(), nAxesSetIdx, API_Y_AXIS );

            if( rFirstTypeGroup.isDeep3dChart() )
            {
                ModelRef< AxisModel > xZAxis = lclGetOrCreateAxis( mrModel.maAxes, API_Z_AXIS, C_TOKEN( serAx ) );
                AxisConverter aZAxisConv( *this, *xZAxis );
                aZAxisConv.convertFromModel( xCoordSystem, rFirstTypeGroup, 0, nAxesSetIdx, API_Z_AXIS );
            }

            // convert all chart type groups, this converts all series data and formatting
            for( TypeGroupConvVector::iterator aTIt = aTypeGroups.begin(), aTEnd = aTypeGroups.end(); aTIt != aTEnd; ++aTIt )
                (*aTIt)->convertFromModel( rxDiagram, xCoordSystem, nAxesSetIdx, bSupportsVaryColorsByPoint );
        }
    }
    catch( Exception& )
    {
    }
}

} // namespace

// ============================================================================

View3DConverter::View3DConverter( const ConverterRoot& rParent, View3DModel& rModel ) :
    ConverterBase< View3DModel >( rParent, rModel )
{
}

View3DConverter::~View3DConverter()
{
}

void View3DConverter::convertFromModel( const Reference< XDiagram >& rxDiagram, TypeGroupConverter& rTypeGroup )
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
    nRotationY %= 360;
    if( nRotationY > 180 ) nRotationY -= 360;
    /*  Perspective (map OOXML [0..200] to Chart2 [0,100]). Seems that MSO 2007 is
        buggy here, the XML plugin of MSO 2003 writes the correct perspective in
        the range from 0 to 100. We will emulate the wrong behaviour of MSO 2007. */
    sal_Int32 nPerspective = getLimitedValue< sal_Int32, sal_Int32 >( mrModel.mnPerspective / 2, 0, 100 );
    // projection mode (parallel axes, if right-angled, #i90360# or if perspective is at 0%)
    bool bParallel = bRightAngled || (nPerspective == 0);
    cssd::ProjectionMode eProjMode = bParallel ? cssd::ProjectionMode_PARALLEL : cssd::ProjectionMode_PERSPECTIVE;

    // set rotation properties
    aPropSet.setProperty( PROP_RotationVertical, nRotationY );
    aPropSet.setProperty( PROP_RotationHorizontal, nRotationX );
    aPropSet.setProperty( PROP_Perspective, nPerspective );
    aPropSet.setProperty( PROP_RightAngledAxes, bRightAngled );
    aPropSet.setProperty( PROP_D3DScenePerspective, eProjMode );

    // set light settings
    aPropSet.setProperty( PROP_D3DSceneShadeMode, cssd::ShadeMode_FLAT );
    aPropSet.setProperty( PROP_D3DSceneAmbientColor, nAmbientColor );
    aPropSet.setProperty( PROP_D3DSceneLightOn1, false );
    aPropSet.setProperty( PROP_D3DSceneLightOn2, true );
    aPropSet.setProperty( PROP_D3DSceneLightColor2, nLightColor );
    aPropSet.setProperty( PROP_D3DSceneLightDirection2, cssd::Direction3D( 0.2, 0.4, 1.0 ) );
}

// ============================================================================

WallFloorConverter::WallFloorConverter( const ConverterRoot& rParent, WallFloorModel& rModel ) :
    ConverterBase< WallFloorModel >( rParent, rModel )
{
}

WallFloorConverter::~WallFloorConverter()
{
}

void WallFloorConverter::convertFromModel( const Reference< XDiagram >& rxDiagram, ObjectType eObjType )
{
    if( rxDiagram.is() )
    {
        PropertySet aPropSet;
        switch( eObjType )
        {
            case OBJECTTYPE_FLOOR:  aPropSet.set( rxDiagram->getFloor() );  break;
            case OBJECTTYPE_WALL:   aPropSet.set( rxDiagram->getWall() );   break;
            default:                OSL_ENSURE( false, "WallFloorConverter::convertFromModel - invalid object type" );
        }
        if( aPropSet.is() )
            getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, mrModel.mxPicOptions.getOrCreate(), eObjType );
    }
}

// ============================================================================

PlotAreaConverter::PlotAreaConverter( const ConverterRoot& rParent, PlotAreaModel& rModel ) :
    ConverterBase< PlotAreaModel >( rParent, rModel ),
    mb3dChart( false ),
    mbWall3dChart( false )
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
        xDiagram.set( createInstance( CREATE_OUSTRING( "com.sun.star.chart2.Diagram" ) ), UNO_QUERY_THROW );
        getChartDocument()->setFirstDiagram( xDiagram );
    }
    catch( Exception& )
    {
    }

    // store all axis models in a map, keyed by axis identifier
    typedef ModelMap< sal_Int32, AxisModel > AxisMap;
    AxisMap aAxisMap;
    for( PlotAreaModel::AxisVector::iterator aAIt = mrModel.maAxes.begin(), aAEnd = mrModel.maAxes.end(); aAIt != aAEnd; ++aAIt )
    {
        PlotAreaModel::AxisVector::value_type xAxis = *aAIt;
        OSL_ENSURE( xAxis->mnAxisId >= 0, "PlotAreaConverter::convertFromModel - invalid axis identifier" );
        OSL_ENSURE( !aAxisMap.has( xAxis->mnAxisId ), "PlotAreaConverter::convertFromModel - axis identifiers not unique" );
        if( xAxis->mnAxisId >= 0 )
            aAxisMap[ xAxis->mnAxisId ] = xAxis;
    }

    // group the type group models into different axes sets
    typedef ModelVector< AxesSetModel > AxesSetVector;
    AxesSetVector aAxesSets;
    sal_Int32 nMaxSeriesIdx = -1;
    for( PlotAreaModel::TypeGroupVector::iterator aTIt = mrModel.maTypeGroups.begin(), aTEnd = mrModel.maTypeGroups.end(); aTIt != aTEnd; ++aTIt )
    {
        PlotAreaModel::TypeGroupVector::value_type xTypeGroup = *aTIt;
        if( !xTypeGroup->maSeries.empty() )
        {
            // try to find a compatible axes set for the type group
            AxesSetModel* pAxesSet = 0;
            for( AxesSetVector::iterator aASIt = aAxesSets.begin(), aASEnd = aAxesSets.end(); !pAxesSet && (aASIt != aASEnd); ++aASIt )
                if( (*aASIt)->maTypeGroups.front()->maAxisIds == xTypeGroup->maAxisIds )
                    pAxesSet = aASIt->get();

            // not possible to insert into an existing axes set -> start a new axes set
            if( !pAxesSet )
            {
                pAxesSet = &aAxesSets.create();
                // find axis models used by the type group
                const TypeGroupModel::AxisIdVector& rAxisIds = xTypeGroup->maAxisIds;
                if( rAxisIds.size() >= 1 )
                    pAxesSet->maAxes[ API_X_AXIS ] = aAxisMap.get( rAxisIds[ 0 ] );
                if( rAxisIds.size() >= 2 )
                    pAxesSet->maAxes[ API_Y_AXIS ] = aAxisMap.get( rAxisIds[ 1 ] );
                if( rAxisIds.size() >= 3 )
                    pAxesSet->maAxes[ API_Z_AXIS ] = aAxisMap.get( rAxisIds[ 2 ] );
            }

            // insert the type group model
            pAxesSet->maTypeGroups.push_back( xTypeGroup );

            // collect the maximum series index for automatic series formatting
            for( TypeGroupModel::SeriesVector::iterator aSIt = xTypeGroup->maSeries.begin(), aSEnd = xTypeGroup->maSeries.end(); aSIt != aSEnd; ++aSIt )
                nMaxSeriesIdx = ::std::max( nMaxSeriesIdx, (*aSIt)->mnIndex );
        }
    }
    getFormatter().setMaxSeriesIndex( nMaxSeriesIdx );

    // varying point colors only for single series in single chart type
    bool bSupportsVaryColorsByPoint = mrModel.maTypeGroups.size() == 1;

    // convert all axes sets
    for( AxesSetVector::iterator aASBeg = aAxesSets.begin(), aASIt = aASBeg, aASEnd = aAxesSets.end(); aASIt != aASEnd; ++aASIt )
    {
        AxesSetConverter aAxesSetConv( *this, **aASIt );
        sal_Int32 nAxesSetIdx = static_cast< sal_Int32 >( aASIt - aASBeg );
        aAxesSetConv.convertFromModel( xDiagram, rView3DModel, nAxesSetIdx, bSupportsVaryColorsByPoint );
        if( nAxesSetIdx == 0 )
        {
            maAutoTitle = aAxesSetConv.getAutomaticTitle();
            mb3dChart = aAxesSetConv.is3dChart();
            mbWall3dChart = aAxesSetConv.isWall3dChart();
        }
        else
        {
            maAutoTitle = OUString();
        }
    }

    // plot area formatting
    if( xDiagram.is() && !mb3dChart )
    {
        PropertySet aPropSet( xDiagram->getWall() );
        getFormatter().convertFrameFormatting( aPropSet, mrModel.mxShapeProp, OBJECTTYPE_PLOTAREA2D );
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

