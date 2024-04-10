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

#include <ObjectHierarchy.hxx>
#include <ObjectIdentifier.hxx>
#include <Diagram.hxx>
#include <RegressionCurveHelper.hxx>
#include <RegressionCurveModel.hxx>
#include <Axis.hxx>
#include <AxisHelper.hxx>
#include <chartview/ExplicitValueProvider.hxx>
#include <ChartType.hxx>
#include <ChartTypeHelper.hxx>
#include <ChartModel.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <GridProperties.hxx>
#include <LegendHelper.hxx>
#include <chartview/DrawModelWrapper.hxx>
#include <unonames.hxx>
#include <BaseCoordinateSystem.hxx>

#include <map>
#include <algorithm>
#include <cstddef>

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;

namespace
{

void lcl_getChildOIDs(
    ::chart::ObjectHierarchy::tChildContainer& rOutChildren,
    const Reference< container::XIndexAccess >& xShapes )
{
    if( !xShapes.is())
        return;

    sal_Int32 nCount = xShapes->getCount();
    for( sal_Int32 i=0; i<nCount; ++i)
    {
        Reference< beans::XPropertySet > xShapeProp( xShapes->getByIndex( i ), uno::UNO_QUERY );
        if( xShapeProp.is())
        {
            Reference< beans::XPropertySetInfo > xInfo( xShapeProp->getPropertySetInfo());
            OUString aName;
            if( xInfo.is() &&
                xInfo->hasPropertyByName( u"Name"_ustr) &&
                (xShapeProp->getPropertyValue( u"Name"_ustr) >>= aName ) &&
                !aName.isEmpty() &&
                ::chart::ObjectIdentifier::isCID( aName ))
            {
                rOutChildren.emplace_back( aName );
            }
            Reference< container::XIndexAccess > xNewShapes( xShapeProp, uno::UNO_QUERY );
            if( xNewShapes.is())
                lcl_getChildOIDs( rOutChildren, xNewShapes );
        }
    }
}

void lcl_addAxisTitle( const rtl::Reference< ::chart::Axis >& xAxis, ::chart::ObjectHierarchy::tChildContainer& rContainer, const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    if( xAxis.is())
    {
        Reference< XTitle > xAxisTitle( xAxis->getTitleObject());
        if( xAxisTitle.is())
            rContainer.emplace_back( ::chart::ObjectIdentifier::createClassifiedIdentifierForObject( xAxisTitle, xChartModel ) );
    }
}

} // anonymous namespace

namespace chart
{

void ObjectHierarchy::createTree( const rtl::Reference<::chart::ChartModel>& xChartDocument )
{
    m_aChildMap.clear();

    if( !xChartDocument.is() )
        return;

    //@todo: change ObjectIdentifier to take an XChartDocument rather than XModel
    rtl::Reference< Diagram > xDiagram = xChartDocument->getFirstChartDiagram();
    ObjectIdentifier aDiaOID;
    if( xDiagram.is() )
        aDiaOID = ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( static_cast<cppu::OWeakObject*>(xDiagram.get()), xChartDocument ) );
    tChildContainer aTopLevelContainer;

    // First Level

    // Chart Area
    if( m_bOrderingForElementSelector )
    {
        aTopLevelContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, u"" ) );
        if( xDiagram.is() )
        {
            aTopLevelContainer.push_back( aDiaOID );
            createWallAndFloor( aTopLevelContainer, xDiagram );
            createLegendTree( aTopLevelContainer, xChartDocument, xDiagram  );
        }
    }

    // Main Title
    Reference< XTitle > xMainTitle( xChartDocument->getTitleObject());
    if( xMainTitle.is())
        aTopLevelContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierForObject( xMainTitle, xChartDocument ) );

    if( xDiagram.is())
    {
        // Sub Title.  Note: This is interpreted of being top level
        Reference< XTitle > xSubTitle( xDiagram->getTitleObject());
        if( xSubTitle.is())
            aTopLevelContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierForObject( xSubTitle, xChartDocument ) );

        if( !m_bOrderingForElementSelector )
        {
            // Axis Titles. Note: These are interpreted of being top level
            const std::vector< rtl::Reference< Axis > > aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram );
            for( rtl::Reference< Axis > const & axis : aAxes )
                lcl_addAxisTitle( axis, aTopLevelContainer, xChartDocument );

            // Diagram
            aTopLevelContainer.push_back( aDiaOID );
        }

        if( m_bFlattenDiagram )
            createDiagramTree( aTopLevelContainer, xChartDocument, xDiagram );
        else
        {
            tChildContainer aSubContainer;
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
        aTopLevelContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, u"" ) );

    if( ! aTopLevelContainer.empty())
        m_aChildMap[ObjectHierarchy::getRootNodeOID()] = aTopLevelContainer;
}

void ObjectHierarchy::createLegendTree(
    tChildContainer & rContainer,
    const rtl::Reference<::chart::ChartModel> & xChartDoc,
    const rtl::Reference< Diagram > & xDiagram  )
{
    if( !(xDiagram.is() && LegendHelper::hasLegend( xDiagram )) )
        return;

    ObjectIdentifier aLegendOID( ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForObject( xDiagram->getLegend(), xChartDoc ) ) );
    rContainer.push_back( aLegendOID );

    // iterate over child shapes of legend and search for matching CIDs
    if( m_pExplicitValueProvider )
    {
        rtl::Reference< SvxShapeGroupAnyD > xLegendShapeContainer =
            dynamic_cast<SvxShapeGroupAnyD*>(
                m_pExplicitValueProvider->getShapeForCID( aLegendOID.getObjectCID() ).get() );
        tChildContainer aLegendEntryOIDs;
        lcl_getChildOIDs( aLegendEntryOIDs, xLegendShapeContainer );

        m_aChildMap[ aLegendOID ] = aLegendEntryOIDs;
    }
}

void ObjectHierarchy::createAxesTree(
    tChildContainer & rContainer,
    const rtl::Reference<::chart::ChartModel> & xChartDoc,
    const rtl::Reference< Diagram > & xDiagram  )
{
    sal_Int32 nDimensionCount = xDiagram->getDimension();
    rtl::Reference< ChartType > xChartType( xDiagram->getChartTypeByIndex( 0 ) );
    bool bSupportsAxesGrids = ChartTypeHelper::isSupportingMainAxis( xChartType, nDimensionCount, 0 );
    if( !bSupportsAxesGrids )
        return;

    // Data Table
    uno::Reference<chart2::XDataTable> xDataTable = xDiagram->getDataTable();
    if (xDataTable.is())
    {
        rContainer.push_back(ObjectIdentifier::createClassifiedIdentifierForObject(xDataTable, xChartDoc));
    }

    // Axes
    std::vector< rtl::Reference< Axis > > aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram, /* bOnlyVisible = */ true );
    if( !m_bOrderingForElementSelector )
    {
        for (const auto& rAxis : aAxes)
            rContainer.push_back( ObjectIdentifier::createClassifiedIdentifierForObject( rAxis, xChartDoc ) );
    }

    // get all axes, also invisible ones
    aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram );
    // Grids
    for( rtl::Reference< Axis > const & xAxis : aAxes )
    {
        if(!xAxis.is())
            continue;

        sal_Int32 nCooSysIndex = 0;
        sal_Int32 nDimensionIndex = 0;
        sal_Int32 nAxisIndex = 0;
        AxisHelper::getIndicesForAxis( xAxis, xDiagram, nCooSysIndex, nDimensionIndex, nAxisIndex );
        if( nAxisIndex>0 && !ChartTypeHelper::isSupportingSecondaryAxis( xChartType, nDimensionCount ) )
            continue;

        if( m_bOrderingForElementSelector )
        {
            // axis
            if( AxisHelper::isAxisVisible( xAxis ) )
                rContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierForObject( xAxis, xChartDoc ) );

            // axis title
            lcl_addAxisTitle( xAxis, rContainer, xChartDoc );
        }

        rtl::Reference< ::chart::GridProperties > xGridProperties( xAxis->getGridProperties2() );
        if( AxisHelper::isGridVisible( xGridProperties ) )
        {
            //main grid
            rContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartDoc ) );
        }

        std::vector< rtl::Reference< ::chart::GridProperties > > aSubGrids( xAxis->getSubGridProperties2() );
        for( size_t nSubGrid = 0; nSubGrid < aSubGrids.size(); ++nSubGrid )
        {
            if( AxisHelper::isGridVisible( aSubGrids[nSubGrid] ) )
            {
                //sub grid
                rContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartDoc, nSubGrid ) );
            }
        }
    }
}

void ObjectHierarchy::createWallAndFloor(
    tChildContainer & rContainer,
    const rtl::Reference< Diagram > & xDiagram )
{
    sal_Int32 nDimensionCount = xDiagram->getDimension();
    bool bIsThreeD = ( nDimensionCount == 3 );
    bool bHasWall = xDiagram->isSupportingFloorAndWall();
    if( bHasWall && bIsThreeD )
    {
        rContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, u"" ) );

        Reference< beans::XPropertySet > xFloor( xDiagram->getFloor());
        if( xFloor.is())
            rContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_FLOOR, u"" ) );
    }

}

void ObjectHierarchy::createDiagramTree(
    tChildContainer & rContainer,
    const rtl::Reference<::chart::ChartModel> & xChartDoc,
    const rtl::Reference< Diagram > & xDiagram )
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

void ObjectHierarchy::createDataSeriesTree(
    tChildContainer & rOutDiagramSubContainer,
    const rtl::Reference< Diagram > & xDiagram )
{
    try
    {
        sal_Int32 nDimensionCount = xDiagram->getDimension();
        std::vector< rtl::Reference< BaseCoordinateSystem > > aCooSysSeq(
            xDiagram->getBaseCoordinateSystems());
        for( std::size_t nCooSysIdx=0; nCooSysIdx<aCooSysSeq.size(); ++nCooSysIdx )
        {
            std::vector< rtl::Reference< ChartType > > aChartTypeSeq( aCooSysSeq[nCooSysIdx]->getChartTypes2());
            for( std::size_t nCTIdx=0; nCTIdx<aChartTypeSeq.size(); ++nCTIdx )
            {
                rtl::Reference< ChartType > xChartType( aChartTypeSeq[nCTIdx] );
                std::vector< rtl::Reference< DataSeries > > aSeriesSeq( xChartType->getDataSeries2() );
                const sal_Int32 nNumberOfSeries =
                    ChartTypeHelper::getNumberOfDisplayedSeries( xChartType, aSeriesSeq.size());

                for( sal_Int32 nSeriesIdx=0; nSeriesIdx<nNumberOfSeries; ++nSeriesIdx )
                {
                    OUString aSeriesParticle(
                        ObjectIdentifier::createParticleForSeries(
                            0, nCooSysIdx, nCTIdx, nSeriesIdx ));
                    ObjectIdentifier aSeriesOID(
                        ObjectIdentifier( ObjectIdentifier::createClassifiedIdentifierForParticle( aSeriesParticle ) ) );
                    rOutDiagramSubContainer.push_back( aSeriesOID );

                    tChildContainer aSeriesSubContainer;

                    rtl::Reference< DataSeries > const & xSeries = aSeriesSeq[nSeriesIdx];

                    // data labels
                    if( DataSeriesHelper::hasDataLabelsAtSeries( xSeries ) )
                    {
                        OUString aChildParticle( ObjectIdentifier::getStringForType( OBJECTTYPE_DATA_LABELS ) + "=" );
                        aSeriesSubContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierForParticles( aSeriesParticle, aChildParticle ) );
                    }

                    // Statistics
                    if( ChartTypeHelper::isSupportingStatisticProperties( xChartType, nDimensionCount ) )
                    {
                        const std::vector< rtl::Reference< RegressionCurveModel > > & rCurves( xSeries->getRegressionCurves2());
                        for( size_t nCurveIdx=0; nCurveIdx<rCurves.size(); ++nCurveIdx )
                        {
                            bool bIsAverageLine = RegressionCurveHelper::isMeanValueLine( rCurves[nCurveIdx] );
                            aSeriesSubContainer.emplace_back( ObjectIdentifier::createDataCurveCID( aSeriesParticle, nCurveIdx, bIsAverageLine ) );
                            if( RegressionCurveHelper::hasEquation( rCurves[nCurveIdx] ) )
                            {
                                aSeriesSubContainer.emplace_back( ObjectIdentifier::createDataCurveEquationCID( aSeriesParticle, nCurveIdx ) );
                            }
                        }
                        Reference< beans::XPropertySet > xErrorBarProp;
                        if( (xSeries->getPropertyValue( CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarProp) &&
                            xErrorBarProp.is())
                        {
                            sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;
                            if( ( xErrorBarProp->getPropertyValue( u"ErrorBarStyle"_ustr) >>= nStyle ) &&
                                ( nStyle != css::chart::ErrorBarStyle::NONE ) )
                            {
                                aSeriesSubContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierWithParent(
                                        OBJECTTYPE_DATA_ERRORS_Y, u"", aSeriesParticle ) );
                            }
                        }

                        if( (xSeries->getPropertyValue(CHART_UNONAME_ERRORBAR_X) >>= xErrorBarProp) &&
                            xErrorBarProp.is())
                        {
                            sal_Int32 nStyle = css::chart::ErrorBarStyle::NONE;
                            if( ( xErrorBarProp->getPropertyValue( u"ErrorBarStyle"_ustr) >>= nStyle ) &&
                                ( nStyle != css::chart::ErrorBarStyle::NONE ) )
                            {
                                aSeriesSubContainer.emplace_back( ObjectIdentifier::createClassifiedIdentifierWithParent(
                                        OBJECTTYPE_DATA_ERRORS_X, u"", aSeriesParticle ) );
                            }
                        }
                    }

                    // Data Points
                    // iterate over child shapes of legend and search for matching CIDs
                    if( m_pExplicitValueProvider )
                    {
                        rtl::Reference< SvxShapeGroupAnyD > xSeriesShapeContainer =
                            dynamic_cast<SvxShapeGroupAnyD*>(
                            m_pExplicitValueProvider->getShapeForCID( aSeriesOID.getObjectCID() ).get() );
                        lcl_getChildOIDs( aSeriesSubContainer, xSeriesShapeContainer );
                    }

                    if( ! aSeriesSubContainer.empty())
                        m_aChildMap[ aSeriesOID ] = aSeriesSubContainer;
                }
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

void ObjectHierarchy::createAdditionalShapesTree(tChildContainer& rContainer)
{
    try
    {
        if ( m_pExplicitValueProvider )
        {
            rtl::Reference<SvxDrawPage> xDrawPage( m_pExplicitValueProvider->getDrawModelWrapper()->getMainDrawPage() );
            Reference< drawing::XShapes > xChartRoot( DrawModelWrapper::getChartRootShape( xDrawPage ) );
            sal_Int32 nCount = xDrawPage->getCount();
            for ( sal_Int32 i = 0; i < nCount; ++i )
            {
                Reference< drawing::XShape > xShape;
                if ( xDrawPage->getByIndex( i ) >>= xShape )
                {
                    if ( xShape.is() && xShape != xChartRoot )
                    {
                        rContainer.emplace_back( xShape );
                    }
                }
            }
        }
    }
    catch ( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

bool ObjectHierarchy::hasChildren( const ObjectIdentifier& rParent ) const
{
    if ( rParent.isValid() )
    {
        tChildMap::const_iterator aIt( m_aChildMap.find( rParent ));
        if( aIt != m_aChildMap.end())
            return ! (aIt->second.empty());
    }
    return false;
}

const ObjectHierarchy::tChildContainer & ObjectHierarchy::getChildren( const ObjectIdentifier& rParent ) const
{
    if ( rParent.isValid() )
    {
        tChildMap::const_iterator aIt( m_aChildMap.find( rParent ));
        if( aIt != m_aChildMap.end())
            return aIt->second;
    }
    static const tChildContainer EMPTY;
    return EMPTY;
}

const ObjectHierarchy::tChildContainer & ObjectHierarchy::getSiblings( const ObjectIdentifier& rNode ) const
{
    if ( rNode.isValid() && !ObjectHierarchy::isRootNode( rNode ) )
    {
        for (auto const& child : m_aChildMap)
        {
            tChildContainer::const_iterator aElemIt(
                std::find( child.second.begin(), child.second.end(), rNode ));
            if( aElemIt != child.second.end())
                return child.second;
        }
    }
    static const tChildContainer EMPTY;
    return EMPTY;
}

ObjectIdentifier ObjectHierarchy::getParentImpl(
    const ObjectIdentifier & rParentOID,
    const ObjectIdentifier & rOID ) const
{
    // search children
    tChildContainer aChildren( getChildren( rParentOID ));
    tChildContainer::const_iterator aIt(
        std::find( aChildren.begin(), aChildren.end(), rOID ));
    // recursion end
    if( aIt != aChildren.end())
        return rParentOID;

    for (auto const& child : aChildren)
    {
        // recursion
        ObjectIdentifier aTempParent( getParentImpl( child, rOID ));
        if ( aTempParent.isValid() )
        {
            // exit on success
            return aTempParent;
        }
    }

    // exit on fail
    return ObjectIdentifier();
}

ObjectIdentifier ObjectHierarchy::getParent(
    const ObjectIdentifier & rOID ) const
{
    return getParentImpl( ObjectHierarchy::getRootNodeOID(), rOID );
}

ObjectHierarchy::ObjectHierarchy(
    const rtl::Reference<::chart::ChartModel> & xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider /* = 0 */,
    bool bFlattenDiagram /* = false */,
    bool bOrderingForElementSelector /* = false */) :
        m_pExplicitValueProvider( pExplicitValueProvider ),
        m_bFlattenDiagram( bFlattenDiagram ),
        m_bOrderingForElementSelector( bOrderingForElementSelector )
{
    createTree( xChartDocument );
    // don't remember this helper to avoid access after lifetime
    m_pExplicitValueProvider = nullptr;
}

ObjectHierarchy::~ObjectHierarchy()
{}

ObjectIdentifier ObjectHierarchy::getRootNodeOID()
{
    return ObjectIdentifier( u"ROOT"_ustr );
}

bool ObjectHierarchy::isRootNode( const ObjectIdentifier& rOID )
{
    return ( rOID == ObjectHierarchy::getRootNodeOID() );
}

const ObjectHierarchy::tChildContainer & ObjectHierarchy::getTopLevelChildren() const
{
    return getChildren( ObjectHierarchy::getRootNodeOID());
}

sal_Int32 ObjectHierarchy::getIndexInParent(
    const ObjectIdentifier& rNode ) const
{
    ObjectIdentifier aParentOID( getParent( rNode ));
    const tChildContainer & aChildren( getChildren( aParentOID ) );
    sal_Int32 nIndex = 0;
    for (auto const& child : aChildren)
    {
        if ( child == rNode )
            return nIndex;
        ++nIndex;
    }
    return -1;
}

ObjectKeyNavigation::ObjectKeyNavigation(
    ObjectIdentifier aCurrentOID,
    rtl::Reference<::chart::ChartModel> xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider /* = 0 */ ) :
        m_aCurrentOID(std::move( aCurrentOID )),
        m_xChartDocument(std::move( xChartDocument )),
        m_pExplicitValueProvider( pExplicitValueProvider )
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

void ObjectKeyNavigation::setCurrentSelection( const ObjectIdentifier& rOID )
{
    m_aCurrentOID = rOID;
}

bool ObjectKeyNavigation::first()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
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
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
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
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection() ) );
    bool bResult = !aSiblings.empty();
    if( bResult )
    {
        ObjectHierarchy::tChildContainer::const_iterator aIt(
            std::find( aSiblings.begin(), aSiblings.end(), getCurrentSelection()));
        assert(aIt != aSiblings.end());
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
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection()));
    bool bResult = !aSiblings.empty();
    if( bResult )
    {
        ObjectHierarchy::tChildContainer::const_iterator aIt(
            std::find( aSiblings.begin(), aSiblings.end(), getCurrentSelection()));
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
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
    bool bResult = !ObjectHierarchy::isRootNode( getCurrentSelection());
    if( bResult )
        setCurrentSelection( aHierarchy.getParent( getCurrentSelection()));
    return bResult;
}

bool ObjectKeyNavigation::down()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
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
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
    ObjectHierarchy::tChildContainer aChildren( aHierarchy.getTopLevelChildren());
    bool bResult = !aChildren.empty();
    if( bResult )
        setCurrentSelection( aChildren.front());
    return bResult;
}

bool ObjectKeyNavigation::veryLast()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider );
    ObjectHierarchy::tChildContainer aChildren( aHierarchy.getTopLevelChildren());
    bool bResult = !aChildren.empty();
    if( bResult )
        setCurrentSelection( aChildren.back());
    return bResult;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
