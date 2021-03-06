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

#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include "xmlfilterjar.hxx"

#include <map>
#include <memory>
#include <vector>
#include <stack>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace beans { struct PropertyValue; }
}

enum ImportState
{
    e_Root,
    e_Filters,
    e_Types,
    e_Filter,
    e_Type,
    e_Property,
    e_Value,
    e_Unknown
};

typedef std::map<OUString, OUString> PropertyMap;

struct Node
{
    OUString maName;
    PropertyMap maPropertyMap;
};

class TypeDetectionImporter : public cppu::WeakImplHelper < css::xml::sax::XDocumentHandler >
{
public:
    TypeDetectionImporter();
    virtual ~TypeDetectionImporter() override;

    static void doImport( const css::uno::Reference< css::uno::XComponentContext >& rxContext, const css::uno::Reference < css::io::XInputStream >& xOS,
                          std::vector< std::unique_ptr<filter_info_impl> >& rFilters );

    virtual void SAL_CALL startDocument(  ) override;
    virtual void SAL_CALL endDocument(  ) override;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs ) override;
    virtual void SAL_CALL endElement( const OUString& aName ) override;
    virtual void SAL_CALL characters( const OUString& aChars ) override;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData ) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator ) override;

private:
    void fillFilterVector(  std::vector< std::unique_ptr<filter_info_impl> >& rFilters );
    std::unique_ptr<filter_info_impl> createFilterForNode( Node * pNode );
    Node* findTypeNode( const OUString& rType );

    std::stack< ImportState > maStack;
    PropertyMap maPropertyMap;

    std::vector< std::unique_ptr<Node> > maFilterNodes;
    std::vector< std::unique_ptr<Node> > maTypeNodes;

    OUString maValue;
    OUString maNodeName;
    OUString maPropertyName;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
