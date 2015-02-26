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

#include <SwXMLSectionList.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/token/tokens.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <com/sun/star/xml/sax/FastToken.hpp>


using namespace ::com::sun::star;
using namespace css::uno;
using namespace ::xmloff;


// #110680#
SwXMLSectionList::SwXMLSectionList(
    const uno::Reference< uno::XComponentContext >& rContext,
    std::vector<OUString*> &rNewSectionList)
:   SvXMLImport( rContext, "" ),
    rSectionList ( rNewSectionList )
{
}

SwXMLSectionList::~SwXMLSectionList()
    throw()
{
}

SvXMLImportContext *SwXMLSectionList::CreateFastContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( Element == (FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_body) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_p) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_h) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_a) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_span) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_section) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_index_body) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_index_title) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_insertion) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_deletion) )
    {
        pContext = new SvXMLSectionListContext( *this, Element, xAttrList );
    }
    else
        pContext = SvXMLImport::CreateFastContext( Element, xAttrList );
    return pContext;
}

SvXMLSectionListContext::SvXMLSectionListContext(
   SwXMLSectionList& rImport,
   sal_Int32 /*Element*/,
   const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ ) :
   SvXMLImportContext ( rImport ),
   rLocalRef(rImport)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
SvXMLSectionListContext::createFastChildContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
    throw (uno::RuntimeException, xml::sax::SAXException, std::exception)
{
    SvXMLImportContext *pContext = 0;

    if( (Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_section) ||
        Element == (FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_bookmark)) &&
        xAttrList.is() )
    {
        OUString sName;
        if( xAttrList->hasAttribute( FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_name ) )
            sName = xAttrList->getValue( FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_name );
        if ( !sName.isEmpty() )
            rLocalRef.rSectionList.push_back( new OUString(sName) );
    }

    pContext = new SvXMLSectionListContext (rLocalRef, Element, xAttrList);
    return pContext;
}

SvXMLSectionListContext::~SvXMLSectionListContext ( void )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
