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
#include <tools/solar.h>
#include <RtfFilter.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <dmapper/DomainMapper.hxx>
#include <rtftok/RTFDocument.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#ifdef DBG_COPYPASTE
#include <unotools/localfilehelper.hxx>
#include <tools/stream.hxx>
#include <unotools/ucbstreamhelper.hxx>
#endif

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using ::comphelper::MediaDescriptor;

RtfFilter::RtfFilter( const uno::Reference< uno::XComponentContext >& rxContext)  :
    m_xContext( rxContext )
{
}

RtfFilter::~RtfFilter()
{
}

sal_Bool RtfFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
   throw (uno::RuntimeException)
{
    sal_uInt32 nStartTime = osl_getGlobalTimer();
    if( m_xSrcDoc.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc( xMSF->createInstance("com.sun.star.comp.Writer.RtfExport"), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XExporter > xExprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFltr(xIfc, uno::UNO_QUERY_THROW);
        if (!xExprtr.is() || !xFltr.is())
            return sal_False;
        xExprtr->setSourceDocument(m_xSrcDoc);
        return xFltr->filter(aDescriptor);
    }

    sal_Bool bResult(sal_False);
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    try
    {
        MediaDescriptor aMediaDesc( aDescriptor );
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault( "RepairPackage", false );
        bool bIsNewDoc = aMediaDesc.getUnpackedValueOrDefault( "IsNewDoc", true );
#ifdef DEBUG_IMPORT
        OUString sURL = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL(), OUString() );
        ::std::string sURLc = OUStringToOString(sURL, RTL_TEXTENCODING_ASCII_US).getStr();

        writerfilter::TagLogger::Pointer_t dmapperLogger
            (writerfilter::TagLogger::getInstance("DOMAINMAPPER"));
        dmapperLogger->setFileName(sURLc);
        dmapperLogger->startDocument();
#endif
        uno::Reference< io::XInputStream > xInputStream;

        aMediaDesc.addInputStream();
        aMediaDesc[ MediaDescriptor::PROP_INPUTSTREAM() ] >>= xInputStream;

#ifdef DBG_COPYPASTE
        OUString aOutStr;
        if (utl::LocalFileHelper::ConvertPhysicalNameToURL("/tmp/stream.rtf", aOutStr))
        {
            SvStream* pOut = utl::UcbStreamHelper::CreateStream(aOutStr, STREAM_WRITE);
            SvStream* pIn = utl::UcbStreamHelper::CreateStream(xInputStream);
            *pOut << *pIn;
            delete pOut;
        }
#endif

        uno::Reference<frame::XFrame> xFrame = aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_FRAME(),
                uno::Reference<frame::XFrame>());

        xStatusIndicator = aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_STATUSINDICATOR(),
                uno::Reference<task::XStatusIndicator>());

        writerfilter::Stream::Pointer_t pStream(
                new writerfilter::dmapper::DomainMapper(m_xContext, xInputStream, m_xDstDoc, bRepairStorage, writerfilter::dmapper::DOCUMENT_RTF, bIsNewDoc));
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
    catch (const io::WrongFormatException&)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        SAL_INFO("writerfilter", "Exception caught: " << e.Message);
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return bResult;
}

void RtfFilter::cancel(  ) throw (uno::RuntimeException)
{
}

void RtfFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xSrcDoc = xDoc;
}

void RtfFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xDstDoc = xDoc;
}

void RtfFilter::initialize( const uno::Sequence< uno::Any >& /*aArguments*/ ) throw (uno::Exception, uno::RuntimeException)
{
    // The DOCX exporter here extracts 'type' of the filter, ie 'Word' or
    // 'Word Template' but we don't need it for RTF.
}

OUString RtfFilter::getImplementationName(  ) throw (uno::RuntimeException)
{
   return RtfFilter_getImplementationName();
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExportFilter"
sal_Bool RtfFilter::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    return ( rServiceName == SERVICE_NAME1 || rServiceName == SERVICE_NAME2 );
}

uno::Sequence< OUString > RtfFilter::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return RtfFilter_getSupportedServiceNames();
}

/* Helpers, used by shared lib exports. */

OUString RtfFilter_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( "com.sun.star.comp.Writer.RtfFilter" );
}

uno::Sequence< OUString > RtfFilter_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(2);
   OUString* pArray = aRet.getArray();
   pArray[0] =  SERVICE_NAME1;
   pArray[1] =  SERVICE_NAME2;
   return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

uno::Reference< uno::XInterface > RtfFilter_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new RtfFilter( xContext );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
