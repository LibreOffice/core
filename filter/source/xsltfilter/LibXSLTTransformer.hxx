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

#ifndef __LIBXSLTTRANSFORMER_HXX__
#define __LIBXSLTTRANSFORMER_HXX__

#include <stdio.h>

#include <list>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>
#include <libxml/xpathInternals.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase.hxx>

#include <rtl/ref.hxx>

#include <salhelper/thread.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/xslt/XXSLTTransformer.hpp>

using namespace ::rtl;
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
    class LibXSLTTransformer : public WeakImplHelper1<com::sun::star::xml::xslt::XXSLTTransformer>
    {
    private:
        static const char* const PARAM_SOURCE_URL;
        static const char* const PARAM_SOURCE_BASE_URL;
        static const char* const PARAM_TARGET_URL;
        static const char* const PARAM_TARGET_BASE_URL;
        static const char* const PARAM_DOCTYPE_SYSTEM;
        static const char* const PARAM_DOCTYPE_PUBLIC;

        // the UNO ServiceFactory
        com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> m_rServiceFactory;

        com::sun::star::uno::Reference<XInputStream> m_rInputStream;

        com::sun::star::uno::Reference<XOutputStream> m_rOutputStream;

        typedef ::std::list<com::sun::star::uno::Reference<XStreamListener> > ListenerList;

        ListenerList m_listeners;

        OString m_styleSheetURL;

        ::std::map<const char *, OString> m_parameters;

        rtl::Reference< salhelper::Thread > m_Reader;

    protected:
        virtual ~LibXSLTTransformer() {
            if (m_Reader.is()) {
                    m_Reader->terminate();
                    m_Reader->join();
            }
        }

    public:

        // ctor...
        LibXSLTTransformer(const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> &r);

        // XActiveDataSink
        virtual void SAL_CALL
        setInputStream(const com::sun::star::uno::Reference<XInputStream>& inputStream)
                throw (RuntimeException);
        virtual com::sun::star::uno::Reference<XInputStream> SAL_CALL
        getInputStream() throw (RuntimeException);
        // XActiveDataSource
        virtual void SAL_CALL
        setOutputStream(const com::sun::star::uno::Reference<XOutputStream>& outputStream)
                throw (RuntimeException);
        virtual com::sun::star::uno::Reference<XOutputStream> SAL_CALL
        getOutputStream() throw (RuntimeException);
        // XActiveDataControl
        virtual void SAL_CALL
        addListener(const com::sun::star::uno::Reference<XStreamListener>& listener)
                throw (RuntimeException);
        virtual void SAL_CALL
        removeListener(const com::sun::star::uno::Reference<XStreamListener>& listener)
                throw (RuntimeException);
        virtual void SAL_CALL
        start() throw (RuntimeException);
        virtual void SAL_CALL
        terminate() throw (RuntimeException);
        virtual void SAL_CALL
        initialize(const Sequence<Any>& params) throw (RuntimeException);

        void SAL_CALL
        done();

        void SAL_CALL
        error(const OUString& msg);

        const OString SAL_CALL
        getStyleSheetURL();

        ::std::map<const char*, OString> SAL_CALL
        getParameters();

        virtual com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory> SAL_CALL
        getServiceFactory() {
            return m_rServiceFactory;
        }

    };

    /*
     * Reader provides a worker thread to perform the actual transformation.
     * It pipes the streams provided by a LibXSLTTransformer
     * instance through libxslt.
     */
    class Reader : public salhelper::Thread
    {
    public:
        Reader(LibXSLTTransformer* transformer);
        int SAL_CALL
        read(char * buffer, int len);
        int SAL_CALL
        write(const char * buffer, int len);
        int SAL_CALL
        closeInput();
        int SAL_CALL
        closeOutput();

    private:
        virtual
        ~Reader();

        static const sal_Int32 OUTPUT_BUFFER_SIZE;
        static const sal_Int32 INPUT_BUFFER_SIZE;
        LibXSLTTransformer* m_transformer;
        Sequence<sal_Int8> m_readBuf;
        Sequence<sal_Int8> m_writeBuf;

        virtual void
        execute();
        void SAL_CALL
        registerExtensionModule();
    };

}
;

#endif // __LIBXSLTTRANSFORMER_HXX__
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
