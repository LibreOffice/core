/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            ::oox::core::FilterDetect aDetector( m_xContext );
            xInputStream = aDetector.extractUnencryptedPackage( aMediaDesc );
        }
        catch( uno::Exception& e)
        {
            (void) e;
        }

        if ( !xInputStream.is() )
        {
            return sal_False;
        }

#ifdef DEBUG_IMPORT
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
         m_sFilterName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "writer_MS_Word_2007_Template" ) )) ?
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

#ifdef DEBUG_IMPORT
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

