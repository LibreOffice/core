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


#include "xmlhelper.hxx"

#include "unohelper.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <comphelper/processfactory.hxx>

using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::container::XNameContainer;
using com::sun::star::xml::dom::DocumentBuilder;
using com::sun::star::xml::dom::XDocumentBuilder;


//
// determine valid XML name
//

// character class:
// 1: NameStartChar
// 2: NameChar
// 4: NCNameStartChar
// 8: NCNameChar
inline sal_uInt8 lcl_getCharClass( sal_Unicode c )
{
    sal_uInt8 nClass = 0;

    // NameStartChar
    if( (c >= 'A' && c <= 'Z')
        || c == '_'
        || (c >=    'a' && c <=    'z')
        || (c >= 0x00C0 && c <= 0x00D6)
        || (c >= 0x00D8 && c <= 0x00F6)
        || (c >= 0x00F8 && c <= 0x02FF)
        || (c >= 0x0370 && c <= 0x037D)
        || (c >= 0x037F && c <= 0x1FFF)
        || (c >= 0x200C && c <= 0x200D)
        || (c >= 0x2070 && c <= 0x218F)
        || (c >= 0x2C00 && c <= 0x2FEF)
        || (c >= 0x3001 && c <= 0xD7FF)
        || (c >= 0xF900 && c <= 0xFDCF)
        || (c >= 0xFDF0 && c <= 0xFFFD)

        // surrogates
        || (c >= 0xD800 && c <= 0xDBFF)
        || (c >= 0xDC00 && c <= 0xDFFF) )
    {
        nClass = 15;
    }
    else if( c == '-'
             || c == '.'
             || (c >= '0' && c <= '9')
             || (c == 0x00B7)
             || (c >= 0x0300 && c <= 0x036F)
             || (c >= 0x203F && c <= 0x2040) )
    {
        nClass = 10;
    }
    else if( c == ':' )
    {
        nClass = 3;
    }

    return nClass;
}

bool isValidQName( const OUString& sName,
                   const Reference<XNameContainer>& /*xNamespaces*/ )
{
    sal_Int32 nLength = sName.getLength();
    const sal_Unicode* pName = sName.getStr();

    bool bRet = false;
    sal_Int32 nColon = 0;
    if( nLength > 0 )
    {
        bRet = ( ( lcl_getCharClass( pName[0] ) & 4 ) != 0 );
        for( sal_Int32 n = 1; n < nLength; n++ )
        {
            sal_uInt8 nClass = lcl_getCharClass( pName[n] );
            bRet &= ( ( nClass & 2 ) != 0 );
            if( nClass == 3 )
                nColon++;
        }
    }
    if( nColon > 1 )
        bRet = sal_False;

    return bRet;
}

bool isValidPrefixName( const OUString& sName,
                        const Reference<XNameContainer>& /*xNamespaces*/ )
{
    sal_Int32 nLength = sName.getLength();
    const sal_Unicode* pName = sName.getStr();
    bool bRet = false;

    if( nLength > 0 )
    {
        bRet = ( ( lcl_getCharClass( pName[0] ) & 4 ) != 0 );
        for( sal_Int32 n = 1; n < nLength; n++ )
            bRet &= ( ( lcl_getCharClass( pName[n] ) & 8 ) != 0 );
    }

    return bRet;
}

Reference<XDocumentBuilder> getDocumentBuilder()
{
    Reference<XDocumentBuilder> xBuilder(DocumentBuilder::create(::comphelper::getProcessComponentContext()));
    return xBuilder;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
