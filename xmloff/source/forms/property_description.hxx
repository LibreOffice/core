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

#ifndef XMLOFF_PROPERTY_DESCRIPTION_HXX
#define XMLOFF_PROPERTY_DESCRIPTION_HXX

#include "forms/property_handler.hxx"
#include "property_group.hxx"

#include "xmloff/xmltoken.hxx"

#include <vector>
#include <list>

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    //==================================================================================================================
    //= PropertyDescription
    //==================================================================================================================
    struct AttributeDescription
    {
        sal_uInt16                      namespacePrefix;    // usually XML_NAMESPACE_FORM
        ::xmloff::token::XMLTokenEnum   attributeToken;

        AttributeDescription()
            :namespacePrefix( 0 )
            ,attributeToken( ::xmloff::token::XML_TOKEN_INVALID )
        {
        }

        AttributeDescription(
                const sal_uInt16                    i_namespacePrefix,
                const ::xmloff::token::XMLTokenEnum i_attributeToken
            )
            :namespacePrefix( i_namespacePrefix )
            ,attributeToken( i_attributeToken )
        {
        }
    };

    //..................................................................................................................
    inline bool operator==( const AttributeDescription& i_lhs, const AttributeDescription& i_rhs )
    {
        return  ( i_lhs.namespacePrefix == i_rhs.namespacePrefix )
            &&  ( i_lhs.attributeToken == i_rhs.attributeToken );
    }

    //==================================================================================================================
    //= PropertyDescription
    //==================================================================================================================
    struct PropertyDescription
    {
        /// is the name of the property
        const ::rtl::OUString               propertyName;
        /** denotes the attribute which represents the property. Note that multiple properties might comprise a single
            attribute value.
        */
        const AttributeDescription          attribute;
        /// is the factory for creating a handler for reading and writing the property
        const PropertyHandlerFactory        factory;
        /// the unique ID of the property. The property meta data table must not contain two entries with the same property ID
        const PropertyId                    propertyId;
        /** the group which the property belongs to. Multiple properties belonging to the same group will, all together,
            define the attribute value to be written into the ODF file.

            Consequently, properties which have the same |propertyGroup| value must also have the same |attribute|
            and the same |factory| value, with the only exception being NO_GROUP properties.

            Note that the other direction is not given: It is perfectly legitimate to map the same attribute to different
            (disjunct) property groups.
        */
        const PropertyGroup                 propertyGroup;

        PropertyDescription()
            :propertyName()
            ,attribute()
            ,factory( NULL )
            ,propertyId( PID_INVALID )
            ,propertyGroup( NO_GROUP )
        {
        }

        PropertyDescription(
            const ::rtl::OUString&              i_propertyName,
            const sal_uInt16                    i_namespacePrefix,
            const ::xmloff::token::XMLTokenEnum i_attributeToken,
            const PropertyHandlerFactory        i_factory,
            const PropertyId                    i_propertyId,
            const PropertyGroup                 i_propertyGroup
        )
            :propertyName( i_propertyName )
            ,attribute( i_namespacePrefix, i_attributeToken )
            ,factory( i_factory )
            ,propertyId( i_propertyId )
            ,propertyGroup( i_propertyGroup )
        {
        }
    };

    //==================================================================================================================
    //= PropertyDescriptionList
    //==================================================================================================================
    typedef ::std::vector< const PropertyDescription* > PropertyDescriptionList;

    //==================================================================================================================
    //= PropertyGroups
    //==================================================================================================================
    typedef ::std::list< PropertyDescriptionList >  PropertyGroups;

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................

#endif // XMLOFF_PROPERTY_DESCRIPTION_HXX
