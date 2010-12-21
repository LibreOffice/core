/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#ifndef _SVX_ACCESSIBILITY_DESCRIPTION_GENERATOR_HXX
#define _SVX_ACCESSIBILITY_DESCRIPTION_GENERATOR_HXX

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ustrbuf.hxx>
#include "svx/svxdllapi.h"


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
    DescriptionGenerator (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& xShape);

    ~DescriptionGenerator (void);

    /** Initialize the description with the given prefix followed by the
        shape's style in parantheses and a colon.
        @param sPrefix
            An introductory description of the shape that is made more
            specific by later calls to <member>addProperty</member>.
    */
    void Initialize (::rtl::OUString sPrefix);

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
    ::rtl::OUString operator() (void);

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
    void AddProperty (const ::rtl::OUString& sPropertyName,
        PropertyType aType,
        const ::rtl::OUString& sLocalizedName=::rtl::OUString(),
        long nWhichId=-1);

    /** Add the given property name and its associated value to the
        description string.  If the property value does not differ from the
        default value of the shape's style then the description string is
        not modified.  This method forwards the request to its cousing but
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
    void AddProperty (const ::rtl::OUString& sPropertyName,
        PropertyType aType,
        sal_Int32 nResourceId,
        long nWhichId=-1);

    /** Append the given string as is to the current description.
        @param sString
            String to append to the current description.  It is not modified
            in any way.
    */
    void AppendString (const ::rtl::OUString& sString);

    /** This method adds for debuging and development the list of all known
        properties to the description.   Don't use in production code.
    */
    void AddPropertyNames (void);

    /** Add properties that describe line and border attributes.
    */
    void AddLineProperties (void);

    /** Add properties that describe how areas are filled.
    */
    void AddFillProperties (void);

    /** Add properties that describesattributes of 3D objects.
    */
    void Add3DProperties (void);

    /** Add properties that describe text attributes.
    */
    void AddTextProperties (void);

private:
    /// Reference to the shape from which the properties are extracted.
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape> mxShape;

    /// Reference to the shape's property set.
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> mxSet;

    /// The description string that is build.
    ::rtl::OUStringBuffer msDescription;

    /** This flag is used to determine whether to insert a separator e.g. a
        comma before the next property.
    */
    bool mbIsFirstProperty;

    /** Add a property value formated as color to the description string.
    */
    SVX_DLLPRIVATE void AddColor (const ::rtl::OUString& sPropertyName,
        const ::rtl::OUString& sLocalizedName);

    /** Add a property value of unknown type to the description string.
    */
    SVX_DLLPRIVATE void AddUnknown (const ::rtl::OUString& sPropertyName,
        const ::rtl::OUString& sLocalizedName);

    /** Add a property value formated as integer to the description string.
    */
    SVX_DLLPRIVATE void AddInteger (const ::rtl::OUString& sPropertyName,
        const ::rtl::OUString& sLocalizedName);

    /** Add a property value formated as string to the description string.
        @param sPropertyName
            Name of the property.
    */
    SVX_DLLPRIVATE void AddString (const ::rtl::OUString& sPropertyName,
        const ::rtl::OUString& sLocalizedName, long nWhichId = -1);

    /** Add a property value formated as fill style to the description
        string.  If the fill style is <const>HATCH</const>,
        <const>GRADIENT</const>, or <const>BITMAP</const>, then the of the
        hatch, gradient, or bitmap is appended as well.
        @param sPropertyName
            Name of the property.  Usually this will be "FillStyle".
    */
    SVX_DLLPRIVATE void AddFillStyle (const ::rtl::OUString& sPropertyName,
        const ::rtl::OUString& sLocalizedName);
};


} // end of namespace accessibility


#endif

