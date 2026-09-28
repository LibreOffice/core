/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <mutex>
#include <stack>

#include <sal/types.h>
#include <cppuhelper/implbase.hxx>
#include <cpo/uno/Reference.h>
#include <cpo/uno/Sequence.h>

#include <com/sun/star/xml/dom/XSAXDocumentBuilder2.hpp>
#include <com/sun/star/xml/dom/SAXDocumentBuilderState.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cpo/uno/XComponentContext.hpp>

namespace DOM
{
    class  CSAXDocumentBuilder
        : public ::cppu::WeakImplHelper< css::xml::dom::XSAXDocumentBuilder2, css::lang::XServiceInfo >
    {

    private:
        std::mutex m_Mutex;
        const cpo::uno::Reference< cpo::uno::XComponentContext> m_xContext;

        css::xml::dom::SAXDocumentBuilderState m_aState;
        std::stack< cpo::uno::Reference< css::xml::dom::XNode > > m_aNodeStack;

        cpo::uno::Reference< css::xml::dom::XDocument > m_aDocument;
        cpo::uno::Reference< css::xml::dom::XDocumentFragment > m_aFragment;


    public:
        explicit CSAXDocumentBuilder(const cpo::uno::Reference< cpo::uno::XComponentContext >& );
        static void setElementFastAttributes(const cpo::uno::Reference< css::xml::dom::XElement >& aElement, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& xAttribs);


        // XServiceInfo
        virtual OUString getImplementationName() override;
        virtual bool supportsService(const OUString& ServiceName) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames () override;

        // XFastDocumentHandler
        virtual void startDocument() override;
        virtual void endDocument() override;
        virtual void processingInstruction( const OUString& rTarget, const OUString& rData ) override;
        virtual void setDocumentLocator( const cpo::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;

        // XFastContextHandler
        virtual void startFastElement( sal_Int32 nElement, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
        virtual void startUnknownElement( const OUString& Namespace, const OUString& Name, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
        virtual void endFastElement( sal_Int32 Element ) override;
        virtual void endUnknownElement( const OUString& Namespace, const OUString& Name ) override;
        virtual cpo::uno::Reference< XFastContextHandler > createFastChildContext( sal_Int32 nElement, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
        virtual cpo::uno::Reference< XFastContextHandler > createUnknownChildContext( const OUString& Namespace, const OUString& Name, const cpo::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
        virtual void characters( const OUString& aChars ) override;

        // XSAXDocumentBuilder
        virtual css::xml::dom::SAXDocumentBuilderState getState() override;
        virtual void reset() override;
        virtual cpo::uno::Reference< css::xml::dom::XDocument > getDocument() override;
        virtual cpo::uno::Reference< css::xml::dom::XDocumentFragment > getDocumentFragment() override;
        virtual void startDocumentFragment(const cpo::uno::Reference< css::xml::dom::XDocument >& ownerDoc) override;
        virtual void endDocumentFragment() override;


    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
