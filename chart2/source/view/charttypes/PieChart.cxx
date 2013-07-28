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

#include "PieChart.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "PolarLabelPositionHelper.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/XColorScheme.hpp>

#include <com/sun/star/container/XChild.hpp>
#include <rtl/math.hxx>

#include <boost/scoped_ptr.hpp>

namespace chart
{
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

PieChart::PieChart( const uno::Reference<XChartType>& xChartTypeModel
                   , sal_Int32 nDimensionCount
                   , bool bExcludingPositioning )
        : VSeriesPlotter( xChartTypeModel, nDimensionCount )
        , m_pPosHelper( new PiePositionHelper( NormalAxis_Z, (m_nDimension==3)?0.0:90.0 ) )
        , m_bUseRings(false)
        , m_bSizeExcludesLabelsAndExplodedSegments(bExcludingPositioning)
{
    ::rtl::math::setNan(&m_fMaxOffset);

    PlotterBase::m_pPosHelper = m_pPosHelper;
    VSeriesPlotter::m_pMainPosHelper = m_pPosHelper;
    m_pPosHelper->m_fRadiusOffset = 0.0;
    m_pPosHelper->m_fRingDistance = 0.0;

    uno::Reference< beans::XPropertySet > xChartTypeProps( xChartTypeModel, uno::UNO_QUERY );
    if( xChartTypeProps.is() ) try
    {
        xChartTypeProps->getPropertyValue( "UseRings") >>= m_bUseRings;
        if( m_bUseRings )
        {
            m_pPosHelper->m_fRadiusOffset = 1.0;
            if( nDimensionCount==3 )
                m_pPosHelper->m_fRingDistance = 0.1;
        }
    }
    catch( const uno::Exception& e )
    {
        ASSERT_EXCEPTION( e );
    }
}

PieChart::~PieChart()
{
    delete m_pPosHelper;
}

void PieChart::setScales( const std::vector< ExplicitScaleData >& rScales, bool /* bSwapXAndYAxis */ )
{
    OSL_ENSURE(m_nDimension<=static_cast<sal_Int32>(rScales.size()),"Dimension of Plotter does not fit two dimension of given scale sequence");
    m_pPosHelper->setScales( rScales, true );
}

drawing::Direction3D PieChart::getPreferredDiagramAspectRatio() const
{
    if( m_nDimension == 3 )
        return drawing::Direction3D(1,1,0.10);
    return drawing::Direction3D(1,1,1);
}

bool PieChart::keepAspectRatio() const
{
    if( m_nDimension == 3 )
        return false;
    return true;
}

bool PieChart::shouldSnapRectToUsedArea()
{
    return true;
}

uno::Reference< drawing::XShape > PieChart::createDataPoint(
          const uno::Reference< drawing::XShapes >& xTarget
        , const uno::Reference< beans::XPropertySet >& xObjectProperties
        , double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree
        , double fUnitCircleInnerRadius, double fUnitCircleOuterRadius
        , double fLogicZ, double fDepth, double fExplodePercentage
        , tPropertyNameValueMap* pOverwritePropertiesMap )
{
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

void PieChart::addSeries( VDataSeries* pSeries, sal_Int32 /* zSlot */, sal_Int32 /* xSlot */, sal_Int32 /* ySlot */ )
{
    VSeriesPlotter::addSeries( pSeries, 0, -1, 0 );
}

double PieChart::getMinimumX()
{
    return 0.5;
}
double PieChart::getMaxOffset()
{
    if (!::rtl::math::isNan(m_fMaxOffset))
        // Value already cached.  Use it.
        return m_fMaxOffset;

    m_fMaxOffset = 0.0;
    if( m_aZSlots.size()<=0 )
        return m_fMaxOffset;
    if( m_aZSlots[0].size()<=0 )
        return m_fMaxOffset;

    const ::std::vector< VDataSeries* >& rSeriesList( m_aZSlots[0][0].m_aSeriesVector );
    if( rSeriesList.size()<=0 )
        return m_fMaxOffset;

    VDataSeries* pSeries = rSeriesList[0];
    uno::Reference< beans::XPropertySet > xSeriesProp( pSeries->getPropertiesOfSeries() );
    if( !xSeriesProp.is() )
        return m_fMaxOffset;

    double fExplodePercentage=0.0;
    xSeriesProp->getPropertyValue( "Offset") >>= fExplodePercentage;
    if(fExplodePercentage>m_fMaxOffset)
        m_fMaxOffset=fExplodePercentage;

    if(!m_bSizeExcludesLabelsAndExplodedSegments)
    {
        uno::Sequence< sal_Int32 > aAttributedDataPointIndexList;
        if( xSeriesProp->getPropertyValue( "AttributedDataPoints" ) >>= aAttributedDataPointIndexList )
        {
            for(sal_Int32 nN=aAttributedDataPointIndexList.getLength();nN--;)
            {
                uno::Reference< beans::XPropertySet > xPointProp( pSeries->getPropertiesOfPoint(aAttributedDataPointIndexList[nN]) );
                if(xPointProp.is())
                {
                    fExplodePercentage=0.0;
                    xPointProp->getPropertyValue( "Offset") >>= fExplodePercentage;
                    if(fExplodePercentage>m_fMaxOffset)
                        m_fMaxOffset=fExplodePercentage;
                }
            }
        }
    }
    return m_fMaxOffset;
}
double PieChart::getMaximumX()
{
    double fMaxOffset = getMaxOffset();
    if( m_aZSlots.size()>0 && m_bUseRings)
        return m_aZSlots[0].size()+0.5+fMaxOffset;
    return 1.5+fMaxOffset;
}
double PieChart::getMinimumYInRange( double /* fMinimumX */, double /* fMaximumX */, sal_Int32 /* nAxisIndex */ )
{
    return 0.0;
}

double PieChart::getMaximumYInRange( double /* fMinimumX */, double /* fMaximumX */, sal_Int32 /* nAxisIndex */ )
{
    return 1.0;
}

bool PieChart::isExpandBorderToIncrementRhythm( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

bool PieChart::isExpandIfValuesCloseToBorder( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

bool PieChart::isExpandWideValuesToZero( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

bool PieChart::isExpandNarrowValuesTowardZero( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

bool PieChart::isSeparateStackingForDifferentSigns( sal_Int32 /* nDimensionIndex */ )
{
    return false;
}

void PieChart::createShapes()
{
    if( m_aZSlots.begin() == m_aZSlots.end() ) //no series
        return;

    OSL_ENSURE(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"PieChart is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;

    //the text labels should be always on top of the other series shapes
    //therefore create an own group for the texts to move them to front
    //(because the text group is created after the series group the texts are displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget,OUString() ));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget,OUString() ));
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aZSlots[0].begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aZSlots[0].end();

    ::std::vector< VDataSeriesGroup >::size_type nExplodeableSlot = 0;
    if( m_pPosHelper->isMathematicalOrientationRadius() && m_bUseRings )
        nExplodeableSlot = m_aZSlots[0].size()-1;

    m_aLabelInfoList.clear();
    ::rtl::math::setNan(&m_fMaxOffset);
    sal_Int32 n3DRelativeHeight = 100;
    uno::Reference< beans::XPropertySet > xPropertySet( m_xChartTypeModel, uno::UNO_QUERY );
    if ( (m_nDimension==3) && xPropertySet.is())
    {
        try
        {
            uno::Any aAny = xPropertySet->getPropertyValue( "3DRelativeHeight" );
            aAny >>= n3DRelativeHeight;
        }
        catch (const uno::Exception&) { }
    }

    for( double fSlotX=0; aXSlotIter != aXSlotEnd && (m_bUseRings||fSlotX<0.5 ); ++aXSlotIter, fSlotX+=1.0 )
    {
        ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
        if( pSeriesList->size()<=0 )//there should be only one series in each x slot
            continue;
        VDataSeries* pSeries = (*pSeriesList)[0];
        if(!pSeries)
            continue;

        m_pPosHelper->m_fAngleDegreeOffset = pSeries->getStartingAngle();

        double fLogicYSum = 0.0;
        //iterate through all points to get the sum
        sal_Int32 nPointIndex=0;
        sal_Int32 nPointCount=pSeries->getTotalPointCount();
        for( nPointIndex = 0; nPointIndex < nPointCount; nPointIndex++ )
        {
            double fY = pSeries->getYValue( nPointIndex );
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
        for( nPointIndex = 0; nPointIndex < nPointCount; nPointIndex++ )
        {
            double fLogicInnerRadius, fLogicOuterRadius;
            double fOffset = getMaxOffset();
            bool bIsVisible = m_pPosHelper->getInnerAndOuterRadius( fSlotX+1.0, fLogicInnerRadius, fLogicOuterRadius, m_bUseRings, fOffset );
            if( !bIsVisible )
                continue;

            double fDepth  = this->getTransformedDepth() * (n3DRelativeHeight / 100.0);

            uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(pSeries, xSeriesTarget);
            //collect data point information (logic coordinates, style ):
            double fLogicYValue = fabs(pSeries->getYValue( nPointIndex ));
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
                    xPointProperties->getPropertyValue( "Offset") >>= fExplodePercentage;
                }
                catch( const uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                }

                //transforme to unit circle:
                double fUnitCircleWidthAngleDegree = m_pPosHelper->getWidthAngleDegree( fLogicStartAngleValue, fLogicEndAngleValue );
                double fUnitCircleStartAngleDegree = m_pPosHelper->transformToAngleDegree( fLogicStartAngleValue );
                double fUnitCircleInnerRadius = m_pPosHelper->transformToRadius( fLogicInnerRadius );
                double fUnitCircleOuterRadius = m_pPosHelper->transformToRadius( fLogicOuterRadius );

                //point color:
                boost::scoped_ptr< tPropertyNameValueMap > apOverwritePropertiesMap(NULL);
                {
                    if(!pSeries->hasPointOwnColor(nPointIndex) && m_xColorScheme.is())
                    {
                        apOverwritePropertiesMap.reset( new tPropertyNameValueMap() );
                        (*apOverwritePropertiesMap)["FillColor"] = uno::makeAny(
                            m_xColorScheme->getColorByIndex( nPointIndex ));
                    }
                }

                //create data point
                double fLogicZ = -1.0; // For 3D pie chart label position
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

                    sal_Int32 nLabelPlacement = pSeries->getLabelPlacement( nPointIndex, m_xChartTypeModel, m_nDimension, m_pPosHelper->isSwapXAndY() );

                    // AVOID_OVERLAP is in fact "Best fit" in the UI.
                    bool bMovementAllowed = ( nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::AVOID_OVERLAP );
                    if( bMovementAllowed )
                        // Use center for "Best fit" for now. In the future we
                        // may want to implement a real best fit algorithm.
                        // But center is good enough, and close to what Excel
                        // does.
                        nLabelPlacement = ::com::sun::star::chart::DataLabelPlacement::CENTER;

                    LabelAlignment eAlignment(LABEL_ALIGN_CENTER);
                    sal_Int32 nScreenValueOffsetInRadiusDirection = 0 ;
                    if( nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::OUTSIDE )
                        nScreenValueOffsetInRadiusDirection = (3!=m_nDimension) ? 150 : 0;//todo maybe calculate this font height dependent
                    else if( nLabelPlacement == ::com::sun::star::chart::DataLabelPlacement::INSIDE )
                        nScreenValueOffsetInRadiusDirection = (3!=m_nDimension) ? -150 : 0;//todo maybe calculate this font height dependent
                    PolarLabelPositionHelper aPolarPosHelper(m_pPosHelper,m_nDimension,m_xLogicTarget,m_pShapeFactory);
                    awt::Point aScreenPosition2D(
                        aPolarPosHelper.getLabelScreenPositionAndAlignmentForUnitCircleValues(eAlignment, nLabelPlacement
                        , fUnitCircleStartAngleDegree, fUnitCircleWidthAngleDegree
                        , fUnitCircleInnerRadius, fUnitCircleOuterRadius, fLogicZ+0.5, 0 ));

                    PieLabelInfo aPieLabelInfo;
                    aPieLabelInfo.aFirstPosition = basegfx::B2IVector( aScreenPosition2D.X, aScreenPosition2D.Y );
                    awt::Point aOrigin( aPolarPosHelper.transformSceneToScreenPosition( m_pPosHelper->transformUnitCircleToScene( 0.0, 0.0, fLogicZ+1.0 ) ) );
                    aPieLabelInfo.aOrigin = basegfx::B2IVector( aOrigin.X, aOrigin.Y );

                    //add a scaling independent Offset if requested
                    if( nScreenValueOffsetInRadiusDirection != 0)
                    {
                        basegfx::B2IVector aDirection( aScreenPosition2D.X- aOrigin.X, aScreenPosition2D.Y- aOrigin.Y );
                        aDirection.setLength(nScreenValueOffsetInRadiusDirection);
                        aScreenPosition2D.X += aDirection.getX();
                        aScreenPosition2D.Y += aDirection.getY();
                    }

                    aPieLabelInfo.xTextShape = this->createDataLabel( xTextTarget, *pSeries, nPointIndex
                                    , fLogicYValue, fLogicYSum, aScreenPosition2D, eAlignment );

                    uno::Reference< container::XChild > xChild( aPieLabelInfo.xTextShape, uno::UNO_QUERY );
                    if( xChild.is() )
                        aPieLabelInfo.xLabelGroupShape = uno::Reference<drawing::XShape>( xChild->getParent(), uno::UNO_QUERY );
                    aPieLabelInfo.fValue = fLogicYValue;
                    aPieLabelInfo.bMovementAllowed = bMovementAllowed;
                    aPieLabelInfo.bMoved= false;
                    aPieLabelInfo.xTextTarget = xTextTarget;
                    m_aLabelInfoList.push_back(aPieLabelInfo);
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
                    OUString aPointCIDStub( ObjectIdentifier::createSeriesSubObjectStub( OBJECTTYPE_DATA_POINT
                        , pSeries->getSeriesParticle()
                        , ObjectIdentifier::getPieSegmentDragMethodServiceName()
                        , ObjectIdentifier::createPieSegmentDragParameterString(
                            nOffsetPercent, aMinimumPosition, aMaximumPosition )
                        ) );

                    ShapeFactory::setShapeName( xPointShape
                                , ObjectIdentifier::createPointCID( aPointCIDStub, nPointIndex ) );
                }
                catch( const uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                }
            }//next series in x slot (next y slot)
        }//next category
    }//next x slot
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

namespace
{

::basegfx::B2IRectangle lcl_getRect( const uno::Reference< drawing::XShape >& xShape )
{
    ::basegfx::B2IRectangle aRect;
    if( xShape.is() )
        aRect = BaseGFXHelper::makeRectangle(xShape->getPosition(),xShape->getSize() );
    return aRect;
}

bool lcl_isInsidePage( const awt::Point& rPos, const awt::Size& rSize, const awt::Size& rPageSize )
{
    if( rPos.X < 0  || rPos.Y < 0 )
        return false;
    if( (rPos.X + rSize.Width) > rPageSize.Width  )
        return false;
    if( (rPos.Y + rSize.Height) > rPageSize.Height )
        return false;
    return true;
}

}//end anonymous namespace

PieChart::PieLabelInfo::PieLabelInfo()
    : xTextShape(0), xLabelGroupShape(0), aFirstPosition(), aOrigin(), fValue(0.0)
    , bMovementAllowed(false), bMoved(false), xTextTarget(0), pPrevious(0),pNext(0)
{
}

bool PieChart::PieLabelInfo::moveAwayFrom( const PieChart::PieLabelInfo* pFix, const awt::Size& rPageSize, bool bMoveHalfWay, bool bMoveClockwise, bool bAlternativeMoveDirection )
{
    //return true if the move was successful
    if(!this->bMovementAllowed)
        return false;

    const sal_Int32 nLabelDistanceX = rPageSize.Width/50;
    const sal_Int32 nLabelDistanceY = rPageSize.Height/50;

    ::basegfx::B2IRectangle aOverlap( lcl_getRect( this->xLabelGroupShape ) );
    aOverlap.intersect( lcl_getRect( pFix->xLabelGroupShape ) );
    if( !aOverlap.isEmpty() )
    {
        (void)bAlternativeMoveDirection;//todo

        basegfx::B2IVector aRadiusDirection = this->aFirstPosition - this->aOrigin;
        aRadiusDirection.setLength(1.0);
        basegfx::B2IVector aTangentialDirection( -aRadiusDirection.getY(), aRadiusDirection.getX() );
        bool bShiftHorizontal = abs(aTangentialDirection.getX()) > abs(aTangentialDirection.getY());

        sal_Int32 nShift = bShiftHorizontal ? static_cast<sal_Int32>(aOverlap.getWidth()) : static_cast<sal_Int32>(aOverlap.getHeight());
        nShift += (bShiftHorizontal ? nLabelDistanceX : nLabelDistanceY);
        if( bMoveHalfWay )
            nShift/=2;
        if(!bMoveClockwise)
            nShift*=-1;
        awt::Point aOldPos( this->xLabelGroupShape->getPosition() );
        basegfx::B2IVector aNewPos = basegfx::B2IVector( aOldPos.X, aOldPos.Y ) + nShift*aTangentialDirection;

        //check whether the new position is ok
        awt::Point aNewAWTPos( aNewPos.getX(), aNewPos.getY() );
        if( !lcl_isInsidePage( aNewAWTPos, this->xLabelGroupShape->getSize(), rPageSize ) )
            return false;

        this->xLabelGroupShape->setPosition( aNewAWTPos );
        this->bMoved = true;
    }
    return true;
}

void PieChart::resetLabelPositionsToPreviousState()
{
    std::vector< PieLabelInfo >::iterator aIt = m_aLabelInfoList.begin();
    std::vector< PieLabelInfo >::const_iterator aEnd = m_aLabelInfoList.end();
    for( ;aIt!=aEnd; ++aIt )
        aIt->xLabelGroupShape->setPosition(aIt->aPreviousPosition);
}

bool PieChart::detectLabelOverlapsAndMove( const awt::Size& rPageSize )
{
    //returns true when there might be more to do

    //find borders of a group of overlapping labels
    bool bOverlapFound = false;
    PieLabelInfo* pStart = &(*(m_aLabelInfoList.rbegin()));
    PieLabelInfo* pFirstBorder = 0;
    PieLabelInfo* pSecondBorder = 0;
    PieLabelInfo* pCurrent = pStart;
    do
    {
        ::basegfx::B2IRectangle aPreviousOverlap( lcl_getRect( pCurrent->xLabelGroupShape ) );
        ::basegfx::B2IRectangle aNextOverlap( aPreviousOverlap );
        aPreviousOverlap.intersect( lcl_getRect( pCurrent->pPrevious->xLabelGroupShape ) );
        aNextOverlap.intersect( lcl_getRect( pCurrent->pNext->xLabelGroupShape ) );

        bool bPreviousOverlap = !aPreviousOverlap.isEmpty();
        bool bNextOverlap = !aNextOverlap.isEmpty();
        if( bPreviousOverlap || bNextOverlap )
            bOverlapFound = true;
        if( !bPreviousOverlap && bNextOverlap )
        {
            pFirstBorder = pCurrent;
            break;
        }
        pCurrent = pCurrent->pNext;
    }
    while( pCurrent != pStart );

    if( !bOverlapFound )
        return false;

    if( pFirstBorder )
    {
        pCurrent = pFirstBorder;
        do
        {
            ::basegfx::B2IRectangle aPreviousOverlap( lcl_getRect( pCurrent->xLabelGroupShape ) );
            ::basegfx::B2IRectangle aNextOverlap( aPreviousOverlap );
            aPreviousOverlap.intersect( lcl_getRect( pCurrent->pPrevious->xLabelGroupShape ) );
            aNextOverlap.intersect( lcl_getRect( pCurrent->pNext->xLabelGroupShape ) );

            if( !aPreviousOverlap.isEmpty() && aNextOverlap.isEmpty() )
            {
                pSecondBorder = pCurrent;
                break;
            }
            pCurrent = pCurrent->pNext;
        }
        while( pCurrent != pFirstBorder );
    }

    if( !pFirstBorder || !pSecondBorder )
    {
        pFirstBorder = &(*(m_aLabelInfoList.rbegin()));
        pSecondBorder = &(*(m_aLabelInfoList.begin()));
    }

    //find center
    PieLabelInfo* pCenter = pFirstBorder;
    sal_Int32 nOverlapGroupCount = 1;
    for( pCurrent = pFirstBorder ;pCurrent != pSecondBorder; pCurrent = pCurrent->pNext )
        nOverlapGroupCount++;
    sal_Int32 nCenterPos = nOverlapGroupCount/2;
    bool bSingleCenter = nOverlapGroupCount%2 != 0;
    if( bSingleCenter )
        nCenterPos++;
    if(nCenterPos>1)
    {
        pCurrent = pFirstBorder;
        while( --nCenterPos )
            pCurrent = pCurrent->pNext;
        pCenter = pCurrent;
    }

    //remind current positions
    pCurrent = pStart;
    do
    {
        pCurrent->aPreviousPosition = pCurrent->xLabelGroupShape->getPosition();
        pCurrent = pCurrent->pNext;
    }
    while( pCurrent != pStart );

    //
    bool bAlternativeMoveDirection = false;
    if( !tryMoveLabels( pFirstBorder, pSecondBorder, pCenter, bSingleCenter, bAlternativeMoveDirection, rPageSize ) )
        tryMoveLabels( pFirstBorder, pSecondBorder, pCenter, bSingleCenter, bAlternativeMoveDirection, rPageSize );
    return true;
}

bool PieChart::tryMoveLabels( PieLabelInfo* pFirstBorder, PieLabelInfo* pSecondBorder
                             , PieLabelInfo* pCenter
                             , bool bSingleCenter, bool& rbAlternativeMoveDirection, const awt::Size& rPageSize )
{
    PieLabelInfo* p1 = bSingleCenter ? pCenter->pPrevious : pCenter;
    PieLabelInfo* p2 = pCenter->pNext;
    //return true when successful

    bool bLabelOrderIsAntiClockWise = m_pPosHelper->isMathematicalOrientationAngle();

    PieLabelInfo* pCurrent = 0;
    for( pCurrent = p2 ;pCurrent->pPrevious != pSecondBorder; pCurrent = pCurrent->pNext )
    {
        PieLabelInfo* pFix = 0;
        for( pFix = p2->pPrevious ;pFix != pCurrent; pFix = pFix->pNext )
        {
            if( !pCurrent->moveAwayFrom( pFix, rPageSize, !bSingleCenter && pCurrent == p2, !bLabelOrderIsAntiClockWise, rbAlternativeMoveDirection ) )
            {
                if( !rbAlternativeMoveDirection )
                {
                    rbAlternativeMoveDirection = true;
                    resetLabelPositionsToPreviousState();
                    return false;
                }
            }
        }
    }
    for( pCurrent = p1 ;pCurrent->pNext != pFirstBorder; pCurrent = pCurrent->pPrevious )
    {
        PieLabelInfo* pFix = 0;
        for( pFix = p2->pNext ;pFix != pCurrent; pFix = pFix->pPrevious )
        {
            if( !pCurrent->moveAwayFrom( pFix, rPageSize, false, bLabelOrderIsAntiClockWise, rbAlternativeMoveDirection ) )
            {
                if( !rbAlternativeMoveDirection )
                {
                    rbAlternativeMoveDirection = true;
                    resetLabelPositionsToPreviousState();
                    return false;
                }
            }
        }
    }
    return true;
}

void PieChart::rearrangeLabelToAvoidOverlapIfRequested( const awt::Size& rPageSize )
{
    //check whether there are any labels that should be moved
    std::vector< PieLabelInfo >::iterator aIt1 = m_aLabelInfoList.begin();
    std::vector< PieLabelInfo >::const_iterator aEnd = m_aLabelInfoList.end();
    bool bMoveableFound = false;
    for( ;aIt1!=aEnd; ++aIt1 )
    {
        if(aIt1->bMovementAllowed)
        {
            bMoveableFound = true;
            break;
        }
    }
    if(!bMoveableFound)
        return;

    double fPageDiagonaleLength = sqrt( double( rPageSize.Width*rPageSize.Width + rPageSize.Height*rPageSize.Height) );
    if( ::rtl::math::approxEqual( fPageDiagonaleLength, 0.0 ) )
        return;

    //init next and previous
    aIt1 = m_aLabelInfoList.begin();
    std::vector< PieLabelInfo >::iterator aIt2 = aIt1;
    if( aIt1==aEnd )//no need to do anything when we only have one label
        return;
    aIt1->pPrevious = &(*(m_aLabelInfoList.rbegin()));
    ++aIt2;
    for( ;aIt2!=aEnd; ++aIt1, ++aIt2 )
    {
        PieLabelInfo& rInfo1( *aIt1 );
        PieLabelInfo& rInfo2( *aIt2 );
        rInfo1.pNext = &rInfo2;
        rInfo2.pPrevious = &rInfo1;
    }
    aIt1->pNext = &(*(m_aLabelInfoList.begin()));

    //detect overlaps and move
    sal_Int32 nMaxIterations = 50;
    while( detectLabelOverlapsAndMove( rPageSize ) && nMaxIterations > 0 )
        nMaxIterations--;

    //create connection lines for the moved labels
    aEnd = m_aLabelInfoList.end();
    VLineProperties aVLineProperties;
    for( aIt1 = m_aLabelInfoList.begin(); aIt1!=aEnd; ++aIt1 )
    {
        PieLabelInfo& rInfo( *aIt1 );
        if( rInfo.bMoved )
        {
            sal_Int32 nX1 = rInfo.aFirstPosition.getX();
            sal_Int32 nY1 = rInfo.aFirstPosition.getY();
            sal_Int32 nX2 = nX1;
            sal_Int32 nY2 = nY1;
            ::basegfx::B2IRectangle aRect( lcl_getRect( rInfo.xLabelGroupShape ) );
            if( nX1 < aRect.getMinX() )
                nX2 = aRect.getMinX();
            else if( nX1 > aRect.getMaxX() )
                nX2 = aRect.getMaxX();

            if( nY1 < aRect.getMinY() )
                nY2 = aRect.getMinY();
            else if( nY1 > aRect.getMaxY() )
                nY2 = aRect.getMaxY();

            //when the line is very short compared to the page size don't create one
            ::basegfx::B2DVector aLength(nX1-nX2, nY1-nY2);
            if( (aLength.getLength()/fPageDiagonaleLength) < 0.01 )
                continue;

            drawing::PointSequenceSequence aPoints(1);
            aPoints[0].realloc(2);
            aPoints[0][0].X = nX1;
            aPoints[0][0].Y = nY1;
            aPoints[0][1].X = nX2;
            aPoints[0][1].Y = nY2;

            uno::Reference< beans::XPropertySet > xProp( rInfo.xTextShape, uno::UNO_QUERY);
            if( xProp.is() )
            {
                sal_Int32 nColor = 0;
                xProp->getPropertyValue("CharColor") >>= nColor;
                if( nColor != -1 )//automatic font color does not work for lines -> fallback to black
                    aVLineProperties.Color = uno::makeAny(nColor);
            }
            m_pShapeFactory->createLine2D( rInfo.xTextTarget, aPoints, &aVLineProperties );
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
