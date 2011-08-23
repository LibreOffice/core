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

#ifndef _TYPEDETECTION_IMPORT_HXX
#define _TYPEDETECTION_IMPORT_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx> // helper for implementations
#ifndef _COM_SUN_STAR_XML_SAX_XDUCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
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
    PropertyMap	maPropertyMap;

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
