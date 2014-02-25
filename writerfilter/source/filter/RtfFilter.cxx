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

#include <rtl/ustring.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <osl/module.hxx>
#include <RtfFilter.hxx>
#include <unotools/mediadescriptor.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <dmapper/DomainMapper.hxx>
#include <rtftok/RTFDocument.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <unotools/localfilehelper.hxx>
#include <tools/stream.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

using namespace ::com::sun::star;

RtfFilter::RtfFilter(const uno::Reference< uno::XComponentContext >& rxContext)
    : m_xContext(rxContext)
{
}

RtfFilter::~RtfFilter()
{
}

sal_Bool RtfFilter::filter(const uno::Sequence< beans::PropertyValue >& aDescriptor) throw(uno::RuntimeException, std::exception)
{
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    if (m_xSrcDoc.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc(xMSF->createInstance("com.sun.star.comp.Writer.RtfExport"), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XExporter > xExporter(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFilter(xIfc, uno::UNO_QUERY_THROW);
        if (!xExporter.is() || !xFilter.is())
            return sal_False;
        xExporter->setSourceDocument(m_xSrcDoc);
        return xFilter->filter(aDescriptor);
    }

    sal_Bool bResult(sal_False);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    try
    {
        utl::MediaDescriptor aMediaDesc(aDescriptor);
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault("RepairPackage", false);
        bool bIsNewDoc = !aMediaDesc.getUnpackedValueOrDefault("InsertMode", false);
        uno::Reference<text::XTextRange> xInsertTextRange = aMediaDesc.getUnpackedValueOrDefault("TextInsertModeRange", uno::Reference<text::XTextRange>());
#ifdef DEBUG_IMPORT
        OUString sURL = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL(), OUString());
        std::string sURLc = OUStringToOString(sURL, RTL_TEXTENCODING_ASCII_US).getStr();

        writerfilter::TagLogger::Pointer_t dmapperLogger
        (writerfilter::TagLogger::getInstance("DOMAINMAPPER"));
        dmapperLogger->setFileName(sURLc);
        dmapperLogger->startDocument();
#endif
        uno::Reference< io::XInputStream > xInputStream;

        aMediaDesc.addInputStream();
        aMediaDesc[ utl::MediaDescriptor::PROP_INPUTSTREAM() ] >>= xInputStream;

        // If this is set, write to this file, instead of the real document during paste.
        char* pEnv = getenv("SW_DEBUG_RTF_PASTE_TO");
        OUString aOutStr;
        if (!bIsNewDoc && pEnv && utl::LocalFileHelper::ConvertPhysicalNameToURL(OStringToOUString(pEnv, RTL_TEXTENCODING_UTF8), aOutStr))
        {
            SvStream* pOut = utl::UcbStreamHelper::CreateStream(aOutStr, STREAM_WRITE);
            SvStream* pIn = utl::UcbStreamHelper::CreateStream(xInputStream);
            pOut->WriteStream(*pIn);
            delete pOut;
            return true;
        }

        // If this is set, read from this file, instead of the real clipboard during paste.
        pEnv = getenv("SW_DEBUG_RTF_PASTE_FROM");
        if (!bIsNewDoc && pEnv)
        {
            OUString aInStr;
            utl::LocalFileHelper::ConvertPhysicalNameToURL(OStringToOUString(pEnv, RTL_TEXTENCODING_UTF8), aInStr);
            SvStream* pStream = utl::UcbStreamHelper::CreateStream(aInStr, STREAM_READ);
            uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
            xInputStream.set(xStream, uno::UNO_QUERY);
        }

        uno::Reference<frame::XFrame> xFrame = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FRAME(),
                                               uno::Reference<frame::XFrame>());

        xStatusIndicator = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STATUSINDICATOR(),
                           uno::Reference<task::XStatusIndicator>());

        writerfilter::Stream::Pointer_t pStream(
            new writerfilter::dmapper::DomainMapper(m_xContext, xInputStream, m_xDstDoc, bRepairStorage, writerfilter::dmapper::DOCUMENT_RTF, xInsertTextRange, bIsNewDoc));
        writerfilter::rtftok::RTFDocument::Pointer_t const pDocument(
            writerfilter::rtftok::RTFDocumentFactory::createDocument(m_xContext, xInputStream, m_xDstDoc, xFrame, xStatusIndicator));
        pDocument->resolve(*pStream);
        bResult = sal_True;
#ifdef DEBUG_IMPORT
        dmapperLogger->endDocument();
#endif
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        SAL_INFO("writerfilter.profile", OSL_THIS_FUNC << " finished in " << nEndTime - nStartTime << " ms");
    }
    catch (const io::WrongFormatException& e)
    {
        // cannot throw WrongFormatException directly :(
        throw lang::WrappedTargetRuntimeException("",
                static_cast<OWeakObject*>(this), uno::makeAny(e));
    }
    catch (const uno::Exception& e)
    {
        SAL_INFO("writerfilter", "Exception caught: " << e.Message);
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return bResult;
}

void RtfFilter::cancel() throw(uno::RuntimeException, std::exception)
{
}

void RtfFilter::setSourceDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    m_xSrcDoc = xDoc;
}

void RtfFilter::setTargetDocument(const uno::Reference< lang::XComponent >& xDoc) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    m_xDstDoc = xDoc;
}

void RtfFilter::initialize(const uno::Sequence< uno::Any >& /*aArguments*/) throw(uno::Exception, uno::RuntimeException, std::exception)
{
    // The DOCX exporter here extracts 'type' of the filter, ie 'Word' or
    // 'Word Template' but we don't need it for RTF.
}

OUString RtfFilter::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return RtfFilter_getImplementationName();
}

sal_Bool RtfFilter::supportsService(const OUString& rServiceName) throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > RtfFilter::getSupportedServiceNames() throw(uno::RuntimeException, std::exception)
{
    return RtfFilter_getSupportedServiceNames();
}

/* Helpers, used by shared lib exports. */
OUString RtfFilter_getImplementationName() throw(uno::RuntimeException)
{
    return OUString("com.sun.star.comp.Writer.RtfFilter");
}

uno::Sequence< OUString > RtfFilter_getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExportFilter";
    return aRet;
}

uno::Reference< uno::XInterface > RtfFilter_createInstance(const uno::Reference< uno::XComponentContext >& xContext) throw(uno::Exception)
{
    return (cppu::OWeakObject*) new RtfFilter(xContext);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
