/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <utility>
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

#include "LibXSLTTransformer.hxx"
#include "OleHandler.hxx"
#include <memory>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using ::std::pair;

namespace XSLT
{
    const char* const LibXSLTTransformer::PARAM_SOURCE_URL = "sourceURL";
    const char* const LibXSLTTransformer::PARAM_SOURCE_BASE_URL =
            "sourceBaseURL";
    const char* const LibXSLTTransformer::PARAM_TARGET_URL = "targetURL";
    const char* const LibXSLTTransformer::PARAM_TARGET_BASE_URL =
            "targetBaseURL";
    const char* const LibXSLTTransformer::PARAM_DOCTYPE_PUBLIC = "publicType";

    const sal_Int32 Reader::OUTPUT_BUFFER_SIZE = 4096;

    const sal_Int32 Reader::INPUT_BUFFER_SIZE = 4096;

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
        on_close(void * )
        {
            return 0;
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
            tmp->closeOutput();
            return 0;
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
            return nullptr;
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
            if (tctxt == nullptr) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get the transformation context\n");
                return;
            }
            // XXX: someone with better knowledge of libxslt might come up with a better
            // idea to pass the OleHandler than by attaching it to tctxt->_private. See also
            // below.
            data = tctxt->_private;
            if (data == nullptr) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get module data\n");
                return;
            }
            OleHandler * oh = static_cast<OleHandler*> (data);

            xmlXPathObjectPtr value = valuePop(ctxt);
            value = ensureStringValue(value, ctxt);
            xmlXPathObjectPtr streamName = valuePop(ctxt);
            streamName = ensureStringValue(streamName, ctxt);

            oh->insertByName(OStringToOUString(reinterpret_cast<char*>(streamName->stringval), RTL_TEXTENCODING_UTF8),
                             OString(reinterpret_cast<char*>(value->stringval)));
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
            if (tctxt == nullptr) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get the transformation context\n");
                return;
            }
            // XXX: someone with better knowledge of libxslt might come up with a better
            // idea to pass the OleHandler than by attaching it to tctxt->_private
            data = tctxt->_private;
            if (data == nullptr) {
                xsltGenericError(xsltGenericErrorContext,
                    "xsltExtFunctionTest: failed to get module data\n");
                return;
            }
            OleHandler * oh = static_cast<OleHandler*> (data);
            xmlXPathObjectPtr streamName = valuePop(ctxt);
            streamName = ensureStringValue(streamName, ctxt);
            const OString content = oh->getByName(OStringToOUString(reinterpret_cast<char*>(streamName->stringval), RTL_TEXTENCODING_UTF8));
            valuePush(ctxt, xmlXPathNewCString(content.getStr()));
            xmlXPathFreeObject(streamName);
        }
    };

    Reader::Reader(LibXSLTTransformer* transformer) :
        Thread("LibXSLTTransformer"), m_transformer(transformer),
        m_readBuf(INPUT_BUFFER_SIZE), m_writeBuf(OUTPUT_BUFFER_SIZE),
        m_tcontext(nullptr)
    {
        LIBXML_TEST_VERSION;
    }
    ;

    int
    Reader::read(char * buffer, int len)
    {
        //        const char *ptr = (const char *) context;
        if (buffer == nullptr || len < 0)
            return -1;
        sal_Int32 n;
        css::uno::Reference<XInputStream> xis = m_transformer->getInputStream();
        n = xis->readBytes(m_readBuf, len);
        if (n > 0)
            {
                memcpy(buffer, m_readBuf.getArray(), n);
            }
        return n;
    }

    int
    Reader::write(const char * buffer, int len)
    {
        if (buffer == nullptr || len < 0)
            return -1;
        if (len > 0)
        {
            css::uno::Reference<XOutputStream> xos = m_transformer->getOutputStream();
            sal_Int32 writeLen = len;
            sal_Int32 bufLen = ::std::min(writeLen, OUTPUT_BUFFER_SIZE);
            const sal_uInt8* memPtr =
                    reinterpret_cast<const sal_uInt8*> (buffer);
            while (writeLen > 0)
            {
                sal_Int32 n = ::std::min(writeLen, bufLen);
                m_writeBuf.realloc(n);
                memcpy(m_writeBuf.getArray(), memPtr,
                        static_cast<size_t> (n));
                xos->writeBytes(m_writeBuf);
                memPtr += n;
                writeLen -= n;
            }
        }
        return len;
    }

    void
    Reader::closeOutput()
    {
        css::uno::Reference<XOutputStream> xos = m_transformer->getOutputStream();
        if (xos.is())
        {
            xos->flush();
            xos->closeOutput();
        }
        m_transformer->done();
    }

    void
    Reader::execute()
    {
        OSL_ASSERT(m_transformer != nullptr);
        OSL_ASSERT(m_transformer->getInputStream().is());
        OSL_ASSERT(m_transformer->getOutputStream().is());
        OSL_ASSERT(!m_transformer->getStyleSheetURL().isEmpty());
        ::std::map<const char*, OString> pmap = m_transformer->getParameters();
        ::std::vector< const char* > params( pmap.size() * 2 + 1 ); // build parameters
        int paramIndex = 0;
        for (auto const& elem : pmap)
        {
            params[paramIndex++] = elem.first;
            params[paramIndex++] = elem.second.getStr();
        }
        params[paramIndex] = nullptr;
        xmlDocPtr doc = xmlReadIO(&ParserInputBufferCallback::on_read,
                &ParserInputBufferCallback::on_close,
                static_cast<void*> (this), nullptr, nullptr, 0);
        xsltStylesheetPtr styleSheet = xsltParseStylesheetFile(
                reinterpret_cast<const xmlChar *>(m_transformer->getStyleSheetURL().getStr()));
        xmlDocPtr result = nullptr;
        exsltRegisterAll();
        registerExtensionModule();
#ifdef DEBUG_FILTER_LIBXSLTTRANSFORMER
        xsltSetGenericDebugFunc(stderr, NULL);
        xsltDebugDumpExtensions(NULL);
#endif
        std::unique_ptr<OleHandler> oh(new OleHandler(m_transformer->getComponentContext()));
        if (styleSheet)
        {
            xsltTransformContextPtr tcontext = xsltNewTransformContext(
                styleSheet, doc);
            {
                std::scoped_lock<std::mutex> g(m_mutex);
                m_tcontext = tcontext;
            }
            oh->registercontext(m_tcontext);
            xsltQuoteUserParams(m_tcontext, params.data());
            result = xsltApplyStylesheetUser(styleSheet, doc, nullptr, nullptr, nullptr,
                                             m_tcontext);
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
            (void)xmlOutputBufferClose(outBuf);
        }
        else
        {
            const xmlError* lastErr = xmlGetLastError();
            OUString msg;
            if (lastErr)
                msg = OStringToOUString(lastErr->message, RTL_TEXTENCODING_UTF8);
            else
                msg = "Unknown XSLT transformation error";

            m_transformer->error(msg);
        }
        closeOutput();
        oh.reset();
        xsltFreeStylesheet(styleSheet);
        xsltTransformContextPtr tcontext = nullptr;
        {
            std::scoped_lock<std::mutex> g(m_mutex);
            std::swap(m_tcontext, tcontext);
        }
        xsltFreeTransformContext(tcontext);
        xmlFreeDoc(doc);
        xmlFreeDoc(result);
    }

    void
    Reader::registerExtensionModule()
    {
        const xmlChar* oleModuleURI = reinterpret_cast<const xmlChar *>(EXT_MODULE_OLE_URI);
        xsltRegisterExtModule(oleModuleURI, &ExtFuncOleCB::init, nullptr);
        xsltRegisterExtModuleFunction(
                                 reinterpret_cast<const xmlChar*>("insertByName"),
                                 oleModuleURI,
                                 &ExtFuncOleCB::insertByName);
        xsltRegisterExtModuleFunction(
                                reinterpret_cast<const xmlChar*>("getByName"),
                                oleModuleURI,
                                 &ExtFuncOleCB::getByName);

    }

    void Reader::forceStateStopped()
    {
        std::scoped_lock<std::mutex> g(m_mutex);
        if (!m_tcontext)
            return;
        //tdf#100057 If we force a cancel, libxslt will of course just keep on going unless something
        //tells it to stop. Here we force the stopped state so that libxslt will stop processing
        //and so Reader::execute will complete and we can join cleanly
        m_tcontext->state = XSLT_STATE_STOPPED;
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
    {
        m_rInputStream = inputStream;
    }

    css::uno::Reference<XInputStream>
    LibXSLTTransformer::getInputStream()
    {
        return m_rInputStream;
    }

    void
    LibXSLTTransformer::setOutputStream(
            const css::uno::Reference<XOutputStream>& outputStream)
    {
        m_rOutputStream = outputStream;
    }

    css::uno::Reference<XOutputStream>
    LibXSLTTransformer::getOutputStream()
    {
        return m_rOutputStream;
    }

    void
    LibXSLTTransformer::addListener(const css::uno::Reference<XStreamListener>& listener)
    {
        m_listeners.push_front(listener);
    }

    void
    LibXSLTTransformer::removeListener(
            const css::uno::Reference<XStreamListener>& listener)
    {
        m_listeners.erase( std::remove(m_listeners.begin(), m_listeners.end(), listener ), m_listeners.end() );
    }

    void
    LibXSLTTransformer::start()
    {
        for (const css::uno::Reference<XStreamListener>& xl : m_listeners)
        {
            xl->started();
        }
        OSL_ENSURE(!m_Reader.is(), "Somebody forgot to call terminate *and* holds a reference to this LibXSLTTransformer instance");
        m_Reader = new Reader(this);
        m_Reader->launch();
    }

    void
    LibXSLTTransformer::error(const OUString& msg)
    {
        Any arg;
        arg <<= Exception(msg, *this);
        for (const css::uno::Reference<XStreamListener>& xl : m_listeners)
        {
            if (xl.is())
            {
                xl->error(arg);
            }
        }
    }

    void
    LibXSLTTransformer::done()
    {
        for (const css::uno::Reference<XStreamListener>& xl : m_listeners)
        {
            if (xl.is())
            {
                xl->closed();
            }
        }
    }

    void
    LibXSLTTransformer::terminate()
    {
        if (m_Reader.is())
        {
            m_Reader->terminate();
            m_Reader->forceStateStopped();
            m_Reader->join();
        }
        m_Reader.clear();
        m_parameters.clear();
    }

    void
    LibXSLTTransformer::initialize(const Sequence<Any>& args)
    {
        Sequence<Any> params;
        if (!(args[0] >>= params))
        {   // backward compatibility for old clients using createInstance
            params = args;
        }

SAL_WNODEPRECATED_DECLARATIONS_PUSH
        xmlSubstituteEntitiesDefault(0);
SAL_WNODEPRECATED_DECLARATIONS_POP

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
            if (nameUTF8 == "StylesheetURL")
            {
                m_styleSheetURL = valueUTF8;
            }
            else if (nameUTF8 == "SourceURL")
            {
                m_parameters.insert(pair<const char*, OString> (
                        PARAM_SOURCE_URL, valueUTF8));
            }
            else if (nameUTF8 == "SourceBaseURL")
            {
                m_parameters.insert(pair<const char*, OString> (
                        PARAM_SOURCE_BASE_URL, valueUTF8));
            }
            else if (nameUTF8 == "TargetURL")
            {
                m_parameters.insert(pair<const char*, OString> (
                        PARAM_TARGET_URL, valueUTF8));
            }
            else if (nameUTF8 == "TargetBaseURL")
            {
                m_parameters.insert(pair<const char*, OString> (
                        PARAM_TARGET_BASE_URL, valueUTF8));
            }
            else if (nameUTF8 == "DoctypePublic")
            {
                m_parameters.insert(pair<const char*, OString> (
                        PARAM_DOCTYPE_PUBLIC, valueUTF8));
            }
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

