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

#include <memory>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/scopeguard.hxx>

#include <dmapper/DomainMapperFactory.hxx>
#include <rtftok/RTFDocument.hxx>

using namespace ::com::sun::star;

namespace
{
/// Invokes the RTF tokenizer + dmapper or RtfExportFilter in sw via UNO.
class RtfFilter
    : public cppu::WeakImplHelper<document::XFilter, document::XImporter, document::XExporter,
                                  lang::XInitialization, lang::XServiceInfo>
{
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XComponent> m_xSrcDoc, m_xDstDoc;

public:
    explicit RtfFilter(uno::Reference<uno::XComponentContext> xContext);

    // XFilter
    sal_Bool SAL_CALL filter(const uno::Sequence<beans::PropertyValue>& rDescriptor) override;
    void SAL_CALL cancel() override;

    // XImporter
    void SAL_CALL setTargetDocument(const uno::Reference<lang::XComponent>& xDoc) override;

    // XExporter
    void SAL_CALL setSourceDocument(const uno::Reference<lang::XComponent>& xDoc) override;

    // XInitialization
    void SAL_CALL initialize(const uno::Sequence<uno::Any>& rArguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};
}

RtfFilter::RtfFilter(uno::Reference<uno::XComponentContext> xContext)
    : m_xContext(std::move(xContext))
{
}

sal_Bool RtfFilter::filter(const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    if (m_xSrcDoc.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xMSF(m_xContext->getServiceManager(),
                                                        uno::UNO_QUERY_THROW);
        uno::Reference<uno::XInterface> xIfc(
            xMSF->createInstance("com.sun.star.comp.Writer.RtfExport"), uno::UNO_SET_THROW);
        uno::Reference<document::XExporter> xExporter(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference<document::XFilter> xFilter(xIfc, uno::UNO_QUERY_THROW);
        xExporter->setSourceDocument(m_xSrcDoc);
        return xFilter->filter(rDescriptor);
    }

    bool bResult(false);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    uno::Reference<beans::XPropertySet> xDocProps;
    if (m_xDstDoc.is()) // not in cppunittest?
    {
        xDocProps.set(m_xDstDoc, uno::UNO_QUERY);
        xDocProps->setPropertyValue("UndocumentedWriterfilterHack", uno::Any(true));
    }
    comphelper::ScopeGuard g([xDocProps] {
        if (xDocProps.is()) // not in cppunittest?
        {
            // note: pStream.clear calls RemoveLastParagraph()
            xDocProps->setPropertyValue("UndocumentedWriterfilterHack", uno::Any(false));
        }
    });

    try
    {
        utl::MediaDescriptor aMediaDesc(rDescriptor);
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault("RepairPackage", false);
        bool bIsNewDoc = !aMediaDesc.getUnpackedValueOrDefault("InsertMode", false);
        uno::Reference<io::XInputStream> xInputStream;

        aMediaDesc.addInputStream();
        aMediaDesc[utl::MediaDescriptor::PROP_INPUTSTREAM] >>= xInputStream;

        // If this is set, write to this file, instead of the real document during paste.
        char* pEnv = getenv("SW_DEBUG_RTF_PASTE_TO");
        OUString aOutStr;
        if (!bIsNewDoc && pEnv
            && osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pEnv), aOutStr)
                   == osl::FileBase::E_None)
        {
            std::unique_ptr<SvStream> pOut(
                utl::UcbStreamHelper::CreateStream(aOutStr, StreamMode::WRITE));
            std::unique_ptr<SvStream> pIn(utl::UcbStreamHelper::CreateStream(xInputStream));
            pOut->WriteStream(*pIn);
            return true;
        }

        // If this is set, read from this file, instead of the real clipboard during paste.
        pEnv = getenv("SW_DEBUG_RTF_PASTE_FROM");
        if (!bIsNewDoc && pEnv)
        {
            OUString aInStr;
            osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pEnv), aInStr);
            std::unique_ptr<SvStream> pStream
                = utl::UcbStreamHelper::CreateStream(aInStr, StreamMode::READ);
            uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
            xInputStream.set(xStream, uno::UNO_QUERY);
        }

        uno::Reference<frame::XFrame> xFrame = aMediaDesc.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_FRAME, uno::Reference<frame::XFrame>());

        xStatusIndicator = aMediaDesc.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_STATUSINDICATOR, uno::Reference<task::XStatusIndicator>());

        writerfilter::Stream::Pointer_t pStream(
            writerfilter::dmapper::DomainMapperFactory::createMapper(
                m_xContext, xInputStream, m_xDstDoc, bRepairStorage,
                writerfilter::dmapper::SourceDocumentType::RTF, aMediaDesc));
        writerfilter::rtftok::RTFDocument::Pointer_t pDocument(
            writerfilter::rtftok::RTFDocumentFactory::createDocument(
                m_xContext, xInputStream, m_xDstDoc, xFrame, xStatusIndicator, aMediaDesc));
        pDocument->resolve(*pStream);
        bResult = true;
    }
    catch (const io::WrongFormatException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        // cannot throw WrongFormatException directly :(
        throw lang::WrappedTargetRuntimeException("", getXWeak(), anyEx);
    }
    catch (const uno::Exception&)
    {
        TOOLS_INFO_EXCEPTION("writerfilter", "Exception caught");
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return bResult;
}

void RtfFilter::cancel() {}

void RtfFilter::setSourceDocument(const uno::Reference<lang::XComponent>& xDoc)
{
    m_xSrcDoc = xDoc;
}

void RtfFilter::setTargetDocument(const uno::Reference<lang::XComponent>& xDoc)
{
    m_xDstDoc = xDoc;
}

void RtfFilter::initialize(const uno::Sequence<uno::Any>& /*aArguments*/)
{
    // The DOCX exporter here extracts 'type' of the filter, ie 'Word' or
    // 'Word Template' but we don't need it for RTF.
}

OUString RtfFilter::getImplementationName() { return "com.sun.star.comp.Writer.RtfFilter"; }

sal_Bool RtfFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> RtfFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet = { OUString("com.sun.star.document.ImportFilter"),
                                     OUString("com.sun.star.document.ExportFilter") };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_RtfFilter_get_implementation(uno::XComponentContext* pComponent,
                                                      uno::Sequence<uno::Any> const& /*rSequence*/)
{
    return cppu::acquire(new RtfFilter(pComponent));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
