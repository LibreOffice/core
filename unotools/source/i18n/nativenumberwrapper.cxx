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

#include <sal/log.hxx>
#include <unotools/nativenumberwrapper.hxx>
#include <com/sun/star/i18n/NativeNumberSupplier.hpp>

using namespace ::com::sun::star;

NativeNumberWrapper::NativeNumberWrapper(
            const uno::Reference< uno::XComponentContext > & rxContext
            )
{
    xNNS = i18n::NativeNumberSupplier::create(rxContext);
}

NativeNumberWrapper::~NativeNumberWrapper()
{
}

OUString
NativeNumberWrapper::getNativeNumberString(
                    const OUString& rNumberString,
                    const css::lang::Locale& rLocale,
                    sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->getNativeNumberString( rNumberString, rLocale, nNativeNumberMode );
    }
    catch ( const uno::Exception& )
    {
        SAL_WARN( "unotools.i18n", "getNativeNumberString: Exception caught!" );
    }
    return OUString();
}

i18n::NativeNumberXmlAttributes
NativeNumberWrapper::convertToXmlAttributes(
                    const css::lang::Locale& rLocale,
                    sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->convertToXmlAttributes( rLocale, nNativeNumberMode );
    }
    catch ( const uno::Exception& )
    {
        SAL_WARN( "unotools.i18n", "convertToXmlAttributes: Exception caught!" );
    }
    return i18n::NativeNumberXmlAttributes();
}

sal_Int16
NativeNumberWrapper::convertFromXmlAttributes(
                    const i18n::NativeNumberXmlAttributes& rAttr ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->convertFromXmlAttributes( rAttr );
    }
    catch ( const uno::Exception& )
    {
        SAL_WARN( "unotools.i18n", "convertFromXmlAttributes: Exception caught!" );
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
