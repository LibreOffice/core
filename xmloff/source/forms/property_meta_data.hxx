/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef XMLOFF_PROPERTY_META_DATA_HXX
#define XMLOFF_PROPERTY_META_DATA_HXX

#include "property_description.hxx"

//......................................................................................................................
namespace xmloff { namespace metadata
{
//......................................................................................................................

    const PropertyDescription* getPropertyDescription( const ::rtl::OUString& i_propertyName );

    /** retries the descriptions of all properties which add to the given XML attribute

        <p>Effectively, the complete property map is search for all entries which have the given
        namespace/attribute values.</p>
    */
    void getPropertyGroup(
            const PropertyGroup i_propertyGroup,
            PropertyDescriptionList& o_propertyDescriptions
        );

    /** retrieves all known property groups which are mapped to the given attribute
    */
    void getPropertyGroupList(
        const AttributeDescription& i_attribute,
        PropertyGroups& o_propertyGroups
    );

    /** retrieves the attribute descriptor for the attribute given by namespace prefix and attribute name
    */
    AttributeDescription
        getAttributeDescription( const sal_uInt16 i_namespacePrefix, const ::rtl::OUString& i_attributeName );

//......................................................................................................................
} } // namespace xmloff::metadata
//......................................................................................................................

#endif // XMLOFF_PROPERTY_META_DATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
