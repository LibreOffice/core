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

#ifndef INCLUDED_FRAMEWORK_INC_XML_SAXNAMESPACEFILTER_HXX
#define INCLUDED_FRAMEWORK_INC_XML_SAXNAMESPACEFILTER_HXX

#include <config_options.h>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xml/xmlnamespaces.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <stack>
#include <framework/fwkdllapi.h>

namespace framework
{

// workaround for incremental linking bugs in MSVC2015
class SAL_DLLPUBLIC_TEMPLATE SaxNamespaceFilter_Base : public cppu::WeakImplHelper< css::xml::sax::XDocumentHandler > {};

class SaxNamespaceFilter final : public SaxNamespaceFilter_Base
{
    public:
        SaxNamespaceFilter( css::uno::Reference< css::xml::sax::XDocumentHandler > const & rSax1DocumentHandler );
        virtual ~SaxNamespaceFilter() override;

        // XDocumentHandler
        virtual void SAL_CALL startDocument() override;

        virtual void SAL_CALL endDocument() override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs) override;

        virtual void SAL_CALL endElement(const OUString& aName) override;

        virtual void SAL_CALL characters(const OUString& aChars) override;

        virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) override;

        virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                    const OUString& aData) override;

        virtual void SAL_CALL setDocumentLocator(
            const css::uno::Reference< css::xml::sax::XLocator > &xLocator) override;

    private:
        typedef ::std::stack< XMLNamespaces > NamespaceStack;

        OUString getErrorLineString();

        css::uno::Reference< css::xml::sax::XLocator >          m_xLocator;
        css::uno::Reference< css::xml::sax::XDocumentHandler>   xDocumentHandler;
        NamespaceStack                                          m_aNamespaceStack;
};

}

#endif // INCLUDED_FRAMEWORK_INC_XML_SAXNAMESPACEFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
