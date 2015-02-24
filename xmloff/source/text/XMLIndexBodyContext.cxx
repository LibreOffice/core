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

#include "XMLIndexBodyContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <rtl/ustring.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using css::xml::sax::XFastAttributeList;
using css::xml::sax::XFastContextHandler;


TYPEINIT1( XMLIndexBodyContext, SvXMLImportContext);

XMLIndexBodyContext::XMLIndexBodyContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        bHasContent(false)
{
}

XMLIndexBodyContext::XMLIndexBodyContext(
    SvXMLImport& rImport, sal_Int32 /*Element*/ )
:   SvXMLImportContext( rImport ),
    bHasContent(false)
{
}

XMLIndexBodyContext::~XMLIndexBodyContext()
{
}

SvXMLImportContext* XMLIndexBodyContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList)
{
    SvXMLImportContext* pContext = NULL;

    // return text content (if possible)
    pContext = GetImport().GetTextImport()->CreateTextChildContext(
        GetImport(), nPrefix, rLocalName, xAttrList, XML_TEXT_TYPE_SECTION );
    if (NULL == pContext)
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
    else
        bHasContent = true;

    return pContext;
}

Reference< XFastContextHandler > SAL_CALL
    XMLIndexBodyContext::createFastChildContext( sal_Int32 Element,
    const Reference< XFastAttributeList >& xAttrList )
    throw(css::uno::RuntimeException, css::xml::sax::SAXException, std::exception)
{
    Reference< XFastContextHandler > pContext = 0;

    // return text content (if possible)
    pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), Element, xAttrList, XML_TEXT_TYPE_SECTION );

    if( !pContext.is() )
    {
        pContext = new SvXMLImportContext( GetImport() );
    }
    else
        bHasContent = true;

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
