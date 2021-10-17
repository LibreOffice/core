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

#include "SchXMLTextListContext.hxx"
#include "SchXMLParagraphContext.hxx"

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <sal/log.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using namespace com::sun::star;
using namespace ::xmloff::token;

namespace {

class SchXMLListItemContext : public SvXMLImportContext
{
public:
    SchXMLListItemContext( SvXMLImport& rImport, OUString& rText );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

private:
    OUString& m_rText;
};

}

SchXMLListItemContext::SchXMLListItemContext(
        SvXMLImport& rImport
        , OUString& rText )
        : SvXMLImportContext( rImport )
        , m_rText( rText )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLListItemContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;
    if( nElement == XML_ELEMENT(TEXT, XML_P) ||
        nElement == XML_ELEMENT(LO_EXT, XML_P) )
        pContext = new SchXMLParagraphContext( GetImport(), m_rText );
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return pContext;
}

SchXMLTextListContext::SchXMLTextListContext(
        SvXMLImport& rImport
        , Sequence< OUString>& rTextList )
        : SvXMLImportContext( rImport )
        , m_rTextList( rTextList )
{
}

SchXMLTextListContext::~SchXMLTextListContext()
{
}

void SchXMLTextListContext::endFastElement(sal_Int32 )
{
    sal_Int32 nCount = m_aTextVector.size();
    auto pTextList = m_rTextList.realloc(nCount);
    for( sal_Int32 nN=0; nN<nCount; nN++ )
        pTextList[nN]=m_aTextVector[nN];
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SchXMLTextListContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    SvXMLImportContext* pContext = nullptr;
    if( nElement == XML_ELEMENT(TEXT, XML_LIST_ITEM) )
    {
        m_aTextVector.emplace_back( );
        pContext = new SchXMLListItemContext( GetImport(), m_aTextVector.back() );
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
