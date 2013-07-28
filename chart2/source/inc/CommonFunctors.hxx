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
#ifndef CHART2_COMMONFUNCTORS_HXX
#define CHART2_COMMONFUNCTORS_HXX

#include <algorithm>
#include <functional>
#include <o3tl/compat_functional.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "charttoolsdllapi.hxx"

namespace chart
{
namespace CommonFunctors
{

/** unary function to convert any type T into a ::com::sun::star::uno::Any.

    <p>uno::makeAny is an inline function.  Thus is cannot be taken directly
    (via mem_fun_ptr)</p>
*/
template< typename T >
    struct makeAny : public ::std::unary_function< T, ::com::sun::star::uno::Any >
{
    ::com::sun::star::uno::Any operator() ( const T & aVal )
    {
        return ::com::sun::star::uno::makeAny( aVal );
    }
};

/** unary function to convert ::com::sun::star::uno::Any into a double number.

    <p>In case no number can be generated from the Any, NaN (see
    rtl::math::SetNAN()) is returned.</p>
*/
struct OOO_DLLPUBLIC_CHARTTOOLS AnyToDouble : public ::std::unary_function< ::com::sun::star::uno::Any, double >
{
    double operator() ( const ::com::sun::star::uno::Any & rAny )
    {
        double fResult;
        ::rtl::math::setNan( & fResult );

        ::com::sun::star::uno::TypeClass eClass( rAny.getValueType().getTypeClass() );
        if( eClass == ::com::sun::star::uno::TypeClass_DOUBLE )
        {
            fResult = * reinterpret_cast< const double * >( rAny.getValue() );
        }

        return fResult;
    }
};

/** unary function to convert ::com::sun::star::uno::Any into an
    OUString.
*/
struct OOO_DLLPUBLIC_CHARTTOOLS AnyToString : public ::std::unary_function< ::com::sun::star::uno::Any,  OUString >
{
    OUString operator() ( const ::com::sun::star::uno::Any & rAny )
    {
        ::com::sun::star::uno::TypeClass eClass( rAny.getValueType().getTypeClass() );
        if( eClass == ::com::sun::star::uno::TypeClass_DOUBLE )
        {
            const double* pDouble = reinterpret_cast< const double * >( rAny.getValue() );
            if( ::rtl::math::isNan(*pDouble) )
                return OUString();
            return ::rtl::math::doubleToUString(
                * pDouble,
                rtl_math_StringFormat_Automatic,
                -1, // use maximum decimal places available
                sal_Char( '.' ), // decimal separator
                false // do not erase trailing zeros
                );
        }
        else if( eClass == ::com::sun::star::uno::TypeClass_STRING )
        {
            return * reinterpret_cast< const OUString * >( rAny.getValue() );
        }

        return OUString();
    }
};

/** unary function to convert an OUString into a double number.

    <p>For conversion rtl::math::StringToDouble is used.</p>
 */
struct OOO_DLLPUBLIC_CHARTTOOLS OUStringToDouble : public ::std::unary_function< OUString, double >
{
    double operator() ( const OUString & rStr )
    {
        rtl_math_ConversionStatus eConversionStatus;
        double fResult = ::rtl::math::stringToDouble( rStr, '.', ',', & eConversionStatus, NULL );

        if( eConversionStatus != rtl_math_ConversionStatus_Ok )
            ::rtl::math::setNan( & fResult );

        return fResult;
    }
};

/** unary function to convert a double number into an OUString.

    <p>For conversion rtl::math::DoubleToOUString is used.</p>
 */
struct OOO_DLLPUBLIC_CHARTTOOLS DoubleToOUString : public ::std::unary_function< double, OUString >
{
    OUString operator() ( double fNumber )
    {
        return ::rtl::math::doubleToUString(
            fNumber,
            rtl_math_StringFormat_Automatic,
            -1, // use maximum number of decimal places
            static_cast< sal_Char >( '.' ),
            false // do not erase trailing zeros
            );
    }
};

/** can be used to find an element with a specific first element in e.g. a
    vector of pairs (for searching keys in maps you will of course use map::find)
 */
template< typename First, typename Second >
    class FirstOfPairEquals : public ::std::unary_function< ::std::pair< First, Second >, bool >
{
public:
    FirstOfPairEquals( const First & aVal )
            : m_aValueToCompareWith( aVal )
    {}
    bool operator() ( const ::std::pair< First, Second > & rElem )
    {
        return rElem.first == m_aValueToCompareWith;
    }

private:
    First m_aValueToCompareWith;
};

/** can be used to find a certain value in a map

    ::std::find_if( aMap.begin(), aMap.end(),
                    SecondOfPairEquals< string, int >( 42 ));
 */
template< typename Key, typename Value >
    class SecondOfPairEquals : public ::std::unary_function< ::std::pair< Key, Value >, bool >
{
public:
    SecondOfPairEquals( const Value & aVal )
            : m_aValueToCompareWith( aVal )
    {}
    bool operator() ( const ::std::pair< Key, Value > & rMapElem )
    {
        return rMapElem.second == m_aValueToCompareWith;
    }

private:
    Value m_aValueToCompareWith;
};

/** Searches for data in a given map, i.e. not for the key but for the data
    pointed to by the keys.

    To find a key (value) you can use rMap.find( rValue )
 */
template< class MapType >
    inline typename MapType::const_iterator
    findValueInMap( const MapType & rMap, const typename MapType::mapped_type & rData )
{
    return ::std::find_if( rMap.begin(), rMap.end(),
                           ::o3tl::compose1( ::std::bind2nd(
                                                ::std::equal_to< typename MapType::mapped_type >(),
                                                rData ),
                                            ::o3tl::select2nd< typename MapType::value_type >()));
}

/** Functor that deletes the object behind the given pointer by calling the
    delete operator
 */
template< typename T >
    struct DeletePtr : public ::std::unary_function< T *, void >
{
    void operator() ( T * pObj )
    { delete pObj; }
};

} //  namespace CommonFunctors
} //  namespace chart

// CHART2_COMMONFUNCTORS_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
