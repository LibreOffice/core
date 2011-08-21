/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

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
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "getNativeNumberString: Exception caught!" );
    }
    return ::rtl::OUString();
}


sal_Bool
NativeNumberWrapper::isValidNatNum(
                    const ::com::sun::star::lang::Locale& rLocale,
                    sal_Int16 nNativeNumberMode ) const
{
    try
    {
        if ( xNNS.is() )
            return xNNS->isValidNatNum( rLocale, nNativeNumberMode );
    }
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "isValidNatNum: Exception caught!" );
    }
    return sal_False;
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
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "convertToXmlAttributes: Exception caught!" );
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
    catch ( uno::Exception& e )
    {
        (void)e;
        DBG_ERRORFILE( "convertFromXmlAttributes: Exception caught!" );
    }
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
