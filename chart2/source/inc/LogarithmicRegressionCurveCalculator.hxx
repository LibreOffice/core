/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LogarithmicRegressionCurveCalculator.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:56:51 $
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
#ifndef CHART2_LOGARITHMICREGRESSIONCURVECALCULATOR_HXX
#define CHART2_LOGARITHMICREGRESSIONCURVECALCULATOR_HXX

#include "RegressionCurveCalculator.hxx"

namespace chart
{

class LogarithmicRegressionCurveCalculator : public RegressionCurveCalculator
{
public:
    LogarithmicRegressionCurveCalculator();
    virtual ~LogarithmicRegressionCurveCalculator();

protected:
    virtual ::rtl::OUString ImplGetRepresentation(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xNumFormatter,
        ::sal_Int32 nNumberFormatKey ) const;

private:
    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL recalculateRegression(
        const ::com::sun::star::uno::Sequence< double >& aXValues,
        const ::com::sun::star::uno::Sequence< double >& aYValues )
        throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getCurveValue( double x )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > SAL_CALL getCurveValues(
        double min,
        double max,
        ::sal_Int32 nPointCount,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >& xScalingX,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >& xScalingY,
        ::sal_Bool bMaySkipPointsInCalculation )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // formula is: f(x) = m_fSlope * log( x ) + m_fIntercept
    double m_fSlope;
    double m_fIntercept;
};

} //  namespace chart

// CHART2_LOGARITHMICREGRESSIONCURVECALCULATOR_HXX
#endif
