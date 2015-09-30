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

#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_TYPEDETECTIONIMPORT_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_TYPEDETECTIONIMPORT_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include "xmlfilterjar.hxx"

#include <map>
#include <vector>
#include <stack>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { struct PropertyValue; }
} } }

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

typedef std::vector< Node* > NodeVector;

class TypeDetectionImporter : public cppu::WeakImplHelper < css::xml::sax::XDocumentHandler >
{
public:
    TypeDetectionImporter();
    virtual ~TypeDetectionImporter();

    static void doImport( const css::uno::Reference< css::uno::XComponentContext >& rxContext, css::uno::Reference < css::io::XInputStream > xOS, XMLFilterVector& rFilters );

    virtual void SAL_CALL startDocument(  )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endDocument(  )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL startElement( const OUString& aName, const css::uno::Reference< css::xml::sax::XAttributeList >& xAttribs )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL endElement( const OUString& aName )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL characters( const OUString& aChars )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL processingInstruction( const OUString& aTarget, const OUString& aData )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw(css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    void fillFilterVector(  XMLFilterVector& rFilters );
    filter_info_impl* createFilterForNode( Node * pNode );
    Node* findTypeNode( const OUString& rType );

    std::stack< ImportState > maStack;
    PropertyMap maPropertyMap;

    NodeVector maFilterNodes;
    NodeVector maTypeNodes;

    OUString maValue;
    OUString maNodeName;
    OUString maPropertyName;

    const OUString sRootNode;
    const OUString sNode;
    const OUString sName;
    const OUString sProp;
    const OUString sValue;
    const OUString sUIName;
    const OUString sData;
    const OUString sFilters;
    const OUString sTypes;
    const OUString sFilterAdaptorService;
    const OUString sXSLTFilterService;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
