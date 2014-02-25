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
#include <comphelper/property.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <uno/data.h>
#include <com/sun/star/uno/genfunc.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <rtl/ustrbuf.hxx>

#include <algorithm>


namespace comphelper
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;


//= OPropertyContainer


OPropertyContainer::OPropertyContainer(::cppu::OBroadcastHelper& _rBHelper)
    :OPropertySetHelper(_rBHelper)
{
}


OPropertyContainer::~OPropertyContainer()
{
}


Sequence< Type > SAL_CALL OPropertyContainer::getTypes() throw (RuntimeException, std::exception)
{
    // just the types from our one and only base class
    ::cppu::OTypeCollection aTypes(
        ::getCppuType( static_cast< Reference< XPropertySet >* >(NULL)),
        ::getCppuType( static_cast< Reference< XFastPropertySet >* >(NULL)),
        ::getCppuType( static_cast< Reference< XMultiPropertySet >* >(NULL))
    );
    return aTypes.getTypes();
}


void SAL_CALL OPropertyContainer::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue ) throw ( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    OPropertySetHelper::setFastPropertyValue( nHandle, rValue );
}


sal_Bool OPropertyContainer::convertFastPropertyValue(
    Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue ) throw (IllegalArgumentException)
{
    return OPropertyContainerHelper::convertFastPropertyValue( _rConvertedValue, _rOldValue, _nHandle, _rValue );
}


void OPropertyContainer::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
    throw (Exception, std::exception)
{
    OPropertyContainerHelper::setFastPropertyValue( _nHandle, _rValue );
}


void OPropertyContainer::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    OPropertyContainerHelper::getFastPropertyValue( _rValue, _nHandle );
}


}   // namespace comphelper



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
