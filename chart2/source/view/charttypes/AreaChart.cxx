/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AreaChart.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:33:56 $
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
#include "TransformationHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "Splines.hxx"
#include "ChartTypeHelper.hxx"
#include "LabelPositionHelper.hxx"
#include "Clipping.hxx"

#ifndef _COM_SUN_STAR_CHART2_SYMBOL_HPP_
#include <com/sun/star/chart2/Symbol.hpp>
#endif

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

AreaChart::AreaChart( const uno::Reference<XChartType>& xChartTypeModel, bool bCategoryXAxis, bool bNoArea, PlottingPositionHelper* pPlottingPositionHelper )
        : VSeriesPlotter( xChartTypeModel, bCategoryXAxis )
        , m_bArea(!bNoArea)
        , m_bLine(bNoArea)
        , m_bSymbol( ChartTypeHelper::isSupportingSymbolProperties(xChartTypeModel) )
        , m_eCurveStyle(CurveStyle_LINES)
        , m_nCurveResolution(20)
        , m_nSplineOrder(3)
        , m_xSeriesTarget(0)
        , m_xErrorBarTarget(0)
        , m_xTextTarget(0)
{
    if( pPlottingPositionHelper )
        m_pPosHelper = pPlottingPositionHelper;
    else
        m_pPosHelper = new PlottingPositionHelper();

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
    }
}

AreaChart::~AreaChart()
{
    delete m_pPosHelper;
}

double AreaChart::getMaximumZ()
{
    if( 3!=m_nDimension )
        return VSeriesPlotter::getMaximumZ();

    return m_aXSlots.size()-0.5;
}

double AreaChart::getTransformedDepth() const
{
    //return the depth for a logic 1
    double MinZ = m_pPosHelper->getLogicMinZ();
    double MaxZ = m_pPosHelper->getLogicMaxZ();
    m_pPosHelper->doLogicScaling( 0, 0, &MinZ );
    m_pPosHelper->doLogicScaling( 0, 0, &MaxZ );
    return FIXED_SIZE_FOR_3D_CHART_VOLUME/(MaxZ-MinZ);
}

double AreaChart::getLogicGrounding() const
{
    //@todo get this from model axis crosses at if that value is between min and max
    //@todo get this for other directions - so far only y
    double fRet=0.0;
    m_pPosHelper->clipLogicValues( 0, &fRet, 0 );
    return fRet;
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

bool isPolygonEmptyOrSinglePoint(drawing::PolyPolygonShape3D& rPoly)
{
    if(!rPoly.SequenceX.getLength())
        return true;
    if(rPoly.SequenceX[0].getLength()<=1)
        return true;
    return false;
}

void closePolygon( drawing::PolyPolygonShape3D& rPoly)
{
    //add a last point == first point
    if(isPolygonEmptyOrSinglePoint(rPoly))
        return;
    drawing::Position3D aFirst(rPoly.SequenceX[0][0],rPoly.SequenceY[0][0],rPoly.SequenceZ[0][0]);
    AddPointToPoly( rPoly, aFirst );
}

void AreaChart::addSeries( VDataSeries* pSeries, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, xSlot, ySlot );
}

drawing::PolyPolygonShape3D createBorderPolygon(
                      drawing::PolyPolygonShape3D& rPoly
                    , double fDepth )
{
    drawing::PolyPolygonShape3D aRet;

    sal_Int32 nPolyCount = rPoly.SequenceX.getLength();
    sal_Int32 nBorder=0;
    for(sal_Int32 nPoly=0;nPoly<nPolyCount;nPoly++)
    {
        sal_Int32 nPointCount = rPoly.SequenceX[nPoly].getLength();
        for(sal_Int32 nPoint=0;nPoint<nPointCount-1;nPoint++)
        {
            aRet.SequenceX.realloc(nBorder+1);
            aRet.SequenceY.realloc(nBorder+1);
            aRet.SequenceZ.realloc(nBorder+1);

            aRet.SequenceX[nBorder].realloc(5);
            aRet.SequenceY[nBorder].realloc(5);
            aRet.SequenceZ[nBorder].realloc(5);

            aRet.SequenceX[nBorder][0] = aRet.SequenceX[nBorder][3] = aRet.SequenceX[nBorder][4] = rPoly.SequenceX[nPoly][nPoint];
            aRet.SequenceY[nBorder][0] = aRet.SequenceY[nBorder][3] = aRet.SequenceY[nBorder][4] = rPoly.SequenceY[nPoly][nPoint];
            aRet.SequenceZ[nBorder][0] = aRet.SequenceZ[nBorder][3] = aRet.SequenceZ[nBorder][4] = rPoly.SequenceZ[nPoly][nPoint];
            aRet.SequenceZ[nBorder][3] += fDepth;

            aRet.SequenceX[nBorder][1] = aRet.SequenceX[nBorder][2] = rPoly.SequenceX[nPoly][nPoint+1];
            aRet.SequenceY[nBorder][1] = aRet.SequenceY[nBorder][2] = rPoly.SequenceY[nPoly][nPoint+1];
            aRet.SequenceZ[nBorder][1] = aRet.SequenceZ[nBorder][2] = rPoly.SequenceZ[nPoly][nPoint+1];
            aRet.SequenceZ[nBorder][2] += fDepth;

            nBorder++;
        }
    }
    return aRet;
}

uno::Reference< drawing::XShape >
        create3DLine( const uno::Reference< drawing::XShapes >& xTarget
                    , uno::Reference< lang::XMultiServiceFactory > m_xShapeFactory
                    , drawing::PolyPolygonShape3D& rPoly
                    , double fDepth )
{
    //create shape
    uno::Reference< drawing::XShape > xShape(
            m_xShapeFactory->createInstance( C2U(
            "com.sun.star.drawing.Shape3DExtrudeObject" ) ), uno::UNO_QUERY );
    xTarget->add(xShape);

    //set properties
    uno::Reference< beans::XPropertySet > xProp( xShape, uno::UNO_QUERY );
    DBG_ASSERT(xProp.is(), "created shape offers no XPropertySet");
    if( xProp.is())
    {
        try
        {
            //depth
            xProp->setPropertyValue( C2U( UNO_NAME_3D_EXTRUDE_DEPTH )
                , uno::makeAny((sal_Int32)fDepth) );

            //Polygon
            xProp->setPropertyValue( C2U( UNO_NAME_3D_POLYPOLYGON3D )
                , uno::makeAny( rPoly ) );

            //NormalsKind
            xProp->setPropertyValue( C2U( UNO_NAME_3D_NORMALS_KIND )
                , uno::makeAny( drawing::NormalsKind_FLAT ) );

            //DoubleSided
            xProp->setPropertyValue( C2U( UNO_NAME_3D_DOUBLE_SIDED )
                , uno::makeAny( (sal_Bool)true) );
        }
        catch( uno::Exception& e )
        {
            ASSERT_EXCEPTION( e );
        }
    }
    return xShape;
}

bool AreaChart::impl_createLine( VDataSeries* pSeries
                , drawing::PolyPolygonShape3D* pSeriesPoly )
{
    //return true if a line was created successfully
    uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeBackChild(pSeries, m_xSeriesTarget);

    drawing::PolyPolygonShape3D aPoly;
    if(CurveStyle_CUBIC_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateCubicSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution );
        Clipping::clipPolygonAtRectangle( aSplinePoly, m_pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }
    else if(CurveStyle_B_SPLINES==m_eCurveStyle)
    {
        drawing::PolyPolygonShape3D aSplinePoly;
        SplineCalculater::CalculateBSplines( *pSeriesPoly, aSplinePoly, m_nCurveResolution, m_nSplineOrder );
        Clipping::clipPolygonAtRectangle( aSplinePoly, m_pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }
    else
    {
        Clipping::clipPolygonAtRectangle( *pSeriesPoly, m_pPosHelper->getScaledLogicClipDoubleRect(), aPoly );
    }

    if(isPolygonEmptyOrSinglePoint(aPoly))
        return false;

    //transformation 3) -> 4)
    m_pPosHelper->transformScaledLogicToScene( aPoly );

    //create line:
    uno::Reference< drawing::XShape > xShape(NULL);
    if(m_nDimension==3)
    {
        xShape = create3DLine( xSeriesGroupShape_Shapes, m_xShapeFactory
                , aPoly, this->getTransformedDepth() );
        this->setMappedProperties( xShape
                , pSeries->getPropertiesOfSeries()
                , m_aShapePropertyMapForArea );
        //createBorder
        {
            drawing::PolyPolygonShape3D aBorderPoly = createBorderPolygon(
                aPoly, this->getTransformedDepth() );
            VLineProperties aLineProperties;
            aLineProperties.initFromPropertySet( pSeries->getPropertiesOfSeries(), true );
            uno::Reference< drawing::XShape > xBorder =
                m_pShapeFactory->createLine3D( xSeriesGroupShape_Shapes
                    , aBorderPoly, aLineProperties );
            //because of this name this line will be used for marking
            m_pShapeFactory->setShapeName( xBorder, C2U("MarkHandles") );
        }
    }
    else //m_nDimension!=3
    {
        xShape = m_pShapeFactory->createLine2D( xSeriesGroupShape_Shapes
                , PolyToPointSequence( aPoly ), VLineProperties() );
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
                , drawing::PolyPolygonShape3D* pPreviousSeriesPoly )
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
        double fY = this->getLogicGrounding();

        //clip to scale
        if(fMaxX<m_pPosHelper->getLogicMinX() || fMinX>m_pPosHelper->getLogicMaxX())
            return false;//no visible shape needed
        m_pPosHelper->clipLogicValues( &fMinX, &fY, 0 );
        m_pPosHelper->clipLogicValues( &fMaxX, 0, 0 );

        //apply scaling
        {
            m_pPosHelper->doLogicScaling( &fMinX, &fY, &zValue );
            m_pPosHelper->doLogicScaling( &fMaxX, 0, 0 );
        }

        AddPointToPoly( aPoly, drawing::Position3D( fMaxX,fY,zValue) );
        AddPointToPoly( aPoly, drawing::Position3D( fMinX,fY,zValue) );
    }
    else
    {
        appendPoly( aPoly, *pPreviousSeriesPoly );
    }
    closePolygon(aPoly);

    //apply clipping
    {
        drawing::PolyPolygonShape3D aClippedPoly;
        Clipping::clipPolygonAtRectangle( aPoly, m_pPosHelper->getScaledLogicClipDoubleRect(), aClippedPoly, false );
        closePolygon(aClippedPoly); //again necessary after clipping
        aPoly = aClippedPoly;
    }

    if(isPolygonEmptyOrSinglePoint(aPoly))
        return false;

    //transformation 3) -> 4)
    m_pPosHelper->transformScaledLogicToScene( aPoly );

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
                , m_aShapePropertyMapForArea );
    //because of this name this line will be used for marking
    m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    return true;
}

void AreaChart::impl_createSeriesShapes()
{
    //the polygon shapes for each series need to be created before

    //iterate through all series again to create the series shapes
    ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
    //for the area chart there should be at most one x slot (no side by side stacking available)
    //handle as if independent series
    for( ; aXSlotIter != aXSlotEnd; aXSlotIter++ )
    {
        ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

        ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
        const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();
//=============================================================================

        drawing::PolyPolygonShape3D* pPreviousSeriesPoly = NULL;
        drawing::PolyPolygonShape3D* pSeriesPoly = NULL;

        //iterate through all series
        for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
        {
            createRegressionCurvesShapes( **aSeriesIter, m_xErrorBarTarget );

            pSeriesPoly = &(*aSeriesIter)->m_aPolyPolygonShape3D;
            if( m_bArea )
            {
                if( !impl_createArea( *aSeriesIter, pSeriesPoly, pPreviousSeriesPoly ) )
                    continue;
            }
            if( m_bLine )
            {
                if( !impl_createLine( *aSeriesIter, pSeriesPoly ) )
                    continue;
            }
            pPreviousSeriesPoly = pSeriesPoly;
        }//next series in x slot (next y slot)
    }//next x slot
}

void AreaChart::createShapes()
{
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

    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    //update/create information for current group
    double fLogicZ        = -0.5;//as defined

    sal_Int32 nStartIndex = 0; // inclusive       ;..todo get somehow from x scale
    sal_Int32 nEndIndex = VSeriesPlotter::getPointCount(m_aXSlots);
    if(nEndIndex<=0)
        nEndIndex=1;
//=============================================================================
    //iterate through all x values per indices
    for( sal_Int32 nIndex = nStartIndex; nIndex < nEndIndex; nIndex++ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
        //for the area chart there should be at most one x slot (no side by side stacking available)
        //attention different: xSlots are always interpreted as independent areas one behind the other: @todo this doesn't work why not???
        for( sal_Int32 nZ=0; aXSlotIter != aXSlotEnd; aXSlotIter++, nZ++ )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

            double fLogicYForNextSeries = 0.0;

            double fLogicYSum = 0.0;
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                double fAdd = (*aSeriesIter)->getY( nIndex );
                if( !::rtl::math::isNan(fAdd) )
                    fLogicYSum += fAdd;
            }
            aSeriesIter = pSeriesList->begin();
//=============================================================================
            //iterate through all series
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShapeFrontChild(*aSeriesIter, m_xSeriesTarget);

                if(m_nDimension==3)
                    fLogicZ = nZ+0.5;
                (*aSeriesIter)->m_fLogicZPos = fLogicZ;

                //collect data point information (logic coordinates, style ):
                double fLogicX = (*aSeriesIter)->getX(nIndex);
                double fLogicY = (*aSeriesIter)->getY(nIndex);

                bool bPointForAreaBoundingOnly = false;
                if(    ::rtl::math::isNan(fLogicX) || ::rtl::math::isInf(fLogicX)
                    || ::rtl::math::isNan(fLogicY) || ::rtl::math::isInf(fLogicY)
                    || ::rtl::math::isNan(fLogicZ) || ::rtl::math::isInf(fLogicZ) )
                {
                    if(! (m_bCategoryXAxis&&m_bArea) )
                        continue;

                    bPointForAreaBoundingOnly = true;
                    fLogicX = nIndex+1.0;
                    fLogicY=0;//@todo maybe there is another grounding ?? - for sum 0 is right
                    fLogicZ = nZ+0.5;
                }

                fLogicY += fLogicYForNextSeries;
                fLogicYForNextSeries = fLogicY;

                bool bIsVisible = !bPointForAreaBoundingOnly && m_pPosHelper->isLogicVisible( fLogicX, fLogicY, fLogicZ );

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
                //apply scaling
                m_pPosHelper->doLogicScaling( &fLogicX, &fLogicY, &fLogicZ );
                drawing::Position3D aScaledLogicPosition(fLogicX,fLogicY,fLogicZ);
                //transformation 3) -> 4)
                drawing::Position3D aScenePosition( m_pPosHelper->transformLogicToScene( fLogicX,fLogicY,fLogicZ, false ) );

                //store point information for series polygon
                //for area and/or line (symbols only do not need this)
                if(    !::rtl::math::isNan(fLogicX) && !::rtl::math::isInf(fLogicX)
                    && !::rtl::math::isNan(fLogicY) && !::rtl::math::isInf(fLogicY)
                    && !::rtl::math::isNan(fLogicZ) && !::rtl::math::isInf(fLogicZ) )
                {
                    drawing::PolyPolygonShape3D& rSeriesPoly = (*aSeriesIter)->m_aPolyPolygonShape3D;
                    AddPointToPoly( rSeriesPoly, aScaledLogicPosition );
                }

                //create a single datapoint if point is visible
                //apply clipping:
                if( !bIsVisible )
                    continue;

                //create a group shape for this point and add to the series shape:
                rtl::OUString aPointCID = ObjectIdentifier::createPointCID(
                    (*aSeriesIter)->getPointCID_Stub(), nIndex );
                uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                    createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                        uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                {
                    //create data point
                    drawing::Direction3D aSymbolSize(0,0,0);
                    if( m_bSymbol )
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
                            Symbol* pSymbolProperties = (*aSeriesIter)->getSymbolProperties( nIndex );
                            if( pSymbolProperties )
                            {
                                if( pSymbolProperties->aStyle == SymbolStyle_STANDARD )
                                {
                                    aSymbolSize.DirectionX = pSymbolProperties->aSize.Width;
                                    aSymbolSize.DirectionY = pSymbolProperties->aSize.Height;
                                    m_pShapeFactory->createSymbol2D( xPointGroupShape_Shapes
                                            , aScenePosition, aSymbolSize
                                            , pSymbolProperties->nStandardSymbol
                                            , pSymbolProperties->nFillColor );
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
                                    , aScenePosition.PositionY-aSymbolSize.DirectionY/2-1
                                    , aScenePosition.PositionZ+this->getTransformedDepth() );
                        awt::Point aScreenPosition2D( LabelPositionHelper(m_pPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory)
                            .transformSceneToScreenPosition( aScenePosition3D ) );
                        this->createDataLabel( m_xTextTarget, **aSeriesIter, nIndex
                                        , aUnscaledLogicPosition.PositionY
                                        , fLogicYSum, aScreenPosition2D, eAlignment );
                    }
                }

                //remove PointGroupShape if empty
                if(!xPointGroupShape_Shapes->getCount())
                    xSeriesGroupShape_Shapes->remove(xPointGroupShape_Shape);

            }//next series in x slot (next y slot)
        }//next x slot
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
}

//.............................................................................
} //namespace chart
//.............................................................................
