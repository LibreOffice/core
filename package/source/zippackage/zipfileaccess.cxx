/*************************************************************************
 *
 *  $RCSfile: zipfileaccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-18 17:22:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INVALIDARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#include <zipfileaccess.hxx>
#include <ZipEnumeration.hxx>
#include <ZipPackageSink.hxx>
#include <EncryptionData.hxx>

#include <ucbhelper/content.hxx>

#include <memory>


using namespace ::com::sun::star;

extern void copyInputToOutput_Impl( uno::Reference< io::XInputStream >& aIn, uno::Reference< io::XOutputStream >& aOut );

uno::Reference< io::XInputStream > GetSeekableTempCopy_Impl( uno::Reference< io::XInputStream > xInStream,
                                                        uno::Reference< lang::XMultiServiceFactory > xFactory )
{
    if ( !xInStream.is() )
        uno::Reference< io::XInputStream >();

    if ( !xFactory.is() )
        throw uno::RuntimeException();

    uno::Reference < io::XOutputStream > xTempOut(
                        xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                        uno::UNO_QUERY );
    uno::Reference < io::XInputStream > xTempIn( xTempOut, uno::UNO_QUERY );
    uno::Reference < io::XSeekable > xSeekable( xTempOut, uno::UNO_QUERY );

    if ( !xTempOut.is() || !xTempIn.is() || !xSeekable.is() )
        throw io::IOException();

    copyInputToOutput_Impl( xInStream, xTempOut );
    xTempOut->closeOutput();
    xSeekable->seek(0);

    return xTempIn;
}

// ----------------------------------------------------------------
OZipFileAccess::OZipFileAccess( const uno::Reference< lang::XMultiServiceFactory >& xFactory )
: m_xFactory( xFactory )
, m_pZipFile( NULL )
, m_pListenersContainer( NULL )
, m_bDisposed( sal_False )
{
    if ( !xFactory.is() )
        throw uno::RuntimeException();
}

// ----------------------------------------------------------------
OZipFileAccess::~OZipFileAccess()
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
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
EntryHash& OZipFileAccess::GetEntryHash_Impl()
{
    OSL_ENSURE( m_pZipFile, "ZipFile must be set already!\n" );
    if ( !m_aEntries.size() && m_pZipFile )
    {
        ::std::auto_ptr< ZipEnumeration > pZipEnum( m_pZipFile->entries() );
        while( pZipEnum->hasMoreElements() )
        {
            const ZipEntry* pZipEntry = pZipEnum->nextElement();
            m_aEntries[pZipEntry->sName] = *pZipEntry;
        }
    }

    return m_aEntries;
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
    if ( !nInd )
        return sal_False;

    if ( nInd == 1 )
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
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_pZipFile )
        throw uno::Exception(); // TODO: initialization is allowed only one time

    if ( !aArguments.getLength() )
        throw lang::IllegalArgumentException(); // TODO:

    OSL_ENSURE( aArguments.getLength() == 1, "Too meny arguments are provided, only the first one will be used!\n" );

    ::rtl::OUString aParamURL;
    uno::Reference< io::XStream > xStream;
    uno::Reference< io::XSeekable > xSeekable;

    if ( ( aArguments[0] >>= aParamURL ) )
    {
        ::ucb::Content aContent ( aParamURL, uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
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
        throw lang::IllegalArgumentException(); // TODO:

    if ( !m_xContentStream.is() )
        throw io::IOException(); // TODO:

    if ( !xSeekable.is() )
    {
        // TODO: after fwkbugfix02 is integrated a helper class can be used to make the stream seekable
        throw io::IOException();
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
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pZipFile )
        throw io::NotConnectedException();

    GetEntryHash_Impl();
    EntryHash::iterator aIter = m_aEntries.find( aName );
    if ( aIter == m_aEntries.end() )
        throw container::NoSuchElementException();

    uno::Reference< io::XInputStream > xEntryStream = GetSeekableTempCopy_Impl(
                                m_pZipFile->getDataStream( (*aIter).second, new EncryptionData(), sal_False ),
                                m_xFactory );

    if ( !xEntryStream.is() )
        throw uno::RuntimeException(); // TODO:

    return uno::makeAny ( xEntryStream );
}

// ----------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OZipFileAccess::getElementNames()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pZipFile )
        throw io::NotConnectedException();

    uno::Sequence< ::rtl::OUString > aNames;
    sal_Int32 nLen = 0;

    GetEntryHash_Impl();

    for ( EntryHash::iterator aIter = m_aEntries.begin(); aIter != m_aEntries.end(); aIter++ )
    {
        aNames.realloc( ++nLen );
        aNames[nLen-1] = (*aIter).second.sName;
    }

    return aNames;
}

// ----------------------------------------------------------------
sal_Bool SAL_CALL OZipFileAccess::hasByName( const ::rtl::OUString& aName )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pZipFile )
        throw io::NotConnectedException();

    GetEntryHash_Impl();
    EntryHash::iterator aIter = m_aEntries.find( aName );

    return ( aIter != m_aEntries.end() );
}

// ----------------------------------------------------------------
uno::Type SAL_CALL OZipFileAccess::getElementType()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pZipFile )
        throw io::NotConnectedException();

    return getCppuType( ( const uno::Reference< io::XInputStream >* )NULL );
}

// ----------------------------------------------------------------
sal_Bool SAL_CALL OZipFileAccess::hasElements()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pZipFile )
        throw io::NotConnectedException();

    GetEntryHash_Impl();
    return ( m_aEntries.size() != 0 );
}

// XZipFileAccess
// ----------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OZipFileAccess::getStreamByPattern( const ::rtl::OUString& aPatternString )
    throw ( container::NoSuchElementException,
            io::IOException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pZipFile )
        throw io::NotConnectedException();

    // Code to compare strings by patterns
    uno::Sequence< ::rtl::OUString > aPattern = GetPatternsFromString_Impl( aPatternString );

    GetEntryHash_Impl();

    for ( EntryHash::iterator aIter = m_aEntries.begin(); aIter != m_aEntries.end(); aIter++ )
    {
        if ( StringGoodForPattern_Impl( (*aIter).second.sName, aPattern ) )
        {
            uno::Reference< io::XInputStream > xEntryStream =
                        GetSeekableTempCopy_Impl(
                            m_pZipFile->getDataStream( (*aIter).second, new EncryptionData(), sal_False ),
                            m_xFactory );

            if ( !xEntryStream.is() )
                throw uno::RuntimeException();
            return xEntryStream;
        }
    }

    throw container::NoSuchElementException();

    // not reachable
    return uno::Reference< io::XInputStream >();
}

// XComponent
// ----------------------------------------------------------------
void SAL_CALL OZipFileAccess::dispose()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

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
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );
    m_pListenersContainer->addInterface( xListener );
}

// ----------------------------------------------------------------
void SAL_CALL OZipFileAccess::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_pListenersContainer )
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

