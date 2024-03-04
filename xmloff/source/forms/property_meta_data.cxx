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

#include "property_description.hxx"
#include "property_meta_data.hxx"
#include <forms/form_handler_factory.hxx>
#include "strings.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>

#include <o3tl/hash_combine.hxx>
#include <tools/debug.hxx>

#include <unordered_map>

namespace xmloff::metadata
{

    using namespace ::xmloff::token;

    //= property meta data
    namespace
    {
        const PropertyDescription* lcl_getPropertyMetaData()
        {
            static const PropertyDescription s_propertyMetaData[] =
            {
                PropertyDescription( PROPERTY_DATE_MIN, XML_NAMESPACE_FORM, XML_MIN_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_DATE_MIN ),
                PropertyDescription( PROPERTY_DATE_MAX, XML_NAMESPACE_FORM, XML_MAX_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_DATE_MAX ),
                PropertyDescription( PROPERTY_DEFAULT_DATE, XML_NAMESPACE_FORM, XML_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_DEFAULT_DATE ),
                PropertyDescription( PROPERTY_DATE, XML_NAMESPACE_FORM, XML_CURRENT_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_DATE ),
                PropertyDescription( PROPERTY_TIME_MIN, XML_NAMESPACE_FORM, XML_MIN_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_TIME_MIN ),
                PropertyDescription( PROPERTY_TIME_MAX, XML_NAMESPACE_FORM, XML_MAX_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_TIME_MAX ),
                PropertyDescription( PROPERTY_DEFAULT_TIME, XML_NAMESPACE_FORM, XML_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_DEFAULT_TIME ),
                PropertyDescription( PROPERTY_TIME, XML_NAMESPACE_FORM, XML_CURRENT_VALUE, &FormHandlerFactory::getFormPropertyHandler, PID_TIME ),

                PropertyDescription()
            };
            return s_propertyMetaData;
        }
    }

    namespace
    {
        // TODO: instead of having all of the below static, it should be some per-instance data. This way, the
        // approach used here would scale much better.
        // That is, if you have multiple "meta data instances", which manage a small, but closed set of properties,
        // then looking through those multiple instances would probably be faster than searching within
        // one big instance, since in this case, every instance can quickly decide whether it is responsible
        // for some attribute or property, and otherwise delegate to the next instance.

        typedef std::unordered_map< OUString, const PropertyDescription* > DescriptionsByName;

        const DescriptionsByName& lcl_getPropertyDescriptions()
        {
            DBG_TESTSOLARMUTEX();
            static DescriptionsByName s_propertyDescriptionsByName;
            if ( s_propertyDescriptionsByName.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( !desc->propertyName.isEmpty() )
                {
                    s_propertyDescriptionsByName[ desc->propertyName ] = desc;
                    ++desc;
                }
            }
            return s_propertyDescriptionsByName;
        }

        typedef std::unordered_map< OUString, XMLTokenEnum > ReverseTokenLookup;

        struct AttributeHash
        {
            size_t operator()( const AttributeDescription& i_attribute ) const
            {
                std::size_t seed = 0;
                o3tl::hash_combine(seed, i_attribute.attributeToken);
                o3tl::hash_combine(seed, i_attribute.namespacePrefix);
                return seed;
            }
        };

        typedef std::unordered_map< AttributeDescription, PropertyGroups, AttributeHash > AttributesWithoutGroup;

        const AttributesWithoutGroup& lcl_getAttributesWithoutGroups()
        {
            DBG_TESTSOLARMUTEX();
            static AttributesWithoutGroup s_attributesWithoutGroup;
            if ( s_attributesWithoutGroup.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( !desc->propertyName.isEmpty() )
                {
                    PropertyDescriptionList singleElementList;
                    singleElementList.push_back( desc );

                    s_attributesWithoutGroup[ desc->attribute ].push_back( singleElementList );
                    ++desc;
                }
            }
            return s_attributesWithoutGroup;
        }
    }

    const PropertyDescription* getPropertyDescription( const OUString& i_propertyName )
    {
        const DescriptionsByName& rAllDescriptions( lcl_getPropertyDescriptions() );
        DescriptionsByName::const_iterator pos = rAllDescriptions.find( i_propertyName );
        if ( pos != rAllDescriptions.end() )
            return pos->second;
        return nullptr;
    }

    void getPropertyGroupList( const AttributeDescription& i_attribute, PropertyGroups& o_propertyGroups )
    {
        // the attribute is not used for any non-trivial group, which means it is mapped directly to
        // a single property
        const AttributesWithoutGroup& attributesWithoutGroups( lcl_getAttributesWithoutGroups() );
        const AttributesWithoutGroup::const_iterator pos = attributesWithoutGroups.find( i_attribute );
        if ( pos != attributesWithoutGroups.end() )
            o_propertyGroups = pos->second;
    }

    AttributeDescription getAttributeDescription( sal_Int32 nAttributeToken )
    {
        AttributeDescription attribute;
        attribute.namespacePrefix = (nAttributeToken >> NMSP_SHIFT) - 1;
        attribute.attributeToken = static_cast<XMLTokenEnum>(nAttributeToken & TOKEN_MASK);
        return attribute;
    }

} // namespace xmloff::metadata

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
