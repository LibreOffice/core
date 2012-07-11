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


#include <unotools/nativenumberwrapper.hxx>
#include <tools/debug.hxx>
#include "instance.hxx"

using namespace ::com::sun::star;

NativeNumberWrapper::NativeNumberWrapper(
            const uno::Reference< lang::XMultiServiceFactory > & xSF
            )
        :
        xSMgr( xSF )
{
    xNNS = uno::Reference< i18n::XNativeNumberSupplier > (
        intl_createInstance( xSMgr, "com.sun.star.i18n.NativeNumberSupplier",
                             "NativeNumberWrapper"), uno::UNO_QUERY );
    DBG_ASSERT( xNNS.is(), "NativeNumberWrapper: no NativeNumberSupplier" );
}


NativeNumberWrapper::~NativeNumberWrapper()
{
}


::rtl::OUString
NativeNumberWrapper::getNativeNumberString(
                    const ::rtl::OUString& rNumberString,
                    const ::com::sun::star::lang::Locale& rLocale,
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
    return ::rtl::OUString();
}


i18n::NativeNumberXmlAttributes
NativeNumberWrapper::convertToXmlAttributes(
                    const ::com::sun::star::lang::Locale& rLocale,
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
