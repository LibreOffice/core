/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Peter Jentsch <pjotr@guineapics.de> ]
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Peter Jentsch <pjotr@guineapics.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <cstdio>
#include <cstring>
#include <list>
#include <map>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/variables.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
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
    const char* const LibXSLTTransformer::PARAM_DOCTYPE_SYSTEM = "sytemType";
    const char* const LibXSLTTransformer::PARAM_DOCTYPE_PUBLIC = "publicType";

    const sal_Int32 Reader::OUTPUT_BUFFER_SIZE = _OUTPUT_BUFFER_SIZE;

    const sal_Int32 Reader::INPUT_BUFFER_SIZE = _INPUT_BUFFER_SIZE;

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

    Reader::Reader(LibXSLTTransformer* transformer) :
        m_transformer(transformer), m_terminated(false), m_readBuf(
                INPUT_BUFFER_SIZE), m_writeBuf(OUTPUT_BUFFER_SIZE)
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
        Reference<XInputStream> xis = this->m_transformer->getInputStream();
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
                Reference<XOutputStream> xos = m_transformer->getOutputStream();
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
        Reference<XOutputStream> xos = m_transformer->getOutputStream();
        if (xos.is())
            {
                xos.get()->flush();
                xos.get()->closeOutput();
            }
        m_transformer->done();
        return 0;
    }

    void
    Reader::run()
    {
        OSL_ASSERT(m_transformer != NULL);
        OSL_ASSERT(m_transformer->getInputStream().is());
        OSL_ASSERT(m_transformer->getOutputStream().is());
        OSL_ASSERT(m_transformer->getStyleSheetURL());
        ::std::map<const char*, OString>::iterator pit;
        ::std::map<const char*, OString> pmap = m_transformer->getParameters();
        const char* params[pmap.size() * 2 + 1]; // build parameters
        int paramIndex = 0;
        for (pit = pmap.begin(); pit != pmap.end(); pit++)
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
        if (styleSheet)
            {
                tcontext = xsltNewTransformContext(styleSheet, doc);
                xsltQuoteUserParams(tcontext, params);
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
        xsltFreeStylesheet(styleSheet);
        xsltFreeTransformContext(tcontext);
        xmlFreeDoc(doc);
        xmlFreeDoc(result);
    }
    ;

    void
    Reader::onTerminated()
    {
        m_terminated = true;
    }
    ;

    Reader::~Reader()
    {
    }

    LibXSLTTransformer::LibXSLTTransformer(
            const Reference<XMultiServiceFactory> &r) :
        m_rServiceFactory(r)
    {
    }

    void
    LibXSLTTransformer::setInputStream(
            const Reference<XInputStream>& inputStream)
            throw (RuntimeException)
    {
        m_rInputStream = inputStream;
    }

    Reference<XInputStream>
    LibXSLTTransformer::getInputStream() throw (RuntimeException)
    {
        return m_rInputStream;
    }

    void
    LibXSLTTransformer::setOutputStream(
            const Reference<XOutputStream>& outputStream)
            throw (RuntimeException)
    {
        m_rOutputStream = outputStream;
    }

    Reference<XOutputStream>
    LibXSLTTransformer::getOutputStream() throw (RuntimeException)
    {
        return m_rOutputStream;
    }

    void
    LibXSLTTransformer::addListener(const Reference<XStreamListener>& listener)
            throw (RuntimeException)
    {
        m_listeners.insert(m_listeners.begin(), listener);
    }

    void
    LibXSLTTransformer::removeListener(
            const Reference<XStreamListener>& listener)
            throw (RuntimeException)
    {
        m_listeners.remove(listener);
    }

    void
    LibXSLTTransformer::start() throw (RuntimeException)
    {
        ListenerList::iterator it;
        ListenerList* l = &m_listeners;
        for (it = l->begin(); it != l->end(); it++)
            {
                Reference<XStreamListener> xl = *it;
                xl.get()->started();
            }
        Reader* r = new Reader(this);
        r->create();
    }

    void
    LibXSLTTransformer::error(const OUString& msg)
    {
        ListenerList* l = &m_listeners;
        Any arg;
        arg <<= Exception(msg, *this);
        for (ListenerList::iterator it = l->begin(); it != l->end(); it++)
            {
                Reference<XStreamListener> xl = *it;
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
        for (ListenerList::iterator it = l->begin(); it != l->end(); it++)
            {
                Reference<XStreamListener> xl = *it;
                if (xl.is())
                    {
                        xl.get()->closed();
                    }
            }
    }

    void
    LibXSLTTransformer::terminate() throw (RuntimeException)
    {
        m_parameters.clear();
    }

    void
    LibXSLTTransformer::initialize(const Sequence<Any>& params)
            throw (RuntimeException)
    {
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
                else if (nameUTF8.equals("DoctypeSystem"))
                    {
                        m_parameters.insert(pair<const char*, OString> (
                                PARAM_DOCTYPE_SYSTEM, valueUTF8));
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

