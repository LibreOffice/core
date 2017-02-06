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
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/embed/XPackageStructureCreator.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <sot/stg.hxx>
#include <sot/storage.hxx>
#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>

using namespace css;

namespace {

class OPackageStructureCreator : public ::cppu::WeakImplHelper< embed::XPackageStructureCreator,
                                                                lang::XServiceInfo >
{
public:
    OPackageStructureCreator() {}

    // XPackageStructureCreator
    virtual void SAL_CALL convertToPackage( const OUString& aFolderUrl, const uno::Reference< io::XOutputStream >& xTargetStream ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};


void SAL_CALL OPackageStructureCreator::convertToPackage( const OUString& aFolderUrl,
                                                          const uno::Reference< io::XOutputStream >& xTargetStream )
{
    uno::Reference< ucb::XCommandEnvironment > xComEnv;

    if ( !xTargetStream.is() )
        throw io::IOException(); // TODO/LATER

    bool bSuccess = false;
    ::ucbhelper::Content aContent;
    if( ::ucbhelper::Content::create( aFolderUrl, xComEnv, comphelper::getProcessComponentContext(), aContent ) )
    {
        SvStream* pTempStream = nullptr;

        OUString aTempURL = ::utl::TempFile().GetURL();
        try {
            if ( aContent.isFolder() )
            {
                UCBStorage* pUCBStorage = new UCBStorage( aContent,
                                                          aFolderUrl,
                                                          StreamMode::READ,
                                                          false,
                                                          true );
                tools::SvRef<SotStorage> aStorage = new SotStorage( pUCBStorage );

                if ( !aTempURL.isEmpty() )
                {
                    pTempStream = new SvFileStream( aTempURL, StreamMode::STD_READWRITE );
                    tools::SvRef<SotStorage> aTargetStorage = new SotStorage( true, *pTempStream );
                    aStorage->CopyTo( aTargetStorage.get() );
                    aTargetStorage->Commit();

                    if ( aStorage->GetError() || aTargetStorage->GetError() || pTempStream->GetError() )
                        throw io::IOException();

                    aTargetStorage = nullptr;
                    aStorage = nullptr;

                    pTempStream->Seek( 0 );

                    uno::Sequence< sal_Int8 > aSeq( 32000 );
                    sal_uInt32 nRead = 0;
                    do {
                        if ( aSeq.getLength() < 32000 )
                            aSeq.realloc( 32000 );

                        nRead = pTempStream->ReadBytes(aSeq.getArray(), 32000);
                        if ( nRead < 32000 )
                            aSeq.realloc( nRead );
                        xTargetStream->writeBytes( aSeq );
                    } while( !pTempStream->IsEof() && !pTempStream->GetError() && nRead );

                    if ( pTempStream->GetError() )
                        throw io::IOException();

                    bSuccess = true;
                }
            }
        }
        catch (const uno::RuntimeException&)
        {
            delete pTempStream;

            if ( !aTempURL.isEmpty() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch (const io::IOException&)
        {
            delete pTempStream;

            if ( !aTempURL.isEmpty() )
                ::utl::UCBContentHelper::Kill( aTempURL );

            throw;
        }
        catch (const uno::Exception&)
        {
        }

        delete pTempStream;

        if ( !aTempURL.isEmpty() )
            ::utl::UCBContentHelper::Kill( aTempURL );
    }

    if ( !bSuccess )
        throw io::IOException(); // TODO/LATER: can't proceed with creation
}

OUString SAL_CALL OPackageStructureCreator::getImplementationName()
{
    return OUString("com.sun.star.comp.embed.PackageStructureCreator");
}

sal_Bool SAL_CALL OPackageStructureCreator::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OPackageStructureCreator::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    aRet[0] = "com.sun.star.embed.PackageStructureCreator";
    aRet[1] = "com.sun.star.comp.embed.PackageStructureCreator";
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_embed_PackageStructureCreator_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OPackageStructureCreator());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
