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


#include "XMLLineNumberingSeparatorImportContext.hxx"

#include <sax/tools/converter.hxx>
#include "XMLLineNumberingImportContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/token/tokens.hxx>
#include <com/sun/star/xml/sax/FastToken.hpp>


using namespace ::com::sun::star::uno;
using namespace com::sun::star::xml::sax;

using ::com::sun::star::xml::sax::XAttributeList;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_INCREMENT;
using xmloff::XML_increment;

TYPEINIT1( XMLLineNumberingSeparatorImportContext, SvXMLImportContext );

XMLLineNumberingSeparatorImportContext::XMLLineNumberingSeparatorImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    XMLLineNumberingImportContext& rLineNumbering) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        rLineNumberingContext(rLineNumbering)
{
}

XMLLineNumberingSeparatorImportContext::XMLLineNumberingSeparatorImportContext(
    SvXMLImport& rImport, sal_Int32 /*Element*/,
    XMLLineNumberingImportContext& rLineNumbering )
:   SvXMLImportContext( rImport ),
    rLineNumberingContext(rLineNumbering)
{
}

XMLLineNumberingSeparatorImportContext::~XMLLineNumberingSeparatorImportContext()
{
}

void XMLLineNumberingSeparatorImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 i=0; i<nLength; i++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(i), &sLocalName );

        if ( (nPrefix == XML_NAMESPACE_TEXT) &&
             IsXMLToken(sLocalName, XML_INCREMENT) )
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(
                nTmp, xAttrList->getValueByIndex(i), 0))
            {
                rLineNumberingContext.SetSeparatorIncrement((sal_Int16)nTmp);
            }
            // else: invalid number -> ignore
        }
        // else: unknown attribute -> ignore
    }
}

void SAL_CALL XMLLineNumberingSeparatorImportContext::startFastElement(
    sal_Int32 Element, const Reference< XFastAttributeList >& xAttrList )
    throw(RuntimeException, SAXException, std::exception)
{
    if( Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_increment)
        && xAttrList.is() && xAttrList->hasAttribute( Element ) )
    {
        sal_Int32 nTmp;
        if( sax::Converter::convertNumber( nTmp,
            xAttrList->getValue( Element ), 0 ) )
        {
            rLineNumberingContext.SetSeparatorIncrement((sal_Int16)nTmp);
        }
        // else: invalid number -> ignore
    }
    // else: unknown attribute -> ignore
}

void XMLLineNumberingSeparatorImportContext::Characters(
    const OUString& rChars )
{
    sSeparatorBuf.append(rChars);
}

void SAL_CALL XMLLineNumberingSeparatorImportContext::characters(
    const OUString& rChars )
    throw( RuntimeException, SAXException, std::exception )
{
    sSeparatorBuf.append(rChars);
}

void XMLLineNumberingSeparatorImportContext::EndElement()
{
    rLineNumberingContext.SetSeparatorText(sSeparatorBuf.makeStringAndClear());
}

void SAL_CALL XMLLineNumberingSeparatorImportContext::endFastElement(
    sal_Int32 /*Element*/ )
    throw( RuntimeException, SAXException, std::exception)
{
    rLineNumberingContext.SetSeparatorText(sSeparatorBuf.makeStringAndClear());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
