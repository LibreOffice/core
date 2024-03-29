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

#include <comphelper/propertycontainer.hxx>
#include <cppuhelper/typeprovider.hxx>


namespace comphelper
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

OPropertyContainer::OPropertyContainer(::cppu::OBroadcastHelper& _rBHelper)
    :OPropertySetHelper(_rBHelper)
{
}


OPropertyContainer::~OPropertyContainer()
{
}


Sequence< Type > OPropertyContainer::getBaseTypes()
{
    // just the types from our one and only base class
    ::cppu::OTypeCollection aTypes(
        cppu::UnoType<XPropertySet>::get(),
        cppu::UnoType<XFastPropertySet>::get(),
        cppu::UnoType<XMultiPropertySet>::get()
    );
    return aTypes.getTypes();
}

sal_Bool OPropertyContainer::convertFastPropertyValue(
    Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue )
{
    return OPropertyContainerHelper::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
}


void OPropertyContainer::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
{
    OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
}


void OPropertyContainer::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    OPropertyContainerHelper::getFastPropertyValue( _rValue, _nHandle );
}


}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
