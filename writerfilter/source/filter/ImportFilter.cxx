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

#include <osl/diagnose.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <oox/core/filterdetect.hxx>
#include <dmapper/DomainMapper.hxx>
#include <WriterFilter.hxx>
#include <doctok/WW8Document.hxx>
#include <ooxml/OOXMLDocument.hxx>
#ifdef DEBUG_IMPORT
#include <iostream>
#include <osl/process.h>
#endif

#include <resourcemodel/TagLogger.hxx>
using namespace ::rtl;
using namespace ::com::sun::star;
using ::comphelper::MediaDescriptor;

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
   throw (uno::RuntimeException)
{
    if( m_xSrcDoc.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc( xMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.DocxExport" ))), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XExporter > xExprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFltr(xIfc, uno::UNO_QUERY_THROW);
        if (!xExprtr.is() || !xFltr.is())
            return sal_False;
        xExprtr->setSourceDocument(m_xSrcDoc);
        return xFltr->filter(aDescriptor);
    }
    else if (m_xDstDoc.is())
    {
        MediaDescriptor aMediaDesc( aDescriptor );
        OUString sFilterName = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_FILTERNAME(), OUString() );

        uno::Reference< io::XInputStream > xInputStream;
        try
        {
            // use the oox.core.FilterDetect implementation to extract the decrypted ZIP package
            uno::Reference< lang::XMultiServiceFactory > xFactory( m_xContext->getServiceManager(), uno::UNO_QUERY_THROW );
            ::oox::core::FilterDetect aDetector( xFactory );
            xInputStream = aDetector.extractUnencryptedPackage( aMediaDesc );
        }
        catch( uno::Exception& )
        {
        }

        if ( !xInputStream.is() )
        {
            return sal_False;
        }

#ifdef DEBUG_ELEMENT
        OUString sURL = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL(), OUString() );
        ::std::string sURLc = OUStringToOString(sURL, RTL_TEXTENCODING_ASCII_US).getStr();
        
        writerfilter::TagLogger::Pointer_t debugLogger
        (writerfilter::TagLogger::getInstance("DEBUG"));
        debugLogger->setFileName(sURLc);
        debugLogger->startDocument();
        
        writerfilter::TagLogger::Pointer_t dmapperLogger
        (writerfilter::TagLogger::getInstance("DOMAINMAPPER"));
        dmapperLogger->setFileName(sURLc);
        dmapperLogger->startDocument();
#endif

    writerfilter::dmapper::SourceDocumentType eType =
        (m_sFilterName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "writer_MS_Word_2007" ) ) ||
         m_sFilterName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "writer_MS_Word_2007_Template" ) ) ||
         m_sFilterName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "writer_OOXML" ) ) ||
         m_sFilterName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "writer_OOXML_Text_Template" ) )) ?
                writerfilter::dmapper::DOCUMENT_OOXML : writerfilter::dmapper::DOCUMENT_DOC;
    writerfilter::Stream::Pointer_t pStream(new writerfilter::dmapper::DomainMapper(m_xContext, xInputStream, m_xDstDoc, eType));
    //create the tokenizer and domain mapper
    if( eType == writerfilter::dmapper::DOCUMENT_OOXML )
    {
        writerfilter::ooxml::OOXMLStream::Pointer_t pDocStream = writerfilter::ooxml::OOXMLDocumentFactory::createStream(m_xContext, xInputStream);
        writerfilter::ooxml::OOXMLDocument::Pointer_t pDocument(writerfilter::ooxml::OOXMLDocumentFactory::createDocument(pDocStream));

        uno::Reference<frame::XModel> xModel(m_xDstDoc, uno::UNO_QUERY_THROW);
        pDocument->setModel(xModel);

        uno::Reference<drawing::XDrawPageSupplier> xDrawings
            (m_xDstDoc, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPage> xDrawPage
            (xDrawings->getDrawPage(), uno::UNO_SET_THROW);
        pDocument->setDrawPage(xDrawPage);

        pDocument->resolve(*pStream);
    }
    else
    {
        writerfilter::doctok::WW8Stream::Pointer_t pDocStream = writerfilter::doctok::WW8DocumentFactory::createStream(m_xContext, xInputStream);
        writerfilter::doctok::WW8Document::Pointer_t pDocument(writerfilter::doctok::WW8DocumentFactory::createDocument(pDocStream));

        pDocument->resolve(*pStream);
    }

#ifdef DEBUG_ELEMENT
    writerfilter::TagLogger::dump("DOMAINMAPPER");
    dmapperLogger->endDocument();
    writerfilter::TagLogger::dump("DEBUG");
    debugLogger->endDocument();
#endif

    return sal_True;
    }
    return sal_False;
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void WriterFilter::cancel(  ) throw (uno::RuntimeException)
{
}

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void WriterFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xDstDoc = xDoc;
}

void WriterFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xSrcDoc = xDoc;
}

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void WriterFilter::initialize( const uno::Sequence< uno::Any >& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
   uno::Sequence < beans::PropertyValue > aAnySeq;
   sal_Int32 nLength = aArguments.getLength();
   if ( nLength && ( aArguments[0] >>= aAnySeq ) )
   {
       const beans::PropertyValue * pValue = aAnySeq.getConstArray();
       nLength = aAnySeq.getLength();
       for ( sal_Int32 i = 0 ; i < nLength; i++)
       {
           if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Type" ) ) )
           {
               pValue[i].Value >>= m_sFilterName;
               break;
           }
       }
   }
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilter_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.WriterFilter" ) );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExportFilter"
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter_supportsService( const OUString& ServiceName ) throw (uno::RuntimeException)
{
   return (ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
           ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ));
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilter_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(2);
   OUString* pArray = aRet.getArray();
   pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
   pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
   return aRet;
}
#undef SERVICE_NAME1
#undef SERVICE_NAME2

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< uno::XInterface > WriterFilter_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new WriterFilter( xContext );
}

/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString WriterFilter::getImplementationName(  ) throw (uno::RuntimeException)
{
   return WriterFilter_getImplementationName();
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool WriterFilter::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    return WriterFilter_supportsService( rServiceName );
}
/*-- 09.06.2006 10:15:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > WriterFilter::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return WriterFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
