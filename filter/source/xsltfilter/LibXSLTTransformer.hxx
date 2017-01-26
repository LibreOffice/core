/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FILTER_SOURCE_XSLTFILTER_LIBXSLTTRANSFORMER_HXX
#define INCLUDED_FILTER_SOURCE_XSLTFILTER_LIBXSLTTRANSFORMER_HXX

#include <list>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>
#include <libxml/xpathInternals.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>

#include <rtl/ref.hxx>

#include <salhelper/thread.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/xslt/XXSLTTransformer.hpp>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::std::list;
using ::std::map;

#define EXT_MODULE_OLE_URI "http://libreoffice.org/2011/xslt/ole"

namespace XSLT
{

    class LibXSLTTransformer;

    /*
     * Reader provides a worker thread to perform the actual transformation.
     * It pipes the streams provided by a LibXSLTTransformer
     * instance through libxslt.
     */
    class Reader : public salhelper::Thread
    {
    public:
        Reader(LibXSLTTransformer* transformer);
        int SAL_CALL read(char * buffer, int len);
        int SAL_CALL write(const char * buffer, int len);
        void forceStateStopped();
        int SAL_CALL closeOutput();

    private:
        virtual ~Reader() override;

        static const sal_Int32 OUTPUT_BUFFER_SIZE;
        static const sal_Int32 INPUT_BUFFER_SIZE;
        LibXSLTTransformer* m_transformer;
        Sequence<sal_Int8> m_readBuf;
        Sequence<sal_Int8> m_writeBuf;
        xsltTransformContextPtr m_tcontext;

        virtual void execute() override;
        static void SAL_CALL registerExtensionModule();
    };

    /*
     * LibXSLTTransformer provides an transforming pipe service to XSLTFilter.
     *
     * It implements XActiveDataSource, XActiveDataSink and XActiveDataControl
     * to consume data. It also notifies upstream of important events such as
     * begin and end of the transformation and of any errors that occur during
     * transformation.
     *
     * TODO: Error reporting leaves room for improvement, currently.
     *
     * The actual transformation is done by a worker thread.
     *
     * See Reader below.
     */
    class LibXSLTTransformer : public WeakImplHelper<css::xml::xslt::XXSLTTransformer>
    {
    private:
        static const char* const PARAM_SOURCE_URL;
        static const char* const PARAM_SOURCE_BASE_URL;
        static const char* const PARAM_TARGET_URL;
        static const char* const PARAM_TARGET_BASE_URL;
        static const char* const PARAM_DOCTYPE_PUBLIC;

        // the UNO ServiceFactory
        css::uno::Reference<css::uno::XComponentContext> m_xContext;

        css::uno::Reference<XInputStream> m_rInputStream;

        css::uno::Reference<XOutputStream> m_rOutputStream;

        typedef ::std::list<css::uno::Reference<XStreamListener> > ListenerList;

        ListenerList m_listeners;

        OString m_styleSheetURL;

        ::std::map<const char *, OString> m_parameters;

        rtl::Reference<Reader> m_Reader;

    protected:
        virtual ~LibXSLTTransformer() override {
            if (m_Reader.is()) {
                m_Reader->terminate();
                m_Reader->forceStateStopped();
                m_Reader->join();
            }
        }

    public:

        // ctor...
        LibXSLTTransformer(const css::uno::Reference<css::uno::XComponentContext> &r);

        // XActiveDataSink
        virtual void SAL_CALL
        setInputStream(const css::uno::Reference<XInputStream>& inputStream) override;
        virtual css::uno::Reference<XInputStream> SAL_CALL
        getInputStream() override;
        // XActiveDataSource
        virtual void SAL_CALL
        setOutputStream(const css::uno::Reference<XOutputStream>& outputStream) override;
        virtual css::uno::Reference<XOutputStream> SAL_CALL
        getOutputStream() override;
        // XActiveDataControl
        virtual void SAL_CALL
        addListener(const css::uno::Reference<XStreamListener>& listener) override;
        virtual void SAL_CALL
        removeListener(const css::uno::Reference<XStreamListener>& listener) override;
        virtual void SAL_CALL
        start() override;
        virtual void SAL_CALL
        terminate() override;
        virtual void SAL_CALL
        initialize(const Sequence<Any>& params) override;

        void SAL_CALL
        done();

        void SAL_CALL
        error(const OUString& msg);

        const OString& SAL_CALL
        getStyleSheetURL() { return m_styleSheetURL; }

        const ::std::map<const char*, OString>& SAL_CALL
        getParameters() { return m_parameters; }

        const css::uno::Reference<css::uno::XComponentContext>& SAL_CALL
        getComponentContext() {
            return m_xContext;
        }

    };
}

#endif // INCLUDED_FILTER_SOURCE_XSLTFILTER_LIBXSLTTRANSFORMER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
