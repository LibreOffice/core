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
#ifndef CHART_PROPERTYHELPER_HXX
#define CHART_PROPERTYHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/property.hxx>
#include <map>
#include "charttoolsdllapi.hxx"

namespace chart
{

typedef int tPropertyValueMapKey;

typedef ::std::map< tPropertyValueMapKey, ::com::sun::star::uno::Any >
    tPropertyValueMap;

namespace PropertyHelper
{

/** adds a line dash with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
OOO_DLLPUBLIC_CHARTTOOLS OUString addLineDashUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName );

/** adds a gradient with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
OOO_DLLPUBLIC_CHARTTOOLS OUString addGradientUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName );

/** adds a transparency gradient with a unique name to the gradient obtained
    by the given factory.

    @return The name used for storing this element in the table
*/
OOO_DLLPUBLIC_CHARTTOOLS
OUString addTransparencyGradientUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName );

/** adds a hatch with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
OOO_DLLPUBLIC_CHARTTOOLS OUString addHatchUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName );

/** adds a bitmap with a unique name to the gradient obtained by the given
    factory.

    @return The name used for storing this element in the table
*/
OOO_DLLPUBLIC_CHARTTOOLS OUString addBitmapUniqueNameToTable(
    const ::com::sun::star::uno::Any & rValue,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName );

/** Set a property to a certain value in the given map.  This works for
    properties that are already set, and those which are not yet in the map.

    @param any is the value encapsulated in the variant type Any
 */
OOO_DLLPUBLIC_CHARTTOOLS
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

OOO_DLLPUBLIC_CHARTTOOLS void setPropertyValueDefaultAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const ::com::sun::star::uno::Any & rAny );

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
OOO_DLLPUBLIC_CHARTTOOLS void setEmptyPropertyValueDefault( tPropertyValueMap & rOutMap, tPropertyValueMapKey key );

} // namespace PropertyHelper

struct OOO_DLLPUBLIC_CHARTTOOLS PropertyNameLess
{
    inline bool operator() ( const ::com::sun::star::beans::Property & first,
                             const ::com::sun::star::beans::Property & second )
    {
        return ( first.Name.compareTo( second.Name ) < 0 );
    }
};

struct OOO_DLLPUBLIC_CHARTTOOLS PropertyLess : public ::std::binary_function<
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

struct OOO_DLLPUBLIC_CHARTTOOLS PropertyValueNameEquals : public ::std::unary_function< ::com::sun::star::beans::PropertyValue, bool >
{
    explicit PropertyValueNameEquals( const OUString & rName ) :
            m_aName( rName )
    {}

    bool operator() ( const ::com::sun::star::beans::PropertyValue & rPropValue )
    {
        return rPropValue.Name.equals( m_aName );
    }

private:
    OUString m_aName;
};

} //  namespace chart

// CHART_PROPERTYHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
