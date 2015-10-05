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

#ifndef INCLUDED_XMLOFF_INC_FORMS_PROPERTY_HANDLER_HXX
#define INCLUDED_XMLOFF_INC_FORMS_PROPERTY_HANDLER_HXX

#include "forms/property_ids.hxx"

#include <com/sun/star/uno/Any.hxx>

#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>

#include <map>
#include <vector>

namespace xmloff
{

    typedef ::std::map< PropertyId, ::com::sun::star::uno::Any >    PropertyValues;

    class PropertyHandlerBase : public ::salhelper::SimpleReferenceObject
    {
    public:
        /** retrieves the XML attribute value for the given property values
        */
        virtual OUString
            getAttributeValue( const PropertyValues& i_propertyValues ) const = 0;

        /** is a convenience method for XML attributes whose value comprises of only one UNO API property
        */
        virtual OUString
            getAttributeValue( const ::com::sun::star::uno::Any& i_propertyValue ) const = 0;

        /** retrieves the values of the properties controlled by an XML attributed, described by a given attribute value
        */
        virtual bool
            getPropertyValues( const OUString& i_attributeValue, PropertyValues& o_propertyValues ) const = 0;

        virtual ~PropertyHandlerBase() {}
    };

    //= PPropertyHandler
    typedef rtl::Reference< PropertyHandlerBase >    PPropertyHandler;

    //= PropertyHandlerFactory
    typedef PPropertyHandler (*PropertyHandlerFactory)( const PropertyId i_propertyId );

} // namespace xmloff

#endif // INCLUDED_XMLOFF_INC_FORMS_PROPERTY_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
