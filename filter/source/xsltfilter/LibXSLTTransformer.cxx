/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <cstdio>
#include <cstring>
#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlstring.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/variables.h>
#include <libxslt/extensions.h>
#include <libexslt/exslt.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase.hxx>

#include <osl/module.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>

#include <LibXSLTTransformer.hxx>
#include <OleHandler.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using ::std::list;
using ::std::map;
using ::std::pair;

#define _INPUT_BUFFER_SIZE 4096
#define _OUTPUT_BUFFER_SIZE 4096

namespace XSLT
{
    const char* const LibXSLTTransformer::PARAM_SOURCE_URL = "sourceURL";
    const char* const LibXSLTTransformer::PARAM_SOURCE_BASE_URL =
            "sourceBaseURL";
    const char* const LibXSLTTransformer::PARAM_TARGET_URL = "targetURL";
    const char* const LibXSLTTransformer::PARAM_TARGET_BASE_URL =
            "targetBaseURL";
    const char* const LibXSLTTransformer::PARAM_DOCTYPE_PUBLIC = "publicType";

    const sal_Int32 Reader::OUTPUT_BUFFER_SIZE = _OUTPUT_BUFFER_SIZE;

    const sal_Int32 Reader::INPUT_BUFFER_SIZE = _INPUT_BUFFER_SIZE;

    /**
     * ParserInputBufferCallback forwards IO call-backs to libxml stream IO.
     */
    struct ParserInputBufferCallback
    {
        static int
        on_read(void * context, char * buffer, int len)
        {
            Reader * tmp = static_cast<Reader*> (context);
            return tmp->read(buffer, len);
        }
        static int
        on_close(void * context)
        {
            Reader * tmp = static_cast<Reader*> (context);
            return tmp->closeInput();
        }
    };
    /**
     * ParserOutputBufferCallback forwards IO call-backs to libxml stream IO.
     */
    struct ParserOutputBufferCallback
    {
        static int
        on_write(void * context, const char * buffer, int len)
        {
            Reader * tmp = static_cast<Reader*> (context);
            return tmp->write(buffer, len);
        }
        static int
        on_close(void * context)
        {
            Reader * tmp = static_cast<Reader*> (context);
            return tmp->closeOutput();
        }
    };
    /**
     * ExtFuncOleCB forwards XPath extension function calls registered with libxslt to the OleHandler instance that actually
     * provides the implementation for those functions.
     *
     * The OLE extension module currently supplies two functions
     * insertByName: registers an OLE object to be later inserted into the output tree.
     * getByName: reads a previously registered OLE object and returns a base64 encoded string representation.
     */
    struct ExtFuncOleCB
    {
        static void *
        init(xsltTransformContextPtr, const xmlChar*)
        {
            return NULL;
        }
        static void
        insertByName(xmlXPathParserContextPtr ctxt, int nargs)
        {
            xsltTransformContextPtr tctxt;
            void *data;
            if (nargs != 2) {
                xsltGenericError(xsltGenericErrorContext,
                    "insertByName: requires exactly 2 arguments\n");
                return;
            }
            tctxt = xsltXPathGetTransformContext(ctxt);
            if (tctxt == NULL) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get the transformation context\n");
                return;
            }
            // XXX: someone with better knowledge of libxslt might come up with a better
            // idea to pass the OleHandler than by attaching it to tctxt->_private. See also
            // below.
            data = tctxt->_private;
            if (data == NULL) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get module data\n");
                return;
            }
            OleHandler * oh = static_cast<OleHandler*> (data);

            xmlXPathObjectPtr value = valuePop(ctxt);
            value = ensureStringValue(value, ctxt);
            xmlXPathObjectPtr streamName = valuePop(ctxt);
            streamName = ensureStringValue(streamName, ctxt);

            oh->insertByName(OUString::createFromAscii((sal_Char*) streamName->stringval), OString((sal_Char*) value->stringval));
            valuePush(ctxt, xmlXPathNewCString(""));
        }

        static xmlXPathObjectPtr ensureStringValue(xmlXPathObjectPtr obj, const xmlXPathParserContextPtr ctxt)
        {
            if (obj->type != XPATH_STRING) {
                valuePush(ctxt, obj);
                xmlXPathStringFunction(ctxt, 1);
                obj = valuePop(ctxt);
            }
            return obj;
        }

        static void getByName(xmlXPathParserContextPtr ctxt, int nargs)
        {
            xsltTransformContextPtr tctxt;
            void *data;
            if (nargs != 1) {
                xsltGenericError(xsltGenericErrorContext,
                    "getByName: requires exactly 1 argument\n");
                return;
            }

            tctxt = xsltXPathGetTransformContext(ctxt);
            if (tctxt == NULL) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get the transformation context\n");
                return;
            }
            // XXX: someone with better knowledge of libxslt might come up with a better
            // idea to pass the OleHandler than by attaching it to tctxt->_private
            data = tctxt->_private;
            if (data == NULL) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get module data\n");
                return;
            }
            OleHandler * oh = static_cast<OleHandler*> (data);
            xmlXPathObjectPtr streamName = valuePop(ctxt);
            streamName = ensureStringValue(streamName, ctxt);
            const OString content = oh->getByName(OUString::createFromAscii((sal_Char*) streamName->stringval));
            valuePush(ctxt, xmlXPathNewCString(content.getStr()));
            xmlXPathFreeObject(streamName);
        }
    };

    Reader::Reader(LibXSLTTransformer* transformer) :
        Thread("LibXSLTTransformer"), m_transformer(transformer),
        m_readBuf(INPUT_BUFFER_SIZE), m_writeBuf(OUTPUT_BUFFER_SIZE)
    {
        LIBXML_TEST_VERSION;
    }
    ;

    int
    Reader::read(char * buffer, int len)
    {
        //        const char *ptr = (const char *) context;
        if (buffer == NULL || len < 0)
            return (-1);
        sal_Int32 n;
        css::uno::Reference<XInputStream> xis = this->m_transformer->getInputStream();
        n = xis.get()->readBytes(m_readBuf, len);
        if (n > 0)
            {
                memcpy(buffer, m_readBuf.getArray(), n);
            }
        return n;
    }

    int
    Reader::write(const char * buffer, int len)
    {
        if (buffer == NULL || len < 0)
            return -1;
        if (len > 0)
            {
                css::uno::Reference<XOutputStream> xos = m_transformer->getOutputStream();
                sal_Int32 writeLen = len;
                sal_Int32 bufLen = ::std::min(writeLen,
                        this->OUTPUT_BUFFER_SIZE);
                const sal_uInt8* memPtr =
                        reinterpret_cast<const sal_uInt8*> (buffer);
                while (writeLen > 0)
                    {
                        sal_Int32 n = ::std::min(writeLen, bufLen);
                        m_writeBuf.realloc(n);
                        memcpy(m_writeBuf.getArray(), memPtr,
                                static_cast<size_t> (n));
                        xos.get()->writeBytes(m_writeBuf);
                        memPtr += n;
                        writeLen -= n;
                    }
            }
        return len;
    }

    int
    Reader::closeInput()
    {
        return 0;
    }

    int
    Reader::closeOutput()
    {
        css::uno::Reference<XOutputStream> xos = m_transformer->getOutputStream();
        if (xos.is())
            {
                xos.get()->flush();
                xos.get()->closeOutput();
            }
        m_transformer->done();
        return 0;
    }

    void
    Reader::execute()
    {
        OSL_ASSERT(m_transformer != NULL);
        OSL_ASSERT(m_transformer->getInputStream().is());
        OSL_ASSERT(m_transformer->getOutputStream().is());
        OSL_ASSERT(!m_transformer->getStyleSheetURL().isEmpty());
        ::std::map<const char*, OString>::iterator pit;
        ::std::map<const char*, OString> pmap = m_transformer->getParameters();
        ::std::vector< const char* > params( pmap.size() * 2 + 1 ); // build parameters
        int paramIndex = 0;
        for (pit = pmap.begin(); pit != pmap.end(); ++pit)
        {
            params[paramIndex++] = (*pit).first;
            params[paramIndex++] = (*pit).second.getStr();
        }
        params[paramIndex] = NULL;
        xmlDocPtr doc = xmlReadIO(&ParserInputBufferCallback::on_read,
                &ParserInputBufferCallback::on_close,
                static_cast<void*> (this), NULL, NULL, 0);
        xsltStylesheetPtr styleSheet = xsltParseStylesheetFile(
                (const xmlChar *) m_transformer->getStyleSheetURL().getStr());
        xmlDocPtr result = NULL;
        xsltTransformContextPtr tcontext = NULL;
        exsltRegisterAll();
        registerExtensionModule();
#if OSL_DEBUG_LEVEL > 1
        xsltSetGenericDebugFunc(stderr, NULL);
        xsltDebugDumpExtensions(NULL);
#endif
        OleHandler* oh = new OleHandler(m_transformer->getComponentContext());
        if (styleSheet)
            {
                tcontext = xsltNewTransformContext(styleSheet, doc);
                tcontext->_private = static_cast<void *> (oh);
                xsltQuoteUserParams(tcontext, &params[0]);
                result = xsltApplyStylesheetUser(styleSheet, doc, 0, 0, 0,
                        tcontext);
            }

        if (result)
            {
                xmlCharEncodingHandlerPtr encoder = xmlGetCharEncodingHandler(
                        XML_CHAR_ENCODING_UTF8);
                xmlOutputBufferPtr outBuf = xmlAllocOutputBuffer(encoder);
                outBuf->context = static_cast<void *> (this);
                outBuf->writecallback = &ParserOutputBufferCallback::on_write;
                outBuf->closecallback = &ParserOutputBufferCallback::on_close;
                xsltSaveResultTo(outBuf, result, styleSheet);
            }
        else
            {
                xmlErrorPtr lastErr = xmlGetLastError();
                OUString msg;
                if (lastErr)
                    msg = OUString::createFromAscii(lastErr->message);
                else
                    msg = OUString::createFromAscii(
                            "Unknown XSLT transformation error");

                m_transformer->error(msg);
            }
        closeOutput();
        delete(oh);
        xsltFreeStylesheet(styleSheet);
        xsltFreeTransformContext(tcontext);
        xmlFreeDoc(doc);
        xmlFreeDoc(result);
    }

    void
    Reader::registerExtensionModule()
    {
        const xmlChar* oleModuleURI = (const xmlChar *) EXT_MODULE_OLE_URI;
        xsltRegisterExtModule(oleModuleURI, &ExtFuncOleCB::init, NULL);
        xsltRegisterExtModuleFunction(
                                 (const xmlChar*) "insertByName",
                                 oleModuleURI,
                                 &ExtFuncOleCB::insertByName);
        xsltRegisterExtModuleFunction(
                                (const xmlChar*) "getByName",
                                oleModuleURI,
                                 &ExtFuncOleCB::getByName);

    }

    Reader::~Reader()
    {
    }

    LibXSLTTransformer::LibXSLTTransformer(
            const css::uno::Reference<XComponentContext> & rxContext) :
        m_xContext(rxContext)
    {
    }

    void
    LibXSLTTransformer::setInputStream(
            const css::uno::Reference<XInputStream>& inputStream)
            throw (RuntimeException)
    {
        m_rInputStream = inputStream;
    }

    css::uno::Reference<XInputStream>
    LibXSLTTransformer::getInputStream() throw (RuntimeException)
    {
        return m_rInputStream;
    }

    void
    LibXSLTTransformer::setOutputStream(
            const css::uno::Reference<XOutputStream>& outputStream)
            throw (RuntimeException)
    {
        m_rOutputStream = outputStream;
    }

    css::uno::Reference<XOutputStream>
    LibXSLTTransformer::getOutputStream() throw (RuntimeException)
    {
        return m_rOutputStream;
    }

    void
    LibXSLTTransformer::addListener(const css::uno::Reference<XStreamListener>& listener)
            throw (RuntimeException)
    {
        m_listeners.insert(m_listeners.begin(), listener);
    }

    void
    LibXSLTTransformer::removeListener(
            const css::uno::Reference<XStreamListener>& listener)
            throw (RuntimeException)
    {
        m_listeners.remove(listener);
    }

    void
    LibXSLTTransformer::start() throw (RuntimeException)
    {
        ListenerList::iterator it;
        ListenerList* l = &m_listeners;
        for (it = l->begin(); it != l->end(); ++it)
            {
                css::uno::Reference<XStreamListener> xl = *it;
                xl.get()->started();
            }
        OSL_ENSURE(!m_Reader.is(), "Somebody forgot to call terminate *and* holds a reference to this LibXSLTTransformer instance");
        m_Reader = new Reader(this);
        m_Reader->launch();
    }

    void
    LibXSLTTransformer::error(const OUString& msg)
    {
        ListenerList* l = &m_listeners;
        Any arg;
        arg <<= Exception(msg, *this);
        for (ListenerList::iterator it = l->begin(); it != l->end(); ++it)
            {
                css::uno::Reference<XStreamListener> xl = *it;
                if (xl.is())
                    {
                        xl.get()->error(arg);
                    }
            }
    }

    void
    LibXSLTTransformer::done()
    {
        ListenerList* l = &m_listeners;
        for (ListenerList::iterator it = l->begin(); it != l->end(); ++it)
            {
                css::uno::Reference<XStreamListener> xl = *it;
                if (xl.is())
                    {
                        xl.get()->closed();
                    }
            }
    }

    void
    LibXSLTTransformer::terminate() throw (RuntimeException)
    {
        if (m_Reader.is())
        {
            m_Reader->terminate();
            m_Reader->join();
        }
        m_Reader.clear();
        m_parameters.clear();
    }

    void
    LibXSLTTransformer::initialize(const Sequence<Any>& args)
            throw (RuntimeException)
    {
        Sequence<Any> params;
        args[0] >>= params;
        xmlSubstituteEntitiesDefault(0);
        m_parameters.clear();
        for (int i = 0; i < params.getLength(); i++)
            {
                NamedValue nv;
                params[i] >>= nv;
                OString nameUTF8 = OUStringToOString(nv.Name,
                        RTL_TEXTENCODING_UTF8);
                OUString value;
                OString valueUTF8;
                if (nv.Value >>= value)
                    {
                        valueUTF8 = OUStringToOString(value,
                                RTL_TEXTENCODING_UTF8);
                    }
                else
                    {
                        // ignore non-string parameters
                        continue;
                    }
                if (nameUTF8.equals("StylesheetURL"))
                    {
                        m_styleSheetURL = valueUTF8;
                    }
                else if (nameUTF8.equals("SourceURL"))
                    {
                        m_parameters.insert(pair<const char*, OString> (
                                PARAM_SOURCE_URL, valueUTF8));
                    }
                else if (nameUTF8.equals("SourceBaseURL"))
                    {
                        m_parameters.insert(pair<const char*, OString> (
                                PARAM_SOURCE_BASE_URL, valueUTF8));
                    }
                else if (nameUTF8.equals("TargetURL"))
                    {
                        m_parameters.insert(pair<const char*, OString> (
                                PARAM_TARGET_URL, valueUTF8));
                    }
                else if (nameUTF8.equals("TargetBaseURL"))
                    {
                        m_parameters.insert(pair<const char*, OString> (
                                PARAM_TARGET_BASE_URL, valueUTF8));
                    }
                else if (nameUTF8.equals("DoctypePublic"))
                    {
                        m_parameters.insert(pair<const char*, OString> (
                                PARAM_DOCTYPE_PUBLIC, valueUTF8));
                    }
            }
    }

    const OString
    LibXSLTTransformer::getStyleSheetURL()
    {
        return m_styleSheetURL;
    }

    ::std::map<const char*, OString>
    LibXSLTTransformer::getParameters()
    {
        return m_parameters;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

