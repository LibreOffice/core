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

#ifndef INCLUDED_UNOXML_SOURCE_DOM_SAXBUILDER_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_SAXBUILDER_HXX

#include <stack>
#include <map>

#include <sal/types.h>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder2.hpp>
#include <com/sun/star/xml/dom/SAXDocumentBuilderState.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentFragment.hpp>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace DOM
{

    typedef std::stack< css::uno::Reference< css::xml::dom::XNode > > NodeStack;
    typedef std::map< OUString, OUString > NSMap;
    typedef std::map< OUString, OUString > AttrMap;
    typedef std::stack< NSMap > NSStack;

    class  CSAXDocumentBuilder
        : public ::cppu::WeakImplHelper< css::xml::dom::XSAXDocumentBuilder2, css::lang::XServiceInfo >
    {

    private:
        ::osl::Mutex m_Mutex;
        const css::uno::Reference< css::lang::XMultiServiceFactory > m_aServiceManager;

        css::xml::dom::SAXDocumentBuilderState m_aState;
        NodeStack m_aNodeStack;
        NSStack m_aNSStack;

        css::uno::Reference< css::xml::dom::XDocument > m_aDocument;
        css::uno::Reference< css::xml::dom::XDocumentFragment > m_aFragment;
        css::uno::Reference< css::xml::sax::XLocator > m_aLocator;


    public:
        // static helpers for service info and component management
        static const char* aImplementationName;
        static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static css::uno::Sequence< OUString > _getSupportedServiceNames();
        static css::uno::Reference< XInterface > _getInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& rSMgr);

        explicit CSAXDocumentBuilder(const css::uno::Reference< css::lang::XMultiServiceFactory >& mgr);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames () override;

        // XDocumentHandler
        virtual void SAL_CALL startDocument() override;
        virtual void SAL_CALL endDocument() override;
        virtual void SAL_CALL startElement( const OUString& aName,
             const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;
        virtual void SAL_CALL endElement( const OUString& aName ) override;
        virtual void SAL_CALL characters( const OUString& aChars ) override;
        virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
        virtual void SAL_CALL processingInstruction( const OUString& aTarget,
             const OUString& aData ) override;
        virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;


        // XSAXDocumentBuilder
        virtual css::xml::dom::SAXDocumentBuilderState SAL_CALL getState() override;
        virtual void SAL_CALL reset() override;
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL getDocument() override;
        virtual css::uno::Reference< css::xml::dom::XDocumentFragment > SAL_CALL getDocumentFragment() override;
        virtual void SAL_CALL startDocumentFragment(const css::uno::Reference< css::xml::dom::XDocument >& ownerDoc) override;
        virtual void SAL_CALL endDocumentFragment() override;


    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
