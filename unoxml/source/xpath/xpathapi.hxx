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

#pragma once

#include <map>
#include <vector>

#include <sal/types.h>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/xpath/XXPathObject.hpp>
#include <com/sun/star/xml/xpath/XXPathExtension.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace XPath
{
    typedef std::map<OUString, OUString> nsmap_t;
    typedef std::vector< css::uno::Reference<css::xml::xpath::XXPathExtension> > extensions_t;

    typedef ::cppu::WeakImplHelper
        <   css::xml::xpath::XXPathAPI
        ,   css::lang::XServiceInfo
        > CXPathAPI_Base;

    class  CXPathAPI
        : public CXPathAPI_Base
    {

    private:
        ::osl::Mutex m_Mutex;
        nsmap_t m_nsmap;
        const css::uno::Reference< css::uno::XComponentContext > m_xContext;
        extensions_t m_extensions;

    public:
        // ctor
        explicit CXPathAPI( const css::uno::Reference< css::uno::XComponentContext >& );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames () override;


        // --- XXPathAPI ---

        virtual void SAL_CALL registerNS(const OUString& aPrefix, const OUString& aURI) override;

        virtual void SAL_CALL unregisterNS(const OUString& aPrefix, const OUString& aURI) override;

        /**
        Use an XPath string to select a nodelist.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL selectNodeList(const css::uno::Reference< css::xml::dom::XNode >& contextNode, const OUString& str) override;

        /**
        Use an XPath string to select a nodelist.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL selectNodeListNS(const css::uno::Reference< css::xml::dom::XNode >& contextNode, const OUString& str, const css::uno::Reference< css::xml::dom::XNode >&  namespaceNode) override;

        /**
        Use an XPath string to select a single node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL selectSingleNode(const css::uno::Reference< css::xml::dom::XNode >& contextNode, const OUString& str) override;

        /**
        Use an XPath string to select a single node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL selectSingleNodeNS(const css::uno::Reference< css::xml::dom::XNode >& contextNode, const OUString& str, const css::uno::Reference< css::xml::dom::XNode >&  namespaceNode) override;

        virtual css::uno::Reference< css::xml::xpath::XXPathObject > SAL_CALL eval(const css::uno::Reference< css::xml::dom::XNode >& contextNode, const OUString& str) override;

        virtual css::uno::Reference< css::xml::xpath::XXPathObject > SAL_CALL evalNS(const css::uno::Reference< css::xml::dom::XNode >& contextNode, const OUString& str, const css::uno::Reference< css::xml::dom::XNode >&  namespaceNode) override;

        virtual void SAL_CALL registerExtension(const OUString& aName) override;
        virtual void SAL_CALL registerExtensionInstance(const css::uno::Reference< css::xml::xpath::XXPathExtension>& aExtension) override;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
