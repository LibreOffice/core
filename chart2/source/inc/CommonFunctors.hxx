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
#ifndef INCLUDED_CHART2_SOURCE_INC_COMMONFUNCTORS_HXX
#define INCLUDED_CHART2_SOURCE_INC_COMMONFUNCTORS_HXX

#include <algorithm>
#include <o3tl/any.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include "charttoolsdllapi.hxx"

namespace chart
{
namespace CommonFunctors
{

/** unary function to convert any type T into a css::uno::Any.

    <p>uno::makeAny is an inline function.  Thus is cannot be taken directly
    (via mem_fun_ptr)</p>
*/
template< typename T >
    struct makeAny
{
    css::uno::Any operator() ( const T & aVal )
    {
        return css::uno::makeAny( aVal );
    }
};

/** unary function to convert css::uno::Any into a double number.

    <p>In case no number can be generated from the Any, NaN (see
    rtl::math::SetNAN()) is returned.</p>
*/
struct AnyToDouble
{
    double operator() ( const css::uno::Any & rAny )
    {
        double fResult;
        ::rtl::math::setNan( & fResult );
        rAny >>= fResult;
        return fResult;
    }
};

/** unary function to convert css::uno::Any into an
    OUString.
*/
struct AnyToString
{
    OUString operator() ( const css::uno::Any & rAny )
    {
        if( auto pDouble = o3tl::tryAccess<double>(rAny) )
        {
            if( ::rtl::math::isNan(*pDouble) )
                return OUString();
            return ::rtl::math::doubleToUString(
                * pDouble,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, // use maximum decimal places available
                sal_Char( '.' ), // decimal separator
                true // remove trailing zeros
                );
        }
        else if( auto s = o3tl::tryAccess<OUString>(rAny) )
        {
            return *s;
        }

        return OUString();
    }
};

/** unary function to convert an OUString into a double number.

    <p>For conversion rtl::math::StringToDouble is used.</p>
 */
struct OUStringToDouble
{
    double operator() ( const OUString & rStr )
    {
        rtl_math_ConversionStatus eConversionStatus;
        double fResult = ::rtl::math::stringToDouble( rStr, '.', ',', & eConversionStatus );

        if( eConversionStatus != rtl_math_ConversionStatus_Ok )
            ::rtl::math::setNan( & fResult );

        return fResult;
    }
};

/** unary function to convert a double number into an OUString.

    <p>For conversion rtl::math::DoubleToOUString is used.</p>
 */
struct DoubleToOUString
{
    OUString operator() ( double fNumber )
    {
        return ::rtl::math::doubleToUString(
            fNumber,
            rtl_math_StringFormat_Automatic,
            rtl_math_DecimalPlaces_Max, // use maximum decimal places available
            static_cast< sal_Char >( '.' ),
            true
            );
    }
};

} //  namespace CommonFunctors
} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_COMMONFUNCTORS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
