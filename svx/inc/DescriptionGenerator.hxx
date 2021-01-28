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

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustrbuf.hxx>

namespace com::sun::star::beans
{
class XPropertySet;
}
namespace com::sun::star::drawing
{
class XShape;
}

namespace accessibility
{
/** This class creates description strings for shapes.
    <p>Initialized with a given shape additional calls to the
    <member>addProperty</member> method will build a descriptive string that
    starts with a general shape description and the shapes style.  Appended
    are all the specified property names and values that differ from the
    default values in the style.</p>
*/
class DescriptionGenerator
{
public:
    enum class PropertyType
    {
        Color,
        Integer
    };

    /** Creates a new description generator with an empty description
        string.  Usually you will want to call initialize next to specify
        a general description of the shape.
        @param xShape
            The shape from which properties will be extracted by later calls
            to <member>addProperty</member>.
    */
    DescriptionGenerator(const css::uno::Reference<css::drawing::XShape>& xShape);

    ~DescriptionGenerator();

    /** Initialize the description with the given prefix followed by the
        shape style in parentheses and a colon.
        @param sPrefix
            An introductory description of the shape that is made more
            specific by later calls to <member>addProperty</member>.
    */
    void Initialize(const OUString& sPrefix);

    /** Initialize the description with the specified string from the
        resource followed by the shape style in parentheses and a colon.
        @param pResourceId
            A resource id the specifies the introductory description of the
            shape that is made more specific by later calls to
            <member>addProperty</member>.
    */
    void Initialize(const char* pResourceId);

    /**  Returns the description string and then resets it.  Usually called
         as last method before destroying the object.
         @return
             The description string in its current form.
    */
    OUString operator()(void);

    /** Add the given property name and its associated value to the
        description string.  If the property value does not differ from the
        default value of the shape's style then the description string is
        not modified.
        @param sPropertyName
            The Name of the property to append.
        @param aType
            Type of the property's value.  It controls the transformation
            into the value's string representation.
        @param sLocalizedName
            Localized name of the property.  An empty string tells the
            method to use the property name instead.
    */
    void AddProperty(const OUString& sPropertyName, PropertyType aType);

    /** Append the given string as is to the current description.
        @param sString
            String to append to the current description.  It is not modified
            in any way.
    */
    void AppendString(std::u16string_view sString);

private:
    /// Reference to the shape from which the properties are extracted.
    css::uno::Reference<css::drawing::XShape> mxShape;

    /// Reference to the shape's property set.
    css::uno::Reference<css::beans::XPropertySet> mxSet;

    /// The description string that is build.
    OUStringBuffer msDescription;

    /** This flag is used to determine whether to insert a separator e.g. a
        comma before the next property.
    */
    bool mbIsFirstProperty;

    /** Add a property value formatted as color to the description string.
    */
    void AddColor(const OUString& sPropertyName);

    /** Add a property value formatted as integer to the description string.
    */
    void AddInteger(const OUString& sPropertyName);
};

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
