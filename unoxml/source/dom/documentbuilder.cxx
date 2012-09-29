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

#include <documentbuilder.hxx>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <libxml/xmlerror.h>
#include <libxml/tree.h>

#include <boost/shared_ptr.hpp>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <node.hxx>
#include <document.hxx>


using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::com::sun::star::xml::sax::InputSource;
using namespace ucbhelper;
using namespace ::com::sun::star::ucb;
using ::com::sun::star::task::XInteractionHandler;


namespace DOM
{

    class CDefaultEntityResolver : public cppu::WeakImplHelper1< XEntityResolver >
    {
    public:
        virtual InputSource SAL_CALL resolveEntity( const OUString& sPublicId, const OUString& sSystemId )
            throw (::com::sun::star::uno::RuntimeException)
        {
            InputSource is;
            is.sPublicId = sPublicId;
            is.sSystemId = sSystemId;
            is.sEncoding = OUString();

            try {
                Reference< XCommandEnvironment > aEnvironment(
                    new CommandEnvironment(Reference< XInteractionHandler >(),
                                           Reference< XProgressHandler >() ));
                Content aContent(sSystemId, aEnvironment, comphelper::getProcessComponentContext());

                is.aInputStream = aContent.openStream();
            } catch (const com::sun::star::uno::Exception&) {
                OSL_FAIL("exception in default entity resolver");
                is.aInputStream = Reference< XInputStream >();
            }
            return is;
        }

    };

    CDocumentBuilder::CDocumentBuilder(
            Reference< XMultiServiceFactory > const& xFactory)
        : m_xFactory(xFactory)
        , m_xEntityResolver(new CDefaultEntityResolver())
    {
        // init libxml. libxml will protect itself against multiple
        // initializations so there is no problem here if this gets
        // called multiple times.
        xmlInitParser();
    }

    Reference< XInterface > CDocumentBuilder::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        return static_cast< XDocumentBuilder* >(new CDocumentBuilder(rSMgr));
    }

    const char* CDocumentBuilder::aImplementationName = "com.sun.star.comp.xml.dom.DocumentBuilder";
    const char* CDocumentBuilder::aSupportedServiceNames[] = {
        "com.sun.star.xml.dom.DocumentBuilder",
        NULL
    };

    OUString CDocumentBuilder::_getImplementationName()
    {
        return OUString::createFromAscii(aImplementationName);
    }
    Sequence<OUString> CDocumentBuilder::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=NULL; i++) {
            aSequence.realloc(i+1);
            aSequence[i]=(OUString::createFromAscii(aSupportedServiceNames[i]));
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CDocumentBuilder::getSupportedServiceNames()
        throw (RuntimeException)
    {
        return CDocumentBuilder::_getSupportedServiceNames();
    }

    OUString SAL_CALL CDocumentBuilder::getImplementationName()
        throw (RuntimeException)
    {
        return CDocumentBuilder::_getImplementationName();
    }

    sal_Bool SAL_CALL CDocumentBuilder::supportsService(const OUString& aServiceName)
        throw (RuntimeException)
    {
        Sequence< OUString > supported = CDocumentBuilder::_getSupportedServiceNames();
        for (sal_Int32 i=0; i<supported.getLength(); i++)
        {
            if (supported[i] == aServiceName) return sal_True;
        }
        return sal_False;
    }

    Reference< XDOMImplementation > SAL_CALL CDocumentBuilder::getDOMImplementation()
        throw (RuntimeException)
    {

        return Reference< XDOMImplementation >();
    }

    sal_Bool SAL_CALL CDocumentBuilder::isNamespaceAware()
        throw (RuntimeException)
    {
        return sal_True;
    }

    sal_Bool SAL_CALL CDocumentBuilder::isValidating()
        throw (RuntimeException)
    {
        return sal_False;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::newDocument()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        // create a new document
        xmlDocPtr pDocument = xmlNewDoc((const xmlChar*)"1.0");
        Reference< XDocument > const xRet(
                CDocument::CreateCDocument(pDocument).get());
        return xRet;
    }

    static OUString make_error_message(xmlParserCtxtPtr ctxt)
    {
        OUStringBuffer buf;
        buf.appendAscii(ctxt->lastError.message);
        buf.appendAscii("Line: ");
        buf.append(static_cast<sal_Int32>(ctxt->lastError.line));
        buf.appendAscii("\nColumn: ");
        buf.append(static_cast<sal_Int32>(ctxt->lastError.int2));
        OUString msg = buf.makeStringAndClear();
        return msg;
    }

    // -- callbacks and context struct for parsing from stream
    // -- c-linkage, so the callbacks can be used by libxml
    extern "C" {

    // context struct passed to IO functions
    typedef struct context {
        CDocumentBuilder *pBuilder;
        Reference< XInputStream > rInputStream;
        bool close;
        bool freeOnClose;
    } context_t;

    static int xmlIO_read_func( void *context, char *buffer, int len)
    {
        // get the context...
        context_t *pctx = static_cast<context_t*>(context);
        if (!pctx->rInputStream.is())
            return -1;
        try {
            // try to read the requested number of bytes
            Sequence< sal_Int8 > chunk(len);
            int nread = pctx->rInputStream->readBytes(chunk, len);

            // copy bytes to the provided buffer
            memcpy(buffer, chunk.getConstArray(), nread);
            return nread;
        } catch (const com::sun::star::uno::Exception& ex) {
            (void) ex;
            OSL_FAIL(OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
            return -1;
        }
    }

    static int xmlIO_close_func(void* context)
    {
        // get the context...
        context_t *pctx = static_cast<context_t*>(context);
        if (!pctx->rInputStream.is())
            return 0;
        try
        {
            if (pctx->close)
                pctx->rInputStream->closeInput();
            if (pctx->freeOnClose)
                delete pctx;
            return 0;
        } catch (const com::sun::star::uno::Exception& ex) {
            (void) ex;
            OSL_FAIL(OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
            return -1;
        }
    }

    static xmlParserInputPtr resolve_func(void *ctx,
                                const xmlChar *publicId,
                                const xmlChar *systemId)
    {
        // get the CDocumentBuilder object
        xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)ctx;
        CDocumentBuilder *builder = static_cast< CDocumentBuilder* >(ctxt->_private);
        Reference< XEntityResolver > resolver = builder->getEntityResolver();
        OUString sysid;
        if (systemId != 0)
            sysid = OUString((sal_Char*)systemId, strlen((char*)systemId), RTL_TEXTENCODING_UTF8);
        OUString pubid;
        if (publicId != 0)
            pubid = OUString((sal_Char*)publicId, strlen((char*)publicId), RTL_TEXTENCODING_UTF8);

        // resolve the entity
        InputSource src = resolver->resolveEntity(pubid, sysid);

        // create IO context on heap because this call will no longer be on the stack
        // when IO is actually performed through the callbacks. The close function must
        // free the memory which is indicated by the freeOnClose field in the context struct
        context_t *c = new context_t;
        c->pBuilder = builder;
        c->rInputStream = src.aInputStream;
        c->close = true;
        c->freeOnClose = true;

        // set up the inputBuffer and inputPtr for libxml
        xmlParserInputBufferPtr pBuffer =
            xmlParserInputBufferCreateIO(xmlIO_read_func, xmlIO_close_func, c, XML_CHAR_ENCODING_NONE);
        xmlParserInputPtr pInput =
                    xmlNewIOInputStream(ctxt, pBuffer, XML_CHAR_ENCODING_NONE);
        return pInput;
    }

#if 0
    static xmlParserInputPtr external_entity_loader(const char *URL, const char * /*ID*/, xmlParserCtxtPtr ctxt)
    {
        // just call our resolver function using the URL as systemId
        return resolve_func(ctxt, 0, (const xmlChar*)URL);
    }
#endif

    // default warning handler does not trigger assertion
    static void warning_func(void * ctx, const char * /*msg*/, ...)
    {
        OUStringBuffer buf("libxml2 warning\n");
        buf.append(make_error_message(static_cast< xmlParserCtxtPtr >(ctx)));
        OString msg = OUStringToOString(buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        OSL_TRACE(msg.getStr());
    }

    // default error handler triggers assertion
    static void error_func(void * ctx, const char * /*msg*/, ...)
    {
        OUStringBuffer buf("libxml2 error\n");
        buf.append(make_error_message(static_cast< xmlParserCtxtPtr >(ctx)));
        OString msg = OUStringToOString(buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        OSL_FAIL(msg.getStr());
    }

    } // extern "C"

    void throwEx(xmlParserCtxtPtr ctxt)
    {
        com::sun::star::xml::sax::SAXParseException saxex;
        saxex.Message = make_error_message(ctxt);
        saxex.LineNumber = static_cast<sal_Int32>(ctxt->lastError.line);
        saxex.ColumnNumber = static_cast<sal_Int32>(ctxt->lastError.int2);
        throw saxex;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parse(const Reference< XInputStream >& is)
        throw (RuntimeException, SAXParseException, IOException)
    {
        if (!is.is()) {
            throw RuntimeException();
        }

        ::osl::MutexGuard const g(m_Mutex);

        // encoding...
        /*
        xmlChar *encstr = (xmlChar*) OUStringToOString(src.sEncoding, RTL_TEXTENCODING_UTF8).getStr();
        xmlCharEncoding enc = xmlParseCharEncoding(encstr);
        */

        ::boost::shared_ptr<xmlParserCtxt> const pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt);

        // register error functions to prevent errors being printed
        // on the console
        pContext->_private = this;
        pContext->sax->error = error_func;
        pContext->sax->warning = warning_func;
        pContext->sax->resolveEntity = resolve_func;

        // IO context struct
        context_t c;
        c.pBuilder = this;
        c.rInputStream = is;
        // we did not open the stream, thus we do not close it.
        c.close = false;
        c.freeOnClose = false;
        xmlDocPtr const pDoc = xmlCtxtReadIO(pContext.get(),
                xmlIO_read_func, xmlIO_close_func, &c, 0, 0, 0);

        if (pDoc == 0) {
            throwEx(pContext.get());
        }
        Reference< XDocument > const xRet(
                CDocument::CreateCDocument(pDoc).get());
        return xRet;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parseURI(const OUString& sUri)
        throw (RuntimeException, SAXParseException, IOException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        ::boost::shared_ptr<xmlParserCtxt> const pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt);
        pContext->_private = this;
        pContext->sax->error = error_func;
        pContext->sax->warning = warning_func;
        pContext->sax->resolveEntity = resolve_func;
        // xmlSetExternalEntityLoader(external_entity_loader);
        OString oUri = OUStringToOString(sUri, RTL_TEXTENCODING_UTF8);
        char *uri = (char*) oUri.getStr();
        xmlDocPtr pDoc = xmlCtxtReadFile(pContext.get(), uri, 0, 0);
        if (pDoc == 0) {
            throwEx(pContext.get());
        }
        Reference< XDocument > const xRet(
                CDocument::CreateCDocument(pDoc).get());
        return xRet;
    }

    void SAL_CALL
    CDocumentBuilder::setEntityResolver(Reference< XEntityResolver > const& xER)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_xEntityResolver = xER;
    }

    Reference< XEntityResolver > SAL_CALL CDocumentBuilder::getEntityResolver()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        return m_xEntityResolver;
    }

    void SAL_CALL
    CDocumentBuilder::setErrorHandler(Reference< XErrorHandler > const& xEH)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_xErrorHandler = xEH;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
