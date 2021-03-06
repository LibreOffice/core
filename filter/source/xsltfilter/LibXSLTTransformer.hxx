/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <deque>
#include <map>
#include <mutex>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>
#include <libxml/xpathInternals.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>

#include <rtl/ref.hxx>

#include <salhelper/thread.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
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
        int read(char * buffer, int len);
        int write(const char * buffer, int len);
        void forceStateStopped();
        void closeOutput();

    private:
        virtual ~Reader() override;

        static const sal_Int32 OUTPUT_BUFFER_SIZE;
        static const sal_Int32 INPUT_BUFFER_SIZE;
        LibXSLTTransformer* m_transformer;
        Sequence<sal_Int8> m_readBuf;
        Sequence<sal_Int8> m_writeBuf;

        std::mutex m_mutex;
        xsltTransformContextPtr m_tcontext;

        virtual void execute() override;
        static void registerExtensionModule();
    };

    /*
     * LibXSLTTransformer provides a transforming pipe service to XSLTFilter.
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
    class LibXSLTTransformer : public WeakImplHelper<css::xml::xslt::XXSLTTransformer, css::lang::XServiceInfo>
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

        typedef ::std::deque<css::uno::Reference<XStreamListener> > ListenerList;

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

        //  XServiceInfo
        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

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

        void
        done();

        void
        error(const OUString& msg);

        const OString&
        getStyleSheetURL() const { return m_styleSheetURL; }

        const ::std::map<const char*, OString>&
        getParameters() const { return m_parameters; }

        const css::uno::Reference<css::uno::XComponentContext>&
        getComponentContext() const {
            return m_xContext;
        }

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
