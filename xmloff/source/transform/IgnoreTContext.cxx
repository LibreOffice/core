/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IgnoreTContext.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_xmloff.hxx"
#include "IgnoreTContext.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLIgnoreTransformerContext, XMLTransformerContext );

XMLIgnoreTransformerContext::XMLIgnoreTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_Bool bIgnoreChars,
        sal_Bool bIgnoreElems ) :
    XMLTransformerContext( rImp, rQName ),
    m_bIgnoreCharacters( bIgnoreChars ),
    m_bIgnoreElements( bIgnoreElems ),
    m_bRecursiveUse( sal_False )
{
}

XMLIgnoreTransformerContext::XMLIgnoreTransformerContext(
        XMLTransformerBase& rTransformer,
        const ::rtl::OUString& rQName,
        sal_Bool bAllowCharactersRecursive ) :
    XMLTransformerContext( rTransformer, rQName ),
    m_bIgnoreCharacters( sal_False ),
    m_bIgnoreElements( sal_False ),
    m_bAllowCharactersRecursive( bAllowCharactersRecursive ),
    m_bRecursiveUse( sal_True )
{
}

XMLIgnoreTransformerContext::~XMLIgnoreTransformerContext()
{
}

XMLTransformerContext *XMLIgnoreTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& xAttrList )
{
    XMLTransformerContext *pContext = 0;
    if( m_bIgnoreElements )
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName, sal_True,
                                                    sal_True );
    else if (m_bRecursiveUse)
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                    rQName, m_bAllowCharactersRecursive );
    else
        pContext = XMLTransformerContext::CreateChildContext(
                        nPrefix, rLocalName, rQName, xAttrList );

    return pContext;
}

void XMLIgnoreTransformerContext::StartElement( const Reference< XAttributeList >& )
{
    // ignore
}

void XMLIgnoreTransformerContext::EndElement()
{
    // ignore
}

void XMLIgnoreTransformerContext::Characters( const OUString& rChars )
{
    if( !m_bIgnoreCharacters )
        GetTransformer().GetDocHandler()->characters( rChars );
    else if ( m_bRecursiveUse && m_bAllowCharactersRecursive )
        GetTransformer().GetDocHandler()->characters( rChars );
}


