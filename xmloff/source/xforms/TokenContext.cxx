/*************************************************************************
 *
 *  $RCSfile: TokenContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:13:12 $
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

#ifndef _XMLOFF_TOKENCONTEXT_HXX
#include "TokenContext.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#include <tools/debug.hxx>

using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XAttributeList;


struct SvXMLTokenMapEntry aEmptyMap[1] =
{
    XML_TOKEN_MAP_END
};


TokenContext::TokenContext( SvXMLImport& rImport,
                            USHORT nPrefix,
                            const OUString& rLocalName,
                            const SvXMLTokenMapEntry* pAttributes,
                            const SvXMLTokenMapEntry* pChildren )
    : SvXMLImportContext( rImport, nPrefix, rLocalName ),
      mpAttributes( pAttributes ),
      mpChildren( pChildren )
{
}

TokenContext::~TokenContext()
{
}

void TokenContext::StartElement(
    const Reference<XAttributeList>& xAttributeList )
{
    // iterate over attributes
    // - if in map: call HandleAttribute
    // - xmlns:... : ignore
    // - other: warning
    DBG_ASSERT( mpAttributes != NULL, "no token map for attributes" );
    SvXMLTokenMap aMap( mpAttributes );

    sal_Int16 nCount = xAttributeList->getLength();
    for( sal_Int16 i = 0; i < nCount; i++ )
    {
        // get key/local-name pair from namespace map
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttributeList->getNameByIndex(i), &sLocalName );

        // get token from token map
        sal_uInt16 nToken = aMap.Get( nPrefix, sLocalName );

        // and the value...
        const OUString& rValue = xAttributeList->getValueByIndex(i);

        if( nToken != XML_TOK_UNKNOWN )
        {
            HandleAttribute( nToken, rValue );
        }
        else if( nPrefix != XML_NAMESPACE_XMLNS )
        {
            // error handling, for all attribute that are not
            // namespace declarations
            GetImport().SetError( XMLERROR_UNKNOWN_ATTRIBUTE,
                                  sLocalName, rValue);
        }
    }
}

SvXMLImportContext* TokenContext::CreateChildContext(
    USHORT nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    // call HandleChild for elements in token map. Ignore other content.

    SvXMLImportContext* pContext = NULL;

    DBG_ASSERT( mpChildren != NULL, "no token map for child elements" );
    SvXMLTokenMap aMap( mpChildren );
    sal_uInt16 nToken = aMap.Get( nPrefix, rLocalName );
    if( nToken != XML_TOK_UNKNOWN )
    {
        // call handle child, and pass down arguments
        pContext = HandleChild( nToken, nPrefix, rLocalName, xAttrList );
    }

    // error handling: create default context and generate warning
    if( pContext == NULL )
    {
        GetImport().SetError( XMLERROR_UNKNOWN_ELEMENT, rLocalName );
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
    return pContext;
}

bool lcl_IsWhiteSpace( sal_Unicode c )
{
    return c == sal_Unicode(  ' ' )
        || c == sal_Unicode( 0x09 )
        || c == sal_Unicode( 0x0A )
        || c == sal_Unicode( 0x0D );
}

void TokenContext::Characters( const ::rtl::OUString& rCharacters )
{
    // get iterators for string data
    const sal_Unicode* pBegin = rCharacters.getStr();
    const sal_Unicode* pEnd = &( pBegin[ rCharacters.getLength() ] );

    // raise error if non-whitespace character is found
    if( find_if( pBegin, pEnd, not1(ptr_fun(lcl_IsWhiteSpace)) ) != pEnd )
        GetImport().SetError( XMLERROR_UNKNOWN_CHARACTERS, rCharacters );
}
