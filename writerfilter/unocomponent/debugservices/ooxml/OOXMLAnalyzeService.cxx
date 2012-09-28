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

#include "OOXMLAnalyzeService.hxx"
#include <stdio.h>
#include <wchar.h>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <osl/process.h>
#include <rtl/string.hxx>
#include <boost/unordered_set.hpp>
#include <assert.h>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/seqstream.hxx>
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
    OUString mString;
    OUString mCRLF;
    OUString mLF;

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
              OUString absFileUrl)
    {
        uno::Reference<ucb::XSimpleFileAccess2> xFileAccess(ucb::SimpleFileAccess::create(xContext));
        xInputStream = xFileAccess->openFileRead(absFileUrl) ;

        mLF = OUString("\n");
        mCRLF = OUString("\r\n");
    }

    OUString getURL()
    {
        OUString aResult;

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

                OString aTmp(pNew);
                OUString aTail(OStringToOUString
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
( const uno::Sequence< OUString >& aArguments )
    throw (uno::RuntimeException)
{
    OUString arg=aArguments[0];

    rtl_uString *dir=NULL;
    osl_getProcessWorkingDir(&dir);

    OUString absFileUrlUrls;
    osl_getAbsoluteFileURL(dir, arg.pData, &absFileUrlUrls.pData);

    URLLister aLister(xContext, absFileUrlUrls);

    fprintf(stdout, "<analyze>\n");

    writerfilter::analyzerIds();

    OUString aURL = aLister.getURL();

    while (!aURL.isEmpty())
    {
        uno::Reference<ucb::XSimpleFileAccess2> xFileAccess(ucb::SimpleFileAccess::create(xContext));

        OString aStr;
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
                    (xContext, xInputStream, false);

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

    return 0;
}

OUString AnalyzeService_getImplementationName ()
{
    return OUString(AnalyzeService::IMPLEMENTATION_NAME );
}

sal_Bool SAL_CALL AnalyzeService_supportsService( const OUString& ServiceName )
{
    return ServiceName == AnalyzeService::SERVICE_NAME;
}
uno::Sequence< OUString > SAL_CALL AnalyzeService_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    uno::Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString(AnalyzeService::SERVICE_NAME );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL AnalyzeService_createInstance( const uno::Reference< uno::XComponentContext > & xContext) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new AnalyzeService( xContext );
}

} } /* end namespace writerfilter::doctok */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
