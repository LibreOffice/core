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
