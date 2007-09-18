/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyHelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:57:46 $
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
#ifndef CHART_PROPERTYHELPER_HXX
#define CHART_PROPERTYHELPER_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <map>

namespace chart
{

typedef int tPropertyValueMapKey;

typedef ::std::map< tPropertyValueMapKey, ::com::sun::star::uno::Any >
    tPropertyValueMap;

namespace PropertyHelper
{

void copyProperties(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & xSource,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & xDestination
    );

/** adds a line dash with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
::rtl::OUString addLineDashUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const ::rtl::OUString & rPreferredName );

/** adds a gradient with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
::rtl::OUString addGradientUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const ::rtl::OUString & rPreferredName );

/** adds a transparency gradient with a unique name to the gradient obtained
    by the given factory.

    @return The name used for storing this element in the table
*/
::rtl::OUString addTransparencyGradientUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const ::rtl::OUString & rPreferredName );

/** adds a hatch with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
::rtl::OUString addHatchUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const ::rtl::OUString & rPreferredName );

/** adds a bitmap with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
::rtl::OUString addBitmapUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const ::rtl::OUString & rPreferredName );

// --------------------------------------------------------------------------------

/** Set a property to a certain value in the given map.  This works for
    properties that are already set, and those which are not yet in the map.

    @param any is the value encapsulated in the variant type Any
 */
void setPropertyValueAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key,
                          const ::com::sun::star::uno::Any & rAny );

/** Set a property to a certain value in the given map.  This works for
    properties that are already set, and those which are not yet in the map.

    @param value is the value of type Value that will be put into a variant type
        Any before set in the property map.
 */
template< typename Value >
    void setPropertyValue( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const Value & value )
{
    setPropertyValueAny( rOutMap, key, ::com::sun::star::uno::makeAny( value ));
}

template<>
    void setPropertyValue< ::com::sun::star::uno::Any >( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const ::com::sun::star::uno::Any & rAny );

void setPropertyValueDefaultAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const ::com::sun::star::uno::Any & rAny );

/** Calls setPropertyValue() but asserts that the given property hasn't been set
    before.
 */
template< typename Value >
    void setPropertyValueDefault( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const Value & value )
{
    setPropertyValueDefaultAny( rOutMap, key, ::com::sun::star::uno::makeAny( value ));
}

/** Calls setPropertyValue() but asserts that the given property hasn't been set
    before.
 */
template<>
    void setPropertyValueDefault< ::com::sun::star::uno::Any >( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const ::com::sun::star::uno::Any & rAny );

/** Calls setPropertyValueDefault() with an empty Any as value
 */
void setEmptyPropertyValueDefault( tPropertyValueMap & rOutMap, tPropertyValueMapKey key );


} // namespace PropertyHelper

// ================================================================================

struct PropertyNameLess
{
    inline bool operator() ( const ::com::sun::star::beans::Property & first,
                             const ::com::sun::star::beans::Property & second )
    {
        return ( first.Name.compareTo( second.Name ) < 0 );
    }
};

struct PropertyLess : public ::std::binary_function<
        ::com::sun::star::beans::Property,
        ::com::sun::star::beans::Property,
        bool >
{
    bool operator() ( const ::com::sun::star::beans::Property & rFirst,
                      const ::com::sun::star::beans::Property & rSecond )
    {
        return ( rFirst.Name.compareTo( rSecond.Name ) < 0 );
    }
};

struct PropertyValueNameEquals : public ::std::unary_function< ::com::sun::star::beans::PropertyValue, bool >
{
    explicit PropertyValueNameEquals( const ::rtl::OUString & rName ) :
            m_aName( rName )
    {}

    bool operator() ( const ::com::sun::star::beans::PropertyValue & rPropValue )
    {
        return rPropValue.Name.equals( m_aName );
    }

private:
    ::rtl::OUString m_aName;
};

} //  namespace chart

// CHART_PROPERTYHELPER_HXX
#endif
