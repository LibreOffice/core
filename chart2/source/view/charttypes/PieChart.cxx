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
#include "AbstractShapeFactory.hxx"
#include "PolarLabelPositionHelper.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "ObjectIdentifier.hxx"

#include <com/sun/star/chart/DataLabelPlacement.hpp>
#include <com/sun/star/chart2/XColorScheme.hpp>

#include <com/sun/star/container/XChild.hpp>
#include <rtl/math.hxx>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace chart {

struct PieChart::ShapeParam
{
    /** the start angle of the slice
     */
    double mfUnitCircleStartAngleDegree;

    /** the angle width of the slice
     */
    double mfUnitCircleWidthAngleDegree;

    /** the normalized outer radius of the ring the slice belongs to.
     */
    double mfUnitCircleOuterRadius;

    /** the normalized inner radius of the ring the slice belongs to
     */
    double mfUnitCircleInnerRadius;

    /** relative distance offset of a slice from the pie center;
     *  this parameter is used for instance when the user performs manual
     *  dragging of a slice (the drag operation is possible only for slices that
     *  belong to the outer ring and only along the ray bisecting the slice);
     *  the value for the given entry in the data series is obtained by the
     *  `Offset` property attached to each entry; note that the value
     *  provided by the `Offset` property is used both as a logical value in
     *  `PiePositionHelper::getInnerAndOuterRadius` and as a percentage value in
     *  the `PieChart::createDataPoint` and `PieChart::createTextLabelShape`
     *  methods; since the logical height of a ring is always 1, this duality
     *  does not cause any incorrect behavior;
     */
    double mfExplodePercentage;

    /** sum of all Y values in a single series
     */
    double mfLogicYSum;

    /** for 3D pie chart: label z coordinate
     */
    double mfLogicZ;

    /** for 3D pie chart: height
     */
    double mfDepth;

    ShapeParam() :
        mfUnitCircleStartAngleDegree(0.0),
        mfUnitCircleWidthAngleDegree(0.0),
        mfUnitCircleOuterRadius(0.0),
        mfUnitCircleInnerRadius(0.0),
        mfExplodePercentage(0.0),
        mfLogicYSum(0.0),
        mfLogicZ(0.0),
        mfDepth(0.0) {}
};

class PiePositionHelper : public PolarPlottingPositionHelper
{
public:
    PiePositionHelper( NormalAxis eNormalAxis, double fAngleDegreeOffset );

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

/** Compute the outer and the inner radius for the current ring (not for the
 *  whole donut!), in general it is:
 *      inner_radius = (ring_index + 1) - 0.5 + max_offset,
 *      outer_radius = (ring_index + 1) + 0.5 + max_offset.
 *  When orientation for the radius axis is reversed these values are swapped.
 *  (Indeed the orientation for the radius axis is always reversed!
 *  See `PieChartTypeTemplate::adaptScales`.)
 *  The maximum relative offset (see notes for `PieChart::getMaxOffset`) is
 *  added to both the inner and the outer radius.
 *  It returns true if the ring is visible (that is not out of the radius
 *  axis scale range).
 */
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
        //in this case the given getMaximumX() was not correct instead the minimum should have been smaller by fMaxOffset
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

    PlotterBase::m_pPosHelper = m_pPosHelper.get();
    VSeriesPlotter::m_pMainPosHelper = m_pPosHelper.get();
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

bool PieChart::shouldSnapRectToUsedArea()
{
    return true;
}

uno::Reference< drawing::XShape > PieChart::createDataPoint(
    const uno::Reference<drawing::XShapes>& xTarget,
    const uno::Reference<beans::XPropertySet>& xObjectProperties,
    tPropertyNameValueMap* pOverwritePropertiesMap,
    const ShapeParam& rParam )
{
    //transform position:
    drawing::Direction3D aOffset;
    if (rParam.mfExplodePercentage != 0.0)
    {
        double fAngle  = rParam.mfUnitCircleStartAngleDegree + rParam.mfUnitCircleWidthAngleDegree/2.0;
        double fRadius = (rParam.mfUnitCircleOuterRadius-rParam.mfUnitCircleInnerRadius)*rParam.mfExplodePercentage;
        drawing::Position3D aOrigin = m_pPosHelper->transformUnitCircleToScene(0, 0, rParam.mfLogicZ);
        drawing::Position3D aNewOrigin = m_pPosHelper->transformUnitCircleToScene(fAngle, fRadius, rParam.mfLogicZ);
        aOffset = aNewOrigin - aOrigin;
    }

    //create point
    uno::Reference< drawing::XShape > xShape(nullptr);
    if(m_nDimension==3)
    {
        xShape = m_pShapeFactory->createPieSegment( xTarget
            , rParam.mfUnitCircleStartAngleDegree, rParam.mfUnitCircleWidthAngleDegree
            , rParam.mfUnitCircleInnerRadius, rParam.mfUnitCircleOuterRadius
            , aOffset, B3DHomMatrixToHomogenMatrix( m_pPosHelper->getUnitCartesianToScene() )
            , rParam.mfDepth );
    }
    else
    {
        xShape = m_pShapeFactory->createPieSegment2D( xTarget
            , rParam.mfUnitCircleStartAngleDegree, rParam.mfUnitCircleWidthAngleDegree
            , rParam.mfUnitCircleInnerRadius, rParam.mfUnitCircleOuterRadius
            , aOffset, B3DHomMatrixToHomogenMatrix( m_pPosHelper->getUnitCartesianToScene() ) );
    }
    setMappedProperties( xShape, xObjectProperties, PropertyMapper::getPropertyNameMapForFilledSeriesProperties(), pOverwritePropertiesMap );
    return xShape;
}

void PieChart::createTextLabelShape(
    const uno::Reference<drawing::XShapes>& xTextTarget,
    VDataSeries& rSeries, sal_Int32 nPointIndex, ShapeParam& rParam )
{
    if (!rSeries.getDataPointLabelIfLabel(nPointIndex))
        // There is no text label for this data point.  Nothing to do.
        return;

    ///by using the `mfExplodePercentage` parameter a normalized offset is added
    ///to both normalized radii. (See notes for
    ///`PolarPlottingPositionHelper::transformToRadius`, especially example 3,
    ///and related comments).
    if (rParam.mfExplodePercentage != 0.0)
    {
        double fExplodeOffset = (rParam.mfUnitCircleOuterRadius-rParam.mfUnitCircleInnerRadius)*rParam.mfExplodePercentage;
        rParam.mfUnitCircleInnerRadius += fExplodeOffset;
        rParam.mfUnitCircleOuterRadius += fExplodeOffset;
    }

    ///get the required label placement type. Available placements are
    ///`AVOID_OVERLAP`, `CENTER`, `OUTSIDE` and `INSIDE`;
    sal_Int32 nLabelPlacement = rSeries.getLabelPlacement(
        nPointIndex, m_xChartTypeModel, m_nDimension, m_pPosHelper->isSwapXAndY());

    ///when the placement is of `AVOID_OVERLAP` type a later rearrangement of
    ///the label position is allowed; the `createTextLabelShape` treats the
    ///`AVOID_OVERLAP` as if it was of `CENTER` type;

    //AVOID_OVERLAP is in fact "Best fit" in the UI.
    bool bMovementAllowed = ( nLabelPlacement == css::chart::DataLabelPlacement::AVOID_OVERLAP );
    if( bMovementAllowed )
        // Use center for "Best fit" for now. In the future we
        // may want to implement a real best fit algorithm.
        // But center is good enough, and close to what Excel
        // does.
        nLabelPlacement = css::chart::DataLabelPlacement::CENTER;

    ///for `OUTSIDE` (`INSIDE`) label placements an offset of 150 (-150), in the
    ///radius direction, is added to the final screen position of the label
    ///anchor point. This is required in order to ensure that the label is
    ///completely outside (inside) the related slice. Indeed this value should
    ///depend on the font height;
    ///pay attention: 150 is not a big offset, in fact the screen position
    ///coordinates for label anchor points are in the 10000-20000 range, hence
    ///these are coordinates of a virtual screen and 150 is a small value;
    LabelAlignment eAlignment(LABEL_ALIGN_CENTER);
    sal_Int32 nScreenValueOffsetInRadiusDirection = 0 ;
    if( nLabelPlacement == css::chart::DataLabelPlacement::OUTSIDE )
        nScreenValueOffsetInRadiusDirection = (3!=m_nDimension) ? 150 : 0;//todo maybe calculate this font height dependent
    else if( nLabelPlacement == css::chart::DataLabelPlacement::INSIDE )
        nScreenValueOffsetInRadiusDirection = (3!=m_nDimension) ? -150 : 0;//todo maybe calculate this font height dependent

    ///the scene position of the label anchor point is calculated (see notes for
    ///`PolarLabelPositionHelper::getLabelScreenPositionAndAlignmentForUnitCircleValues`),
    ///and immediately transformed into the screen position.
    PolarLabelPositionHelper aPolarPosHelper(m_pPosHelper.get(),m_nDimension,m_xLogicTarget,m_pShapeFactory);
    awt::Point aScreenPosition2D(
        aPolarPosHelper.getLabelScreenPositionAndAlignmentForUnitCircleValues(eAlignment, nLabelPlacement
        , rParam.mfUnitCircleStartAngleDegree, rParam.mfUnitCircleWidthAngleDegree
        , rParam.mfUnitCircleInnerRadius, rParam.mfUnitCircleOuterRadius, rParam.mfLogicZ+0.5, 0 ));

    ///the screen position of the pie/donut center is calculated.
    PieLabelInfo aPieLabelInfo;
    aPieLabelInfo.aFirstPosition = basegfx::B2IVector( aScreenPosition2D.X, aScreenPosition2D.Y );
    awt::Point aOrigin( aPolarPosHelper.transformSceneToScreenPosition( m_pPosHelper->transformUnitCircleToScene( 0.0, 0.0, rParam.mfLogicZ+1.0 ) ) );
    aPieLabelInfo.aOrigin = basegfx::B2IVector( aOrigin.X, aOrigin.Y );

    ///add a scaling independent Offset if requested
    if( nScreenValueOffsetInRadiusDirection != 0)
    {
        basegfx::B2IVector aDirection( aScreenPosition2D.X- aOrigin.X, aScreenPosition2D.Y- aOrigin.Y );
        aDirection.setLength(nScreenValueOffsetInRadiusDirection);
        aScreenPosition2D.X += aDirection.getX();
        aScreenPosition2D.Y += aDirection.getY();
    }

   // compute outer pie radius
    awt::Point aOuterCirclePoint = PlottingPositionHelper::transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene(
                    0,
                    rParam.mfUnitCircleOuterRadius,
                    0 ),
            m_xLogicTarget, m_pShapeFactory, m_nDimension );
    basegfx::B2IVector aRadiusVector(
            aOuterCirclePoint.X - aPieLabelInfo.aOrigin.getX(),
            aOuterCirclePoint.Y - aPieLabelInfo.aOrigin.getY() );
    double fSquaredPieRadius = aRadiusVector.scalar(aRadiusVector);
    double fPieRadius = sqrt( fSquaredPieRadius );

    // set the maximum text width to be used when text wrapping is enabled
    double fTextMaximumFrameWidth = 0.8 * fPieRadius;
    sal_Int32 nTextMaximumFrameWidth = ceil(fTextMaximumFrameWidth);

    ///the text shape for the label is created
    double nVal = rSeries.getYValue(nPointIndex);
    aPieLabelInfo.xTextShape = createDataLabel(
        xTextTarget, rSeries, nPointIndex, nVal, rParam.mfLogicYSum,
        aScreenPosition2D, eAlignment, 0, nTextMaximumFrameWidth);

    ///a new `PieLabelInfo` instance is initialized with all the info related to
    ///the current label in order to simplify later label position rearrangement;
    uno::Reference< container::XChild > xChild( aPieLabelInfo.xTextShape, uno::UNO_QUERY );
    if( xChild.is() )
        aPieLabelInfo.xLabelGroupShape.set( xChild->getParent(), uno::UNO_QUERY );
    aPieLabelInfo.fValue = nVal;
    aPieLabelInfo.bMovementAllowed = bMovementAllowed;
    aPieLabelInfo.bMoved= false;
    aPieLabelInfo.xTextTarget = xTextTarget;

    if (bMovementAllowed)
    {
        performLabelBestFit(rParam, aPieLabelInfo);
    }


    m_aLabelInfoList.push_back(aPieLabelInfo);

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
    ///a ZSlot is a vector< vector< VDataSeriesGroup > >. There is only one
    ///ZSlot: m_aZSlots[0] which has a number of elements equal to the total
    ///number of data series (in fact, even if m_aZSlots[0][i] is an object of
    ///type `VDataSeriesGroup`, in the current implementation, there is only one
    ///data series in each data series group).
    if (m_aZSlots.empty())
        // No series to plot.
        return;

    ///m_xLogicTarget is where the group of all data series shapes (e.g. a pie
    ///slice) is added (xSeriesTarget);

    ///m_xFinalTarget is where the group of all text shapes (labels) is added
    ///(xTextTarget).

    ///both have been already created and added to the same root shape
    ///( a member of a VDiagram object); this initialization occurs in
    ///`ChartView::impl_createDiagramAndContent`.

    OSL_ENSURE(m_pShapeFactory && m_xLogicTarget.is() && m_xFinalTarget.is(), "PieChart is not properly initialized.");
    if (!m_pShapeFactory || !m_xLogicTarget.is() || !m_xFinalTarget.is())
        return;

    ///the text labels should be always on top of the other series shapes
    ///therefore create an own group for the texts to move them to front
    ///(because the text group is created after the series group the texts are
    ///displayed on top)
    uno::Reference< drawing::XShapes > xSeriesTarget(
        createGroupShape( m_xLogicTarget ));
    uno::Reference< drawing::XShapes > xTextTarget(
        m_pShapeFactory->createGroup2D( m_xFinalTarget ));
    //check necessary here that different Y axis can not be stacked in the same group? ... hm?

    ///pay attention that the `m_bSwapXAndY` parameter used by the polar
    ///plotting position helper is always set to true for pie/donut charts
    ///(see PieChart::setScales). This fact causes that `createShapes` expects
    ///that the radius axis scale is the one with index 0 and the angle axis
    ///scale is the one with index 1.

    ::std::vector< VDataSeriesGroup >::iterator             aXSlotIter = m_aZSlots[0].begin();
    const ::std::vector< VDataSeriesGroup >::const_iterator aXSlotEnd = m_aZSlots[0].end();

    ///m_bUseRings == true if chart type is `donut`, == false if chart type is
    ///`pie`; if the chart is of `donut` type we have as many rings as many data
    ///series, else we have a single ring (a pie) representing the first data
    ///series;
    ///for what I can see the radius axis orientation is always reversed and
    ///the angle axis orientation is always non-reversed;
    ///the radius axis scale range is [0.5, number of rings + 0.5 + max_offset],
    ///the angle axis scale range is [0, 1]. The max_offset parameter is used
    ///for exploded pie chart and its value is 0.5.

    ///the `explodeable` ring is the first one except when the radius axis
    ///orientation is reversed (always!?) and we are dealing with a donut: in
    ///such a case the `explodeable` ring is the last one.
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
    ///iterate over each xslot, that is on each data series (there is
    ///only one data series in each data series group!); note that if the chart
    ///type is a pie the loop iterates only over the first data series
    ///(m_bUseRings||fSlotX<0.5)
    for( double fSlotX=0; aXSlotIter != aXSlotEnd && (m_bUseRings||fSlotX<0.5 ); ++aXSlotIter, fSlotX+=1.0 )
    {
        ShapeParam aParam;

        ::std::vector< VDataSeries* >* pSeriesList = &(aXSlotIter->m_aSeriesVector);
        if( pSeriesList->size()<=0 )//there should be only one series in each x slot
            continue;
        VDataSeries* pSeries = (*pSeriesList)[0];
        if(!pSeries)
            continue;

        bool bHasFillColorMapping = pSeries->hasPropertyMapping("FillColor");

        /// The angle degree offset is set by the same property of the
        /// data series.
        /// Counter-clockwise offset from the 3 o'clock position.
        m_pPosHelper->m_fAngleDegreeOffset = pSeries->getStartingAngle();

        ///iterate through all points to get the sum of all entries of
        ///the current data series
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
            aParam.mfLogicYSum += fabs(fY);
        }

        if (aParam.mfLogicYSum == 0.0)
            // Total sum of all Y values in this series is zero. Skip the whole series.
            continue;

        double fLogicYForNextPoint = 0.0;
        ///iterate through all points to create shapes
        for( nPointIndex = 0; nPointIndex < nPointCount; nPointIndex++ )
        {
            double fLogicInnerRadius, fLogicOuterRadius;

            ///compute the maximum relative distance offset of the current slice
            ///from the pie center
            ///it is worth noting that after the first invocation the maximum
            ///offset value is cached, so it is evaluated only once per each
            ///call to `createShapes`
            double fOffset = getMaxOffset();

            ///compute the outer and the inner radius for the current ring slice
            bool bIsVisible = m_pPosHelper->getInnerAndOuterRadius( fSlotX+1.0, fLogicInnerRadius, fLogicOuterRadius, m_bUseRings, fOffset );
            if( !bIsVisible )
                continue;

            aParam.mfDepth  = this->getTransformedDepth() * (n3DRelativeHeight / 100.0);

            uno::Reference< drawing::XShapes > xSeriesGroupShape_Shapes = getSeriesGroupShape(pSeries, xSeriesTarget);
            ///collect data point information (logic coordinates, style ):
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
                double fLogicStartAngleValue = fLogicYPos / aParam.mfLogicYSum;
                double fLogicEndAngleValue = (fLogicYPos+fLogicYValue) / aParam.mfLogicYSum;

                ///note that the explode percentage is set to the `Offset`
                ///property of the current data series entry only for slices
                ///belonging to the outer ring
                aParam.mfExplodePercentage = 0.0;
                bool bDoExplode = ( nExplodeableSlot == static_cast< ::std::vector< VDataSeriesGroup >::size_type >(fSlotX) );
                if(bDoExplode) try
                {
                    xPointProperties->getPropertyValue( "Offset") >>= aParam.mfExplodePercentage;
                }
                catch( const uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                }

                ///see notes for `PolarPlottingPositionHelper` methods
                ///transform to unit circle:
                aParam.mfUnitCircleWidthAngleDegree = m_pPosHelper->getWidthAngleDegree( fLogicStartAngleValue, fLogicEndAngleValue );
                aParam.mfUnitCircleStartAngleDegree = m_pPosHelper->transformToAngleDegree( fLogicStartAngleValue );
                aParam.mfUnitCircleInnerRadius = m_pPosHelper->transformToRadius( fLogicInnerRadius );
                aParam.mfUnitCircleOuterRadius = m_pPosHelper->transformToRadius( fLogicOuterRadius );

                ///point color:
                std::unique_ptr< tPropertyNameValueMap > apOverwritePropertiesMap(nullptr);
                if (!pSeries->hasPointOwnColor(nPointIndex) && m_xColorScheme.is())
                {
                    apOverwritePropertiesMap.reset( new tPropertyNameValueMap() );
                    (*apOverwritePropertiesMap)["FillColor"] <<=
                        m_xColorScheme->getColorByIndex( nPointIndex );
                }

                ///create data point
                aParam.mfLogicZ = -1.0; // For 3D pie chart label position
                uno::Reference<drawing::XShape> xPointShape =
                    createDataPoint(
                        xSeriesGroupShape_Shapes, xPointProperties, apOverwritePropertiesMap.get(), aParam);

                if(bHasFillColorMapping)
                {
                    double nPropVal = pSeries->getValueByProperty(nPointIndex, "FillColor");
                    if(!rtl::math::isNan(nPropVal))
                    {
                        uno::Reference< beans::XPropertySet > xProps( xPointShape, uno::UNO_QUERY_THROW );
                        xProps->setPropertyValue("FillColor", uno::Any(static_cast<sal_Int32>( nPropVal)));
                    }
                }

                ///create label
                createTextLabelShape(xTextTarget, *pSeries, nPointIndex, aParam);

                if(!bDoExplode)
                {
                    AbstractShapeFactory::setShapeName( xPointShape
                                , ObjectIdentifier::createPointCID( pSeries->getPointCID_Stub(), nPointIndex ) );
                }
                else try
                {
                    ///enable dragging of outer segments

                    double fAngle  = aParam.mfUnitCircleStartAngleDegree + aParam.mfUnitCircleWidthAngleDegree/2.0;
                    double fMaxDeltaRadius = aParam.mfUnitCircleOuterRadius-aParam.mfUnitCircleInnerRadius;
                    drawing::Position3D aOrigin = m_pPosHelper->transformUnitCircleToScene( fAngle, aParam.mfUnitCircleOuterRadius, aParam.mfLogicZ );
                    drawing::Position3D aNewOrigin = m_pPosHelper->transformUnitCircleToScene( fAngle, aParam.mfUnitCircleOuterRadius + fMaxDeltaRadius, aParam.mfLogicZ );

                    sal_Int32 nOffsetPercent( static_cast<sal_Int32>(aParam.mfExplodePercentage * 100.0) );

                    awt::Point aMinimumPosition( PlottingPositionHelper::transformSceneToScreenPosition(
                        aOrigin, m_xLogicTarget, m_pShapeFactory, m_nDimension ) );
                    awt::Point aMaximumPosition( PlottingPositionHelper::transformSceneToScreenPosition(
                        aNewOrigin, m_xLogicTarget, m_pShapeFactory, m_nDimension ) );

                    //enable dragging of piesegments
                    OUString aPointCIDStub( ObjectIdentifier::createSeriesSubObjectStub( OBJECTTYPE_DATA_POINT
                        , pSeries->getSeriesParticle()
                        , ObjectIdentifier::getPieSegmentDragMethodServiceName()
                        , ObjectIdentifier::createPieSegmentDragParameterString(
                            nOffsetPercent, aMinimumPosition, aMaximumPosition )
                        ) );

                    AbstractShapeFactory::setShapeName( xPointShape
                                , ObjectIdentifier::createPointCID( aPointCIDStub, nPointIndex ) );
                }
                catch( const uno::Exception& e )
                {
                    ASSERT_EXCEPTION( e );
                }
            }//next series in x slot (next y slot)
        }//next category
    }//next x slot
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

inline
double lcl_radToDeg(double fAngleRad)
{
    return (fAngleRad / M_PI) * 180.0;
}

inline
double lcl_degToRad(double fAngleDeg)
{
    return (fAngleDeg / 180) * M_PI;
}

inline
double lcl_getDegAngleInStandardRange(double fAngle)
{
    while( fAngle < 0.0 )
        fAngle += 360.0;
    while( fAngle >= 360.0 )
        fAngle -= 360.0;
    return fAngle;
}

}//end anonymous namespace

PieChart::PieLabelInfo::PieLabelInfo()
    : xTextShape(nullptr), xLabelGroupShape(nullptr), aFirstPosition(), aOrigin(), fValue(0.0)
    , bMovementAllowed(false), bMoved(false), xTextTarget(nullptr), pPrevious(nullptr),pNext(nullptr)
{
}

/** In case this label and the passed label overlap the routine moves this
 *  label in order to fix the issue. After the label position has been
 *  rearranged it is checked that the moved label is still inside the page
 *  document, if the test is positive the routine returns true else returns
 *  false.
 */
bool PieChart::PieLabelInfo::moveAwayFrom( const PieChart::PieLabelInfo* pFix, const awt::Size& rPageSize, bool bMoveHalfWay, bool bMoveClockwise, bool bAlternativeMoveDirection )
{
    //return true if the move was successful
    if(!this->bMovementAllowed)
        return false;

    const sal_Int32 nLabelDistanceX = rPageSize.Width/50;
    const sal_Int32 nLabelDistanceY = rPageSize.Height/50;

    ///compute the rectangle representing the intersection of the label bounding
    ///boxes (`aOverlap`).
    ::basegfx::B2IRectangle aOverlap( lcl_getRect( this->xLabelGroupShape ) );
    aOverlap.intersect( lcl_getRect( pFix->xLabelGroupShape ) );
    if( !aOverlap.isEmpty() )
    {
        (void)bAlternativeMoveDirection;//todo

        ///the label is shifted along the direction orthogonal to the vector
        ///starting at the pie/donut center and ending at this label anchor
        ///point;

        ///named `aTangentialDirection` the unit vector related to such a
        ///direction, the magnitude of the shift along such a direction is
        ///calculated in this way: if the horizontal component of
        ///`aTangentialDirection` is greater than the vertical component,
        ///the magnitude of the shift is equal to `aOverlap.Width` else to
        ///`aOverlap.Height`;
        basegfx::B2IVector aRadiusDirection = this->aFirstPosition - this->aOrigin;
        aRadiusDirection.setLength(1.0);
        basegfx::B2IVector aTangentialDirection( -aRadiusDirection.getY(), aRadiusDirection.getX() );
        bool bShiftHorizontal = abs(aTangentialDirection.getX()) > abs(aTangentialDirection.getY());
        sal_Int32 nShift = bShiftHorizontal ? static_cast<sal_Int32>(aOverlap.getWidth()) : static_cast<sal_Int32>(aOverlap.getHeight());
        ///the magnitude of the shift is also increased by 1/50-th of the width
        ///or the height of the document page;
        nShift += (bShiftHorizontal ? nLabelDistanceX : nLabelDistanceY);
        ///in case the `bMoveHalfWay` parameter is true the magnitude of
        ///the shift is halved.
        if( bMoveHalfWay )
            nShift/=2;
        ///in case the `bMoveClockwise` parameter is false the direction of
        ///`aTangentialDirection` is reversed;
        if(!bMoveClockwise)
            nShift*=-1;
        awt::Point aOldPos( this->xLabelGroupShape->getPosition() );
        basegfx::B2IVector aNewPos = basegfx::B2IVector( aOldPos.X, aOldPos.Y ) + nShift*aTangentialDirection;

        ///a final check is performed in order to be sure that the moved label
        ///is still inside the page document;
        awt::Point aNewAWTPos( aNewPos.getX(), aNewPos.getY() );
        if( !lcl_isInsidePage( aNewAWTPos, this->xLabelGroupShape->getSize(), rPageSize ) )
            return false;

        this->xLabelGroupShape->setPosition( aNewAWTPos );
        this->bMoved = true;
    }
    return true;

    ///note that no further test is performed in order to check that the
    ///overlap is really fixed: this result is surely achieved if the shift
    ///would occur in the horizontal or vertical direction (since, in such a
    ///direction, the magnitude of the shift would be greater than the length
    ///of the overlap), but in general this is not true;
    ///adding a constant term equal to 1/50-th of the width or the height of
    ///the document page increases the probability of success, anyway it is
    ///worth noting that the method can return true even if the overlap issue
    ///is not (completely) fixed;
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
    ///the routine tries to individuate a chain of overlapping labels and
    ///assigns the first and the last of them to `pFirstBorder` and
    ///`pSecondBorder`;
    ///this result is achieved by performing two consecutive while loop.

    ///find borders of a group of overlapping labels

    ///a first while loop is started on the collection of `PieLabelInfo` objects;
    ///the bounding box of each label is checked for overlap against the bounding
    ///box of the previous and of the next label;
    ///when an overlap is found `bOverlapFound` is set to true, however the
    ///iteration is break only if the overlap occurs against only the next label
    ///and not against the previous label: so we exit from the loop whenever an
    ///overlap occurs except when the loop initial label overlaps with the
    ///previous one;
    bool bOverlapFound = false;
    PieLabelInfo* pStart = &(*(m_aLabelInfoList.rbegin()));
    PieLabelInfo* pFirstBorder = nullptr;
    PieLabelInfo* pSecondBorder = nullptr;
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

    ///in case we found a label (`pFirstBorder`) which overlaps with the next
    ///label and not with the previous label a second while loop is started with
    ///`pFirstBorder` as initial label; one more time the bounding box of each
    ///label is checked for overlap against the bounding box of the previous and
    ///of the next label, however this time we exit from the loop only if the
    ///current label overlaps with the previous one but does not with the next
    ///one (the opposite of what is required in the former loop);
    ///in case such a label is found it is assigned to `pSecondBorder` and the
    ///iteration is stopped; so in case there is a chain of overlapping labels
    ///we end up having the first label of the chain pointed by `pFirstBorder`
    ///and the last label of the chain pointed by `pSecondBorder`;
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

    ///when two labels satisfying the required conditions are not found
    ///(`pFirstBorder == 0 || pSecondBorder == 0`) but still an overlap occurs
    ///(`bOverlapFound == true`) we are in the situation where each label
    ///overlaps with both the previous and the next one; so `pFirstBorder` is
    ///set to point to the last `PieLabelInfo` object in the collection and
    ///`pSecondBorder` is set to point to the first one;
    if( !pFirstBorder || !pSecondBorder )
    {
        pFirstBorder = &(*(m_aLabelInfoList.rbegin()));
        pSecondBorder = &(*(m_aLabelInfoList.begin()));
    }

    ///the total number of labels that made up the chain is calculated and used
    ///for getting a pointer to the central label (`pCenter`);
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

    ///the current position of each label in the collection is saved in
    ///`PieLabelInfo.aPreviousPosition`, so that it is possible to undo the label
    ///move action if it is needed; the undo action is provided by the
    ///`PieChart::resetLabelPositionsToPreviousState` method.
    pCurrent = pStart;
    do
    {
        pCurrent->aPreviousPosition = pCurrent->xLabelGroupShape->getPosition();
        pCurrent = pCurrent->pNext;
    }
    while( pCurrent != pStart );

    ///the `PieChart::tryMoveLabels` method is invoked with
    ///`rbAlternativeMoveDirection` boolean parameter set to false, such a method
    ///tries to remove all overlaps that occur in the list of labels going from
    ///`pFirstBorder` to `pSecondBorder`;
    ///if the `PieChart::tryMoveLabels` returns true no further action is
    ///performed, however it is worth noting that it does not mean that all
    ///overlap issues have been surely fixed, but only that all moved labels are
    ///at least completely inside the page document;
    ///when `PieChart::tryMoveLabels` returns false, it means that the attempt
    ///to fix one of the overlap issues caused that a label has been moved
    ///(partially) outside the page document (anyway the `PieChart::tryMoveLabels`
    ///method takes care to restore the position of all labels to their initial
    ///position, and to set the `rbAlternativeMoveDirection` in/out parameter to
    ///true); in such a case a second invocation of `PieChart::tryMoveLabels` is
    ///performed (and this time the `rbAlternativeMoveDirection` boolean
    ///parameter is true) and independently by what the `PieChart::tryMoveLabels`
    ///method returns no further action is performed;
    ///(see notes for `PieChart::tryMoveLabels`);
    bool bAlternativeMoveDirection = false;
    if( !tryMoveLabels( pFirstBorder, pSecondBorder, pCenter, bSingleCenter, bAlternativeMoveDirection, rPageSize ) )
        tryMoveLabels( pFirstBorder, pSecondBorder, pCenter, bSingleCenter, bAlternativeMoveDirection, rPageSize );

    ///in both cases (one or two invocations of `PieChart::tryMoveLabels`) the
    ///`detectLabelOverlapsAndMove` method ends returning true.
    return true;
}


/** Try to remove all overlaps that occur in the list of labels going from
 *  `pFirstBorder` to `pSecondBorder`
 */
bool PieChart::tryMoveLabels( PieLabelInfo* pFirstBorder, PieLabelInfo* pSecondBorder
                             , PieLabelInfo* pCenter
                             , bool bSingleCenter, bool& rbAlternativeMoveDirection, const awt::Size& rPageSize )
{

    PieLabelInfo* p1 = bSingleCenter ? pCenter->pPrevious : pCenter;
    PieLabelInfo* p2 = pCenter->pNext;
    //return true when successful

    bool bLabelOrderIsAntiClockWise = m_pPosHelper->isMathematicalOrientationAngle();

    ///two loops are performed simultaneously: the outer loop iterates on
    ///`PieLabelInfo` objects in the list starting from the central element
    ///(`pCenter`) and moving forward until the last element (`pSecondBorder`);
    ///the inner loop starts from the previous element of `pCenter` and moves
    ///forward until the current `PieLabelInfo` object of the outer loop is
    ///reached
    PieLabelInfo* pCurrent = nullptr;
    for( pCurrent = p2 ;pCurrent->pPrevious != pSecondBorder; pCurrent = pCurrent->pNext )
    {
        PieLabelInfo* pFix = nullptr;
        for( pFix = p2->pPrevious ;pFix != pCurrent; pFix = pFix->pNext )
        {
            ///on the current `PieLabelInfo` object of the outer loop the
            ///`moveAwayFrom` method is invoked by passing the current
            ///`PieLabelInfo` object of the inner loop as argument.

            ///so each label going from the central one to the last one is
            ///checked for overlapping against all previous labels (that comes
            ///after the central label) and in case the overlap occurs the
            ///`moveAwayFrom` method tries to fix the issue;
            ///if `moveAwayFrom` returns true (pay attention: that does not
            ///mean that the overlap issue has been surely fixed but only that
            ///the moved label is at least completely inside the page document:
            ///see notes on `PieChart::PieLabelInfo::moveAwayFrom`), the inner
            ///loop starts a new iteration else the `rbAlternativeMoveDirection`
            ///boolean parameter is tested: if it is false the parameter is set
            ///to true, the position of all labels is restored to the initial
            ///one (through the `PieChart::resetLabelPositionsToPreviousState`
            ///method) and the method ends by returning false, else the inner
            ///loop starts a new iteration step;
            ///so when `rbAlternativeMoveDirection` is true the method goes on
            ///trying to fix left overlap issues even if the last `moveAwayFrom`
            ///invocation has moved a label in a position that it is not
            ///completely inside the page document

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

    ///if the method does not return before ending the first pair of loops,
    ///a second pair of simultaneous loops is performed in the opposite
    ///direction (respect with the previous case): the outer loop iterates on
    ///`PieLabelInfo` objects in the list starting from the central element
    ///(`pCenter`) and moving backward until the first element (`pFirstBorder`);
    ///the inner loop starts from the next element of `pCenter` and moves
    ///backward until the current `PieLabelInfo` object of the outer loop is
    ///reached

    ///like in the previous case on the current `PieLabelInfo` object of
    ///the outer loop the `moveAwayFrom` method is invoked by passing
    ///the current `PieLabelInfo` object of the inner loop as argument

    ///so each label going from the central one to the first one is checked for
    ///overlapping on all subsequent labels (that come before the central label)
    ///and in case the overlap occurs the `moveAwayFrom` method tries to fix
    ///the issue. The subsequent actions performed after the invocation
    ///`moveAwayFrom` are the same detailed above for the first pair of loops

    for( pCurrent = p1 ;pCurrent->pNext != pFirstBorder; pCurrent = pCurrent->pPrevious )
    {
        PieLabelInfo* pFix = nullptr;
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
    ///this method is invoked by `ChartView::impl_createDiagramAndContent` for
    ///pie and donut charts after text label creation;
    ///it tries to rearrange labels only when the label placement type is
    ///`AVOID_OVERLAP`.


    ///check whether there are any labels that should be moved
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
    if( fPageDiagonaleLength == 0.0 )
        return;

    ///initialize next and previous member of `PieLabelInfo` objects
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

    ///detect overlaps and move
    sal_Int32 nMaxIterations = 50;
    while( detectLabelOverlapsAndMove( rPageSize ) && nMaxIterations > 0 )
        nMaxIterations--;

    ///create connection lines for the moved labels
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
                    aVLineProperties.Color <<= nColor;
            }
            m_pShapeFactory->createLine2D( rInfo.xTextTarget, aPoints, &aVLineProperties );
        }
    }
}


/** Handle the placement of the label in the best fit case:
 *  the routine try to place the label inside the related pie slice,
 *  in case of success it returns true else returns false.
 *
 *  Notation:
 *  C: the pie center
 *  s: the bisector ray of the current pie slice
 *  alpha: the angle between the horizontal axis and the bisector ray s
 *  N: the vertex of the label b.b. which is nearest to C
 *  F: the vertex of the label b.b. not adjacent to N; F lies on the pie border
 *  P, Q: the intersection points between the label b.b. and the bisector ray s;
 *        P is the one at minimum distance respect with C
 *  e: the edge of the label b.b. where P lies (the nearest edge to C)
 *  M: the vertex of e that is not N
 *  G: the vertex of the label b.b. which is adjacent to N and that is not M
 *  beta: the angle MPF
 *  theta: the angle CPF
 *
 *
 *     |
 *     |                                /s
 *     |                               /
 *     |                              /
 *     |  G _________________________/____________________________ F
 *     |   |                        /Q                          ..|
 *     |   |                       /                         . .  |
 *     |   |                      /                       .  .    |
 *     |   |                     /                     .   .      |
 *     |   |                    /                   .    .        |
 *     |   |                   /                 .     .          |
 *     |   |                  /              d.      .            |
 *     |   |                 /             .       .              |
 *     |   |                /           .        .                |
 *     |   |               /         .         .                  |
 *     |   |              /       .          .                    |
 *     |   |             /     .           .                      |
 *     |   |            /   .            .                        |
 *     |   |           / .  \ beta     .                          |
 *     |   |__________/._\___|_______.____________________________|
 *     |  N          /P  /         .                               M
 *     |            /___/theta   .
 *     |           /           .
 *     |          /          . r
 *     |         /         .
 *     |        /        .
 *     |       /       .
 *     |      /      .
 *     |     /     .
 *     |    /    .
 *     |   /   .
 *     |  /  .
 *     | /\. alpha
 *   __|/__|_____________________________________________________________
 *     |C
 *     |
 *
 *
 *  When alpha = 45k (k integer) s crosses the label b.b. at N exactly.
 *  In such a case the nearest edge e is defined as the edge having N as the
 *  start vertex and that is covered in the counterclockwise direction when
 *  we move from N to the adjacent vertex.
 *
 *  The nearest vertex N is:
 *   1. the bottom left vertex when 0 < alpha < 90
 *   2. the bottom right vertex when 90 < alpha < 180
 *   3. the top right vertex when 180 < alpha < 270
 *   4. the top left vertex when 270 < alpha < 360.
 *
 *  The nearest edge e is:
 *   1. the left edge when −45 < alpha < 45
 *   2. the bottom edge when 45 < alpha <135
 *   3. the right edge when 135 < alpha < 225
 *   4. the top edge when 225 < alpha < 315.
 *
 **/
bool PieChart::performLabelBestFitInnerPlacement(ShapeParam& rShapeParam, PieLabelInfo& rPieLabelInfo)
{
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "** PieChart::performLabelBestFitInnerPlacement invoked **" );

    // get pie slice properties
    double fStartAngleDeg = lcl_getDegAngleInStandardRange(rShapeParam.mfUnitCircleStartAngleDegree);
    double fWidthAngleDeg = rShapeParam.mfUnitCircleWidthAngleDegree;
    double fHalfWidthAngleDeg = fWidthAngleDeg / 2.0;
    double fBisectingRayAngleDeg = lcl_getDegAngleInStandardRange(fStartAngleDeg + fHalfWidthAngleDeg);

    // get the middle point of the arc representing the pie slice border
    double fLogicZ = rShapeParam.mfLogicZ + 1.0;
    awt::Point aMiddleArcPoint = PlottingPositionHelper::transformSceneToScreenPosition(
            m_pPosHelper->transformUnitCircleToScene(
                    fBisectingRayAngleDeg,
                    rShapeParam.mfUnitCircleOuterRadius,
                    fLogicZ ),
            m_xLogicTarget, m_pShapeFactory, m_nDimension );

    // compute the pie radius
    basegfx::B2IVector aPieCenter = rPieLabelInfo.aOrigin;
    basegfx::B2IVector aRadiusVector(
            aMiddleArcPoint.X - aPieCenter.getX(),
            aMiddleArcPoint.Y - aPieCenter.getY() );
    double fSquaredPieRadius = aRadiusVector.scalar(aRadiusVector);
    double fPieRadius = sqrt( fSquaredPieRadius );

    // the bb is moved as much as possible near to the border of the pie,
    // anyway a small offset from the border is present (0.025 * pie radius)
    const double fPieBorderOffset = 0.025;
    fPieRadius = fPieRadius - fPieRadius * fPieBorderOffset;

    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "    pie sector:" );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      start angle = " << fStartAngleDeg );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      angle width = " << fWidthAngleDeg );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      bisecting ray angle = " << fBisectingRayAngleDeg );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      pie radius = " << fPieRadius );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      pie center = " << rPieLabelInfo.aOrigin );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      middle arc point = (" << aMiddleArcPoint.X << ","
                                           << aMiddleArcPoint.Y << ")" );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "    label bounding box:" );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      old anchor point = " << rPieLabelInfo.aFirstPosition );


    if( fPieRadius == 0.0 )
        return false;

    // get label b.b. width and height
    ::basegfx::B2IRectangle aBb( lcl_getRect( rPieLabelInfo.xLabelGroupShape ) );
    double fLabelWidth = aBb.getWidth();
    double fLabelHeight = aBb.getHeight();

    // -45 <= fAlphaDeg < 315
    double fAlphaDeg = lcl_getDegAngleInStandardRange(fBisectingRayAngleDeg + 45) - 45;
    double fAlphaRad = lcl_degToRad(fAlphaDeg);

    // compute nearest edge index
    // 0 left
    // 1 bottom
    // 2 right
    // 3 top
    int nSectorIndex = floor( (fAlphaDeg + 45) / 45.0 );
    int nNearestEdgeIndex = nSectorIndex / 2;

    // compute lengths of the nearest edge and of the orthogonal edges
    double fNearestEdgeLength = fLabelWidth;
    double fOrthogonalEdgeLength = fLabelHeight;
    int nAxisIndex = 0;
    int nOrthogonalAxisIndex = 1;
    if( nNearestEdgeIndex % 2 == 0 ) // nearest edge is vertical
    {
        fNearestEdgeLength = fLabelHeight;
        fOrthogonalEdgeLength = fLabelWidth;
        nAxisIndex = 1;
        nOrthogonalAxisIndex = 0;
    }

    // compute the distance between N and P
    // such a distance is piece wise linear respect with alpha:
    // given 45k <= alpha < 45(k+1) we have
    // when k is even: d(N,P) = (length(e) / 2) * (1 - (alpha - 45k)/45)
    // when k is odd: d(N,P) = (length(e) / 2) * (1 - (45(k+1) - alpha)/45)
    int nIndex = nSectorIndex -1;  // nIndex = -1...6
    double fIndexMod2 = (nIndex + 8) % 2; // fIndexMod2 must be non negative
    double fSgn = 2.0 * (fIndexMod2 - 0.5); // 0 -> -1, 1 -> 1
    double fDistanceNP = (fNearestEdgeLength / 2.0) * (1 + fSgn * ((fAlphaDeg - 45 * (nIndex + fIndexMod2)) / 45.0));
    double fDistancePM = fNearestEdgeLength - fDistanceNP;

    // compute the length of the diagonal vector d,
    // that is the distance between P and F
    double fSquaredDistancePF = fDistancePM * fDistancePM + fOrthogonalEdgeLength * fOrthogonalEdgeLength;
    double fDistancePF = sqrt( fSquaredDistancePF );

    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      width = " << fLabelWidth );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      height = " <<  fLabelHeight );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      nearest edge index = " << nNearestEdgeIndex );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      alpha = " << fAlphaDeg );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      distance(N,P) = " << fDistanceNP );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "        nIndex = " << nIndex );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "        fIndexMod2 = " << fIndexMod2 );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "        fSgn = " << fSgn );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      distance(P,F) = " << fDistancePF );


    // we check that the condition length(d) <= pie radius holds
    if (fDistancePF > fPieRadius)
    {
        return false;
    }

    // compute beta: the angle of the diagonal vector d,
    // that is, the angle in P respect with the triangle PMF;
    // since both arguments are non negative the returned value is in [0, PI/2]
    double fBetaRad = atan2( fOrthogonalEdgeLength, fDistancePM );

    // compute the theta angle, that is the angle in P
    // respect with the triangle CFP;
    // when the second intersection edge is opposite to the nearest edge,
    // theta depends on alpha and beta according to the following relation:
    // theta = f(alpha, beta) = s * alpha + 90 * (1 - s * i) + beta
    // where i is the nearest edge index and s is the sign of (alpha' - 45),
    // with alpha' = (alpha + 45) mod 90;
    // when the second intersection edge is adjacent to the nearest edge,
    // we have theta = 360 - f(alpha, beta);
    // note that in the former case 0 <= f(alpha, beta) <= 180,
    // whilst in the latter case 180 <= f(alpha, beta) <= 360;
    double fAlphaMod90 = fmod( fAlphaDeg + 45, 90.0 ) - 45;
    double fSign = fAlphaMod90 == 0.0
                       ? 0.0
                       : ( fAlphaMod90 < 0 ) ? -1.0 : 1.0;
    double fThetaRad = fSign * fAlphaRad + M_PI_2 * (1 - fSign * nNearestEdgeIndex) + fBetaRad;
    if( fThetaRad > M_PI )
    {
        fThetaRad = 2 * M_PI - fThetaRad;
    }

    // compute the length of the positional vector,
    // that is the distance between C and P
    double fDistanceCP;
    // when the bisector ray intersects the b.b. in F we have theta mod 180 == 0
    if( fmod(fThetaRad, M_PI) == 0.0 )
    {
        fDistanceCP = fPieRadius - fDistancePF;
    }
    else // general case
    {
        // we can compute d(C,P) by applying some trigonometric formula to
        // the triangle CFP : we know length(d) and length(r) = r and we have
        // computed the angle in P (theta); so named delta the angle in C and
        // gamma the angle in F, by the relation:
        //
        //                r         d(P,F)     d(C,P)
        //            --------- = --------- = ---------
        //            sin theta   sin delta   sin gamma
        //
        // we get the wanted distance
        double fSinTheta = sin( fThetaRad );
        double fSinDelta = fDistancePF * fSinTheta / fPieRadius;
        double fDeltaRad = asin( fSinDelta );
        double fGammaRad = M_PI - (fThetaRad + fDeltaRad);
        double fSinGamma = sin( fGammaRad );
        fDistanceCP = fPieRadius * fSinGamma / fSinTheta;
    }

    // define the positional vector
    basegfx::B2DVector aPositionalVector( cos(fAlphaRad), sin(fAlphaRad) );
    aPositionalVector.setLength(fDistanceCP);

    // we define a direction vector in order to know
    // in which quadrant we are working
    basegfx::B2DVector aDirection(1.0, 1.0);
    if( 90 <= fBisectingRayAngleDeg && fBisectingRayAngleDeg < 270 )
    {
        aDirection.setX(-1.0);
    }
    if( fBisectingRayAngleDeg >= 180 )
    {
        aDirection.setY(-1.0);
    }

    // compute vertices N, M and G respect with pie center C
    basegfx::B2DVector aNearestVertex(aPositionalVector);
    aNearestVertex[nAxisIndex] += -aDirection[nAxisIndex] * fDistanceNP;
    basegfx::B2DVector aVertexM(aNearestVertex);
    aVertexM[nAxisIndex] += aDirection[nAxisIndex] * fNearestEdgeLength;
    basegfx::B2DVector aVertexG(aNearestVertex);
    aVertexG[nOrthogonalAxisIndex] += aDirection[nOrthogonalAxisIndex] * fOrthogonalEdgeLength;

    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      beta = " << lcl_radToDeg(fBetaRad) );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      theta = " << lcl_radToDeg(fThetaRad) );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "        fAlphaMod90 = " << fAlphaMod90 );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "        fSign = " << fSign );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      distance(C,P) = " << fDistanceCP );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      direction vector = " << aDirection );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      N = " << aNearestVertex );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      M = " << aVertexM );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      G = " << aVertexG );

    // in order to be able to place the label inside the pie slice we need
    // to check that each angle between s and the ray starting from C and
    // passing through a b.b. vertex is less than half width of the pie slice;
    // when the nearest edge e crosses a Cartesian axis it is sufficient
    // to test only the vertices belonging to e, else we need to test
    // the 2 vertices that aren’t either N or F . Note that if a b.b. edge
    // crosses a Cartesian axis then it is the nearest edge to C

    // check the angle between CP and CM
    double fAngleRad = aPositionalVector.angle(aVertexM);
    double fAngleDeg = lcl_getDegAngleInStandardRange( lcl_radToDeg(fAngleRad) );
    if( fAngleDeg > 180 )  // in case the wrong angle has been computed
        fAngleDeg = 360 - fAngleDeg;
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      angle between CP and CM: " << fAngleDeg );
    if( fAngleDeg > fHalfWidthAngleDeg )
    {
        return false;
    }

    if( ( aNearestVertex[nAxisIndex] >= 0 && aVertexM[nAxisIndex] <= 0 )
            || ( aNearestVertex[nAxisIndex] <= 0 && aVertexM[nAxisIndex] >= 0 ) )
    {
        // check the angle between CP and CN
        fAngleRad = aPositionalVector.angle(aNearestVertex);
        fAngleDeg = lcl_getDegAngleInStandardRange( lcl_radToDeg(fAngleRad) );
        if( fAngleDeg > 180 )  // in case the wrong angle has been computed
            fAngleDeg = 360 - fAngleDeg;
        SAL_INFO( "chart2.pie.label.bestfit.inside",
                  "      angle between CP and CN: " << fAngleDeg );
        if( fAngleDeg > fHalfWidthAngleDeg )
        {
            return false;
        }
    }
    else
    {
        // check the angle between CP and CG
        fAngleRad = aPositionalVector.angle(aVertexG);
        fAngleDeg = lcl_getDegAngleInStandardRange( lcl_radToDeg(fAngleRad) );
        if( fAngleDeg > 180 )  // in case the wrong angle has been computed
            fAngleDeg = 360 - fAngleDeg;
        SAL_INFO( "chart2.pie.label.bestfit.inside",
                  "      angle between CP and CG: " << fAngleDeg );
        if( fAngleDeg > fHalfWidthAngleDeg )
        {
            return false;
        }
    }

    // compute the b.b. center respect with the pie center
    basegfx::B2DVector aBBCenter(aNearestVertex);
    aBBCenter[nAxisIndex] += aDirection[nAxisIndex] * fNearestEdgeLength / 2;
    aBBCenter[nOrthogonalAxisIndex] += aDirection[nOrthogonalAxisIndex] * fOrthogonalEdgeLength / 2;

    // compute the b.b. anchor point
    basegfx::B2IVector aNewAnchorPoint = aPieCenter;
    aNewAnchorPoint[0] += floor(aBBCenter[0]);
    aNewAnchorPoint[1] -= floor(aBBCenter[1]); // the Y axis on the screen points downward

    // compute the translation vector for moving the label from the current
    // screen position to the new one
    basegfx::B2IVector aTranslationVector = aNewAnchorPoint - rPieLabelInfo.aFirstPosition;

    // compute the new screen position and move the label
    // XShape::getPosition returns the top left vertex of the b.b. of the shape
    awt::Point aOldPos( rPieLabelInfo.xLabelGroupShape->getPosition() );
    awt::Point aNewPos( aOldPos.X + aTranslationVector.getX(),
                        aOldPos.Y + aTranslationVector.getY() );
    rPieLabelInfo.xLabelGroupShape->setPosition(aNewPos);

    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      center = " <<  aBBCenter );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      new anchor point = " << aNewAnchorPoint );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      translation vector = " <<  aTranslationVector );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      old position = (" << aOldPos.X << "," << aOldPos.Y << ")" );
    SAL_INFO( "chart2.pie.label.bestfit.inside",
              "      new position = (" << aNewPos.X << "," << aNewPos.Y << ")" );

    return true;
}

/** Handle the outer placement of the labels in the best fit case.
 *
 */
bool PieChart::performLabelBestFitOuterPlacement(ShapeParam& /*rShapeParam*/, PieLabelInfo& /*rPieLabelInfo*/)
{
    SAL_WARN( "chart2.pie.label.bestfit", "to be implemented" );
    return false;
}

/** Handle the placement of the label in the best fit case.
 *  First off the routine try to place the label inside the related pie slice,
 *  if this is not possible the label is placed outside.
 */
void PieChart::performLabelBestFit(ShapeParam& rShapeParam, PieLabelInfo& rPieLabelInfo)
{
    if( m_bUseRings )
        return;

    if( !performLabelBestFitInnerPlacement(rShapeParam, rPieLabelInfo) )
    {
        performLabelBestFitOuterPlacement(rShapeParam, rPieLabelInfo);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
