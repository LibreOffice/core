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


#include <chrlohdl.hxx>


#include "xmluconv.hxx"




#include <com/sun/star/lang/Locale.hpp>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

// this is a copy of defines in svx/inc/escpitem.hxx
#define DFLT_ESC_PROP	 58
#define DFLT_ESC_AUTO_SUPER	101
#define DFLT_ESC_AUTO_SUB  -101

///////////////////////////////////////////////////////////////////////////////
//
// class XMLEscapementPropHdl
//

XMLCharLanguageHdl::~XMLCharLanguageHdl()
{
    // nothing to do
}

sal_Bool XMLCharLanguageHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Language == aLocale2.Language );

    return bRet;
}

sal_Bool XMLCharLanguageHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    lang::Locale aLocale;

    rValue >>= aLocale;

    if( !IsXMLToken(rStrImpValue, XML_NONE) )
        aLocale.Language = rStrImpValue;

    rValue <<= aLocale;
    return sal_True; 
}

sal_Bool XMLCharLanguageHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    rStrExpValue = aLocale.Language;

    if( !rStrExpValue.getLength() )
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

sal_Bool XMLCharCountryHdl::equals( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 ) const
{
    sal_Bool bRet = sal_False;
    lang::Locale aLocale1, aLocale2;

    if( ( r1 >>= aLocale1 ) && ( r2 >>= aLocale2 ) )
        bRet = ( aLocale1.Country == aLocale2.Country );

    return bRet;
}

sal_Bool XMLCharCountryHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    lang::Locale aLocale;

    rValue >>= aLocale;

    if( !IsXMLToken( rStrImpValue, XML_NONE ) )
        aLocale.Country = rStrImpValue;

    rValue <<= aLocale;
    return sal_True; 
}

sal_Bool XMLCharCountryHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{ 
    lang::Locale aLocale;
    if(!(rValue >>= aLocale))
        return sal_False;

    rStrExpValue = aLocale.Country;
    
    if( !rStrExpValue.getLength() )
        rStrExpValue = GetXMLToken( XML_NONE );

    return sal_True;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
