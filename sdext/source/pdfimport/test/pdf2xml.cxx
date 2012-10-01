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


#include "outputwrap.hxx"
#include "contentsink.hxx"
#include "pdfihelper.hxx"
#include "wrapper.hxx"
#include "pdfparse.hxx"
#include "../pdfiadaptor.hxx"

#include <sal/main.h>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::pdfi;
using namespace ::com::sun::star;

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    if( argc != 5 )
        return 1;

    ::rtl::OUString aBaseURL, aTmpURL, aSrcURL, aDstURL, aIniUrl;

    TreeVisitorFactorySharedPtr pTreeFactory;
    if( rtl_str_compare(argv[1], "-writer") == 0 )
        pTreeFactory = createWriterTreeVisitorFactory();
    else if( rtl_str_compare(argv[1], "-draw") == 0 )
        pTreeFactory = createDrawTreeVisitorFactory();
    else if( rtl_str_compare(argv[1], "-impress") == 0 )
        pTreeFactory = createImpressTreeVisitorFactory();
    else
        return 1;

    osl_getProcessWorkingDir(&aBaseURL.pData);
    osl_getFileURLFromSystemPath( rtl::OUString::createFromAscii(argv[2]).pData,
                                  &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aSrcURL.pData);

    osl_getFileURLFromSystemPath( rtl::OUString::createFromAscii(argv[3]).pData,
                                  &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aDstURL.pData);

    osl_getFileURLFromSystemPath( rtl::OUString::createFromAscii(argv[4]).pData,
                                &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aIniUrl.pData);

    // bootstrap UNO
    uno::Reference< lang::XMultiServiceFactory > xFactory;
    uno::Reference< uno::XComponentContext > xCtx;
    try
    {
        xCtx = ::cppu::defaultBootstrap_InitialComponentContext(aIniUrl);
        xFactory = uno::Reference< lang::XMultiServiceFactory >(  xCtx->getServiceManager(),
                                                                  uno::UNO_QUERY );
        if( xFactory.is() )
            ::comphelper::setProcessServiceFactory( xFactory );
    }
    catch( uno::Exception& )
    {
    }

    if( !xFactory.is() )
    {
        OSL_TRACE( "Could not bootstrap UNO, installation must be in disorder. Exiting." );
        return 1;
    }

    pdfi::PDFIRawAdaptor aAdaptor( xCtx );
    aAdaptor.setTreeVisitorFactory(pTreeFactory);
    aAdaptor.odfConvert( aSrcURL, new OutputWrap(aDstURL), NULL );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
