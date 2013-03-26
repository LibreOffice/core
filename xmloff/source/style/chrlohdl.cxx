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

#include <chrlohdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/Locale.hpp>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementPropHdl
//

XMLCharLanguageHdl::~XMLCharLanguageHdl()
{
    // nothing to do
}

bool XMLCharLanguageHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Language == aLocale2.Language );

    return bRet;
}

sal_Bool XMLCharLanguageHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;

    rValue >>= aLocale;

    if( !IsXMLToken(rStrImpValue, XML_NONE) )
        aLocale.Language = rStrImpValue;

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharLanguageHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    rStrExpValue = aLocale.Language;

    if( rStrExpValue.isEmpty() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementHeightPropHdl
//

XMLCharCountryHdl::~XMLCharCountryHdl()
{
    // nothing to do
}

bool XMLCharCountryHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Country == aLocale2.Country );

    return bRet;
}

sal_Bool XMLCharCountryHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;

    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
        aLocale.Country = rStrImpValue;

    rValue <<= aLocale;
    return sal_True;
}

sal_Bool XMLCharCountryHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    rStrExpValue = aLocale.Country;

    if( rStrExpValue.isEmpty() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
