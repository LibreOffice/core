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

#include <memory>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <node.hxx>
#include <document.hxx>

using namespace css::io;
using namespace css::lang;
using namespace css::ucb;
using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::sax;
using namespace ucbhelper;
using css::task::XInteractionHandler;
using css::xml::sax::InputSource;


namespace DOM
{

    class CDefaultEntityResolver : public cppu::WeakImplHelper< XEntityResolver >
    {
    public:
        virtual InputSource SAL_CALL resolveEntity( const OUString& sPublicId, const OUString& sSystemId )
            throw (css::uno::RuntimeException, std::exception) override
        {
            InputSource is;
            is.sPublicId = sPublicId;
            is.sSystemId = sSystemId;
            is.sEncoding.clear();

            try {
                Reference< XCommandEnvironment > aEnvironment(
                    new CommandEnvironment(Reference< XInteractionHandler >(),
                                           Reference< XProgressHandler >() ));
                Content aContent(sSystemId, aEnvironment, comphelper::getProcessComponentContext());

                is.aInputStream = aContent.openStream();
            } catch (const css::uno::Exception&) {
                OSL_FAIL("exception in default entity resolver");
                is.aInputStream.clear();
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
        nullptr
    };

    OUString CDocumentBuilder::_getImplementationName()
    {
        return OUString::createFromAscii(aImplementationName);
    }
    Sequence<OUString> CDocumentBuilder::_getSupportedServiceNames()
    {
        Sequence<OUString> aSequence;
        for (int i=0; aSupportedServiceNames[i]!=nullptr; i++) {
            aSequence.realloc(i+1);
            aSequence[i]=(OUString::createFromAscii(aSupportedServiceNames[i]));
        }
        return aSequence;
    }

    Sequence< OUString > SAL_CALL CDocumentBuilder::getSupportedServiceNames()
        throw (RuntimeException, std::exception)
    {
        return CDocumentBuilder::_getSupportedServiceNames();
    }

    OUString SAL_CALL CDocumentBuilder::getImplementationName()
        throw (RuntimeException, std::exception)
    {
        return CDocumentBuilder::_getImplementationName();
    }

    sal_Bool SAL_CALL CDocumentBuilder::supportsService(const OUString& aServiceName)
        throw (RuntimeException, std::exception)
    {
        return cppu::supportsService(this, aServiceName);
    }

    Reference< XDOMImplementation > SAL_CALL CDocumentBuilder::getDOMImplementation()
        throw (RuntimeException, std::exception)
    {

        return Reference< XDOMImplementation >();
    }

    sal_Bool SAL_CALL CDocumentBuilder::isNamespaceAware()
        throw (RuntimeException, std::exception)
    {
        return sal_True;
    }

    sal_Bool SAL_CALL CDocumentBuilder::isValidating()
        throw (RuntimeException, std::exception)
    {
        return sal_False;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::newDocument()
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);

        // create a new document
        xmlDocPtr pDocument = xmlNewDoc(reinterpret_cast<const xmlChar*>("1.0"));
        Reference< XDocument > const xRet(
                CDocument::CreateCDocument(pDocument).get());
        return xRet;
    }

    static OUString make_error_message(xmlParserCtxtPtr ctxt)
    {
        OUStringBuffer buf;
        buf.appendAscii(ctxt->lastError.message);
        buf.append("Line: ");
        buf.append(static_cast<sal_Int32>(ctxt->lastError.line));
        buf.append("\nColumn: ");
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
        } catch (const css::uno::Exception& ex) {
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
        } catch (const css::uno::Exception& ex) {
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
        xmlParserCtxtPtr ctxt = static_cast<xmlParserCtxtPtr>(ctx);
        CDocumentBuilder *builder = static_cast< CDocumentBuilder* >(ctxt->_private);
        Reference< XEntityResolver > resolver = builder->getEntityResolver();
        OUString sysid;
        if (systemId != nullptr)
            sysid = OUString(reinterpret_cast<char const *>(systemId), strlen(reinterpret_cast<char const *>(systemId)), RTL_TEXTENCODING_UTF8);
        OUString pubid;
        if (publicId != nullptr)
            pubid = OUString(reinterpret_cast<char const *>(publicId), strlen(reinterpret_cast<char const *>(publicId)), RTL_TEXTENCODING_UTF8);

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
        SAL_INFO(
            "unoxml",
            "libxml2 warning: "
                << make_error_message(static_cast<xmlParserCtxtPtr>(ctx)));
    }

    // default error handler triggers assertion
    static void error_func(void * ctx, const char * /*msg*/, ...)
    {
        SAL_WARN(
            "unoxml",
            "libxml2 error: "
                << make_error_message(static_cast<xmlParserCtxtPtr>(ctx)));
    }

    } // extern "C"

    void throwEx(xmlParserCtxtPtr ctxt)
    {
        css::xml::sax::SAXParseException saxex;
        saxex.Message = make_error_message(ctxt);
        saxex.LineNumber = static_cast<sal_Int32>(ctxt->lastError.line);
        saxex.ColumnNumber = static_cast<sal_Int32>(ctxt->lastError.int2);
        throw saxex;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parse(const Reference< XInputStream >& is)
        throw (RuntimeException, SAXParseException, IOException, std::exception)
    {
        if (!is.is()) {
            throw RuntimeException();
        }

        ::osl::MutexGuard const g(m_Mutex);

        std::shared_ptr<xmlParserCtxt> const pContext(
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
                xmlIO_read_func, xmlIO_close_func, &c, nullptr, nullptr, 0);

        if (pDoc == nullptr) {
            throwEx(pContext.get());
        }
        Reference< XDocument > const xRet(
                CDocument::CreateCDocument(pDoc).get());
        return xRet;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parseURI(const OUString& sUri)
        throw (RuntimeException, SAXParseException, IOException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);

        std::shared_ptr<xmlParserCtxt> const pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt);
        pContext->_private = this;
        pContext->sax->error = error_func;
        pContext->sax->warning = warning_func;
        pContext->sax->resolveEntity = resolve_func;
        // xmlSetExternalEntityLoader(external_entity_loader);
        OString oUri = OUStringToOString(sUri, RTL_TEXTENCODING_UTF8);
        char *uri = const_cast<char*>(oUri.getStr());
        xmlDocPtr pDoc = xmlCtxtReadFile(pContext.get(), uri, nullptr, 0);
        if (pDoc == nullptr) {
            throwEx(pContext.get());
        }
        Reference< XDocument > const xRet(
                CDocument::CreateCDocument(pDoc).get());
        return xRet;
    }

    void SAL_CALL
    CDocumentBuilder::setEntityResolver(Reference< XEntityResolver > const& xER)
        throw (RuntimeException, std::exception)
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
        throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_xErrorHandler = xEH;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
