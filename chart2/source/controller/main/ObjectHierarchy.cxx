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

#include "ObjectHierarchy.hxx"
#include "ObjectIdentifier.hxx"
#include "ChartModelHelper.hxx"
#include "DiagramHelper.hxx"
#include "RegressionCurveHelper.hxx"
#include "AxisHelper.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "macros.hxx"
#include "LineProperties.hxx"
#include "ChartTypeHelper.hxx"
#include "DataSeriesHelper.hxx"
#include "LegendHelper.hxx"
#include "chartview/DrawModelWrapper.hxx"

#include <map>
#include <algorithm>

#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{

struct lcl_ObjectToOID : public ::std::unary_function< Reference< uno::XInterface >, ::chart::ObjectIdentifier >
{
    explicit lcl_ObjectToOID( const Reference< chart2::XChartDocument > & xChartDoc ) :
            m_xModel( xChartDoc, uno::UNO_QUERY )
    {}

    ::chart::ObjectIdentifier operator() ( const Reference< uno::XInterface > & xObj )
    {
        return ::chart::ObjectIdentifier( ::chart::ObjectIdentifier::createClassifiedIdentifierForObject( xObj, m_xModel ) );
    }

private:
    Reference< frame::XModel > m_xModel;
};

void lcl_getChildOIDs(
    ::chart::ObjectHierarchy::tChildContainer& rOutChildren,
    const Reference< container::XIndexAccess >& xShapes )
{
    if( xShapes.is())
    {
        sal_Int32 nCount = xShapes->getCount();
        for( sal_Int32 i=0; i<nCount; ++i)
        {
            Reference< beans::XPropertySet > xShapeProp( xShapes->getByIndex( i ), uno::UNO_QUERY );
            if( xShapeProp.is())
            {
                Reference< beans::XPropertySetInfo > xInfo( xShapeProp->getPropertySetInfo());
                OUString aName;
                if( xInfo.is() &&
                    xInfo->hasPropertyByName( "Name") &&
                    (xShapeProp->getPropertyValue( "Name") >>= aName ) &&
                    !aName.isEmpty() &&
                    ::chart::ObjectIdentifier::isCID( aName ))
                {
                    rOutChildren.push_back( ::chart::ObjectIdentifier( aName ) );
                }
                Reference< container::XIndexAccess > xNewShapes( xShapeProp, uno::UNO_QUERY );
                if( xNewShapes.is())
                    lcl_getChildOIDs( rOutChildren, xNewShapes );
            }
        }
    }
}

void lcl_addAxisTitle( const Reference< XAxis >& xAxis, ::chart::ObjectHierarchy::tChildContainer& rContainer, const Reference< frame::XModel >& xChartModel )
{
    Reference< XTitled > xAxisTitled( xAxis, uno::UNO_QUERY );
    if( xAxisTitled.is())
    {
        Reference< XTitle > xAxisTitle( xAxisTitled->getTitleObject());
        if( xAxisTitle.is())
            rContainer.push_back(
                ::chart::ObjectIdentifier( ::chart::ObjectIdentifier::createClassifiedIdentifierForObject( xAxisTitle, xChartModel ) ) );
    }
}

} // anonymous namespace

namespace chart
{

namespace impl
{

class ImplObjectHierarchy
{
public:
    explicit ImplObjectHierarchy(
        const Reference< XChartDocument >& xChartDocument,
        ExplicitValueProvider* pExplicitValueProvider,
        bool bFlattenDiagram, bool bOrderingForElementSelector );

    bool                              hasChildren( const ObjectHierarchy::tOID& rParent );
    ObjectHierarchy::tChildContainer  getChildren( const ObjectHierarchy::tOID& rParent );
    ObjectHierarchy::tChildContainer  getSiblings( const ObjectHierarchy::tOID& rNode );

    ObjectHierarchy::tOID             getParent( const ObjectHierarchy::tOID& rOID );

private:
    void createTree( const Reference< XChartDocument > & xChartDocument );
    void createAxesTree(
        ObjectHierarchy::tChildContainer & rContainer,
        const Reference< XChartDocument > & xChartDoc,
        const Reference< XDiagram > & xDiagram  );
    void createDiagramTree(
        ObjectHierarchy::tChildContainer& rContainer,
        const Reference< XChartDocument >& xChartDoc,
        const Reference< XDiagram >& xDiagram );
    void createDataSeriesTree(
        ObjectHierarchy::tChildContainer & rOutDiagramSubContainer,
        const Reference< XDiagram > & xDiagram );
    void createWallAndFloor(
        ObjectHierarchy::tChildContainer & rContainer,
        const Reference< XDiagram > & xDiagram );
    void createLegendTree(
        ObjectHierarchy::tChildContainer & rContainer,
        const Reference< XChartDocument > & xChartDoc,
        const Reference< XDiagram > & xDiagram  );
    void createAdditionalShapesTree( ObjectHierarchy::tChildContainer& rContainer );

    ObjectHierarchy::tOID getParentImpl(
        const ObjectHierarchy::tOID& rParentOID,
        const ObjectHierarchy::tOID& rOID );

    typedef ::std::map< ObjectHierarchy::tOID, ObjectHierarchy::tChildContainer >
        tChildMap;
    tChildMap m_aChildMap;
    ExplicitValueProvider* m_pExplicitValueProvider;
    bool m_bFlattenDiagram;
    bool m_bOrderingForElementSelector;
};

ImplObjectHierarchy::ImplObjectHierarchy(
    const Reference< XChartDocument >& xChartDocument,
    ExplicitValueProvider* pExplicitValueProvider,
    bool bFlattenDiagram,
    bool bOrderingForElementSelector ) :
        m_pExplicitValueProvider( pExplicitValueProvider ),
        m_bFlattenDiagram( bFlattenDiagram ),
        m_bOrderingForElementSelector( bOrderingForElementSelector )
{
    createTree( xChartDocument );
    // don't remember this helper to avoid access after lifetime
    m_pExplicitValueProvider = 0;
}

void ImplObjectHierarchy::createTree( const Reference< XChartDocument >& xChartDocument )
{
    m_aChildMap = tChildMap();//clear tree

    if( !xChartDocument.is() )
        return;

    //@todo: change ObjectIdentifier to take an XChartDocument rather than XModel
    Reference< frame::XModel > xModel( xChartDocument, uno::UNO_QUERY );
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartDocument ) );
    ObjectHierarchy::tOID aDiaOID;
    if( xDiagram.is() )
        aDiaOID = ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( xDiagram, xModel ) );
    ObjectHierarchy::tChildContainer aTopLevelContainer;

    // First Level

    // Chart Area
    if( m_bOrderingForElementSelector )
    {
        aTopLevelContainer.push_back( ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, OUString() ) ) );
        if( xDiagram.is() )
        {
            aTopLevelContainer.push_back( aDiaOID );
            createWallAndFloor( aTopLevelContainer, xDiagram );
            createLegendTree( aTopLevelContainer, xChartDocument, xDiagram  );
        }
    }

    // Main Title
    Reference< XTitled > xDocTitled( xChartDocument, uno::UNO_QUERY );
    if( xDocTitled.is())
    {
        Reference< XTitle > xMainTitle( xDocTitled->getTitleObject());
        if( xMainTitle.is())
            aTopLevelContainer.push_back(
                ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( xMainTitle, xModel ) ) );
    }

    if( xDiagram.is())
    {
        // Sub Title.  Note: This is interpreted of being top level
        Reference< XTitled > xDiaTitled( xDiagram, uno::UNO_QUERY );
        if( xDiaTitled.is())
        {
            Reference< XTitle > xSubTitle( xDiaTitled->getTitleObject());
            if( xSubTitle.is())
                aTopLevelContainer.push_back(
                    ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( xSubTitle, xModel ) ) );
        }

        if( !m_bOrderingForElementSelector )
        {
            // Axis Titles. Note: These are interpreted of being top level
            Sequence< Reference< XAxis > > aAxes( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
            for( sal_Int32 i=0; i<aAxes.getLength(); ++i )
                lcl_addAxisTitle( aAxes[i], aTopLevelContainer, xModel );

            // Diagram
            aTopLevelContainer.push_back( aDiaOID );
        }

        if( m_bFlattenDiagram )
            createDiagramTree( aTopLevelContainer, xChartDocument, xDiagram );
        else
        {
            ObjectHierarchy::tChildContainer aSubContainer;
            createDiagramTree( aSubContainer, xChartDocument, xDiagram );
            if( !aSubContainer.empty() )
                m_aChildMap[ aDiaOID ] = aSubContainer;
        }

        if( !m_bOrderingForElementSelector )
            createLegendTree( aTopLevelContainer, xChartDocument, xDiagram  );
    }

    // #i12587# support for shapes in chart
    if ( !m_bOrderingForElementSelector )
    {
        createAdditionalShapesTree( aTopLevelContainer );
    }

    // Chart Area
    if( !m_bOrderingForElementSelector )
        aTopLevelContainer.push_back(
            ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, OUString() ) ) );

    if( ! aTopLevelContainer.empty())
        m_aChildMap[ ObjectHierarchy::getRootNodeOID() ] = aTopLevelContainer;
}

void ImplObjectHierarchy::createLegendTree(
    ObjectHierarchy::tChildContainer & rContainer,
    const Reference< XChartDocument > & xChartDoc,
    const Reference< XDiagram > & xDiagram  )
{
    if( xDiagram.is() && LegendHelper::hasLegend( xDiagram ) )
    {
        ObjectHierarchy::tOID aLegendOID( ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( xDiagram->getLegend(), Reference< frame::XModel >( xChartDoc, uno::UNO_QUERY ) ) ) );
        rContainer.push_back( aLegendOID );

        // iterate over child shapes of legend and search for matching CIDs
        if( m_pExplicitValueProvider )
        {
            Reference< container::XIndexAccess > xLegendShapeContainer(
                m_pExplicitValueProvider->getShapeForCID( aLegendOID.getObjectCID() ), uno::UNO_QUERY );
            ObjectHierarchy::tChildContainer aLegendEntryOIDs;
            lcl_getChildOIDs( aLegendEntryOIDs, xLegendShapeContainer );

            m_aChildMap[ aLegendOID ] = aLegendEntryOIDs;
        }
    }
}

void ImplObjectHierarchy::createAxesTree(
    ObjectHierarchy::tChildContainer & rContainer,
    const Reference< XChartDocument > & xChartDoc,
    const Reference< XDiagram > & xDiagram  )
{
    Reference< XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    bool bSupportsAxesGrids = ChartTypeHelper::isSupportingMainAxis( xChartType, nDimensionCount, 0 );
    if( bSupportsAxesGrids )
    {
        Sequence< Reference< XAxis > > aAxes( AxisHelper::getAllAxesOfDiagram( xDiagram, /* bOnlyVisible = */ true ) );
        if( !m_bOrderingForElementSelector )
            ::std::transform( aAxes.getConstArray(), aAxes.getConstArray() + aAxes.getLength(),
                          ::std::back_inserter( rContainer ),
                          lcl_ObjectToOID( xChartDoc ));

        // get all axes, also invisible ones
        aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram, /* bOnlyVisible = */ false );
        // Grids
        Reference< frame::XModel > xChartModel( xChartDoc, uno::UNO_QUERY );
        for( sal_Int32 nA=0; nA<aAxes.getLength(); ++nA )
        {
            Reference< XAxis > xAxis( aAxes[nA] );
            if(!xAxis.is())
                continue;

            sal_Int32 nCooSysIndex = 0;
            sal_Int32 nDimensionIndex = 0;
            sal_Int32 nAxisIndex = 0;
            AxisHelper::getIndicesForAxis( xAxis, xDiagram, nCooSysIndex, nDimensionIndex, nAxisIndex );
            if( nAxisIndex>0 && !ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimensionCount, nDimensionIndex ) )
                continue;

            if( m_bOrderingForElementSelector )
            {
                // axis
                if( AxisHelper::isAxisVisible( xAxis ) )
                    rContainer.push_back(
                        ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, xChartModel ) ) );

                // axis title
                lcl_addAxisTitle( aAxes[nA], rContainer, xChartModel );
            }

            Reference< beans::XPropertySet > xGridProperties( xAxis->getGridProperties() );
            if( AxisHelper::isGridVisible( xGridProperties ) )
            {
                //main grid
                rContainer.push_back(
                    ObjectIdentifier( ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel ) ) ) );
            }

            Sequence< Reference< beans::XPropertySet > > aSubGrids( xAxis->getSubGridProperties() );;
            sal_Int32 nSubGrid = 0;
            for( nSubGrid = 0; nSubGrid < aSubGrids.getLength(); ++nSubGrid )
            {
                Reference< beans::XPropertySet > xSubGridProperties( aSubGrids[nSubGrid] );
                if( AxisHelper::isGridVisible( xSubGridProperties ) )
                {
                    //sub grid
                    rContainer.push_back(
                        ObjectIdentifier( ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel, nSubGrid ) ) ) );
                }
            }
        }
    }
}

void ImplObjectHierarchy::createWallAndFloor(
    ObjectHierarchy::tChildContainer & rContainer,
    const Reference< XDiagram > & xDiagram )
{
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    bool bIsThreeD = ( nDimensionCount == 3 );
    bool bHasWall = DiagramHelper::isSupportingFloorAndWall( xDiagram );
    if( bHasWall && bIsThreeD )
    {
        rContainer.push_back(
            ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, OUString() ) ) );

        Reference< beans::XPropertySet > xFloor( xDiagram->getFloor());
        if( xFloor.is())
            rContainer.push_back(
                ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_FLOOR, OUString() ) ) );
    }

}

void ImplObjectHierarchy::createDiagramTree(
    ObjectHierarchy::tChildContainer & rContainer,
    const Reference< XChartDocument > & xChartDoc,
    const Reference< XDiagram > & xDiagram )
{
    if( !m_bOrderingForElementSelector )
    {
        createDataSeriesTree( rContainer, xDiagram );
        createAxesTree( rContainer, xChartDoc, xDiagram  );
        createWallAndFloor( rContainer, xDiagram );
    }
    else
    {
        createAxesTree( rContainer, xChartDoc, xDiagram  );
        createDataSeriesTree( rContainer, xDiagram );
    }
}

void ImplObjectHierarchy::createDataSeriesTree(
    ObjectHierarchy::tChildContainer & rOutDiagramSubContainer,
    const Reference< XDiagram > & xDiagram )
{
    Reference< XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );

    try
    {
        sal_Int32 nDiagramIndex = 0;
        sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypeSeq.getLength(); ++nCTIdx )
            {
                Reference< XChartType > xChartType( aChartTypeSeq[nCTIdx] );
                Reference< XDataSeriesContainer > xDSCnt( xChartType, uno::UNO_QUERY_THROW );
                Sequence< Reference< XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                const sal_Int32 nNumberOfSeries =
                    ChartTypeHelper::getNumberOfDisplayedSeries( xChartType, aSeriesSeq.getLength());

                for( sal_Int32 nSeriesIdx=0; nSeriesIdx<nNumberOfSeries; ++nSeriesIdx )
                {
                    OUString aSeriesParticle(
                        ObjectIdentifier::createParticleForSeries(
                            nDiagramIndex, nCooSysIdx, nCTIdx, nSeriesIdx ));
                    ObjectHierarchy::tOID aSeriesOID(
                        ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForParticle( aSeriesParticle ) ) );
                    rOutDiagramSubContainer.push_back( aSeriesOID );

                    ObjectHierarchy::tChildContainer aSeriesSubContainer;

                    Reference< chart2::XDataSeries > xSeries( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY );

                    // data lablels
                    if( DataSeriesHelper::hasDataLabelsAtSeries( xSeries ) )
                    {
                        OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) );
                        aChildParticle+=("=");
                        aSeriesSubContainer.push_back(
                                    ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForParticles( aSeriesParticle, aChildParticle ) ) );
                    }

                    // Statistics
                    if( ChartTypeHelper::isSupportingStatisticProperties( xChartType, nDimensionCount ) )
                    {
                        Reference< chart2::XRegressionCurveContainer > xCurveCnt( xSeries, uno::UNO_QUERY );
                        if( xCurveCnt.is())
                        {
                            Sequence< Reference< chart2::XRegressionCurve > > aCurves( xCurveCnt->getRegressionCurves());
                            for( sal_Int32 nCurveIdx=0; nCurveIdx<aCurves.getLength(); ++nCurveIdx )
                            {
                                bool bIsAverageLine = RegressionCurveHelper::isMeanValueLine( aCurves[nCurveIdx] );
                                aSeriesSubContainer.push_back(
                                    ObjectIdentifier( ObjectIdentifier::createDataCurveCID( aSeriesParticle, nCurveIdx, bIsAverageLine ) ) );
                                if( RegressionCurveHelper::hasEquation( aCurves[nCurveIdx] ) )
                                {
                                    aSeriesSubContainer.push_back(
                                        ObjectIdentifier( ObjectIdentifier::createDataCurveEquationCID( aSeriesParticle, nCurveIdx ) ) );
                                }
                            }
                            Reference< beans::XPropertySet > xSeriesProp( xSeries, uno::UNO_QUERY );
                            Reference< beans::XPropertySet > xErrorBarProp;
                            if( xSeriesProp.is() &&
                                (xSeriesProp->getPropertyValue( "ErrorBarY") >>= xErrorBarProp) &&
                                xErrorBarProp.is())
                            {
                                sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
                                if( ( xErrorBarProp->getPropertyValue( "ErrorBarStyle") >>= nStyle ) &&
                                    ( nStyle != ::com::sun::star::chart::ErrorBarStyle::NONE ) )
                                {
                                    aSeriesSubContainer.push_back(
                                        ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierWithParent(
                                            OBJECTTYPE_DATA_ERRORS_Y, OUString(), aSeriesParticle ) ) );
                                }
                            }

                            if( xSeriesProp.is() &&
                                (xSeriesProp->getPropertyValue( "ErrorBarX") >>= xErrorBarProp) &&
                                xErrorBarProp.is())
                            {
                                sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
                                if( ( xErrorBarProp->getPropertyValue( "ErrorBarStyle") >>= nStyle ) &&
                                    ( nStyle != ::com::sun::star::chart::ErrorBarStyle::NONE ) )
                                {
                                    aSeriesSubContainer.push_back(
                                        ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierWithParent(
                                            OBJECTTYPE_DATA_ERRORS_X, OUString(), aSeriesParticle ) ) );
                                }
                            }
                        }
                    }

                    // Data Points
                    // iterate over child shapes of legend and search for matching CIDs
                    if( m_pExplicitValueProvider )
                    {
                        Reference< container::XIndexAccess > xSeriesShapeContainer(
                            m_pExplicitValueProvider->getShapeForCID( aSeriesOID.getObjectCID() ), uno::UNO_QUERY );
                        lcl_getChildOIDs( aSeriesSubContainer, xSeriesShapeContainer );
                    }

                    if( ! aSeriesSubContainer.empty())
                        m_aChildMap[ aSeriesOID ] = aSeriesSubContainer;
                }
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

void ImplObjectHierarchy::createAdditionalShapesTree( ObjectHierarchy::tChildContainer& rContainer )
{
    try
    {
        if ( m_pExplicitValueProvider )
        {
            Reference< drawing::XDrawPage > xDrawPage( m_pExplicitValueProvider->getDrawModelWrapper()->getMainDrawPage() );
            Reference< drawing::XShapes > xDrawPageShapes( xDrawPage, uno::UNO_QUERY_THROW );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPageShapes->getCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPageShapes->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() && xShape != xChartRoot )
                    {
                        rContainer.push_back( ObjectIdentifier( xShape ) );
                    }
                }
            }
        }
    }
    catch ( const uno::Exception& ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

bool ImplObjectHierarchy::hasChildren( const ObjectHierarchy::tOID& rParent )
{
    if ( rParent.isValid() )
    {
        tChildMap::const_iterator aIt( m_aChildMap.find( rParent ));
        if( aIt != m_aChildMap.end())
            return ! (aIt->second.empty());
    }
    return false;
}

ObjectHierarchy::tChildContainer ImplObjectHierarchy::getChildren( const ObjectHierarchy::tOID& rParent )
{
    if ( rParent.isValid() )
    {
        tChildMap::const_iterator aIt( m_aChildMap.find( rParent ));
        if( aIt != m_aChildMap.end())
            return aIt->second;
    }
    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tChildContainer ImplObjectHierarchy::getSiblings( const ObjectHierarchy::tOID& rNode )
{
    if ( rNode.isValid() && !ObjectHierarchy::isRootNode( rNode ) )
    {
        for( tChildMap::const_iterator aIt( m_aChildMap.begin());
             aIt != m_aChildMap.end(); ++aIt )
        {
            ObjectHierarchy::tChildContainer::const_iterator aElemIt(
                ::std::find( aIt->second.begin(), aIt->second.end(), rNode ));
            if( aElemIt != aIt->second.end())
                return aIt->second;
        }
    }
    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tOID ImplObjectHierarchy::getParentImpl(
    const ObjectHierarchy::tOID & rParentOID,
    const ObjectHierarchy::tOID & rOID )
{
    // search children
    ObjectHierarchy::tChildContainer aChildren( getChildren( rParentOID ));
    ObjectHierarchy::tChildContainer::const_iterator aIt(
        ::std::find( aChildren.begin(), aChildren.end(), rOID ));
    // recursion end
    if( aIt != aChildren.end())
        return rParentOID;

    for( aIt = aChildren.begin(); aIt != aChildren.end(); ++aIt )
    {
        // recursion
        ObjectHierarchy::tOID aTempParent( getParentImpl( *aIt, rOID ));
        if ( aTempParent.isValid() )
        {
            // exit on success
            return aTempParent;
        }
    }

    // exit on fail
    return ObjectHierarchy::tOID();
}

ObjectHierarchy::tOID ImplObjectHierarchy::getParent(
    const ObjectHierarchy::tOID & rOID )
{
    return getParentImpl( ObjectHierarchy::getRootNodeOID(), rOID );
}

} // namespace impl

ObjectHierarchy::ObjectHierarchy(
    const Reference< XChartDocument > & xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider /* = 0 */,
    bool bFlattenDiagram /* = false */,
    bool bOrderingForElementSelector /* = false */) :
        m_apImpl( new impl::ImplObjectHierarchy( xChartDocument, pExplicitValueProvider, bFlattenDiagram, bOrderingForElementSelector ))
{}

ObjectHierarchy::~ObjectHierarchy()
{}

ObjectHierarchy::tOID ObjectHierarchy::getRootNodeOID()
{
    return ObjectIdentifier( "ROOT" );
}

bool ObjectHierarchy::isRootNode( const ObjectHierarchy::tOID& rOID )
{
    return ( rOID == ObjectHierarchy::getRootNodeOID() );
}

ObjectHierarchy::tChildContainer ObjectHierarchy::getTopLevelChildren() const
{
    return m_apImpl->getChildren( ObjectHierarchy::getRootNodeOID());
}

bool ObjectHierarchy::hasChildren( const tOID& rParent ) const
{
    return m_apImpl->hasChildren( rParent );
}

ObjectHierarchy::tChildContainer ObjectHierarchy::getChildren(
    const ObjectHierarchy::tOID& rParent ) const
{
    if ( rParent.isValid() )
        return m_apImpl->getChildren( rParent );

    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tChildContainer ObjectHierarchy::getSiblings(
    const ObjectHierarchy::tOID& rNode ) const
{
    if ( rNode.isValid() && !isRootNode( rNode ) )
        return m_apImpl->getSiblings( rNode );

    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tOID ObjectHierarchy::getParent(
    const ObjectHierarchy::tOID& rNode ) const
{
    return m_apImpl->getParent( rNode );
}

sal_Int32 ObjectHierarchy::getIndexInParent(
    const ObjectHierarchy::tOID& rNode ) const
{
    tOID aParentOID( m_apImpl->getParent( rNode ));
    tChildContainer aChildren( m_apImpl->getChildren( aParentOID ) );
    tChildContainer::const_iterator aIt( aChildren.begin() );
    for( sal_Int32 nIndex = 0; aIt != aChildren.end(); ++nIndex, ++aIt )
    {
        if ( *aIt == rNode )
            return nIndex;
    }
    return -1;
}

ObjectKeyNavigation::ObjectKeyNavigation(
    const ObjectHierarchy::tOID & rCurrentOID,
    const Reference< chart2::XChartDocument > & xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider /* = 0 */ ) :
        m_aCurrentOID( rCurrentOID ),
        m_xChartDocument( xChartDocument ),
        m_pExplicitValueProvider( pExplicitValueProvider ),
        m_bStepDownInDiagram( true )
{
    if ( !m_aCurrentOID.isValid() )
    {
        setCurrentSelection( ObjectHierarchy::getRootNodeOID() );
    }
}

bool ObjectKeyNavigation::handleKeyEvent(
    const awt::KeyEvent & rEvent )
{
    bool bResult = false;

    switch( rEvent.KeyCode )
    {
        case awt::Key::TAB:
            if( rEvent.Modifiers & awt::KeyModifier::SHIFT )
                bResult = previous();
            else
                bResult = next();
            break;
        case awt::Key::HOME:
            bResult = first();
            break;
        case awt::Key::END:
            bResult = last();
            break;
        case awt::Key::F3:
            if( rEvent.Modifiers & awt::KeyModifier::SHIFT )
                bResult = up();
            else
                bResult = down();
            break;
        case awt::Key::ESCAPE:
            setCurrentSelection( ObjectIdentifier() );
            bResult = true;
            break;
        default:
            bResult = false;
            break;
    }
    return bResult;
}

void ObjectKeyNavigation::setCurrentSelection( const ObjectHierarchy::tOID& rOID )
{
    m_aCurrentOID = rOID;
}

ObjectHierarchy::tOID ObjectKeyNavigation::getCurrentSelection() const
{
    return m_aCurrentOID;
}

bool ObjectKeyNavigation::first()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection() ) );
    bool bResult = !aSiblings.empty();
    if( bResult )
        setCurrentSelection( aSiblings.front());
    else
        bResult = veryFirst();
    return bResult;
}

bool ObjectKeyNavigation::last()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection() ) );
    bool bResult = !aSiblings.empty();
    if( bResult )
        setCurrentSelection( aSiblings.back());
    else
        bResult = veryLast();
    return bResult;
}

bool ObjectKeyNavigation::next()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection() ) );
    bool bResult = !aSiblings.empty();
    if( bResult )
    {
        ObjectHierarchy::tChildContainer::const_iterator aIt(
            ::std::find( aSiblings.begin(), aSiblings.end(), getCurrentSelection()));
        OSL_ASSERT( aIt != aSiblings.end());
        if( ++aIt == aSiblings.end())
            aIt = aSiblings.begin();
        setCurrentSelection( *aIt );
    }
    else
        bResult = veryFirst();

    return bResult;
}

bool ObjectKeyNavigation::previous()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection()));
    bool bResult = !aSiblings.empty();
    if( bResult )
    {
        ObjectHierarchy::tChildContainer::const_iterator aIt(
            ::std::find( aSiblings.begin(), aSiblings.end(), getCurrentSelection()));
        OSL_ASSERT( aIt != aSiblings.end());
        if( aIt == aSiblings.begin())
            aIt = aSiblings.end();
        --aIt;
        setCurrentSelection( *aIt );
    }
    else
        bResult = veryLast();
    return bResult;
}

bool ObjectKeyNavigation::up()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    bool bResult = !ObjectHierarchy::isRootNode( getCurrentSelection());
    if( bResult )
        setCurrentSelection( aHierarchy.getParent( getCurrentSelection()));
    return bResult;
}

bool ObjectKeyNavigation::down()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    bool bResult = aHierarchy.hasChildren( getCurrentSelection());
    if( bResult )
    {
        ObjectHierarchy::tChildContainer aChildren = aHierarchy.getChildren( getCurrentSelection());
        OSL_ASSERT( !aChildren.empty());
        setCurrentSelection( aChildren.front());
    }
    return bResult;
}

bool ObjectKeyNavigation::veryFirst()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aChildren( aHierarchy.getTopLevelChildren());
    bool bResult = !aChildren.empty();
    if( bResult )
        setCurrentSelection( aChildren.front());
    return bResult;
}

bool ObjectKeyNavigation::veryLast()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aChildren( aHierarchy.getTopLevelChildren());
    bool bResult = !aChildren.empty();
    if( bResult )
        setCurrentSelection( aChildren.back());
    return bResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
