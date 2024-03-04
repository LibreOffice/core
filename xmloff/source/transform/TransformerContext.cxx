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

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <utility>
#include <osl/diagnose.h>

#include "TransformerBase.hxx"

#include "TransformerContext.hxx"


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

bool XMLTransformerContext::HasQName( sal_uInt16 nPrefix,
                       ::xmloff::token::XMLTokenEnum eToken ) const
{
    OUString aLocalName;
    return GetTransformer().GetNamespaceMap().GetKeyByAttrName( m_aQName,
                                              &aLocalName ) == nPrefix &&
           ::xmloff::token::IsXMLToken( aLocalName, eToken );
}

bool XMLTransformerContext::HasNamespace( sal_uInt16 nPrefix ) const
{
    return GetTransformer().GetNamespaceMap().GetKeyByAttrName( m_aQName ) == nPrefix;
}

XMLTransformerContext::XMLTransformerContext( XMLTransformerBase& rImp,
                                                OUString aQName )
    : m_rTransformer(rImp)
    , m_aQName(std::move(aQName))
{
}

rtl::Reference<XMLTransformerContext> XMLTransformerContext::CreateChildContext( sal_uInt16 nPrefix,
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
    GetTransformer().GetDocHandler()->endElement( m_aQName );
}

void XMLTransformerContext::Characters( const OUString& rChars )
{
    m_rTransformer.GetDocHandler()->characters( rChars );
}

bool XMLTransformerContext::IsPersistent() const
{
    return false;
}

void XMLTransformerContext::Export()
{
    OSL_ENSURE( false, "context is not persistent" );
}

void XMLTransformerContext::ExportContent()
{
    OSL_ENSURE( false, "context is not persistent" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
