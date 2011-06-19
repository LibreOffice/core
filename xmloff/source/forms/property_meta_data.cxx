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

#include "precompiled_xmloff.hxx"

#include "property_description.hxx"
#include "forms/form_handler_factory.hxx"
#include "strings.hxx"
#include "xmloff/xmltoken.hxx"
#include "xmloff/xmlnmspe.hxx"

#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>

#include <boost/unordered_map.hpp>

//......................................................................................................................
namespace xmloff { namespace metadata
{
//......................................................................................................................

    using namespace ::xmloff::token;

#define FORM_SINGLE_PROPERTY( id, att ) \
    PropertyDescription( PROPERTY_##id, XML_NAMESPACE_FORM, att, &FormHandlerFactory::getFormPropertyHandler, PID_##id, NO_GROUP )

    //==================================================================================================================
    //= property meta data
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        const PropertyDescription* lcl_getPropertyMetaData()
        {
            static const PropertyDescription s_propertyMetaData[] =
            {
                FORM_SINGLE_PROPERTY( DATE_MIN,        XML_MIN_VALUE        ),
                FORM_SINGLE_PROPERTY( DATE_MAX,        XML_MAX_VALUE        ),
                FORM_SINGLE_PROPERTY( DEFAULT_DATE,    XML_VALUE            ),
                FORM_SINGLE_PROPERTY( DATE,            XML_CURRENT_VALUE    ),
                FORM_SINGLE_PROPERTY( TIME_MIN,        XML_MIN_VALUE        ),
                FORM_SINGLE_PROPERTY( TIME_MAX,        XML_MAX_VALUE        ),
                FORM_SINGLE_PROPERTY( DEFAULT_TIME,    XML_VALUE            ),
                FORM_SINGLE_PROPERTY( TIME,            XML_CURRENT_VALUE    ),

                PropertyDescription()
            };
            return s_propertyMetaData;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    namespace
    {
        // TODO: instead of having all of the below static, it should be some per-instance data. This way, the
        // approach used here would scale much better.
        // That is, if you have multiple "meta data instances", which manage a small, but closed set of properties,
        // then looking looking through those multiple instances would probably be faster than searching within
        // one big instance, since in this case, every instance can quickly decide whether it is responsible
        // for some attribute or property, and otherwise delegate to the next instance.

        //..............................................................................................................
        typedef ::boost::unordered_map< ::rtl::OUString, const PropertyDescription*, ::rtl::OUStringHash > DescriptionsByName;

        //..............................................................................................................
        const DescriptionsByName& lcl_getPropertyDescriptions()
        {
            DBG_TESTSOLARMUTEX();
            static DescriptionsByName s_propertyDescriptionsByName;
            if ( s_propertyDescriptionsByName.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( desc->propertyName.getLength() != 0 )
                {
                    s_propertyDescriptionsByName[ desc->propertyName ] = desc;
                    ++desc;
                }
            }
            return s_propertyDescriptionsByName;
        }

        //..............................................................................................................
        typedef ::std::map< PropertyGroup, PropertyDescriptionList > IndexedPropertyGroups;

        //..............................................................................................................
        const IndexedPropertyGroups& lcl_getIndexedPropertyGroups()
        {
            DBG_TESTSOLARMUTEX();
            static IndexedPropertyGroups s_indexedPropertyGroups;
            if ( s_indexedPropertyGroups.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( desc->propertyName.getLength() != 0 )
                {
                    if ( desc->propertyGroup != NO_GROUP )
                        s_indexedPropertyGroups[ desc->propertyGroup ].push_back( desc );
                    ++desc;
                }
            }
            return s_indexedPropertyGroups;
        }

        //..............................................................................................................
        typedef ::boost::unordered_map< ::rtl::OUString, XMLTokenEnum, ::rtl::OUStringHash > ReverseTokenLookup;

        //..............................................................................................................
        const ReverseTokenLookup& getReverseTokenLookup()
        {
            DBG_TESTSOLARMUTEX();
            static ReverseTokenLookup s_reverseTokenLookup;
            if ( s_reverseTokenLookup.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( desc->propertyName.getLength() != 0 )
                {
                    s_reverseTokenLookup[ token::GetXMLToken( desc->attribute.attributeToken ) ] = desc->attribute.attributeToken;
                    ++desc;
                }
            }
            return s_reverseTokenLookup;
        }

        //..............................................................................................................
        struct AttributeHash : public ::std::unary_function< AttributeDescription, size_t >
        {
            size_t operator()( const AttributeDescription& i_attribute ) const
            {
                return size_t( i_attribute.attributeToken * 100 ) + size_t( i_attribute.namespacePrefix );
            }
        };

        //..............................................................................................................
        typedef ::boost::unordered_multimap< AttributeDescription, PropertyGroup, AttributeHash > AttributeGroups;

        //..............................................................................................................
        const AttributeGroups& lcl_getAttributeGroups()
        {
            DBG_TESTSOLARMUTEX();
            static AttributeGroups s_attributeGroups;
            if ( s_attributeGroups.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( desc->propertyName.getLength() != 0 )
                {
                    if ( desc->propertyGroup != NO_GROUP )
                        s_attributeGroups.insert( AttributeGroups::value_type( desc->attribute, desc->propertyGroup ) );
                    ++desc;
                }
            }
            return s_attributeGroups;
        }

        //..............................................................................................................
        typedef ::boost::unordered_map< AttributeDescription, PropertyGroups, AttributeHash > AttributesWithoutGroup;

        //..............................................................................................................
        const AttributesWithoutGroup& lcl_getAttributesWithoutGroups()
        {
            DBG_TESTSOLARMUTEX();
            static AttributesWithoutGroup s_attributesWithoutGroup;
            if ( s_attributesWithoutGroup.empty() )
            {
                const PropertyDescription* desc = lcl_getPropertyMetaData();
                while ( desc->propertyName.getLength() != 0 )
                {
                    if ( desc->propertyGroup == NO_GROUP )
                    {
                        PropertyDescriptionList singleElementList;
                        singleElementList.push_back( desc );

                        s_attributesWithoutGroup[ desc->attribute ].push_back( singleElementList );
                    }
                    ++desc;
                }
            }
            return s_attributesWithoutGroup;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    const PropertyDescription* getPropertyDescription( const ::rtl::OUString& i_propertyName )
    {
        const DescriptionsByName& rAllDescriptions( lcl_getPropertyDescriptions() );
        DescriptionsByName::const_iterator pos = rAllDescriptions.find( i_propertyName );
        if ( pos != rAllDescriptions.end() )
            return pos->second;
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void getPropertyGroup( const PropertyGroup i_propertyGroup, PropertyDescriptionList& o_propertyDescriptions )
    {
        OSL_ENSURE( i_propertyGroup != NO_GROUP, "xmloff::metadata::getPropertyGroup: illegal group!" );

        const IndexedPropertyGroups& rPropertyGroups( lcl_getIndexedPropertyGroups() );
        const IndexedPropertyGroups::const_iterator pos = rPropertyGroups.find( i_propertyGroup );
        if ( pos != rPropertyGroups.end() )
            o_propertyDescriptions = pos->second;
    }

    //------------------------------------------------------------------------------------------------------------------
    void getPropertyGroupList( const AttributeDescription& i_attribute, PropertyGroups& o_propertyGroups )
    {
        const AttributeGroups& rAttributeGroups = lcl_getAttributeGroups();

        ::std::pair< AttributeGroups::const_iterator, AttributeGroups::const_iterator >
            range = rAttributeGroups.equal_range( i_attribute );

        if ( range.first == range.second )
        {
            // the attribute is not used for any non-trivial group, which means it is mapped directly to
            // a single property
            const AttributesWithoutGroup& attributesWithoutGroups( lcl_getAttributesWithoutGroups() );
            const AttributesWithoutGroup::const_iterator pos = attributesWithoutGroups.find( i_attribute );
            if ( pos != attributesWithoutGroups.end() )
                o_propertyGroups = pos->second;
        }
        else
        {
            const IndexedPropertyGroups& rPropertyGroups = lcl_getIndexedPropertyGroups();
            for ( AttributeGroups::const_iterator group = range.first; group != range.second; ++group )
            {
                const PropertyGroup propGroup = group->second;
                const IndexedPropertyGroups::const_iterator groupPos = rPropertyGroups.find( propGroup );
                ENSURE_OR_CONTINUE( groupPos != rPropertyGroups.end(), "getPropertyGroupList: inconsistency!" );
                o_propertyGroups.push_back( groupPos->second );
            }
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    AttributeDescription getAttributeDescription( const sal_uInt16 i_namespacePrefix, const ::rtl::OUString& i_attributeName )
    {
        AttributeDescription attribute;
        const ReverseTokenLookup& rTokenLookup( getReverseTokenLookup() );
        const ReverseTokenLookup::const_iterator pos = rTokenLookup.find( i_attributeName );
        if ( pos != rTokenLookup.end() )
        {
            attribute.namespacePrefix = i_namespacePrefix;
            attribute.attributeToken = pos->second;
        }
        return attribute;
    }

//......................................................................................................................
} } // namespace xmloff::metadata
//......................................................................................................................
