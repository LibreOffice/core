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

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XOutputStream.hpp>


#include "xpackcreator.hxx"

#include <comphelper/processfactory.hxx>
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
    aRet[0] = ::rtl::OUString("com.sun.star.embed.PackageStructureCreator");
    aRet[1] = ::rtl::OUString("com.sun.star.comp.embed.PackageStructureCreator");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPackageStructureCreator::impl_getStaticImplementationName()
{
    return ::rtl::OUString("com.sun.star.comp.embed.PackageStructureCreator");
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
    if( ::ucbhelper::Content::create( aFolderUrl, xComEnv, comphelper::getProcessComponentContext(), aContent ) )
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

                if ( !aTempURL.isEmpty() )
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
        catch (const uno::RuntimeException&)
        {
            if ( pTempStream )
                delete pTempStream;

            if ( !aTempURL.isEmpty() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch (const io::IOException&)
        {
            if ( pTempStream )
                delete pTempStream;

            if ( !aTempURL.isEmpty() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch (const uno::Exception&)
        {
        }

        if ( pTempStream )
            delete pTempStream;

        if ( !aTempURL.isEmpty() )
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
