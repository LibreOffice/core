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


#include <fsfactory.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/content.hxx>

#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>

#include "fsstorage.hxx"


using namespace ::com::sun::star;


uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstance()
{
    OUString aTempURL = ::utl::CreateTempURL( nullptr, true );
    if ( aTempURL.isEmpty() )
        throw uno::RuntimeException(u"Cannot create tempfile."_ustr);

    ::ucbhelper::Content aResultContent(
        aTempURL, uno::Reference< ucb::XCommandEnvironment >(),
        comphelper::getProcessComponentContext() );

    return cppu::getXWeak(
            new FSStorage(  aResultContent,
                            embed::ElementModes::READWRITE,
                            m_xContext ) );
}

/**
 * The request for storage can be done with up to three arguments.
 * The first argument specifies a source for the storage it must be URL.
 * The second value is a mode the storage should be open in.
 * The third value is a media descriptor.
 */
uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstanceWithArguments(
            const uno::Sequence< uno::Any >& aArguments )
{
    sal_Int32 nArgNum = aArguments.getLength();
    OSL_ENSURE( nArgNum < 4, "Wrong parameter number" );

    if ( !nArgNum )
        return createInstance();

    // first try to retrieve storage open mode if any
    // by default the storage will be open in readonly mode
    sal_Int32 nStorageMode = embed::ElementModes::READ;
    if ( nArgNum >= 2 )
    {
        if( !( aArguments[1] >>= nStorageMode ) )
        {
            throw lang::IllegalArgumentException(
                (u"second argument to css.embed.FileSystemStorageFactory."
                 "createInstanceWithArguments must be a"
                 " css.embed.ElementModes"_ustr),
                getXWeak(), -1);
        }
        // it's always possible to read written storage in this implementation
        nStorageMode |= embed::ElementModes::READ;
    }

    // retrieve storage source URL
    OUString aURL;

    if ( !( aArguments[0] >>= aURL ) || aURL.isEmpty() )
    {
        throw lang::IllegalArgumentException(
            (u"first argument to"
             " css.embed.FileSystemStorageFactory.createInstanceWithArguments"
             " must be a (non-empty) URL"_ustr),
            getXWeak(), -1);
    }

    // allow to use other ucp's
    // if ( !isLocalNotFile_Impl( aURL ) )
    if ( aURL.startsWithIgnoreAsciiCase("vnd.sun.star.pkg:")
      || aURL.startsWithIgnoreAsciiCase("vnd.sun.star.zip:")
      || ::utl::UCBContentHelper::IsDocument( aURL ) )
    {
        throw lang::IllegalArgumentException(
            ("URL \"" + aURL + "\" passed as first argument to"
             " css.embed.FileSystemStorageFactory.createInstanceWithArguments"
             " must be a file URL denoting a directory"),
            getXWeak(), -1);
    }

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && !( nStorageMode & embed::ElementModes::NOCREATE ) )
        FSStorage::MakeFolderNoUI( aURL );
    else if ( !::utl::UCBContentHelper::IsFolder( aURL ) )
        throw io::IOException(
            ("URL \"" + aURL + "\" passed to"
             " css.embed.FileSystemStorageFactory.createInstanceWithArguments"
             " does not denote an existing directory"),
            getXWeak());

    ::ucbhelper::Content aResultContent(
        aURL, uno::Reference< ucb::XCommandEnvironment >(),
        comphelper::getProcessComponentContext() );

    // create storage based on source
    return cppu::getXWeak( new FSStorage( aResultContent,
                                                    nStorageMode,
                                                    m_xContext ) );
}

OUString SAL_CALL FSStorageFactory::getImplementationName()
{
    return u"com.sun.star.comp.embed.FileSystemStorageFactory"_ustr;
}

sal_Bool SAL_CALL FSStorageFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL FSStorageFactory::getSupportedServiceNames()
{
    return { u"com.sun.star.embed.FileSystemStorageFactory"_ustr,
                u"com.sun.star.comp.embed.FileSystemStorageFactory"_ustr };
}



extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
svl_FSStorageFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new FSStorageFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
