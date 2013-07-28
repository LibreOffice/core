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

#include "Tickmarks_Equidistant.hxx"
#include "ViewDefines.hxx"
#include <rtl/math.hxx>

#include <limits>
#include <memory>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::rtl::math;
using ::basegfx::B2DVector;

//static
double EquidistantTickFactory::getMinimumAtIncrement( double fMin, const ExplicitIncrementData& rIncrement )
{
    //the returned value will be <= fMin and on a Major Tick given by rIncrement
    if(rIncrement.Distance<=0.0)
        return fMin;

    double fRet = rIncrement.BaseValue +
        floor( approxSub( fMin, rIncrement.BaseValue )
                    / rIncrement.Distance)
            *rIncrement.Distance;

    if( fRet > fMin )
    {
        if( !approxEqual(fRet, fMin) )
            fRet -= rIncrement.Distance;
    }
    return fRet;
}
//static
double EquidistantTickFactory::getMaximumAtIncrement( double fMax, const ExplicitIncrementData& rIncrement )
{
    //the returned value will be >= fMax and on a Major Tick given by rIncrement
    if(rIncrement.Distance<=0.0)
        return fMax;

    double fRet = rIncrement.BaseValue +
        floor( approxSub( fMax, rIncrement.BaseValue )
                    / rIncrement.Distance)
            *rIncrement.Distance;

    if( fRet < fMax )
    {
        if( !approxEqual(fRet, fMax) )
            fRet += rIncrement.Distance;
    }
    return fRet;
}

EquidistantTickFactory::EquidistantTickFactory(
          const ExplicitScaleData& rScale, const ExplicitIncrementData& rIncrement )
            : m_rScale( rScale )
            , m_rIncrement( rIncrement )
            , m_xInverseScaling(NULL)
            , m_pfCurrentValues(NULL)
{
    //@todo: make sure that the scale is valid for the scaling

    m_pfCurrentValues = new double[getTickDepth()];

    if( m_rScale.Scaling.is() )
    {
        m_xInverseScaling = m_rScale.Scaling->getInverseScaling();
        OSL_ENSURE( m_xInverseScaling.is(), "each Scaling needs to return a inverse Scaling" );
    }

    double fMin = m_fScaledVisibleMin = m_rScale.Minimum;
    if( m_xInverseScaling.is() )
    {
        m_fScaledVisibleMin = m_rScale.Scaling->doScaling(m_fScaledVisibleMin);
        if(m_rIncrement.PostEquidistant )
            fMin = m_fScaledVisibleMin;
    }

    double fMax = m_fScaledVisibleMax = m_rScale.Maximum;
    if( m_xInverseScaling.is() )
    {
        m_fScaledVisibleMax = m_rScale.Scaling->doScaling(m_fScaledVisibleMax);
        if(m_rIncrement.PostEquidistant )
            fMax = m_fScaledVisibleMax;
    }

    //--
    m_fOuterMajorTickBorderMin = EquidistantTickFactory::getMinimumAtIncrement( fMin, m_rIncrement );
    m_fOuterMajorTickBorderMax = EquidistantTickFactory::getMaximumAtIncrement( fMax, m_rIncrement );
    //--

    m_fOuterMajorTickBorderMin_Scaled = m_fOuterMajorTickBorderMin;
    m_fOuterMajorTickBorderMax_Scaled = m_fOuterMajorTickBorderMax;
    if(!m_rIncrement.PostEquidistant && m_xInverseScaling.is() )
    {
        m_fOuterMajorTickBorderMin_Scaled = m_rScale.Scaling->doScaling(m_fOuterMajorTickBorderMin);
        m_fOuterMajorTickBorderMax_Scaled = m_rScale.Scaling->doScaling(m_fOuterMajorTickBorderMax);

        //check validity of new range: m_fOuterMajorTickBorderMin <-> m_fOuterMajorTickBorderMax
        //it is assumed here, that the original range in the given Scale is valid
        if( !rtl::math::isFinite(m_fOuterMajorTickBorderMin_Scaled) )
        {
            m_fOuterMajorTickBorderMin += m_rIncrement.Distance;
            m_fOuterMajorTickBorderMin_Scaled = m_rScale.Scaling->doScaling(m_fOuterMajorTickBorderMin);
        }
        if( !rtl::math::isFinite(m_fOuterMajorTickBorderMax_Scaled) )
        {
            m_fOuterMajorTickBorderMax -= m_rIncrement.Distance;
            m_fOuterMajorTickBorderMax_Scaled = m_rScale.Scaling->doScaling(m_fOuterMajorTickBorderMax);
        }
    }
}

EquidistantTickFactory::~EquidistantTickFactory()
{
    delete[] m_pfCurrentValues;
}

sal_Int32 EquidistantTickFactory::getTickDepth() const
{
    return static_cast<sal_Int32>(m_rIncrement.SubIncrements.size()) + 1;
}

void EquidistantTickFactory::addSubTicks( sal_Int32 nDepth, uno::Sequence< uno::Sequence< double > >& rParentTicks ) const
{
    EquidistantTickIter aIter( rParentTicks, m_rIncrement, 0, nDepth-1 );
    double* pfNextParentTick = aIter.firstValue();
    if(!pfNextParentTick)
        return;
    double fLastParentTick = *pfNextParentTick;
    pfNextParentTick = aIter.nextValue();
    if(!pfNextParentTick)
        return;

    sal_Int32 nMaxSubTickCount = this->getMaxTickCount( nDepth );
    if(!nMaxSubTickCount)
        return;

    uno::Sequence< double > aSubTicks(nMaxSubTickCount);
    sal_Int32 nRealSubTickCount = 0;
    sal_Int32 nIntervalCount = m_rIncrement.SubIncrements[nDepth-1].IntervalCount;

    double* pValue = NULL;
    for(; pfNextParentTick; fLastParentTick=*pfNextParentTick, pfNextParentTick = aIter.nextValue())
    {
        for( sal_Int32 nPartTick = 1; nPartTick<nIntervalCount; nPartTick++ )
        {
            pValue = this->getMinorTick( nPartTick, nDepth
                        , fLastParentTick, *pfNextParentTick );
            if(!pValue)
                continue;

            aSubTicks[nRealSubTickCount] = *pValue;
            nRealSubTickCount++;
        }
    }

    aSubTicks.realloc(nRealSubTickCount);
    rParentTicks[nDepth] = aSubTicks;
    if(static_cast<sal_Int32>(m_rIncrement.SubIncrements.size())>nDepth)
        addSubTicks( nDepth+1, rParentTicks );
}

sal_Int32 EquidistantTickFactory::getMaxTickCount( sal_Int32 nDepth ) const
{
    //return the maximum amount of ticks
    //possibly open intervals at the two ends of the region are handled as if they were completely visible
    //(this is necessary for calculating the sub ticks at the borders correctly)

    if( nDepth >= getTickDepth() )
        return 0;
    if( m_fOuterMajorTickBorderMax < m_fOuterMajorTickBorderMin )
        return 0;
    if( m_rIncrement.Distance<=0.0)
        return 0;

    double fSub;
    if(m_rIncrement.PostEquidistant  )
        fSub = approxSub( m_fScaledVisibleMax, m_fScaledVisibleMin );
    else
        fSub = approxSub( m_rScale.Maximum, m_rScale.Minimum );

    if (!isFinite(fSub))
        return 0;

    double fIntervalCount = fSub / m_rIncrement.Distance;
    if (fIntervalCount > std::numeric_limits<sal_Int32>::max())
        // Interval count too high!  Bail out.
        return 0;

    sal_Int32 nIntervalCount = static_cast<sal_Int32>(fIntervalCount);

    nIntervalCount+=3;
    for(sal_Int32 nN=0; nN<nDepth-1; nN++)
    {
        if( m_rIncrement.SubIncrements[nN].IntervalCount>1 )
            nIntervalCount *= m_rIncrement.SubIncrements[nN].IntervalCount;
    }

    sal_Int32 nTickCount = nIntervalCount;
    if(nDepth>0 && m_rIncrement.SubIncrements[nDepth-1].IntervalCount>1)
        nTickCount = nIntervalCount * (m_rIncrement.SubIncrements[nDepth-1].IntervalCount-1);

    return nTickCount;
}

double* EquidistantTickFactory::getMajorTick( sal_Int32 nTick ) const
{
    m_pfCurrentValues[0] = m_fOuterMajorTickBorderMin + nTick*m_rIncrement.Distance;

    if(m_pfCurrentValues[0]>m_fOuterMajorTickBorderMax)
    {
        if( !approxEqual(m_pfCurrentValues[0],m_fOuterMajorTickBorderMax) )
            return NULL;
    }
    if(m_pfCurrentValues[0]<m_fOuterMajorTickBorderMin)
    {
        if( !approxEqual(m_pfCurrentValues[0],m_fOuterMajorTickBorderMin) )
            return NULL;
    }

    //return always the value after scaling
    if(!m_rIncrement.PostEquidistant && m_xInverseScaling.is() )
        m_pfCurrentValues[0] = m_rScale.Scaling->doScaling( m_pfCurrentValues[0] );

    return &m_pfCurrentValues[0];
}

double* EquidistantTickFactory::getMinorTick( sal_Int32 nTick, sal_Int32 nDepth
                            , double fStartParentTick, double fNextParentTick ) const
{
    //check validity of arguments
    {
        //OSL_ENSURE( fStartParentTick < fNextParentTick, "fStartParentTick >= fNextParentTick");
        if(fStartParentTick >= fNextParentTick)
            return NULL;
        if(nDepth>static_cast<sal_Int32>(m_rIncrement.SubIncrements.size()) || nDepth<=0)
            return NULL;

        //subticks are only calculated if they are laying between parent ticks:
        if(nTick<=0)
            return NULL;
        if(nTick>=m_rIncrement.SubIncrements[nDepth-1].IntervalCount)
            return NULL;
    }

    bool    bPostEquidistant = m_rIncrement.SubIncrements[nDepth-1].PostEquidistant;

    double fAdaptedStartParent = fStartParentTick;
    double fAdaptedNextParent  = fNextParentTick;

    if( !bPostEquidistant && m_xInverseScaling.is() )
    {
        fAdaptedStartParent = m_xInverseScaling->doScaling(fStartParentTick);
        fAdaptedNextParent  = m_xInverseScaling->doScaling(fNextParentTick);
    }

    double fDistance = (fAdaptedNextParent - fAdaptedStartParent)/m_rIncrement.SubIncrements[nDepth-1].IntervalCount;

    m_pfCurrentValues[nDepth] = fAdaptedStartParent + nTick*fDistance;

    //return always the value after scaling
    if(!bPostEquidistant && m_xInverseScaling.is() )
        m_pfCurrentValues[nDepth] = m_rScale.Scaling->doScaling( m_pfCurrentValues[nDepth] );

    if( !isWithinOuterBorder( m_pfCurrentValues[nDepth] ) )
        return NULL;

    return &m_pfCurrentValues[nDepth];
}

bool EquidistantTickFactory::isWithinOuterBorder( double fScaledValue ) const
{
    if(fScaledValue>m_fOuterMajorTickBorderMax_Scaled)
        return false;
    if(fScaledValue<m_fOuterMajorTickBorderMin_Scaled)
        return false;

    return true;
}

bool EquidistantTickFactory::isVisible( double fScaledValue ) const
{
    if(fScaledValue>m_fScaledVisibleMax)
    {
        if( !approxEqual(fScaledValue,m_fScaledVisibleMax) )
            return false;
    }
    if(fScaledValue<m_fScaledVisibleMin)
    {
        if( !approxEqual(fScaledValue,m_fScaledVisibleMin) )
            return false;
    }
    return true;
}

void EquidistantTickFactory::getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    uno::Sequence< uno::Sequence< double > > aAllTicks;

    //create point sequences for each tick depth
    sal_Int32 nDepthCount = this->getTickDepth();
    sal_Int32 nMaxMajorTickCount = this->getMaxTickCount( 0 );

    if (nDepthCount <= 0 || nMaxMajorTickCount <= 0)
        return;

    aAllTicks.realloc(nDepthCount);
    aAllTicks[0].realloc(nMaxMajorTickCount);

    sal_Int32 nRealMajorTickCount = 0;
    double* pValue = NULL;
    for( sal_Int32 nMajorTick=0; nMajorTick<nMaxMajorTickCount; nMajorTick++ )
    {
        pValue = this->getMajorTick( nMajorTick );
        if(!pValue)
            continue;
        aAllTicks[0][nRealMajorTickCount] = *pValue;
        nRealMajorTickCount++;
    }
    if(!nRealMajorTickCount)
        return;
    aAllTicks[0].realloc(nRealMajorTickCount);

    if(nDepthCount>0)
        this->addSubTicks( 1, aAllTicks );

    //so far we have added all ticks between the outer major tick marks
    //this was necessary to create sub ticks correctly
    //now we reduce all ticks to the visible ones that lie between the real borders
    sal_Int32 nDepth = 0;
    sal_Int32 nTick = 0;
    for( nDepth = 0; nDepth < nDepthCount; nDepth++)
    {
        sal_Int32 nInvisibleAtLowerBorder = 0;
        sal_Int32 nInvisibleAtUpperBorder = 0;
        //we need only to check all ticks within the first major interval at each border
        sal_Int32 nCheckCount = 1;
        for(sal_Int32 nN=0; nN<nDepth; nN++)
        {
            if( m_rIncrement.SubIncrements[nN].IntervalCount>1 )
                nCheckCount *= m_rIncrement.SubIncrements[nN].IntervalCount;
        }
        uno::Sequence< double >& rTicks = aAllTicks[nDepth];
        sal_Int32 nCount = rTicks.getLength();
        //check lower border
        for( nTick=0; nTick<nCheckCount && nTick<nCount; nTick++)
        {
            if( !isVisible( rTicks[nTick] ) )
                nInvisibleAtLowerBorder++;
        }
        //check upper border
        for( nTick=nCount-1; nTick>nCount-1-nCheckCount && nTick>=0; nTick--)
        {
            if( !isVisible( rTicks[nTick] ) )
                nInvisibleAtUpperBorder++;
        }
        //resize sequence
        if( !nInvisibleAtLowerBorder && !nInvisibleAtUpperBorder)
            continue;
        if( !nInvisibleAtLowerBorder )
            rTicks.realloc(nCount-nInvisibleAtUpperBorder);
        else
        {
            sal_Int32 nNewCount = nCount-nInvisibleAtUpperBorder-nInvisibleAtLowerBorder;
            if(nNewCount<0)
                nNewCount=0;

            uno::Sequence< double > aOldTicks(rTicks);
            rTicks.realloc(nNewCount);
            for(nTick = 0; nTick<nNewCount; nTick++)
                rTicks[nTick] = aOldTicks[nInvisibleAtLowerBorder+nTick];
        }
    }

    //fill return value
    rAllTickInfos.resize(aAllTicks.getLength());
    for( nDepth=0 ;nDepth<aAllTicks.getLength(); nDepth++ )
    {
        sal_Int32 nCount = aAllTicks[nDepth].getLength();

        ::std::vector< TickInfo >& rTickInfoVector = rAllTickInfos[nDepth];
        rTickInfoVector.clear();
        rTickInfoVector.reserve( nCount );
        for(sal_Int32 nN = 0; nN<nCount; nN++)
        {
            TickInfo aTickInfo(m_xInverseScaling);
            aTickInfo.fScaledTickValue = aAllTicks[nDepth][nN];
            rTickInfoVector.push_back(aTickInfo);
        }
    }
}

void EquidistantTickFactory::getAllTicksShifted( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    ExplicitIncrementData aShiftedIncrement( m_rIncrement );
    aShiftedIncrement.BaseValue = m_rIncrement.BaseValue-m_rIncrement.Distance/2.0;
    EquidistantTickFactory( m_rScale, aShiftedIncrement ).getAllTicks(rAllTickInfos);
}

EquidistantTickIter::EquidistantTickIter( const uno::Sequence< uno::Sequence< double > >& rTicks
                   , const ExplicitIncrementData& rIncrement
                   , sal_Int32 nMinDepth, sal_Int32 nMaxDepth )
                : m_pSimpleTicks(&rTicks)
                , m_pInfoTicks(0)
                , m_rIncrement(rIncrement)
                , m_nMaxDepth(0)
                , m_nTickCount(0), m_pnPositions(NULL)
                , m_pnPreParentCount(NULL), m_pbIntervalFinished(NULL)
                , m_nCurrentDepth(-1), m_nCurrentPos(-1), m_fCurrentValue( 0.0 )
{
    initIter( nMinDepth, nMaxDepth );
}

EquidistantTickIter::EquidistantTickIter( ::std::vector< ::std::vector< TickInfo > >& rTicks
                   , const ExplicitIncrementData& rIncrement
                   , sal_Int32 nMinDepth, sal_Int32 nMaxDepth )
                : m_pSimpleTicks(NULL)
                , m_pInfoTicks(&rTicks)
                , m_rIncrement(rIncrement)
                , m_nMaxDepth(0)
                , m_nTickCount(0), m_pnPositions(NULL)
                , m_pnPreParentCount(NULL), m_pbIntervalFinished(NULL)
                , m_nCurrentDepth(-1), m_nCurrentPos(-1), m_fCurrentValue( 0.0 )
{
    initIter( nMinDepth, nMaxDepth );
}

void EquidistantTickIter::initIter( sal_Int32 /*nMinDepth*/, sal_Int32 nMaxDepth )
{
    m_nMaxDepth = nMaxDepth;
    if(nMaxDepth<0 || m_nMaxDepth>getMaxDepth())
        m_nMaxDepth=getMaxDepth();

    sal_Int32 nDepth = 0;
    for( nDepth = 0; nDepth<=m_nMaxDepth ;nDepth++ )
        m_nTickCount += getTickCount(nDepth);

    if(!m_nTickCount)
        return;

    m_pnPositions      = new sal_Int32[m_nMaxDepth+1];

    m_pnPreParentCount = new sal_Int32[m_nMaxDepth+1];
    m_pbIntervalFinished = new bool[m_nMaxDepth+1];
    m_pnPreParentCount[0] = 0;
    m_pbIntervalFinished[0] = false;
    double fParentValue = getTickValue(0,0);
    for( nDepth = 1; nDepth<=m_nMaxDepth ;nDepth++ )
    {
        m_pbIntervalFinished[nDepth] = false;

        sal_Int32 nPreParentCount = 0;
        sal_Int32 nCount = getTickCount(nDepth);
        for(sal_Int32 nN = 0; nN<nCount; nN++)
        {
            if(getTickValue(nDepth,nN) < fParentValue)
                nPreParentCount++;
            else
                break;
        }
        m_pnPreParentCount[nDepth] = nPreParentCount;
        if(nCount)
        {
            double fNextParentValue = getTickValue(nDepth,0);
            if( fNextParentValue < fParentValue )
                fParentValue = fNextParentValue;
        }
    }
}

EquidistantTickIter::~EquidistantTickIter()
{
    delete[] m_pnPositions;
    delete[] m_pnPreParentCount;
    delete[] m_pbIntervalFinished;
}

sal_Int32 EquidistantTickIter::getStartDepth() const
{
    //find the depth of the first visible tickmark:
    //it is the depth of the smallest value
    sal_Int32 nReturnDepth=0;
    double fMinValue = DBL_MAX;
    for(sal_Int32 nDepth = 0; nDepth<=m_nMaxDepth ;nDepth++ )
    {
        sal_Int32 nCount = getTickCount(nDepth);
        if( !nCount )
            continue;
        double fThisValue = getTickValue(nDepth,0);
        if(fThisValue<fMinValue)
        {
            nReturnDepth = nDepth;
            fMinValue = fThisValue;
        }
    }
    return nReturnDepth;
}

double* EquidistantTickIter::firstValue()
{
    if( gotoFirst() )
    {
        m_fCurrentValue = getTickValue(m_nCurrentDepth, m_pnPositions[m_nCurrentDepth]);
        return &m_fCurrentValue;
    }
    return NULL;
}

TickInfo* EquidistantTickIter::firstInfo()
{
    if( m_pInfoTicks && gotoFirst() )
        return &(*m_pInfoTicks)[m_nCurrentDepth][m_pnPositions[m_nCurrentDepth]];
    return NULL;
}

sal_Int32 EquidistantTickIter::getIntervalCount( sal_Int32 nDepth )
{
    if(nDepth>static_cast<sal_Int32>(m_rIncrement.SubIncrements.size()) || nDepth<0)
        return 0;

    if(!nDepth)
        return m_nTickCount;

    return m_rIncrement.SubIncrements[nDepth-1].IntervalCount;
}

bool EquidistantTickIter::isAtLastPartTick()
{
    if(!m_nCurrentDepth)
        return false;
    sal_Int32 nIntervalCount = getIntervalCount( m_nCurrentDepth );
    if(!nIntervalCount || nIntervalCount == 1)
        return true;
    if( m_pbIntervalFinished[m_nCurrentDepth] )
        return false;
    sal_Int32 nPos = m_pnPositions[m_nCurrentDepth]+1;
    if(m_pnPreParentCount[m_nCurrentDepth])
        nPos += nIntervalCount-1 - m_pnPreParentCount[m_nCurrentDepth];
    bool bRet = nPos && nPos % (nIntervalCount-1) == 0;
    if(!nPos && !m_pnPreParentCount[m_nCurrentDepth]
             && m_pnPositions[m_nCurrentDepth-1]==-1 )
         bRet = true;
    return bRet;
}

bool EquidistantTickIter::gotoFirst()
{
    if( m_nMaxDepth<0 )
        return false;
    if( !m_nTickCount )
        return false;

    for(sal_Int32 nDepth = 0; nDepth<=m_nMaxDepth ;nDepth++ )
        m_pnPositions[nDepth] = -1;

    m_nCurrentPos   = 0;
    m_nCurrentDepth = getStartDepth();
    m_pnPositions[m_nCurrentDepth] = 0;
    return true;
}

bool EquidistantTickIter::gotoNext()
{
    if( m_nCurrentPos < 0 )
        return false;
    m_nCurrentPos++;

    if( m_nCurrentPos >= m_nTickCount )
        return false;

    if( m_nCurrentDepth==m_nMaxDepth && isAtLastPartTick() )
    {
        do
        {
            m_pbIntervalFinished[m_nCurrentDepth] = true;
            m_nCurrentDepth--;
        }
        while( m_nCurrentDepth && isAtLastPartTick() );
    }
    else if( m_nCurrentDepth<m_nMaxDepth )
    {
        do
        {
            m_nCurrentDepth++;
        }
        while( m_nCurrentDepth<m_nMaxDepth );
    }
    m_pbIntervalFinished[m_nCurrentDepth] = false;
    m_pnPositions[m_nCurrentDepth] = m_pnPositions[m_nCurrentDepth]+1;
    return true;
}

double* EquidistantTickIter::nextValue()
{
    if( gotoNext() )
    {
        m_fCurrentValue = getTickValue(m_nCurrentDepth, m_pnPositions[m_nCurrentDepth]);
        return &m_fCurrentValue;
    }
    return NULL;
}

TickInfo* EquidistantTickIter::nextInfo()
{
    if( m_pInfoTicks && gotoNext() &&
        static_cast< sal_Int32 >(
            (*m_pInfoTicks)[m_nCurrentDepth].size()) > m_pnPositions[m_nCurrentDepth] )
    {
        return &(*m_pInfoTicks)[m_nCurrentDepth][m_pnPositions[m_nCurrentDepth]];
    }
    return NULL;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
