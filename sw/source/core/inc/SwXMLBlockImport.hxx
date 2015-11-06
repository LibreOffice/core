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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_SWXMLBLOCKIMPORT_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SWXMLBLOCKIMPORT_HXX

#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>
#include <sax/fastattribs.hxx>
#include <cppuhelper/implbase.hxx>

using namespace css::xml::sax;
using namespace xmloff::token;

class SwXMLTextBlocks;
class SwXMLBlockListImport : public SvXMLImport
{
private:
    SwXMLTextBlocks &rBlockList;

protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext* CreateFastContext( sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;

public:
    SwXMLBlockListImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        SwXMLTextBlocks &rBlocks );

    SwXMLTextBlocks& getBlockList()
    {
        return rBlockList;
    }
    virtual ~SwXMLBlockListImport()
        throw();
};

class SwXMLTextBlockImport : public SvXMLImport
{
protected:
    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext* CreateFastContext( sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;

public:
    bool bTextOnly;
    OUString &m_rText;
    SwXMLTextBlockImport(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        OUString &rNewText, bool bNewTextOnly );

    virtual ~SwXMLTextBlockImport()
        throw();
    virtual void SAL_CALL endDocument()
        throw( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
};

enum SwXMLTextBlockToken : sal_Int32
{
    OFFICE_BODY = FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_BODY,
    OFFICE_TEXT = FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_TEXT,
    OFFICE_DOCUMENT = FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_DOCUMENT,
    OFFICE_DOCUMENT_CONTENT = FastToken::NAMESPACE | XML_NAMESPACE_OFFICE | XML_DOCUMENT_CONTENT,
    TEXT_P = FastToken::NAMESPACE | XML_NAMESPACE_TEXT | XML_P
};

class SwXMLTextBlockTokenHandler : public
        cppu::WeakImplHelper< css::xml::sax::XFastTokenHandler >,
        public sax_fastparser::FastTokenHandlerBase
{
public:
    SwXMLTextBlockTokenHandler();
    virtual ~SwXMLTextBlockTokenHandler();

    //XFastTokenHandler
    sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence< sal_Int8 >& Identifier )
        throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 Token )
        throw (css::uno::RuntimeException, std::exception) override;

    //Much fast direct C++ shortcut to the method that matters
    virtual sal_Int32 getTokenDirect( const char *pTag, sal_Int32 nLength ) const override;
};

enum SwXMLBlockListToken : sal_Int32
{
    ABBREVIATED_NAME = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_ABBREVIATED_NAME,
    BLOCK = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_BLOCK,
    BLOCK_LIST = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_BLOCK_LIST,
    LIST_NAME = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_LIST_NAME,
    NAME = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_NAME,
    PACKAGE_NAME = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_PACKAGE_NAME,
    UNFORMATTED_TEXT = FastToken::NAMESPACE | XML_NAMESPACE_BLOCKLIST | XML_UNFORMATTED_TEXT
};

class SwXMLBlockListTokenHandler : public
        cppu::WeakImplHelper< css::xml::sax::XFastTokenHandler >,
        public sax_fastparser::FastTokenHandlerBase
{
public:
    SwXMLBlockListTokenHandler();
    virtual ~SwXMLBlockListTokenHandler();

    //XFastTokenHandler
    sal_Int32 SAL_CALL getTokenFromUTF8( const css::uno::Sequence< sal_Int8 >& Identifier )
        throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< sal_Int8 > SAL_CALL getUTF8Identifier( sal_Int32 Token )
        throw (css::uno::RuntimeException, std::exception) override;

    //Much fast direct C++ shortcut to the method that matters
    virtual sal_Int32 getTokenDirect( const char *pTag, sal_Int32 nLength ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
