/*************************************************************************
 *
 *  $RCSfile: TickmarkHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-13 13:55:43 $
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
#include "TickmarkHelper.hxx"
#include "ViewDefines.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;
using namespace ::rtl::math;

TickInfo::TickInfo()
: fScaledTickValue( 0.0 )
, fUnscaledTickValue( 0.0 )
, nScreenTickValue( 0 )
, bPaintIt( true )
, xTextShape( NULL )
{
}

void TickInfo::updateUnscaledValue( const uno::Reference< XScaling >& xInverseScaling )
{
    if( xInverseScaling.is() )
        this->fUnscaledTickValue = xInverseScaling->doScaling( this->fScaledTickValue );
    else
        this->fUnscaledTickValue = this->fScaledTickValue;
}

TickIter::TickIter( const uno::Sequence< uno::Sequence< double > >& rTicks
                   , const ExplicitIncrementData& rIncrement
                   , sal_Int32 nMinDepth, sal_Int32 nMaxDepth )
                : m_pSimpleTicks(&rTicks), m_pInfoTicks(NULL)
                , m_rIncrement(rIncrement)
                , m_nMinDepth(0), m_nMaxDepth(0)
                , m_nTickCount(0), m_pnPositions(NULL)
                , m_pnPreParentCount(NULL), m_pbIntervalFinished(NULL)
                , m_nCurrentDepth(-1), m_nCurrentPos(-1), m_fCurrentValue( 0.0 )
{
    initIter( nMinDepth, nMaxDepth );
}

TickIter::TickIter( ::std::vector< ::std::vector< TickInfo > >& rTicks
                   , const ExplicitIncrementData& rIncrement
                   , sal_Int32 nMinDepth, sal_Int32 nMaxDepth )
                : m_pSimpleTicks(NULL), m_pInfoTicks(&rTicks)
                , m_rIncrement(rIncrement)
                , m_nMinDepth(0), m_nMaxDepth(0)
                , m_nTickCount(0), m_pnPositions(NULL)
                , m_pnPreParentCount(NULL), m_pbIntervalFinished(NULL)
                , m_nCurrentDepth(-1), m_nCurrentPos(-1), m_fCurrentValue( 0.0 )
{
    initIter( nMinDepth, nMaxDepth );
}

void TickIter::initIter( sal_Int32 nMinDepth, sal_Int32 nMaxDepth )
{
    m_nMaxDepth = nMaxDepth;
    if(nMaxDepth<0 || m_nMaxDepth>_getMaxDepth())
        m_nMaxDepth=_getMaxDepth();

    sal_Int32 nDepth = 0;
    for( nDepth = 0; nDepth<=m_nMaxDepth ;nDepth++ )
        m_nTickCount += _getTickCount(nDepth);

    if(!m_nTickCount)
        return;

    m_pnPositions      = new sal_Int32[m_nMaxDepth+1];

    m_pnPreParentCount = new sal_Int32[m_nMaxDepth+1];
    m_pbIntervalFinished = new bool[m_nMaxDepth+1];
    m_pnPreParentCount[0] = 0;
    m_pbIntervalFinished[0] = false;
    double fParentValue = _getTickValue(0,0);
    for( nDepth = 1; nDepth<=m_nMaxDepth ;nDepth++ )
    {
        m_pbIntervalFinished[nDepth] = false;

        sal_Int32 nPreParentCount = 0;
        sal_Int32 nCount = _getTickCount(nDepth);
        for(sal_Int32 nN = 0; nN<nCount; nN++)
        {
            if(_getTickValue(nDepth,nN) < fParentValue)
                nPreParentCount++;
            else
                break;
        }
        m_pnPreParentCount[nDepth] = nPreParentCount;
        if(nCount)
        {
            double fNextParentValue = _getTickValue(nDepth,0);
            if( fNextParentValue < fParentValue )
                fParentValue = fNextParentValue;
        }
    }
}

TickIter::~TickIter()
{
    delete[] m_pnPositions;
    delete[] m_pnPreParentCount;
    delete[] m_pbIntervalFinished;
}

sal_Int32 TickIter::getStartDepth() const
{
    //find the depth of the first visible tickmark:
    //it is the depth of the smallest value
    sal_Int32 nReturnDepth=0;
    double fMinValue = DBL_MAX;
    for(sal_Int32 nDepth = 0; nDepth<=m_nMaxDepth ;nDepth++ )
    {
        sal_Int32 nCount = _getTickCount(nDepth);
        if( !nCount )
            continue;
        double fThisValue = _getTickValue(nDepth,0);
        if(fThisValue<fMinValue)
        {
            nReturnDepth = nDepth;
            fMinValue = fThisValue;
        }
    }
    return nReturnDepth;
}

double* TickIter::firstValue()
{
    if( gotoFirst() )
    {
        m_fCurrentValue = _getTickValue(m_nCurrentDepth, m_pnPositions[m_nCurrentDepth]);
        return &m_fCurrentValue;
    }
    return NULL;
}

TickInfo* TickIter::firstInfo()
{
    if( m_pInfoTicks && gotoFirst() )
        return &(*m_pInfoTicks)[m_nCurrentDepth][m_pnPositions[m_nCurrentDepth]];
    return NULL;
}

sal_Int32 TickIter::getIntervalCount( sal_Int32 nDepth )
{
    if(nDepth>m_rIncrement.SubIncrements.getLength() || nDepth<0)
        return 0;

    if(!nDepth)
        return m_nTickCount;

    return m_rIncrement.SubIncrements[nDepth-1].IntervalCount;
}

bool TickIter::isAtLastPartTick()
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

bool TickIter::gotoFirst()
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

bool TickIter::gotoNext()
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

double* TickIter::nextValue()
{
    if( gotoNext() )
    {
        m_fCurrentValue = _getTickValue(m_nCurrentDepth, m_pnPositions[m_nCurrentDepth]);
        return &m_fCurrentValue;
    }
    return NULL;
}

TickInfo* TickIter::nextInfo()
{
    if( m_pInfoTicks && gotoNext() )
        return &(*m_pInfoTicks)[m_nCurrentDepth][m_pnPositions[m_nCurrentDepth]];
    return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
double TickmarkHelper::getMinimumAtIncrement( double fMin, const ExplicitIncrementData& rIncrement )
{
    //the returned value will be <= fMin and on a Major Tick given by rIncrement
    if(rIncrement.Distance<=0.0)
        return fMin;

    double fRet = rIncrement.BaseValue +
        static_cast<sal_Int32>(
                approxSub( fMin, rIncrement.BaseValue )
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
double TickmarkHelper::getMaximumAtIncrement( double fMax, const ExplicitIncrementData& rIncrement )
{
    //the returned value will be >= fMax and on a Major Tick given by rIncrement
    if(rIncrement.Distance<=0.0)
        return fMax;

    double fRet = rIncrement.BaseValue +
        static_cast<sal_Int32>(
                approxSub( fMax, rIncrement.BaseValue )
                    / rIncrement.Distance)
            *rIncrement.Distance;

    if( fRet < fMax )
    {
        if( !approxEqual(fRet, fMax) )
            fRet += rIncrement.Distance;
    }
    return fRet;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TickmarkHelper::TickmarkHelper(
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
        DBG_ASSERT( m_xInverseScaling.is(), "each Scaling needs to return a inverse Scaling" );
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
    m_fOuterMajorTickBorderMin = TickmarkHelper::getMinimumAtIncrement( fMin, m_rIncrement );
    m_fOuterMajorTickBorderMax = TickmarkHelper::getMaximumAtIncrement( fMax, m_rIncrement );
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

TickmarkHelper::~TickmarkHelper()
{
    delete[] m_pfCurrentValues;
}

sal_Int32 TickmarkHelper::getTickDepth() const
{
    return m_rIncrement.SubIncrements.getLength() + 1;
}

sal_Int32 TickmarkHelper::getMaxTickCount( sal_Int32 nDepth ) const
{
    //return the maximum amount of ticks
    //possibly open intervals at the two ends of the region are handled as if they were completely visible
    //(this is necessary for calculating the sub ticks at the borders correctly)

    if( nDepth >= getTickDepth() )
        return 0;
    if( m_fOuterMajorTickBorderMax < m_fOuterMajorTickBorderMin )
        return 0;

    sal_Int32 nIntervalCount;
    if(m_rIncrement.PostEquidistant  )
        nIntervalCount = static_cast<sal_Int32>
                        ( approxSub( m_fScaledVisibleMax, m_fScaledVisibleMin )
                            / m_rIncrement.Distance );
    else
        nIntervalCount = static_cast<sal_Int32>
                        ( approxSub( m_rScale.Maximum, m_rScale.Minimum )
                            / m_rIncrement.Distance );
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

double* TickmarkHelper::getMajorTick( sal_Int32 nTick ) const
{
    m_pfCurrentValues[0] = m_fOuterMajorTickBorderMin + nTick*m_rIncrement.Distance;

    if(m_pfCurrentValues[0]>m_fOuterMajorTickBorderMax)
        return NULL;
    if(m_pfCurrentValues[0]<m_fOuterMajorTickBorderMin)
        return NULL;

    //return always the value after scaling
    if(!m_rIncrement.PostEquidistant && m_xInverseScaling.is() )
        m_pfCurrentValues[0] = m_rScale.Scaling->doScaling( m_pfCurrentValues[0] );

    return &m_pfCurrentValues[0];
}

double* TickmarkHelper::getMinorTick( sal_Int32 nTick, sal_Int32 nDepth
                            , double fStartParentTick, double fNextParentTick ) const
{
    //check validity of arguments
    {
        //DBG_ASSERT( fStartParentTick < fNextParentTick, "fStartParentTick >= fNextParentTick");
        if(fStartParentTick >= fNextParentTick)
            return NULL;
        if(nDepth>m_rIncrement.SubIncrements.getLength() || nDepth<=0)
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
    double& fTest = m_pfCurrentValues[nDepth];

    //return always the value after scaling
    if(!bPostEquidistant && m_xInverseScaling.is() )
        m_pfCurrentValues[nDepth] = m_rScale.Scaling->doScaling( m_pfCurrentValues[nDepth] );

    if( !isWithinOuterBorder( m_pfCurrentValues[nDepth] ) )
        return NULL;

    return &m_pfCurrentValues[nDepth];
}

bool TickmarkHelper::isWithinOuterBorder( double fScaledValue ) const
{
    if(fScaledValue>m_fOuterMajorTickBorderMax_Scaled)
        return false;
    if(fScaledValue<m_fOuterMajorTickBorderMin_Scaled)
        return false;

    return true;
}


bool TickmarkHelper::isVisible( double fScaledValue ) const
{
    if(fScaledValue>m_fScaledVisibleMax)
        return false;
    if(fScaledValue<m_fScaledVisibleMin)
        return false;

    return true;
}

bool TickmarkHelper::isPostEquidistant( sal_Int32 nDepth ) const
{
    if( nDepth<0 || nDepth>m_rIncrement.SubIncrements.getLength() )
    {
        DBG_ERROR("invalid depth for tickmark");
        return true;
    }

    if( nDepth==0 )
        return m_rIncrement.PostEquidistant;

    return m_rIncrement.SubIncrements[nDepth-1].PostEquidistant;
}

void TickmarkHelper::getAllTicks( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    uno::Sequence< uno::Sequence< double > > aAllTicks;

    //create point sequences for each tick depth
    sal_Int32 nDepthCount = this->getTickDepth();
    sal_Int32 nMaxMajorTickCount = this->getMaxTickCount( 0 );

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
        for( sal_Int32 nTick=0; nTick<nCheckCount && nTick<nCount; nTick++)
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
        rAllTickInfos[nDepth].resize( nCount );
        for(sal_Int32 nN = 0; nN<nCount; nN++)
        {
            rAllTickInfos[nDepth][nN].fScaledTickValue = aAllTicks[nDepth][nN];
        }
    }

    //-----------------------------------------
    //get the transformed screen values for all tickmarks in aAllTickInfos
    this->updateScreenValues( rAllTickInfos );

    //-----------------------------------------
    //'hide' tickmarks with identical screen values in aAllTickInfos
    this->hideIdenticalScreenValues( rAllTickInfos );
}

void TickmarkHelper::addSubTicks( sal_Int32 nDepth, uno::Sequence< uno::Sequence< double > >& rParentTicks ) const
{
    TickIter aIter( rParentTicks, m_rIncrement, 0, nDepth-1 );
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
    if(m_rIncrement.SubIncrements.getLength()>nDepth)
        addSubTicks( nDepth+1, rParentTicks );
}

//-----------------------------------------------------------------------------
// ___TickmarkHelper_3D___
//-----------------------------------------------------------------------------
TickmarkHelper_2D::TickmarkHelper_2D(
          const ExplicitScaleData& rScale, const ExplicitIncrementData& rIncrement
          , double fStrech_SceneToScreen, double fOffset_SceneToScreen )
          : TickmarkHelper( rScale, rIncrement )
          , m_fStrech_LogicToScreen(1.0)
          , m_fOffset_LogicToScreen(0.0)
{
    double fWidthY = m_fScaledVisibleMax - m_fScaledVisibleMin;
    m_fStrech_LogicToScreen = FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthY * fStrech_SceneToScreen;
    m_fOffset_LogicToScreen = -m_fScaledVisibleMin*m_fStrech_LogicToScreen + fOffset_SceneToScreen;
}

TickmarkHelper_2D::~TickmarkHelper_2D()
{
}

sal_Int32 TickmarkHelper_2D::transformScaledLogicTickToScreen( double fValue ) const
{
    sal_Int32 nRet = static_cast<sal_Int32>(
        fValue*m_fStrech_LogicToScreen + m_fOffset_LogicToScreen);
    return nRet;
}

//static
sal_Int32 TickmarkHelper_2D::getTickScreenDistance( TickIter& rIter )
{
    //return the positive distance between the two first tickmarks in screen values
    //if there are less than two tickmarks -1 is returned

    const TickInfo* pFirstTickInfo = rIter.firstInfo();
    const TickInfo* pSecondTickInfo = rIter.nextInfo();
    if(!pSecondTickInfo  || !pFirstTickInfo)
        return -1;

    sal_Int32 nRet = pSecondTickInfo->nScreenTickValue - pFirstTickInfo->nScreenTickValue;
    if(nRet<0)
        nRet *= -1;
    return nRet;
}

sal_Int32 TickmarkHelper_2D::getScreenValueForMinimum() const
{
    //return the screen value at the end of the axis where the scale has its minimum
    //(lower end of axis line)
    return this->transformScaledLogicTickToScreen(
                    m_fScaledVisibleMin );
}

sal_Int32 TickmarkHelper_2D::getScreenValueForMaximum() const
{
    //return the screen value at the end of the axis where the scale has its maximum
    //(upper end of axis line)
    return this->transformScaledLogicTickToScreen(
                    m_fScaledVisibleMax );
}

void TickmarkHelper_2D::updateScreenValues( ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    //get the transformed screen values for all tickmarks in rAllTickInfos
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter       = rAllTickInfos.begin();
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = rAllTickInfos.end();
    for( ; aDepthIter != aDepthEnd; aDepthIter++ )
    {
        ::std::vector< TickInfo >::iterator       aTickIter = (*aDepthIter).begin();
        const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
        for( ; aTickIter != aTickEnd; aTickIter++ )
        {
            TickInfo& rTickInfo = (*aTickIter);
            rTickInfo.nScreenTickValue =
                this->transformScaledLogicTickToScreen(
                    rTickInfo.fScaledTickValue );
        }
    }
}

//'hide' tickmarks with identical screen values in aAllTickInfos
void TickmarkHelper_2D::hideIdenticalScreenValues(
        ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    TickIter aIter( rAllTickInfos, m_rIncrement );

    TickInfo* pPreviousTickInfo = aIter.firstInfo();
    if(!pPreviousTickInfo)
        return;
    pPreviousTickInfo->bPaintIt = true;
    for( TickInfo* pTickInfo = aIter.nextInfo(); pTickInfo; pTickInfo = aIter.nextInfo())
    {
        pTickInfo->bPaintIt = pTickInfo->nScreenTickValue != pPreviousTickInfo->nScreenTickValue;
        pPreviousTickInfo = pTickInfo;
    }
}
//-----------------------------------------------------------------------------
// ___TickmarkHelper_3D___
//-----------------------------------------------------------------------------
TickmarkHelper_3D::TickmarkHelper_3D(
          const ExplicitScaleData& rScale, const ExplicitIncrementData& rIncrement )
          : TickmarkHelper( rScale, rIncrement )
{
}

TickmarkHelper_3D::~TickmarkHelper_3D()
{
}

//.............................................................................
} //namespace chart
//.............................................................................
