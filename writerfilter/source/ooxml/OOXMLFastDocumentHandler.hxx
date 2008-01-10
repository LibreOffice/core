/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLFastDocumentHandler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:58:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

private:
    OOXMLFastDocumentHandler(OOXMLFastDocumentHandler &); // not defined
    void operator =(OOXMLFastDocumentHandler &); // not defined

    uno::Reference< uno::XComponentContext > m_xContext;

    Stream * mpStream;
    OOXMLDocument * mpDocument;
    ::rtl::OUString msXNoteId;
    boost::shared_ptr<OOXMLFastContextHandler> mpContextHandler;
};
}}

#endif // INCLUDED_OOXML_FAST_DOCUMENT_HANDLER_HXX
