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


#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <sax/tools/documenthandleradapter.hxx>

#include <osl/diagnose.h>
#include <osl/time.h>
#include <osl/conditn.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <rtl/ref.hxx>
#include <rtl/uri.hxx>

#include <comphelper/interaction.hxx>

#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XImportFilter2.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>

#include <com/sun/star/util/theMacroExpander.hpp>

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/xml/xslt/XSLTTransformer.hpp>
#include <utility>

#define TRANSFORMATION_TIMEOUT_SEC 60

using namespace ::cppu;
using namespace ::osl;
using namespace ::sax;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::task;

namespace XSLT
{
    namespace {

    class XSLTFilter;
    class XSLTFilterStreamListener : public WeakImplHelper<XStreamListener>
    {
    public:
        XSLTFilterStreamListener(XSLTFilter& rParent) : m_rParent(rParent) {}

        // XStreamListener
        virtual void SAL_CALL
        error(const Any& a) override;
        virtual void SAL_CALL
        closed() override;
        virtual void SAL_CALL
        terminated() override;
        virtual void SAL_CALL
        started() override;
        virtual void SAL_CALL
        disposing(const EventObject& e) override;
    private:
        XSLTFilter& m_rParent;
    };

    /*
     * XSLTFilter reads flat XML streams from the XML filter framework and passes
     * them to an XSLT transformation service. XSLT transformation errors are
     * reported to XSLTFilter.
     *
     * Currently, our transformation service is libxslt based, so it
     * only supports XSLT 1.0. There is a possibility to use XSLT 2.0
     * supporting service from an extension for a specific filter; the
     * service must support com.sun.star.xml.xslt.XSLT2Transformer.
     */
    class XSLTFilter : public WeakImplHelper<XImportFilter, XImportFilter2, XExportFilter,
            ExtendedDocumentHandlerAdapter, XServiceInfo>
    {
        friend class XSLTFilterStreamListener;
    private:

        // the UNO ServiceFactory
        css::uno::Reference<XComponentContext> m_xContext;

        // DocumentHandler interface of the css::xml::sax::Writer service
        css::uno::Reference<XOutputStream> m_rOutputStream;

        css::uno::Reference<xslt::XXSLTTransformer> m_tcontrol;

        osl::Condition m_cTransformed;
        bool m_bTerminated;
        bool m_bError;

        OUString m_aExportBaseUrl;

        OUString
        rel2abs(const OUString&);
        OUString
        expandUrl(const OUString&);

        css::uno::Reference<xslt::XXSLTTransformer> impl_createTransformer(const OUString& rTransformer, const Sequence<Any>& rArgs);

    public:

        // ctor...
        explicit XSLTFilter(css::uno::Reference<XComponentContext> x);

        //  XServiceInfo
        virtual sal_Bool SAL_CALL supportsService(const OUString& sServiceName) override;
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XImportFilter
        virtual sal_Bool SAL_CALL
        importer(const Sequence<PropertyValue>& aSourceData, const css::uno::Reference<
                XDocumentHandler>& xHandler,
                const Sequence<OUString>& msUserData) override;

        // XImportFilter2
        virtual sal_Bool SAL_CALL
        importer(const Sequence<PropertyValue>& aSourceData, const css::uno::Reference<
                XFastParser>& xFastParser,
                const Sequence<OUString>& msUserData) override;

        // XExportFilter
        virtual sal_Bool SAL_CALL
        exporter(const Sequence<PropertyValue>& aSourceData, const Sequence<
                OUString>& msUserData) override;

        // XDocumentHandler
        virtual void SAL_CALL
        startDocument() override;
        virtual void SAL_CALL
        endDocument() override;
    };

    }

    XSLTFilter::XSLTFilter(css::uno::Reference<XComponentContext> x):
        m_xContext(std::move(x)), m_bTerminated(false), m_bError(false)
    {}

    void
    XSLTFilterStreamListener::disposing(const EventObject&)
    {
    }

    //  XServiceInfo
    sal_Bool XSLTFilter::supportsService(const OUString& sServiceName)
    {
        return cppu::supportsService(this, sServiceName);
    }
    OUString XSLTFilter::getImplementationName()
    {
        return u"com.sun.star.comp.documentconversion.XSLTFilter"_ustr;
    }
    css::uno::Sequence< OUString > XSLTFilter::getSupportedServiceNames()
    {
        return { u"com.sun.star.documentconversion.XSLTFilter"_ustr };
    }

    OUString
    XSLTFilter::expandUrl(const OUString& sUrl)
    {
        OUString sPreparedURL(sUrl);
        if (sPreparedURL.startsWithIgnoreAsciiCase("vnd.sun.star.expand:", &sPreparedURL))
        {
            sPreparedURL = rtl::Uri::decode(sPreparedURL, rtl_UriDecodeWithCharset,
                                            RTL_TEXTENCODING_UTF8);
            css::uno::Reference<XMacroExpander>
                xMacroExpander = theMacroExpander::get(m_xContext);
            sPreparedURL = xMacroExpander->expandMacros(sPreparedURL);
        }
        return sPreparedURL;
    }

    css::uno::Reference<xslt::XXSLTTransformer>
    XSLTFilter::impl_createTransformer(const OUString& rTransformer, const Sequence<Any>& rArgs)
    {
        css::uno::Reference<xslt::XXSLTTransformer> xTransformer;

        // check if the filter needs XSLT-2.0-capable transformer
        // COMPATIBILITY: libreoffice 3.5/3.6 used to save the impl.
        // name of the XSLT 2.0 transformation service there, so check
        // for that too (it is sufficient to check that there is _a_
        // service name there)
        if (rTransformer.toBoolean() || rTransformer.startsWith("com.sun."))
        {
            try
            {
                xTransformer.set(
                    m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        u"com.sun.star.xml.xslt.XSLT2Transformer"_ustr, rArgs, m_xContext),
                    css::uno::UNO_QUERY_THROW);
            }
            catch (const Exception&)
            {
                // TODO: put a dialog telling about the need to install
                // xslt2-transformer extension here
                SAL_WARN("filter.xslt", "could not create XSLT 2.0 transformer");
                throw;
            }
        }
        else
        {
            xTransformer = xslt::XSLTTransformer::create(m_xContext, rArgs);
        }

        return xTransformer;
    }

    void
    XSLTFilterStreamListener::started()
    {
        m_rParent.m_cTransformed.reset();
    }
    void
    XSLTFilterStreamListener::error(const Any& a)
    {
        SAL_WARN("filter.xslt", "XSLTFilter::error was called: " << exceptionToString(a));
        m_rParent.m_bError = true;
        m_rParent.m_cTransformed.set();
    }
    void
    XSLTFilterStreamListener::closed()
    {
        m_rParent.m_cTransformed.set();
    }
    void
    XSLTFilterStreamListener::terminated()
    {
        m_rParent.m_bTerminated = true;
        m_rParent.m_cTransformed.set();
    }

    OUString
    XSLTFilter::rel2abs(const OUString& s)
    {

        css::uno::Reference<XStringSubstitution>
                subs(css::util::PathSubstitution::create(m_xContext));
        OUString aWorkingDir(subs->getSubstituteVariableValue( u"$(progurl)"_ustr ));
        INetURLObject aObj(aWorkingDir);
        aObj.setFinalSlash();
        bool bWasAbsolute;
        INetURLObject aURL = aObj.smartRel2Abs(s, bWasAbsolute, false,
                INetURLObject::EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, true);
        return aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    }

    sal_Bool
    XSLTFilter::importer(const Sequence<PropertyValue>& aSourceData,
            const css::uno::Reference<XDocumentHandler>& xHandler, const Sequence<
                    OUString>& msUserData)
    {
        if (msUserData.getLength() < 5)
            return false;

        OUString udStyleSheet = rel2abs(msUserData[4]);

        // get information from media descriptor
        // the input stream that represents the imported file
        // is most important here since we need to supply it to
        // the sax parser that drives the supplied document handler
        OUString aName, aURL;
        css::uno::Reference<XInputStream> xInputStream;
        css::uno::Reference<XInteractionHandler> xInterActionHandler;
        for (const auto& sourceDataItem : aSourceData)
        {
            aName = sourceDataItem.Name;
            Any value = sourceDataItem.Value;
            if ( aName == "InputStream" )
                value >>= xInputStream;
            else if ( aName == "URL" )
                value >>= aURL;
            else if ( aName == "InteractionHandler" )
                value >>= xInterActionHandler;
        }
        OSL_ASSERT(xInputStream.is());
        if (!xInputStream.is())
            return false;

        // create transformer
        Sequence<Any> args{ Any(NamedValue(u"StylesheetURL"_ustr, Any(expandUrl(udStyleSheet)))),
                            Any(NamedValue(u"SourceURL"_ustr, Any(aURL))),
                            Any(NamedValue(u"SourceBaseURL"_ustr, Any(INetURLObject(aURL).getBase()))) };
        m_tcontrol = impl_createTransformer(msUserData[1], args);

        OSL_ASSERT(xHandler.is());
        OSL_ASSERT(xInputStream.is());
        OSL_ASSERT(m_tcontrol.is());
        if (xHandler.is() && xInputStream.is() && m_tcontrol.is())
        {
                try
                    {
                        css::uno::Reference<css::io::XSeekable> xSeek(xInputStream, UNO_QUERY);
                        if (xSeek.is())
                            xSeek->seek(0);

                        // we want to be notified when the processing is done...
                        m_tcontrol->addListener(new XSLTFilterStreamListener(*this));

                        // connect input to transformer
                        m_tcontrol->setInputStream(xInputStream);

                        // create pipe
                        css::uno::Reference<XOutputStream> pipeout =
                                        Pipe::create(m_xContext);

                        //connect transformer to pipe
                        m_tcontrol->setOutputStream(pipeout);

                        // connect pipe to sax parser
                        InputSource aInput;
                        aInput.sSystemId = aURL;
                        aInput.sPublicId = aURL;
                        aInput.aInputStream.set(pipeout, UNO_QUERY);

                        css::uno::Reference< css::xml::sax::XFastParser > xFastParser(
                            xHandler, css::uno::UNO_QUERY );

                        // transform
                        m_tcontrol->start();
                        TimeValue timeout = { TRANSFORMATION_TIMEOUT_SEC, 0};
                        osl::Condition::Result result(m_cTransformed.wait(&timeout));
                        while (osl::Condition::result_timeout == result) {
                                if (xInterActionHandler.is()) {
                                        Sequence<Any> excArgs(0);
                                        css::ucb::InteractiveAugmentedIOException exc(
                                                u"Timeout!"_ustr,
                                                getXWeak(),
                                                InteractionClassification_ERROR,
                                                css::ucb::IOErrorCode_GENERAL,
                                                 excArgs);
                                        Any r;
                                        r <<= exc;
                                        rtl::Reference<::comphelper::OInteractionRequest> pRequest = new ::comphelper::OInteractionRequest(r);
                                        rtl::Reference<::comphelper::OInteractionRetry> pRetry = new ::comphelper::OInteractionRetry;
                                        rtl::Reference<::comphelper::OInteractionAbort> pAbort = new ::comphelper::OInteractionAbort;
                                        pRequest->addContinuation(pRetry);
                                        pRequest->addContinuation(pAbort);
                                        xInterActionHandler->handle(pRequest);
                                        if (pAbort->wasSelected()) {
                                                m_bError = true;
                                                m_cTransformed.set();
                                        }
                                }
                                result = m_cTransformed.wait(&timeout);
                        };
                        if (!m_bError) {
                                if( xFastParser.is() )
                                    xFastParser->parseStream( aInput );
                                else
                                {
                                    // create SAX parser that will read the document file
                                    // and provide events to xHandler passed to this call
                                    css::uno::Reference<XParser> xSaxParser = Parser::create(m_xContext);
                                    // set doc handler
                                    xSaxParser->setDocumentHandler(xHandler);
                                    xSaxParser->parseStream( aInput );
                                }
                        }
                        m_tcontrol->terminate();
                        return !m_bError;
                    }
                catch( const Exception& )
                    {
                        // something went wrong
                        TOOLS_WARN_EXCEPTION("filter.xslt", "");
                        return false;
                    }
        }
        else
        {
            return false;
        }
    }

    sal_Bool
    XSLTFilter::importer(const Sequence<PropertyValue>& aSourceData,
            const css::uno::Reference<XFastParser>& xFastParser, const Sequence<
                    OUString>& msUserData)
    {
        if (msUserData.getLength() < 5)
            return false;

        OUString udStyleSheet = rel2abs(msUserData[4]);

        // get information from media descriptor
        // the input stream that represents the imported file
        // is most important here since we need to supply it to
        // the sax parser that drives the supplied document handler
        sal_Int32 nLength = aSourceData.getLength();
        OUString aName, aURL;
        css::uno::Reference<XInputStream> xInputStream;
        css::uno::Reference<XInteractionHandler> xInterActionHandler;
        for (sal_Int32 i = 0; i < nLength; i++)
        {
            aName = aSourceData[i].Name;
            Any value = aSourceData[i].Value;
            if ( aName == "InputStream" )
                value >>= xInputStream;
            else if ( aName == "URL" )
                value >>= aURL;
            else if ( aName == "InteractionHandler" )
                value >>= xInterActionHandler;
        }
        OSL_ASSERT(xInputStream.is());
        if (!xInputStream.is())
            return false;

        // create transformer
        Sequence<Any> args{ Any(NamedValue(u"StylesheetURL"_ustr, Any(expandUrl(udStyleSheet)))),
                            Any(NamedValue(u"SourceURL"_ustr, Any(aURL))),
                            Any(NamedValue(u"SourceBaseURL"_ustr, Any(INetURLObject(aURL).getBase()))) };
        m_tcontrol = impl_createTransformer(msUserData[1], args);

        assert(xFastParser.is());
        OSL_ASSERT(xInputStream.is());
        OSL_ASSERT(m_tcontrol.is());
        if (xFastParser.is() && xInputStream.is() && m_tcontrol.is())
        {
                try
                    {
                        css::uno::Reference<css::io::XSeekable> xSeek(xInputStream, UNO_QUERY);
                        if (xSeek.is())
                            xSeek->seek(0);

                        // we want to be notified when the processing is done...
                        m_tcontrol->addListener(new XSLTFilterStreamListener(*this));

                        // connect input to transformer
                        m_tcontrol->setInputStream(xInputStream);

                        // create pipe
                        css::uno::Reference<XOutputStream> pipeout =
                                        Pipe::create(m_xContext);

                        //connect transformer to pipe
                        m_tcontrol->setOutputStream(pipeout);

                        // connect pipe to sax parser
                        InputSource aInput;
                        aInput.sSystemId = aURL;
                        aInput.sPublicId = aURL;
                        aInput.aInputStream.set(pipeout, UNO_QUERY);

                        // transform
                        m_tcontrol->start();
                        TimeValue timeout = { TRANSFORMATION_TIMEOUT_SEC, 0};
                        osl::Condition::Result result(m_cTransformed.wait(&timeout));
                        while (osl::Condition::result_timeout == result) {
                                if (xInterActionHandler.is()) {
                                        Sequence<Any> excArgs(0);
                                        css::ucb::InteractiveAugmentedIOException exc(
                                                u"Timeout!"_ustr,
                                                getXWeak(),
                                                InteractionClassification_ERROR,
                                                css::ucb::IOErrorCode_GENERAL,
                                                 excArgs);
                                        Any r;
                                        r <<= exc;
                                        rtl::Reference<::comphelper::OInteractionRequest> pRequest = new ::comphelper::OInteractionRequest(r);
                                        rtl::Reference<::comphelper::OInteractionRetry> pRetry = new ::comphelper::OInteractionRetry;
                                        rtl::Reference<::comphelper::OInteractionAbort> pAbort = new ::comphelper::OInteractionAbort;
                                        pRequest->addContinuation(pRetry);
                                        pRequest->addContinuation(pAbort);
                                        xInterActionHandler->handle(pRequest);
                                        if (pAbort->wasSelected()) {
                                                m_bError = true;
                                                m_cTransformed.set();
                                        }
                                }
                                result = m_cTransformed.wait(&timeout);
                        };
                        if (!m_bError)
                            xFastParser->parseStream( aInput );
                        m_tcontrol->terminate();
                        return !m_bError;
                    }
                catch( const Exception& )
                    {
                        // something went wrong
                        TOOLS_WARN_EXCEPTION("filter.xslt", "");
                        return false;
                    }
        }
        else
        {
            return false;
        }
    }

    sal_Bool
    XSLTFilter::exporter(const Sequence<PropertyValue>& aSourceData,
            const Sequence<OUString>& msUserData)
    {
        if (msUserData.getLength() < 6)
            return false;

        // get interesting values from user data
        OUString udStyleSheet = rel2abs(msUserData[5]);

        // read source data
        // we are especially interested in the output stream
        // since that is where our xml-writer will push the data
        // from its data-source interface
        OUString aName, sURL;
        OUString aDoctypePublic;
        // css::uno::Reference<XOutputStream> rOutputStream;
        sal_Int32 nLength = aSourceData.getLength();
        for (sal_Int32 i = 0; i < nLength; i++)
        {
            aName = aSourceData[i].Name;
            if ( aName == "DocType_Public" )
                aSourceData[i].Value >>= aDoctypePublic;
            else if ( aName == "OutputStream" )
                aSourceData[i].Value >>= m_rOutputStream;
            else if ( aName == "URL" )
                aSourceData[i].Value >>= sURL;
        }

        if (!getDelegate().is())
        {
            // get the document writer
            setDelegate(css::uno::Reference<XExtendedDocumentHandler>(
                            Writer::create(m_xContext),
                            UNO_QUERY_THROW));
        }

        // create transformer
        INetURLObject ineturl(sURL);
        ineturl.removeSegment();
        m_aExportBaseUrl = ineturl.GetMainURL(INetURLObject::DecodeMechanism::NONE);
        Sequence<Any> args{ Any(NamedValue(u"StylesheetURL"_ustr, Any(expandUrl(udStyleSheet)))),
                            Any(NamedValue(u"TargetURL"_ustr, Any(sURL))),
                            Any(NamedValue(u"DoctypePublic"_ustr, Any(aDoctypePublic))),
                            Any(NamedValue(u"TargetBaseURL"_ustr, Any(m_aExportBaseUrl))) };
        m_tcontrol = impl_createTransformer(msUserData[1], args);

        OSL_ASSERT(m_rOutputStream.is());
        OSL_ASSERT(m_tcontrol.is());
        if (m_tcontrol.is() && m_rOutputStream.is())
        {
                // we want to be notified when the processing is done...
                m_tcontrol->addListener(new XSLTFilterStreamListener(*this));

                // create pipe
                css::uno::Reference<XOutputStream> pipeout =
                                Pipe::create(m_xContext);
                css::uno::Reference<XInputStream> pipein(pipeout, UNO_QUERY);

                // connect sax writer to pipe
                css::uno::Reference<XActiveDataSource> xmlsource(getDelegate(),
                        UNO_QUERY);
                xmlsource->setOutputStream(pipeout);

                // connect pipe to transformer
                m_tcontrol->setInputStream(pipein);

                // connect transformer to output
                m_tcontrol->setOutputStream(m_rOutputStream);

                // we will start receiving events after returning 'true'.
                // we will start the transformation as soon as we receive the startDocument
                // event.
                return true;
        }
        else
        {
            return false;
        }
    }

    // for the DocumentHandler implementation, we just proxy the
    // events to the XML writer that we created upon the output stream
    // that was provided by the XMLFilterAdapter
    void
    XSLTFilter::startDocument()
    {
        ExtendedDocumentHandlerAdapter::startDocument();
        m_tcontrol->start();
    }

    void
    XSLTFilter::endDocument()
    {
        ExtendedDocumentHandlerAdapter::endDocument();
        // wait for the transformer to finish
        m_cTransformed.wait();
        m_tcontrol->terminate();
        if (m_bError || m_bTerminated)
            throw RuntimeException();
    }


}

// Component management

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_XSLTFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new XSLT::XSLTFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
