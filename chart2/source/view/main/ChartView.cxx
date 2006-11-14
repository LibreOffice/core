/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartView.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:35:32 $
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
#include "ChartViewImpl.hxx"
#include "ViewDefines.hxx"
#include "VDiagram.hxx"
#include "VTitle.hxx"
#include "ShapeFactory.hxx"
#include "VCoordinateSystem.hxx"
#include "VSeriesPlotter.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "TitleHelper.hxx"
#include "LegendHelper.hxx"
#include "VLegend.hxx"
#include "PropertyMapper.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_EXPLICITSUBINCREMENT_HPP_
#include <com/sun/star/chart2/ExplicitSubIncrement.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPEGROUP_HPP_
#include <com/sun/star/chart2/XChartTypeGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XSTACKABLESCALEGROUP_HPP_
#include <com/sun/star/chart2/XStackableScaleGroup.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif

#ifndef _COM_SUN_STAR_LAYOUT_RELATIVEPOSITION_HPP_
#include <com/sun/star/layout/RelativePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_LAYOUT_RELATIVESIZE_HPP_
#include <com/sun/star/layout/RelativeSize.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

ChartView::~ChartView()
{
}

//static
ChartView* ChartView::createView(
                  const uno::Reference< uno::XComponentContext >& xCC
                , const uno::Reference< frame::XModel >& xChartModel
                , const uno::Reference<drawing::XDrawPagesSupplier>& xDrawPagesSuplier
                , NumberFormatterWrapper* pNumberFormatterWrapper )
{
    return new ChartViewImpl(xCC,xChartModel,xDrawPagesSuplier,pNumberFormatterWrapper);
}

ChartViewImpl::ChartViewImpl(
          const uno::Reference< uno::XComponentContext >& xCC
        , const uno::Reference< frame::XModel >& xChartModel
        , const uno::Reference<drawing::XDrawPagesSupplier>& xDrawPagesSuplier
        , NumberFormatterWrapper* pNumberFormatterWrapper )
    : m_xCC(xCC)
    , m_xChartModel(xChartModel)
    , m_xShapeFactory(NULL)
    , m_xDrawPage(NULL)
    , m_xDrawPages(NULL)
    , m_pNumberFormatterWrapper( pNumberFormatterWrapper )
{
    //get factory from draw model
    m_xShapeFactory = uno::Reference<lang::XMultiServiceFactory>( xDrawPagesSuplier, uno::UNO_QUERY );

    //create draw page:
    m_xDrawPages = xDrawPagesSuplier->getDrawPages ();
    m_xDrawPage = m_xDrawPages->insertNewByIndex ( 0 );
}

ChartViewImpl::~ChartViewImpl()
{
    m_xDrawPages->remove( m_xDrawPage );
    m_xDrawPage = NULL;

    //delete all coordinate systems
    ::std::vector< VCoordinateSystem* >::const_iterator       aIter = m_aVCooSysList.begin();
    const ::std::vector< VCoordinateSystem* >::const_iterator aEnd  = m_aVCooSysList.end();
    for( ; aIter != aEnd; aIter++ )
    {
        delete *aIter;
    }
    m_aVCooSysList.clear();
}

::basegfx::B3DHomMatrix createTransformationSceneToScreen(
    const awt::Point& rPos, const awt::Size& rSize )
{
    ::basegfx::B3DHomMatrix aM4;
    aM4.scale(double(rSize.Width)/FIXED_SIZE_FOR_3D_CHART_VOLUME
            , -double(rSize.Height)/FIXED_SIZE_FOR_3D_CHART_VOLUME, 1.0 );
    aM4.translate(double(rPos.X), double(rPos.Y+rSize.Height-1), 0);
    return aM4;
}

uno::Reference< drawing::XShapes > createDiagram(
              const uno::Reference< drawing::XShapes>& xPageShapes
            , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
            , const awt::Point& rPos, const awt::Size& rSize
            , sal_Int32 nDimension
            , const uno::Reference< XDiagram > & xDia
             )
{
    VDiagram aVDiagram(xDia, nDimension);
    aVDiagram.init(xPageShapes,xPageShapes,xShapeFactory);
    aVDiagram.createShapes(rPos,rSize);
    uno::Reference< drawing::XShapes > xTarget = aVDiagram.getCoordinateRegion();
    return xTarget;
}

void getCoordinateOrigin( double* fCoordinateOrigin, const uno::Reference< XBoundedCoordinateSystem >& xCoordSys )
{
    if(xCoordSys.is())
    try
    {
        uno::Sequence< uno::Any > aCoord( xCoordSys->getOrigin());
        if( aCoord.getLength() >= 1 )
            aCoord[0]>>=fCoordinateOrigin[0];
        if( aCoord.getLength() >= 2 )
            aCoord[1]>>=fCoordinateOrigin[1];
        if( aCoord.getLength() >= 3 )
            aCoord[2]>>=fCoordinateOrigin[2];
    }
    catch( uno::Exception& ex)
    {
        ASSERT_EXCEPTION( ex );
    }
}

sal_Int32 getDimension( const uno::Reference< XDiagram >& xDiagram )
{
    return ChartTypeHelper::getDimensionCount(
        ChartModelHelper::getFirstChartType( xDiagram ) );
}

// void getCoordinateSystems( std::vector< VCoordinateSystem >& rVCooSysList, const uno::Reference< XDiagram >& xDiagram )
// {
//     uno::Reference< XBoundedCoordinateSystemContainer > xCooSysContainer =
//         uno::Reference< XBoundedCoordinateSystemContainer >::query( xDiagram );
//     if(xCooSysContainer.is())
//     {
//         uno::Sequence< uno::Reference< XBoundedCoordinateSystem > > aXCooSysList = xCooSysContainer->getCoordinateSystems();
//         for( sal_Int32 nC=0; nC < aXCooSysList.getLength(); nC++)
//         {
//             VCoordinateSystem aCooSys(aXCooSysList[nC]);

//             double fCoordinateOrigin[3] = { 0.0, 0.0, 0.0 };
//             getCoordinateOrigin( fCoordinateOrigin, aXCooSysList );
//             aCooSys.setOrigin(fCoordinateOrigin);

//             rVCooSysList.push_back( aCooSys );
//         }
//         if(!aXCooSysList.getLength())
//         {
//             //get coosys from diagram tree
//                 //...
//         }
//     }
// }

const VCoordinateSystem* findInCooSysList( const std::vector< VCoordinateSystem* >& rVCooSysList
                                    , const uno::Reference< XBoundedCoordinateSystem >& xCooSys )
{
    for( size_t nC=0; nC < rVCooSysList.size(); nC++)
    {
        const VCoordinateSystem* pVCooSys = rVCooSysList[nC];
        if(pVCooSys->getModel()==xCooSys)
            return pVCooSys;
    }
    return NULL;
}

void addCooSysToList( std::vector< VCoordinateSystem* >& rVCooSysList
            , const uno::Reference< XBoundedCoordinateSystem >& xCooSys
            , double fCoordinateOrigin [] )
{
    if( !findInCooSysList( rVCooSysList, xCooSys ) )
    {
        VCoordinateSystem* pVCooSys( VCoordinateSystem::createCoordinateSystem(xCooSys ) );
        if(pVCooSys)
        {
            pVCooSys->setOrigin(fCoordinateOrigin);
            rVCooSysList.push_back( pVCooSys );
        }
    }
}

void getAxesAndAddToCooSys( uno::Sequence< uno::Reference< XAxis > >& rAxisList
                             , const uno::Reference< XDiagram >& xDiagram
                             , std::vector< VCoordinateSystem* >& rVCooSysList )
{
    uno::Reference< XAxisContainer > xAxisContainer( xDiagram, uno::UNO_QUERY );
    if( xAxisContainer.is())
    {
        rAxisList = xAxisContainer->getAxes();
        for( sal_Int32 nA = 0; nA < rAxisList.getLength(); nA++ )
        {
            uno::Reference< XAxis > xAxis( rAxisList[nA] );
            for( size_t nC=0; nC < rVCooSysList.size(); nC++)
            {
                if(xAxis->getCoordinateSystem() == rVCooSysList[nC]->getModel() )
                {
                    rVCooSysList[nC]->addAxis( xAxis );
                }
            }
        }
    }
}

void addGridsToCooSys(  const uno::Reference< XDiagram >& xDiagram
                             , std::vector< VCoordinateSystem* >& rVCooSysList )
{
    uno::Reference< XGridContainer > xGridContainer( xDiagram, uno::UNO_QUERY );
    if( xGridContainer.is())
    {
        uno::Sequence< uno::Reference< XGrid > > aGridList(
                                        xGridContainer->getGrids() );
        for( sal_Int32 nA = 0; nA < aGridList.getLength(); nA++ )
        {
            uno::Reference< XGrid > xGrid( aGridList[nA] );
            for( size_t nC=0; nC < rVCooSysList.size(); nC++)
            {
                if(xGrid->getCoordinateSystem() == rVCooSysList[nC]->getModel() )
                {
                    rVCooSysList[nC]->addGrid( xGrid );
                }
            }
        }
    }
}

void addSeriesToPlotter( const uno::Sequence< uno::Reference< XDataSeriesTreeNode > >& rSeriesList
                        , VSeriesPlotter* pPlotter
                        , StackMode eYStackMode )
{
    if(!pPlotter)
        return;
    for( sal_Int32 nS = 0; nS < rSeriesList.getLength(); ++nS )
    {
        uno::Reference< XDataSeries > xDataSeries( rSeriesList[nS], uno::UNO_QUERY );
        if(!xDataSeries.is())
            continue;
        VDataSeries* pTestSeries = new VDataSeries( xDataSeries );
        //virtual void addSeries( VDataSeries* pSeries, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );
        sal_Int32 nXSlot2 = 0;
        if(eYStackMode==StackMode_NONE)
            nXSlot2=-1;
        //@todo
        pPlotter->addSeries( pTestSeries, nXSlot2 );
    // pPlotter->addSeries( pTestSeries, nXSlot2, nYSlot );
        /*
        if(nN==nSeriesCount-1)
            pPlotter->addSeries( pTestSeries, -1 );
        else
            pPlotter->addSeries( pTestSeries, 0 );
        */
    }
}
void initializeDiagramAndGetCooSys( std::vector< VCoordinateSystem* >& rVCooSysList
            , const uno::Reference< uno::XComponentContext>& xCC
            , const uno::Reference< drawing::XShapes>& xPageShapes
            , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
            , NumberFormatterWrapper* pNumberFormatterWrapper
            , const awt::Point& rPos, const awt::Size& rSize
            , const uno::Reference< frame::XModel >& xChartModel )
{
    //------------ get model series from model
    uno::Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    if( !xDiagram.is())
        return;

    sal_Int32 nDimension = getDimension( xDiagram );

    //------------ create Diagram shapes
    uno::Reference< drawing::XShapes > xTarget = createDiagram( xPageShapes, xShapeFactory, rPos, rSize, nDimension, xDiagram );

    //------------ get all coordinatesystems
//     getCoordinateSystems( rVCooSysList, xDiagram );

    //------------ add series to plotter and thus prepare him for providing minimum and maximum values
    uno::Reference< XDataSeriesTreeParent > xTree = xDiagram->getTree();
    uno::Sequence< uno::Reference< XDataSeriesTreeNode > >  aChartTypes( xTree->getChildren() );
    for( sal_Int32 i = 0; i < aChartTypes.getLength(); ++i )
    {
        //iterate through different charttypes:

        uno::Reference< XChartTypeGroup > xChartTypeGroup( aChartTypes[i], uno::UNO_QUERY );
        DBG_ASSERT(xChartTypeGroup.is(),"First node at the diagram tree needs to be a ChartTypeGroup");
        if( !xChartTypeGroup.is() )
            continue;
        ::std::auto_ptr< VSeriesPlotter > apPlotter( VSeriesPlotter::createSeriesPlotter( xChartTypeGroup->getChartType() ) );

        //------------ add series to plotter and thus prepare him for providing minimum and maximum values

        sal_Int32 nXSlot = -1;
        sal_Int32 nYSlot = -1;
        uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aXSlots( xChartTypeGroup->getChildren() );
        for( sal_Int32 nX = 0; nX < aXSlots.getLength(); ++nX )
        {
            uno::Reference< XDataSeriesTreeParent > xXSlot( aXSlots[nX], uno::UNO_QUERY );
            DBG_ASSERT( xXSlot.is(), "a node for the first dimension of a chart tree should always be a parent" );
            if(!xXSlot.is())
                continue;
            uno::Reference< XStackableScaleGroup > xStackGroup( xXSlot, uno::UNO_QUERY );
            if( xStackGroup.is() && xStackGroup->getStackMode()==StackMode_STACKED)
                nXSlot++;
            uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aYSlots( xXSlot->getChildren() );
            for( sal_Int32 nY = 0; nY < aYSlots.getLength(); ++nY )
            {
                uno::Reference< XDataSeriesTreeParent > xYSlot( aYSlots[nY], uno::UNO_QUERY );
                DBG_ASSERT( xYSlot.is(), "a node for the second dimension of a chart tree should always be a parent" );
                if(!xYSlot.is())
                    continue;
                xStackGroup.set( uno::Reference< XStackableScaleGroup >::query( xYSlot ));
                StackMode aYStackMode = StackMode_NONE;
                if(xStackGroup.is())
                    aYStackMode = xStackGroup->getStackMode();
                if(aYStackMode==StackMode_STACKED)
                    nYSlot++;
                if( 2 == nDimension )
                {
                    uno::Reference< XScaleGroup > xScaleGroup( xYSlot, uno::UNO_QUERY );
                    {
                        double fCoordinateOrigin[3] = { 0.0, 0.0, 0.0 };
                        getCoordinateOrigin( fCoordinateOrigin, xScaleGroup->getCoordinateSystem() );
                        addCooSysToList(rVCooSysList,xScaleGroup->getCoordinateSystem(),fCoordinateOrigin);
                    }
                    addSeriesToPlotter( xYSlot->getChildren(), apPlotter.get(), aYStackMode );
                 }
                else
                {
                    uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aZSlots( xYSlot->getChildren() );
                    for( sal_Int32 nZ = 0; nZ < aZSlots.getLength(); ++nZ )
                    {
                        uno::Reference< XScaleGroup > xScaleGroup( aZSlots[nZ], uno::UNO_QUERY );
                        if(xScaleGroup.is())
                        {
                            double fCoordinateOrigin[3] = { 0.0, 0.0, 0.0 };
                            getCoordinateOrigin( fCoordinateOrigin, xScaleGroup->getCoordinateSystem() );
                            addCooSysToList(rVCooSysList,xScaleGroup->getCoordinateSystem(),fCoordinateOrigin);
                        }
                        uno::Reference< XDataSeriesTreeParent > xZSlot( aZSlots[nZ], uno::UNO_QUERY );
                        DBG_ASSERT( xZSlot.is(), "a node for the third dimension of a 3 dimensional chart tree should always be a parent" );
                        if(!xZSlot.is())
                            continue;
                        addSeriesToPlotter( xZSlot->getChildren(), apPlotter.get(), aYStackMode );
                    }
                }
            }
        }

        //------------ get all axes from model and add to VCoordinateSystems
        uno::Sequence< uno::Reference< XAxis > > aAxisList;
        getAxesAndAddToCooSys( aAxisList, xDiagram, rVCooSysList );
        addGridsToCooSys( xDiagram, rVCooSysList );

        //------------ iterate through all coordinate systems
        for( size_t nC=0; nC < rVCooSysList.size(); nC++)
        {
            //------------ create explicit scales and increments
            VCoordinateSystem* pVCooSys = rVCooSysList[nC];
            pVCooSys->doAutoScale( apPlotter.get() );

            ::basegfx::B3DHomMatrix aM4_SceneToScreen( createTransformationSceneToScreen(rPos,rSize) );
            drawing::HomogenMatrix aHM_SceneToScreen( Matrix4DToHomogenMatrix(aM4_SceneToScreen) );

            pVCooSys->initPlottingTargets(xTarget,xPageShapes,xShapeFactory);
            pVCooSys->setTransformationSceneToScreen(aHM_SceneToScreen);

            //------------ create axes and grids --- @todo do auto layout / fontscaling
            pVCooSys->createAxesShapes( rSize, pNumberFormatterWrapper );
            pVCooSys->createGridShapes();

            //------------ set scale to plotter / create series
            if(apPlotter.get())
            {
                apPlotter->init(xTarget,xPageShapes,xShapeFactory);
                if(2==nDimension)
                    apPlotter->setTransformationSceneToScreen( aHM_SceneToScreen );
                apPlotter->setScales( pVCooSys->getExplicitScales() );
                apPlotter->createShapes();
            }
        }
    }
}

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------

void ChartViewImpl::getExplicitValuesForMeter(
                     uno::Reference< XMeter > xMeter
                     , ExplicitScaleData&  rExplicitScale
                     , ExplicitIncrementData& rExplicitIncrement
                     , double& rfExplicitOrigin )
{
    if(!xMeter.is())
        return;
    uno::Reference< XBoundedCoordinateSystem > xCooSys = xMeter->getCoordinateSystem();
    const VCoordinateSystem* pVCooSys = findInCooSysList(m_aVCooSysList,xCooSys);
    if(!pVCooSys)
        return;
    const uno::Sequence< ExplicitScaleData >&     rScales      =pVCooSys->getExplicitScales();
    const uno::Sequence< ExplicitIncrementData >& rIncrements  =pVCooSys->getExplicitIncrements();
    sal_Int32 nDim = xMeter->getRepresentedDimension();
    if(nDim<=rScales.getLength())
        rExplicitScale=rScales[nDim];
    if(nDim<=rIncrements.getLength())
        rExplicitIncrement=rIncrements[nDim];
    rfExplicitOrigin = pVCooSys->getOriginByDimension( nDim );
}

double lcl_getPageLayoutDistancePercentage()
{
    return 0.02;
}

bool getPosAndSizeForDiagram(
    awt::Point& rOutPos, awt::Size& rOutSize
    , const awt::Rectangle& rSpaceLeft
    , const awt::Size & rPageSize
    , const uno::Reference< XDiagram > & xDiagram )
{
    //@todo: we need a size dependent on the axis labels
    awt::Rectangle aRemainingSpace(rSpaceLeft);
    {
        sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
        sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
        aRemainingSpace.X+=nXDistance;
        aRemainingSpace.Width-=2*nXDistance;
        aRemainingSpace.Y+=nYDistance;
        aRemainingSpace.Height-=2*nYDistance;
    }
    if(aRemainingSpace.Width <= 0 || aRemainingSpace.Height <= 0 )
        return false;

    //long nHeight = aRemainingSpace.Height * 5 / 6;
    // (1 - 5/6) / 2 = 1/12
    //long nOffsetY = aRemainingSpace.Y + aRemainingSpace.Height / 12;
    long nHeight = aRemainingSpace.Height * 11 / 12;
    long nOffsetY = aRemainingSpace.Y;

    long nWidth = aRemainingSpace.Width * 5 / 6;
    // (1 - 5/6) / 2 = 1/12
    long nOffsetX = aRemainingSpace.X + aRemainingSpace.Width / 12;

    if( nHeight <= 0 || nWidth <= 0 )
        return false;

    uno::Reference< beans::XPropertySet > xProp(xDiagram, uno::UNO_QUERY);

    //size:
    ::com::sun::star::layout::RelativeSize aRelativeSize;
    if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativeSize" ) )>>=aRelativeSize) )
    {
        rOutSize.Height = static_cast<sal_Int32>(aRelativeSize.Secondary*rPageSize.Height);
        rOutSize.Width = static_cast<sal_Int32>(aRelativeSize.Primary*rPageSize.Width);
    }
    else
        rOutSize = awt::Size(nWidth,nHeight);

    //position:
    ::com::sun::star::layout::RelativePosition aRelativePosition;
    if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativePosition" ) )>>=aRelativePosition) )
    {
        //@todo decide wether x is primary or secondary

        //the anchor point at the page is in the middle of the page
        double fX = rPageSize.Width/2.0+aRelativePosition.Primary*rPageSize.Width;
        double fY = rPageSize.Height/2.0+aRelativePosition.Secondary*rPageSize.Height;

        //the anchor point at the diagram object is in the middle
        fY -= rOutSize.Height/2;
        fX -= rOutSize.Width/2;

        rOutPos.X = static_cast<sal_Int32>(fX);
        rOutPos.Y = static_cast<sal_Int32>(fY);
    }
    else
        rOutPos = awt::Point(nOffsetX,nOffsetY);

    return true;
}

enum TitleAlignment { ALIGN_LEFT, ALIGN_TOP, ALIGN_RIGHT, ALIGN_BOTTOM, ALIGN_Z };

void changePositionOfAxisTitle( VTitle* pVTitle, TitleAlignment eAlignment
                               , awt::Rectangle& rDiagramPlusAxesRect, const awt::Size & rPageSize )
{
    if(!pVTitle)
        return;

    awt::Point aNewPosition(0,0);
    awt::Size aTitleSize = pVTitle->getFinalSize();
    sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
    sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
    switch( eAlignment )
    {
    case ALIGN_BOTTOM:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width/2
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height + aTitleSize.Height/2  + nYDistance );
        break;
    case ALIGN_LEFT:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X - aTitleSize.Width/2 - nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height/2 );
        break;
    case ALIGN_Z:
        aNewPosition = awt::Point( rDiagramPlusAxesRect.X + rDiagramPlusAxesRect.Width + aTitleSize.Width/2 + nXDistance
                                    , rDiagramPlusAxesRect.Y + rDiagramPlusAxesRect.Height - aTitleSize.Height/2 );
       break;
    default:
        break;
    }

    pVTitle->changePosition( aNewPosition );
}

std::auto_ptr<VTitle> createTitle( const uno::Reference< XTitle >& xTitle
                , const uno::Reference< drawing::XShapes>& xPageShapes
                , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                , awt::Rectangle& rRemainingSpace
                , const awt::Size & rPageSize
                , TitleAlignment eAlignment
                , bool& rbAutoPosition )
{
    std::auto_ptr<VTitle> apVTitle;
    if(xTitle.is())
    {
        //create title
        double fAdditionalRotationAngleDegree = 0.0;
        if( ALIGN_LEFT==eAlignment)
            fAdditionalRotationAngleDegree = 90.0;

        apVTitle = std::auto_ptr<VTitle>(new VTitle(xTitle,fAdditionalRotationAngleDegree));
        apVTitle->init(xPageShapes,xShapeFactory);
        apVTitle->createShapes( awt::Point(0,0), rPageSize );
        awt::Size aTitleSize = apVTitle->getFinalSize();

        //position
        rbAutoPosition=true;
        awt::Point aNewPosition(0,0);
        sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
        sal_Int32 nXDistance = static_cast<sal_Int32>(rPageSize.Width*lcl_getPageLayoutDistancePercentage());
        ::com::sun::star::layout::RelativePosition aRelativePosition;
        uno::Reference< beans::XPropertySet > xProp(xTitle, uno::UNO_QUERY);
        if( xProp.is() && (xProp->getPropertyValue( C2U( "RelativePosition" ) )>>=aRelativePosition) )
        {
            rbAutoPosition = false;

            //@todo decide wether x is primary or secondary
            aNewPosition.X = static_cast<sal_Int32>(aRelativePosition.Primary*rPageSize.Width);
            aNewPosition.Y = static_cast<sal_Int32>(aRelativePosition.Secondary*rPageSize.Height);

            //the anchor point at the title object is top/middle
            aNewPosition.Y += aTitleSize.Height/2;
        }
        else //auto position
        {
            switch( eAlignment )
            {
            case ALIGN_TOP:
                aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width/2
                                         , rRemainingSpace.Y + aTitleSize.Height/2 + nYDistance );
                break;
            case ALIGN_BOTTOM:
                aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width/2
                                         , rRemainingSpace.Y + rRemainingSpace.Height - aTitleSize.Height/2 - nYDistance );
                break;
            case ALIGN_LEFT:
                aNewPosition = awt::Point( rRemainingSpace.X + aTitleSize.Width/2 + nXDistance
                                         , rRemainingSpace.Y + rRemainingSpace.Height/2 );
                break;
            case ALIGN_RIGHT:
                aNewPosition = awt::Point( rRemainingSpace.X + rRemainingSpace.Width - aTitleSize.Width/2 - nXDistance
                                         , rRemainingSpace.Y + rRemainingSpace.Height/2 );
                break;
            default:
                break;

            }
        }
        apVTitle->changePosition( aNewPosition );

        //remaining space
        switch( eAlignment )
        {
            case ALIGN_TOP:
                rRemainingSpace.Y += ( aTitleSize.Height + nYDistance );
                rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
                break;
            case ALIGN_BOTTOM:
                rRemainingSpace.Height -= ( aTitleSize.Height + nYDistance );
                break;
            case ALIGN_LEFT:
                rRemainingSpace.X += ( aTitleSize.Width + nXDistance );
                rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
                break;
            case ALIGN_RIGHT:
                rRemainingSpace.Width -= ( aTitleSize.Width + nXDistance );
                break;
            default:
                break;
        }
    }
    return apVTitle;
}

bool createLegend( const uno::Reference< XLegend > & xLegend
                   , const uno::Reference< drawing::XShapes>& xPageShapes
                   , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                   , awt::Rectangle & rOutSpaceLeft
                   , const awt::Size & rPageSize
                   , const uno::Reference< frame::XModel > & xModel )
{
    if( VLegend::isVisible( xLegend ))
    {
        VLegend aVLegend( xLegend );
        aVLegend.init( xPageShapes, xShapeFactory, xModel );
        aVLegend.createShapes( awt::Size( rOutSpaceLeft.Width, rOutSpaceLeft.Height ),
                               rPageSize );
        aVLegend.changePosition( rOutSpaceLeft, rPageSize );
        return true;
    }
    return false;
}

void formatPage(
      const uno::Reference< frame::XModel > & xModel
    , const awt::Size rPageSize
    , const uno::Reference< drawing::XShapes >& xTarget
    , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
    )
{
    try
    {
        uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
        OSL_ASSERT( xChartDoc.is());
        if( ! xChartDoc.is())
            return;
        uno::Reference< beans::XPropertySet > xModelPage( xChartDoc->getPageBackground());
        if( ! xModelPage.is())
            return;

        uno::Reference< beans::XPropertySet > xPageProp( xTarget, uno::UNO_QUERY );
        // the following is just a workaround as the draw page is no XPropertySet
        // ------------ workaround
        if( ! xPageProp.is() )
        {
            // if we get here, we need a shape to place on the page
            if( xShapeFactory.is())
            {
                uno::Reference< drawing::XShape > xShape(
                    xShapeFactory->createInstance(
                        C2U( "com.sun.star.drawing.RectangleShape" )), uno::UNO_QUERY );
                if( xTarget.is() &&
                    xShape.is())
                {
                    xTarget->add( xShape );
                    xShape->setSize( rPageSize );
                    xPageProp.set( xShape, uno::UNO_QUERY );
                    if( xPageProp.is())
                        xPageProp->setPropertyValue( C2U("LineStyle"), uno::makeAny( drawing::LineStyle_NONE ));
                }
            }
        }
        // ------------ workaround

        if( xPageProp.is())
        {
            tPropertyNameValueMap aNameValueMap;
            tMakePropertyNameMap aNameMap = PropertyMapper::getPropertyNameMapForFillProperties();
            const tMakePropertyNameMap& rLineNameMap = PropertyMapper::getPropertyNameMapForLineProperties();
            aNameMap.insert( rLineNameMap.begin(), rLineNameMap.end());
            PropertyMapper::getValueMap( aNameValueMap, aNameMap, xModelPage );

            tNameSequence aNames;
            tAnySequence aValues;
            PropertyMapper::getMultiPropertyListsFromValueMap( aNames, aValues, aNameValueMap );
            PropertyMapper::setMultiProperties( aNames, aValues, xPageProp );
        }
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

bool ChartViewImpl::create( const awt::Size& rPageSize )
{
    uno::Reference<drawing::XShapes> xPageShapes =
        uno::Reference<drawing::XShapes>( m_xDrawPage, uno::UNO_QUERY );

    //------------ apply fill properties to page
    // todo: it would be nicer to just pass the page m_xDrawPage and format it,
    // but the page we have here does not support XPropertySet
    formatPage( m_xChartModel, rPageSize, xPageShapes, m_xShapeFactory );

    //sal_Int32 nYDistance = static_cast<sal_Int32>(rPageSize.Height*lcl_getPageLayoutDistancePercentage());
    awt::Rectangle aRemainingSpace( 0, 0, rPageSize.Width, rPageSize.Height );

    //create the group shape for diagram and axes first to have title and legends on top of it
    uno::Reference< drawing::XShapes > xDiagramPlusAxesGroup_Shapes = ShapeFactory(m_xShapeFactory).createGroup2D(xPageShapes,C2U("CID/Diagram=XXX_CID"));//@todo read CID from model

    //------------ create some titles
    std::auto_ptr<VTitle> apVTitle(0);
    bool bAutoPositionDummy;

    //------------ create main title shape
    createTitle( TitleHelper::getTitle( TitleHelper::MAIN_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory
                , aRemainingSpace, rPageSize, ALIGN_TOP, bAutoPositionDummy );
    if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
        return true;

    //------------ create sub title shape
    createTitle( TitleHelper::getTitle( TitleHelper::SUB_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory
                , aRemainingSpace, rPageSize, ALIGN_TOP, bAutoPositionDummy );
    if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
        return true;

    //------------ create legend
    createLegend( LegendHelper::getLegend( m_xChartModel ), xPageShapes, m_xShapeFactory
                , aRemainingSpace, rPageSize, m_xChartModel );
    if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
        return true;

    //------------ create x axis title
    bool bAutoPosition_XTitle;
    std::auto_ptr<VTitle> apVTitle_X( createTitle( TitleHelper::getTitle( TitleHelper::X_AXIS_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory
                , aRemainingSpace, rPageSize, ALIGN_BOTTOM, bAutoPosition_XTitle) );
    if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
        return true;

    //------------ create y axis title
    bool bAutoPosition_YTitle;
    std::auto_ptr<VTitle> apVTitle_Y( createTitle( TitleHelper::getTitle( TitleHelper::Y_AXIS_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory
                , aRemainingSpace, rPageSize, ALIGN_LEFT, bAutoPosition_YTitle) );
    if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
        return true;

    //------------ create z axis title
    bool bAutoPosition_ZTitle;
    std::auto_ptr<VTitle> apVTitle_Z( createTitle( TitleHelper::getTitle( TitleHelper::Z_AXIS_TITLE, m_xChartModel ), xPageShapes, m_xShapeFactory
                , aRemainingSpace, rPageSize, ALIGN_RIGHT, bAutoPosition_ZTitle) );
    if(aRemainingSpace.Width<=0||aRemainingSpace.Height<=0)
        return true;

    //------------ create complete diagram shape (inclusive axis and series)
    awt::Point aPosDia;
    awt::Size  aSizeDia;
    if( getPosAndSizeForDiagram( aPosDia, aSizeDia, aRemainingSpace, rPageSize, ChartModelHelper::findDiagram( m_xChartModel ) ) )
    {
        //create diagram and all its content
        initializeDiagramAndGetCooSys( m_aVCooSysList
                    , m_xCC, xDiagramPlusAxesGroup_Shapes, m_xShapeFactory, m_pNumberFormatterWrapper
                    , aPosDia ,aSizeDia, m_xChartModel );

        //correct axis title position
        uno::Reference< drawing::XShape > xDiagramPlusAxesGroup_Shape( xDiagramPlusAxesGroup_Shapes, uno::UNO_QUERY );
        awt::Point aPos = xDiagramPlusAxesGroup_Shape->getPosition();
        awt::Size aSize = xDiagramPlusAxesGroup_Shape->getSize();
        awt::Rectangle aRect(aPos.X,aPos.Y,aSize.Width,aSize.Height);
        awt::Rectangle aDiagramPlusAxesRect(aPos.X,aPos.Y,aSize.Width,aSize.Height);
        if(bAutoPosition_XTitle)
            changePositionOfAxisTitle( apVTitle_X.get(), ALIGN_BOTTOM, aDiagramPlusAxesRect, rPageSize );
        if(bAutoPosition_YTitle)
            changePositionOfAxisTitle( apVTitle_Y.get(), ALIGN_LEFT, aDiagramPlusAxesRect, rPageSize );
        if(bAutoPosition_ZTitle)
            changePositionOfAxisTitle( apVTitle_Z.get(), ALIGN_Z, aDiagramPlusAxesRect, rPageSize );
    }
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
