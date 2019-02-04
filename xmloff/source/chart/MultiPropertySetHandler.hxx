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

#ifndef INCLUDED_XMLOFF_SOURCE_CHART_MULTIPROPERTYSETHANDLER_HXX
#define INCLUDED_XMLOFF_SOURCE_CHART_MULTIPROPERTYSETHANDLER_HXX

#include <memory>

#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

/** @descr  MultiPropertySetHandler handles the two slightly different
        interfaces XPropertySet and XMultiPropertySet for accessing
        properties of an object.

        It uses the classes PropertyWrapperBase and the template
        PropertyWrapper for a type safe access to single properties.

        The function class OUStringComparison is used by a STL map to
        sort the properties by names.
*/

/** @descr  Base class for the templated property wrappers.
        Having a common base class allows to set a variable to the
        property's value without explicit knowledge of its type.
*/
class   PropertyWrapperBase
{
public:
    /** @descr  Create a class instance and store the given name.
        @param  rName   The name of the property.
    */
    explicit PropertyWrapperBase (const OUString & rName)
        :   msName (rName)
    {}
    virtual ~PropertyWrapperBase()
    {}

    /** @descr  Abstract interface of a method for setting a variables
            value to that of the property.
    */
    virtual void    SetValue    (const css::uno::Any & rValue) = 0;

    const OUString msName;
};

/** @descr  For every property type there will be one instantiation of this
        template class with its own and type specific version of SetValue.
*/
template<class T> class PropertyWrapper : public PropertyWrapperBase
{
public:
    /** @descr  Create a wrapper for a property of type T.
    */
    PropertyWrapper (const OUString & rName, T & rValue)
        :   PropertyWrapperBase (rName),
            mrValue (rValue)
    {}

    /** descr   Set the given value inside an Any to the variable referenced
        by the data member.
    */
    virtual void    SetValue    (const css::uno::Any & rValue) override
    {
        rValue >>= mrValue;
    }

private:
    /// Reference to a variable.  Its value can be modified by a call to SetValue.
    T   &   mrValue;
};

/** @descr  Function object for comparing two OUStrings.
*/
class   OUStringComparison
{
public:
    /// Compare two strings.  Returns true if the first is before the second.
    bool    operator()  (const OUString & a, const OUString & b) const
    {
        return (a.compareTo (b) < 0);
    }
};

/** @descr  This class lets you get the values from an object that either
        supports the interface XPropertySet or XMultiPropertySet.  If it
        supports both interfaces then XMultiPropertySet is preferred.

        Using it works in three steps.
        1.  Create an instance and pass a reference to the object from which to
            get the property values.
        2.  Make all properties whose values you want to get known to the object
            by using the Add method.  This creates instances of a template class
            that stores the properties name and a reference to the variable in
            which to store its value.
        3.  Finally call GetProperties to store the properties values into the
            variables specified in step 2.  This uses either the XPropertySet or
            (preferred) the XMultiPropertySet interface.
*/
class   MultiPropertySetHandler
{
public:
    /** @descr  Create a handler of the property set of the given
            object.
        @param  xObject A reference to any of the object's interfaces.
            not necessarily XPropertySet or XMultiPropertySet.  It
            is casted later to one of the two of them.
    */
    explicit MultiPropertySetHandler (css::uno::Reference<
        css::uno::XInterface> const & xObject);
    /** @descr  Add a property to handle.  The type given implicitly by the
            reference to a variable is used to create an instance of
            the PropertyWrapper template class.
        @param  sName   Name of the property.
        @param  rValue  Reference to a variable whose value is set by the
            call to GetProperties to the property's value.
    */
    template<class T> void  Add (const OUString & sName, T& rValue)
    {
        aPropertyList[sName] = std::make_unique<PropertyWrapper<T>>(sName, rValue);
    }

    /** @descr  Try to get the values for all properties added with the Add
            method.  If possible it uses the XMultiPropertySet.  If that fails
            (i.e. for an UnknownPropertyExcption) or if the interface is not
            supported it uses the XPropertySet interface.
        @return If none of the two interfaces is supported or using them both
            fails then sal_False is returned.  Else True is returned.
    */
    inline  bool    GetProperties();

private:
    /** @descr  Try to use the XMultiPropertySet interface to get the property
            values.
        @param  rNameList   A precomputed and sorted sequence of OUStrings
            containing the properties names.
        @return True if values could be derived.
    */
    inline  bool    MultiGet    (const css::uno::Sequence<
        OUString> & rNameList);

    /** @descr  Try to use the XPropertySet interface to get the property
            values.
        @param  rNameList   A precomputed and sorted sequence of OUStrings
            containing the properties names.
        @return True if values could be derived.
    */
    inline  bool    SingleGet   (const css::uno::Sequence<
        OUString> & rNameList);

    /** @descr  STL map that maps from property names to polymorphic instances of
            PropertyWrapper.  It uses OUStringComparison for sorting
            the property names.
    */
    ::std::map< OUString, std::unique_ptr<PropertyWrapperBase>, OUStringComparison> aPropertyList;

    /// The object from which to get the property values.
    css::uno::Reference< css::uno::XInterface>    mxObject;
};

MultiPropertySetHandler::MultiPropertySetHandler (css::uno::Reference<
    css::uno::XInterface> const & xObject)
        :   mxObject (xObject)
{
}

bool    MultiPropertySetHandler::GetProperties()
{
    css::uno::Sequence< OUString> aNameList (aPropertyList.size());
    int i = 0;
    for (const auto& rProperty : aPropertyList)
        aNameList[i++] = rProperty.second->msName;
    if ( ! MultiGet(aNameList))
        if ( ! SingleGet(aNameList))
            return false;
    return true;
}

bool    MultiPropertySetHandler::MultiGet   (const css::uno::Sequence<
    OUString> & rNameList)
{
    css::uno::Reference< css::beans::XMultiPropertySet> xMultiSet (
        mxObject, css::uno::UNO_QUERY);
    if (xMultiSet.is())
        try
        {
            int i = 0;
            css::uno::Sequence< css::uno::Any> aValueList =
                xMultiSet->getPropertyValues (rNameList);
            for (auto& rProperty : aPropertyList)
                rProperty.second->SetValue (aValueList[i++]);
        }
        catch (const css::beans::UnknownPropertyException&)
        {
            return false;
        }
    else
        return false;

    return true;
}

bool    MultiPropertySetHandler::SingleGet  (const css::uno::Sequence<
    OUString> & rNameList)
{
    css::uno::Reference< css::beans::XPropertySet> xSingleSet (
        mxObject, css::uno::UNO_QUERY);
    if (xSingleSet.is())
        try
        {
            int i = 0;
            for (auto& rProperty : aPropertyList)
                rProperty.second->SetValue (xSingleSet->getPropertyValue (rNameList[i++]));
        }
        catch (const css::beans::UnknownPropertyException&)
        {
            return false;
        }
    else
        return false;

    return true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
