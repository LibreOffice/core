/*************************************************************************
 *
 *  $RCSfile: ChartView.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: iha $ $Date: 2003-10-28 18:17:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "ChartViewImpl.hxx"
#include "PlottingPositionHelper.hxx"
#include "ViewDefines.hxx"
#include "VDiagram.hxx"
#include "VTitle.hxx"
#include "ShapeFactory.hxx"
#include "VAxis.hxx"
#include "VSeriesPlotter.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "TitleHelper.hxx"
#include "LegendHelper.hxx"
#include "VLegend.hxx"
#include "PropertyMapper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_AXISORIENTATION_HPP_
#include <drafts/com/sun/star/chart2/AxisOrientation.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_EXPLICITSUBINCREMENT_HPP_
#include <drafts/com/sun/star/chart2/ExplicitSubIncrement.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTTYPEGROUP_HPP_
#include <drafts/com/sun/star/chart2/XChartTypeGroup.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <drafts/com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XSTACKABLESCALEGROUP_HPP_
#include <drafts/com/sun/star/chart2/XStackableScaleGroup.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <drafts/com/sun/star/chart2/XTitled.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

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
    , m_pNumberFormatterWrapper( pNumberFormatterWrapper )
    , m_pVDiagram(NULL)
    , m_pVLegend(NULL)
    , m_pVTitle(NULL)
    , m_pVSubtitle(NULL)
{
    //get factory from draw model
    m_xShapeFactory = uno::Reference<lang::XMultiServiceFactory>( xDrawPagesSuplier, uno::UNO_QUERY );

    //create draw page:
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSuplier->getDrawPages ();
    m_xDrawPage = xDrawPages->insertNewByIndex ( 0 );
}

ChartViewImpl::~ChartViewImpl()
{
    m_xDrawPage = NULL;
}

Matrix4D createTransformationSceneToScreen(
    const awt::Point& rPos, const awt::Size& rSize )
{
    Matrix4D aM4;
    aM4.Scale(double(rSize.Width)/FIXED_SIZE_FOR_3D_CHART_VOLUME
            , -double(rSize.Height)/FIXED_SIZE_FOR_3D_CHART_VOLUME, 1.0 );
    aM4.Translate(double(rPos.X), double(rPos.Y+rSize.Height-1), 0);
    return aM4;
}

drawing::HomogenMatrix createTransformationMatrix()
{
    Matrix4D aM4;
    aM4.RotateY( -ZDIRECTION*F_PI/9.0 );
    aM4.RotateX( ZDIRECTION*F_PI/10.0 );

    //aM4.RotateY( ZDIRECTION*F_PI/2.0 );
    //aM4.RotateX( -ZDIRECTION*F_PI/2.0 );

    //aM4.RotateX( -ZDIRECTION*F_PI/2.0 );

    aM4.Translate(0, -FIXED_SIZE_FOR_3D_CHART_VOLUME/2.0, 0);
//  aM4.Scale(1, 1, ZDIRECTION); //invert direction of z coordinate to get a left handed system
    aM4.Scale(1.0, 1.0, ZDIRECTION);
    drawing::HomogenMatrix aHM = Matrix4DToHomogenMatrix(aM4);
    return aHM;
}

ShapeAppearance getDefaultStyle( sal_Int32 nStyle )
{
    switch(nStyle%8)
    {
        case 1:
            return ShapeAppearance( 0, 0, GEOMETRY_CYLINDER, SYMBOL_DIAMOND );
        case 2:
            return ShapeAppearance( 0, 0, GEOMETRY_PYRAMID, SYMBOL_ARROW_DOWN );
        case 3:
            return ShapeAppearance( 0, 0, GEOMETRY_CONE, SYMBOL_ARROW_UP );
        case 4:
            return ShapeAppearance( 0, 0, GEOMETRY_CUBOID, SYMBOL_ARROW_RIGHT );
        case 5:
            return ShapeAppearance( 0, 0, GEOMETRY_CYLINDER, SYMBOL_ARROW_LEFT );
        case 6:
            return ShapeAppearance( 0, 0, GEOMETRY_PYRAMID, SYMBOL_BOWTIE );
        case 7:
            return ShapeAppearance( 0, 0, GEOMETRY_CONE, SYMBOL_SANDGLASS );
        default:
            return ShapeAppearance( 0, 0, GEOMETRY_CUBOID, SYMBOL_SQUARE );
    }
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
    if(3==nDimension)
        aVDiagram.setSceneMatrix( createTransformationMatrix() );
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
    sal_Int32 nDimension = 2;

    //@todo maybe get the dimension better from diagram properties ... -> need model change
    uno::Reference< XDataSeriesTreeParent > xTree = xDiagram->getTree();
    if(!xTree.is())
        return nDimension;
    uno::Sequence< uno::Reference< XDataSeriesTreeNode > >  aChartTypes( xTree->getChildren() );
    for( sal_Int32 i = 0; i < aChartTypes.getLength(); ++i )
    {
        uno::Reference< XChartTypeGroup > xChartTypeGroup( aChartTypes[i], uno::UNO_QUERY );
        DBG_ASSERT(xChartTypeGroup.is(),"First node at the diagram tree needs to be a ChartTypeGroup");
        if( !xChartTypeGroup.is() )
            continue;
        uno::Reference< XChartType > xChartType = xChartTypeGroup->getChartType();
        if( !xChartType.is() )
            continue;
        uno::Reference< beans::XPropertySet > xChartTypeProp( xChartType, uno::UNO_QUERY );
        if( xChartTypeProp.is())
        {
            try
            {
                if( !(xChartTypeProp->getPropertyValue( C2U( "Dimension" )) >>= nDimension) )
                {
                    DBG_ERROR( "Couldn't get Dimension from ChartTypeGroup" );
                }
            }
            catch( beans::UnknownPropertyException ex )
            {
                ASSERT_EXCEPTION( ex );
            }
            break;
        }
    }
    return nDimension;
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

const VCoordinateSystem* findInCooSysList( const std::vector< VCoordinateSystem >& rVCooSysList
                                    , const uno::Reference< XBoundedCoordinateSystem >& xCooSys )
{
    for( size_t nC=0; nC < rVCooSysList.size(); nC++)
    {
        const VCoordinateSystem& rVCooSys = rVCooSysList[nC];
        if(rVCooSys.getModel()==xCooSys)
            return &rVCooSys;
    }
    return NULL;
}

void addCooSysToList( std::vector< VCoordinateSystem >& rVCooSysList
            , const uno::Reference< XBoundedCoordinateSystem >& xCooSys
            , double fCoordinateOrigin [] )
{
    if( !findInCooSysList( rVCooSysList, xCooSys ) )
    {
        VCoordinateSystem aVCooSys(xCooSys);
        aVCooSys.setOrigin(fCoordinateOrigin);

        rVCooSysList.push_back( aVCooSys );
    }
}

void getAxesAndAddToCooSys( uno::Sequence< uno::Reference< XAxis > >& rAxisList
                             , const uno::Reference< XDiagram >& xDiagram
                             , std::vector< VCoordinateSystem >& rVCooSysList )
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
                if(xAxis->getCoordinateSystem() == rVCooSysList[nC].getModel() )
                {
                    rVCooSysList[nC].addAxis( xAxis );
                }
            }
        }
    }
}

void addGridsToCooSys(  const uno::Reference< XDiagram >& xDiagram
                             , std::vector< VCoordinateSystem >& rVCooSysList )
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
                if(xGrid->getCoordinateSystem() == rVCooSysList[nC].getModel() )
                {
                    rVCooSysList[nC].addGrid( xGrid );
                }
            }
        }
    }
}

void initializeDiagramAndGetCooSys( std::vector< VCoordinateSystem >& rVCooSysList
            , const uno::Reference< uno::XComponentContext>& xCC
            , const uno::Reference< drawing::XShapes>& xPageShapes
            , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
            , NumberFormatterWrapper* pNumberFormatterWrapper
            , const awt::Point& rPos, const awt::Size& rSize
            , const uno::Reference< frame::XModel >& xModel )
{
    //------------ get model series from model
    uno::Reference< XChartDocument >        xChartDocument( xModel, uno::UNO_QUERY );
    if( !xChartDocument.is() )
        return;
    uno::Reference< XDiagram >              xDiagram = xChartDocument->getDiagram();
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
        ::std::auto_ptr< VSeriesPlotter > apPlotter( VSeriesPlotter::createSeriesPlotter( xChartTypeGroup->getChartType()->getChartType(), nDimension ) );

        //------------ add series to plotter and thus prepare him for providing minimum and maximum values

        sal_Int32 nXSlot = -1;
        sal_Int32 nYSlot = -1;
        uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aXSlots( xChartTypeGroup->getChildren() );
        for( sal_Int32 nX = 0; nX < aXSlots.getLength(); ++nX )
        {
            uno::Reference< XDataSeriesTreeParent > xXSlot = uno::Reference< XDataSeriesTreeParent >::query( aXSlots[nX] );
            DBG_ASSERT( xXSlot.is(), "a node for the first dimension of a chart tree should always be a parent" );
            if(!xXSlot.is())
                return;
            uno::Reference< XStackableScaleGroup > xStackGroup = uno::Reference< XStackableScaleGroup >::query( xXSlot );
            if( xStackGroup.is() &&
                xStackGroup->getStackMode()==StackMode_STACKED)
                nXSlot++;
            uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aYSlots(
                xXSlot->getChildren() );
            for( sal_Int32 nY = 0; nY < aYSlots.getLength(); ++nY )
            {
                uno::Reference< XDataSeriesTreeParent > xYSlot = uno::Reference< XDataSeriesTreeParent >::query( aYSlots[nY] );
                DBG_ASSERT( xYSlot.is(), "a node for the second dimension of a chart tree should always be a parent" );
                if(!xYSlot.is())
                    return;

                uno::Reference< XScaleGroup > xScaleGroup( xYSlot, uno::UNO_QUERY );

                {
                    double fCoordinateOrigin[3] = { 0.0, 0.0, 0.0 };
                    getCoordinateOrigin( fCoordinateOrigin, xScaleGroup->getCoordinateSystem() );
                    addCooSysToList(rVCooSysList,xScaleGroup->getCoordinateSystem(),fCoordinateOrigin);
                }

                xStackGroup.set( uno::Reference< XStackableScaleGroup >::query( xYSlot ));
                StackMode aYStackMode = StackMode_NONE;
                if(xStackGroup.is())
                    aYStackMode = xStackGroup->getStackMode();
                if(aYStackMode==StackMode_STACKED)
                    nYSlot++;
                uno::Sequence< uno::Reference< XDataSeriesTreeNode > > aSeriesList( xYSlot->getChildren() );
                for( sal_Int32 nS = 0; nS < aSeriesList.getLength(); ++nS )
                {
                    uno::Reference< XDataSeries > xDataSeries( aSeriesList[nS], uno::UNO_QUERY );

                    VDataSeries* pTestSeries = new VDataSeries( xDataSeries, getDefaultStyle(nS) );
                    //virtual void addSeries( VDataSeries* pSeries, sal_Int32 xSlot = -1,sal_Int32 ySlot = -1 );
                    sal_Int32 nXSlot2 = 0;
                    if(aYStackMode==StackMode_NONE)
                        nXSlot2=-1;
                    //@todo
                    apPlotter->addSeries( pTestSeries, nXSlot2 );
                   // apPlotter->addSeries( pTestSeries, nXSlot2, nYSlot );
                    /*
                    if(nN==nSeriesCount-1)
                        apPlotter->addSeries( pTestSeries, -1 );
                    else
                        apPlotter->addSeries( pTestSeries, 0 );
                    */
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
            VCoordinateSystem& rVCooSys = rVCooSysList[nC];
            rVCooSys.doAutoScale( apPlotter.get() );

            const uno::Sequence< ExplicitScaleData >&     rExplicitScales     = rVCooSys.getExplicitScales();
            const uno::Sequence< ExplicitIncrementData >& rExplicitIncrements = rVCooSys.getExplicitIncrements();

            double fCoordinateOrigin[3] = { 0.0, 0.0, 0.0 };
            for( sal_Int32 nDim = 0; nDim < 3; nDim++ )
                fCoordinateOrigin[nDim] = rVCooSys.getOriginByDimension( nDim );

            Matrix4D aM4_SceneToScreen( createTransformationSceneToScreen(rPos,rSize) );
            drawing::HomogenMatrix aHM_SceneToScreen( Matrix4DToHomogenMatrix(aM4_SceneToScreen) );

            //------------ create axes --- @todo do auto layout / fontscaling
            for( nDim = 0; nDim < 3; nDim++ )
            {
                uno::Reference< XAxis > xAxis = rVCooSys.getAxisByDimension(nDim);
                if(xAxis.is()
                    &&2==nDimension) //@todo remove this restriction if 3D axes are available
                {
                    AxisProperties aAxisProperties;
                    aAxisProperties.m_xAxisModel = xAxis;
                    aAxisProperties.m_pfExrtaLinePositionAtOtherAxis =
                        new double(nDim==1?fCoordinateOrigin[0]:fCoordinateOrigin[1]);
                    aAxisProperties.m_bTESTTEST_HorizontalAdjustmentIsLeft = sal_False;
                    //-------------------
                    VAxis aAxis(aAxisProperties,pNumberFormatterWrapper);
                    aAxis.setMeterData( rExplicitScales[nDim], rExplicitIncrements[nDim] );

                    aAxis.init(xTarget,xPageShapes,xShapeFactory);
                    if(2==nDimension)
                        aAxis.setTransformationSceneToScreen( aHM_SceneToScreen );
                    aAxis.setScales( rExplicitScales );
                    aAxis.createShapes();
                }
            }
            //------------ create grids
            rVCooSys.createGridShapes( xShapeFactory, xTarget, aHM_SceneToScreen );

            //------------ set scale to plotter
            apPlotter->init(xTarget,xPageShapes,xShapeFactory);
            if(2==nDimension)
                apPlotter->setTransformationSceneToScreen( aHM_SceneToScreen );
            apPlotter->setScales( rExplicitScales );
            apPlotter->createShapes();
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

bool getPosAndSizeForDiagram(
    awt::Point& rOutPos, awt::Size& rOutSize
    , const awt::Rectangle& rSpaceLeft )
{
    //@todo: we need a size dependent on the axis labels

    if(rSpaceLeft.Width <= 0 || rSpaceLeft.Height <= 0 )
        return false;

    //long nHeight = rSpaceLeft.Height * 5 / 6;
    // (1 - 5/6) / 2 = 1/12
    //long nOffsetY = rSpaceLeft.Y + rSpaceLeft.Height / 12;
    long nHeight = rSpaceLeft.Height * 11 / 12;
    long nOffsetY = rSpaceLeft.Y;

    long nWidth = rSpaceLeft.Width * 5 / 6;
    // (1 - 5/6) / 2 = 1/12
    long nOffsetX = rSpaceLeft.X + rSpaceLeft.Width / 12;

    if( nHeight <= 0 || nWidth <= 0 )
        return false;

    rOutPos = awt::Point(nOffsetX,nOffsetY);
    rOutSize = awt::Size(nWidth,nHeight);
    return true;
}

void createTitle( const uno::Reference< XTitle >& xTitle
                , sal_Int32 nXPosition
                , sal_Int32& nrYOffset
                , sal_Int32 nYDistance
                , const uno::Reference< drawing::XShapes>& xPageShapes
                , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
                 )
{
    if(xTitle.is())
    {
        VTitle aVTitle(xTitle);
        aVTitle.init(xPageShapes,xShapeFactory);
        aVTitle.createShapes( awt::Point(0,0) );
        awt::Size aTitleSize = aVTitle.getSize();
        aVTitle.changePosition( awt::Point( nXPosition, nrYOffset + aTitleSize.Height/2 + nYDistance ) );
        nrYOffset += aTitleSize.Height + 2*nYDistance;
    }
}

void createLegend( const uno::Reference< XLegend > & xLegend
                   , awt::Rectangle & rOutSpaceLeft
                   , const awt::Size & rPageSize
                   , const uno::Reference< drawing::XShapes>& xPageShapes
                   , const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
    )
{
    if( VLegend::isVisible( xLegend ))
    {
        VLegend aVLegend( xLegend );
        aVLegend.init( xPageShapes, xShapeFactory );
        aVLegend.createShapes( awt::Size( rOutSpaceLeft.Width, rOutSpaceLeft.Height ) );
        aVLegend.changePosition( rOutSpaceLeft, rPageSize );
    }
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
            tPropertyNameValueMap aFillValueMap;
            tMakePropertyNameMap aCharNameMap = PropertyMapper::getPropertyNameMapForFillProperties();
            PropertyMapper::getValueMap( aFillValueMap, aCharNameMap, xModelPage );

            tNameSequence aNames;
            tAnySequence aValues;
            PropertyMapper::getMultiPropertyListsFromValueMap( aNames, aValues, aFillValueMap );
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

    sal_Int32 nYOffset = 0;
    sal_Int32 nXPosition = rPageSize.Width/2;
    sal_Int32 nYDistance = rPageSize.Height*2/100;

    //------------ apply fill properties to page
    // todo: it would be nicer to just pass the page m_xDrawPage and format it,
    // but the page we have here does not support XPropertySet
    formatPage( m_xChartModel, rPageSize, xPageShapes, m_xShapeFactory );

    //------------ create main title shape
    createTitle( TitleHelper::getTitle( TitleHelper::MAIN_TITLE, m_xChartModel )
                , nXPosition, nYOffset, nYDistance, xPageShapes, m_xShapeFactory );
    if(nYOffset+nYDistance>=rPageSize.Height)
        return true;

    //------------ create sub title shape
    createTitle( TitleHelper::getTitle( TitleHelper::SUB_TITLE, m_xChartModel )
                , nXPosition, nYOffset, nYDistance, xPageShapes, m_xShapeFactory );
    if(nYOffset+nYDistance>=rPageSize.Height)
        return true;

    //------------ create legend
    awt::Rectangle aSpaceLeft( 0, nYOffset, rPageSize.Width, rPageSize.Height - nYOffset );
    createLegend( LegendHelper::getLegend( m_xChartModel )
                  , aSpaceLeft, rPageSize, xPageShapes, m_xShapeFactory );

    //------------ create complete diagram shape (inclusive axis and series)
    awt::Point aPosDia;
    awt::Size  aSizeDia;
    aSpaceLeft.Y += nYDistance;
    aSpaceLeft.Height -= nYDistance;//@todo substract 2*nYOffset if the diagram size is calculated dependent on axis labels
    if( getPosAndSizeForDiagram( aPosDia, aSizeDia, aSpaceLeft ) )
        initializeDiagramAndGetCooSys( m_aVCooSysList
                    , m_xCC, xPageShapes, m_xShapeFactory, m_pNumberFormatterWrapper
                    , aPosDia ,aSizeDia
                    , m_xChartModel );
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
