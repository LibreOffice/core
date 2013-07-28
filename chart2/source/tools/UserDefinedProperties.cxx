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

#include "UserDefinedProperties.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;

namespace chart
{

void UserDefinedProperties::AddPropertiesToVector(
    ::std::vector< Property > & rOutProperties )
{
    rOutProperties.push_back(
        Property( "ChartUserDefinedAttributes",
                  PROP_XML_USERDEF_CHART,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "TextUserDefinedAttributes",
                  PROP_XML_USERDEF_TEXT,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    rOutProperties.push_back(
        Property( "ParaUserDefinedAttributes",
                  PROP_XML_USERDEF_PARA,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
    // UserDefinedAttributesSupplier
    rOutProperties.push_back(
        Property( "UserDefinedAttributes",
                  PROP_XML_USERDEF,
                  ::getCppuType( reinterpret_cast< const uno::Reference< container::XNameContainer > * >(0)),
                  beans::PropertyAttribute::BOUND
                  | beans::PropertyAttribute::MAYBEVOID ));
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
