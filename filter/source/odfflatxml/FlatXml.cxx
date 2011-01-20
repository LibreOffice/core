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

#include "FlatXml.hxx"

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

    sal_Bool
    XFlatXml::importer(const Sequence<PropertyValue>& aSourceData,
            const Reference<XDocumentHandler>& xHandler, const Sequence<
                    OUString>& /* msUserData */) throw (RuntimeException)
    {
        // get information from media descriptor
        // the imput stream that represents the imported file
        // is most important here since we need to supply it to
        // the sax parser that drives the supplied document handler
        sal_Int32 nLength = aSourceData.getLength();
        OUString aName, aFileName, aURL;
        Reference<XInputStream> xInputStream;
        for (sal_Int32 i = 0; i < nLength; i++)
            {
                aName = aSourceData[i].Name;
                if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("InputStream")))
                    aSourceData[i].Value >>= xInputStream;
                else if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("FileName")))
                    aSourceData[i].Value >>= aFileName;
                else if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("URL")))
                    aSourceData[i].Value >>= aURL;
            }

        // we need an input stream
        OSL_ASSERT(xInputStream.is());
        if (!xInputStream.is())
            return sal_False;

        // rewind seekable stream
        Reference<XSeekable> xSeek(xInputStream, UNO_QUERY);
        if (xSeek.is())
            xSeek->seek(0);

        // create SAX parser that will read the document file
        // and provide events to xHandler passed to this call
        Reference<XParser>
                xSaxParser(
                        m_rServiceFactory->createInstance(
                                OUString(
                                        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser"))),
                        UNO_QUERY);
        OSL_ASSERT(xSaxParser.is());
        if (!xSaxParser.is())
            return sal_False;

        // let the parser try to send the sax event to the document handler
        try
            {
                InputSource aInput;
                aInput.sSystemId = aURL;
                aInput.sPublicId = aURL;
                aInput.aInputStream = xInputStream;
                xSaxParser->setDocumentHandler(xHandler);
                xSaxParser->parseStream(aInput);
            }
        catch (Exception &exc)
            {
                // something went wrong
                OString msg = OUStringToOString(exc.Message,
                        RTL_TEXTENCODING_ASCII_US);
                OSL_ENSURE(0, msg);
                return sal_False;
            }

        // done
        return sal_True;
    }

    sal_Bool
    XFlatXml::exporter(const Sequence<PropertyValue>& aSourceData,
            const Sequence<OUString>& /*msUserData*/) throw (RuntimeException)
    {

        // read source data
        // we are especialy interested in the output stream
        // since that is where our xml-writer will push the data
        // from it's data-source interface
        OUString aName, sURL;
        Reference<XOutputStream> rOutputStream;
        sal_Int32 nLength = aSourceData.getLength();
        for (sal_Int32 i = 0; i < nLength; i++)
            {
                aName = aSourceData[i].Name;
                if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("OutputStream")))
                    aSourceData[i].Value >>= rOutputStream;
                else if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("URL")))
                    aSourceData[i].Value >>= sURL;
            }

        if (!m_rDocumentHandler.is())
            {
                // get the document writer
                m_rDocumentHandler
                        = Reference<XExtendedDocumentHandler> (
                                m_rServiceFactory->createInstance(
                                        OUString(
                                                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Writer"))),
                                UNO_QUERY);
                OSL_ASSERT(m_rDocumentHandler.is());
                if (!m_rDocumentHandler.is())
                    return sal_False;
            }
        // get data source interface ...
        Reference<XActiveDataSource> rDataSource(m_rDocumentHandler, UNO_QUERY);
        OSL_ASSERT(rDataSource.is());
        if (!rDataSource.is())
            return sal_False;
        OSL_ASSERT(rOutputStream.is());
        if (!rOutputStream.is())
            return sal_False;
        // ... and set output stream
        rDataSource->setOutputStream(rOutputStream);

        return sal_True;
    }

    // for the DocumentHandler implementation, we just proxy the the
    // events to the XML writer that we created upon the output stream
    // that was provided by the XMLFilterAdapter
    void
    XFlatXml::startDocument() throw (SAXException, RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->startDocument();
    }

    void
    XFlatXml::endDocument() throw (SAXException, RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->endDocument();
    }

    void
    XFlatXml::startElement(const OUString& str,
            const Reference<XAttributeList>& attriblist) throw (SAXException,
            RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->startElement(str, attriblist);
    }

    void
    XFlatXml::endElement(const OUString& str) throw (SAXException,
            RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->endElement(str);
    }

    void
    XFlatXml::characters(const OUString& str) throw (SAXException,
            RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->characters(str);
    }

    void
    XFlatXml::ignorableWhitespace(const OUString& str) throw (SAXException,
            RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        if (!m_bPrettyPrint)
            return;
        m_rDocumentHandler->ignorableWhitespace(str);
    }

    void
    XFlatXml::processingInstruction(const OUString& str, const OUString& str2)
            throw (SAXException, RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->processingInstruction(str, str2);
    }

    void
    XFlatXml::setDocumentLocator(const Reference<XLocator>& doclocator)
            throw (SAXException, RuntimeException)
    {
        OSL_ASSERT(m_rDocumentHandler.is());
        m_rDocumentHandler->setDocumentLocator(doclocator);
    }

    // --------------------------------------
    // Component management
    // --------------------------------------
    Reference<XInterface> SAL_CALL
    CreateInstance(const Reference<XMultiServiceFactory> &r)
    {
        return Reference<XInterface> ((OWeakObject *) new XFlatXml(r));
    }

    Sequence<OUString>
    getSupportedServiceNames()
    {
        static Sequence<OUString> *pNames = 0;
        if (!pNames)
            {
                MutexGuard guard(Mutex::getGlobalMutex());
                if (!pNames)
                    {
                        static Sequence<OUString> seqNames(2);
                        seqNames.getArray()[0] = OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.document.ImportFilter"));
                        seqNames.getArray()[1] = OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.document.ExportFilter"));
                        pNames = &seqNames;
                    }
            }
        return *pNames;
    }

}

using namespace XFlatXml;
#define IMPLEMENTATION_NAME "com.sun.star.comp.filter.FlatXmlCpp"

extern "C"
{
    void SAL_CALL
    component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName,
            uno_Environment ** /* ppEnv */)
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    sal_Bool SAL_CALL
    component_writeInfo(void * /* pServiceManager */, void * pRegistryKey)
    {
        if (pRegistryKey)
            {
                try
                    {
                        Reference<XRegistryKey>
                                xNewKey(
                                        reinterpret_cast<XRegistryKey *> (pRegistryKey)->createKey(
                                                OUString(
                                                        RTL_CONSTASCII_USTRINGPARAM( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ))));

                        const Sequence<OUString> & rSNL =
                                getSupportedServiceNames();
                        const OUString * pArray = rSNL.getConstArray();
                        for (sal_Int32 nPos = rSNL.getLength(); nPos--;)
                            xNewKey->createKey(pArray[nPos]);

                        return sal_True;
                    }
                catch (InvalidRegistryException &)
                    {
                        OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
                    }
            }
        return sal_False;
    }

    void *
    SAL_CALL component_getFactory(const sal_Char * pImplName,
            void * pServiceManager, void * /* pRegistryKey */)
    {
        void * pRet = 0;

        if (pServiceManager && rtl_str_compare(pImplName, IMPLEMENTATION_NAME)
                == 0)
            {
                Reference<XSingleServiceFactory>
                        xFactory(
                                createSingleFactory(
                                        reinterpret_cast<XMultiServiceFactory *> (pServiceManager),
                                        OUString::createFromAscii(pImplName),
                                        CreateInstance,
                                        getSupportedServiceNames()));

                if (xFactory.is())
                    {
                        xFactory->acquire();
                        pRet = xFactory.get();
                    }
            }
        return pRet;
    }

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
