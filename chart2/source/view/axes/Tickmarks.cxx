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

#include "Tickmarks.hxx"
#include "Tickmarks_Equidistant.hxx"
#include "Tickmarks_Dates.hxx"
#include <ViewDefines.hxx>
#include "VAxisProperties.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;
using ::basegfx::B2DVector;

namespace chart {

TickInfo::TickInfo( const uno::Reference<chart2::XScaling>& xInverse )
: fScaledTickValue( 0.0 )
, xInverseScaling( xInverse )
, aTickScreenPosition(0.0,0.0)
, bPaintIt( true )
, nFactorForLimitedTextWidth(1)
{
}

double TickInfo::getUnscaledTickValue() const
{
    if( xInverseScaling.is() )
        return xInverseScaling->doScaling( fScaledTickValue );
    else
        return fScaledTickValue;
}

sal_Int32 TickInfo::getScreenDistanceBetweenTicks( const TickInfo& rOherTickInfo ) const
{
    //return the positive distance between the two first tickmarks in screen values

    B2DVector aDistance = rOherTickInfo.aTickScreenPosition - aTickScreenPosition;
    sal_Int32 nRet = static_cast<sal_Int32>(aDistance.getLength());
    if(nRet<0)
        nRet *= -1;
    return nRet;
}

PureTickIter::PureTickIter( TickInfoArrayType& rTickInfoVector )
            : m_rTickVector(rTickInfoVector)
            , m_aTickIter(m_rTickVector.begin())
{
}
PureTickIter::~PureTickIter()
{
}
TickInfo* PureTickIter::firstInfo()
{
    m_aTickIter = m_rTickVector.begin();
    if(m_aTickIter!=m_rTickVector.end())
        return &*m_aTickIter;
    return nullptr;
}
TickInfo* PureTickIter::nextInfo()
{
    if(m_aTickIter!=m_rTickVector.end())
    {
        ++m_aTickIter;
        if(m_aTickIter!=m_rTickVector.end())
            return &*m_aTickIter;
    }
    return nullptr;
}

TickFactory::TickFactory(
          const ExplicitScaleData& rScale, const ExplicitIncrementData& rIncrement )
            : m_rScale( rScale )
            , m_rIncrement( rIncrement )
{
    //@todo: make sure that the scale is valid for the scaling

    if( m_rScale.Scaling.is() )
    {
        m_xInverseScaling = m_rScale.Scaling->getInverseScaling();
        OSL_ENSURE( m_xInverseScaling.is(), "each Scaling needs to return an inverse Scaling" );
    }

    m_fScaledVisibleMin = m_rScale.Minimum;
    if( m_xInverseScaling.is() )
        m_fScaledVisibleMin = m_rScale.Scaling->doScaling(m_fScaledVisibleMin);

    m_fScaledVisibleMax = m_rScale.Maximum;
    if( m_xInverseScaling.is() )
        m_fScaledVisibleMax = m_rScale.Scaling->doScaling(m_fScaledVisibleMax);
}

TickFactory::~TickFactory()
{
}

bool TickFactory::isDateAxis() const
{
    return m_rScale.AxisType == chart2::AxisType::DATE;
}

void TickFactory::getAllTicks( TickInfoArraysType& rAllTickInfos ) const
{
    if( isDateAxis() )
        DateTickFactory( m_rScale, m_rIncrement ).getAllTicks( rAllTickInfos );
    else
        EquidistantTickFactory( m_rScale, m_rIncrement ).getAllTicks( rAllTickInfos );
}

void TickFactory::getAllTicksShifted( TickInfoArraysType& rAllTickInfos ) const
{
    if( isDateAxis() )
        DateTickFactory( m_rScale, m_rIncrement ).getAllTicksShifted( rAllTickInfos );
    else
        EquidistantTickFactory( m_rScale, m_rIncrement ).getAllTicksShifted( rAllTickInfos );
}

// ___TickFactory_2D___
TickFactory2D::TickFactory2D(
          const ExplicitScaleData& rScale, const ExplicitIncrementData& rIncrement
          //, double fStretch_SceneToScreen, double fOffset_SceneToScreen )
          , const B2DVector& rStartScreenPos, const B2DVector& rEndScreenPos
          , const B2DVector& rAxisLineToLabelLineShift )
          : TickFactory( rScale, rIncrement )
          , m_aAxisStartScreenPosition2D(rStartScreenPos)
          , m_aAxisEndScreenPosition2D(rEndScreenPos)
          , m_aAxisLineToLabelLineShift(rAxisLineToLabelLineShift)
          , m_fStretch_LogicToScreen(1.0)
          , m_fOffset_LogicToScreen(0.0)
{
    double fWidthY = m_fScaledVisibleMax - m_fScaledVisibleMin;
    if (m_rScale.Orientation == chart2::AxisOrientation_MATHEMATICAL)
    {
        m_fStretch_LogicToScreen = 1.0/fWidthY;
        m_fOffset_LogicToScreen = -m_fScaledVisibleMin;
    }
    else
    {
        B2DVector aSwap(m_aAxisStartScreenPosition2D);
        m_aAxisStartScreenPosition2D = m_aAxisEndScreenPosition2D;
        m_aAxisEndScreenPosition2D = aSwap;

        m_fStretch_LogicToScreen = -1.0/fWidthY;
        m_fOffset_LogicToScreen = -m_fScaledVisibleMax;
    }
}

TickFactory2D::~TickFactory2D()
{
}

bool TickFactory2D::isHorizontalAxis() const
{
    // check trivial cases:
    if ( m_aAxisStartScreenPosition2D.getY() == m_aAxisEndScreenPosition2D.getY() )
        return true;
    if ( m_aAxisStartScreenPosition2D.getX() == m_aAxisEndScreenPosition2D.getX() )
        return false;

    // for skew axes compare angle with horizontal vector
    double fInclination = std::abs(B2DVector(m_aAxisEndScreenPosition2D-m_aAxisStartScreenPosition2D).angle(B2DVector(1.0, 0.0)));
    return fInclination < F_PI4 || fInclination > (F_PI-F_PI4);
}
bool TickFactory2D::isVerticalAxis() const
{
    // check trivial cases:
    if ( m_aAxisStartScreenPosition2D.getX() == m_aAxisEndScreenPosition2D.getX() )
        return true;
    if ( m_aAxisStartScreenPosition2D.getY() == m_aAxisEndScreenPosition2D.getY() )
        return false;

    // for skew axes compare angle with vertical vector
    double fInclination = std::abs(B2DVector(m_aAxisEndScreenPosition2D-m_aAxisStartScreenPosition2D).angle(B2DVector(0.0, -1.0)));
    return fInclination < F_PI4 || fInclination > (F_PI-F_PI4);
}
//static
sal_Int32 TickFactory2D::getTickScreenDistance( TickIter& rIter )
{
    //return the positive distance between the two first tickmarks in screen values
    //if there are less than two tickmarks -1 is returned

    const TickInfo* pFirstTickInfo = rIter.firstInfo();
    const TickInfo* pSecondTickInfo = rIter.nextInfo();
    if(!pSecondTickInfo  || !pFirstTickInfo)
        return -1;

    return pFirstTickInfo->getScreenDistanceBetweenTicks( *pSecondTickInfo );
}

const B2DVector & TickFactory2D::getXaxisStartPos() const
{
    return m_aAxisStartScreenPosition2D;
}

B2DVector TickFactory2D::getTickScreenPosition2D( double fScaledLogicTickValue ) const
{
    B2DVector aRet(m_aAxisStartScreenPosition2D);
    aRet += (m_aAxisEndScreenPosition2D-m_aAxisStartScreenPosition2D)
                *((fScaledLogicTickValue+m_fOffset_LogicToScreen)*m_fStretch_LogicToScreen);
    return aRet;
}

void TickFactory2D::addPointSequenceForTickLine( drawing::PointSequenceSequence& rPoints
                                , sal_Int32 nSequenceIndex
                                , double fScaledLogicTickValue, double fInnerDirectionSign
                                , const TickmarkProperties& rTickmarkProperties
                                , bool bPlaceAtLabels ) const
{
    if( fInnerDirectionSign==0.0 )
        fInnerDirectionSign = 1.0;

    B2DVector aTickScreenPosition = getTickScreenPosition2D(fScaledLogicTickValue);
    if( bPlaceAtLabels )
        aTickScreenPosition += m_aAxisLineToLabelLineShift;

    B2DVector aMainDirection = m_aAxisEndScreenPosition2D-m_aAxisStartScreenPosition2D;
    aMainDirection.normalize();
    B2DVector aOrthoDirection(-aMainDirection.getY(),aMainDirection.getX());
    aOrthoDirection *= fInnerDirectionSign;
    aOrthoDirection.normalize();

    B2DVector aStart = aTickScreenPosition + aOrthoDirection*rTickmarkProperties.RelativePos;
    B2DVector aEnd = aStart - aOrthoDirection*rTickmarkProperties.Length;

    rPoints[nSequenceIndex].realloc(2);
    rPoints[nSequenceIndex][0].X = static_cast<sal_Int32>(aStart.getX());
    rPoints[nSequenceIndex][0].Y = static_cast<sal_Int32>(aStart.getY());
    rPoints[nSequenceIndex][1].X = static_cast<sal_Int32>(aEnd.getX());
    rPoints[nSequenceIndex][1].Y = static_cast<sal_Int32>(aEnd.getY());
}

B2DVector TickFactory2D::getDistanceAxisTickToText( const AxisProperties& rAxisProperties, bool bIncludeFarAwayDistanceIfSo, bool bIncludeSpaceBetweenTickAndText ) const
{
    bool bFarAwayLabels = false;
    if( rAxisProperties.m_eLabelPos == css::chart::ChartAxisLabelPosition_OUTSIDE_START
        || rAxisProperties.m_eLabelPos == css::chart::ChartAxisLabelPosition_OUTSIDE_END )
        bFarAwayLabels = true;

    double fInnerDirectionSign = rAxisProperties.maLabelAlignment.mfInnerTickDirection;
    if( fInnerDirectionSign==0.0 )
        fInnerDirectionSign = 1.0;

    B2DVector aMainDirection = m_aAxisEndScreenPosition2D-m_aAxisStartScreenPosition2D;
    aMainDirection.normalize();
    B2DVector aOrthoDirection(-aMainDirection.getY(),aMainDirection.getX());
    aOrthoDirection *= fInnerDirectionSign;
    aOrthoDirection.normalize();

    B2DVector aStart(0,0), aEnd(0,0);
    if( bFarAwayLabels )
    {
        TickmarkProperties aProps( AxisProperties::getBiggestTickmarkProperties() );
        aStart = aOrthoDirection*aProps.RelativePos;
        aEnd = aStart - aOrthoDirection*aProps.Length;
    }
    else
    {
        for( sal_Int32 nN=rAxisProperties.m_aTickmarkPropertiesList.size();nN--;)
        {
            const TickmarkProperties& rProps = rAxisProperties.m_aTickmarkPropertiesList[nN];
            B2DVector aNewStart = aOrthoDirection*rProps.RelativePos;
            B2DVector aNewEnd = aNewStart - aOrthoDirection*rProps.Length;
            if(aNewStart.getLength()>aStart.getLength())
                aStart=aNewStart;
            if(aNewEnd.getLength()>aEnd.getLength())
                aEnd=aNewEnd;
        }
    }

    B2DVector aLabelDirection(aStart);
    if (rAxisProperties.maLabelAlignment.mfInnerTickDirection != rAxisProperties.maLabelAlignment.mfLabelDirection)
        aLabelDirection = aEnd;

    B2DVector aOrthoLabelDirection(aOrthoDirection);
    if (rAxisProperties.maLabelAlignment.mfInnerTickDirection != rAxisProperties.maLabelAlignment.mfLabelDirection)
        aOrthoLabelDirection*=-1.0;
    aOrthoLabelDirection.normalize();
    if( bIncludeSpaceBetweenTickAndText )
        aLabelDirection += aOrthoLabelDirection*AXIS2D_TICKLABELSPACING;
    if( bFarAwayLabels && bIncludeFarAwayDistanceIfSo )
        aLabelDirection += m_aAxisLineToLabelLineShift;
    return aLabelDirection;
}

void TickFactory2D::createPointSequenceForAxisMainLine( drawing::PointSequenceSequence& rPoints ) const
{
    rPoints[0].realloc(2);
    rPoints[0][0].X = static_cast<sal_Int32>(m_aAxisStartScreenPosition2D.getX());
    rPoints[0][0].Y = static_cast<sal_Int32>(m_aAxisStartScreenPosition2D.getY());
    rPoints[0][1].X = static_cast<sal_Int32>(m_aAxisEndScreenPosition2D.getX());
    rPoints[0][1].Y = static_cast<sal_Int32>(m_aAxisEndScreenPosition2D.getY());
}

void TickFactory2D::updateScreenValues( TickInfoArraysType& rAllTickInfos ) const
{
    //get the transformed screen values for all tickmarks in rAllTickInfos
    for (auto & tickInfos : rAllTickInfos)
    {
        for (auto & tickInfo : tickInfos)
        {
            tickInfo.aTickScreenPosition =
                getTickScreenPosition2D(tickInfo.fScaledTickValue);
        }
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
