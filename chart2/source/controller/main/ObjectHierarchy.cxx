/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectHierarchy.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:02:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

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
using ::rtl::OUString;

namespace
{
struct lcl_ObjectToCID : public ::std::unary_function< Reference< uno::XInterface >, OUString >
{
    explicit lcl_ObjectToCID( const Reference< chart2::XChartDocument > & xChartDoc ) :
            m_xModel( xChartDoc, uno::UNO_QUERY )
    {}

    OUString operator() ( const Reference< uno::XInterface > & xObj )
    {
        return ::chart::ObjectIdentifier::createClassifiedIdentifierForObject( xObj, m_xModel );
    }

private:
    Reference< frame::XModel > m_xModel;
};

void lcl_getChildCIDs(
    ::chart::ObjectHierarchy::tChildContainer & rOutChildren,
    const Reference< container::XIndexAccess > & xShapes )
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
                    xInfo->hasPropertyByName( C2U("Name")) &&
                    (xShapeProp->getPropertyValue( C2U("Name")) >>= aName ) &&
                    aName.getLength() > 0 &&
                    ::chart::ObjectIdentifier::isCID( aName ))
                {
                    rOutChildren.push_back( aName );
                }
                Reference< container::XIndexAccess > xNewShapes( xShapeProp, uno::UNO_QUERY );
                if( xNewShapes.is())
                    lcl_getChildCIDs( rOutChildren, xNewShapes );
            }
        }
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
        const Reference< XChartDocument > & xChartDocument,
        ExplicitValueProvider * pExplicitValueProvider,
        bool bFlattenDiagram );

    bool                              hasChildren( const OUString & rParent );
    ObjectHierarchy::tChildContainer  getChildren( const OUString & rParent );
    ObjectHierarchy::tChildContainer  getSiblings( const OUString & rNode );

    ObjectHierarchy::tCID             getParent( const ObjectHierarchy::tCID & rCID );

private:
    void createTree( const Reference< XChartDocument > & xChartDocument );
    void createDiagramTree(
        ObjectHierarchy::tChildContainer & rContainer,
        const Reference< XChartDocument > & xChartDoc,
        const Reference< XDiagram > & xDiagram );
    void createDataSeriesTree(
        ObjectHierarchy::tChildContainer & rOutDiagramSubContainer,
        const Reference< XCoordinateSystemContainer > & xCooSysCnt );
    ObjectHierarchy::tCID getParentImpl(
        const ObjectHierarchy::tCID & rParentCID,
        const ObjectHierarchy::tCID & rCID );

    typedef ::std::map< OUString, ObjectHierarchy::tChildContainer >
        tChildMap;
    tChildMap m_aChildMap;
    ExplicitValueProvider * m_pExplicitValueProvider;
    bool m_bFlattenDiagram;
};

ImplObjectHierarchy::ImplObjectHierarchy(
    const Reference< XChartDocument > & xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider,
    bool bFlattenDiagram ) :
        m_pExplicitValueProvider( pExplicitValueProvider ),
        m_bFlattenDiagram( bFlattenDiagram )
{
    createTree( xChartDocument );
    // don't remember this helper to avoid access after lifetime
    m_pExplicitValueProvider = 0;
}

void ImplObjectHierarchy::createTree( const Reference< XChartDocument > & xChartDocument )
{
    if( !xChartDocument.is())
        return;

    //@todo: change ObjectIdentifier to take an XChartDocument rather than XModel
    Reference< frame::XModel > xModel( xChartDocument, uno::UNO_QUERY );
    ObjectHierarchy::tChildContainer aTopLevelContainer;

    // First Level

    // Main Title
    Reference< XTitled > xDocTitled( xChartDocument, uno::UNO_QUERY );
    if( xDocTitled.is())
    {
        Reference< XTitle > xMainTitle( xDocTitled->getTitleObject());
        if( xMainTitle.is())
            aTopLevelContainer.push_back(
                ObjectIdentifier::createClassifiedIdentifierForObject( xMainTitle, xModel ));
    }

    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartDocument ));
    if( xDiagram.is())
    {
        // Sub Title.  Note: This is interpreted of being top level
        Reference< XTitled > xDiaTitled( xDiagram, uno::UNO_QUERY );
        if( xDiaTitled.is())
        {
            Reference< XTitle > xSubTitle( xDiaTitled->getTitleObject());
            if( xSubTitle.is())
                aTopLevelContainer.push_back(
                    ObjectIdentifier::createClassifiedIdentifierForObject( xSubTitle, xModel ));
        }

        // Axis Titles. Note: These are interpreted of being top level
        Sequence< Reference< XAxis > > aAxes( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
        for( sal_Int32 i=0; i<aAxes.getLength(); ++i )
        {
            Reference< XTitled > xAxisTitled( aAxes[i], uno::UNO_QUERY );
            if( xAxisTitled.is())
            {
                Reference< XTitle > xAxisTitle( xAxisTitled->getTitleObject());
                if( xAxisTitle.is())
                    aTopLevelContainer.push_back(
                        ObjectIdentifier::createClassifiedIdentifierForObject( xAxisTitle, xModel ));
            }
        }

        // Diagram
        OUString aDiaCID( ObjectIdentifier::createClassifiedIdentifierForObject( xDiagram, xModel ));
        OSL_ASSERT( aDiaCID.getLength());
        aTopLevelContainer.push_back( aDiaCID );
        if( m_bFlattenDiagram )
            createDiagramTree( aTopLevelContainer, xChartDocument, xDiagram );
        else
        {
            ObjectHierarchy::tChildContainer aSubContainer;
            createDiagramTree( aSubContainer, xChartDocument, xDiagram );
            if( ! aSubContainer.empty())
                m_aChildMap[ aDiaCID ] = aSubContainer;
        }


        // Legend. Note: This is interpreted of being top level
        Reference< XLegend > xLegend( xDiagram->getLegend());
        if( xLegend.is())
        {
            Reference< beans::XPropertySet > xLegendProp( xLegend, uno::UNO_QUERY );
            bool bShow = false;
            if( xLegendProp.is() &&
                (xLegendProp->getPropertyValue( C2U("Show")) >>= bShow) &&
                bShow )
            {
                OUString aLegendCID( ObjectIdentifier::createClassifiedIdentifierForObject( xLegend, xModel ));
                aTopLevelContainer.push_back( aLegendCID );

                // iterate over child shapes of legend and search for matching CIDs
                if( m_pExplicitValueProvider )
                {
                    Reference< container::XIndexAccess > xLegendShapeContainer(
                        m_pExplicitValueProvider->getShapeForCID( aLegendCID ), uno::UNO_QUERY );
                    ObjectHierarchy::tChildContainer aLegendEntryCIDs;
                    lcl_getChildCIDs( aLegendEntryCIDs, xLegendShapeContainer );

                    m_aChildMap[ aLegendCID ] = aLegendEntryCIDs;
                }
            }
        }
    }

    // Chart Area
    aTopLevelContainer.push_back(
        ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, OUString() ) );

    if( ! aTopLevelContainer.empty())
        m_aChildMap[ ObjectHierarchy::getRootNodeCID() ] = aTopLevelContainer;
}

void ImplObjectHierarchy::createDiagramTree(
    ObjectHierarchy::tChildContainer & rContainer,
    const Reference< XChartDocument > & xChartDoc,
    const Reference< XDiagram > & xDiagram )
{
    // Data Series
    Reference< XCoordinateSystemContainer > xCooSysCnt( xDiagram, uno::UNO_QUERY_THROW );
    createDataSeriesTree( rContainer, xCooSysCnt );

    // Axes
    sal_Int32 nDimensionCount = DiagramHelper::getDimension( xDiagram );
    uno::Reference< chart2::XChartType > xChartType( DiagramHelper::getChartTypeByIndex( xDiagram, 0 ) );
    bool bSupportsAxesGrids = ChartTypeHelper::isSupportingMainAxis( xChartType, nDimensionCount, 0 );
    bool bIsThreeD = ( nDimensionCount == 3 );
    bool bHasWall = DiagramHelper::isSupportingFloorAndWall( xDiagram );
    if( bSupportsAxesGrids )
    {
        Sequence< Reference< XAxis > > aAxes( AxisHelper::getAllAxesOfDiagram( xDiagram, /* bOnlyVisible = */ true ) );
        ::std::transform( aAxes.getConstArray(), aAxes.getConstArray() + aAxes.getLength(),
                          ::std::back_inserter( rContainer ),
                          lcl_ObjectToCID( xChartDoc ));

        // get all axes, also invisible ones
        aAxes = AxisHelper::getAllAxesOfDiagram( xDiagram, /* bOnlyVisible = */ false );
        // Grids
        Reference< frame::XModel > xChartModel( xChartDoc, uno::UNO_QUERY );
        for( sal_Int32 nA=0; nA<aAxes.getLength(); ++nA )
        {
            Reference< XAxis > xAxis( aAxes[nA] );
            if(!xAxis.is())
                continue;

            Reference< beans::XPropertySet > xGridProperties( xAxis->getGridProperties() );
            if( AxisHelper::isGridVisible( xGridProperties ) )
            {
                //main grid
                rContainer.push_back(
                    ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel ) );
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
                        ObjectIdentifier::createClassifiedIdentifierForGrid( xAxis, xChartModel, nSubGrid ) );
                }
            }
        }
    }

    // Wall
    if( bHasWall )
    {
        rContainer.push_back(
            ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, rtl::OUString()));
    }

    // Floor
    if( bHasWall && bIsThreeD )
    {
        Reference< beans::XPropertySet > xFloor( xDiagram->getFloor());
        if( xFloor.is())
            rContainer.push_back(
                ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_FLOOR, rtl::OUString()));
    }
}

void ImplObjectHierarchy::createDataSeriesTree(
    ObjectHierarchy::tChildContainer & rOutDiagramSubContainer,
    const Reference< XCoordinateSystemContainer > & xCooSysCnt )
{
    try
    {
        sal_Int32 nDiagramIndex = 0;
        Sequence< Reference< XCoordinateSystem > > aCooSysSeq(
            xCooSysCnt->getCoordinateSystems());
        for( sal_Int32 nCooSysIdx=0; nCooSysIdx<aCooSysSeq.getLength(); ++nCooSysIdx )
        {
            Reference< XChartTypeContainer > xCTCnt( aCooSysSeq[nCooSysIdx], uno::UNO_QUERY_THROW );
            Sequence< Reference< XChartType > > aChartTypeSeq( xCTCnt->getChartTypes());
            for( sal_Int32 nCTIdx=0; nCTIdx<aChartTypeSeq.getLength(); ++nCTIdx )
            {
                Reference< XDataSeriesContainer > xDSCnt( aChartTypeSeq[nCTIdx], uno::UNO_QUERY_THROW );
                Sequence< Reference< XDataSeries > > aSeriesSeq( xDSCnt->getDataSeries() );
                const sal_Int32 nNumberOfSeries =
                    ChartTypeHelper::getNumberOfDisplayedSeries( aChartTypeSeq[nCTIdx], aSeriesSeq.getLength());

                for( sal_Int32 nSeriesIdx=0; nSeriesIdx<nNumberOfSeries; ++nSeriesIdx )
                {
                    OUString aSeriesParticle(
                        ObjectIdentifier::createParticleForSeries(
                            nDiagramIndex, nCooSysIdx, nCTIdx, nSeriesIdx ));
                    ObjectHierarchy::tCID aSeriesCID(
                        ObjectIdentifier::createClassifiedIdentifierForParticle( aSeriesParticle ));
                    rOutDiagramSubContainer.push_back( aSeriesCID );

                    ObjectHierarchy::tChildContainer aSeriesSubContainer;

                    // Statistics
                    Reference< chart2::XRegressionCurveContainer > xCurveCnt( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY );
                    if( xCurveCnt.is())
                    {
                        Sequence< Reference< chart2::XRegressionCurve > > aCurves( xCurveCnt->getRegressionCurves());
                        for( sal_Int32 nCurveIdx=0; nCurveIdx<aCurves.getLength(); ++nCurveIdx )
                        {
                            bool bIsAverageLine = RegressionCurveHelper::isMeanValueLine( aCurves[nCurveIdx] );
                            aSeriesSubContainer.push_back(
                                ObjectIdentifier::createDataCurveCID( aSeriesParticle, nCurveIdx, bIsAverageLine ));
                            Reference< beans::XPropertySet > xEqProp( aCurves[nCurveIdx]->getEquationProperties());
                            bool bShowEq = false;
                            bool bShowCoeff = false;
                            if( xEqProp.is() &&
                                ( (xEqProp->getPropertyValue( C2U("ShowEquation")) >>= bShowEq) ||
                                  (xEqProp->getPropertyValue( C2U("ShowCorrelationCoefficient")) >>= bShowCoeff) ) &&
                                ( bShowEq || bShowCoeff ) )
                            {
                                aSeriesSubContainer.push_back(
                                    ObjectIdentifier::createDataCurveEquationCID( aSeriesParticle, nCurveIdx ));
                            }
                        }
                        Reference< beans::XPropertySet > xSeriesProp( aSeriesSeq[nSeriesIdx], uno::UNO_QUERY );
                        Reference< beans::XPropertySet > xErrorBarProp;
                        if( xSeriesProp.is() &&
                            (xSeriesProp->getPropertyValue( C2U("ErrorBarY")) >>= xErrorBarProp) &&
                            xErrorBarProp.is())
                        {
                            sal_Int32 nStyle = ::com::sun::star::chart::ErrorBarStyle::NONE;
                            if( ( xErrorBarProp->getPropertyValue( C2U("ErrorBarStyle")) >>= nStyle ) &&
                                ( nStyle != ::com::sun::star::chart::ErrorBarStyle::NONE ) )
                            {
                                aSeriesSubContainer.push_back(
                                    ObjectIdentifier::createClassifiedIdentifierWithParent(
                                        OBJECTTYPE_DATA_ERRORS, OUString(), aSeriesParticle ));
                            }
                        }
                    }

                    // Data Points
                    // iterate over child shapes of legend and search for matching CIDs
                    if( m_pExplicitValueProvider )
                    {
                        Reference< container::XIndexAccess > xSeriesShapeContainer(
                            m_pExplicitValueProvider->getShapeForCID( aSeriesCID ), uno::UNO_QUERY );
                        lcl_getChildCIDs( aSeriesSubContainer, xSeriesShapeContainer );
                    }

                    if( ! aSeriesSubContainer.empty())
                        m_aChildMap[ aSeriesCID ] = aSeriesSubContainer;
                }
            }
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

bool ImplObjectHierarchy::hasChildren( const OUString & rParent )
{
    if( rParent.getLength())
    {
        tChildMap::const_iterator aIt( m_aChildMap.find( rParent ));
        if( aIt != m_aChildMap.end())
            return ! (aIt->second.empty());
    }
    return false;
}

ObjectHierarchy::tChildContainer ImplObjectHierarchy::getChildren( const OUString & rParent )
{
    if( rParent.getLength())
    {
        tChildMap::const_iterator aIt( m_aChildMap.find( rParent ));
        if( aIt != m_aChildMap.end())
            return aIt->second;
    }
    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tChildContainer ImplObjectHierarchy::getSiblings( const OUString & rNode )
{
    if( rNode.getLength() && !ObjectHierarchy::isRootNode( rNode ))
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

ObjectHierarchy::tCID ImplObjectHierarchy::getParentImpl(
    const ObjectHierarchy::tCID & rParentCID,
    const ObjectHierarchy::tCID & rCID )
{
    // search children
    ObjectHierarchy::tChildContainer aChildren( getChildren( rParentCID ));
    ObjectHierarchy::tChildContainer::const_iterator aIt(
        ::std::find( aChildren.begin(), aChildren.end(), rCID ));
    // recursion end
    if( aIt != aChildren.end())
        return rParentCID;

    for( aIt = aChildren.begin(); aIt != aChildren.end(); ++aIt )
    {
        // recursion
        ObjectHierarchy::tCID aTempParent( getParentImpl( *aIt, rCID ));
        if( aTempParent.getLength())
        {
            // exit on success
            return aTempParent;
        }
    }

    // exit on fail
    return ObjectHierarchy::tCID();
}

ObjectHierarchy::tCID ImplObjectHierarchy::getParent(
    const ObjectHierarchy::tCID & rCID )
{
    return getParentImpl( ObjectHierarchy::getRootNodeCID(), rCID );
}

} // namespace impl

ObjectHierarchy::ObjectHierarchy(
    const Reference< XChartDocument > & xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider /* = 0 */,
    bool bFlattenDiagram /* = false */ ) :
        m_apImpl( new impl::ImplObjectHierarchy( xChartDocument, pExplicitValueProvider, bFlattenDiagram ))
{}

ObjectHierarchy::~ObjectHierarchy()
{}

// static
ObjectHierarchy::tCID ObjectHierarchy::getRootNodeCID()
{
    return C2U("ROOT");
}

// static
bool ObjectHierarchy::isRootNode( const ObjectHierarchy::tCID & rCID )
{
    return rCID.equals( ObjectHierarchy::getRootNodeCID());
}

ObjectHierarchy::tChildContainer ObjectHierarchy::getTopLevelChildren() const
{
    return m_apImpl->getChildren( ObjectHierarchy::getRootNodeCID());
}

bool ObjectHierarchy::hasChildren( const tCID & rParent ) const
{
    return m_apImpl->hasChildren( rParent );
}

ObjectHierarchy::tChildContainer ObjectHierarchy::getChildren(
    const ObjectHierarchy::tCID & rParent ) const
{
    if( rParent.getLength())
        return m_apImpl->getChildren( rParent );

    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tChildContainer ObjectHierarchy::getSiblings(
    const ObjectHierarchy::tCID & rNode ) const
{
    if( rNode.getLength() && !isRootNode( rNode ))
        return m_apImpl->getSiblings( rNode );

    return ObjectHierarchy::tChildContainer();
}

ObjectHierarchy::tCID ObjectHierarchy::getParent(
    const ObjectHierarchy::tCID & rNode ) const
{
    return m_apImpl->getParent( rNode );
}

sal_Int32 ObjectHierarchy::getIndexInParent(
    const ObjectHierarchy::tCID & rNode ) const
{
    tCID aParentCID( m_apImpl->getParent( rNode ));
    tChildContainer aChildren( m_apImpl->getChildren( aParentCID ));
    tChildContainer::const_iterator aIt( aChildren.begin());
    for( sal_Int32 nIndex = 0; aIt != aChildren.end(); ++nIndex, ++aIt )
    {
        if( aIt->equals( rNode ))
            return nIndex;
    }
    return -1;
}

// ================================================================================

ObjectKeyNavigation::ObjectKeyNavigation(
    const ObjectHierarchy::tCID & rCurrentCID,
    const Reference< chart2::XChartDocument > & xChartDocument,
    ExplicitValueProvider * pExplicitValueProvider /* = 0 */ ) :
        m_aCurrentCID( rCurrentCID ),
        m_xChartDocument( xChartDocument ),
        m_pExplicitValueProvider( pExplicitValueProvider ),
        m_bStepDownInDiagram( true )
{
    if( m_aCurrentCID.getLength() == 0 )
        setCurrentSelection( ObjectHierarchy::getRootNodeCID());
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
            setCurrentSelection( OUString());
            bResult = true;
            break;
        default:
            bResult = false;
            break;
    }
    return bResult;
}

void ObjectKeyNavigation::setCurrentSelection( const ObjectHierarchy::tCID & rCID )
{
    m_aCurrentCID = rCID;
}

ObjectHierarchy::tCID ObjectKeyNavigation::getCurrentSelection() const
{
    return m_aCurrentCID;
}

bool ObjectKeyNavigation::first()
{
    ObjectHierarchy aHierarchy( m_xChartDocument, m_pExplicitValueProvider, m_bStepDownInDiagram );
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection()));
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
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection()));
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
    ObjectHierarchy::tChildContainer aSiblings( aHierarchy.getSiblings( getCurrentSelection()));
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
