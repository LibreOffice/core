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

#ifndef INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTDOCUMENTHANDLER_HXX
#define INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTDOCUMENTHANDLER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <dmapper/resourcemodel.hxx>
#include <ooxml/OOXMLDocument.hxx>
#include <rtl/ref.hxx>
#include "OOXMLParserState.hxx"

namespace writerfilter {
namespace ooxml
{

class OOXMLFastContextHandler;

class OOXMLFastDocumentHandler : public cppu::WeakImplHelper<css::xml::sax::XFastDocumentHandler>
{
public:
    OOXMLFastDocumentHandler(
        css::uno::Reference< css::uno::XComponentContext > const & context,
        Stream* pStream,
        OOXMLDocumentImpl* pDocument,
        sal_Int32 nXNoteId );
    virtual ~OOXMLFastDocumentHandler();

    // css::xml::sax::XFastDocumentHandler:
    virtual void SAL_CALL startDocument()
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endDocument()
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator
    (const css::uno::Reference< css::xml::sax::XLocator > & xLocator)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    // css::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL startUnknownElement
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endFastElement(::sal_Int32 Element)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL endUnknownElement
    (const OUString & Namespace,
     const OUString & Name)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createFastChildContext
    (::sal_Int32 Element,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL
    createUnknownChildContext
    (const OUString & Namespace,
     const OUString & Name,
     const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;
    virtual void SAL_CALL characters(const OUString & aChars)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

    void setIsSubstream( bool bSubstream );

private:
    OOXMLFastDocumentHandler(OOXMLFastDocumentHandler &) = delete;
    void operator =(OOXMLFastDocumentHandler &) = delete;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    Stream * mpStream;
    OOXMLDocumentImpl* mpDocument;
    sal_Int32 mnXNoteId;
    mutable rtl::Reference<OOXMLFastContextHandler> mxContextHandler;
    rtl::Reference<OOXMLFastContextHandler> const & getContextHandler() const;
};
}}

#endif // INCLUDED_WRITERFILTER_SOURCE_OOXML_OOXMLFASTDOCUMENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
