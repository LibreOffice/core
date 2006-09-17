/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PieChart.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:33:48 $
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
//#include "chartview/servicenames_charttypes.hxx"
//#include "servicenames_coosystems.hxx"

#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "chartview/ObjectIdentifier.hxx"

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
    PiePositionHelper();
    virtual ~PiePositionHelper();

    bool                getInnerAndOuterRadius( double fCategoryX, double& fLogicInnerRadius, double& fLogicOuterRadius ) const;

    sal_Int32           getStartCategoryIndex() const {
                            //first category (index 0) matches with real number 1.0
                            sal_Int32 nStart = static_cast<sal_Int32>(getLogicMinX() - 0.5);
                            if( nStart < 0 )
                                nStart = 0;
                            return nStart;
                        }
    sal_Int32           getEndCategoryIndex() const  {
                            //first category (index 0) matches with real number 1.0
                            sal_Int32 nEnd = static_cast<sal_Int32>(getLogicMaxX() - 0.5);
                            if( nEnd < 0 )
                                nEnd = 0;
                            return nEnd;
                        }

public:
    //Distance between different category rings, seen relative to width of a ring:
    double      m_fRingDistance; //>=0 m_fRingDistance=1 --> distance == width
};

PiePositionHelper::PiePositionHelper()
        : PolarPlottingPositionHelper(true)
        , m_fRingDistance(0.0)
{
    m_fRadiusOffset = 0.0;
}

PiePositionHelper::~PiePositionHelper()
{
}
bool PiePositionHelper::getInnerAndOuterRadius( double fCategoryX, double& fLogicInnerRadius, double& fLogicOuterRadius ) const
{
    bool bIsVisible = true;
    double fLogicInner = fCategoryX -0.5+m_fRingDistance/2.0;
    double fLogicOuter = fCategoryX +0.5-m_fRingDistance/2.0;

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
    return bIsVisible;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PieChart::PieChart( const uno::Reference<XChartType>& xChartTypeModel, double fRadiusOffset, double fRingDistance )
        : VSeriesPlotter( xChartTypeModel )
        , m_pPosHelper( new PiePositionHelper() )
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
    m_pPosHelper->m_fRadiusOffset = fRadiusOffset;
    m_pPosHelper->m_fRingDistance = fRingDistance;
}

PieChart::~PieChart()
{
    delete m_pPosHelper;
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
    return m_pPosHelper->getEndCategoryIndex()==1 ;
}

uno::Reference< drawing::XShape > PieChart::createDataPoint(
          const uno::Reference< drawing::XShapes >& xTarget
        , const uno::Reference< beans::XPropertySet >& xObjectProperties
        , double fLogicStartAngleValue, double fLogicEndAngleValue
        , double fLogicInnerRadius, double fLogicOuterRadius
        , double fLogicZ, double fDepth )
{
    //transformation 3) -> 4)
    uno::Reference< XTransformation > xTransformation = m_pPosHelper->getTransformationLogicToScene();

    //---------------------------
    //transformed angle:
    double fWidthAngleDegree = m_pPosHelper->getWidthAngleDegree(fLogicStartAngleValue,fLogicEndAngleValue);
    double fStartAngleDegree = m_pPosHelper->transformToAngleDegree(fLogicStartAngleValue );

    //---------------------------
    //transform origin:
    drawing::Position3D aTransformedOrigin;
    {
        drawing::Position3D aLogicOrigin(0.0,0.0,0.0);
        aTransformedOrigin = SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicOrigin) ) );
    }

    //---------------------------
    //transform radii:
    double fTransformedInnerXRadius, fTransformedOuterXRadius, fTransformedOuterYRadius;
    double fTransformedDepth;
    {
        double fInnerRadius = m_pPosHelper->transformToRadius( fLogicInnerRadius );
        double fOuterRadius = m_pPosHelper->transformToRadius( fLogicOuterRadius );

        drawing::Position3D aLogicPos1;
        drawing::Position3D aLogicPos2( aLogicPos1 );
        aLogicPos2.PositionX += fInnerRadius;
        aLogicPos2.PositionZ += fDepth;
        drawing::Position3D aLogicPos3( aLogicPos1 );
        aLogicPos3.PositionX += fOuterRadius;
        aLogicPos3.PositionY += fOuterRadius;

        drawing::Position3D aTransformedPos1( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos1) ) ) );
        drawing::Position3D aTransformedPos2( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos2) ) ) );
        drawing::Position3D aTransformedPos3( SequenceToPosition3D( xTransformation->transform( Position3DToSequence(aLogicPos3) ) ) );

        fTransformedInnerXRadius = aTransformedPos2.PositionX - aTransformedPos1.PositionX;
        fTransformedOuterXRadius = aTransformedPos3.PositionX - aTransformedPos1.PositionX;
        fTransformedOuterYRadius = aTransformedPos3.PositionY - aTransformedPos1.PositionY;
        fTransformedDepth        = aTransformedPos2.PositionZ - aTransformedPos1.PositionZ;
    }

    uno::Reference< drawing::XShape > xShape(0);
    if(m_nDimension==3)
        xShape = m_pShapeFactory->createPieSegment( xTarget
            , fStartAngleDegree, fWidthAngleDegree
            , fTransformedInnerXRadius, fTransformedOuterXRadius, fTransformedOuterYRadius
            , aTransformedOrigin, fTransformedDepth );
    else
        xShape = m_pShapeFactory->createPieSegment2D( xTarget
            , fStartAngleDegree, fWidthAngleDegree
            , fTransformedInnerXRadius, fTransformedOuterXRadius, fTransformedOuterYRadius
            , aTransformedOrigin );

    this->setMappedProperties( xShape, xObjectProperties, m_aShapePropertyMapForArea );
    return xShape;
}

void PieChart::addSeries( VDataSeries* pSeries, sal_Int32 xSlot, sal_Int32 ySlot )
{
    VSeriesPlotter::addSeries( pSeries, 0, ySlot );
}

double PieChart::getMinimumYInRange( double fMinimumX, double fMaximumX )
{
    return 0.0;
}

double PieChart::getMaximumYInRange( double fMinimumX, double fMaximumX )
{
    return 1.0;
}

void PieChart::createShapes()
{
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

    //update/create information for current group
    //(@todo maybe different iteration for breaks in axis ?)
    sal_Int32 nStartCategoryIndex = m_pPosHelper->getStartCategoryIndex(); // inclusive
    sal_Int32 nEndCategoryIndex   = m_pPosHelper->getEndCategoryIndex(); //inclusive
//=============================================================================
    //iterate through all shown categories
    for( sal_Int32 nCatIndex = nStartCategoryIndex; nCatIndex <= nEndCategoryIndex; nCatIndex++ )
    {
        ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aXSlots.begin();
        const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aXSlots.end();
//=============================================================================
        //for the pie chart there should be at most one x slot per category (one ring is one x slot and there is only one ring per category)
        if( aXSlotIter != aXSlotEnd )
        {
            ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);

            ::std::vector< VDataSeries* >::const_iterator       aSeriesIter = pSeriesList->begin();
            const ::std::vector< VDataSeries* >::const_iterator aSeriesEnd  = pSeriesList->end();

            double fLogicYForNextSeries = 0.0;

            double fLogicYSum = 0.0;
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                double fY = (*aSeriesIter)->getY( nCatIndex );
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
            aSeriesIter = pSeriesList->begin();

            double fLogicInnerRadius, fLogicOuterRadius;
            bool bIsVisible = m_pPosHelper->getInnerAndOuterRadius( (double)nCatIndex+1.0, fLogicInnerRadius, fLogicOuterRadius );
            if( !bIsVisible )
                continue;

            double fLogicZ = -0.5;//as defined
            double fDepth  = 1.0; //as defined
//=============================================================================
            //iterate through all series in this x slot (in this ring)
            for( ; aSeriesIter != aSeriesEnd; aSeriesIter++ )
            {
                uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(*aSeriesIter, xSeriesTarget);
                //collect data point information (logic coordinates, style ):
                double fLogicYValue = fabs((*aSeriesIter)->getY( nCatIndex ));
                if( ::rtl::math::isNan(fLogicYValue) )
                    continue;
                if(fLogicYValue==0.0)//@todo: continue also if the resolution to small
                    continue;
                double fLogicYPos = fLogicYForNextSeries;
                fLogicYForNextSeries += fLogicYValue;

                rtl::OUString aPointCID = ObjectIdentifier::createPointCID(
                    (*aSeriesIter)->getPointCID_Stub(), nCatIndex );
                //each subsystem may add an additional shape to form the whole point
                //create a group shape for this point and add to the series shape:
                uno::Reference< drawing::XShapes > xPointGroupShape_Shapes(
                    createGroupShape(xSeriesGroupShape_Shapes,aPointCID) );
                uno::Reference<drawing::XShape> xPointGroupShape_Shape =
                        uno::Reference<drawing::XShape>( xPointGroupShape_Shapes, uno::UNO_QUERY );

                //iterate through all subsystems to create partial points
                {
                    //logic values on angle axis:
                    double fLogicStartAngleValue = fLogicYPos/fLogicYSum;
                    double fLogicEndAngleValue = (fLogicYPos+fLogicYValue)/fLogicYSum;

                    //create data point
                    createDataPoint( xPointGroupShape_Shapes ,(*aSeriesIter)->getPropertiesOfPoint( nCatIndex )
                                        , fLogicStartAngleValue, fLogicEndAngleValue
                                        , fLogicInnerRadius, fLogicOuterRadius
                                        , fLogicZ, fDepth );

                    //create label
                    if( (**aSeriesIter).getDataPointLabelIfLabel(nCatIndex) )
                    {
                        LabelAlignment eAlignment(LABEL_ALIGN_CENTER);
                        awt::Point aScreenPosition2D(
                            PolarLabelPositionHelper(m_pPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory).getLabelScreenPositionAndAlignment(eAlignment, this->isSingleRingChart()
                            , fLogicStartAngleValue, fLogicEndAngleValue
                            , fLogicInnerRadius, fLogicOuterRadius, fLogicZ ));
                        this->createDataLabel( xTextTarget, **aSeriesIter, nCatIndex
                                        , fLogicYValue, fLogicYSum, aScreenPosition2D, eAlignment );
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
