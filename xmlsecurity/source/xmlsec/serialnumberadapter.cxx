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

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/security/XSerialNumberAdapter.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include "xmlsecurity/biginteger.hxx"

#include "serialnumberadapter.hxx"

namespace {

class Service:
    public cppu::WeakImplHelper<
        css::lang::XServiceInfo, css::security::XSerialNumberAdapter >,
    private boost::noncopyable
{
public:
    Service() {}

private:
    virtual ~Service() {}

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return xml_security::serial_number_adapter::implementationName(); }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    { return xml_security::serial_number_adapter::serviceNames(); }

    virtual OUString SAL_CALL toString(
        css::uno::Sequence< sal_Int8 > const & SerialNumber)
        throw (css::uno::RuntimeException, std::exception) override
    { return bigIntegerToNumericString(SerialNumber); }

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL toSequence(
        OUString const & SerialNumber)
        throw (css::uno::RuntimeException, std::exception) override
    { return numericStringToBigInteger(SerialNumber); }
};

}

css::uno::Reference< css::uno::XInterface >
xml_security::serial_number_adapter::create(
    css::uno::Reference< css::uno::XComponentContext > const &)
{
    return static_cast< cppu::OWeakObject * >(new Service);
}

OUString xml_security::serial_number_adapter::implementationName()
    throw (css::uno::RuntimeException)
{
    return OUString("com.sun.star.comp.security.SerialNumberAdapter");
}

css::uno::Sequence< OUString >
xml_security::serial_number_adapter::serviceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > s { "com.sun.star.security.SerialNumberAdapter" };
    return s;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
