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
#ifndef __LIBXSLTTRANSFORMER_HXX__
#define __LIBXSLTTRANSFORMER_HXX__
#include "precompiled_filter.hxx"

#include <stdio.h>

#include <list>
#include <map>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase.hxx>

#include <osl/thread.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/NamedValue.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::std::list;
using ::std::map;

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
    class LibXSLTTransformer : public WeakImplHelper4<XActiveDataSink,
            XActiveDataSource, XActiveDataControl, XInitialization>
    {
    private:
        static const char* const PARAM_SOURCE_URL;
        static const char* const PARAM_SOURCE_BASE_URL;
        static const char* const PARAM_TARGET_URL;
        static const char* const PARAM_TARGET_BASE_URL;
        static const char* const PARAM_DOCTYPE_SYSTEM;
        static const char* const PARAM_DOCTYPE_PUBLIC;

        // the UNO ServiceFactory
        Reference<XMultiServiceFactory> m_rServiceFactory;

        Reference<XInputStream> m_rInputStream;

        Reference<XOutputStream> m_rOutputStream;

        typedef ::std::list<Reference<XStreamListener> > ListenerList;

        ListenerList m_listeners;

        OString m_styleSheetURL;

        ::std::map<const char *, OString> m_parameters;

    public:

        // ctor...
        LibXSLTTransformer(const Reference<XMultiServiceFactory> &r);

        // XActiveDataSink
        virtual void SAL_CALL
        setInputStream(const Reference<XInputStream>& inputStream)
                throw (RuntimeException);
        virtual Reference<XInputStream> SAL_CALL
        getInputStream() throw (RuntimeException);
        // XActiveDataSource
        virtual void SAL_CALL
        setOutputStream(const Reference<XOutputStream>& outputStream)
                throw (RuntimeException);
        virtual Reference<XOutputStream> SAL_CALL
        getOutputStream() throw (RuntimeException);
        // XActiveDataControl
        virtual void SAL_CALL
        addListener(const Reference<XStreamListener>& listener)
                throw (RuntimeException);
        virtual void SAL_CALL
        removeListener(const Reference<XStreamListener>& listener)
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

    };

    /*
     * Reader provides a worker thread to perform the actual transformation.
     * It pipes the streams provided by a LibXSLTTransformer
     * instance through libxslt.
     */
    class Reader : public osl::Thread
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

    protected:
        virtual
        ~Reader();

    private:
        static const sal_Int32 OUTPUT_BUFFER_SIZE;
        static const sal_Int32 INPUT_BUFFER_SIZE;
        LibXSLTTransformer* m_transformer;
        sal_Bool m_terminated;
        Sequence<sal_Int8> m_readBuf;
        Sequence<sal_Int8> m_writeBuf;

        virtual void SAL_CALL
        run();
        virtual void SAL_CALL
        onTerminated();
    };

}
;

#endif // __LIBXSLTTRANSFORMER_HXX__
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
