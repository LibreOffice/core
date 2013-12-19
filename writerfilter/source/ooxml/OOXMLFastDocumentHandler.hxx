/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    OOXMLFastDocumentHandler(
        uno::Reference< uno::XComponentContext > const & context,
        Stream* pStream,
        OOXMLDocument* pDocument );

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
    mutable boost::shared_ptr<OOXMLFastContextHandler> mpContextHandler;
    boost::shared_ptr<OOXMLFastContextHandler> getContextHandler() const;
};
}}

#endif // INCLUDED_OOXML_FAST_DOCUMENT_HANDLER_HXX
