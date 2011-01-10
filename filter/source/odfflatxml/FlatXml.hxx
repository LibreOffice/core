/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
#ifndef __XFLATXML_HXX__
#define __XFLATXML_HXX__

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;

namespace XFlatXml
{

    /*
     * XFlatXml export and imports ODF flat XML documents by plumbing a pass-through
     * filter implementation to XmlFilterAdaptor.
     *
     * XXX: Currently, this code adds nothing to the flat XML filter implementation
     * provided with the ODK.
     */
    class XFlatXml : public WeakImplHelper3<XImportFilter, XExportFilter,
            XDocumentHandler>
    {
    private:
        // the UNO ServiceFactory
        Reference<XMultiServiceFactory> m_rServiceFactory;

        // DocumentHandler interface of the css::xml::sax::Writer service
        Reference<XExtendedDocumentHandler> m_rDocumentHandler;

        // controls pretty-printing
        sal_Bool m_bPrettyPrint;

    public:

        // ctor...
        XFlatXml(const Reference<XMultiServiceFactory> &r) :
            m_rServiceFactory(r), m_bPrettyPrint(sal_True)
        {
        }

        // XImportFilter
        virtual sal_Bool SAL_CALL
        importer(const Sequence<PropertyValue>& aSourceData, const Reference<
                XDocumentHandler>& xHandler,
                const Sequence<OUString>& msUserData) throw (RuntimeException);

        // XExportFilter
        virtual sal_Bool SAL_CALL
        exporter(const Sequence<PropertyValue>& aSourceData, const Sequence<
                OUString>& msUserData) throw (RuntimeException);

        // XDocumentHandler
        virtual void SAL_CALL
        startDocument() throw (SAXException, RuntimeException);
        virtual void SAL_CALL
        endDocument() throw (SAXException, RuntimeException);
        virtual void SAL_CALL
        startElement(const OUString& str,
                const Reference<XAttributeList>& attriblist)
                throw (SAXException, RuntimeException);
        virtual void SAL_CALL
        endElement(const OUString& str) throw (SAXException, RuntimeException);
        virtual void SAL_CALL
        characters(const OUString& str) throw (SAXException, RuntimeException);
        virtual void SAL_CALL
        ignorableWhitespace(const OUString& str) throw (SAXException,
                RuntimeException);
        virtual void SAL_CALL
        processingInstruction(const OUString& str, const OUString& str2)
                throw (com::sun::star::xml::sax::SAXException,
                RuntimeException);
        virtual void SAL_CALL
        setDocumentLocator(const Reference<XLocator>& doclocator)
                throw (SAXException, RuntimeException);
    };
}
#endif // __XFLATXML_HXX__
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
