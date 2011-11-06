/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef XMLOFF_PROPERTY_HANDLER_HXX
#define XMLOFF_PROPERTY_HANDLER_HXX

#include "forms/property_ids.hxx"

#include <com/sun/star/uno/Any.hxx>

#include <rtl/ref.hxx>

#include <map>
#include <vector>

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    typedef ::std::map< PropertyId, ::com::sun::star::uno::Any >    PropertyValues;

    //==================================================================================================================
    //= IPropertyHandler
    //==================================================================================================================
    class IPropertyHandler : public ::rtl::IReference
    {
    public:
        /** retrieves the XML attribute value for the given property values
        */
        virtual ::rtl::OUString
            getAttributeValue( const PropertyValues& i_propertyValues ) const = 0;

        /** is a convenience method for XML attributes whose value comprises of only one UNO API property
        */
        virtual ::rtl::OUString
            getAttributeValue( const ::com::sun::star::uno::Any& i_propertyValue ) const = 0;

        /** retrieves the values of the properties controlled by an XML attributed, described by a given attribute value
        */
        virtual bool
            getPropertyValues( const ::rtl::OUString i_attributeValue, PropertyValues& o_propertyValues ) const = 0;

        virtual ~IPropertyHandler() { }
    };

    //==================================================================================================================
    //= PPropertyHandler
    //==================================================================================================================
    typedef ::rtl::Reference< IPropertyHandler >    PPropertyHandler;

    //==================================================================================================================
    //= PropertyHandlerFactory
    //==================================================================================================================
    typedef PPropertyHandler (*PropertyHandlerFactory)( const PropertyId i_propertyId );

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................

#endif // XMLOFF_PROPERTY_HANDLER_HXX
