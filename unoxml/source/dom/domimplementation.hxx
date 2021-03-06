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

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XDocumentType.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>

#include <cppuhelper/implbase.hxx>

namespace DOM
{
    class CDOMImplementation
        : public cppu::WeakImplHelper< css::xml::dom::XDOMImplementation >
    {

    public:
        static CDOMImplementation* get();

        // there is just 1 static instance, so these must not delete it!
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        /**
        Creates a DOM Document object of the specified type with its document element.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL createDocument(const OUString& namespaceURI, const OUString& qualifiedName, const css::uno::Reference< css::xml::dom::XDocumentType >& doctype) override;

        /**
        Creates an empty DocumentType node.
        */
        virtual css::uno::Reference< css::xml::dom::XDocumentType > SAL_CALL createDocumentType(const OUString& qualifiedName, const OUString& publicId, const OUString& systemId) override;

        /**
        Test if the DOM implementation implements a specific feature.
        */
        virtual sal_Bool SAL_CALL hasFeature(const OUString& feature, const OUString& ver) override;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
