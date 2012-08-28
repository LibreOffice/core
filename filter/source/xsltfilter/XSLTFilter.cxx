/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <stdio.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase.hxx>

#include <sax/tools/documenthandleradapter.hxx>

#include <osl/time.h>
#include <osl/conditn.h>
#include <rtl/strbuf.hxx>
#include <tools/urlobj.hxx>

#include <comphelper/componentcontext.hxx>
#include <comphelper/interaction.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/util/XMacroExpander.hpp>

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XStreamListener.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>

#include <xmloff/attrlist.hxx>

#include <LibXSLTTransformer.hxx>

#define TRANSFORMATION_TIMEOUT_SEC 60

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::sax;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::task;

namespace css = com::sun::star;

namespace XSLT
{
    /*
     * XSLTFilter reads flat XML streams from the XML filter framework and passes
     * them to an XSLT transformation service. XSLT transformation errors are
     * reported to XSLTFilter.
     *
     * Currently, two implementations for the XSLT transformation service exist:
     * a java based service (see XSLTransformer.java) and  a libxslt based
     * service (LibXSLTTransformer.cxx).
     *
     * The libxslt implementation will be used by default.
     *
     * If the value of the 2nd "UserData" parameter of the filter configuration is
     * not empty, the service name given there will be used.
     */
    class XSLTFilter : public WeakImplHelper4<XImportFilter, XExportFilter,
            XStreamListener, ExtendedDocumentHandlerAdapter>
    {
    private:

        // the UNO ServiceFactory
        css::uno::Reference<XMultiServiceFactory> m_rServiceFactory;

        // DocumentHandler interface of the css::xml::sax::Writer service
        css::uno::Reference<XOutputStream> m_rOutputStream;

        css::uno::Reference<XActiveDataControl> m_tcontrol;

        oslCondition m_cTransformed;
        sal_Bool m_bTerminated;
        sal_Bool m_bError;

        OUString m_aExportBaseUrl;

        OUString
        rel2abs(const OUString&);
        OUString
        expandUrl(const OUString&);

    public:

        // ctor...
        XSLTFilter(const css::uno::Reference<XMultiServiceFactory> &r);

        // XStreamListener
        virtual void SAL_CALL
        error(const Any& a) throw (RuntimeException);
        virtual void SAL_CALL
        closed() throw (RuntimeException);
        virtual void SAL_CALL
        terminated() throw (RuntimeException);
        virtual void SAL_CALL
        started() throw (RuntimeException);
        virtual void SAL_CALL
        disposing(const EventObject& e) throw (RuntimeException);

        // XImportFilter
        virtual sal_Bool SAL_CALL
        importer(const Sequence<PropertyValue>& aSourceData, const css::uno::Reference<
                XDocumentHandler>& xHandler,
                const Sequence<OUString>& msUserData) throw (RuntimeException);

        // XExportFilter
        virtual sal_Bool SAL_CALL
        exporter(const Sequence<PropertyValue>& aSourceData, const Sequence<
                OUString>& msUserData) throw (RuntimeException);

        // XDocumentHandler
        virtual void SAL_CALL
        startDocument() throw (SAXException, RuntimeException);
        virtual void SAL_CALL
        endDocument() throw (SAXException, RuntimeException);
    };

    XSLTFilter::XSLTFilter(const css::uno::Reference<XMultiServiceFactory> &r):
m_rServiceFactory(r), m_bTerminated(sal_False), m_bError(sal_False)
    {
        m_cTransformed = osl_createCondition();
    }

    void
    XSLTFilter::disposing(const EventObject&) throw (RuntimeException)
    {
    }

    ::rtl::OUString
    XSLTFilter::expandUrl(const ::rtl::OUString& sUrl)
    {
        ::rtl::OUString sExpandedUrl;
        try
            {
                css::uno::Reference<XComponentContext> xContext;
                css::uno::Reference<XPropertySet> xProps(m_rServiceFactory,
                        UNO_QUERY_THROW);
                xContext.set(xProps->getPropertyValue(::rtl::OUString(
                         "DefaultContext" )),
                        UNO_QUERY_THROW);
                css::uno::Reference<XMacroExpander>
                        xMacroExpander(
                                xContext->getValueByName(
                                        ::rtl::OUString(
                                                 "/singletons/com.sun.star.util.theMacroExpander" )),
                                UNO_QUERY_THROW);
                sExpandedUrl = xMacroExpander->expandMacros(sUrl);
                sal_Int32 nPos = sExpandedUrl.indexOf( "vnd.sun.star.expand:" );
                if (nPos != -1)
                    sExpandedUrl = sExpandedUrl.copy(nPos + 20);
            }
        catch (const Exception&)
            {
            }
        return sExpandedUrl;
    }

    void
    XSLTFilter::started() throw (RuntimeException)
    {
        osl_resetCondition(m_cTransformed);
    }
    void
    XSLTFilter::error(const Any& a) throw (RuntimeException)
    {
        Exception e;
        if (a >>= e)
        {
            rtl::OStringBuffer aMessage(RTL_CONSTASCII_STRINGPARAM("XSLTFilter::error was called: "));
            aMessage.append(rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
            OSL_FAIL(aMessage.getStr());
        }
        m_bError = sal_True;
        osl_setCondition(m_cTransformed);
    }
    void
    XSLTFilter::closed() throw (RuntimeException)
    {
        osl_setCondition(m_cTransformed);
    }
    void
    XSLTFilter::terminated() throw (RuntimeException)
    {
        m_bTerminated = sal_True;
        osl_setCondition(m_cTransformed);
    }

    OUString
    XSLTFilter::rel2abs(const OUString& s)
    {

        css::uno::Reference< css::uno::XComponentContext > xContext( comphelper::ComponentContext(m_rServiceFactory).getUNOContext() );
        css::uno::Reference<XStringSubstitution>
                subs(css::util::PathSubstitution::create(xContext));
        OUString aWorkingDir(subs->getSubstituteVariableValue(OUString( "$(progurl)")));
        INetURLObject aObj(aWorkingDir);
        aObj.setFinalSlash();
        bool bWasAbsolute;
        INetURLObject aURL = aObj.smartRel2Abs(s, bWasAbsolute, false,
                INetURLObject::WAS_ENCODED, RTL_TEXTENCODING_UTF8, true);
        return aURL.GetMainURL(INetURLObject::NO_DECODE);
    }

    sal_Bool
    XSLTFilter::importer(const Sequence<PropertyValue>& aSourceData,
            const css::uno::Reference<XDocumentHandler>& xHandler, const Sequence<
                    OUString>& msUserData) throw (RuntimeException)
    {
        if (msUserData.getLength() < 5)
            return sal_False;

        OUString udImport = msUserData[2];
        OUString udStyleSheet = rel2abs(msUserData[4]);

        // get information from media descriptor
        // the imput stream that represents the imported file
        // is most important here since we need to supply it to
        // the sax parser that drives the supplied document handler
        sal_Int32 nLength = aSourceData.getLength();
        OUString aName, aFileName, aURL;
        css::uno::Reference<XInputStream> xInputStream;
        css::uno::Reference<XInteractionHandler> xInterActionHandler;
        for (sal_Int32 i = 0; i < nLength; i++)
            {
                aName = aSourceData[i].Name;
                Any value = aSourceData[i].Value;
                if ( aName == "InputStream" )
                    value >>= xInputStream;
                else if ( aName == "FileName" )
                    value >>= aFileName;
                else if ( aName == "URL" )
                    value >>= aURL;
                else if ( aName == "InteractionHandler" )
                    value >>= xInterActionHandler;
            }
        OSL_ASSERT(xInputStream.is());
        if (!xInputStream.is())
            return sal_False;

        // create SAX parser that will read the document file
        // and provide events to xHandler passed to this call
        css::uno::Reference<XParser>
                xSaxParser(
                        m_rServiceFactory->createInstance(
                                OUString(
                                         "com.sun.star.xml.sax.Parser" )),
                        UNO_QUERY);
        OSL_ASSERT(xSaxParser.is());
        if (!xSaxParser.is())
            return sal_False;

        // create transformer
        Sequence<Any> args(3);
        NamedValue nv;

        nv.Name = OUString( "StylesheetURL" );
        nv.Value <<= expandUrl(udStyleSheet);
        args[0] <<= nv;
        nv.Name = OUString( "SourceURL" );
        nv.Value <<= aURL;
        args[1] <<= nv;
        nv.Name = OUString( "SourceBaseURL" );
        nv.Value <<= OUString(INetURLObject(aURL).getBase());
        args[2] <<= nv;

        OUString serviceName("com.sun.star.comp.documentconversion.LibXSLTTransformer");
        if (!msUserData[1].isEmpty())
            serviceName = msUserData[1];

        m_tcontrol = css::uno::Reference<XActiveDataControl> (m_rServiceFactory->createInstanceWithArguments(serviceName, args), UNO_QUERY);

        OSL_ASSERT(xHandler.is());
        OSL_ASSERT(xInputStream.is());
        OSL_ASSERT(m_tcontrol.is());
        if (xHandler.is() && xInputStream.is() && m_tcontrol.is())
            {
                try
                    {
                        // we want to be notfied when the processing is done...
                        m_tcontrol->addListener(css::uno::Reference<XStreamListener> (
                                this));

                        // connect input to transformer
                        css::uno::Reference<XActiveDataSink> tsink(m_tcontrol, UNO_QUERY);
                        tsink->setInputStream(xInputStream);

                        // create pipe
                        css::uno::Reference<XOutputStream> pipeout(
                                        Pipe::create(comphelper::ComponentContext(m_rServiceFactory).getUNOContext()),
                                        UNO_QUERY);
                        css::uno::Reference<XInputStream> pipein(pipeout, UNO_QUERY);

                        //connect transformer to pipe
                        css::uno::Reference<XActiveDataSource> tsource(m_tcontrol,
                                UNO_QUERY);
                        tsource->setOutputStream(pipeout);

                        // connect pipe to sax parser
                        InputSource aInput;
                        aInput.sSystemId = aURL;
                        aInput.sPublicId = aURL;
                        aInput.aInputStream = pipein;

                        // set doc handler
                        xSaxParser->setDocumentHandler(xHandler);

                        // transform
                        m_tcontrol->start();
                        TimeValue timeout = { TRANSFORMATION_TIMEOUT_SEC, 0};
                        oslConditionResult result(osl_waitCondition(m_cTransformed, &timeout));
                        while (osl_cond_result_timeout == result) {
                                if (xInterActionHandler.is()) {
                                        Sequence<Any> excArgs(0);
                                        ::com::sun::star::ucb::InteractiveAugmentedIOException exc(
                                                rtl::OUString("Timeout!"),
                                                static_cast< OWeakObject * >( this ),
                                                InteractionClassification_ERROR,
                                                ::com::sun::star::ucb::IOErrorCode_GENERAL,
                                                 excArgs);
                                        Any r;
                                        r <<= exc;
                                        ::comphelper::OInteractionRequest* pRequest = new ::comphelper::OInteractionRequest(r);
                                        css::uno::Reference< XInteractionRequest > xRequest(pRequest);
                                        ::comphelper::OInteractionRetry* pRetry = new ::comphelper::OInteractionRetry;
                                        ::comphelper::OInteractionAbort* pAbort = new ::comphelper::OInteractionAbort;
                                        pRequest->addContinuation(pRetry);
                                        pRequest->addContinuation(pAbort);
                                        xInterActionHandler->handle(xRequest);
                                        if (pAbort->wasSelected()) {
                                                m_bError = sal_True;
                                                osl_setCondition(m_cTransformed);
                                        }
                                }
                                result = osl_waitCondition(m_cTransformed, &timeout);
                        };
                        if (!m_bError) {
                                xSaxParser->parseStream(aInput);
                        }
                        m_tcontrol->terminate();
                        return !m_bError;
                    }
#if OSL_DEBUG_LEVEL > 0
                catch( const Exception& exc)
#else
                catch (const Exception&)
#endif
                    {
                        // something went wrong
                        OSL_FAIL(OUStringToOString(exc.Message, RTL_TEXTENCODING_ASCII_US).getStr());
                        return sal_False;
                    }
            }
        else
            {
                return sal_False;
            }
    }

    sal_Bool
    XSLTFilter::exporter(const Sequence<PropertyValue>& aSourceData,
            const Sequence<OUString>& msUserData) throw (RuntimeException)
    {
        if (msUserData.getLength() < 6)
            return sal_False;

        // get interesting values from user data
        OUString udImport = msUserData[2];
        OUString udStyleSheet = rel2abs(msUserData[5]);

        // read source data
        // we are especialy interested in the output stream
        // since that is where our xml-writer will push the data
        // from it's data-source interface
        OUString aName, sURL;
        sal_Bool bIndent = sal_False;
        OUString aDoctypePublic;
        OUString aDoctypeSystem;
        // css::uno::Reference<XOutputStream> rOutputStream;
        sal_Int32 nLength = aSourceData.getLength();
        for (sal_Int32 i = 0; i < nLength; i++)
            {
                aName = aSourceData[i].Name;
                if ( aName == "Indent" )
                    aSourceData[i].Value >>= bIndent;
                if ( aName == "DocType_Public" )
                    aSourceData[i].Value >>= aDoctypePublic;
                if ( aName == "DocType_System" )
                    aSourceData[i].Value >>= aDoctypeSystem;
                if ( aName == "OutputStream" )
                    aSourceData[i].Value >>= m_rOutputStream;
                else if ( aName == "URL" )
                    aSourceData[i].Value >>= sURL;
            }

        if (!getDelegate().is())
            {
                // get the document writer
                setDelegate(css::uno::Reference<XExtendedDocumentHandler> (
                                m_rServiceFactory->createInstance(
                                        OUString(
                                                 "com.sun.star.xml.sax.Writer" )),
                                UNO_QUERY));
            }

        // create transformer
        Sequence<Any> args(4);
        NamedValue nv;
        nv.Name = OUString( "StylesheetURL" );
        nv.Value <<= expandUrl(udStyleSheet);
        args[0] <<= nv;
        nv.Name = OUString( "TargetURL" );
        nv.Value <<= sURL;
        args[1] <<= nv;
        nv.Name = OUString( "DoctypeSystem" );
        nv.Value <<= aDoctypeSystem;
        args[2] <<= nv;
        nv.Name = OUString( "DoctypePublic" );
        nv.Value <<= aDoctypePublic;
        args[3] <<= nv;
        nv.Name = OUString( "TargetBaseURL" );
        INetURLObject ineturl(sURL);
        ineturl.removeSegment();
        m_aExportBaseUrl = ineturl.GetMainURL(INetURLObject::NO_DECODE);
        nv.Value <<= m_aExportBaseUrl;
        args[3] <<= nv;

        OUString serviceName("com.sun.star.comp.documentconversion.LibXSLTTransformer");
        if (!msUserData[1].isEmpty())
            serviceName = msUserData[1];

        m_tcontrol = css::uno::Reference<XActiveDataControl> (m_rServiceFactory->createInstanceWithArguments(serviceName, args), UNO_QUERY);

        OSL_ASSERT(m_rOutputStream.is());
        OSL_ASSERT(m_tcontrol.is());
        if (m_tcontrol.is() && m_rOutputStream.is())
            {
                // we want to be notfied when the processing is done...
                m_tcontrol->addListener(css::uno::Reference<XStreamListener> (this));

                // create pipe
                css::uno::Reference<XOutputStream> pipeout(
                                Pipe::create(comphelper::ComponentContext(m_rServiceFactory).getUNOContext()),
                                UNO_QUERY);
                css::uno::Reference<XInputStream> pipein(pipeout, UNO_QUERY);

                // connect sax writer to pipe
                css::uno::Reference<XActiveDataSource> xmlsource(getDelegate(),
                        UNO_QUERY);
                xmlsource->setOutputStream(pipeout);

                // connect pipe to transformer
                css::uno::Reference<XActiveDataSink> tsink(m_tcontrol, UNO_QUERY);
                tsink->setInputStream(pipein);

                // connect transformer to output
                css::uno::Reference<XActiveDataSource> tsource(m_tcontrol, UNO_QUERY);
                tsource->setOutputStream(m_rOutputStream);

                // we will start receiving events after returning 'true'.
                // we will start the transformation as soon as we receive the startDocument
                // event.
                return sal_True;
            }
        else
            {
                return sal_False;
            }
    }

    // for the DocumentHandler implementation, we just proxy the the
    // events to the XML writer that we created upon the output stream
    // that was provided by the XMLFilterAdapter
    void
    XSLTFilter::startDocument() throw (SAXException, RuntimeException)
    {
        ExtendedDocumentHandlerAdapter::startDocument();
        m_tcontrol->start();
    }

    void
    XSLTFilter::endDocument() throw (SAXException, RuntimeException)
    {
        ExtendedDocumentHandlerAdapter::endDocument();
        // wait for the transformer to finish
        osl_waitCondition(m_cTransformed, 0);
        m_tcontrol->terminate();
        if (!m_bError && !m_bTerminated)
            {
                return;
            }
        else
            {
                throw RuntimeException();
            }

    }


    // --------------------------------------
    // Component management
    // --------------------------------------
#define FILTER_SERVICE_NAME "com.sun.star.documentconversion.XSLTFilter"
#define FILTER_IMPL_NAME "com.sun.star.comp.documentconversion.XSLTFilter"
#define TRANSFORMER_SERVICE_NAME "com.sun.star.documentconversion.LibXSLTTransformer"
#define TRANSFORMER_IMPL_NAME "com.sun.star.comp.documentconversion.LibXSLTTransformer"

    static css::uno::Reference<XInterface> SAL_CALL
    CreateTransformerInstance(const css::uno::Reference<XMultiServiceFactory> &r)
    {
        return css::uno::Reference<XInterface> ((OWeakObject *) new LibXSLTTransformer(r));
    }

    static css::uno::Reference<XInterface> SAL_CALL
    CreateFilterInstance(const css::uno::Reference<XMultiServiceFactory> &r)
    {
        return css::uno::Reference<XInterface> ((OWeakObject *) new XSLTFilter(r));
    }

}

using namespace XSLT;

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(const sal_Char * pImplName,
            void * pServiceManager, void * /* pRegistryKey */)
    {
        void * pRet = 0;

        if (pServiceManager)
            {
                if (rtl_str_compare(pImplName, FILTER_IMPL_NAME) == 0)
                    {
                        Sequence<OUString> serviceNames(1);
                        serviceNames.getArray()[0]
                                = OUString(
                                         FILTER_SERVICE_NAME );

                        css::uno::Reference<XSingleServiceFactory>
                                xFactory(
                                        createSingleFactory(
                                                reinterpret_cast<XMultiServiceFactory *> (pServiceManager),
                                                OUString::createFromAscii(
                                                        pImplName),
                                                CreateFilterInstance,
                                                serviceNames));

                        if (xFactory.is())
                            {
                                xFactory->acquire();
                                pRet = xFactory.get();
                            }
                    }
                else if (rtl_str_compare(pImplName, TRANSFORMER_IMPL_NAME) == 0)
                    {
                        Sequence<OUString> serviceNames(1);
                        serviceNames.getArray()[0]
                                = OUString(
                                         TRANSFORMER_SERVICE_NAME );
                        css::uno::Reference<XSingleServiceFactory>
                                xFactory(
                                        createSingleFactory(
                                                reinterpret_cast<XMultiServiceFactory *> (pServiceManager),
                                                OUString::createFromAscii(
                                                        pImplName),
                                                CreateTransformerInstance,
                                                serviceNames));

                        if (xFactory.is())
                            {
                                xFactory->acquire();
                                pRet = xFactory.get();
                            }

                    }
            }
        return pRet;
    }

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
