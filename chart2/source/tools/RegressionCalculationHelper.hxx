/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCalculationHelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:07:00 $
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
#ifndef CHART2_REGRESSIONCALCULATIONHELPER_HXX
#define CHART2_REGRESSIONCALCULATIONHELPER_HXX

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <utility>
#include <functional>
#include <vector>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#define NUMBER_TO_STR(number) (::rtl::OStringToOUString(::rtl::math::doubleToString( \
          number, rtl_math_StringFormat_G, 4, '.', true ),RTL_TEXTENCODING_ASCII_US ))

#define UC_SPACE (sal_Unicode(' '))
#define UC_MINUS_SIGN (sal_Unicode('-'))
// #define UC_MINUS_SIGN (sal_Unicode(0x2212))

namespace chart
{
namespace RegressionCalculationHelper
{

typedef ::std::pair< ::std::vector< double >, ::std::vector< double > > tDoubleVectorPair;

/** takes the given x- and y-values and copyies them into the resulting pair,
    which contains x-values in the first element and the y-values in the second
    one.  All tuples for which aPred is false are not copied.

    <p>The functors below provide a set of useful predicates that can be
    used to pass as parameter aPred.</p>
 */
template< class Pred >
tDoubleVectorPair
    cleanup( const ::com::sun::star::uno::Sequence< double > & rXValues,
             const ::com::sun::star::uno::Sequence< double > & rYValues,
             Pred aPred )
{
    tDoubleVectorPair aResult;
    sal_Int32 nSize = ::std::min( rXValues.getLength(), rYValues.getLength());
    for( sal_Int32 i=0; i<nSize; ++i )
    {
        if( aPred( rXValues[i], rYValues[i] ))
        {
            aResult.first.push_back( rXValues[i] );
            aResult.second.push_back( rYValues[i] );
        }
    }

    return aResult;
}


class isValid : public ::std::binary_function< double, double, bool >
{
public:
    inline bool operator()( double x, double y )
    { return ! ( ::rtl::math::isNan( x ) ||
                 ::rtl::math::isNan( y ) ||
                 ::rtl::math::isInf( x ) ||
                 ::rtl::math::isInf( y ) );
    }
};

class isValidAndXPositive : public ::std::binary_function< double, double, bool >
{
public:
    inline bool operator()( double x, double y )
    { return ! ( ::rtl::math::isNan( x ) ||
                 ::rtl::math::isNan( y ) ||
                 ::rtl::math::isInf( x ) ||
                 ::rtl::math::isInf( y ) ||
                 x <= 0.0 );
    }
};

class isValidAndYPositive : public ::std::binary_function< double, double, bool >
{
public:
    inline bool operator()( double x, double y )
    { return ! ( ::rtl::math::isNan( x ) ||
                 ::rtl::math::isNan( y ) ||
                 ::rtl::math::isInf( x ) ||
                 ::rtl::math::isInf( y ) ||
                 y <= 0.0 );
    }
};

class isValidAndBothPositive : public ::std::binary_function< double, double, bool >
{
public:
    inline bool operator()( double x, double y )
    { return ! ( ::rtl::math::isNan( x ) ||
                 ::rtl::math::isNan( y ) ||
                 ::rtl::math::isInf( x ) ||
                 ::rtl::math::isInf( y ) ||
                 x <= 0.0 ||
                 y <= 0.0 );
    }
};

} //  namespace RegressionCalculationHelper
} //  namespace chart

// CHART2_REGRESSIONCALCULATIONHELPER_HXX
#endif
