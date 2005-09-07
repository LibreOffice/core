/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PotentialRegressionCurveCalculator.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:45:03 $
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
#ifndef CHART2_POTENTIALREGRESSIONCURVECALCULATOR_HXX
#define CHART2_POTENTIALREGRESSIONCURVECALCULATOR_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVECALCULATOR_HPP_
#include <com/sun/star/chart2/XRegressionCurveCalculator.hpp>
#endif

namespace chart
{


class PotentialRegressionCurveCalculator : public
    ::cppu::WeakImplHelper1< ::com::sun::star::chart2::XRegressionCurveCalculator >
{
public:
    PotentialRegressionCurveCalculator();
    virtual ~PotentialRegressionCurveCalculator();

protected:
    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL recalculateRegression(
        const ::com::sun::star::uno::Sequence< double >& aXValues,
        const ::com::sun::star::uno::Sequence< double >& aYValues )
        throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getCurveValue( double x )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getCorrelationCoefficient()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRepresentation()
        throw (::com::sun::star::uno::RuntimeException);

private:
    // formula is: f(x) = x ^ m_fSlope * m_fIntercept
    double m_fSlope;
    double m_fIntercept;
    double m_fCorrelationCoeffitient;
};

} //  namespace chart

// CHART2_POTENTIALREGRESSIONCURVECALCULATOR_HXX
#endif
