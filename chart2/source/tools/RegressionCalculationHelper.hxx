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
#ifndef CHART2_REGRESSIONCALCULATIONHELPER_HXX
#define CHART2_REGRESSIONCALCULATIONHELPER_HXX

#include <rtl/math.hxx>

#include <utility>
#include <functional>
#include <vector>

#define NUMBER_TO_STR(number) (OStringToOUString(::rtl::math::doubleToString( \
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
