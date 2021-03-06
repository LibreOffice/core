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

#include <forms/property_handler.hxx>
#include <xmloff/xmltoken.hxx>

#include <vector>

namespace xmloff
{

    //= PropertyDescription
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

    inline bool operator==( const AttributeDescription& i_lhs, const AttributeDescription& i_rhs )
    {
        return  ( i_lhs.namespacePrefix == i_rhs.namespacePrefix )
            &&  ( i_lhs.attributeToken == i_rhs.attributeToken );
    }

    //= PropertyDescription
    struct PropertyDescription
    {
        /// is the name of the property
        const OUString               propertyName;
        /** denotes the attribute which represents the property. Note that multiple properties might comprise a single
            attribute value.
        */
        const AttributeDescription          attribute;
        /// is the factory for creating a handler for reading and writing the property
        const PropertyHandlerFactory        factory;
        /// the unique ID of the property. The property meta data table must not contain two entries with the same property ID
        const PropertyId                    propertyId;

        PropertyDescription()
            :propertyName()
            ,attribute()
            ,factory( nullptr )
            ,propertyId( PID_INVALID )
        {
        }

        PropertyDescription(
            const OUString&              i_propertyName,
            const sal_uInt16                    i_namespacePrefix,
            const ::xmloff::token::XMLTokenEnum i_attributeToken,
            const PropertyHandlerFactory        i_factory,
            const PropertyId                    i_propertyId
        )
            :propertyName( i_propertyName )
            ,attribute( i_namespacePrefix, i_attributeToken )
            ,factory( i_factory )
            ,propertyId( i_propertyId )
        {
        }
    };

    //= PropertyDescriptionList
    typedef ::std::vector< const PropertyDescription* > PropertyDescriptionList;

    //= PropertyGroups
    typedef ::std::vector< PropertyDescriptionList >  PropertyGroups;

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
