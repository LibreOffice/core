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

#include "../svgreader.hxx"
#include "odfserializer.hxx"

#include <sal/main.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/oslfile2streamwrap.hxx>

using namespace ::com::sun::star;

namespace
{
    class OutputWrap : public cppu::WeakImplHelper1<
        io::XOutputStream>
    {
        osl::File maFile;

    public:

        explicit OutputWrap( const OUString& rURL ) : maFile(rURL)
        {
            maFile.open( osl_File_OpenFlag_Create|osl_File_OpenFlag_Write );
        }

        virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (com::sun::star::io::NotConnectedException,com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE

        {
            sal_uInt64 nBytesWritten(0);
            maFile.write(aData.getConstArray(),aData.getLength(),nBytesWritten);
        }

        virtual void SAL_CALL flush() throw (com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
        }

        virtual void SAL_CALL closeOutput() throw (com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            maFile.close();
        }
    };
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    if( argc != 4 )
    {
        OSL_TRACE( "Invocation: svg2odf <base_url> <dst_url> <ini_file>. Exiting" );
        return 1;
    }

    OUString aBaseURL, aTmpURL, aSrcURL, aDstURL, aIniUrl;

    osl_getProcessWorkingDir(&aBaseURL.pData);
    osl_getFileURLFromSystemPath( OUString::createFromAscii(argv[1]).pData,
                                  &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aSrcURL.pData);

    osl_getFileURLFromSystemPath( OUString::createFromAscii(argv[2]).pData,
                                  &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aDstURL.pData);

    osl_getFileURLFromSystemPath( OUString::createFromAscii(argv[3]).pData,
                                &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aIniUrl.pData);

    // bootstrap UNO
    uno::Reference< lang::XMultiServiceFactory > xFactory;
    uno::Reference< uno::XComponentContext > xCtx;
    try
    {
        xCtx = ::cppu::defaultBootstrap_InitialComponentContext(aIniUrl);
        xFactory = uno::Reference< lang::XMultiServiceFactory >(xCtx->getServiceManager(),
                                                                uno::UNO_QUERY);
        if( xFactory.is() )
            ::comphelper::setProcessServiceFactory( xFactory );
    }
    catch( const uno::Exception& )
    {
    }

    if( !xFactory.is() )
    {
        OSL_TRACE( "Could not bootstrap UNO, installation must be in disorder. Exiting." );
        return 1;
    }

    osl::File aInputFile(aSrcURL);
    if( osl::FileBase::E_None!=aInputFile.open(osl_File_OpenFlag_Read) )
    {
        OSL_TRACE( "Cannot open input file" );
        return 1;
    }

    svgi::SVGReader aReader(xCtx,
                            uno::Reference<io::XInputStream>(
                                new comphelper::OSLInputStreamWrapper(aInputFile)),
                            svgi::createSerializer(new OutputWrap(aDstURL)));
    return aReader.parseAndConvert() ? 0 : 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
