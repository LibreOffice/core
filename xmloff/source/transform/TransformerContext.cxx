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
#include "precompiled_xmloff.hxx"
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <xmloff/nmspmap.hxx>

#include "TransformerBase.hxx"

#include "TransformerContext.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
