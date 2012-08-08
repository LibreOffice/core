/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_DOMBUILDERCONTEXT_HXX
#define _XMLOFF_DOMBUILDERCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>


//
// forward declarations
//

namespace com { namespace sun { namespace star {
    namespace xml { namespace dom {
        class XNode;
        class XDocument;
    } }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}
class SvXMLImport;
class SvXMLImportContext;

/**
 * DomBuilderContext creates a DOM tree suitable for in-memory processing of
 * XML data from a sequence of SAX events */
class DomBuilderContext : public SvXMLImportContext
{
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> mxNode;

public:

    /** default constructor: create new DOM tree */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const ::rtl::OUString& rLocalName );

    /** constructor: create DOM subtree under the given node */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const ::rtl::OUString& rLocalName,
                       com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode>& );

    virtual ~DomBuilderContext();


    //
    // access to the DOM tree
    //

    /** access the DOM tree */
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocument> getTree();


    //
    // implement SvXMLImportContext methods:
    //

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement(
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
