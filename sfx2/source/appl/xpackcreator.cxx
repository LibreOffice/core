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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XOutputStream.hpp>


#include "xpackcreator.hxx"

#include <sot/stg.hxx>
#include <sot/storage.hxx>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OPackageStructureCreator::impl_getStaticSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.PackageStructureCreator"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.PackageStructureCreator"));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPackageStructureCreator::impl_getStaticImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.PackageStructureCreator"));
}

//-------------------------------------------------------------------------
uno::Reference< lang::XSingleServiceFactory > SAL_CALL OPackageStructureCreator::impl_createFactory(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return ::cppu::createOneInstanceFactory( xServiceManager,
                                OPackageStructureCreator::impl_getStaticImplementationName(),
                                OPackageStructureCreator::impl_staticCreateSelfInstance,
                                OPackageStructureCreator::impl_getStaticSupportedServiceNames() );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OPackageStructureCreator::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OPackageStructureCreator( xServiceManager ) );
}


//-------------------------------------------------------------------------
void SAL_CALL OPackageStructureCreator::convertToPackage( const ::rtl::OUString& aFolderUrl,
                                                          const uno::Reference< io::XOutputStream >& xTargetStream )
        throw ( io::IOException,
                uno::RuntimeException )
{
    uno::Reference< ucb::XCommandEnvironment > xComEnv;

    if ( !xTargetStream.is() )
        throw io::IOException(); // TODO/LATER

    sal_Bool bSuccess = sal_False;
    ::ucbhelper::Content aContent;
    if( ::ucbhelper::Content::create( aFolderUrl, xComEnv, aContent ) )
    {
        SvStream* pTempStream = NULL;

        ::rtl::OUString aTempURL = ::utl::TempFile().GetURL();
        try {
            if ( aContent.isFolder() )
            {
                UCBStorage* pUCBStorage = new UCBStorage( aContent,
                                                          aFolderUrl,
                                                          STREAM_READ,
                                                          sal_False,
                                                          sal_True );
                SotStorageRef aStorage = new SotStorage( pUCBStorage );

                if ( aTempURL.getLength() )
                {
                    pTempStream = new SvFileStream( aTempURL, STREAM_STD_READWRITE );
                    SotStorageRef aTargetStorage = new SotStorage( sal_True, *pTempStream );
                    aStorage->CopyTo( aTargetStorage );
                    aTargetStorage->Commit();

                    if ( aStorage->GetError() || aTargetStorage->GetError() || pTempStream->GetError() )
                        throw io::IOException();

                    aTargetStorage = NULL;
                    aStorage = NULL;

                    pTempStream->Seek( 0 );

                    uno::Sequence< sal_Int8 > aSeq( 32000 );
                    sal_uInt32 nRead = 0;
                    do {
                        if ( aSeq.getLength() < 32000 )
                            aSeq.realloc( 32000 );

                        nRead = pTempStream->Read( aSeq.getArray(), 32000 );
                        if ( nRead < 32000 )
                            aSeq.realloc( nRead );
                        xTargetStream->writeBytes( aSeq );
                    } while( !pTempStream->IsEof() && !pTempStream->GetError() && nRead );

                    if ( pTempStream->GetError() )
                        throw io::IOException();

                    bSuccess = sal_True;
                }
            }
        }
        catch ( uno::RuntimeException& )
        {
            if ( pTempStream )
                delete pTempStream;

            if ( aTempURL.getLength() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch ( io::IOException& )
        {
            if ( pTempStream )
                delete pTempStream;

            if ( aTempURL.getLength() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch ( uno::Exception& )
        {
        }

        if ( pTempStream )
            delete pTempStream;

        if ( aTempURL.getLength() )
            ::utl::UCBContentHelper::Kill( aTempURL );
    }

    if ( !bSuccess )
        throw io::IOException(); // TODO/LATER: can't proceed with creation
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPackageStructureCreator::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OPackageStructureCreator::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_getStaticSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OPackageStructureCreator::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
