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


#include "fsfactory.hxx"
#include "cppuhelper/factory.hxx"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/processfactory.hxx>

#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/content.hxx>

#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>

#include "fsstorage.hxx"


using namespace ::com::sun::star;

uno::Sequence< OUString > SAL_CALL FSStorageFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    aRet[0] = OUString("com.sun.star.embed.FileSystemStorageFactory");
    aRet[1] = OUString("com.sun.star.comp.embed.FileSystemStorageFactory");
    return aRet;
}

OUString SAL_CALL FSStorageFactory::impl_staticGetImplementationName()
{
    return OUString("com.sun.star.comp.embed.FileSystemStorageFactory");
}

uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new FSStorageFactory( comphelper::getComponentContext(xServiceManager) ) );
}

uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstance()
    throw ( uno::Exception,
            uno::RuntimeException )
{
    OUString aTempURL;

    aTempURL = ::utl::TempFile( NULL, sal_True ).GetURL();

    if ( aTempURL.isEmpty() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    ::ucbhelper::Content aResultContent(
        aTempURL, uno::Reference< ucb::XCommandEnvironment >(),
        comphelper::getProcessComponentContext() );

    return uno::Reference< uno::XInterface >(
        static_cast< OWeakObject* >(
            new FSStorage(  aResultContent,
                            embed::ElementModes::READWRITE,
                            m_xContext ) ),
        uno::UNO_QUERY );
}

uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstanceWithArguments(
            const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException )
{
    // The request for storage can be done with up to three arguments

    // The first argument specifies a source for the storage
    // it must be URL.
    // The second value is a mode the storage should be open in.
    // And the third value is a media descriptor.

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
                ("second argument to css.embed.FileSystemStorageFactory."
                 "createInstanceWithArguments must be a"
                 " css.embed.ElementModes"),
                static_cast< OWeakObject * >(this), -1);
        }
        // it's always possible to read written storage in this implementation
        nStorageMode |= embed::ElementModes::READ;
    }

    // retrieve storage source URL
    OUString aURL;

    if ( !( aArguments[0] >>= aURL ) || aURL.isEmpty() )
    {
        throw lang::IllegalArgumentException(
            ("first argument to"
             " css.embed.FileSystemStorageFactory.createInstanceWithArguments"
             " must be a (non-empty) URL"),
            static_cast< OWeakObject * >(this), -1);
    }

    // allow to use other ucp's
    // if ( !isLocalNotFile_Impl( aURL ) )
    if ( aURL.equalsIgnoreAsciiCase("vnd.sun.star.pkg")
      || aURL.equalsIgnoreAsciiCase("vnd.sun.star.zip")
      || ::utl::UCBContentHelper::IsDocument( aURL ) )
    {
        throw lang::IllegalArgumentException(
            ("URL \"" + aURL + "\" passed as first argument to"
             " css.embed.FileSystemStorageFactory.createInstanceWithArguments"
             " must be a file URL denoting a directory"),
            static_cast< OWeakObject * >(this), -1);
    }

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && !( nStorageMode & embed::ElementModes::NOCREATE ) )
        FSStorage::MakeFolderNoUI( aURL );
    else if ( !::utl::UCBContentHelper::IsFolder( aURL ) )
        throw io::IOException(
            ("URL \"" + aURL + "\" passed to"
             " css.embed.FileSystemStorageFactory.createInstanceWithArguments"
             " does not denote an existing directory"),
            static_cast< OWeakObject * >(this));

    ::ucbhelper::Content aResultContent(
        aURL, uno::Reference< ucb::XCommandEnvironment >(),
        comphelper::getProcessComponentContext() );

    // create storage based on source
    return uno::Reference< uno::XInterface >(
        static_cast< OWeakObject* >( new FSStorage( aResultContent,
                                                    nStorageMode,
                                                    m_xContext ) ),
        uno::UNO_QUERY );
}

OUString SAL_CALL FSStorageFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

sal_Bool SAL_CALL FSStorageFactory::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName == aSeq[nInd] )
            return sal_True;

    return sal_False;
}

uno::Sequence< OUString > SAL_CALL FSStorageFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}


extern "C"
{
SAL_DLLPUBLIC_EXPORT void * SAL_CALL fsstorage_component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /* pRegistryKey */)
{
    void * pResult = 0;
    if (pServiceManager)
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        if (FSStorageFactory::impl_staticGetImplementationName().compareToAscii (pImplementationName) == 0)
        {
            xFactory = cppu::createOneInstanceFactory (
                reinterpret_cast< lang::XMultiServiceFactory* >(pServiceManager),
                FSStorageFactory::impl_staticGetImplementationName(),
                FSStorageFactory::impl_staticCreateSelfInstance,
                FSStorageFactory::impl_staticGetSupportedServiceNames() );
        }
        if (xFactory.is())
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
