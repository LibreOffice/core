/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatisticsHelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:35:03 $
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
#include "StatisticsHelper.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

using ::com::sun::star::uno::Sequence;

namespace
{

double lcl_getVariance( const Sequence< double > & rData, sal_Int32 & rOutValidCount,
        bool bUnbiasedEstimator )
{
    const sal_Int32 nCount = rData.getLength();
    rOutValidCount = nCount;

    double fSum = 0.0;
    double fQuadSum = 0.0;

    for( sal_Int32 i = 0; i < nCount; ++i )
    {
        const double fData = rData[i];
        if( ::rtl::math::isNan( fData ))
            --rOutValidCount;
        else
        {
            fSum     += fData;
            fQuadSum += fData * fData;
        }
    }

    double fResult;
    if( rOutValidCount == 0 )
        ::rtl::math::setNan( & fResult );
    else
    {
        const double fN = static_cast< double >( rOutValidCount );
        if( bUnbiasedEstimator )
            fResult = (fQuadSum - fSum*fSum/fN) / (fN - 1);
        else
            fResult = (fQuadSum - fSum*fSum/fN) / fN;
    }

    return fResult;
}

} // anonymous namespace

namespace chart
{

// static
double StatisticsHelper::getVariance(
    const Sequence< double > & rData,
    bool bUnbiasedEstimator /* = false */ )
{
    sal_Int32 nValCount;
    return lcl_getVariance( rData, nValCount, bUnbiasedEstimator );
}

// static
double StatisticsHelper::getStandardDeviation( const Sequence< double > & rData )
{
    double fResult = getVariance( rData );
    if( ! ::rtl::math::isNan( fResult ))
        fResult = sqrt( fResult );

    return fResult;
}

// static
double StatisticsHelper::getStandardError( const Sequence< double > & rData )
{
    sal_Int32 nValCount;
    double fVar = lcl_getVariance( rData, nValCount, false );
    double fResult;

    if( nValCount == 0 ||
        ::rtl::math::isNan( fVar ))
    {
        ::rtl::math::setNan( & fResult );
    }
    else
    {
        // standard-deviation / sqrt(n)
        fResult = sqrt( fVar ) / sqrt( double(nValCount) );
    }

    return fResult;
}


} //  namespace chart
