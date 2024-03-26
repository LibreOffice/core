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

#include <comphelper/comphelperdllapi.h>
#include <comphelper/propertycontainerhelper.hxx>
#include <comphelper/propshlp.hxx>

namespace com::sun::star::uno
{
class Any;
}
namespace com::sun::star::uno
{
class Type;
}

namespace comphelper
{
/**
    This class is a copy of comphelper::OPropertyContainer except that it extends comphelper::OPropertySetHelper
    instead of cppu::OPropertySetHelper.

    An OPropertySetHelper implementation which is just a simple container for properties represented
    by class members, usually in a derived class.
    <BR>
    A restriction of this class is that no value conversions are made on a setPropertyValue call. Though
    the base class supports this with the convertFastPropertyValue method, the OPropertyContainer accepts only
    values which already have the correct type, it's unable to convert, for instance, a long to a short.
*/
class COMPHELPER_DLLPUBLIC OPropertyContainer2 : public comphelper::OPropertySetHelper,
                                                 public OPropertyContainerHelper
{
public:
    // this dtor is needed otherwise we can get a wrong delete operator
    virtual ~OPropertyContainer2();

protected:
    OPropertyContainer2();

    /// for scripting : the types of the interfaces supported by this class
    ///
    /// @throws css::uno::RuntimeException
    static css::uno::Sequence<css::uno::Type> getBaseTypes();

    // OPropertySetHelper overridables
    virtual bool convertFastPropertyValue(std::unique_lock<std::mutex>& rGuard,
                                          css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue) override;

    virtual void setFastPropertyValue_NoBroadcast(std::unique_lock<std::mutex>& rGuard,
                                                  sal_Int32 nHandle,
                                                  const css::uno::Any& rValue) override;

    using OPropertySetHelper::getFastPropertyValue;
    virtual void getFastPropertyValue(std::unique_lock<std::mutex>& rGuard, css::uno::Any& rValue,
                                      sal_Int32 nHandle) const override;

    // disambiguate a base class method (XFastPropertySet)
    using OPropertySetHelper::setFastPropertyValue;
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
