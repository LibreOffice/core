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
#include "precompiled_svl.hxx"

#include "fsfactory.hxx"
#include "cppuhelper/factory.hxx"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XSeekable.hpp>


#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/content.hxx>

#include <unotools/tempfile.hxx>
#include <unotools/ucbhelper.hxx>

#include "fsstorage.hxx"


using namespace ::com::sun::star;

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL FSStorageFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.FileSystemStorageFactory"));
    aRet[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.FileSystemStorageFactory"));
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL FSStorageFactory::impl_staticGetImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.embed.FileSystemStorageFactory"));
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new FSStorageFactory( xServiceManager ) );
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL FSStorageFactory::createInstance()
    throw ( uno::Exception,
            uno::RuntimeException )
{
    ::rtl::OUString aTempURL;

    aTempURL = ::utl::TempFile( NULL, sal_True ).GetURL();

    if ( !aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    ::ucbhelper::Content aResultContent(
        aTempURL, uno::Reference< ucb::XCommandEnvironment >() );

    return uno::Reference< uno::XInterface >(
        static_cast< OWeakObject* >(
            new FSStorage(  aResultContent,
                            embed::ElementModes::READWRITE,
                            uno::Sequence< beans::PropertyValue >(),
                            m_xFactory ) ),
        uno::UNO_QUERY );
}

//-------------------------------------------------------------------------
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
            OSL_ENSURE( sal_False, "Wrong second argument!\n" );
            throw uno::Exception(); // TODO: Illegal argument
        }
        // it's allways possible to read written storage in this implementation
        nStorageMode |= embed::ElementModes::READ;
    }

    // retrieve storage source URL
    ::rtl::OUString aURL;

    if ( aArguments[0] >>= aURL )
    {
        if ( !aURL.getLength() )
        {
            OSL_ENSURE( sal_False, "Empty URL is provided!\n" );
            throw uno::Exception(); // TODO: illegal argument
        }
    }
    else
    {
        OSL_ENSURE( sal_False, "Wrong first argument!\n" );
        throw uno::Exception(); // TODO: Illegal argument
    }

    // retrieve mediadescriptor and set storage properties
    uno::Sequence< beans::PropertyValue > aDescr;
    uno::Sequence< beans::PropertyValue > aPropsToSet;

    if ( nArgNum >= 3 )
    {
        if( aArguments[2] >>= aDescr )
        {
            aPropsToSet.realloc(1);
            aPropsToSet[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
            aPropsToSet[0].Value <<= aURL;

            for ( sal_Int32 nInd = 0, nNumArgs = 1; nInd < aDescr.getLength(); nInd++ )
            {
                if ( aDescr[nInd].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "InteractionHandler" ) ) )
                {
                    aPropsToSet.realloc( ++nNumArgs );
                    aPropsToSet[nNumArgs-1].Name = aDescr[nInd].Name;
                    aPropsToSet[nNumArgs-1].Value = aDescr[nInd].Value;
                    break;
                }
                else
                    OSL_ENSURE( sal_False, "Unacceptable property, will be ignored!\n" );
            }
        }
        else
        {
            OSL_ENSURE( sal_False, "Wrong third argument!\n" );
            throw uno::Exception(); // TODO: Illegal argument
        }
    }

    // allow to use other ucp's
    // if ( !isLocalNotFile_Impl( aURL ) )
    if ( aURL.equalsIgnoreAsciiCaseAsciiL( "vnd.sun.star.pkg", 16 )
      || aURL.equalsIgnoreAsciiCaseAsciiL( "vnd.sun.star.zip", 16 )
      || ::utl::UCBContentHelper::IsDocument( aURL ) )
    {
        OSL_ENSURE( sal_False, "File system storages can be based only on file URLs!\n" ); // ???
        throw uno::Exception(); // TODO: illegal argument
    }

    if ( ( nStorageMode & embed::ElementModes::WRITE ) && !( nStorageMode & embed::ElementModes::NOCREATE ) )
        FSStorage::MakeFolderNoUI( aURL, sal_False );
    else if ( !::utl::UCBContentHelper::IsFolder( aURL ) )
        throw io::IOException(); // there is no such folder

    ::ucbhelper::Content aResultContent(
        aURL, uno::Reference< ucb::XCommandEnvironment >() );

    // create storage based on source
    return uno::Reference< uno::XInterface >(
        static_cast< OWeakObject* >( new FSStorage( aResultContent,
                                                    nStorageMode,
                                                    aPropsToSet,
                                                    m_xFactory ) ),
        uno::UNO_QUERY );
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL FSStorageFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL FSStorageFactory::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL FSStorageFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

//-------------------------------------------------------------------------

extern "C"
{
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplementationName, void * pServiceManager, void * /* pRegistryKey */)
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
