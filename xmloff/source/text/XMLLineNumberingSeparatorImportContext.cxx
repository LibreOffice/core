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
#include <XMLLineNumberingImportContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::com::sun::star::uno;

using ::xmloff::token::XML_INCREMENT;


XMLLineNumberingSeparatorImportContext::XMLLineNumberingSeparatorImportContext(
    SvXMLImport& rImport,
    XMLLineNumberingImportContext& rLineNumbering) :
        SvXMLImportContext(rImport),
        rLineNumberingContext(rLineNumbering)
{
}

XMLLineNumberingSeparatorImportContext::~XMLLineNumberingSeparatorImportContext()
{
}

void XMLLineNumberingSeparatorImportContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if ( aIter.getToken() == XML_ELEMENT(TEXT, XML_INCREMENT) )
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(nTmp, aIter.toView(), 0))
            {
                rLineNumberingContext.SetSeparatorIncrement(static_cast<sal_Int16>(nTmp));
            }
            // else: invalid number -> ignore
        }
        // else: unknown attribute -> ignore
    }
}

void XMLLineNumberingSeparatorImportContext::characters(
    const OUString& rChars )
{
    sSeparatorBuf.append(rChars);
}

void XMLLineNumberingSeparatorImportContext::endFastElement(sal_Int32 )
{
    rLineNumberingContext.SetSeparatorText(sSeparatorBuf.makeStringAndClear());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
