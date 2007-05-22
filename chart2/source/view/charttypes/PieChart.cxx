/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PieChart.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:16:08 $
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

#include "PieChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "PolarLabelPositionHelper.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCOLORSCHEME_HPP_
#include <com/sun/star/chart2/XColorScheme.hpp>
#endif

//#include "chartview/servicenames_charttypes.hxx"
//#include "servicenames_coosystems.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

class PiePositionHelper : public PolarPlottingPositionHelper
{
public:
    PiePositionHelper( NormalAxis eNormalAxis, double fAngleDegreeOffset );
    virtual ~PiePositionHelper();

    bool    getInnerAndOuterRadius( double fCategoryX, double& fLogicInnerRadius, double& fLogicOuterRadius, bool bUseRings, double fMaxOffset ) const;

public:
    //Distance between different category rings, seen relative to width of a ring:
    double  m_fRingDistance; //>=0 m_fRingDistance=1 --> distance == width
};

PiePositionHelper::PiePositionHelper( NormalAxis eNormalAxis, double fAngleDegreeOffset )
        : PolarPlottingPositionHelper(eNormalAxis)
        , m_fRingDistance(0.0)
{
    m_fRadiusOffset = 0.0;
    m_fAngleDegreeOffset = fAngleDegreeOffset;
}

PiePositionHelper::~PiePositionHelper()
{
}

bool PiePositionHelper::getInnerAndOuterRadius( double fCategoryX
                                               , double& fLogicInnerRadius, double& fLogicOuterRadius
                                               , bool bUseRings, double fMaxOffset ) const
{
    if( !bUseRings )
        fCategoryX = 1.0;

    bool bIsVisible = true;
    double fLogicInner = fCategoryX -0.5+m_fRingDistance/2.0;
    double fLogicOuter = fCategoryX +0.5-m_fRingDistance/2.0;

    if( !isMathematicalOrientationRadius() )
    {
        //in this case the given getMaximumX() was not corrcect instead the minimum should have been smaller by fMaxOffset
        //but during getMaximumX and getMimumX we do not know the axis orientation
        fLogicInner += fMaxOffset;
        fLogicOuter += fMaxOffset;
    }

    if( fLogicInner >= getLogicMaxX() )
        return false;
    if( fLogicOuter <= getLogicMinX() )
        return false;

    if( fLogicInner < getLogicMinX() )
        fLogicInner = getLogicMinX();
    if( fLogicOuter > getLogicMaxX() )
        fLogicOuter = getLogicMaxX();

    fLogicInnerRadius = fLogicInner;
    fLogicOuterRadius = fLogicOuter;
    if( !isMathematicalOrientationRadius() )
        std::swap(fLogicInnerRadius,fLogicOuterRadius);
    return bIsVisible;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PieChart::PieChart( const uno::Reference<XChartType>& xChartTypeModel
                   , sal_Int32 nDimensionCount )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount )
        , m_pPosHelper( new PiePositionHelper( NormalAxis_Z, (m_nDimension==3)?0.0:90.0 ) )
        , m_bUseRings(false)
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
    VSeriesPlotter::m_pMainPosHelper = m_pPosHelper;
    m_pPosHelper->m_fRadiusOffset = 0.0;
    m_pPosHelper->m_fRingDistance = 0.0;

    uno::Reference< beans::XPropertySet > xChartTypeProps( xChartTypeModel, uno::UNO_QUERY );
    if( xChartTypeProps.is() ) try
    {
        xChartTypeProps->getPropertyValue( C2U( "UseRings" )) >>= m_bUseRings;
        if( m_bUseRings )
        {
            m_pPosHelper->m_fRadiusOffset = 1.0;
            if( nDimensionCount==3 )
                m_pPosHelper->m_fRingDistance = 0.1;
        }
    }
    catch( uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

PieChart::~PieChart()
{
    delete m_pPosHelper;
}

//-----------------------------------------------------------------

void SAL_CALL PieChart::setScales( const uno::Sequence< ExplicitScaleData >& rScales
                                     , sal_Bool bSwapXAndYAxis )
                            throw (uno::RuntimeException)
{
    DBG_ASSERT(m_nDimension<=rScales.getLength(),"Dimension of Plotter does not fit two dimension of given scale sequence");
    m_pPosHelper->setScales( rScales, true );
}

//-----------------------------------------------------------------

drawing::Direction3D PieChart::getPreferredDiagramAspectRatio() const
{
    if( m_nDimension == 3 )
        return drawing::Direction3D(1,1,0.25);
    return drawing::Direction3D(1,1,1);
}

bool PieChart::keepAspectRatio() const
{
    if( m_nDimension == 3 )
        return false;
    return true;
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------
/*
APPHELPER_XSERVICEINFO_IMPL(PieChart,CHART2_VIEW_PIECHART_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > PieChart
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_PIECHART_SERVICE_NAME;
    return aSNS;
}
*/

bool PieChart::isSingleRingChart() const
{
    if( m_aZSlots.size() == 1 && m_aZSlots[0].size() ==1 )
        return true;
    return !m_bUseRings;
}

uno::Reference< drawing::XShape > PieChart::createDataPoint(
          const uno::Reference< drawing::XShapes >& xTarget
        , const uno::Reference< beans::XPropertySet >& xObjectProperties
        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
        , double fLogicZ, double fDepth, double fExplodePercentage
        , tPropertyNameValueMap* pOverwritePropertiesMap )
{
    //---------------------------
    //transform position:
    drawing::Direction3D aOffset;
    if( !::rtl::math::approxEqual( fExplodePercentage, 0.0 ) )
    {
        double fAngle  = fUnitCircleStartAngleDegree + fUnitCircleWidthAngleDegree/2.0;
        double fRadius = (fUnitCircleOuterRadius-fUnitCircleInnerRadius)*fExplodePercentage;
        drawing::Position3D aOrigin = m_pPosHelper->transformUnitCircleToScene( 0, 0, fLogicZ );
        drawing::Position3D aNewOrigin = m_pPosHelper->transformUnitCircleToScene( fAngle, fRadius, fLogicZ );
        aOffset = aNewOrigin - aOrigin;
    }

    //---------------------------
    //create point
    uno::Reference< drawing::XShape > xShape(0);
    if(m_nDimension==3)
    {
        xShape = m_pShapeFactory->createPieSegment( xTarget
            , fUnitCircleStartAngleDegree, fUnitCircleWidthAngleDegree
            , fUnitCircleInnerRadius, fUnitCircleOuterRadius
            , aOffset, B3DHomMatrixToHomogenMatrix( m_pPosHelper->getUnitCartesianToScene() )
            , fDepth );
    }
    else
    {
        xShape = m_pShapeFactory->createPieSegment2D( xTarget
            , fUnitCircleStartAngleDegree, fUnitCircleWidthAngleDegree
            , fUnitCircleInnerRadius, fUnitCircleOuterRadius
            , aOffset, B3DHomMatrixToHomogenMatrix( m_pPosHelper->getUnitCartesianToScene() ) );
    }
    this->setMappedProperties( xShape, xObjectProperties, PropertyMapper::getPropertyNameMapForFilledSeriesProperties(), pOverwritePropertiesMap );
    return xShape;
}

void PieChart::addSeries( VDataSeries* pSeries, sal_Int32 zSlot, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, 0, -1, 0 );
}

double PieChart::getMinimumX()
{
    return 0.5;
}
double PieChart::getMaxOffset() const
{
    double fRet = 0.0;
    if( m_aZSlots.size()<=0 )
        return fRet;
    if( m_aZSlots[0].size()<=0 )
        return fRet;

    const ::std::vector< VDataSeries* >& rSeriesList( m_aZSlots[0][0].m_aSeriesVector );
    if( rSeriesList.size()<=0 )
        return fRet;

    VDataSeries* pSeries = rSeriesList[0];
    uno::Reference< beans::XPropertySet > xSeriesProp( pSeries->getPropertiesOfSeries() );
    if( !xSeriesProp.is() )
        return fRet;

    double fExplodePercentage=0.0;
    xSeriesProp->getPropertyValue( C2U( "Offset" )) >>= fExplodePercentage;
    if(fExplodePercentage>fRet)
        fRet=fExplodePercentage;

    uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
    if( xSeriesProp->getPropertyValue( C2U( "AttributedDataPoints" ) ) >>= aAttributedDataPointIndexList )
    {
        for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
        {
            uno::Reference< beans::XPropertySet > xPointProp( pSeries->getPropertiesOfPoint(aAttributedDataPointIndexList[nN]) );
            if(xPointProp.is())
            {
                fExplodePercentage=0.0;
                xPointProp->getPropertyValue( C2U( "Offset" )) >>= fExplodePercentage;
                if(fExplodePercentage>fRet)
                    fRet=fExplodePercentage;
            }
        }
    }
    return fRet;
}
double PieChart::getMaximumX()
{
    double fMaxOffset = getMaxOffset();
    if( m_aZSlots.size()>0 && m_bUseRings)
        return m_aZSlots[0].size()+0.5+fMaxOffset;
    return 1.5+fMaxOffset;
}
double PieChart::getMinimumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    return 0.0;
}

double PieChart::getMaximumYInRange( double fMinimumX, double fMaximumX, sal_Int32 nAxisIndex )
{
    return 1.0;
}

bool PieChart::isExpandBorderToIncrementRhythm( sal_Int32 nDimensionIndex )
{
    return false;
}

bool PieChart::isExpandIfValuesCloseToBorder( sal_Int32 nDimensionIndex )
{
    return false;
}

bool PieChart::isExpandWideValuesToZero( sal_Int32 nDimensionIndex )
{
    return false;
}

bool PieChart::isExpandNarrowValuesTowardZero( sal_Int32 nDimensionIndex )
{
    return false;
}

bool PieChart::isSeperateStackingForDifferentSigns( sal_Int32 nDimensionIndex )
{
    return false;
}

void PieChart::createShapes()
{
    if( m_aZSlots.begin() == m_aZSlots.end() ) //no series
        return;

    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"PieChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,rtl::OUString() ));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget,rtl::OUString() ));
    //---------------------------------------------
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

//=============================================================================
    ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aZSlots[0].begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aZSlots[0].end();

    ::std::vector< VDataSeriesGroup >::size_type nExplodeableSlot = 0;
    if( m_pPosHelper->isMathematicalOrientationRadius() && m_bUseRings )
        nExplodeableSlot = m_aZSlots[0].size()-1;
//=============================================================================
    for( double fSlotX=0; aXSlotIter != aXSlotEnd && (m_bUseRings||fSlotX<0.5 ); aXSlotIter++, fSlotX+=1.0 )
    {
        ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
        if( pSeriesList->size()<=0 )//there should be only one series in each x slot
            continue;
        VDataSeries* pSeries = (*pSeriesList)[0];
        if(!pSeries)
            continue;

        double fLogicYSum = 0.0;
        //iterate through all points to get the sum
        sal_Int32 nPointIndex=0;
        sal_Int32 nPointCount=pSeries->getTotalPointCount();
        for( nPointIndex = 0; nPointIndex <= nPointCount; nPointIndex++ )
        {
            double fY = pSeries->getY( nPointIndex );
            if(fY<0.0)
            {
                //@todo warn somehow that negative values are treated as positive
            }
            if( ::rtl::math::isNan(fY) )
                continue;
            fLogicYSum += fabs(fY);
        }
        if(fLogicYSum==0.0)
            continue;
        double fLogicYForNextPoint = 0.0;
        //iterate through all points to create shapes
        for( nPointIndex = 0; nPointIndex <= nPointCount; nPointIndex++ )
        {
            double fLogicInnerRadius, fLogicOuterRadius;
            bool bIsVisible = m_pPosHelper->getInnerAndOuterRadius( fSlotX+1.0, fLogicInnerRadius, fLogicOuterRadius, m_bUseRings, getMaxOffset() );
            if( !bIsVisible )
                continue;

            double fLogicZ = -0.5;//as defined
            double fDepth  = this->getTransformedDepth();
//=============================================================================

            uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(pSeries, xSeriesTarget);
            //collect data point information (logic coordinates, style ):
            double fLogicYValue = fabs(pSeries->getY( nPointIndex ));
            if( ::rtl::math::isNan(fLogicYValue) )
                continue;
            if(fLogicYValue==0.0)//@todo: continue also if the resolution to small
                continue;
            double fLogicYPos = fLogicYForNextPoint;
            fLogicYForNextPoint += fLogicYValue;

            uno::Reference< beans::XPropertySet > xPointProperties = pSeries->getPropertiesOfPoint( nPointIndex );

            //iterate through all subsystems to create partial points
            {
                //logic values on angle axis:
                double fLogicStartAngleValue = fLogicYPos/fLogicYSum;
                double fLogicEndAngleValue = (fLogicYPos+fLogicYValue)/fLogicYSum;

                double fExplodePercentage=0.0;
                bool bDoExplode = ( nExplodeableSlot == static_cast< ::std::vector< VDataSeriesGroup >::size_type >(fSlotX) );
                if(bDoExplode) try
                {
                    xPointProperties->getPropertyValue( C2U( "Offset" )) >>= fExplodePercentage;
                }
                catch( uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                }

                //---------------------------
                //transforme to unit circle:
                double fUnitCircleWidthAngleDegree = m_pPosHelper->getWidthAngleDegree( fLogicStartAngleValue, fLogicEndAngleValue );
                double fUnitCircleStartAngleDegree = m_pPosHelper->transformToAngleDegree( fLogicStartAngleValue );
                double fUnitCircleInnerRadius = m_pPosHelper->transformToRadius( fLogicInnerRadius );
                double fUnitCircleOuterRadius = m_pPosHelper->transformToRadius( fLogicOuterRadius );

                //---------------------------
                //point color:
                std::auto_ptr< tPropertyNameValueMap > apOverwritePropertiesMap(0);
                {
                    if(!pSeries->hasPointOwnColor(nPointIndex) && m_xColorScheme.is())
                    {
                        apOverwritePropertiesMap = std::auto_ptr< tPropertyNameValueMap >( new tPropertyNameValueMap() );
                        (*apOverwritePropertiesMap)[C2U("FillColor")] = uno::makeAny(
                            m_xColorScheme->getColorByIndex( nPointIndex ));
                    }
                }

                //create data point
                uno::Reference<drawing::XShape> xPointShape(
                    createDataPoint( xSeriesGroupShape_Shapes, xPointProperties
                                    , fUnitCircleStartAngleDegree, fUnitCircleWidthAngleDegree
                                    , fUnitCircleInnerRadius, fUnitCircleOuterRadius
                                    , fLogicZ, fDepth, fExplodePercentage, apOverwritePropertiesMap.get() ) );

                //create label
                if( pSeries->getDataPointLabelIfLabel(nPointIndex) )
                {
                    if( !::rtl::math::approxEqual( fExplodePercentage, 0.0 ) )
                    {
                        double fExplodeOffset = (fUnitCircleOuterRadius-fUnitCircleInnerRadius)*fExplodePercentage;
                        fUnitCircleInnerRadius += fExplodeOffset;
                        fUnitCircleOuterRadius += fExplodeOffset;
                    }

                    LabelAlignment eAlignment(LABEL_ALIGN_CENTER);
                    sal_Int32 nScreenValueOffsetInRadiusDirection = (3!=m_nDimension && this->isSingleRingChart()) ? 500 : 0 ;//todo maybe calculate this font height dependent
                    awt::Point aScreenPosition2D(
                        PolarLabelPositionHelper(m_pPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory)
                        .getLabelScreenPositionAndAlignmentForUnitCircleValues(eAlignment, this->isSingleRingChart()
                        , fUnitCircleStartAngleDegree, fUnitCircleWidthAngleDegree
                        , fUnitCircleInnerRadius, fUnitCircleOuterRadius, 0.0, nScreenValueOffsetInRadiusDirection ));
                    this->createDataLabel( xTextTarget, *pSeries, nPointIndex
                                    , fLogicYValue, fLogicYSum, aScreenPosition2D, eAlignment );
                }

                if(!bDoExplode)
                {
                    ShapeFactory::setShapeName( xPointShape
                                , ObjectIdentifier::createPointCID( pSeries->getPointCID_Stub(), nPointIndex ) );
                }
                else try
                {
                    //enable dragging of outer segments

                    double fAngle  = fUnitCircleStartAngleDegree + fUnitCircleWidthAngleDegree/2.0;
                    double fMaxDeltaRadius = fUnitCircleOuterRadius-fUnitCircleInnerRadius;
                    drawing::Position3D aOrigin = m_pPosHelper->transformUnitCircleToScene( fAngle, fUnitCircleOuterRadius, fLogicZ );
                    drawing::Position3D aNewOrigin = m_pPosHelper->transformUnitCircleToScene( fAngle, fUnitCircleOuterRadius + fMaxDeltaRadius, fLogicZ );

                    sal_Int32 nOffsetPercent( static_cast<sal_Int32>(fExplodePercentage * 100.0) );

                    awt::Point aMinimumPosition( PlottingPositionHelper::transformSceneToScreenPosition(
                        aOrigin, m_xLogicTarget, m_pShapeFactory, m_nDimension ) );
                    awt::Point aMaximumPosition( PlottingPositionHelper::transformSceneToScreenPosition(
                        aNewOrigin, m_xLogicTarget, m_pShapeFactory, m_nDimension ) );

                    //enable draging of piesegments
                    rtl::OUString aPointCIDStub( ObjectIdentifier::createSeriesSubObjectStub( OBJECTTYPE_DATA_POINT
                        , pSeries->getSeriesParticle()
                        , ObjectIdentifier::getPieSegmentDragMethodServiceName()
                        , ObjectIdentifier::createPieSegmentDragParameterString(
                            nOffsetPercent, aMinimumPosition, aMaximumPosition )
                        ) );

                    ShapeFactory::setShapeName( xPointShape
                                , ObjectIdentifier::createPointCID( aPointCIDStub, nPointIndex ) );
                }
                catch( uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                }
            }//next series in x slot (next y slot)
        }//next category
    }//next x slot
//=============================================================================
//=============================================================================
//=============================================================================
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
