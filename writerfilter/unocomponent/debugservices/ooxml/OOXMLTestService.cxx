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



#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>

#include "OOXMLTestService.hxx"
#include <stdio.h>
#include <wchar.h>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <ucbhelper/contentbroker.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <osl/process.h>
#include <rtl/string.hxx>
#include <hash_set>
#include <assert.h>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <ooxml/OOXMLDocument.hxx>
#include <resourcemodel/TagLogger.hxx>

#include <ctype.h>

using namespace ::com::sun::star;

namespace writerfilter { namespace ooxmltest  {

const sal_Char ScannerTestService::SERVICE_NAME[40] = "debugservices.ooxml.ScannerTestService";
const sal_Char ScannerTestService::IMPLEMENTATION_NAME[40] = "debugservices.ooxml.ScannerTestService";




ScannerTestService::ScannerTestService(const uno::Reference< uno::XComponentContext > &xContext_) :
xContext( xContext_ )
{
}

sal_Int32 SAL_CALL ScannerTestService::run( const uno::Sequence< rtl::OUString >& aArguments ) throw (uno::RuntimeException)
{
    uno::Sequence<uno::Any> aUcbInitSequence(2);
    aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    aUcbInitSequence[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiComponentFactory> xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW );
    if (::ucbhelper::ContentBroker::initialize(xServiceFactory, aUcbInitSequence))
    {
#ifdef DEBUG_ELEMENT
        writerfilter::TagLogger::Pointer_t debugLogger
        (writerfilter::TagLogger::getInstance("DEBUG"));
        debugLogger->startDocument();
#endif

        rtl::OUString arg=aArguments[0];

        ::comphelper::setProcessServiceFactory(xServiceFactory);

        uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess
            (xFactory->createInstanceWithContext
             (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.ucb.SimpleFileAccess")),
              xContext), uno::UNO_QUERY_THROW );

        rtl_uString *dir=NULL;
        osl_getProcessWorkingDir(&dir);
        rtl::OUString absFileUrl;
        osl_getAbsoluteFileURL(dir, arg.pData, &absFileUrl.pData);
        rtl_uString_release(dir);

        uno::Reference<io::XInputStream> xInputStream =
            xFileAccess->openFileRead(absFileUrl);
        ooxml::OOXMLStream::Pointer_t pDocStream =
            ooxml::OOXMLDocumentFactory::createStream(xContext, xInputStream);

        ooxml::OOXMLDocument::Pointer_t pDocument
            (ooxml::OOXMLDocumentFactory::createDocument(pDocStream));

#if 0
        uno::Reference<text::XTextDocument> xDocument
            (xFactory->createInstanceWithContext
             (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.text.TextDocument")),
              xContext), uno::UNO_QUERY_THROW );
        uno::Reference<frame::XModel> xModel
            (xDocument, uno::UNO_QUERY_THROW);

        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier
            (xDocument, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XShapes> xShapes
            (xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY_THROW);

        pDocument->setModel(xModel);
        pDocument->setShapes(xShapes);
#endif

        Stream::Pointer_t pStream = createStreamHandler();
        pDocument->resolve(*pStream);

#ifdef DEBUG_ELEMENT
        writerfilter::TagLogger::dump("DEBUG");
        debugLogger->endDocument();
#endif

        ::ucbhelper::ContentBroker::deinitialize();
    }
    else
    {
        fprintf(stderr, "can't initialize UCB");
    }
    return 0;
}

::rtl::OUString ScannerTestService_getImplementationName ()
{
    return rtl::OUString::createFromAscii ( ScannerTestService::IMPLEMENTATION_NAME );
}

sal_Bool SAL_CALL ScannerTestService_supportsService( const ::rtl::OUString& ServiceName )
{
    return ServiceName.equals( rtl::OUString::createFromAscii( ScannerTestService::SERVICE_NAME ) );
}
uno::Sequence< rtl::OUString > SAL_CALL ScannerTestService_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    uno::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString::createFromAscii ( ScannerTestService::SERVICE_NAME );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL ScannerTestService_createInstance( const uno::Reference< uno::XComponentContext > & xContext) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new ScannerTestService( xContext );
}

} } /* end namespace writerfilter::ooxml */
