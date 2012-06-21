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

#ifndef _TYPEDETECTION_IMPORT_HXX
#define _TYPEDETECTION_IMPORT_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <comphelper/stl_types.hxx>

#include "xmlfilterjar.hxx"

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

DECLARE_STL_USTRINGACCESS_MAP( ::rtl::OUString, PropertyMap );

struct Node
{
    ::rtl::OUString maName;
    PropertyMap maPropertyMap;
};

typedef std::vector< Node* > NodeVector;

class TypeDetectionImporter : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
public:
    TypeDetectionImporter( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF );
    virtual ~TypeDetectionImporter( void );

    static void doImport( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xMSF, com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xOS, XMLFilterVector& rFilters );

    virtual void SAL_CALL startDocument(  )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    void fillFilterVector(  XMLFilterVector& rFilters );
    filter_info_impl* createFilterForNode( Node * pNode );
    Node* findTypeNode( const ::rtl::OUString& rType );

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    std::stack< ImportState > maStack;
    PropertyMap maPropertyMap;

    NodeVector maFilterNodes;
    NodeVector maTypeNodes;

    ::rtl::OUString maValue;
    ::rtl::OUString maNodeName;
    ::rtl::OUString maPropertyName;

    const ::rtl::OUString sRootNode;
    const ::rtl::OUString sNode;
    const ::rtl::OUString sName;
    const ::rtl::OUString sProp;
    const ::rtl::OUString sValue;
    const ::rtl::OUString sUIName;
    const ::rtl::OUString sData;
    const ::rtl::OUString sFilters;
    const ::rtl::OUString sTypes;
    const ::rtl::OUString sFilterAdaptorService;
    const ::rtl::OUString sXSLTFilterService;

    const ::rtl::OUString sCdataAttribute;
    const ::rtl::OUString sWhiteSpace;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
