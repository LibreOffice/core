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

#include "XMLLabelSeparatorContext.hxx"

#include "SchXMLParagraphContext.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>

TYPEINIT1( XMLLabelSeparatorContext, XMLElementPropertyContext );

using namespace ::com::sun::star;

XMLLabelSeparatorContext::XMLLabelSeparatorContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const OUString& rLocalName,
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
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & /*xAttrList*/ )
{
    SvXMLImportContext* pContext = nullptr;
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
    if( !m_aSeparator.isEmpty() )
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= m_aSeparator;
        SetInsert( true );
    }

    XMLElementPropertyContext::EndElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
