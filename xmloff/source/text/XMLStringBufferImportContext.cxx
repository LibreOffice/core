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

#include <XMLStringBufferImportContext.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_P;


XMLStringBufferImportContext::XMLStringBufferImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& sLocalName,
    OUStringBuffer& rBuffer) :
    SvXMLImportContext(rImport, nPrefix, sLocalName),
    rTextBuffer(rBuffer)
{
}

XMLStringBufferImportContext::XMLStringBufferImportContext(
    SvXMLImport& rImport,
    OUStringBuffer& rBuffer) :
    SvXMLImportContext(rImport),
    rTextBuffer(rBuffer)
{
}

XMLStringBufferImportContext::~XMLStringBufferImportContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLStringBufferImportContext::createFastChildContext(
        sal_Int32 /*nElement*/, const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new XMLStringBufferImportContext(GetImport(), rTextBuffer);
}

SvXMLImportContextRef XMLStringBufferImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> &)
{
    return new XMLStringBufferImportContext(GetImport(), nPrefix,
                                            rLocalName, rTextBuffer);
}

void XMLStringBufferImportContext::characters(const OUString& rChars )
{
    rTextBuffer.append(rChars);
}

void XMLStringBufferImportContext::Characters(
    const OUString& rChars )
{
    rTextBuffer.append(rChars);
}

void XMLStringBufferImportContext::endFastElement(sal_Int32 nElement)
{
    // add return for paragraph elements
    if ( nElement == XML_ELEMENT(TEXT, XML_P) || nElement == XML_ELEMENT(LO_EXT, XML_P))
    {
        rTextBuffer.append(u'\x000a');
    }
}

void XMLStringBufferImportContext::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
}

void XMLStringBufferImportContext::EndElement()
{
    // add return for paragraph elements
    if ( (XML_NAMESPACE_TEXT == GetPrefix() ||
                XML_NAMESPACE_LO_EXT == GetPrefix()) &&
         (IsXMLToken(GetLocalName(), XML_P))    )
    {
        rTextBuffer.append(u'\x000a');
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
