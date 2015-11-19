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

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xml/xmlnamespaces.hxx>
#include <rtl/ustring.hxx>
#include <vcl/menu.hxx>
#include <cppuhelper/implbase.hxx>

#include <stack>
#include <framework/fwedllapi.h>

namespace framework
{

class FWE_DLLPUBLIC SaxNamespaceFilter :
                           public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
    public:
        SaxNamespaceFilter( css::uno::Reference< css::xml::sax::XDocumentHandler >& rSax1DocumentHandler );
        virtual ~SaxNamespaceFilter();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endDocument()
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endElement(const OUString& aName)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL characters(const OUString& aChars)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                    const OUString& aData)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setDocumentLocator(
            const css::uno::Reference< css::xml::sax::XLocator > &xLocator)
        throw(  css::xml::sax::SAXException,
                css::uno::RuntimeException, std::exception ) override;

    protected:
        typedef ::std::stack< XMLNamespaces > NamespaceStack;

        OUString getErrorLineString();

        css::uno::Reference< css::xml::sax::XLocator >          m_xLocator;
        css::uno::Reference< css::xml::sax::XDocumentHandler>   xDocumentHandler;
        NamespaceStack                                          m_aNamespaceStack;

        OUString m_aXMLAttributeNamespace;
        OUString m_aXMLAttributeType;

};

}

#endif // INCLUDED_FRAMEWORK_INC_XML_SAXNAMESPACEFILTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
