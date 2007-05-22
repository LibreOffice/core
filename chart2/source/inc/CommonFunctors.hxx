/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CommonFunctors.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:13:55 $
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
#ifndef CHART2_COMMONFUNCTORS_HXX
#define CHART2_COMMONFUNCTORS_HXX

#include <algorithm>
#include <functional>

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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
struct AnyToDouble : public ::std::unary_function< ::com::sun::star::uno::Any, double >
{
    double operator() ( const ::com::sun::star::uno::Any & rAny )
    {
        double fResult;
        ::rtl::math::setNan( & fResult );

        ::com::sun::star::uno::TypeClass eClass( rAny.getValueType().getTypeClass() );
        if( eClass == ::com::sun::star::uno::TypeClass_STRING )
        {
            rtl_math_ConversionStatus eConversionStatus;
            fResult = ::rtl::math::stringToDouble(
                * reinterpret_cast< const ::rtl::OUString * >( rAny.getValue() ),
                sal_Char( '.' ), sal_Char( ',' ),
                & eConversionStatus, NULL );

            if( eConversionStatus != rtl_math_ConversionStatus_Ok )
                ::rtl::math::setNan( & fResult );
        }
        else if( eClass == ::com::sun::star::uno::TypeClass_DOUBLE )
        {
            fResult = * reinterpret_cast< const double * >( rAny.getValue() );
        }

        return fResult;
    }
};

/** unary function to convert ::com::sun::star::uno::Any into an
    ::rtl::OUString.
*/
struct AnyToString : public ::std::unary_function< ::com::sun::star::uno::Any,  ::rtl::OUString >
{
    ::rtl::OUString operator() ( const ::com::sun::star::uno::Any & rAny )
    {
        ::com::sun::star::uno::TypeClass eClass( rAny.getValueType().getTypeClass() );
        if( eClass == ::com::sun::star::uno::TypeClass_DOUBLE )
        {
            return ::rtl::math::doubleToUString(
                * reinterpret_cast< const double * >( rAny.getValue() ),
                rtl_math_StringFormat_Automatic,
                -1, // use maximum decimal places available
                sal_Char( '.' ), // decimal separator
                false // do not erase trailing zeros
                );
        }
        else if( eClass == ::com::sun::star::uno::TypeClass_STRING )
        {
            return * reinterpret_cast< const ::rtl::OUString * >( rAny.getValue() );
        }

        return ::rtl::OUString();
    }
};

/** unary function to convert an ::rtl::OUString into a double number.

    <p>For conversion rtl::math::StringToDouble is used.</p>
 */
struct OUStringToDouble : public ::std::unary_function< ::rtl::OUString, double >
{
    double operator() ( const ::rtl::OUString & rStr )
    {
        rtl_math_ConversionStatus eConversionStatus;
        double fResult = ::rtl::math::stringToDouble( rStr, '.', ',', & eConversionStatus, NULL );

        if( eConversionStatus != rtl_math_ConversionStatus_Ok )
            ::rtl::math::setNan( & fResult );

        return fResult;
    }
};

/** unary function to convert a double number into an ::rtl::OUString.

    <p>For conversion rtl::math::DoubleToOUString is used.</p>
 */
struct DoubleToOUString : public ::std::unary_function< double, ::rtl::OUString >
{
    ::rtl::OUString operator() ( double fNumber )
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

// ================================================================================

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
                           ::std::compose1( ::std::bind2nd(
                                                ::std::equal_to< typename MapType::mapped_type >(),
                                                rData ),
                                            ::std::select2nd< typename MapType::value_type >()));
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
