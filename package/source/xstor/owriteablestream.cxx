/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: owriteablestream.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:11:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "owriteablestream.hxx"

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include <comphelper/storagehelper.hxx>

#include "oseekinstream.hxx"
#include "mutexholder.hxx"
#include "xstorage.hxx"

#include <rtl/digest.h>
#include <rtl/logfile.hxx>


using namespace ::com::sun::star;

//-----------------------------------------------
void SetEncryptionKeyProperty_Impl( const uno::Reference< beans::XPropertySet >& xPropertySet,
                                    const uno::Sequence< sal_Int8 >& aKey )
{
    OSL_ENSURE( xPropertySet.is(), "No property set is provided!\n" );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    ::rtl::OUString aString_EncryptionKey = ::rtl::OUString::createFromAscii( "EncryptionKey" );
    try {
        xPropertySet->setPropertyValue( aString_EncryptionKey, uno::makeAny( aKey ) );
    }
    catch ( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Can't write encryption related properties!\n" );
        throw io::IOException(); // TODO
    }
}

//-----------------------------------------------
uno::Any GetEncryptionKeyProperty_Impl( const uno::Reference< beans::XPropertySet >& xPropertySet )
{
    OSL_ENSURE( xPropertySet.is(), "No property set is provided!\n" );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    ::rtl::OUString aString_EncryptionKey = ::rtl::OUString::createFromAscii( "EncryptionKey" );
    try {
        return xPropertySet->getPropertyValue( aString_EncryptionKey );
    }
    catch ( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Can't get encryption related properties!\n" );
        throw io::IOException(); // TODO
    }
}

//-----------------------------------------------
void completeStorageStreamCopy_Impl( const uno::Reference< io::XStream >& xSource,
                              const uno::Reference< io::XStream >& xDest );

//-----------------------------------------------
sal_Bool SequencesEqual( uno::Sequence< sal_Int8 > aSequence1, uno::Sequence< sal_Int8 > aSequence2 )
{
    if ( aSequence1.getLength() != aSequence2.getLength() )
        return sal_False;

    for ( sal_Int32 nInd = 0; nInd < aSequence1.getLength(); nInd++ )
        if ( aSequence1[nInd] != aSequence2[nInd] )
            return sal_False;

    return sal_True;
}

//-----------------------------------------------
sal_Bool KillFile( const ::rtl::OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory )
{
    if ( !xFactory.is() )
        return sal_False;

    sal_Bool bRet = sal_False;

    try
    {
        uno::Reference < ucb::XSimpleFileAccess > xAccess(
                xFactory->createInstance (
                        ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                uno::UNO_QUERY );

        if ( xAccess.is() )
        {
            xAccess->kill( aURL );
            bRet = sal_True;
        }
    }
    catch( uno::Exception& )
    {
    }

    return bRet;
}

const sal_Int32 n_ConstBufferSize = 32000;

//-----------------------------------------------

::rtl::OUString GetNewTempFileURL( const uno::Reference< lang::XMultiServiceFactory > xFactory )
{
    ::rtl::OUString aTempURL;

    uno::Reference < beans::XPropertySet > xTempFile(
            xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY );

    if ( !xTempFile.is() )
        throw uno::RuntimeException(); // TODO

    try {
        xTempFile->setPropertyValue( ::rtl::OUString::createFromAscii( "RemoveFile" ), uno::makeAny( sal_False ) );
        uno::Any aUrl = xTempFile->getPropertyValue( ::rtl::OUString::createFromAscii( "Uri" ) );
        aUrl >>= aTempURL;
    }
    catch ( uno::Exception& )
    {
    }

    if ( !aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO: can not create tempfile

    return aTempURL;
}

uno::Sequence< sal_Int8 > MakeKeyFromPass( ::rtl::OUString aPass, sal_Bool bUseUTF )
{
    // MS_1252 encoding was used for SO60 document format password encoding,
    // this encoding supports only a minor subset of nonascii characters,
    // but for compatibility reasons it has to be used for old document formats

    ::rtl::OString aByteStrPass;
    if ( bUseUTF )
        aByteStrPass = ::rtl::OUStringToOString( aPass, RTL_TEXTENCODING_UTF8 );
    else
        aByteStrPass = ::rtl::OUStringToOString( aPass, RTL_TEXTENCODING_MS_1252 );

    sal_uInt8 pBuffer[RTL_DIGEST_LENGTH_SHA1];
    rtlDigestError nError = rtl_digest_SHA1( aByteStrPass.getStr(),
                                            aByteStrPass.getLength(),
                                            pBuffer,
                                            RTL_DIGEST_LENGTH_SHA1 );

    if ( nError != rtl_Digest_E_None )
        throw uno::RuntimeException();

    return uno::Sequence< sal_Int8 >( (sal_Int8*)pBuffer, RTL_DIGEST_LENGTH_SHA1 );

}

// ================================================================

//-----------------------------------------------
OWriteStream_Impl::OWriteStream_Impl( OStorage_Impl* pParent,
                                      uno::Reference< packages::XDataSinkEncrSupport > xPackageStream,
                                      uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xPackage,
                                      uno::Reference< lang::XMultiServiceFactory > xFactory,
                                      sal_Bool bForceEncrypted )
: m_pAntiImpl( NULL )
, m_bHasDataToFlush( sal_False )
, m_bFlushed( sal_False )
, m_xPackageStream( xPackageStream )
, m_xFactory( xFactory )
, m_pParent( pParent )
, m_bForceEncrypted( bForceEncrypted )
, m_bUseCommonPass( sal_False )
, m_bHasCachedPassword( sal_False )
, m_xPackage( xPackage )
, m_bHasInsertedStreamOptimization( sal_False )
{
    OSL_ENSURE( xPackageStream.is(), "No package stream is provided!\n" );
    OSL_ENSURE( xPackage.is(), "No package component is provided!\n" );
    OSL_ENSURE( m_xFactory.is(), "No package stream is provided!\n" );
    OSL_ENSURE( pParent, "No parent storage is provided!\n" );
}

//-----------------------------------------------
OWriteStream_Impl::~OWriteStream_Impl()
{
    DisposeWrappers();

    if ( m_aTempURL.getLength() )
    {
        KillFile( m_aTempURL, GetServiceFactory() );
        m_aTempURL = ::rtl::OUString();
    }
}

//-----------------------------------------------
void OWriteStream_Impl::InsertIntoPackageFolder( const ::rtl::OUString& aName,
                                                  const uno::Reference< container::XNameContainer >& xParentPackageFolder )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_bFlushed, "This method must not be called for nonflushed streams!\n" );
    if ( m_bFlushed )
    {
        OSL_ENSURE( m_xPackageStream.is(), "An inserted stream is incomplete!\n" );
        uno::Reference< lang::XUnoTunnel > xTunnel( m_xPackageStream, uno::UNO_QUERY );
        if ( !xTunnel.is() )
            throw uno::RuntimeException(); // TODO

        xParentPackageFolder->insertByName( aName, uno::makeAny( xTunnel ) );

        m_bFlushed = sal_False;
        m_bHasInsertedStreamOptimization = sal_False;
    }
}
//-----------------------------------------------
sal_Bool OWriteStream_Impl::IsEncrypted()
{
    if ( m_bUseCommonPass )
        return sal_False;

    if ( m_bForceEncrypted || m_bHasCachedPassword )
        return sal_True;

    if ( m_aTempURL.getLength() )
        return sal_False;

    GetStreamProperties();

    // the following value can not be cached since it can change after root commit
    sal_Bool bWasEncr = sal_False;
    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        uno::Any aValue = xPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "WasEncrypted" ) );
        if ( !( aValue >>= bWasEncr ) )
        {
            OSL_ENSURE( sal_False, "The property WasEncrypted has wrong type!\n" );
        }
    }

    sal_Bool bToBeEncr = sal_False;
    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name.equalsAscii( "Encrypted" ) )
        {
            if ( !( m_aProps[nInd].Value >>= bToBeEncr ) )
            {
                OSL_ENSURE( sal_False, "The property has wrong type!\n" );
            }
        }
    }

    // since a new key set to the package stream it should not be removed except the case when
    // the stream becomes nonencrypted
    uno::Sequence< sal_Int8 > aKey;
    if ( bToBeEncr )
        GetEncryptionKeyProperty_Impl( xPropSet ) >>= aKey;

    // If the properties must be investigated the stream is either
    // was never changed or was changed, the parent was commited
    // and the stream was closed.
    // That means that if it is intended to use common storage key
    // it is already has no encryption but is marked to be stored
    // encrypted and the key is empty.
    if ( !bWasEncr && bToBeEncr && !aKey.getLength() )
    {
        // the stream is intended to use common storage password
        m_bUseCommonPass = sal_True;
        return sal_False;
    }
    else
        return bToBeEncr;
}

//-----------------------------------------------
void OWriteStream_Impl::SetDecrypted()
{
    GetStreamProperties();

    // let the stream be modified
    GetFilledTempFile();
    m_bHasDataToFlush = sal_True;

    // remove encryption
    m_bForceEncrypted = sal_False;
    m_bHasCachedPassword = sal_False;
    m_aPass = ::rtl::OUString();

    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name.equalsAscii( "Encrypted" ) )
            m_aProps[nInd].Value <<= sal_False;
    }
}

//-----------------------------------------------
void OWriteStream_Impl::SetEncryptedWithPass( const ::rtl::OUString& aPass )
{
    GetStreamProperties();

    // let the stream be modified
    GetFilledTempFile();
    m_bHasDataToFlush = sal_True;

    // introduce encryption info
    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name.equalsAscii( "Encrypted" ) )
            m_aProps[nInd].Value <<= sal_True;
    }

    m_bUseCommonPass = sal_False; // very important to set it to false

    m_bHasCachedPassword = sal_True;
    m_aPass = aPass;
}

//-----------------------------------------------
void OWriteStream_Impl::DisposeWrappers()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    if ( m_pAntiImpl )
    {
        try {
            m_pAntiImpl->dispose();
        }
        catch ( uno::RuntimeException& )
        {}
        m_pAntiImpl = NULL;
    }
    m_pParent = NULL;

    if ( !m_aInputStreamsList.empty() )
    {
        for ( InputStreamsList_Impl::iterator pStreamIter = m_aInputStreamsList.begin();
              pStreamIter != m_aInputStreamsList.end(); pStreamIter++ )
        {
            if ( (*pStreamIter) )
            {
                (*pStreamIter)->InternalDispose();
                (*pStreamIter) = NULL;
            }
        }

        m_aInputStreamsList.clear();
    }
}

//-----------------------------------------------
uno::Reference< lang::XMultiServiceFactory > OWriteStream_Impl::GetServiceFactory()
{
    if ( m_xFactory.is() )
        return m_xFactory;

    return ::comphelper::getProcessServiceFactory();
}

//-----------------------------------------------
::rtl::OUString OWriteStream_Impl::GetFilledTempFile()
{
    if ( !m_aTempURL.getLength() )
    {
        m_aTempURL = GetNewTempFileURL( GetServiceFactory() );

        try {
            if ( m_aTempURL )
            {
                uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                                GetServiceFactory()->createInstance (
                                        ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                                uno::UNO_QUERY );

                if ( !xTempAccess.is() )
                    throw uno::RuntimeException(); // TODO:


                // in case of new inserted package stream it is possible that input stream still was not set
                uno::Reference< io::XInputStream > xOrigStream = m_xPackageStream->getDataStream();
                if ( xOrigStream.is() )
                {
                    uno::Reference< io::XOutputStream > xTempOutStream = xTempAccess->openFileWrite( m_aTempURL );
                    if ( xTempOutStream.is() )
                    {
                        // copy stream contents to the file
                        ::comphelper::OStorageHelper::CopyInputToOutput( xOrigStream, xTempOutStream );
                        xTempOutStream->closeOutput();
                        xTempOutStream = uno::Reference< io::XOutputStream >();
                    }
                    else
                        throw io::IOException(); // TODO:
                }
            }
        }
        catch( packages::WrongPasswordException& )
        {
            KillFile( m_aTempURL, GetServiceFactory() );
            m_aTempURL = ::rtl::OUString();

            throw;
        }
        catch( uno::Exception& )
        {
            KillFile( m_aTempURL, GetServiceFactory() );
            m_aTempURL = ::rtl::OUString();
        }
    }

    return m_aTempURL;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetTempFileAsStream()
{
    uno::Reference< io::XStream > xTempStream;

    if ( !m_aTempURL.getLength() )
        m_aTempURL = GetFilledTempFile();

    uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                    GetServiceFactory()->createInstance (
                            ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );

    if ( !xTempAccess.is() )
        throw uno::RuntimeException(); // TODO:

    try
    {
        xTempStream = xTempAccess->openFileReadWrite( m_aTempURL );
    }
    catch( uno::Exception& )
    {
    }

    // the method must always return a stream
    // in case the stream can not be open
    // an exception should be thrown
    if ( !xTempStream.is() )
        throw io::IOException(); //TODO:

    return xTempStream;
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OWriteStream_Impl::GetTempFileAsInputStream()
{
    uno::Reference< io::XInputStream > xInputStream;

    if ( !m_aTempURL.getLength() )
        m_aTempURL = GetFilledTempFile();

    uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                    GetServiceFactory()->createInstance (
                            ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );

    if ( !xTempAccess.is() )
        throw uno::RuntimeException(); // TODO:

    try
    {
        xInputStream = xTempAccess->openFileRead( m_aTempURL );
    }
    catch( uno::Exception& )
    {
    }

    // the method must always return a stream
    // in case the stream can not be open
    // an exception should be thrown
    if ( !xInputStream.is() )
        throw io::IOException(); // TODO:

    return xInputStream;
}

//-----------------------------------------------
void OWriteStream_Impl::CopyTempFileToOutput( uno::Reference< io::XOutputStream > xOutStream )
{
    OSL_ENSURE( xOutStream.is(), "The stream must be specified!\n" );
    OSL_ENSURE( m_aTempURL.getLength(), "The temporary must exist!\n" );

    uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                    GetServiceFactory()->createInstance (
                            ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );

    if ( !xTempAccess.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< io::XInputStream > xTempInStream;
    try
    {
        xTempInStream = xTempAccess->openFileRead( m_aTempURL );
    }
    catch( uno::Exception& )
    {
    }

    if ( !xTempInStream.is() )
        throw io::IOException(); //TODO:

    ::comphelper::OStorageHelper::CopyInputToOutput( xTempInStream, xOutStream );
}

// =================================================================================================

//-----------------------------------------------
void OWriteStream_Impl::InsertStreamDirectly( const uno::Reference< io::XInputStream >& xInStream,
                                              const uno::Sequence< beans::PropertyValue >& aProps )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    // this call can be made only during parent storage commit
    // the  parent storage is responsible for the correct handling
    // of deleted and renamed contents

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_bHasDataToFlush )
        throw io::IOException();

    OSL_ENSURE( !m_aTempURL.getLength(), "The temporary must not exist!\n" );

    // use new file as current persistent representation
    // the new file will be removed after it's stream is closed
    m_xPackageStream->setDataStream( xInStream );

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    // The storage-package communication has a problem
    // the storage caches properties, thus if the package changes one of them itself
    // the storage does not know about it

    // Depending from MediaType value the package can change the compressed property itself
    // Thus if Compressed property is provided it must be set as the latest one
    sal_Bool bCompressedIsSet = sal_False;
    sal_Bool bCompressed = sal_False;
    ::rtl::OUString aComprPropName( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) );
    ::rtl::OUString aMedTypePropName( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) );
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
    {
        if ( aProps[nInd].Name.equals( aMedTypePropName ) )
            xPropertySet->setPropertyValue( aProps[nInd].Name, aProps[nInd].Value );
        else if ( aProps[nInd].Name.equals( aComprPropName ) )
        {
            bCompressedIsSet = sal_True;
            aProps[nInd].Value >>= bCompressed;
        }
        else if ( aProps[nInd].Name.equalsAscii( "UseCommonStoragePasswordEncryption" ) )
            aProps[nInd].Value >>= m_bUseCommonPass;

        // if there are cached properties update them
        if ( aProps[nInd].Name.equals( aMedTypePropName ) || aProps[nInd].Name.equals( aComprPropName ) )
            for ( sal_Int32 nMemInd = 0; nMemInd < m_aProps.getLength(); nMemInd++ )
            {
                if ( aProps[nInd].Name.equals( m_aProps[nMemInd].Name ) )
                    m_aProps[nMemInd].Value = aProps[nInd].Value;
            }
    }

    if ( bCompressedIsSet )
            xPropertySet->setPropertyValue( aComprPropName, uno::makeAny( (sal_Bool)bCompressed ) );

    if ( m_bUseCommonPass )
    {
        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( ::rtl::OUString::createFromAscii( "EncryptionKey" ),
                                        uno::makeAny( uno::Sequence< sal_Int8 >() ) );
        xPropertySet->setPropertyValue( ::rtl::OUString::createFromAscii( "Encrypted" ),
                                        uno::makeAny( sal_True ) );
    }

    // the stream should be free soon, after package is stored
    m_bHasDataToFlush = sal_False;
    m_bFlushed = sal_True; // will allow to use transaction on stream level if will need it
    m_bHasInsertedStreamOptimization = sal_True;
}

//-----------------------------------------------
void OWriteStream_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    // this call can be made only during parent storage commit
    // the  parent storage is responsible for the correct handling
    // of deleted and renamed contents

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( !m_bHasDataToFlush )
        return;

    uno::Reference< packages::XDataSinkEncrSupport > xNewPackageStream;

    OSL_ENSURE( m_bHasInsertedStreamOptimization || m_aTempURL.getLength(), "The temporary must exist!\n" );
    if ( m_aTempURL.getLength() )
    {
        uno::Reference < io::XOutputStream > xTempOut(
                            GetServiceFactory()->createInstance (
                                    ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                            uno::UNO_QUERY );
        uno::Reference < io::XInputStream > xTempIn( xTempOut, uno::UNO_QUERY );

        if ( !xTempOut.is() || !xTempIn.is() )
            throw io::IOException();

        // Copy temporary file to a new one
        CopyTempFileToOutput( xTempOut );
        xTempOut->closeOutput();

        uno::Sequence< uno::Any > aSeq( 1 );
        aSeq[0] <<= sal_False;
        xNewPackageStream = uno::Reference< packages::XDataSinkEncrSupport >(
                                                        m_xPackage->createInstanceWithArguments( aSeq ),
                                                        uno::UNO_QUERY );
        if ( !xNewPackageStream.is() )
            throw uno::RuntimeException();

        // use new file as current persistent representation
        // the new file will be removed after it's stream is closed
        xNewPackageStream->setDataStream( xTempIn );
    }
    else // if ( m_bHasInsertedStreamOptimization )
    {
        // if the optimization is used the stream can be accessed directly
        xNewPackageStream = m_xPackageStream;
    }

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( xNewPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
    {
        if ( m_aProps[nInd].Name.equalsAscii( "Size" ) )
        {
            if ( m_pAntiImpl && !m_bHasInsertedStreamOptimization && m_pAntiImpl->m_xSeekable.is() )
            {
                m_aProps[nInd].Value <<= ((sal_Int32)m_pAntiImpl->m_xSeekable->getLength());
                xPropertySet->setPropertyValue( m_aProps[nInd].Name, m_aProps[nInd].Value );
            }
        }
        else
            xPropertySet->setPropertyValue( m_aProps[nInd].Name, m_aProps[nInd].Value );
    }

    if ( m_bUseCommonPass )
    {
        // set to be encrypted but do not use encryption key
        xPropertySet->setPropertyValue( ::rtl::OUString::createFromAscii( "EncryptionKey" ),
                                        uno::makeAny( uno::Sequence< sal_Int8 >() ) );
        xPropertySet->setPropertyValue( ::rtl::OUString::createFromAscii( "Encrypted" ),
                                        uno::makeAny( sal_True ) );
    }
    else if ( m_bHasCachedPassword )
        xPropertySet->setPropertyValue( ::rtl::OUString::createFromAscii( "EncryptionKey" ),
                                        uno::makeAny( MakeKeyFromPass( m_aPass, sal_True ) ) );

    // the stream should be free soon, after package is stored
    m_xPackageStream = xNewPackageStream;
    m_bHasDataToFlush = sal_False;
    m_bFlushed = sal_True; // will allow to use transaction on stream level if will need it
}

//-----------------------------------------------
void OWriteStream_Impl::Revert()
{
    // can be called only from parent storage
    // means complete reload of the stream

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( !m_bHasDataToFlush )
        return; // nothing to do

    // The stream must be free
    OSL_ENSURE( !m_pAntiImpl, "Reverting storage while a write stream is open!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO

    OSL_ENSURE( m_aTempURL.getLength(), "The temporary must exist!\n" );

    if ( m_aTempURL.getLength() )
    {
        KillFile( m_aTempURL, GetServiceFactory() );
        m_aTempURL = ::rtl::OUString();
    }

    m_aProps.realloc( 0 );

    m_bHasDataToFlush = sal_False;

    m_bUseCommonPass = sal_False;
    m_bHasCachedPassword = sal_False;
    m_aPass = ::rtl::OUString();
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::GetStreamProperties()
{
    if ( !m_aProps.getLength() )
        m_aProps = ReadPackageStreamProperties();

    return m_aProps;
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::InsertOwnProps(
                                                                    const uno::Sequence< beans::PropertyValue >& aProps,
                                                                    sal_Bool bUseCommonPass )
{
    uno::Sequence< beans::PropertyValue > aResult( aProps );

    sal_Int32 nLen = aResult.getLength();
    for ( sal_Int32 nInd = 0; nInd < nLen; nInd++ )
        if ( aResult[nInd].Name.equalsAscii( "UseCommonStoragePasswordEncryption" ) )
        {
            aResult[nInd].Value <<= bUseCommonPass;
            return aResult;
        }

    aResult.realloc( ++nLen );
    aResult[nLen - 1].Name = ::rtl::OUString::createFromAscii("UseCommonStoragePasswordEncryption");
    aResult[nLen - 1].Value <<= bUseCommonPass;

    return aResult;
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::ReadPackageStreamProperties()
{
    uno::Sequence< beans::PropertyValue > aResult( 4 );

    aResult[0].Name = ::rtl::OUString::createFromAscii("MediaType");
    aResult[1].Name = ::rtl::OUString::createFromAscii("Size");
    aResult[2].Name = ::rtl::OUString::createFromAscii("Encrypted");
    aResult[3].Name = ::rtl::OUString::createFromAscii("Compressed");
    // TODO: may be also raw stream should be marked

    uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        for ( sal_Int32 nInd = 0; nInd < aResult.getLength(); nInd++ )
        {
            try {
                aResult[nInd].Value = xPropSet->getPropertyValue( aResult[nInd].Name );
            }
            catch( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "A property can't be retrieved!\n" );
            }
        }
    }
    else
    {
        OSL_ENSURE( sal_False, "Can not get properties from a package stream!\n" );
        throw uno::RuntimeException();
    }

    return aResult;
}

//-----------------------------------------------
void OWriteStream_Impl::CopyInternallyTo_Impl( const uno::Reference< io::XStream >& xDestStream,
                                                const ::rtl::OUString& aPass )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( !m_bUseCommonPass, "The stream can not be encrypted!" );

    if ( m_pAntiImpl )
    {
        m_pAntiImpl->CopyToStreamInternally_Impl( xDestStream );
    }
    else
    {
        uno::Reference< io::XStream > xOwnStream = GetStream( embed::ElementModes::READ, aPass );
        if ( !xOwnStream.is() )
            throw io::IOException(); // TODO

        completeStorageStreamCopy_Impl( xOwnStream, xDestStream );
    }

    uno::Reference< embed::XEncryptionProtectedSource > xEncr( xDestStream, uno::UNO_QUERY );
    if ( xEncr.is() )
        xEncr->setEncryptionPassword( aPass );
}

//-----------------------------------------------
void OWriteStream_Impl::CopyInternallyTo_Impl( const uno::Reference< io::XStream >& xDestStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( m_pAntiImpl )
    {
        m_pAntiImpl->CopyToStreamInternally_Impl( xDestStream );
    }
    else
    {
        uno::Reference< io::XStream > xOwnStream = GetStream( embed::ElementModes::READ );
        if ( !xOwnStream.is() )
            throw io::IOException(); // TODO

        completeStorageStreamCopy_Impl( xOwnStream, xDestStream );
    }

}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode, const ::rtl::OUString& aPass )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XStream > xResultStream;

    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    if ( m_bHasCachedPassword )
    {
        if ( !m_aPass.equals( aPass ) )
            throw packages::WrongPasswordException();

        // the correct key must be set already
        xResultStream = GetStream_Impl( nStreamMode );
    }
    else
    {
        SetEncryptionKeyProperty_Impl( xPropertySet, MakeKeyFromPass( aPass, sal_True ) );

        try {
            xResultStream = GetStream_Impl( nStreamMode );
            m_bHasCachedPassword = sal_True;
            m_aPass = aPass;
        }
        catch( packages::WrongPasswordException& )
        {
            // retry with different encoding
            SetEncryptionKeyProperty_Impl( xPropertySet, MakeKeyFromPass( aPass, sal_False ) );
            try {
                // the stream must be cashed to be resaved
                xResultStream = GetStream_Impl( nStreamMode | embed::ElementModes::SEEKABLE );
                m_bHasCachedPassword = sal_True;
                m_aPass = aPass;

                // the stream must be resaved with new password encryption
                if ( nStreamMode & embed::ElementModes::WRITE )
                {
                    GetFilledTempFile();
                    m_bHasDataToFlush = sal_True;

                    // TODO/LATER: should the notification be done?
                    if ( m_pParent )
                        m_pParent->m_bIsModified = sal_True;
                }
            }
            catch( packages::WrongPasswordException& )
            {
                SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
                throw;
            }
            catch ( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Can't write encryption related properties!\n" );
                SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
                throw io::IOException(); // TODO:
            }
        }
        catch( uno::Exception& )
        {
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );

            throw;
        }

    }

    OSL_ENSURE( xResultStream.is(), "In case stream can not be retrieved an exception must be thrown!\n" );

    return xResultStream;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    uno::Reference< io::XStream > xResultStream;

    if ( IsEncrypted() )
    {
        ::rtl::OUString aGlobalPass;
        try
        {
            aGlobalPass = GetCommonRootPass();
        }
        catch( packages::NoEncryptionException& )
        {
            throw packages::WrongPasswordException();
        }

        xResultStream = GetStream( nStreamMode, aGlobalPass );
    }
    else
        xResultStream = GetStream_Impl( nStreamMode );

    return xResultStream;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream_Impl( sal_Int32 nStreamMode )
{
    // private method, no mutex is used
    GetStreamProperties();

    if ( ( nStreamMode & embed::ElementModes::READWRITE ) == embed::ElementModes::READ )
    {
        uno::Reference< io::XInputStream > xInStream;
        if ( m_aTempURL.getLength() )
            xInStream = GetTempFileAsInputStream(); //TODO:
        else
            xInStream = m_xPackageStream->getDataStream();

        // The stream does not exist in the storage
        if ( !xInStream.is() )
            throw io::IOException();

        OInputCompStream* pStream = new OInputCompStream( *this, xInStream, InsertOwnProps( m_aProps, m_bUseCommonPass ) );
        uno::Reference< io::XStream > xCompStream(
                        static_cast< ::cppu::OWeakObject* >( pStream ),
                        uno::UNO_QUERY );
        OSL_ENSURE( xCompStream.is(),
                    "OInputCompStream MUST provide XStream interfaces!\n" );

        m_aInputStreamsList.push_back( pStream );
        return xCompStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::READWRITE ) == embed::ElementModes::SEEKABLEREAD )
    {
        if ( !m_aTempURL.getLength() && !( m_xPackageStream->getDataStream().is() ) )
        {
            // The stream does not exist in the storage
            throw io::IOException();
        }

        uno::Reference< io::XInputStream > xInStream;

        xInStream = GetTempFileAsInputStream(); //TODO:

        if ( !xInStream.is() )
            throw io::IOException();

        OInputSeekStream* pStream = new OInputSeekStream( *this, xInStream, InsertOwnProps( m_aProps, m_bUseCommonPass ) );
        uno::Reference< io::XStream > xSeekStream(
                        static_cast< ::cppu::OWeakObject* >( pStream ),
                        uno::UNO_QUERY );
        OSL_ENSURE( xSeekStream.is(),
                    "OInputSeekStream MUST provide XStream interfaces!\n" );

        m_aInputStreamsList.push_back( pStream );
        return xSeekStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::WRITE ) == embed::ElementModes::WRITE )
    {
        if ( !m_aInputStreamsList.empty() )
            throw io::IOException(); // TODO:

        uno::Reference< io::XStream > xStream;
        if ( ( nStreamMode & embed::ElementModes::TRUNCATE ) == embed::ElementModes::TRUNCATE )
        {
            if ( m_aTempURL.getLength() )
                KillFile( m_aTempURL, GetServiceFactory() );

            // open new empty temp file
            m_aTempURL = GetNewTempFileURL( GetServiceFactory() );

            m_bHasDataToFlush = sal_True;

            // this call is triggered by the parent and it will recognize the change of the state
            if ( m_pParent )
                m_pParent->m_bIsModified = sal_True;

            xStream = GetTempFileAsStream();
        }
        else if ( !m_bHasInsertedStreamOptimization )
        {
            if ( !m_aTempURL.getLength() && !( m_xPackageStream->getDataStream().is() ) )
            {
                // The stream does not exist in the storage
                m_bHasDataToFlush = sal_True;

                // this call is triggered by the parent and it will recognize the change of the state
                if ( m_pParent )
                    m_pParent->m_bIsModified = sal_True;
                xStream = GetTempFileAsStream();
            }

            // if the stream exists the temporary file is created on demand
            // xStream = GetTempFileAsStream();
        }

        if ( !xStream.is() )
            m_pAntiImpl = new OWriteStream( this );
        else
            m_pAntiImpl = new OWriteStream( this, xStream );

        uno::Reference< io::XStream > xWriteStream =
                                uno::Reference< io::XStream >( static_cast< ::cppu::OWeakObject* >( m_pAntiImpl ),
                                                                uno::UNO_QUERY );

        OSL_ENSURE( xWriteStream.is(), "OWriteStream MUST implement XStream && XComponent interfaces!\n" );

        return xWriteStream;
    }

    throw lang::IllegalArgumentException(); // TODO
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OWriteStream_Impl::GetPlainRawInStream()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    // this method is used only internally, this stream object should not go outside of this implementation
    // if ( m_pAntiImpl )
    //  throw io::IOException(); // TODO:

    return m_xPackageStream->getPlainRawStream();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > OWriteStream_Impl::GetRawInStream()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    OSL_ENSURE( IsEncrypted(), "Impossible to get raw representation for nonencrypted stream!\n" );
    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    return m_xPackageStream->getRawStream();
}

//-----------------------------------------------
::rtl::OUString OWriteStream_Impl::GetCommonRootPass()
    throw ( packages::NoEncryptionException )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( !m_pParent )
        throw packages::NoEncryptionException();

    return m_pParent->GetCommonRootPass();
}

//-----------------------------------------------
void OWriteStream_Impl::InputStreamDisposed( OInputCompStream* pStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    m_aInputStreamsList.remove( pStream );
}

//-----------------------------------------------
void OWriteStream_Impl::CreateReadonlyCopyBasedOnData( const uno::Reference< io::XInputStream >& xDataToCopy, const uno::Sequence< beans::PropertyValue >& aProps, sal_Bool, uno::Reference< io::XStream >& xTargetStream )
{
    uno::Reference < io::XStream > xTempFile;
    if ( !xTargetStream.is() )
        xTempFile = uno::Reference < io::XStream >(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY );
    else
        xTempFile = xTargetStream;

    uno::Reference < io::XSeekable > xTempSeek( xTempFile, uno::UNO_QUERY );
    if ( !xTempSeek.is() )
        throw uno::RuntimeException(); // TODO

    uno::Reference < io::XOutputStream > xTempOut = xTempFile->getOutputStream();
    if ( !xTempOut.is() )
        throw uno::RuntimeException();

    if ( xDataToCopy.is() )
        ::comphelper::OStorageHelper::CopyInputToOutput( xDataToCopy, xTempOut );

    xTempOut->closeOutput();
    xTempSeek->seek( 0 );

    uno::Reference< io::XInputStream > xInStream = xTempFile->getInputStream();
    if ( !xInStream.is() )
        throw io::IOException();

    // TODO: remember last state of m_bUseCommonPass
    if ( !xTargetStream.is() )
        xTargetStream = uno::Reference< io::XStream > (
            static_cast< ::cppu::OWeakObject* >(
                new OInputSeekStream( xInStream, InsertOwnProps( aProps, m_bUseCommonPass ) ) ),
            uno::UNO_QUERY_THROW );
}

//-----------------------------------------------
void OWriteStream_Impl::GetCopyOfLastCommit( uno::Reference< io::XStream >& xTargetStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageStream.is(), "The source stream for copying is incomplete!\n" );
    if ( !m_xPackageStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XInputStream > xDataToCopy;
    if ( IsEncrypted() )
    {
        // an encrypted stream must contain input stream
        ::rtl::OUString aGlobalPass;
        try
        {
            aGlobalPass = GetCommonRootPass();
        }
        catch( packages::NoEncryptionException& )
        {
            throw packages::WrongPasswordException();
        }

        GetCopyOfLastCommit( xTargetStream, aGlobalPass );
    }
    else
    {
        xDataToCopy = m_xPackageStream->getDataStream();

        // in case of new inserted package stream it is possible that input stream still was not set
        GetStreamProperties();

        CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, m_bUseCommonPass, xTargetStream );
    }
}

//-----------------------------------------------
void OWriteStream_Impl::GetCopyOfLastCommit( uno::Reference< io::XStream >& xTargetStream, const ::rtl::OUString& aPass )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );

    OSL_ENSURE( m_xPackageStream.is(), "The source stream for copying is incomplete!\n" );
    if ( !m_xPackageStream.is() )
        throw uno::RuntimeException();

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XInputStream > xDataToCopy;

    if ( m_bHasCachedPassword )
    {
        // TODO: introduce last commited cashed password information and use it here
        // that means "use common pass" also should be remembered on flash
        uno::Sequence< sal_Int8 > aNewKey = MakeKeyFromPass( aPass, sal_True );
        uno::Sequence< sal_Int8 > aOldKey = MakeKeyFromPass( aPass, sal_False );

        uno::Reference< beans::XPropertySet > xProps( m_xPackageStream, uno::UNO_QUERY );
        if ( !xProps.is() )
            throw uno::RuntimeException();

        sal_Bool bEncr = sal_False;
        xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Encrypted" ) ) >>= bEncr;
        if ( !bEncr )
            throw packages::NoEncryptionException();

        uno::Sequence< sal_Int8 > aEncrKey;
        xProps->getPropertyValue( ::rtl::OUString::createFromAscii( "EncryptionKey" ) ) >>= aEncrKey;
        if ( !SequencesEqual( aNewKey, aEncrKey ) && !SequencesEqual( aOldKey, aEncrKey ) )
            throw packages::WrongPasswordException();

        // the correct key must be set already
        xDataToCopy = m_xPackageStream->getDataStream();
    }
    else
    {
        uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
        SetEncryptionKeyProperty_Impl( xPropertySet, MakeKeyFromPass( aPass, sal_True ) );

        try {
            xDataToCopy = m_xPackageStream->getDataStream();

            if ( !xDataToCopy.is() )
            {
                OSL_ENSURE( sal_False, "Encrypted ZipStream must already have input stream inside!\n" );
                SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
            }
        }
        catch( packages::WrongPasswordException& )
        {
            SetEncryptionKeyProperty_Impl( xPropertySet, MakeKeyFromPass( aPass, sal_False ) );
            try {
                xDataToCopy = m_xPackageStream->getDataStream();

                if ( !xDataToCopy.is() )
                {
                    OSL_ENSURE( sal_False, "Encrypted ZipStream must already have input stream inside!\n" );
                    SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
                    throw;
                }
            }
            catch( uno::Exception& )
            {
                SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
                throw;
            }
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Can't open encrypted stream!\n" );
            SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
            throw;
        }

        SetEncryptionKeyProperty_Impl( xPropertySet, uno::Sequence< sal_Int8 >() );
    }

    // in case of new inserted package stream it is possible that input stream still was not set
    GetStreamProperties();

    CreateReadonlyCopyBasedOnData( xDataToCopy, m_aProps, m_bUseCommonPass, xTargetStream );
}

//===============================================
// OWriteStream implementation
//===============================================

//-----------------------------------------------
OWriteStream::OWriteStream( OWriteStream_Impl* pImpl )
: m_pImpl( pImpl )
, m_bInStreamDisconnected( sal_False )
, m_bInitOnDemand( sal_True )
{
    OSL_ENSURE( pImpl, "No base implementation!\n" );
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex!\n" );

    if ( !m_pImpl || !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster

    m_pData = new WSInternalData_Impl( pImpl->m_rMutexRef );
}

//-----------------------------------------------
OWriteStream::OWriteStream( OWriteStream_Impl* pImpl, uno::Reference< io::XStream > xStream )
: m_pImpl( pImpl )
, m_bInStreamDisconnected( sal_False )
, m_bInitOnDemand( sal_False )
{
    OSL_ENSURE( pImpl && xStream.is(), "No base implementation!\n" );
    OSL_ENSURE( m_pImpl->m_rMutexRef.Is(), "No mutex!\n" );

    if ( !m_pImpl || !m_pImpl->m_rMutexRef.Is() )
        throw uno::RuntimeException(); // just a disaster

    m_pData = new WSInternalData_Impl( pImpl->m_rMutexRef );

    if ( xStream.is() )
    {
        m_xInStream = xStream->getInputStream();
        m_xOutStream = xStream->getOutputStream();
        m_xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );
        OSL_ENSURE( m_xInStream.is() && m_xOutStream.is() && m_xSeekable.is(), "Stream implementation is incomplete!\n" );
    }
}

//-----------------------------------------------
OWriteStream::~OWriteStream()
{
    {
        ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );
        if ( m_pImpl )
        {
            m_refCount++;
            try {
                dispose();
            }
            catch( uno::RuntimeException& )
            {}
        }
    }

    if ( m_pData )
        delete m_pData;
}

//-----------------------------------------------
void OWriteStream::CheckInitOnDemand()
{
    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_bInitOnDemand )
    {
        RTL_LOGFILE_CONTEXT( aLog, "package (mv76033) OWriteStream::CheckInitOnDemand, initializing" );
        uno::Reference< io::XStream > xStream = m_pImpl->GetTempFileAsStream();
        if ( xStream.is() )
        {
            m_xInStream = xStream->getInputStream();
            m_xOutStream = xStream->getOutputStream();
            m_xSeekable = uno::Reference< io::XSeekable >( xStream, uno::UNO_QUERY );
            OSL_ENSURE( m_xInStream.is() && m_xOutStream.is() && m_xSeekable.is(), "Stream implementation is incomplete!\n" );

            m_bInitOnDemand = sal_False;
        }
    }
}

//-----------------------------------------------
void OWriteStream::CopyToStreamInternally_Impl( const uno::Reference< io::XStream >& xDest )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_xInStream.is() )
        throw uno::RuntimeException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    uno::Reference< beans::XPropertySet > xDestProps( xDest, uno::UNO_QUERY );
    if ( !xDestProps.is() )
        throw uno::RuntimeException(); //TODO

    uno::Reference< io::XOutputStream > xDestOutStream = xDest->getOutputStream();
    if ( !xDestOutStream.is() )
        throw io::IOException(); // TODO

    sal_Int64 nCurPos = m_xSeekable->getPosition();
    m_xSeekable->seek( 0 );

    uno::Exception eThrown;
    sal_Bool bThrown = sal_False;
    try {
        ::comphelper::OStorageHelper::CopyInputToOutput( m_xInStream, xDestOutStream );
    }
    catch ( uno::Exception& e )
    {
        eThrown = e;
        bThrown = sal_True;
    }

    // position-related section below is critical
    // if it fails the stream will become invalid
    try {
        m_xSeekable->seek( nCurPos );
    }
    catch ( uno::Exception& )
    {
        // TODO: set the stoream in invalid state or dispose
        OSL_ENSURE( sal_False, "The stream become invalid during copiing!\n" );
        throw uno::RuntimeException();
    }

    if ( bThrown )
        throw eThrown;

    // now the properties can be copied
    const char* pStrings[3] = { "MediaType", "Compressed", "UseCommonStoragePasswordEncryption" };
    for ( int ind = 0; ind < 3; ind++ )
    {
        ::rtl::OUString aPropName = ::rtl::OUString::createFromAscii( pStrings[ind] );
        xDestProps->setPropertyValue( aPropName, getPropertyValue( aPropName ) );
    }
}

//-----------------------------------------------
void OWriteStream::ModifyParentUnlockMutex_Impl( ::osl::ResettableMutexGuard& aGuard )
{
    if ( m_pImpl->m_pParent )
    {
        if ( m_pImpl->m_pParent->m_pAntiImpl )
        {
            uno::Reference< util::XModifiable > xParentModif( (util::XModifiable*)(m_pImpl->m_pParent->m_pAntiImpl) );
            aGuard.clear();
            xParentModif->setModified( sal_True );
        }
        else
            m_pImpl->m_pParent->m_bIsModified = sal_True;
    }
}

//-----------------------------------------------
sal_Int32 SAL_CALL OWriteStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readBytes( aData, nBytesToRead );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OWriteStream::readSomeBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->readSomeBytes( aData, nMaxBytesToRead );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::skipBytes( sal_Int32 nBytesToSkip )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    m_xInStream->skipBytes( nBytesToSkip );
}

//-----------------------------------------------
sal_Int32 SAL_CALL OWriteStream::available(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    return m_xInStream->available();

}

//-----------------------------------------------
void SAL_CALL OWriteStream::closeInput(  )
        throw ( io::NotConnectedException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        throw io::NotConnectedException();

    // the input part of the stream stays open for internal purposes ( to allow reading during copiing )
    // since it can not be reopened until output part is closed, it will be closed with output part.
    m_bInStreamDisconnected = sal_True;
    // m_xInStream->closeInput();
    // m_xInStream = uno::Reference< io::XInputStream >();

    if ( !m_xOutStream.is() )
        dispose();
}

//-----------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL OWriteStream::getInputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_bInitOnDemand && ( m_bInStreamDisconnected || !m_xInStream.is() ) )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OWriteStream::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        return uno::Reference< io::XOutputStream >();

    return uno::Reference< io::XOutputStream >( static_cast< io::XOutputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::writeBytes( const uno::Sequence< sal_Int8 >& aData )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    m_xOutStream->writeBytes( aData );
    m_pImpl->m_bHasDataToFlush = sal_True;

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::flush()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    // In case stream is flushed it's current version becomes visible
    // to the parent storage. Usually parent storage flushes the stream
    // during own commit but a user can explicitly flush the stream
    // so the changes will be available through cloning functionality.

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_bInitOnDemand )
    {
        if ( !m_xOutStream.is() )
            throw io::NotConnectedException();

        m_xOutStream->flush();
        m_pImpl->Commit();
    }
}

//-----------------------------------------------
void OWriteStream::CloseOutput_Impl()
{
    // all the checks must be done in calling method

    CheckInitOnDemand();

    m_xOutStream->closeOutput();
    m_xOutStream = uno::Reference< io::XOutputStream >();

    // after the stream is disposed it can be commited
    // so transport correct size property
    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
    {
        if ( m_pImpl->m_aProps[nInd].Name.equalsAscii( "Size" ) )
            m_pImpl->m_aProps[nInd].Value <<= ((sal_Int32)m_xSeekable->getLength());
    }
}

//-----------------------------------------------
void SAL_CALL OWriteStream::closeOutput()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    CloseOutput_Impl();

    if ( m_bInStreamDisconnected || !m_xInStream.is() )
        dispose();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    m_xSeekable->seek( location );
}

//-----------------------------------------------
sal_Int64 SAL_CALL OWriteStream::getPosition()
        throw ( io::IOException,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getPosition();
}

//-----------------------------------------------
sal_Int64 SAL_CALL OWriteStream::getLength()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xSeekable.is() )
        throw uno::RuntimeException();

    return m_xSeekable->getLength();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::truncate()
        throw ( io::IOException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        throw uno::RuntimeException();

    uno::Reference< io::XTruncate > xTruncate( m_xOutStream, uno::UNO_QUERY );

    if ( !xTruncate.is() )
    {
        OSL_ENSURE( sal_False, "The output stream must support XTruncate interface!\n" );
        throw uno::RuntimeException();
    }

    xTruncate->truncate();

    m_pImpl->m_bHasDataToFlush = sal_True;

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::dispose()
        throw ( uno::RuntimeException )
{
    // should be an internal method since it can be called only from parent storage

    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( m_xOutStream.is() )
        CloseOutput_Impl();

    if ( m_xInStream.is() )
    {
        m_xInStream->closeInput();
        m_xInStream = uno::Reference< io::XInputStream >();
    }

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    m_pData->m_aListenersContainer.disposeAndClear( aSource );

    m_pImpl->m_pAntiImpl = NULL;

    if ( !m_bInitOnDemand )
        m_pImpl->Commit();

    m_pImpl = NULL;
}

//-----------------------------------------------
void SAL_CALL OWriteStream::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.removeInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::setEncryptionPassword( const ::rtl::OUString& aPass )
    throw ( uno::RuntimeException,
            io::IOException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!\n" );

    m_pImpl->SetEncryptedWithPass( aPass );

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeEncryption()
    throw ( uno::RuntimeException,
            io::IOException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    CheckInitOnDemand();

    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!\n" );

    m_pImpl->SetDecrypted();

    ModifyParentUnlockMutex_Impl( aGuard );
}

//-----------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OWriteStream::getPropertySetInfo()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    //TODO:
    return uno::Reference< beans::XPropertySetInfo >();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw ( beans::UnknownPropertyException,
                beans::PropertyVetoException,
                lang::IllegalArgumentException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::ResettableMutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pImpl->GetStreamProperties();

    if ( aPropertyName.equalsAscii( "MediaType" )
      || aPropertyName.equalsAscii( "Compressed" ) )
    {
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                m_pImpl->m_aProps[nInd].Value = aValue;
        }
    }
    else if ( aPropertyName.equalsAscii( "UseCommonStoragePasswordEncryption" ) )
    {
        sal_Bool bUseCommonPass = sal_False;
        if ( aValue >>= bUseCommonPass )
        {
            if ( m_bInitOnDemand && m_pImpl->m_bHasInsertedStreamOptimization )
            {
                // the data stream is provided to the packagestream directly
                m_pImpl->m_bUseCommonPass = bUseCommonPass;
            }
            else if ( bUseCommonPass )
            {
                if ( !m_pImpl->m_bUseCommonPass )
                {
                    m_pImpl->SetDecrypted();
                    m_pImpl->m_bUseCommonPass = sal_True;
                }
            }
            else
                m_pImpl->m_bUseCommonPass = sal_False;
        }
        else
            throw lang::IllegalArgumentException(); //TODO
    }
    else if ( aPropertyName.equalsAscii( "IsEncrypted" ) || aPropertyName.equalsAscii( "Encrypted" )
            || aPropertyName.equalsAscii( "Size" ) )
        throw beans::PropertyVetoException(); // TODO
    else
        throw beans::UnknownPropertyException(); // TODO

    m_pImpl->m_bHasDataToFlush = sal_True;
    ModifyParentUnlockMutex_Impl( aGuard );
}


//-----------------------------------------------
uno::Any SAL_CALL OWriteStream::getPropertyValue( const ::rtl::OUString& aProp )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    ::rtl::OUString aPropertyName;
    if ( aProp.equalsAscii( "IsEncrypted" ) )
        aPropertyName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Encrypted" ) );
    else
        aPropertyName = aProp;

    if ( aPropertyName.equalsAscii( "MediaType" )
      || aPropertyName.equalsAscii( "Encrypted" )
      || aPropertyName.equalsAscii( "Compressed" ) )
    {
        m_pImpl->GetStreamProperties();

        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                return m_pImpl->m_aProps[nInd].Value;
        }
    }
    else if ( aPropertyName.equalsAscii( "UseCommonStoragePasswordEncryption" ) )
        return uno::makeAny( m_pImpl->m_bUseCommonPass );
    else if ( aPropertyName.equalsAscii( "Size" ) )
    {
        CheckInitOnDemand();

        if ( !m_xSeekable.is() )
            throw uno::RuntimeException();

        return uno::makeAny( (sal_Int32)m_xSeekable->getLength() );
    }

    throw beans::UnknownPropertyException(); // TODO
}


//-----------------------------------------------
void SAL_CALL OWriteStream::addPropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OWriteStream::removePropertyChangeListener(
    const ::rtl::OUString& /*aPropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OWriteStream::addVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


//-----------------------------------------------
void SAL_CALL OWriteStream::removeVetoableChangeListener(
    const ::rtl::OUString& /*PropertyName*/,
    const uno::Reference< beans::XVetoableChangeListener >& /*aListener*/ )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


