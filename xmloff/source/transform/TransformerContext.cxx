/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TransformerContext.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:16:44 $
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

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif

#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif

#ifndef _XMLOFF_TRANSFOERMERCONTEXT_HXX
#include "TransformerContext.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT0( XMLTransformerContext );

sal_Bool XMLTransformerContext::HasQName( sal_uInt16 nPrefix,
                       ::xmloff::token::XMLTokenEnum eToken ) const
{
    OUString aLocalName;
    return GetTransformer().GetNamespaceMap().GetKeyByAttrName( m_aQName,
                                              &aLocalName ) == nPrefix &&
           ::xmloff::token::IsXMLToken( aLocalName, eToken );
}

sal_Bool XMLTransformerContext::HasNamespace( sal_uInt16 nPrefix ) const
{
    return GetTransformer().GetNamespaceMap().GetKeyByAttrName( m_aQName ) == nPrefix;
}

XMLTransformerContext::XMLTransformerContext( XMLTransformerBase& rImp,
                                                const OUString& rQName ) :
    m_rTransformer( rImp ),
    m_aQName( rQName ),
    m_pRewindMap( 0 )
{
}

XMLTransformerContext::~XMLTransformerContext()
{
}

XMLTransformerContext *XMLTransformerContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLocalName,
                                            const OUString& rQName,
                                            const Reference< XAttributeList >& )
{
    return m_rTransformer.CreateContext( nPrefix, rLocalName, rQName );
}

void XMLTransformerContext::StartElement( const Reference< XAttributeList >& rAttrList )
{
    m_rTransformer.GetDocHandler()->startElement( m_aQName, rAttrList );
}

void XMLTransformerContext::EndElement()
{
    m_rTransformer.GetDocHandler()->endElement( m_aQName );
}

void XMLTransformerContext::Characters( const OUString& rChars )
{
    m_rTransformer.GetDocHandler()->characters( rChars );
}

sal_Bool XMLTransformerContext::IsPersistent() const
{
    return sal_False;
}

void XMLTransformerContext::Export()
{
    OSL_ENSURE( !this, "context is not persistent" );
}

void XMLTransformerContext::ExportContent()
{
    OSL_ENSURE( !this, "context is not persistent" );
}
