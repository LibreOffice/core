/*************************************************************************
 *
 *  $RCSfile: ScaleAutomatism.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:33 $
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
#include "ScaleAutomatism.hxx"
#include "macros.hxx"
#include "TickmarkHelper.hxx"

#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif

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

ScaleAutomatism::ScaleAutomatism( const ScaleData& rSourceScale )
                    : m_fValueMinimum( 0.0 )
                    , m_fValueMaximum( 0.0 )
                    , m_nMaximumAutomaticMainIncrementCount(10)
                    , m_aSourceScale( rSourceScale )
                    , m_aSourceIncrement()
                    , m_aSourceSubIncrementList()
{
    ::rtl::math::setNan( &m_fValueMinimum );
    ::rtl::math::setNan( &m_fValueMaximum );
}

ScaleAutomatism::ScaleAutomatism( const ScaleData& rSourceScale
                    , const IncrementData& rSourceIncrement
                    , const uno::Sequence< SubIncrement >& rSourceSubIncrementList )
                    : m_fValueMinimum( 0.0 )
                    , m_fValueMaximum( 0.0 )
                    , m_nMaximumAutomaticMainIncrementCount(9)
                    , m_aSourceScale( rSourceScale )
                    , m_aSourceIncrement( rSourceIncrement )
                    , m_aSourceSubIncrementList( rSourceSubIncrementList )
{
    ::rtl::math::setNan( &m_fValueMinimum );
    ::rtl::math::setNan( &m_fValueMaximum );
}

ScaleAutomatism::~ScaleAutomatism()
{
}

//@todo these method should become part of the scaling interface and implementation somehow
//@todo problem with outparamters at api
ExplicitIncrementData getExplicitIncrementAndScaleForLogarithm(
        bool bChangeMinimumToIncrementRythm, bool bChangeMaximumToIncrementRythm
        , sal_Int32 nMaximumAutomaticMainIncrementCount
        , ExplicitScaleData& rExplicitScale
        , const IncrementData& rSourceIncrement
        , const uno::Sequence< SubIncrement >& rSourceSubIncrementList )
{
    //minimum and maximum of the ExplicitScaleData may be changed
    //to suiteable values if allowed
    //but they will definitly be changed if they are out of allowed borders

    if( nMaximumAutomaticMainIncrementCount <= 0 )
        nMaximumAutomaticMainIncrementCount = 5;

    //make sure that minimum and maximum are not out of allowed range
    {
        if( rExplicitScale.Maximum<=0.0 )
            rExplicitScale.Maximum = 100.0;
        if( rExplicitScale.Minimum<=0.0 )
        {
            rExplicitScale.Minimum = 0.1;
            if( rExplicitScale.Minimum >= rExplicitScale.Maximum )
                rExplicitScale.Minimum =  log(rExplicitScale.Maximum)/10.0;
        }
    }

    ExplicitIncrementData aRet;
    if(!(rSourceIncrement.PostEquidistant>>=aRet.PostEquidistant))
    {
        //maybe scaling dependent
        aRet.PostEquidistant = sal_True;
    }
    if(!(rSourceIncrement.Distance>>=aRet.Distance))
    {
        //autocalculate the distance
        if(aRet.PostEquidistant && rExplicitScale.Scaling.is() )
        {
            double fRange = rExplicitScale.Scaling->doScaling( rExplicitScale.Maximum )
                          - rExplicitScale.Scaling->doScaling( rExplicitScale.Minimum );
            double fSlice = fRange/nMaximumAutomaticMainIncrementCount;
            //make a fine value out of fSlice now:
            //only integers are reasonable as distance values
            sal_Int32 nDistance = static_cast<sal_Int32>(fSlice);
            if(nDistance<=0)
                nDistance=1;
            aRet.Distance = nDistance;
        }
        else
        {
            //@todo this was not tested
            double fRange = rExplicitScale.Maximum - rExplicitScale.Minimum;
            double fSlice = fRange/nMaximumAutomaticMainIncrementCount;
            //make a fine value out of fSlice now:
            double fSliceMagnitude = pow (10, floor (log10 (fSlice)));
            aRet.Distance = static_cast<sal_Int32>(fSlice/fSliceMagnitude)*fSliceMagnitude;
        }
    }

    if(!(rSourceIncrement.BaseValue>>=aRet.BaseValue))
    {
        //scaling dependent
        //@maybe todo is this default also plotter dependent ??
        aRet.BaseValue = 1.0;
    }
    else if( aRet.BaseValue<=0.0 ) //make sure that BaseValue is not out of allowed ranges
        aRet.BaseValue = 1.0;

    if(bChangeMinimumToIncrementRythm)
    {
        double fMin = rExplicitScale.Minimum;
        if( aRet.PostEquidistant && rExplicitScale.Scaling.is() )
            fMin = rExplicitScale.Scaling->doScaling(fMin);

        fMin = TickmarkHelper::getMinimumAtIncrement( fMin, aRet );
        if( aRet.PostEquidistant && rExplicitScale.Scaling.is() )
            fMin = rExplicitScale.Scaling->getInverseScaling()->doScaling(fMin);
        rExplicitScale.Minimum = fMin;

        if( rExplicitScale.Minimum<=0.0 )
        {
            rExplicitScale.Minimum = 0.1;
            if( rExplicitScale.Minimum >= rExplicitScale.Maximum )
                rExplicitScale.Minimum =  log(rExplicitScale.Maximum)/10.0;
        }
    }
    if(bChangeMaximumToIncrementRythm)
    {
        double fMax = rExplicitScale.Maximum;
        if( aRet.PostEquidistant && rExplicitScale.Scaling.is() )
            fMax = rExplicitScale.Scaling->doScaling(fMax);
        fMax = TickmarkHelper::getMaximumAtIncrement( fMax, aRet );
        if( aRet.PostEquidistant && rExplicitScale.Scaling.is() )
            fMax = rExplicitScale.Scaling->getInverseScaling()->doScaling(fMax);
        rExplicitScale.Maximum = fMax;
    }
    //---------------------------------------------------------------
    //fill explicit sub increment
    sal_Int32 nSubCount = rSourceSubIncrementList.getLength();
    aRet.SubIncrements.realloc(nSubCount);
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        const SubIncrement&     rSubIncrement         = rSourceSubIncrementList[nN];
        ExplicitSubIncrement&   rExplicitSubIncrement = aRet.SubIncrements[nN];

        if(!(rSubIncrement.IntervalCount>>=rExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            rExplicitSubIncrement.IntervalCount = 5;
        }
        if(!(rSubIncrement.PostEquidistant>>=rExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            rExplicitSubIncrement.PostEquidistant = sal_False;
        }
    }
    return aRet;
}

ExplicitIncrementData getExplicitIncrementAndScaleForLinear(
        bool bChangeMinimumToIncrementRythm, bool bChangeMaximumToIncrementRythm
        , sal_Int32 nMaximumAutomaticMainIncrementCount
        , ExplicitScaleData& rExplicitScale
        , const IncrementData& rSourceIncrement
        , const uno::Sequence< SubIncrement >& rSourceSubIncrementList )
{
    //minimum and maximum of the ExplicitScaleData may be changed
    //to suiteable values if allowed
    //but they will definitly be changed if they are out of allowed borders

    ExplicitIncrementData aRet;
    if(!(rSourceIncrement.PostEquidistant>>=aRet.PostEquidistant))
    {
        //maybe scaling dependent
        aRet.PostEquidistant = sal_True;
    }
    if(!(rSourceIncrement.Distance>>=aRet.Distance))
    {
        //autocalculate the distance
        double fRange = rExplicitScale.Maximum - rExplicitScale.Minimum;
        double fSlice = fRange/nMaximumAutomaticMainIncrementCount;

        //make a fine value out of fSlice now:
        double fSliceMagnitude = pow (10, floor (log10 (fSlice)));
        fSlice /= fSliceMagnitude;
        if(fSlice<=1.0)
            fSlice=1.0;
        else if( fSlice<= 2.0 )
            fSlice=2.0;
        else if( fSlice<= 2.5 )
            fSlice=2.5;
        else if( fSlice<= 5.0)
            fSlice=5.0;
        else
            fSlice=10.0;

        aRet.Distance = fSlice*fSliceMagnitude;
    }
    if(!(rSourceIncrement.BaseValue>>=aRet.BaseValue))
    {
        //@maybe todo is this default also plotter dependent ??
        aRet.BaseValue = 0.0;
    }
    if(bChangeMinimumToIncrementRythm)
    {
        rExplicitScale.Minimum = TickmarkHelper::getMinimumAtIncrement( rExplicitScale.Minimum, aRet );
    }
    if(bChangeMaximumToIncrementRythm)
    {
        rExplicitScale.Maximum = TickmarkHelper::getMaximumAtIncrement( rExplicitScale.Maximum, aRet );
    }
    //---------------------------------------------------------------
    //fill explicit sub increment
    sal_Int32 nSubCount = rSourceSubIncrementList.getLength();
    aRet.SubIncrements.realloc(nSubCount);
    for( sal_Int32 nN=0; nN<nSubCount; nN++ )
    {
        const SubIncrement&     rSubIncrement         = rSourceSubIncrementList[nN];
        ExplicitSubIncrement&   rExplicitSubIncrement = aRet.SubIncrements[nN];

        if(!(rSubIncrement.IntervalCount>>=rExplicitSubIncrement.IntervalCount))
        {
            //scaling dependent
            //@todo autocalculate IntervalCount dependent on MainIncrement and scaling
            rExplicitSubIncrement.IntervalCount = 2;
        }
        if(!(rSubIncrement.PostEquidistant>>=rExplicitSubIncrement.PostEquidistant))
        {
            //scaling dependent
            rExplicitSubIncrement.PostEquidistant = sal_False;
        }
    }
    return aRet;
}

void ScaleAutomatism::calculateExplicitScaleAndIncrement(
            ExplicitScaleData& rExplicitScale
            , ExplicitIncrementData& rExplicitIncrement )
{
    //---------------------------------------------------------------
    //fill explicit scale
    bool bChangeMinimumToIncrementRythm=false, bChangeMaximumToIncrementRythm=false;
    if(!(m_aSourceScale.Minimum>>=rExplicitScale.Minimum))
    {
        //autocalculate the minimum in first iteration
        //the increment is considered below
        if( !::rtl::math::isNan(m_fValueMinimum) )
            rExplicitScale.Minimum = m_fValueMinimum;
        else
            rExplicitScale.Minimum = 0.0;//@todo get Minimum from scsaling or from plotter????
        bChangeMinimumToIncrementRythm = true;
    }
    if(!(m_aSourceScale.Maximum>>=rExplicitScale.Maximum))
    {
        //autocalculate the maximum in first iteration
        //the increment is considered below
        if( !::rtl::math::isNan(m_fValueMaximum) )
            rExplicitScale.Maximum = m_fValueMaximum;
        else
            rExplicitScale.Maximum = 10.0;//@todo get Maximum from scaling or from plotter????
        bChangeMaximumToIncrementRythm=true;
    }
    rExplicitScale.Orientation = m_aSourceScale.Orientation;//AxisOrientation_MATHEMATICAL;
    rExplicitScale.Scaling = m_aSourceScale.Scaling;
    rExplicitScale.Breaks = m_aSourceScale.Breaks;
    //---------------------------------------------------------------
    //fill explicit increment
    //minimum and maximum of the ExplicitScaleData may be changed if allowed
    uno::Reference< lang::XServiceName > xServiceName( rExplicitScale.Scaling, uno::UNO_QUERY );
    bool bIsLogarithm = ( xServiceName.is() && (xServiceName->getServiceName()).equals(
                      C2U( "com.sun.star.chart2.LogarithmicScaling" )));
    if(bIsLogarithm)
        rExplicitIncrement = getExplicitIncrementAndScaleForLogarithm( bChangeMinimumToIncrementRythm, bChangeMaximumToIncrementRythm, m_nMaximumAutomaticMainIncrementCount
                            , rExplicitScale, m_aSourceIncrement, m_aSourceSubIncrementList );
    else
        rExplicitIncrement = getExplicitIncrementAndScaleForLinear( bChangeMinimumToIncrementRythm, bChangeMaximumToIncrementRythm, m_nMaximumAutomaticMainIncrementCount
                            , rExplicitScale, m_aSourceIncrement, m_aSourceSubIncrementList );
}

//.............................................................................
} //namespace chart
//.............................................................................
