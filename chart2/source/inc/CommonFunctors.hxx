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
#pragma once

#include <o3tl/any.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include "charttoolsdllapi.hxx"

#include <algorithm>
#include <limits>

namespace chart::CommonFunctors
{

/** unary function to convert any type T into a css::uno::Any.

    <p>uno::makeAny is an inline function.  Thus is cannot be taken directly
    (via mem_fun_ptr)</p>
*/
struct makeAny
{
    template< typename T >
    css::uno::Any operator() ( const T & aVal )
    {
        return css::uno::Any( aVal );
    }
};

/** unary function to convert an OUString or css::uno::Any into a double number.

    <p>For conversion of OUString, rtl::math::StringToDouble is used.</p>
    <p>In case no number can be generated from the Any, NaN is returned.</p>
 */
struct OOO_DLLPUBLIC_CHARTTOOLS ToDouble
{
    double operator() ( const css::uno::Any & rAny )
    {
        double fResult = std::numeric_limits<double>::quiet_NaN();
        rAny >>= fResult;
        return fResult;
    }

    double operator() ( std::u16string_view rStr )
    {
        rtl_math_ConversionStatus eConversionStatus;
        double fResult = ::rtl::math::stringToDouble( rStr, '.', ',', & eConversionStatus );

        if( eConversionStatus != rtl_math_ConversionStatus_Ok )
            return std::numeric_limits<double>::quiet_NaN();

        return fResult;
    }
};

/** unary function to convert a double number or css::uno::Any into an OUString.

    <p>For conversion of doubles, rtl::math::DoubleToOUString is used.</p>
 */
struct OOO_DLLPUBLIC_CHARTTOOLS ToString
{
    OUString operator() ( const css::uno::Any & rAny )
    {
        if( auto pDouble = o3tl::tryAccess<double>(rAny) )
        {
            if( std::isnan(*pDouble) )
                return OUString();
            return ::rtl::math::doubleToUString(
                * pDouble,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, // use maximum decimal places available
                '.', // decimal separator
                true // remove trailing zeros
                );
        }
        else if( auto s = o3tl::tryAccess<OUString>(rAny) )
        {
            return *s;
        }

        return OUString();
    }

    OUString operator() ( double fNumber )
    {
        return ::rtl::math::doubleToUString(
            fNumber,
            rtl_math_StringFormat_Automatic,
            rtl_math_DecimalPlaces_Max, // use maximum decimal places available
            '.',
            true
            );
    }
};

template <class Container, class Func> auto convertToSequence(const Container& container, Func f)
{
    css::uno::Sequence<decltype(f(container[0]))> result(container.size());
    std::transform(container.begin(), container.end(), result.getArray(), f);
    return result;
}

} //  namespace chart::CommonFunctors

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
