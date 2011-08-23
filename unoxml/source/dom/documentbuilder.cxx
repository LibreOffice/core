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

#include "documentbuilder.hxx"
#include "node.hxx"
#include "document.hxx"

#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/implbase1.hxx>

#include <libxml/xmlerror.h>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

#include <string.h>
#include <stdio.h>
#include <stdarg.h>


using ::rtl::OUStringBuffer;
using ::rtl::OString;
using ::com::sun::star::xml::sax::InputSource;
using namespace ucbhelper;
using namespace ::com::sun::star::ucb;
using ::com::sun::star::task::XInteractionHandler;


namespace DOM
{
    extern "C" {
        //char *strdup(const char *s);
        /*
        static char* strdupfunc(const char* s)
        {
            sal_Int32 len = 0;
            while (s[len] != '\0') len++;
            char *newStr = (char*)rtl_allocateMemory(len+1);
            if (newStr != NULL)
                rtl_copyMemory(newStr, s, len+1);
            return newStr;
        }
        */
    }


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
                Content aContent(sSystemId, aEnvironment);

                is.aInputStream = aContent.openStream();
            } catch (com::sun::star::uno::Exception) {
                OSL_ENSURE(sal_False, "exception in default entity resolver");
                is.aInputStream = Reference< XInputStream >();
            }
            return is;
        }

    };

    CDocumentBuilder::CDocumentBuilder(const Reference< XMultiServiceFactory >& xFactory)
        : m_aFactory(xFactory)
        , m_aEntityResolver(Reference< XEntityResolver > (new CDefaultEntityResolver()))
    {
        // init libxml. libxml will protect itself against multiple
        // initializations so there is no problem here if this gets
        // called multiple times.
        xmlInitParser();
    }

    Reference< XInterface > CDocumentBuilder::_getInstance(const Reference< XMultiServiceFactory >& rSMgr)
    {
        // XXX
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
        // create a new document
        xmlDocPtr pDocument = xmlNewDoc((const xmlChar*)"1.0");
        return Reference< XDocument >(static_cast< CDocument* >(CNode::get((xmlNodePtr)pDocument)));
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
            rtl_copyMemory(buffer, chunk.getConstArray(), nread);            
            return nread;
        } catch (com::sun::star::uno::Exception& ex) {
            (void) ex;
            OSL_ENSURE(sal_False, OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
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
        } catch (com::sun::star::uno::Exception& ex) {
            (void) ex;
            OSL_ENSURE(sal_False, OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
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

    static xmlParserInputPtr external_entity_loader(const char *URL, const char * /*ID*/, xmlParserCtxtPtr ctxt)
    {
        // just call our resolver function using the URL as systemId
        return resolve_func(ctxt, 0, (const xmlChar*)URL);
    }

    // default warning handler triggers assertion
    static void warning_func(void * ctx, const char * /*msg*/, ...)
    {
        OUStringBuffer buf(OUString::createFromAscii("libxml2 warning\n"));
        buf.append(make_error_message(static_cast< xmlParserCtxtPtr >(ctx)));
        OString msg = OUStringToOString(buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(sal_False, msg.getStr());
    }

    // default error handler triggers assertion
    static void error_func(void * ctx, const char * /*msg*/, ...)
    {		
        OUStringBuffer buf(OUString::createFromAscii("libxml2 error\n"));
        buf.append(make_error_message(static_cast< xmlParserCtxtPtr >(ctx)));
        OString msg = OUStringToOString(buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US);
        OSL_ENSURE(sal_False, msg.getStr());
    }

    } // extern "C"

    void throwEx(xmlParserCtxtPtr ctxt) {
        OUString msg = make_error_message(ctxt);
        xmlFreeParserCtxt(ctxt);
        com::sun::star::xml::sax::SAXParseException saxex;
        saxex.Message = msg;
        saxex.LineNumber = static_cast<sal_Int32>(ctxt->lastError.line);
        saxex.ColumnNumber = static_cast<sal_Int32>(ctxt->lastError.int2);
        throw saxex;
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parse(const Reference< XInputStream >& is)
        throw (RuntimeException, SAXParseException, IOException)
    {

        // encoding...
        /*
        xmlChar *encstr = (xmlChar*) OUStringToOString(src.sEncoding, RTL_TEXTENCODING_UTF8).getStr();
        xmlCharEncoding enc = xmlParseCharEncoding(encstr);
        */

        xmlParserCtxtPtr ctxt = xmlNewParserCtxt();

        // register error functions to prevent errors being printed
        // on the console
        ctxt->_private = this;
        ctxt->sax->error = error_func;
        ctxt->sax->warning = warning_func;
        ctxt->sax->resolveEntity = resolve_func;

        // IO context struct
        context_t c;
        c.pBuilder = this;
        c.rInputStream = is;
        // we did not open the stream, thus we do not close it.
        c.close = false;
        c.freeOnClose = false;
        xmlDocPtr pDoc = xmlCtxtReadIO(ctxt, xmlIO_read_func, xmlIO_close_func, &c,
                     0, 0, 0);

        if (pDoc == 0) {
            throwEx(ctxt);
        }
        xmlFreeParserCtxt(ctxt);
        return Reference< XDocument >(static_cast< CDocument* >(CNode::get((xmlNodePtr)pDoc)));
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parseSource(const InputSource& is)
        throw (RuntimeException, SAXParseException, IOException)
    {
        // if there is an encoding specified in the input source, use it
        xmlCharEncoding enc = XML_CHAR_ENCODING_NONE;
        if (is.sEncoding.getLength() > 0) {
            OString oEncstr = OUStringToOString(is.sEncoding, RTL_TEXTENCODING_UTF8);
            char *encstr = (char*) oEncstr.getStr();
            enc = xmlParseCharEncoding(encstr);
        }

        // set up parser context
        xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
        // register error functions to prevent errors being printed
        // on the console
        ctxt->_private = this;
        ctxt->sax->error = error_func;
        ctxt->sax->warning = warning_func;

        // setup entity resolver binding(s)
        ctxt->sax->resolveEntity = resolve_func;
        xmlSetExternalEntityLoader(external_entity_loader);

        // if an input stream is provided, use it

        // use the systemID

        return Reference< XDocument >();
    }

    Reference< XDocument > SAL_CALL CDocumentBuilder::parseURI(const OUString& sUri)
        throw (RuntimeException, SAXParseException, IOException)
    {
        xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
        ctxt->_private = this;
        ctxt->sax->error = error_func;
        ctxt->sax->warning = warning_func;
        ctxt->sax->resolveEntity = resolve_func;
        // xmlSetExternalEntityLoader(external_entity_loader);
        OString oUri = OUStringToOString(sUri, RTL_TEXTENCODING_UTF8);
        char *uri = (char*) oUri.getStr();
        xmlDocPtr pDoc = xmlCtxtReadFile(ctxt, uri, 0, 0);
        if (pDoc == 0) {
            throwEx(ctxt);
        }
        xmlFreeParserCtxt(ctxt);
        return Reference< XDocument >(static_cast< CDocument* >(CNode::get((xmlNodePtr)pDoc)));
    }

    void SAL_CALL CDocumentBuilder::setEntityResolver(const Reference< XEntityResolver >& er)
        throw (RuntimeException)
    {
        m_aEntityResolver = er;
    }

    Reference< XEntityResolver > SAL_CALL CDocumentBuilder::getEntityResolver()
        throw (RuntimeException)
    {
        return m_aEntityResolver;
    }


    void SAL_CALL CDocumentBuilder::setErrorHandler(const Reference< XErrorHandler >& eh)
        throw (RuntimeException)
    {
        m_aErrorHandler = eh;
    }
}
