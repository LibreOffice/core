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

#ifndef INCLUDED_COMPHELPER_PROPERTY_HXX
#define INCLUDED_COMPHELPER_PROPERTY_HXX

#include <cppuhelper/proptypehlp.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <functional>
#include <comphelper/comphelperdllapi.h>
#include <cppu/unotype.hxx>

namespace comphelper
{

    // comparing two property instances
    struct PropertyCompareByName : public ::std::binary_function< css::beans::Property, css::beans::Property, bool >
    {
        bool operator() (const css::beans::Property& x, const css::beans::Property& y) const
        {
            return x.Name.compareTo(y.Name) < 0;
        }
    };

    struct PropertyStringEqualFunctor : ::std::binary_function< css::beans::Property, OUString, bool >
    {

        inline bool operator()( const css::beans::Property& lhs, const OUString& rhs ) const
        {
            return lhs.Name == rhs ;
        }

        inline bool operator()( const OUString& lhs, const css::beans::Property& rhs ) const
        {
            return lhs == rhs.Name ;
        }
    };

    struct PropertyEqualByName : public ::std::binary_function< css::beans::Property, css::beans::Property, bool >
    {
        bool operator() (const css::beans::Property& x, const css::beans::Property& y) const
        {
            return x.Name == y.Name ;
        }
    };

/// remove the property with the given name from the given sequence
COMPHELPER_DLLPUBLIC void RemoveProperty(css::uno::Sequence<css::beans::Property>& seqProps, const OUString& _rPropName);

/** within the given property sequence, modify attributes of a special property
    @param  _rProps         the sequence of properties to search in
    @param  _sPropName      the name of the property which's attributes should be modified
    @param  _nAddAttrib     the attributes which should be added
    @param  _nRemoveAttrib  the attributes which should be removed
*/
COMPHELPER_DLLPUBLIC void ModifyPropertyAttributes(css::uno::Sequence<css::beans::Property>& _rProps, const OUString& _sPropName, sal_Int16 _nAddAttrib, sal_Int16 _nRemoveAttrib);

/** check if the given set has the given property.
*/
COMPHELPER_DLLPUBLIC bool hasProperty(const OUString& _rName, const css::uno::Reference<css::beans::XPropertySet>& _rxSet);

/** copy properties between property sets, in compliance with the property
    attributes of the target object
*/
COMPHELPER_DLLPUBLIC void copyProperties(const css::uno::Reference<css::beans::XPropertySet>& _rxSource,
                    const css::uno::Reference<css::beans::XPropertySet>& _rxDest);

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue
    @param          _rConvertedValue    the conversion result (if successful)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to the requested type (which is the template argument)
*/
template <typename T>
bool tryPropertyValue(css::uno::Any& /*out*/_rConvertedValue, css::uno::Any& /*out*/_rOldValue, const css::uno::Any& _rValueToSet, const T& _rCurrentValue)
{
    bool bModified(false);
    T aNewValue = T();
    ::cppu::convertPropertyValue(aNewValue, _rValueToSet);
    if (aNewValue != _rCurrentValue)
    {
        _rConvertedValue <<= aNewValue;
        _rOldValue <<= _rCurrentValue;
        bModified = true;
    }
    return bModified;
}

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue for enum values
    @param          _rConvertedValue    the conversion result (if successful)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to the requested type (which is the template argument)
*/
template <class ENUMTYPE>
bool tryPropertyValueEnum(css::uno::Any& /*out*/_rConvertedValue, css::uno::Any& /*out*/_rOldValue, const css::uno::Any& _rValueToSet, const ENUMTYPE& _rCurrentValue)
{
    if (cppu::getTypeFavourUnsigned(&_rCurrentValue).getTypeClass()
        != css::uno::TypeClass_ENUM)
        return tryPropertyValue(_rConvertedValue, _rOldValue, _rValueToSet, _rCurrentValue);

    bool bModified(false);
    ENUMTYPE aNewValue;
    ::cppu::any2enum(aNewValue, _rValueToSet);
        // will throw an exception if not convertible

    if (aNewValue != _rCurrentValue)
    {
        _rConvertedValue <<= aNewValue;
        _rOldValue <<= _rCurrentValue;
        bModified = true;
    }
    return bModified;
}

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue for boolean properties
    @param          _rConvertedValue    the conversion result (if successful)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to a boolean type
*/
inline bool tryPropertyValue(css::uno::Any& /*out*/_rConvertedValue, css::uno::Any& /*out*/_rOldValue, const css::uno::Any& _rValueToSet, bool _bCurrentValue)
{
    bool bModified(false);
    sal_Bool bNewValue(sal_False);
    ::cppu::convertPropertyValue(bNewValue, _rValueToSet);
    if (bool(bNewValue) != _bCurrentValue)
    {
        _rConvertedValue.setValue(&bNewValue, cppu::UnoType<bool>::get());
        _rOldValue.setValue(&_bCurrentValue, cppu::UnoType<bool>::get());
        bModified = true;
    }
    return bModified;
}

/** helper for implementing ::cppu::OPropertySetHelper::convertFastPropertyValue
    @param          _rConvertedValue    the conversion result (if successful)
    @param          _rOldValue          the old value of the property, calculated from _rCurrentValue
    @param          _rValueToSet        the new value which is about to be set
    @param          _rCurrentValue      the current value of the property
    @param          _rExpectedType      the type which the property should have (if not void)
    @return         sal_True, if the value could be converted and has changed
                    sal_False, if the value could be converted and has not changed
    @exception      InvalidArgumentException thrown if the value could not be converted to the requested type (which is the template argument)
*/
COMPHELPER_DLLPUBLIC bool tryPropertyValue(css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, const css::uno::Any& _rValueToSet, const css::uno::Any& _rCurrentValue, const css::uno::Type& _rExpectedType);

}

#endif // INCLUDED_COMPHELPER_PROPERTY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
