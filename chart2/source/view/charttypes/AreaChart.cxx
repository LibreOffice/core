/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AreaChart.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:05:02 $
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

#include "AreaChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
//#include "chartview/servicenames_charttypes.hxx"
#include "CommonConverters.hxx"
#include "macros.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"
#include "Splines.hxx"
#include "ChartTypeHelper.hxx"
#include "LabelPositionHelper.hxx"
#include "Clipping.hxx"
#include "Stripe.hxx"

#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif
#include <com/sun/star/chart/DataLabelPlacement.hpp>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_UNOPRNMS_HXX
#include <svx/unoprnms.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_NORMALSKIND_HPP_
#include <com/sun/star/drawing/NormalsKind.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::rtl::math;
using namespace ::com::sun::star::chart2;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

AreaChart::AreaChart( const uno::Reference<XChartType>& xChartTypeModel
                     , sal_Int32 nDimensionCount
                     , bool bCategoryXAxis
                     , bool bNoArea
                     , PlottingPositionHelper* pPlottingPositionHelper
                     , bool bConnectLastToFirstPoint
                     , bool bAddOneToXMax
                     , bool bExpandIfValuesCloseToBorder
                     , sal_Int32 nKeepAspectRatio
                     , const drawing::Direction3D& rAspectRatio
                     )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount, bCategoryXAxis )
        , m_pMainPosHelper(pPlottingPositionHelper)
        , m_bArea(!bNoArea)
        , m_bLine(bNoArea)
        , m_bSymbol( ChartTypeHelper::isSupportingSymbolProperties(xChartTypeModel,nDimensionCount) )
        , m_bConnectLastToFirstPoint( bConnectLastToFirstPoint )
        , m_bAddOneToXMax(bAddOneToXMax)
        , m_bExpandIfValuesCloseToBorder( bExpandIfValuesCloseToBorder )
        , m_nKeepAspectRatio(nKeepAspectRatio)
        , m_aGivenAspectRatio(rAspectRatio)
        , m_eNanHandling( bCategoryXAxis ? NAN_AS_GAP : NAN_AS_INTERPOLATED )
        , m_eCurveStyle(CurveStyle_LINES)
        , m_nCurveResolution(20)
        , m_nSplineOrder(3)
        , m_xSeriesTarget(0)
        , m_xErrorBarTarget(0)
        , m_xTextTarget(0)
        , m_xRegressionCurveEquationTarget(0)
{
    if( !m_pMainPosHelper )
        m_pMainPosHelper = new PlottingPositionHelper();
    PlotterBase::m_pPosHelper = m_pMainPosHelper;
    VSeriesPlotter::m_pMainPosHelper = m_pMainPosHelper;

    if( m_bArea )
        m_eNanHandling = NAN_AS_ZERO;

    try
    {
        if( m_xChartTypeModelProps.is() )
        {
            m_xChartTypeModelProps->getPropertyValue( C2U( "CurveStyle" ) ) >>= m_eCurveStyle;
            m_xChartTypeModelProps->getPropertyValue( C2U( "CurveResolution" ) ) >>= m_nCurveResolution;
            m_xChartTypeModelProps->getPropertyValue( C2U( "SplineOrder" ) ) >>= m_nSplineOrder;
        }
    }
    catch( uno::Exception& e )
    {
        //the above properties are not supported by all charttypes supported by this class (e.g. area or net chart)
        //in that cases this exception is ok
        e.Context.is();//to have debug information without compilation warnings
    }
}

AreaChart::~AreaChart()
{
    delete m_pMainPosHelper;
}

double AreaChart::getMaximumX()
{
    if( m_bAddOneToXMax )
    {
        //return category count
        sal_Int32 nPointCount = getPointCount();
        return nPointCount+1;
    }
    return VSeriesPlotter::getMaximumX();
}

bool AreaChart::isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex )
{
    return m_bExpandIfValuesCloseToBorder &&
        VSeriesPlotter::isExpandIfValuesCloseToBorder( nDimensionIndex );
}

bool AreaChart::isSeperateStackingForDifferentSigns( sal_Int32 /*nDimensionIndex*/ )
{
    // no separate stacking in all types of line/area charts
    return false;
}

//-----------------------------------------------------------------

LegendSymbolStyle AreaChart::getLegendSymbolStyle()
{
    if( m_bArea || m_nDimension == 3 )
        return chart2::LegendSymbolStyle_BOX;
    return chart2::LegendSymbolStyle_LINE_WITH_SYMBOL;
}

uno::Any AreaChart::getExplicitSymbol( const VDataSeries& rSeries, sal_Int32 nPointIndex )
{
    uno::Any aRet;

    Symbol* pSymbolProperties = rSeries.getSymbolProperties( nPointIndex );
    if( pSymbolProperties )
    {
        aRet = uno::makeAny(*pSymbolProperties);
    }

    return aRet;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------
/*
APPHELPER_XSERVICEINFO_IMPL(AreaChart,CHART2_VIEW_AREACHART_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > AreaChart
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_AREACHART_SERVICE_NAME;
    return aSNS;
}
*/
/*
//-----------------------------------------------------------------
// chart2::XPlotter
//-----------------------------------------------------------------

    ::rtl::OUString SAL_CALL AreaChart
::getCoordinateSystemTypeID()
    throw (uno::RuntimeException)
{
    return CHART2_COOSYSTEM_CARTESIAN_SERVICE_NAME;
}
*/
drawing::Direction3D AreaChart::getPreferredDiagramAspectRatio() const
{
    if( m_nKeepAspectRatio == 1 )
        return m_aGivenAspectRatio;
    drawing::Direction3D aRet(1,-1,1);
    if( m_nDimension == 2 )
        aRet = drawing::Direction3D(-1,-1,-1);
    else
    {
        drawing::Direction3D aScale( m_pPosHelper->getScaledLogicWidth() );
        aRet.DirectionZ = aScale.DirectionZ*0.2;
        if(aRet.DirectionZ>1.0)
            aRet.DirectionZ=1.0;
        if(aRet.DirectionZ>10)
            aRet.DirectionZ=10;
    }
    return aRet;
}

bool AreaChart::keepAspectRatio() const
{
    if( m_nKeepAspectRatio == 0 )
        return false;
    if( m_nKeepAspectRatio == 1 )
        return true;
    if( m_nDimension == 2 )
    {
        if( !m_bSymbol )
            return false;
    }
    return true;
}

void AreaChart::addSeries( VDataSeries* pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    if( m_nDimension == 3 && !m_bCategoryXAxis )
    {
        //3D xy always deep
        DBG_ASSERT( zSlot==-1,"3D xy charts should be deep stacked in model also" );
        zSlot=-1;
        xSlot=0;
        ySlot=0;
    }
    VSeriesPlotter::addSeries( pSeries, zSlot, xSlot, ySlot );
}

void lcl_removeDuplicatePoints( drawing::PolyPolygonShape3D& rPolyPoly, PlottingPositionHelper& rPosHelper )
{
    sal_Int32 nPolyCount = rPolyPoly.SequenceX.getLength();
    if(!nPolyCount)
        return;

    drawing::PolyPolygonShape3D aTmp;
    aTmp.SequenceX.realloc(nPolyCount);
    aTmp.SequenceY.realloc(nPolyCount);
    aTmp.SequenceZ.realloc(nPolyCount);

    for( sal_Int32 nPolygonIndex = 0; nPolygonIndex<nPolyCount; nPolygonIndex++ )
    {
        drawing::DoubleSequence* pOuterSourceX = &rPolyPoly.SequenceX.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSourceY = &rPolyPoly.SequenceY.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterSourceZ = &rPolyPoly.SequenceZ.getArray()[nPolygonIndex];

        drawing::DoubleSequence* pOuterTargetX = &aTmp.SequenceX.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterTargetY = &aTmp.SequenceY.getArray()[nPolygonIndex];
        drawing::DoubleSequence* pOuterTargetZ = &aTmp.SequenceZ.getArray()[nPolygonIndex];

        sal_Int32 nPointCount = pOuterSourceX->getLength();
        if( !nPointCount )
            continue;

        pOuterTargetX->realloc(nPointCount);
        pOuterTargetY->realloc(nPointCount);
        pOuterTargetZ->realloc(nPointCount);

        double* pSourceX = pOuterSourceX->getArray();
        double* pSourceY = pOuterSourceY->getArray();
        double* pSourceZ = pOuterSourceZ->getArray();

        double* pTargetX = pOuterTargetX->getArray();
        double* pTargetY = pOuterTargetY->getArray();
        double* pTargetZ = pOuterTargetZ->getArray();

        //copy first point
        *pTargetX=*pSourceX++;
        *pTargetY=*pSourceY++;
        *pTargetZ=*pSourceZ++;
        sal_Int32 nTargetPointCount=1;

        for( sal_Int32 nSource=1; nSource<nPointCount; nSource++ )
        {
            if( !rPosHelper.isSameForGivenResolution( *pTargetX, *pTargetY, *pTargetZ
                                                   , *pSourceX, *pSourceY, *pSourceZ ) )
            {
                pTargetX++; pTargetY++; pTargetZ++;
                *pTargetX=*pSourceX;
                *pTargetY=*pSourceY;
                *pTargetZ=*pSourceZ;
                nTargetPointCount++;
            }
            pSourceX++; pSourceY++; pSourceZ++;
        }

        //free unused space
        if( nTargetPointCount<nPointCount )
        {
            pOuterTargetX->realloc(nTargetPointCount);
            pOuterTargetY->realloc(nTargetPointCount);
            pOuterTargetZ->realloc(nTargetPointCount);
        }

        pOuterSourceX->realloc(0);
        pOuterSourceY->realloc(0);
        pOuterSourceZ->realloc(0);
    }

    //free space
    rPolyPoly.SequenceX.realloc(nPolyCount);
    rPolyPoly.SequenceY.realloc(nPolyCount);
    rPolyPoly.SequenceZ.realloc(nPolyCount);

    rPolyPoly=aTmp;
}

bool AreaChart::impl_createLine( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , PlottingPositionHelper* pPosHelper )
{
    //return true if a line was created successfully
    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);

    drawing::PolyPolygonShape3D aPoly;
    if(CurveStyle_CUBIC_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateCubicSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution );
        lcl_removeDuplicatePoints( aSplinePoly, *pPosHelper );
        Clipping::clipPolygonAtRectangle( aSplinePoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }
    else if(CurveStyle_B_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateBSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution, m_nSplineOrder );
        lcl_removeDuplicatePoints( aSplinePoly, *pPosHelper );
        Clipping::clipPolygonAtRectangle( aSplinePoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }
    else
    {
        bool bIsClipped = false;
        if( m_bConnectLastToFirstPoint && !ShapeFactory::isPolygonEmptyOrSinglePoint(*pSeriesPoly) )
        {
            // do NOT connect last and first point, if one is NAN, and NAN handling is NAN_AS_GAP
            double fFirstY = pSeries->getY( 0 );
            double fLastY = pSeries->getY( VSeriesPlotter::getPointCount() - 1 );
            if( (m_eNanHandling != NAN_AS_GAP) || (::rtl::math::isFinite( fFirstY ) && ::rtl::math::isFinite( fLastY )) )
            {
                // connect last point in last polygon with first point in first polygon
                ::basegfx::B2DRectangle aScaledLogicClipDoubleRect( pPosHelper->getScaledLogicClipDoubleRect() );
                drawing::PolyPolygonShape3D aTmpPoly(*pSeriesPoly);
                drawing::Position3D aLast(aScaledLogicClipDoubleRect.getMaxX(),aTmpPoly.SequenceY[0][0],aTmpPoly.SequenceZ[0][0]);
                // add connector line to last polygon
                AddPointToPoly( aTmpPoly, aLast, pSeriesPoly->SequenceX.getLength() - 1 );
                Clipping::clipPolygonAtRectangle( aTmpPoly, aScaledLogicClipDoubleRect, aPoly );
                bIsClipped = true;
            }
        }

        if( !bIsClipped )
            Clipping::clipPolygonAtRectangle( *pSeriesPoly, pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }

    if(!ShapeFactory::hasPolygonAnyLines(aPoly))
        return false;

    //transformation 3) -> 4)
    pPosHelper->transformScaledLogicToScene( aPoly );

    //create line:
    uno::Reference< drawing::XShape > xShape(NULL);
    if(m_nDimension==3)
    {
        double fDepth = this->getTransformedDepth();
        sal_Int32 nPolyCount = aPoly.SequenceX.getLength();
        for(sal_Int32 nPoly=0;nPoly<nPolyCount;nPoly++)
        {
            sal_Int32 nPointCount = aPoly.SequenceX[nPoly].getLength();
            for(sal_Int32 nPoint=0;nPoint<nPointCount-1;nPoint++)
            {
                drawing::Position3D aPoint1, aPoint2;
                aPoint1.PositionX = aPoly.SequenceX[nPoly][nPoint+1];
                aPoint1.PositionY = aPoly.SequenceY[nPoly][nPoint+1];
                aPoint1.PositionZ = aPoly.SequenceZ[nPoly][nPoint+1];

                aPoint2.PositionX = aPoly.SequenceX[nPoly][nPoint];
                aPoint2.PositionY = aPoly.SequenceY[nPoly][nPoint];
                aPoint2.PositionZ = aPoly.SequenceZ[nPoly][nPoint];

                Stripe aStripe( aPoint1, aPoint2, fDepth );

                m_pShapeFactory->createStripe(xSeriesGroupShape_Shapes
                    , Stripe( aPoint1, aPoint2, fDepth )
                    , pSeries->getPropertiesOfSeries(), PropertyMapper::getPropertyNameMapForFilledSeriesProperties(), true );
            }
        }
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createLine2D( xSeriesGroupShape_Shapes
                , PolyToPointSequence( aPoly ) );
        this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForLineSeriesProperties() );
        //because of this name this line will be used for marking
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
    return true;
}

bool AreaChart::impl_createArea( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly
                , drawing::PolyPolygonShape3D* pPreviousSeriesPoly
                , PlottingPositionHelper* pPosHelper )
{
    //return true if an area was created successfully

    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);
    double zValue = pSeries->m_fLogicZPos;

    drawing::PolyPolygonShape3D aPoly( *pSeriesPoly );
    //add second part to the polygon (grounding points or previous series points)
    if(!pPreviousSeriesPoly)
    {
        double fMinX = pSeries->m_fLogicMinX;
        double fMaxX = pSeries->m_fLogicMaxX;
        double fY = pPosHelper->getBaseValueY();//logic grounding
        if( m_nDimension==3 )
            fY = pPosHelper->getLogicMinY();

        //clip to scale
        if(fMaxX<pPosHelper->getLogicMinX() || fMinX>pPosHelper->getLogicMaxX())
            return false;//no visible shape needed
        pPosHelper->clipLogicValues( &fMinX, &fY, 0 );
        pPosHelper->clipLogicValues( &fMaxX, 0, 0 );

        //apply scaling
        {
            pPosHelper->doLogicScaling( &fMinX, &fY, &zValue );
            pPosHelper->doLogicScaling( &fMaxX, 0, 0 );
        }

        AddPointToPoly( aPoly, drawing::Position3D( fMaxX,fY,zValue) );
        AddPointToPoly( aPoly, drawing::Position3D( fMinX,fY,zValue) );
    }
    else
    {
        appendPoly( aPoly, *pPreviousSeriesPoly );
    }
    ShapeFactory::closePolygon(aPoly);

    //apply clipping
    {
        drawing::PolyPolygonShape3D aClippedPoly;
        Clipping::clipPolygonAtRectangle( aPoly, pPosHelper->getScaledLogicClipDoubleRect(), aClippedPoly, false );
        ShapeFactory::closePolygon(aClippedPoly); //again necessary after clipping
        aPoly = aClippedPoly;
    }

    if(!ShapeFactory::hasPolygonAnyLines(aPoly))
        return false;

    //transformation 3) -> 4)
    pPosHelper->transformScaledLogicToScene( aPoly );

    //create area:
    uno::Reference< drawing::XShape > xShape(NULL);
    if(m_nDimension==3)
    {
        xShape = m_pShapeFactory->createArea3D( xSeriesGroupShape_Shapes
                , aPoly, this->getTransformedDepth() );
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createArea2D( xSeriesGroupShape_Shapes
                , aPoly );
    }
    this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , PropertyMapper::getPropertyNameMapForFilledSeriesProperties() );
    //because of this name this line will be used for marking
    m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    return true;
}

void AreaChart::impl_createSeriesShapes()
{
    //the polygon shapes for each series need to be created before

    //iterate through all series again to create the series shapes
    ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator            aZSlotIter = m_aZSlots.begin();
    const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator aZSlotEnd = m_aZSlots.end();
//=============================================================================
    for( sal_Int32 nZ=1; aZSlotIter != aZSlotEnd; aZSlotIter++, nZ++ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

    //=============================================================================
        for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
    //=============================================================================

            std::map< sal_Int32, drawing::PolyPolygonShape3D* > aPreviousSeriesPolyMap;//a PreviousSeriesPoly for each different nAttachedAxisIndex
            drawing::PolyPolygonShape3D* pSeriesPoly = NULL;

            //iterate through all series
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                sal_Int32 nAttachedAxisIndex = (*aSeriesIter)->getAttachedAxisIndex();
                PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                if(!pPosHelper)
                    pPosHelper = m_pMainPosHelper;
                PlotterBase::m_pPosHelper = pPosHelper;

                createRegressionCurvesShapes( **aSeriesIter, m_xErrorBarTarget, m_xRegressionCurveEquationTarget,
                                              m_pPosHelper->maySkipPointsInRegressionCalculation());

                pSeriesPoly = &(*aSeriesIter)->m_aPolyPolygonShape3D;
                if( m_bArea )
                {
                    if( !impl_createArea( *aSeriesIter, pSeriesPoly, aPreviousSeriesPolyMap[nAttachedAxisIndex], pPosHelper ) )
                        continue;
                }
                if( m_bLine )
                {
                    if( !impl_createLine( *aSeriesIter, pSeriesPoly, pPosHelper ) )
                        continue;
                }
                aPreviousSeriesPolyMap[nAttachedAxisIndex] = pSeriesPoly;
            }//next series in x slot (next y slot)
        }//next x slot
    }//next z slot
}

namespace
{

void lcl_reorderSeries( ::std::vector< ::std::vector< VDataSeriesGroup > >&  rZSlots )
{
    ::std::vector< ::std::vector< VDataSeriesGroup > >  aRet( rZSlots.size() );

    ::std::vector< ::std::vector< VDataSeriesGroup > >::reverse_iterator aZIt( rZSlots.rbegin() );
    ::std::vector< ::std::vector< VDataSeriesGroup > >::reverse_iterator aZEnd( rZSlots.rend() );
    for( ; aZIt != aZEnd; ++aZIt )
    {
        ::std::vector< VDataSeriesGroup > aXSlot( aZIt->size() );

        ::std::vector< VDataSeriesGroup >::reverse_iterator aXIt( aZIt->rbegin() );
        ::std::vector< VDataSeriesGroup >::reverse_iterator aXEnd( aZIt->rend() );
        for( ; aXIt != aXEnd; ++aXIt )
            aXSlot.push_back(*aXIt);

        aRet.push_back(aXSlot);
    }

    rZSlots.clear();
    rZSlots = aRet;
}

}//anonymous namespace

void AreaChart::impl_maybeReplaceNanWithZero( double& rfValue )
{
    if( m_eNanHandling == NAN_AS_ZERO &&
        ( ::rtl::math::isNan(rfValue) || ::rtl::math::isInf(rfValue) )  )
        rfValue = 0.0;
}

//better performance for big data
struct FormerPoint
{
    FormerPoint( double fX, double fY, double fZ )
        : m_fX(fX), m_fY(fY), m_fZ(fZ)
        {}
    FormerPoint()
    {
        ::rtl::math::setNan( &m_fX );
        ::rtl::math::setNan( &m_fY );
        ::rtl::math::setNan( &m_fZ );
    }

    double m_fX;
    double m_fY;
    double m_fZ;
};

void AreaChart::createShapes()
{
    if( m_aZSlots.begin() == m_aZSlots.end() ) //no series
        return;

    if( m_nDimension == 2 && ( m_bArea || !m_bCategoryXAxis ) )
        lcl_reorderSeries( m_aZSlots );

    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"AreaChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //for area chart the error bars should be always on top of the other series shapes

    //therefore create an own group for the texts and the error bars to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    m_xSeriesTarget   = createGroupShape( m_xLogicTarget,rtl::OUString() );
    if( m_bArea )
        m_xErrorBarTarget = createGroupShape( m_xLogicTarget,rtl::OUString() );
    else
        m_xErrorBarTarget = m_xSeriesTarget;
    m_xTextTarget     = m_pShapeFactory->createGroup2D( m_xFinalTarget,rtl::OUString() );
    m_xRegressionCurveEquationTarget = m_pShapeFactory->createGroup2D( m_xFinalTarget,rtl::OUString() );

    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    //update/create information for current group
    double fLogicZ        = 0.5;//as defined

    sal_Int32 nStartIndex = 0; // inclusive       ;..todo get somehow from x scale
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount();
    if(nEndIndex<=0)
        nEndIndex=1;

    //better performance for big data
    std::map< VDataSeries*, FormerPoint > aSeriesFormerPointMap;
    m_bPointsWereSkipped = false;
    sal_Int32 nSkippedPoints = 0;
    sal_Int32 nCreatedPoints = 0;
    //

//=============================================================================
    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< ::std::vector< VDataSeriesGroup > >::iterator             aZSlotIter = m_aZSlots.begin();
        const ::std::vector< ::std::vector< VDataSeriesGroup > >::const_iterator  aZSlotEnd = m_aZSlots.end();

        std::map< sal_Int32, double > aLogicYSumMap;//one for each different nAttachedAxisIndex
        for( ; aZSlotIter != aZSlotEnd; aZSlotIter++ )
        {
            ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
            const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

            //iterate through all x slots in this category to get 100percent sum
            for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
            {
                ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
                ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
                const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

                for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
                {
                    VDataSeries* pSeries( *aSeriesIter );
                    if(!pSeries)
                        continue;

                    sal_Int32 nAttachedAxisIndex = pSeries->getAttachedAxisIndex();
                    if( aLogicYSumMap.find(nAttachedAxisIndex)==aLogicYSumMap.end() )
                        aLogicYSumMap[nAttachedAxisIndex]=0.0;

                    PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                    if(!pPosHelper)
                        pPosHelper = m_pMainPosHelper;
                    PlotterBase::m_pPosHelper = pPosHelper;

                    double fAdd = pSeries->getY( nIndex );
                    impl_maybeReplaceNanWithZero( fAdd );
                    if( !::rtl::math::isNan(fAdd) && !::rtl::math::isInf(fAdd) )
                        aLogicYSumMap[nAttachedAxisIndex] += fabs( fAdd );
                }
            }
        }

//=============================================================================
        aZSlotIter = m_aZSlots.begin();
        for( sal_Int32 nZ=1; aZSlotIter != aZSlotEnd; aZSlotIter++, nZ++ )
        {
            ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = aZSlotIter->begin();
            const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = aZSlotIter->end();

            //for the area chart there should be at most one x slot (no side by side stacking available)
            //attention different: xSlots are always interpreted as independent areas one behind the other: @todo this doesn't work why not???
            aXSlotIter = aZSlotIter->begin();
            for( sal_Int32 nX=0; aXSlotIter != aXSlotEnd; aXSlotIter++, nX++ )
            {
                ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
                ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
                const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

                std::map< sal_Int32, double > aLogicYForNextSeriesMap;//one for each different nAttachedAxisIndex
    //=============================================================================
                //iterate through all series
                for( sal_Int32 nSeriesIndex = 0; aSeriesIter != aSeriesEnd; aSeriesIter++, nSeriesIndex++ )
                {
                    VDataSeries* pSeries( *aSeriesIter );
                    if(!pSeries)
                        continue;

                    /*  #i70133# ignore points outside of series length in standard area
                        charts. Stacked area charts will use missing points as zeros. In
                        standard charts, pSeriesList contains only one series. */
                    if( m_bArea && (pSeriesList->size() == 1) && (nIndex >= (*aSeriesIter)->getTotalPointCount()) )
                        continue;

                    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeFrontChild(*aSeriesIter, m_xSeriesTarget);

                    sal_Int32 nAttachedAxisIndex = (*aSeriesIter)->getAttachedAxisIndex();
                    PlottingPositionHelper* pPosHelper = &(this->getPlottingPositionHelper( nAttachedAxisIndex ));
                    if(!pPosHelper)
                        pPosHelper = m_pMainPosHelper;
                    PlotterBase::m_pPosHelper = pPosHelper;

                    if(m_nDimension==3)
                        fLogicZ = nZ+0.5;
                    (*aSeriesIter)->m_fLogicZPos = fLogicZ;

                    //collect data point information (logic coordinates, style ):
                    double fLogicX = (*aSeriesIter)->getX(nIndex);
                    double fLogicY = (*aSeriesIter)->getY(nIndex);
                    impl_maybeReplaceNanWithZero( fLogicX );
                    impl_maybeReplaceNanWithZero( fLogicY );

                    if( m_nDimension==3 && m_bArea && pSeriesList->size()!=1 )
                        fLogicY = fabs( fLogicY );

                    if( pPosHelper->isPercentY() && !::rtl::math::approxEqual( aLogicYSumMap[nAttachedAxisIndex], 0.0 ) )
                    {
                        fLogicY = fabs( fLogicY )/aLogicYSumMap[nAttachedAxisIndex];
                    }

                    if(    ::rtl::math::isNan(fLogicX) || ::rtl::math::isInf(fLogicX)
                        || ::rtl::math::isNan(fLogicY) || ::rtl::math::isInf(fLogicY)
                        || ::rtl::math::isNan(fLogicZ) || ::rtl::math::isInf(fLogicZ) )
                    {
                        if( m_eNanHandling == NAN_AS_GAP )
                        {
                            drawing::PolyPolygonShape3D& rPolygon = (*aSeriesIter)->m_aPolyPolygonShape3D;
                            sal_Int32& rIndex = (*aSeriesIter)->m_nPolygonIndex;
                            if( 0<= rIndex && rIndex < rPolygon.SequenceX.getLength() )
                            {
                                if( rPolygon.SequenceX[ rIndex ].getLength() )
                                    rIndex++; //start a new polygon for the next point if the current poly is not empty
                            }
                        }
                        continue;
                    }

                    if( aLogicYForNextSeriesMap.find(nAttachedAxisIndex) == aLogicYForNextSeriesMap.end() )
                        aLogicYForNextSeriesMap[nAttachedAxisIndex] = 0.0;

                    double fLogicValueForLabeDisplay = fLogicY;

                    fLogicY += aLogicYForNextSeriesMap[nAttachedAxisIndex];
                    aLogicYForNextSeriesMap[nAttachedAxisIndex] = fLogicY;

                    bool bIsVisible = pPosHelper->isLogicVisible( fLogicX, fLogicY, fLogicZ );

                    //remind minimal and maximal x values for area 'grounding' points
                    //only for filled area
                    {
                        double& rfMinX = (*aSeriesIter)->m_fLogicMinX;
                        if(!nIndex||fLogicX<rfMinX)
                            rfMinX=fLogicX;
                        double& rfMaxX = (*aSeriesIter)->m_fLogicMaxX;
                        if(!nIndex||fLogicX>rfMaxX)
                            rfMaxX=fLogicX;
                    }

                    drawing::Position3D aUnscaledLogicPosition( fLogicX, fLogicY, fLogicZ );
                    drawing::Position3D aScaledLogicPosition(aUnscaledLogicPosition);
                    pPosHelper->doLogicScaling( aScaledLogicPosition );

                    //transformation 3) -> 4)
                    drawing::Position3D aScenePosition( pPosHelper->transformLogicToScene( fLogicX,fLogicY,fLogicZ, false ) );

                    //better performance for big data
                    FormerPoint aFormerPoint( aSeriesFormerPointMap[pSeries] );
                    pPosHelper->setCoordinateSystemResolution( m_aCoordinateSystemResolution );
                    if( !pSeries->isAttributedDataPoint(nIndex)
                            &&
                        pPosHelper->isSameForGivenResolution( aFormerPoint.m_fX, aFormerPoint.m_fY, aFormerPoint.m_fZ
                                                            , aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY, aScaledLogicPosition.PositionZ ) )
                    {
                        nSkippedPoints++;
                        m_bPointsWereSkipped = true;
                        continue;
                    }
                    aSeriesFormerPointMap[pSeries] = FormerPoint(aScaledLogicPosition.PositionX, aScaledLogicPosition.PositionY, aScaledLogicPosition.PositionZ);
                    //

                    //store point information for series polygon
                    //for area and/or line (symbols only do not need this)
                    if( isValidPosition(aScaledLogicPosition) )
                        AddPointToPoly( (*aSeriesIter)->m_aPolyPolygonShape3D, aScaledLogicPosition, (*aSeriesIter)->m_nPolygonIndex );

                    //create a single datapoint if point is visible
                    //apply clipping:
                    if( !bIsVisible )
                        continue;

                    bool bCreateErrorBar = false;
                    {
                        uno::Reference< beans::XPropertySet > xErrorBarProp(pSeries->getYErrorBarProperties(nIndex));
                        if( xErrorBarProp.is() )
                        {
                            bool bShowPositive = false;
                            bool bShowNegative = false;
                            xErrorBarProp->getPropertyValue( C2U( "ShowPositiveError" )) >>= bShowPositive;
                            xErrorBarProp->getPropertyValue( C2U( "ShowNegativeError" )) >>= bShowNegative;
                            bCreateErrorBar = bShowPositive || bShowNegative;
                        }
                    }

                    Symbol* pSymbolProperties = m_bSymbol ? (*aSeriesIter)->getSymbolProperties( nIndex ) : 0;
                    bool bCreateSymbol = pSymbolProperties && (pSymbolProperties->Style != SymbolStyle_NONE);

                    if( !bCreateSymbol && !bCreateErrorBar && !pSeries->getDataPointLabelIfLabel(nIndex) )
                        continue;

                    //create a group shape for this point and add to the series shape:
                    rtl::OUString aPointCID = ObjectIdentifier::createPointCID(
                        (*aSeriesIter)->getPointCID_Stub(), nIndex );
                    uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                        createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                    uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                            uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                    {
                        nCreatedPoints++;

                        //create data point
                        drawing::Direction3D aSymbolSize(0,0,0);
                        if( bCreateSymbol )
                        {
                            if(m_nDimension==3)
                            {
                                /* //no symbols for 3D
                                m_pShapeFactory->createSymbol3D( xPointGroupShape_Shapes
                                        , aScenePosition, aTransformedGeom.m_aSize
                                        , (*aSeriesIter)->getSymbolTypeOfPoint( nIndex ) );
                                        */
                            }
                            else //m_nDimension!=3
                            {
                                if( pSymbolProperties )
                                {
                                    if( pSymbolProperties->Style != SymbolStyle_NONE )
                                    {
                                        aSymbolSize.DirectionX = pSymbolProperties->Size.Width;
                                        aSymbolSize.DirectionY = pSymbolProperties->Size.Height;
                                    }

                                    if( pSymbolProperties->Style == SymbolStyle_STANDARD )
                                    {
                                        sal_Int32 nSymbol = pSymbolProperties->StandardSymbol;
                                        m_pShapeFactory->createSymbol2D( xPointGroupShape_Shapes
                                                , aScenePosition, aSymbolSize
                                                , nSymbol
                                                , pSymbolProperties->BorderColor
                                                , pSymbolProperties->FillColor );
                                    }
                                    else if( pSymbolProperties->Style == SymbolStyle_GRAPHIC )
                                    {
                                        m_pShapeFactory->createGraphic2D( xPointGroupShape_Shapes
                                                , aScenePosition , aSymbolSize
                                                , pSymbolProperties->Graphic );
                                    }
                                    //@todo other symbol styles
                                }
                            }
                        }
                        //create error bar
                        createErrorBar_Y( aUnscaledLogicPosition, **aSeriesIter, nIndex, m_xErrorBarTarget );

                        //create data point label
                        if( (**aSeriesIter).getDataPointLabelIfLabel(nIndex) )
                        {
                            LabelAlignment eAlignment = LABEL_ALIGN_TOP;
                            drawing::Position3D aScenePosition3D( aScenePosition.PositionX
                                        , aScenePosition.PositionY
                                        , aScenePosition.PositionZ+this->getTransformedDepth() );

                            sal_Int32 nLabelPlacement = pSeries->getLabelPlacement( nIndex, m_xChartTypeModel, m_nDimension, pPosHelper->isSwapXAndY() );

                            switch(nLabelPlacement)
                            {
                            case ::com::sun::star::chart::DataLabelPlacement::TOP:
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::BOTTOM:
                                aScenePosition3D.PositionY += (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_BOTTOM;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::LEFT:
                                aScenePosition3D.PositionX -= (aSymbolSize.DirectionX/2+1);
                                eAlignment = LABEL_ALIGN_LEFT;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::RIGHT:
                                aScenePosition3D.PositionX += (aSymbolSize.DirectionX/2+1);
                                eAlignment = LABEL_ALIGN_RIGHT;
                                break;
                            case ::com::sun::star::chart::DataLabelPlacement::CENTER:
                                eAlignment = LABEL_ALIGN_CENTER;
                                //todo implement this different for area charts
                                break;
                            default:
                                DBG_ERROR("this label alignment is not implemented yet");
                                aScenePosition3D.PositionY -= (aSymbolSize.DirectionY/2+1);
                                eAlignment = LABEL_ALIGN_TOP;
                                break;
                            }


                            awt::Point aScreenPosition2D( LabelPositionHelper(pPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory)
                                .transformSceneToScreenPosition( aScenePosition3D ) );
                            this->createDataLabel( m_xTextTarget, **aSeriesIter, nIndex
                                            , fLogicValueForLabeDisplay
                                            , aLogicYSumMap[nAttachedAxisIndex], aScreenPosition2D, eAlignment );
                        }
                    }

                    //remove PointGroupShape if empty
                    if(!xPointGroupShape_Shapes->getCount())
                        xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);

                }//next series in x slot (next y slot)
            }//next x slot
        }//next z slot
    }//next category
//=============================================================================
//=============================================================================
//=============================================================================

    impl_createSeriesShapes();

    /* @todo remove series shapes if empty
    //remove and delete point-group-shape if empty
    if(!xSeriesGroupShape_Shapes->getCount())
    {
        (*aSeriesIter)->m_xShape.set(NULL);
        m_xLogicTarget->remove(xSeriesGroupShape_Shape);
    }
    */

    //remove and delete series-group-shape if empty

    //... todo

    OSL_TRACE( "\nPPPPPPPPP<<<<<<<<<<<< area chart :: createShapes():: skipped points: %d created points: %d", nSkippedPoints, nCreatedPoints );
}

//.............................................................................
} //namespace chart
//.............................................................................
