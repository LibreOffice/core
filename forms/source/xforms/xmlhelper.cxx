/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xmlhelper.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_forms.hxx"

#include "xmlhelper.hxx"

#include "unohelper.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>

using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::container::XNameContainer;
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
    if( c >= 'A' && c <= 'Z'
        || c == '_'
        || c >=    'a' && c <=    'z'
        || c >= 0x00C0 && c <= 0x00D6
        || c >= 0x00D8 && c <= 0x00F6
        || c >= 0x00F8 && c <= 0x02FF
        || c >= 0x0370 && c <= 0x037D
        || c >= 0x037F && c <= 0x1FFF
        || c >= 0x200C && c <= 0x200D
        || c >= 0x2070 && c <= 0x218F
        || c >= 0x2C00 && c <= 0x2FEF
        || c >= 0x3001 && c <= 0xD7FF
        || c >= 0xF900 && c <= 0xFDCF
        || c >= 0xFDF0 && c <= 0xFFFD

        // surrogates
        || c >= 0xD800 && c <= 0xDBFF
        || c >= 0xDC00 && c <= 0xDFFF )
    {
        nClass = 15;
    }
    else if( c == '-'
             || c == '.'
             || c >= '0' && c <= '9'
             || c == 0x00B7
             || c >= 0x0300 && c <= 0x036F
             || c >= 0x203F && c <= 0x2040 )
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
    Reference<XDocumentBuilder> xBuilder(
        xforms::createInstance(
            OUSTRING("com.sun.star.xml.dom.DocumentBuilder") ),
        UNO_QUERY_THROW );
    OSL_ENSURE( xBuilder.is(), "no document builder?" );
    return xBuilder;
}

