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
#include "precompiled_package.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#ifndef _COM_SUN_STAR_LANG_INVALIDARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <zipfileaccess.hxx>
#include <ZipEnumeration.hxx>
#include <ZipPackageSink.hxx>
#include <EncryptionData.hxx>

#include <ucbhelper/content.hxx>
#include <rtl/ref.hxx>

#include <memory>


using namespace ::com::sun::star;

// ----------------------------------------------------------------
OZipFileAccess::OZipFileAccess( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_aMutexHolder( new SotMutexHolder )
, m_xFactory( xFactory )
, m_pZipFile( NULL )
, m_pListenersContainer( NULL )
, m_bDisposed( sal_False )
{
    if ( !xFactory.is() )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

// ----------------------------------------------------------------
OZipFileAccess::~OZipFileAccess()
{
    {
        ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );
        if ( !m_bDisposed )
        {
            try {
                m_refCount++; // dispose will use refcounting so the further distruction must be avoided
                dispose();
            } catch( uno::Exception& )
            {}
        }
    }
}

// ----------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OZipFileAccess::GetPatternsFromString_Impl( const ::rtl::OUString& aString )
{
    if ( !aString.getLength() )
        return uno::Sequence< ::rtl::OUString >();

    uno::Sequence< ::rtl::OUString > aPattern( 1 );
    sal_Int32 nInd = 0;

    const sal_Unicode* pString = aString.getStr();
    while( *pString )
    {
        if ( *pString == (sal_Unicode)'\\' )
        {
            pString++;

            if ( *pString == (sal_Unicode)'\\' )
            {
                aPattern[nInd] += ::rtl::OUString::valueOf( (sal_Unicode)'\\' );
                pString++;
            }
            else if ( *pString == (sal_Unicode)'*' )
            {
                aPattern[nInd] += ::rtl::OUString::valueOf( (sal_Unicode)'*' );
                pString++;
            }
            else
            {
                OSL_ENSURE( sal_False, "The backslash is not guarded!\n" );
                aPattern[nInd] += ::rtl::OUString::valueOf( (sal_Unicode)'\\' );
            }
        }
        else if ( *pString == (sal_Unicode)'*' )
        {
            aPattern.realloc( ( ++nInd ) + 1 );
            pString++;
        }
        else
        {
            aPattern[nInd] += ::rtl::OUString::valueOf( *pString );
            pString++;
        }
    }

    return aPattern;
}

// ----------------------------------------------------------------
sal_Bool OZipFileAccess::StringGoodForPattern_Impl( const ::rtl::OUString& aString,
                                                    const uno::Sequence< ::rtl::OUString >& aPattern )
{
    sal_Int32 nInd = aPattern.getLength() - 1;
    if ( nInd < 0 )
        return sal_False;

    if ( nInd == 0 )
    {
        if ( !aPattern[0].getLength() )
            return sal_True;

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
            if ( !aPattern[nCurInd].getLength() )
                continue;

            if ( nEndInd == nBeginInd )
                return sal_False;

            // check that search does not use nEndInd position
            sal_Int32 nLastInd = aString.lastIndexOf( aPattern[nCurInd], nEndInd - 1 );

            if ( nLastInd == -1 )
                return sal_False;

            if ( nLastInd < nBeginInd )
                return sal_False;

            nEndInd = nLastInd;
        }

        return sal_True;
    }

    return sal_False;
}

// XInitialization
// ----------------------------------------------------------------
void SAL_CALL OZipFileAccess::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw ( uno::Exception,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( m_pZipFile )
        throw uno::Exception( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() ); // initialization is allowed only one time

    if ( !aArguments.getLength() )
        throw lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 1 );

    OSL_ENSURE( aArguments.getLength() == 1, "Too meny arguments are provided, only the first one will be used!\n" );

    ::rtl::OUString aParamURL;
    uno::Reference< io::XStream > xStream;
    uno::Reference< io::XSeekable > xSeekable;

    if ( ( aArguments[0] >>= aParamURL ) )
    {
        ::ucbhelper::Content aContent ( aParamURL, uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
        uno::Reference < io::XActiveDataSink > xSink = new ZipPackageSink;
        if ( aContent.openStream ( xSink ) )
        {
            m_xContentStream = xSink->getInputStream();
            xSeekable = uno::Reference< io::XSeekable >( m_xContentStream, uno::UNO_QUERY );
        }
    }
    else if ( (aArguments[0] >>= xStream ) )
    {
        // a writable stream can implement both XStream & XInputStream
        m_xContentStream = xStream->getInputStream();
        xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );
    }
    else if ( !( aArguments[0] >>= m_xContentStream ) )
    {
        xSeekable = uno::Reference< io::XSeekable >( m_xContentStream, uno::UNO_QUERY );
    }
    else
        throw lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 1 );

    if ( !m_xContentStream.is() )
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !xSeekable.is() )
    {
        // TODO: after fwkbugfix02 is integrated a helper class can be used to make the stream seekable
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    }

    // TODO: in case xSeekable is implemented on separated XStream implementation a wrapper is required
    m_pZipFile = new ZipFile(
                m_xContentStream,
                m_xFactory,
                sal_True );
}

// XNameAccess
// ----------------------------------------------------------------
uno::Any SAL_CALL OZipFileAccess::getByName( const ::rtl::OUString& aName )
    throw ( container::NoSuchElementException,
            lang::WrappedTargetException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pZipFile )
        throw io::NotConnectedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    EntryHash::iterator aIter = m_pZipFile->GetEntryHash().find( aName );
    if ( aIter == m_pZipFile->GetEntryHash().end() )
        throw container::NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    uno::Reference< io::XInputStream > xEntryStream( m_pZipFile->getDataStream( (*aIter).second,
                                                                                ::rtl::Reference< EncryptionData >(),
                                                                                sal_False,
                                                                                m_aMutexHolder ) );

    if ( !xEntryStream.is() )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return uno::makeAny ( xEntryStream );
}

// ----------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OZipFileAccess::getElementNames()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pZipFile )
        throw io::NotConnectedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    uno::Sequence< ::rtl::OUString > aNames( m_pZipFile->GetEntryHash().size() );
    sal_Int32 nLen = 0;

    for ( EntryHash::iterator aIter = m_pZipFile->GetEntryHash().begin(); aIter != m_pZipFile->GetEntryHash().end(); aIter++ )
    {
        if ( aNames.getLength() < ++nLen )
        {
            OSL_ENSURE( sal_False, "The size must be the same!\n" );
            aNames.realloc( nLen );
        }

        aNames[nLen-1] = (*aIter).second.sPath;
    }

    if ( aNames.getLength() != nLen )
    {
        OSL_ENSURE( sal_False, "The size must be the same!\n" );
        aNames.realloc( nLen );
    }

    return aNames;
}

// ----------------------------------------------------------------
sal_Bool SAL_CALL OZipFileAccess::hasByName( const ::rtl::OUString& aName )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pZipFile )
        throw io::NotConnectedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    EntryHash::iterator aIter = m_pZipFile->GetEntryHash().find( aName );

    return ( aIter != m_pZipFile->GetEntryHash().end() );
}

// ----------------------------------------------------------------
uno::Type SAL_CALL OZipFileAccess::getElementType()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pZipFile )
        throw io::NotConnectedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return getCppuType( ( const uno::Reference< io::XInputStream >* )NULL );
}

// ----------------------------------------------------------------
sal_Bool SAL_CALL OZipFileAccess::hasElements()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pZipFile )
        throw io::NotConnectedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    return ( m_pZipFile->GetEntryHash().size() != 0 );
}

// XZipFileAccess
// ----------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OZipFileAccess::getStreamByPattern( const ::rtl::OUString& aPatternString )
    throw ( container::NoSuchElementException,
            io::IOException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pZipFile )
        throw io::NotConnectedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    // Code to compare strings by patterns
    uno::Sequence< ::rtl::OUString > aPattern = GetPatternsFromString_Impl( aPatternString );

    for ( EntryHash::iterator aIter = m_pZipFile->GetEntryHash().begin(); aIter != m_pZipFile->GetEntryHash().end(); aIter++ )
    {
        if ( StringGoodForPattern_Impl( (*aIter).second.sPath, aPattern ) )
        {
            uno::Reference< io::XInputStream > xEntryStream( m_pZipFile->getDataStream( (*aIter).second,
                                                                                        ::rtl::Reference< EncryptionData >(),
                                                                                        sal_False,
                                                                                        m_aMutexHolder ) );

            if ( !xEntryStream.is() )
                throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
            return xEntryStream;
        }
    }

    throw container::NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

// XComponent
// ----------------------------------------------------------------
void SAL_CALL OZipFileAccess::dispose()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( m_pListenersContainer )
    {
           lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
        m_pListenersContainer->disposeAndClear( aSource );
        delete m_pListenersContainer;
        m_pListenersContainer = NULL;
    }

    if ( m_pZipFile )
    {
        delete m_pZipFile;
        m_pZipFile = NULL;
    }

    if ( m_xContentStream.is() )
        try {
            m_xContentStream->closeInput();
        } catch( uno::Exception& )
        {}

    m_bDisposed = sal_True;
}

// ----------------------------------------------------------------
void SAL_CALL OZipFileAccess::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutexHolder->GetMutex() );
    m_pListenersContainer->addInterface( xListener );
}

// ----------------------------------------------------------------
void SAL_CALL OZipFileAccess::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_bDisposed )
        throw lang::DisposedException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OZipFileAccess::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > aRet(2);
    aRet[0] = ::rtl::OUString::createFromAscii("com.sun.star.packages.zip.ZipFileAccess");
    aRet[1] = ::rtl::OUString::createFromAscii("com.sun.star.comp.packages.zip.ZipFileAccess");
    return aRet;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OZipFileAccess::impl_staticGetImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.package.zip.ZipFileAccess");
}

//-------------------------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OZipFileAccess::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OZipFileAccess( xServiceManager ) );
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OZipFileAccess::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OZipFileAccess::supportsService( const ::rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OZipFileAccess::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

