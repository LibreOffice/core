/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IgnoreTContext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:14:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _XMLOFF_IGNORETCONTEXT_HXX
#include "IgnoreTContext.hxx"
#endif
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


