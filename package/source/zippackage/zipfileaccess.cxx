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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <zipfileaccess.hxx>
#include <ZipEnumeration.hxx>
#include <ZipPackageSink.hxx>
#include <EncryptionData.hxx>

#include <ucbhelper/content.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

OZipFileAccess::OZipFileAccess( const uno::Reference< uno::XComponentContext >& rxContext )
: m_aMutexHolder( new SotMutexHolder )
, m_xContext( rxContext )
, m_pZipFile( nullptr )
, m_pListenersContainer( nullptr )
, m_bDisposed( false )
, m_bOwnContent( false )
{
    if ( !rxContext.is() )
        throw uno::RuntimeException(THROW_WHERE );
}

OZipFileAccess::~OZipFileAccess()
{
    {
        ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );
        if ( !m_bDisposed )
        {
            try {
                m_refCount++; // dispose will use refcounting so the further destruction must be avoided
                dispose();
            } catch( uno::Exception& )
            {}
        }
    }
}

uno::Sequence< OUString > OZipFileAccess::GetPatternsFromString_Impl( const OUString& aString )
{
    if ( aString.isEmpty() )
        return uno::Sequence< OUString >();

    uno::Sequence< OUString > aPattern( 1 );
    sal_Int32 nInd = 0;

    const sal_Unicode* pString = aString.getStr();
    while( *pString )
    {
        if ( *pString == (sal_Unicode)'\\' )
        {
            pString++;

            if ( *pString == (sal_Unicode)'\\' )
            {
                aPattern[nInd] += "\\";
                pString++;
            }
            else if ( *pString == (sal_Unicode)'*' )
            {
                aPattern[nInd] += "*";
                pString++;
            }
            else
            {
                OSL_FAIL( "The backslash is not guarded!\n" );
                aPattern[nInd] += "\\";
            }
        }
        else if ( *pString == (sal_Unicode)'*' )
        {
            aPattern.realloc( ( ++nInd ) + 1 );
            pString++;
        }
        else
        {
            aPattern[nInd] += OUString( *pString );
            pString++;
        }
    }

    return aPattern;
}

bool OZipFileAccess::StringGoodForPattern_Impl( const OUString& aString,
                                                    const uno::Sequence< OUString >& aPattern )
{
    sal_Int32 nInd = aPattern.getLength() - 1;
    if ( nInd < 0 )
        return false;

    if ( nInd == 0 )
    {
        if ( aPattern[0].isEmpty() )
            return true;

        return aString.equals( aPattern[0] );
    }

    sal_Int32 nBeginInd = aPattern[0].getLength();
    sal_Int32 nEndInd = aString.getLength() - aPattern[nInd].getLength();
    if ( nEndInd >= nBeginInd
      && ( nEndInd == aString.getLength() || aString.copy( nEndInd ).equals( aPattern[nInd] ) )
      && ( nBeginInd == 0 || aString.copy( 0, nBeginInd ).equals( aPattern[0] ) ) )
    {
        for ( sal_Int32 nCurInd = aPattern.getLength() - 2; nCurInd > 0; nCurInd-- )
        {
            if ( aPattern[nCurInd].isEmpty() )
                continue;

            if ( nEndInd == nBeginInd )
                return false;

            // check that search does not use nEndInd position
            sal_Int32 nLastInd = aString.lastIndexOf( aPattern[nCurInd], nEndInd - 1 );

            if ( nLastInd == -1 )
                return false;

            if ( nLastInd < nBeginInd )
                return false;

            nEndInd = nLastInd;
        }

        return true;
    }

    return false;
}

// XInitialization
void SAL_CALL OZipFileAccess::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( m_pZipFile )
        throw uno::RuntimeException(THROW_WHERE ); // initialization is allowed only one time

    if ( !aArguments.getLength() )
        throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

    OSL_ENSURE( aArguments.getLength() == 1, "Too many arguments are provided, only the first one will be used!\n" );

    OUString aParamURL;
    uno::Reference< io::XStream > xStream;
    uno::Reference< io::XSeekable > xSeekable;

    if ( ( aArguments[0] >>= aParamURL ) )
    {
        ::ucbhelper::Content aContent(
            aParamURL,
            uno::Reference< css::ucb::XCommandEnvironment >(),
            m_xContext );
        uno::Reference < io::XActiveDataSink > xSink = new ZipPackageSink;
        if ( aContent.openStream ( xSink ) )
        {
            m_xContentStream = xSink->getInputStream();
            m_bOwnContent = true;
            xSeekable.set( m_xContentStream, uno::UNO_QUERY );
        }
    }
    else if ( (aArguments[0] >>= xStream ) )
    {
        // a writable stream can implement both XStream & XInputStream
        m_xContentStream = xStream->getInputStream();
        xSeekable.set( xStream, uno::UNO_QUERY );
    }
    else if ( aArguments[0] >>= m_xContentStream )
    {
        xSeekable.set( m_xContentStream, uno::UNO_QUERY );
    }
    else
        throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

    if ( !m_xContentStream.is() )
        throw io::IOException(THROW_WHERE );

    if ( !xSeekable.is() )
    {
        // TODO: after fwkbugfix02 is integrated a helper class can be used to make the stream seekable
        throw io::IOException(THROW_WHERE );
    }

    // TODO: in case xSeekable is implemented on separated XStream implementation a wrapper is required
    m_pZipFile = new ZipFile(
                m_xContentStream,
                m_xContext,
                true );
}

// XNameAccess
uno::Any SAL_CALL OZipFileAccess::getByName( const OUString& aName )
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pZipFile )
        throw uno::RuntimeException(THROW_WHERE);

    EntryHash::iterator aIter = m_pZipFile->GetEntryHash().find( aName );
    if ( aIter == m_pZipFile->GetEntryHash().end() )
        throw container::NoSuchElementException(THROW_WHERE );

    uno::Reference< io::XInputStream > xEntryStream;
    try
    {
        xEntryStream  = m_pZipFile->getDataStream((*aIter).second,
                                                  ::rtl::Reference< EncryptionData >(),
                                                  false,
                                                  m_aMutexHolder);
    }
    catch (const container::NoSuchElementException&)
    {
        throw;
    }
    catch (const lang::WrappedTargetException&)
    {
        throw;
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        throw lang::WrappedTargetException( "This package is unusable!",
                  static_cast < OWeakObject * > ( this ),
                                        makeAny(e));
    }

    if ( !xEntryStream.is() )
        throw uno::RuntimeException(THROW_WHERE );

    return uno::makeAny ( xEntryStream );
}

uno::Sequence< OUString > SAL_CALL OZipFileAccess::getElementNames()
    throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pZipFile )
        throw uno::RuntimeException(THROW_WHERE);

    uno::Sequence< OUString > aNames( m_pZipFile->GetEntryHash().size() );
    sal_Int32 nLen = 0;

    for ( EntryHash::iterator aIter = m_pZipFile->GetEntryHash().begin(); aIter != m_pZipFile->GetEntryHash().end(); ++aIter )
    {
        if ( aNames.getLength() < ++nLen )
        {
            OSL_FAIL( "The size must be the same!\n" );
            aNames.realloc( nLen );
        }

        aNames[nLen-1] = (*aIter).second.sPath;
    }

    if ( aNames.getLength() != nLen )
    {
        OSL_FAIL( "The size must be the same!\n" );
        aNames.realloc( nLen );
    }

    return aNames;
}

sal_Bool SAL_CALL OZipFileAccess::hasByName( const OUString& aName )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pZipFile )
        throw uno::RuntimeException(THROW_WHERE);

    EntryHash::iterator aIter = m_pZipFile->GetEntryHash().find( aName );

    return ( aIter != m_pZipFile->GetEntryHash().end() );
}

uno::Type SAL_CALL OZipFileAccess::getElementType()
    throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pZipFile )
        throw uno::RuntimeException(THROW_WHERE);

    return cppu::UnoType<io::XInputStream>::get();
}

sal_Bool SAL_CALL OZipFileAccess::hasElements()
    throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pZipFile )
        throw uno::RuntimeException(THROW_WHERE);

    return ( m_pZipFile->GetEntryHash().size() != 0 );
}

// XZipFileAccess
uno::Reference< io::XInputStream > SAL_CALL OZipFileAccess::getStreamByPattern( const OUString& aPatternString )
    throw ( container::NoSuchElementException,
            io::IOException, packages::WrongPasswordException, packages::zip::ZipException,
            uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pZipFile )
        throw io::NotConnectedException(THROW_WHERE );

    // Code to compare strings by patterns
    uno::Sequence< OUString > aPattern = GetPatternsFromString_Impl( aPatternString );

    for ( EntryHash::iterator aIter = m_pZipFile->GetEntryHash().begin(); aIter != m_pZipFile->GetEntryHash().end(); ++aIter )
    {
        if ( StringGoodForPattern_Impl( (*aIter).second.sPath, aPattern ) )
        {
            uno::Reference< io::XInputStream > xEntryStream( m_pZipFile->getDataStream( (*aIter).second,
                                                                                        ::rtl::Reference< EncryptionData >(),
                                                                                        false,
                                                                                        m_aMutexHolder ) );

            if ( !xEntryStream.is() )
                throw uno::RuntimeException(THROW_WHERE );
            return xEntryStream;
        }
    }

    throw container::NoSuchElementException(THROW_WHERE );
}

// XComponent
void SAL_CALL OZipFileAccess::dispose()
    throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( m_pListenersContainer )
    {
           lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
        m_pListenersContainer->disposeAndClear( aSource );
        delete m_pListenersContainer;
        m_pListenersContainer = nullptr;
    }

    if ( m_pZipFile )
    {
        delete m_pZipFile;
        m_pZipFile = nullptr;
    }

    if ( m_xContentStream.is() && m_bOwnContent )
        try {
            m_xContentStream->closeInput();
        } catch( uno::Exception& )
        {}

    m_bDisposed = true;
}

void SAL_CALL OZipFileAccess::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutexHolder->GetMutex() );
    m_pListenersContainer->addInterface( xListener );
}

void SAL_CALL OZipFileAccess::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException(THROW_WHERE );

    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

uno::Sequence< OUString > SAL_CALL OZipFileAccess::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    aRet[0] = "com.sun.star.packages.zip.ZipFileAccess";
    aRet[1] = "com.sun.star.comp.packages.zip.ZipFileAccess";
    return aRet;
}

OUString SAL_CALL OZipFileAccess::impl_staticGetImplementationName()
{
    return OUString("com.sun.star.comp.package.zip.ZipFileAccess");
}

uno::Reference< uno::XInterface > SAL_CALL OZipFileAccess::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& rxMSF )
{
    return uno::Reference< uno::XInterface >( *new OZipFileAccess( comphelper::getComponentContext(rxMSF) ) );
}

OUString SAL_CALL OZipFileAccess::getImplementationName()
    throw ( uno::RuntimeException, std::exception )
{
    return impl_staticGetImplementationName();
}

sal_Bool SAL_CALL OZipFileAccess::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OZipFileAccess::getSupportedServiceNames()
    throw ( uno::RuntimeException, std::exception )
{
    return impl_staticGetSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
