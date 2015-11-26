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

#ifndef INCLUDED_XMLOFF_INC_DOMBUILDERCONTEXT_HXX
#define INCLUDED_XMLOFF_INC_DOMBUILDERCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>



// forward declarations


namespace com { namespace sun { namespace star {
    namespace xml { namespace dom {
        class XNode;
        class XDocument;
    } }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
class SvXMLImport;
class SvXMLImportContext;

/**
 * DomBuilderContext creates a DOM tree suitable for in-memory processing of
 * XML data from a sequence of SAX events */
class DomBuilderContext : public SvXMLImportContext
{
    css::uno::Reference<css::xml::dom::XNode> mxNode;

public:

    /** default constructor: create new DOM tree */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const OUString& rLocalName );

    /** constructor: create DOM subtree under the given node */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const OUString& rLocalName,
                       css::uno::Reference<css::xml::dom::XNode>& );

    virtual ~DomBuilderContext();



    // access to the DOM tree


    /** access the DOM tree */
    css::uno::Reference<css::xml::dom::XDocument> getTree();



    // implement SvXMLImportContext methods:


    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList >& xAttrList ) override;

    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;

    virtual void Characters( const OUString& rChars ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
