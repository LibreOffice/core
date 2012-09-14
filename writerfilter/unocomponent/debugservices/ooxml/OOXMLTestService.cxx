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

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "OOXMLTestService.hxx"
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
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/seqstream.hxx>
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

sal_Int32 SAL_CALL ScannerTestService::run( const uno::Sequence< OUString >& aArguments ) throw (uno::RuntimeException)
{
#ifdef DEBUG_ELEMENT
    writerfilter::TagLogger::Pointer_t debugLogger
        (writerfilter::TagLogger::getInstance("DEBUG"));
    debugLogger->startDocument();
#endif

    OUString arg=aArguments[0];

    uno::Reference<ucb::XSimpleFileAccess2> xFileAccess(ucb::SimpleFileAccess::create(xContext));

    rtl_uString *dir=NULL;
    osl_getProcessWorkingDir(&dir);
    OUString absFileUrl;
    osl_getAbsoluteFileURL(dir, arg.pData, &absFileUrl.pData);
    rtl_uString_release(dir);

    uno::Reference<io::XInputStream> xInputStream =
        xFileAccess->openFileRead(absFileUrl);
    ooxml::OOXMLStream::Pointer_t pDocStream =
        ooxml::OOXMLDocumentFactory::createStream(xContext, xInputStream);

    ooxml::OOXMLDocument::Pointer_t pDocument
        (ooxml::OOXMLDocumentFactory::createDocument(pDocStream));

    Stream::Pointer_t pStream = createStreamHandler();
    pDocument->resolve(*pStream);

#ifdef DEBUG_ELEMENT
    debugLogger->endDocument();
#endif

    return 0;
}

OUString ScannerTestService_getImplementationName ()
{
    return OUString(ScannerTestService::IMPLEMENTATION_NAME );
}

uno::Sequence< OUString > SAL_CALL ScannerTestService_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    uno::Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString(ScannerTestService::SERVICE_NAME );
    return aRet;
}

uno::Reference< uno::XInterface > SAL_CALL ScannerTestService_createInstance( const uno::Reference< uno::XComponentContext > & xContext) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new ScannerTestService( xContext );
}

} } /* end namespace writerfilter::ooxml */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
