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
#include <sal/log.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>


using namespace ::com::sun::star;

XMLLabelSeparatorContext::XMLLabelSeparatorContext(
    SvXMLImport& rImport, sal_Int32 nElement,
    const XMLPropertyState& rProp,
    ::std::vector< XMLPropertyState > &rProps ) :
        XMLElementPropertyContext(
            rImport, nElement, rProp, rProps ),
            m_aSeparator()
{
}

XMLLabelSeparatorContext::~XMLLabelSeparatorContext()
{}

void XMLLabelSeparatorContext::startFastElement( sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLLabelSeparatorContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if( (nElement & TOKEN_MASK) == xmloff::token::XML_P )
    {
        return new SchXMLParagraphContext( GetImport(), m_aSeparator );
    }
    SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
    return nullptr;
}

void XMLLabelSeparatorContext::endFastElement(sal_Int32 nElement)
{
    if( !m_aSeparator.isEmpty() )
    {
        // aProp is a member of XMLElementPropertyContext
        aProp.maValue <<= m_aSeparator;
        SetInsert( true );
    }

    XMLElementPropertyContext::endFastElement(nElement);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
