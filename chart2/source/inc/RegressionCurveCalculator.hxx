/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveCalculator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 11:57:52 $
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
#ifndef CHART2_REGRESSIONCURVECALCULATOR_HXX
#define CHART2_REGRESSIONCURVECALCULATOR_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/chart2/XRegressionCurveCalculator.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>

namespace chart
{

class RegressionCurveCalculator : public
    ::cppu::WeakImplHelper1< ::com::sun::star::chart2::XRegressionCurveCalculator >
{
public:
    RegressionCurveCalculator();
    virtual ~RegressionCurveCalculator();

    static bool isLinearScaling(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling > & xScaling );
    static bool isLogarithmicScaling(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling > & xScaling );

protected:
    virtual ::rtl::OUString ImplGetRepresentation(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xNumFormatter,
        ::sal_Int32 nNumberFormatKey ) const = 0;

    ::rtl::OUString getFormattedString(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& xNumFormatter,
        ::sal_Int32 nNumberFormatKey,
        double fNumber ) const;

    double m_fCorrelationCoeffitient;

    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL recalculateRegression(
        const ::com::sun::star::uno::Sequence< double >& aXValues,
        const ::com::sun::star::uno::Sequence< double >& aYValues )
        throw (::com::sun::star::uno::RuntimeException) = 0;
    virtual double SAL_CALL getCurveValue( double x )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException) = 0;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::geometry::RealPoint2D > SAL_CALL getCurveValues(
        double min,
        double max,
        ::sal_Int32 nPointCount,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >& xScalingX,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XScaling >& xScalingY,
        ::sal_Bool bMaySkipPointsInCalculation )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getCorrelationCoefficient()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRepresentation()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getFormattedRepresentation(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >& xNumFmtSupplier,
        ::sal_Int32 nNumberFormatKey )
        throw (::com::sun::star::uno::RuntimeException);
};

} //  namespace chart

// CHART2_REGRESSIONCURVECALCULATOR_HXX
#endif
