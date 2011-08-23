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

#include <comphelper/processfactory.hxx>

#include "OOXMLAnalyzeService.hxx"
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
#include <resourcemodel/WW8ResourceModel.hxx>
#include <ooxml/OOXMLDocument.hxx>

#include <ctype.h>

using namespace ::com::sun::star;

namespace writerfilter { namespace ooxmltest  {

const sal_Char AnalyzeService::SERVICE_NAME[40] = "debugservices.ooxml.AnalyzeService";
const sal_Char AnalyzeService::IMPLEMENTATION_NAME[40] = "debugservices.ooxml.AnalyzeService";

class URLLister
{
    uno::Reference<io::XInputStream> xInputStream;
    rtl::OUString mString;
    rtl::OUString mCRLF;
    rtl::OUString mLF;

    sal_uInt32 getEOLIndex()
    {
        sal_Int32 nIndex = mString.indexOf(mCRLF);

        if (nIndex == -1)
        {
            nIndex = mString.indexOf(mLF);            
        }

        return nIndex;
    }

public:
    URLLister(uno::Reference<com::sun::star::uno::XComponentContext> xContext,
              uno::Reference<lang::XMultiComponentFactory> xFactory,
              rtl::OUString absFileUrl)
    {
        uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess
            (xFactory->createInstanceWithContext
             (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                              ("com.sun.star.ucb.SimpleFileAccess")), 
              xContext), uno::UNO_QUERY_THROW);
        xInputStream = xFileAccess->openFileRead(absFileUrl) ;

        mLF = rtl::OUString::createFromAscii("\n");
        mCRLF = rtl::OUString::createFromAscii("\r\n");
    }

    rtl::OUString getURL()
    {
        rtl::OUString aResult;

        sal_Int32 nIndex = getEOLIndex();

        while (nIndex == -1)
        {
            uno::Sequence<sal_Int8> aSeq;
            sal_uInt32 nCount = xInputStream->readBytes(aSeq, 1024);

            if (nCount > 0)
            {
                rtl_String * pNew = 0;
                rtl_string_newFromStr_WithLength
                    (&pNew, reinterpret_cast<const sal_Char *>(&aSeq[0]),
                     nCount);

                rtl::OString aTmp(pNew);
                rtl::OUString aTail(rtl::OStringToOUString
                                    (aTmp, RTL_TEXTENCODING_ASCII_US));
                mString = mString.concat(aTail);
            }

            nIndex = getEOLIndex();

            if (nCount != 1024)
                break;
        }

        if (nIndex > 0)
        {
            aResult = mString.copy(0, nIndex);

            if (mString.match(mCRLF, nIndex))
                mString = mString.copy(nIndex + 2);
            else
                mString = mString.copy(nIndex + 1);
        }

        return aResult;
    }
};

AnalyzeService::AnalyzeService(const uno::Reference< uno::XComponentContext > &xContext_) :
xContext( xContext_ )
{
}

sal_Int32 SAL_CALL AnalyzeService::run
( const uno::Sequence< rtl::OUString >& aArguments ) 
    throw (uno::RuntimeException)
{
    uno::Sequence<uno::Any> aUcbInitSequence(2);
    aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
    aUcbInitSequence[1] <<= 
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
    uno::Reference<lang::XMultiServiceFactory> 
        xServiceFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);
    uno::Reference<lang::XMultiComponentFactory> 
        xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW );

    if (::ucbhelper::ContentBroker::initialize(xServiceFactory, aUcbInitSequence))
    {
        ::comphelper::setProcessServiceFactory(xServiceFactory);

        rtl::OUString arg=aArguments[0];

        rtl_uString *dir=NULL;
        osl_getProcessWorkingDir(&dir);
                
        rtl::OUString absFileUrlUrls;
        osl_getAbsoluteFileURL(dir, arg.pData, &absFileUrlUrls.pData);

        URLLister aLister(xContext, xFactory, absFileUrlUrls);

        fprintf(stdout, "<analyze>\n");

        writerfilter::analyzerIds();

        rtl::OUString aURL = aLister.getURL();

        while (aURL.getLength() > 0)
        {
            uno::Reference<com::sun::star::ucb::XSimpleFileAccess> xFileAccess
                (xFactory->createInstanceWithContext
                 (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM
                                  ("com.sun.star.ucb.SimpleFileAccess")), 
                  xContext), uno::UNO_QUERY_THROW );
            
            rtl::OString aStr;
            aURL.convertToString(&aStr, RTL_TEXTENCODING_ASCII_US,
                                 OUSTRING_TO_OSTRING_CVTFLAGS);
          
            fprintf(stdout, "<file><name>%s</name>\n", aStr.getStr());
            fprintf(stderr, "%s\n", aStr.getStr());
            fflush(stderr);
        
            bool bStatus = true;
            try 
            {
                uno::Reference<io::XInputStream> xInputStream = 
                    xFileAccess->openFileRead(aURL);

                if (xInputStream.is())
                {
                    ooxml::OOXMLStream::Pointer_t pDocStream = 
                        ooxml::OOXMLDocumentFactory::createStream
                        (xContext, xInputStream);
                    
                    if (pDocStream.get() != NULL)
                    {
                        ooxml::OOXMLDocument::Pointer_t pDocument
                            (ooxml::OOXMLDocumentFactory::createDocument
                             (pDocStream));
                        
                        Stream::Pointer_t pAnalyzer = 
                            writerfilter::createAnalyzer();
                        pDocument->resolve(*pAnalyzer);
                    }
                    else
                    {
                        fprintf(stdout, 
                                "<exception>file open failed</exception>\n");
                        bStatus = false;
                    }
                    fprintf(stderr, "done\n");
                }

                xInputStream->closeInput();
            }
            catch (...)
            {
                fprintf(stdout, "<exception>unknown</exception>\n");
                bStatus = false;
            }

            if (bStatus)
                fprintf(stdout, "<status>ok</status>\n");
            else
                fprintf(stdout, "<status>failed</status>\n");

            aURL = aLister.getURL();

            fprintf(stdout, "</file>\n");
            fflush(stdout);
        }
        
        fprintf(stdout, "</analyze>\n");

        rtl_uString_release(dir);
        ::ucbhelper::ContentBroker::deinitialize();

        
    }
    else
    {
        fprintf(stdout, "can't initialize UCB");
    }
    return 0;
}

::rtl::OUString AnalyzeService_getImplementationName ()
{
    return rtl::OUString::createFromAscii ( AnalyzeService::IMPLEMENTATION_NAME );
}

sal_Bool SAL_CALL AnalyzeService_supportsService( const ::rtl::OUString& ServiceName )
{
    return ServiceName.equals( rtl::OUString::createFromAscii( AnalyzeService::SERVICE_NAME ) );
}
uno::Sequence< rtl::OUString > SAL_CALL AnalyzeService_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    uno::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString::createFromAscii ( AnalyzeService::SERVICE_NAME );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL AnalyzeService_createInstance( const uno::Reference< uno::XComponentContext > & xContext) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new AnalyzeService( xContext );
}

} } /* end namespace writerfilter::doctok */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
