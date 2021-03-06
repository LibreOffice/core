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

#include <xmloff/xmlictxt.hxx>


// forward declarations


namespace com::sun::star {
    namespace xml::dom {
        class XNode;
        class XDocument;
    }
    namespace xml::sax {
        class XAttributeList;
    }
}
class SvXMLImport;
class SvXMLImportContext;

/**
 * DomBuilderContext creates a DOM tree suitable for in-memory processing of
 * XML data from a sequence of SAX events */
class DomBuilderContext final : public SvXMLImportContext
{
    css::uno::Reference<css::xml::dom::XNode> mxNode;

    void HandleAttributes(const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs);

public:

    /** default constructor: create new DOM tree */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_Int32 nElement );
    DomBuilderContext( SvXMLImport& rImport,
                       const OUString & Namespace, const OUString & Name );

    /** constructor: create DOM subtree under the given node */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_Int32 nElement,
                       css::uno::Reference<css::xml::dom::XNode> const & );
    /** constructor: create DOM subtree under the given node */
    DomBuilderContext( SvXMLImport& rImport,
                       const OUString & Namespace, const OUString & Name,
                       css::uno::Reference<css::xml::dom::XNode> const & );

    virtual ~DomBuilderContext() override;


    // access to the DOM tree


    /** access the DOM tree */
    css::uno::Reference<css::xml::dom::XDocument> getTree();


    // implement SvXMLImportContext methods:

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext(
        const OUString& Namespace, const OUString& Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;

    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
    virtual void SAL_CALL startUnknownElement(const OUString & Namespace, const OUString & Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & Attribs) override;

    virtual void SAL_CALL characters( const OUString& rChars ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
