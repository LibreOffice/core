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


#ifndef INCLUDED_SVX_DESCRIPTIONGENERATOR_HXX
#define INCLUDED_SVX_DESCRIPTIONGENERATOR_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ustrbuf.hxx>
#include <svx/svxdllapi.h>


namespace accessibility {

/** This class creates description strings for shapes.
    <p>Initialized with a given shape additional calls to the
    <member>addProperty</member> method will build a descriptive string that
    starts with a general shape description and the shapes style.  Appended
    are all the specified property names and values that differ from the
    default values in the style.</p>
*/
class SVX_DLLPUBLIC DescriptionGenerator
{
public:
    enum PropertyType {
        COLOR,
        INTEGER,
        STRING,
        FILL_STYLE
    };

    /** Creates a new description generator with an empty description
        string.  Usually you will want to call initialize next to specifiy
        a general description of the shape.
        @param xShape
            The shape from which properties will be extracted by later calls
            to <member>addProperty</member>.
    */
    DescriptionGenerator (const css::uno::Reference<
        css::drawing::XShape>& xShape);

    ~DescriptionGenerator();

    /** Initialize the description with the given prefix followed by the
        shape's style in parantheses and a colon.
        @param sPrefix
            An introductory description of the shape that is made more
            specific by later calls to <member>addProperty</member>.
    */
    void Initialize (const OUString& sPrefix);

    /** Initialize the description with the specified string from the
        resource followed by the shape's style in parantheses and a colon.
        @param nResourceId
            A resource id the specifies the introductory description of the
            shape that is made more specific by later calls to
            <member>addProperty</member>.
    */
    void Initialize (sal_Int32 nResourceId);

    /**  Returns the description string and then resets it.  Usually called
         as last method before destroying the object.
         @return
             The description string in its current form.
    */
    OUString operator() (void);

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
        @param nWhichId
            This which id is used to localize the property value.  If it is
            not known a value of -1 signals to use a default representation.
    */
    void AddProperty (const OUString& sPropertyName,
        PropertyType aType,
        const OUString& sLocalizedName=OUString(),
        long nWhichId=-1);

    /** Add the given property name and its associated value to the
        description string.  If the property value does not differ from the
        default value of the shape's style then the description string is
        not modified.  This method forwards the request to its cousin but
        first replaces the id of the localized name by the associated string
        from the resource.
        @param sPropertyName
            The Name of the property to append.
        @param aType
            Type of the property's value.  It controls the transformation
            into the value's string representation.
        @param nResourceId
            Id of the kocalized name of the property int the resource.
        @param nWhichId
            This which id is used to localize the property value.  If it is
            not known a value of -1 signals to use a default representation.
    */
    void AddProperty (const OUString& sPropertyName,
        PropertyType aType,
        sal_Int32 nResourceId,
        long nWhichId=-1);

    /** Append the given string as is to the current description.
        @param sString
            String to append to the current description.  It is not modified
            in any way.
    */
    void AppendString (const OUString& sString);

    /** Add properties that describe line and border attributes.
    */
    void AddLineProperties();

    /** Add properties that describe how areas are filled.
    */
    void AddFillProperties();

    /** Add properties that describesattributes of 3D objects.
    */
    void Add3DProperties();

    /** Add properties that describe text attributes.
    */
    void AddTextProperties();

private:
    /// Reference to the shape from which the properties are extracted.
    css::uno::Reference< css::drawing::XShape> mxShape;

    /// Reference to the shape's property set.
    css::uno::Reference< css::beans::XPropertySet> mxSet;

    /// The description string that is build.
    OUStringBuffer msDescription;

    /** This flag is used to determine whether to insert a separator e.g. a
        comma before the next property.
    */
    bool mbIsFirstProperty;

    /** Add a property value formatted as color to the description string.
    */
    SVX_DLLPRIVATE void AddColor (const OUString& sPropertyName,
        const OUString& sLocalizedName);

    /** Add a property value formatted as integer to the description string.
    */
    SVX_DLLPRIVATE void AddInteger (const OUString& sPropertyName,
        const OUString& sLocalizedName);

    /** Add a property value formatted as string to the description string.
        @param sPropertyName
            Name of the property.
    */
    SVX_DLLPRIVATE void AddString (const OUString& sPropertyName,
        const OUString& sLocalizedName, long nWhichId = -1);

    /** Add a property value formatted as fill style to the description
        string.  If the fill style is <const>HATCH</const>,
        <const>GRADIENT</const>, or <const>BITMAP</const>, then the of the
        hatch, gradient, or bitmap is appended as well.
        @param sPropertyName
            Name of the property.  Usually this will be "FillStyle".
    */
    SVX_DLLPRIVATE void AddFillStyle (const OUString& sPropertyName,
        const OUString& sLocalizedName);
};


} // end of namespace accessibility


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
