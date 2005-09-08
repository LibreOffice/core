/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MeanValueRegressionCurveCalculator.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:31:08 $
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
#include "MeanValueRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace chart
{

MeanValueRegressionCurveCalculator::MeanValueRegressionCurveCalculator() :
        m_fMeanValue( 0.0 ),
        m_fCorrelationCoeffitient( 0.0 )
{
    ::rtl::math::setNan( & m_fMeanValue );
    ::rtl::math::setNan( & m_fCorrelationCoeffitient );
}

MeanValueRegressionCurveCalculator::~MeanValueRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL MeanValueRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    sal_Int32 nMax = aYValues.getLength();
    double fSumY = 0.0;
    const double * pY = aYValues.getConstArray();

    for( sal_Int32 i = 0; i < nMax; ++i )
    {
        if( ::rtl::math::isNan( pY[i] ) ||
            ::rtl::math::isInf( pY[i] ))
            --nMax;
        else
            fSumY += pY[i];
    }

    if( nMax == 0 )
        ::rtl::math::setNan( & m_fMeanValue );
    else
        m_fMeanValue = fSumY / static_cast< double >( nMax );

    // todo: calculate r for constant regression (== standard deviation?)
//     m_fCorrelationCoeffitient = ?;
}

double SAL_CALL MeanValueRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    return m_fMeanValue;
}

double SAL_CALL MeanValueRegressionCurveCalculator::getCorrelationCoefficient()
    throw (uno::RuntimeException)
{
    return m_fCorrelationCoeffitient;
}

OUString SAL_CALL MeanValueRegressionCurveCalculator::getRepresentation()
    throw (uno::RuntimeException)
{
    OUStringBuffer aBuf( C2U( "f(x) = " ));

    aBuf.append( NUMBER_TO_STR( m_fMeanValue ));

    return aBuf.makeStringAndClear();
}

} //  namespace chart
