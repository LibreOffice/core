/*************************************************************************
 *
 *  $RCSfile: xmlhelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:58:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
                   const Reference<XNameContainer>& xNamespaces )
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
                        const Reference<XNameContainer>& xNamespaces )
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

