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

#ifndef INCLUDED_OOXML_FAST_DOCUMENT_HANDLER_HXX
#define INCLUDED_OOXML_FAST_DOCUMENT_HANDLER_HXX

#include <sal/config.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <ooxml/OOXMLDocument.hxx>
#include "OOXMLParserState.hxx"

namespace writerfilter {
namespace ooxml
{
using namespace ::com::sun::star;

class OOXMLFastContextHandler;

class OOXMLFastDocumentHandler:
    public ::cppu::WeakImplHelper1<
        xml::sax::XFastDocumentHandler>
{
public:
    OOXMLFastDocumentHandler
    (uno::Reference< uno::XComponentContext > const & context);
    virtual ~OOXMLFastDocumentHandler() {}

    // ::com::sun::star::xml::sax::XFastDocumentHandler:
    virtual void SAL_CALL startDocument() 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void SAL_CALL endDocument() 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void SAL_CALL setDocumentLocator
    (const uno::Reference< xml::sax::XLocator > & xLocator) 
        throw (uno::RuntimeException, xml::sax::SAXException);

    // ::com::sun::star::xml::sax::XFastContextHandler:
    virtual void SAL_CALL startFastElement
    (::sal_Int32 Element, 
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void SAL_CALL startUnknownElement
    (const ::rtl::OUString & Namespace, 
     const ::rtl::OUString & Name, 
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void SAL_CALL endFastElement(::sal_Int32 Element) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void SAL_CALL endUnknownElement
    (const ::rtl::OUString & Namespace, 
     const ::rtl::OUString & Name) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL 
    createFastChildContext
    (::sal_Int32 Element, 
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual uno::Reference< xml::sax::XFastContextHandler > SAL_CALL 
    createUnknownChildContext
    (const ::rtl::OUString & Namespace, 
     const ::rtl::OUString & Name, 
     const uno::Reference< xml::sax::XFastAttributeList > & Attribs) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    virtual void SAL_CALL characters(const ::rtl::OUString & aChars) 
        throw (uno::RuntimeException, xml::sax::SAXException);
    
    void setStream(Stream * pStream);
    void setDocument(OOXMLDocument * pDocument);
    void setXNoteId(const ::rtl::OUString & rXNoteId);
    
    void setIsSubstream( bool bSubstream );

private:
    OOXMLFastDocumentHandler(OOXMLFastDocumentHandler &); // not defined
    void operator =(OOXMLFastDocumentHandler &); // not defined

    uno::Reference< uno::XComponentContext > m_xContext;

    Stream * mpStream;
#ifdef DEBUG_ELEMENT
    Stream::Pointer_t mpTmpStream;
#endif
    OOXMLDocument * mpDocument;
    ::rtl::OUString msXNoteId;
    mutable boost::shared_ptr<OOXMLFastContextHandler> mpContextHandler;
    boost::shared_ptr<OOXMLFastContextHandler> getContextHandler() const;
};
}}

#endif // INCLUDED_OOXML_FAST_DOCUMENT_HANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
