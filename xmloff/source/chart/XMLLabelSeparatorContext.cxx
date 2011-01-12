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
#include "XMLLabelSeparatorContext.hxx"

#include "SchXMLParagraphContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>

TYPEINIT1( XMLLabelSeparatorContext, XMLElementPropertyContext );

using namespace ::com::sun::star;

XMLLabelSeparatorContext::XMLLabelSeparatorContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const ::rtl::OUString& rLocalName,
    const XMLPropertyState& rProp,
    ::std::vector< XMLPropertyState > &rProps ) :
        XMLElementPropertyContext(
            rImport, nPrfx, rLocalName, rProp, rProps ),
            m_aSeparator()
{
}

XMLLabelSeparatorContext::~XMLLabelSeparatorContext()
{}

void XMLLabelSeparatorContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& /*xAttrList*/ )
{
}

SvXMLImportContext* XMLLabelSeparatorContext::CreateChildContext(
    sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & /*xAttrList*/ )
{
    SvXMLImportContext* pContext = NULL;
    if( xmloff::token::IsXMLToken( rLocalName, xmloff::token::XML_P ) )
    {
        pContext = new SchXMLParagraphContext( GetImport(),
                            rLocalName, m_aSeparator );
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLLabelSeparatorContext::EndElement()
{
    if( m_aSeparator.getLength() )
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= m_aSeparator;
        SetInsert( sal_True );
    }

    XMLElementPropertyContext::EndElement();
}
