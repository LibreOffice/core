/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pdf2xml.cxx,v $
 *
 * $Revision: 1.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

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
        OSL_TRACE( "Could not bootstrap UNO, installation must be in disorder. Exiting.\n" );
        return 1;
    }

    pdfi::PDFIRawAdaptor aAdaptor( xCtx );
    aAdaptor.setTreeVisitorFactory(pTreeFactory);
    aAdaptor.odfConvert( aSrcURL, new OutputWrap(aDstURL), NULL );

    return 0;
}
