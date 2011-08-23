/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *      Parts copyright 2005 by Sun Microsystems, Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "../svgreader.hxx"
#include "odfserializer.hxx"

#include <sal/main.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>
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

        explicit OutputWrap( const rtl::OUString& rURL ) : maFile(rURL)
        {
            maFile.open(osl_File_OpenFlag_Create|OpenFlag_Write);
        }

        virtual void SAL_CALL writeBytes( const com::sun::star::uno::Sequence< ::sal_Int8 >& aData ) throw (com::sun::star::io::NotConnectedException,com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)

        {
            sal_uInt64 nBytesWritten(0);
            maFile.write(aData.getConstArray(),aData.getLength(),nBytesWritten);
        }

        virtual void SAL_CALL flush() throw (com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
        {
        }

        virtual void SAL_CALL closeOutput() throw (com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
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

    ::rtl::OUString aBaseURL, aTmpURL, aSrcURL, aDstURL, aIniUrl;

    osl_getProcessWorkingDir(&aBaseURL.pData);
    osl_getFileURLFromSystemPath( rtl::OUString::createFromAscii(argv[1]).pData,
                                  &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aSrcURL.pData);

    osl_getFileURLFromSystemPath( rtl::OUString::createFromAscii(argv[2]).pData,
                                  &aTmpURL.pData );
    osl_getAbsoluteFileURL(aBaseURL.pData,aTmpURL.pData,&aDstURL.pData);

    osl_getFileURLFromSystemPath( rtl::OUString::createFromAscii(argv[3]).pData,
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
    catch( uno::Exception& )
    {
    }

    if( !xFactory.is() )
    {
        OSL_TRACE( "Could not bootstrap UNO, installation must be in disorder. Exiting." );
        return 1;
    }

    osl::File aInputFile(aSrcURL);
    if( osl::FileBase::E_None!=aInputFile.open(OpenFlag_Read) )
    {
        OSL_TRACE( "Cannot open input file" );
        return 1;
    }

    svgi::SVGReader aReader(xFactory,
                            uno::Reference<io::XInputStream>(
                                new comphelper::OSLInputStreamWrapper(aInputFile)),
                            svgi::createSerializer(new OutputWrap(aDstURL)));
    return aReader.parseAndConvert() ? 0 : 1;
}
