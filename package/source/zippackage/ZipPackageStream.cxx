/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ZipPackageStream.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 09:05:31 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif

#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif


#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif
#ifndef _ZIP_PACKAGE_HXX
#include <ZipPackage.hxx>
#endif
#ifndef _ZIP_FILE_HXX
#include <ZipFile.hxx>
#endif
#ifndef _ENCRYPTED_DATA_HEADER_HXX_
#include <EncryptedDataHeader.hxx>
#endif

#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

#ifndef _WRAPSTREAMFORSHARE_HXX_
#include "wrapstreamforshare.hxx"
#endif

#include <comphelper/seekableinput.hxx>
#include <comphelper/storagehelper.hxx>

#include <PackageConstants.hxx>

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star;
using namespace cppu;
using namespace rtl;

Sequence < sal_Int8 > ZipPackageStream::aImplementationId = Sequence < sal_Int8 > ();


ZipPackageStream::ZipPackageStream ( ZipPackage & rNewPackage,
                                    const Reference< XMultiServiceFactory >& xFactory,
                                    sal_Bool bAllowRemoveOnInsert )
: m_xFactory( xFactory )
, rZipPackage(rNewPackage)
, bToBeCompressed ( sal_True )
, bToBeEncrypted ( sal_False )
, bHaveOwnKey ( sal_False )
, bIsEncrypted ( sal_False )
, xEncryptionData ( )
, m_nStreamMode( PACKAGE_STREAM_NOTSET )
, m_nMagicalHackPos( 0 )
, m_nMagicalHackSize( 0 )
, m_bHasSeekable( sal_False )
, m_bCompressedIsSetFromOutside( sal_False )
{
    OSL_ENSURE( m_xFactory.is(), "No factory is provided to ZipPackageStream!\n" );

    this->mbAllowRemoveOnInsert = bAllowRemoveOnInsert;

    SetFolder ( sal_False );
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = -1;
    aEntry.nTime        = -1;
    aEntry.nCrc         = -1;
    aEntry.nCompressedSize  = -1;
    aEntry.nSize        = -1;
    aEntry.nOffset      = -1;
    aEntry.nNameLen     = -1;
    aEntry.nExtraLen    = -1;

    if ( !aImplementationId.getLength() )
        {
            aImplementationId = getImplementationId();
        }
}

ZipPackageStream::~ZipPackageStream( void )
{
}

void ZipPackageStream::setZipEntryOnLoading( const ZipEntry &rInEntry)
{
    aEntry.nVersion = rInEntry.nVersion;
    aEntry.nFlag = rInEntry.nFlag;
    aEntry.nMethod = rInEntry.nMethod;
    aEntry.nTime = rInEntry.nTime;
    aEntry.nCrc = rInEntry.nCrc;
    aEntry.nCompressedSize = rInEntry.nCompressedSize;
    aEntry.nSize = rInEntry.nSize;
    aEntry.nOffset = rInEntry.nOffset;
    aEntry.sName = rInEntry.sName;
    aEntry.nNameLen = rInEntry.nNameLen;
    aEntry.nExtraLen = rInEntry.nExtraLen;

    if ( aEntry.nMethod == STORED )
        bToBeCompressed = sal_False;
}

//--------------------------------------------------------------------------
void ZipPackageStream::CloseOwnStreamIfAny()
{
    if ( xStream.is() )
    {
        xStream->closeInput();
        xStream = uno::Reference< io::XInputStream >();
        m_bHasSeekable = sal_False;
    }
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream >& ZipPackageStream::GetOwnSeekStream()
{
    if ( !m_bHasSeekable && xStream.is() )
    {
        // The package component requires that every stream either be FROM a package or it must support XSeekable!
        // The only exception is a nonseekable stream that is provided only for storing, if such a stream
        // is accessed before commit it MUST be wrapped.
        // Wrap the stream in case it is not seekable
        xStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( xStream, m_xFactory );
        Reference< io::XSeekable > xSeek( xStream, UNO_QUERY );
        if ( !xSeek.is() )
            throw RuntimeException( OUString::createFromAscii( "The stream must support XSeekable!" ),
                                    Reference< XInterface >() );

        m_bHasSeekable = sal_True;
    }

    return xStream;
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > ZipPackageStream::GetRawEncrStreamNoHeaderCopy()
{
    if ( m_nStreamMode != PACKAGE_STREAM_RAW || !GetOwnSeekStream().is() )
        throw io::IOException(); // TODO

    if ( xEncryptionData.isEmpty() )
        throw ZipIOException( OUString::createFromAscii( "Encrypted stream without encryption data!\n" ),
                            Reference< XInterface >() );

    uno::Reference< io::XSeekable > xSeek( GetOwnSeekStream(), UNO_QUERY );
    if ( !xSeek.is() )
        throw ZipIOException( OUString::createFromAscii( "The stream must be seekable!\n" ),
                            Reference< XInterface >() );

    // skip header
    xSeek->seek( n_ConstHeaderSize + xEncryptionData->aInitVector.getLength() +
                    xEncryptionData->aSalt.getLength() + xEncryptionData->aDigest.getLength() );

    // create temporary stream
    uno::Reference < io::XOutputStream > xTempOut(
                        m_xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                        uno::UNO_QUERY );
    uno::Reference < io::XInputStream > xTempIn( xTempOut, UNO_QUERY );
    uno::Reference < io::XSeekable > xTempSeek( xTempOut, UNO_QUERY );
    if ( !xTempOut.is() || !xTempIn.is() || !xTempSeek.is() )
        throw io::IOException(); // TODO:

    // copy the raw stream to the temporary file starting from the current position
    ::comphelper::OStorageHelper::CopyInputToOutput( GetOwnSeekStream(), xTempOut );
    xTempOut->closeOutput();
    xTempSeek->seek( 0 );

    return xTempIn;
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > ZipPackageStream::TryToGetRawFromDataStream( sal_Bool bAddHeaderForEncr )
{
    if ( m_nStreamMode != PACKAGE_STREAM_DATA || !GetOwnSeekStream().is() || bAddHeaderForEncr && !bToBeEncrypted )
        throw packages::NoEncryptionException(); // TODO

    Sequence< sal_Int8 > aKey;

    if ( bToBeEncrypted )
    {
        aKey = ( xEncryptionData.isEmpty() || !bHaveOwnKey ) ? rZipPackage.getEncryptionKey() :
                                                                                xEncryptionData->aKey;
        if ( !aKey.getLength() )
            throw packages::NoEncryptionException(); // TODO
    }

    try
    {
        // create temporary file
        uno::Reference < io::XStream > xTempStream(
                            m_xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                            uno::UNO_QUERY );
        if ( !xTempStream.is() )
            throw io::IOException(); // TODO:

        // create a package based on it
        ZipPackage* pPackage = new ZipPackage( m_xFactory );
        Reference< XSingleServiceFactory > xPackageAsFactory( static_cast< XSingleServiceFactory* >( pPackage ) );
        if ( !xPackageAsFactory.is() )
            throw RuntimeException(); // TODO

        Sequence< Any > aArgs( 1 );
        aArgs[0] <<= xTempStream;
        pPackage->initialize( aArgs );

        // create a new package stream
        Reference< XDataSinkEncrSupport > xNewPackStream( xPackageAsFactory->createInstance(), UNO_QUERY );
        if ( !xNewPackStream.is() )
            throw RuntimeException(); // TODO

        if ( !m_aSharedMutexRef.Is() )
            m_aSharedMutexRef = new SotMutexHolder();
        xNewPackStream->setDataStream( static_cast< io::XInputStream* >(
                                                    new WrapStreamForShare( GetOwnSeekStream(), m_aSharedMutexRef ) ) );

        Reference< XPropertySet > xNewPSProps( xNewPackStream, UNO_QUERY );
        if ( !xNewPSProps.is() )
            throw RuntimeException(); // TODO

        // copy all the properties of this stream to the new stream
        xNewPSProps->setPropertyValue( ::rtl::OUString::createFromAscii( "MediaType" ), makeAny( sMediaType ) );
        xNewPSProps->setPropertyValue( ::rtl::OUString::createFromAscii( "Compressed" ), makeAny( bToBeCompressed ) );
        if ( bToBeEncrypted )
        {
            xNewPSProps->setPropertyValue( ::rtl::OUString::createFromAscii( "EncryptionKey" ), makeAny( aKey ) );
            xNewPSProps->setPropertyValue( ::rtl::OUString::createFromAscii( "Encrypted" ), makeAny( sal_True ) );
        }

        // insert a new stream in the package
        Reference< XUnoTunnel > xTunnel;
        Any aRoot = pPackage->getByHierarchicalName( ::rtl::OUString::createFromAscii( "/" ) );
        aRoot >>= xTunnel;
        Reference< container::XNameContainer > xRootNameContainer( xTunnel, UNO_QUERY );
        if ( !xRootNameContainer.is() )
            throw RuntimeException(); // TODO

        Reference< XUnoTunnel > xNPSTunnel( xNewPackStream, UNO_QUERY );
        xRootNameContainer->insertByName( ::rtl::OUString::createFromAscii( "dummy" ), makeAny( xNPSTunnel ) );

        // commit the temporary package
        pPackage->commitChanges();

        // get raw stream from the temporary package
        Reference< io::XInputStream > xInRaw;
        if ( bAddHeaderForEncr )
            xInRaw = xNewPackStream->getRawStream();
        else
            xInRaw = xNewPackStream->getPlainRawStream();

        // create another temporary file
        uno::Reference < io::XOutputStream > xTempOut(
                            m_xFactory->createInstance ( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
                            uno::UNO_QUERY );
        uno::Reference < io::XInputStream > xTempIn( xTempOut, UNO_QUERY );
        uno::Reference < io::XSeekable > xTempSeek( xTempOut, UNO_QUERY );
        if ( !xTempOut.is() || !xTempIn.is() || !xTempSeek.is() )
            throw io::IOException(); // TODO:

        // copy the raw stream to the temporary file
        ::comphelper::OStorageHelper::CopyInputToOutput( xInRaw, xTempOut );
        xTempOut->closeOutput();
        xTempSeek->seek( 0 );

        // close raw stream, package stream and folder
        xInRaw = Reference< io::XInputStream >();
        xNewPSProps = Reference< XPropertySet >();
        xNPSTunnel = Reference< XUnoTunnel >();
        xNewPackStream = Reference< XDataSinkEncrSupport >();
        xTunnel = Reference< XUnoTunnel >();
        xRootNameContainer = Reference< container::XNameContainer >();

        // return the stream representing the first temporary file
        return xTempIn;
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
    }

    throw io::IOException(); // TODO
}

//--------------------------------------------------------------------------
sal_Bool ZipPackageStream::ParsePackageRawStream()
{
    OSL_ENSURE( GetOwnSeekStream().is(), "A stream must be provided!\n" );

    if ( !GetOwnSeekStream().is() )
        return sal_False;

    sal_Bool bOk = sal_False;

    vos::ORef < EncryptionData > xTempEncrData;
    sal_Int32 nMagHackSize = 0;
    Sequence < sal_Int8 > aHeader ( 4 );

    try
    {
        if ( GetOwnSeekStream()->readBytes ( aHeader, 4 ) == 4 )
        {
            const sal_Int8 *pHeader = aHeader.getConstArray();
            sal_uInt32 nHeader = ( pHeader [0] & 0xFF )       |
                                 ( pHeader [1] & 0xFF ) << 8  |
                                 ( pHeader [2] & 0xFF ) << 16 |
                                 ( pHeader [3] & 0xFF ) << 24;
            if ( nHeader == n_ConstHeader )
            {
                // this is one of our god-awful, but extremely devious hacks, everyone cheer
                xTempEncrData = new EncryptionData;

                ::rtl::OUString aMediaType;
                if ( ZipFile::StaticFillData ( xTempEncrData, nMagHackSize, aMediaType, GetOwnSeekStream() ) )
                {
                    // We'll want to skip the data we've just read, so calculate how much we just read
                    // and remember it
                    m_nMagicalHackPos = n_ConstHeaderSize + xTempEncrData->aSalt.getLength()
                                                        + xTempEncrData->aInitVector.getLength()
                                                        + xTempEncrData->aDigest.getLength()
                                                        + aMediaType.getLength() * sizeof( sal_Unicode );
                    m_nMagicalHackSize = nMagHackSize;
                    sMediaType = aMediaType;

                    bOk = sal_True;
                }
            }
        }
    }
    catch( Exception& )
    {
    }

    if ( !bOk )
    {
        // the provided stream is not a raw stream
        return sal_False;
    }

    xEncryptionData = xTempEncrData;
    SetIsEncrypted ( sal_True );
    // it's already compressed and encrypted
    bToBeEncrypted = bToBeCompressed = sal_False;

    return sal_True;
}

void ZipPackageStream::SetPackageMember( sal_Bool bNewValue )
{
    if ( bNewValue )
    {
        m_nStreamMode = PACKAGE_STREAM_PACKAGEMEMBER;
        m_nMagicalHackPos = 0;
        m_nMagicalHackSize = 0;
    }
    else if ( m_nStreamMode == PACKAGE_STREAM_PACKAGEMEMBER )
        m_nStreamMode = PACKAGE_STREAM_NOTSET; // must be reset
}

// XActiveDataSink
//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setInputStream( const Reference< io::XInputStream >& aStream )
        throw(RuntimeException)
{
    // if seekable access is required the wrapping will be done on demand
    m_aSharedMutexRef = new SotMutexHolder();
    xStream = aStream;
    m_bHasSeekable = sal_False;
    SetPackageMember ( sal_False );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_DETECT;
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawData()
        throw(RuntimeException)
{
    try
    {
        if (IsPackageMember())
        {
            if ( !xEncryptionData.isEmpty() && !bHaveOwnKey )
                xEncryptionData->aKey = rZipPackage.getEncryptionKey();
            return rZipPackage.getZipFile().getRawData( aEntry, xEncryptionData, bIsEncrypted );
        }
        else if ( GetOwnSeekStream().is() )
        {
            if ( !m_aSharedMutexRef.Is() )
                m_aSharedMutexRef = new SotMutexHolder();
            return new WrapStreamForShare( GetOwnSeekStream(), m_aSharedMutexRef );
        }
        else
            return Reference < io::XInputStream > ();
    }
    catch (ZipException &)//rException)
    {
        VOS_ENSURE( 0, "ZipException thrown");//rException.Message);
        return Reference < io::XInputStream > ();
    }
    catch (Exception &)
    {
        VOS_ENSURE( 0, "Exception is thrown during stream wrapping!\n");
        return Reference < io::XInputStream > ();
    }
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getInputStream(  )
        throw(RuntimeException)
{
    try
    {
        if (IsPackageMember())
        {
            if ( !xEncryptionData.isEmpty() && !bHaveOwnKey )
                xEncryptionData->aKey = rZipPackage.getEncryptionKey();
            return rZipPackage.getZipFile().getInputStream( aEntry, xEncryptionData, bIsEncrypted );
        }
        else if ( GetOwnSeekStream().is() )
        {
            if ( !m_aSharedMutexRef.Is() )
                m_aSharedMutexRef = new SotMutexHolder();
            return new WrapStreamForShare( GetOwnSeekStream(), m_aSharedMutexRef );
        }
        else
            return Reference < io::XInputStream > ();
    }
    catch (ZipException &)//rException)
    {
        VOS_ENSURE( 0,"ZipException thrown");//rException.Message);
        return Reference < io::XInputStream > ();
    }
    catch (Exception &)
    {
        VOS_ENSURE( 0, "Exception is thrown during stream wrapping!\n");
        return Reference < io::XInputStream > ();
    }
}

// XDataSinkEncrSupport
//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getDataStream()
        throw ( packages::WrongPasswordException,
                io::IOException,
                RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException(); // TODO

    if ( !xEncryptionData.isEmpty() && !bHaveOwnKey )
        xEncryptionData->aKey = rZipPackage.getEncryptionKey();

    if (IsPackageMember())
    {
        if ( !xEncryptionData.isEmpty() && !bHaveOwnKey )
            xEncryptionData->aKey = rZipPackage.getEncryptionKey();

        return rZipPackage.getZipFile().getDataStream( aEntry, xEncryptionData, bIsEncrypted );
    }
    else if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        return ZipFile::StaticGetDataFromRawStream( GetOwnSeekStream(), xEncryptionData );
    else if ( GetOwnSeekStream().is() )
    {
        if ( !m_aSharedMutexRef.Is() )
            m_aSharedMutexRef = new SotMutexHolder();
        return new WrapStreamForShare( GetOwnSeekStream(), m_aSharedMutexRef );
    }
    else
        return uno::Reference< io::XInputStream >();
}

//--------------------------------------------------------------------------
Reference< io::XInputStream > SAL_CALL ZipPackageStream::getRawStream()
        throw ( packages::NoEncryptionException,
                io::IOException,
                uno::RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException(); // TODO

    if (IsPackageMember())
    {
        if ( !bIsEncrypted || xEncryptionData.isEmpty() )
            throw packages::NoEncryptionException(); // TODO

        return rZipPackage.getZipFile().getWrappedRawStream( aEntry, xEncryptionData, sMediaType );
    }
    else if ( GetOwnSeekStream().is() )
    {
        if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        {
            if ( !m_aSharedMutexRef.Is() )
                m_aSharedMutexRef = new SotMutexHolder();
            return new WrapStreamForShare( GetOwnSeekStream(), m_aSharedMutexRef );
        }
        else if ( m_nStreamMode == PACKAGE_STREAM_DATA && bToBeEncrypted )
            return TryToGetRawFromDataStream( sal_True );
    }

    throw packages::NoEncryptionException(); // TODO
}


//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setDataStream( const Reference< io::XInputStream >& aStream )
        throw ( io::IOException,
                RuntimeException )
{
    setInputStream( aStream );
    m_nStreamMode = PACKAGE_STREAM_DATA;
}

//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setRawStream( const Reference< io::XInputStream >& aStream )
        throw ( packages::EncryptionNotAllowedException,
                packages::NoRawFormatException,
                io::IOException,
                RuntimeException)
{
    // wrap the stream in case it is not seekable
    Reference< io::XInputStream > xNewStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( aStream, m_xFactory );
    Reference< io::XSeekable > xSeek( xNewStream, UNO_QUERY );
    if ( !xSeek.is() )
        throw RuntimeException( OUString::createFromAscii( "The stream must support XSeekable!" ),
                                    Reference< XInterface >() );

    xSeek->seek( 0 );
    Reference< io::XInputStream > xOldStream = xStream;
    xStream = xNewStream;
    if ( !ParsePackageRawStream() )
    {
        xStream = xOldStream;
        throw packages::NoRawFormatException();
    }

    // the raw stream MUST have seekable access
    m_bHasSeekable = sal_True;

    m_aSharedMutexRef = new SotMutexHolder();
    SetPackageMember ( sal_False );
    aEntry.nTime = -1;
    m_nStreamMode = PACKAGE_STREAM_RAW;
}

//--------------------------------------------------------------------------
uno::Reference< io::XInputStream > SAL_CALL ZipPackageStream::getPlainRawStream()
        throw ( io::IOException,
                uno::RuntimeException )
{
    // There is no stream attached to this object
    if ( m_nStreamMode == PACKAGE_STREAM_NOTSET )
        return Reference< io::XInputStream >();

    // this method can not be used together with old approach
    if ( m_nStreamMode == PACKAGE_STREAM_DETECT )
        throw packages::zip::ZipIOException(); // TODO

    if (IsPackageMember())
    {
        return rZipPackage.getZipFile().getRawData( aEntry, xEncryptionData, bIsEncrypted );
    }
    else if ( GetOwnSeekStream().is() )
    {
        if ( m_nStreamMode == PACKAGE_STREAM_RAW )
        {
            // the header should not be returned here
            return GetRawEncrStreamNoHeaderCopy();
        }
        else if ( m_nStreamMode == PACKAGE_STREAM_DATA )
            return TryToGetRawFromDataStream( sal_False );
    }

    return Reference< io::XInputStream >();
}

// XUnoTunnel

//--------------------------------------------------------------------------
sal_Int64 SAL_CALL ZipPackageStream::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == rtl_compareMemory( static_getImplementationId().getConstArray(), aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}

// XPropertySet
//--------------------------------------------------------------------------
void SAL_CALL ZipPackageStream::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(beans::UnknownPropertyException, beans::PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")))
    {
        if ( rZipPackage.getFormat() != PACKAGE_FORMAT && rZipPackage.getFormat() != OFOPXML_FORMAT )
            throw beans::PropertyVetoException();

        if ( aValue >>= sMediaType )
        {
            if (sMediaType.getLength() > 0)
            {
                if ( sMediaType.indexOf (OUString( RTL_CONSTASCII_USTRINGPARAM ( "text" ) ) ) != -1
                 || sMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM ( "application/vnd.sun.star.oleobject" ) ) ) )
                    bToBeCompressed = sal_True;
                else if ( !m_bCompressedIsSetFromOutside )
                    bToBeCompressed = sal_False;
            }
        }
        else
            throw IllegalArgumentException( OUString::createFromAscii( "MediaType must be a string!\n" ),
                                            Reference< XInterface >(),
                                            2 );

    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Size") ) )
    {
        if ( !( aValue >>= aEntry.nSize ) )
            throw IllegalArgumentException( OUString::createFromAscii( "Wrong type for Size property!\n" ),
                                            Reference< XInterface >(),
                                            2 );
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Encrypted") ) )
    {
        if ( rZipPackage.getFormat() != PACKAGE_FORMAT )
            throw beans::PropertyVetoException();

        sal_Bool bEnc = sal_False;
        if ( aValue >>= bEnc )
        {
            // In case of new raw stream, the stream must not be encrypted on storing
            if ( bEnc && m_nStreamMode == PACKAGE_STREAM_RAW )
                throw IllegalArgumentException( OUString::createFromAscii( "Raw stream can not be encrypted on storing" ),
                                                Reference< XInterface >(),
                                                2 );

            bToBeEncrypted = bEnc;
            if ( bToBeEncrypted && xEncryptionData.isEmpty())
                xEncryptionData = new EncryptionData;
        }
        else
            throw IllegalArgumentException( OUString::createFromAscii( "Wrong type for Encrypted property!\n" ),
                                            Reference< XInterface >(),
                                            2 );

    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("EncryptionKey") ) )
    {
        if ( rZipPackage.getFormat() != PACKAGE_FORMAT )
            throw beans::PropertyVetoException();

        Sequence < sal_Int8 > aNewKey;

        if ( !( aValue >>= aNewKey ) )
        {
            OUString sTempString;
            if ( ( aValue >>= sTempString ) )
            {
                sal_Int32 nNameLength = sTempString.getLength();
                Sequence < sal_Int8 > aSequence ( nNameLength );
                sal_Int8 *pArray = aSequence.getArray();
                const sal_Unicode *pChar = sTempString.getStr();
                for ( sal_Int16 i = 0; i < nNameLength; i++)
                    pArray[i] = static_cast < const sal_Int8 > (pChar[i]);
                aNewKey = aSequence;
            }
            else
                throw IllegalArgumentException( OUString::createFromAscii( "Wrong type for EncryptionKey property!\n" ),
                                                Reference< XInterface >(),
                                                2 );
        }

        if ( aNewKey.getLength() )
        {
            if ( xEncryptionData.isEmpty())
                xEncryptionData = new EncryptionData;

            xEncryptionData->aKey = aNewKey;
            // In case of new raw stream, the stream must not be encrypted on storing
            bHaveOwnKey = sal_True;
            if ( m_nStreamMode != PACKAGE_STREAM_RAW )
                bToBeEncrypted = sal_True;
        }
        else
            bHaveOwnKey = sal_False;
    }
    else if (aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
    {
        sal_Bool bCompr = sal_False;

        if ( aValue >>= bCompr )
        {
            // In case of new raw stream, the stream must not be encrypted on storing
            if ( bCompr && m_nStreamMode == PACKAGE_STREAM_RAW )
                throw IllegalArgumentException( OUString::createFromAscii( "Raw stream can not be encrypted on storing" ),
                                                Reference< XInterface >(),
                                                2 );

            bToBeCompressed = bCompr;
            m_bCompressedIsSetFromOutside = sal_True;
        }
        else
            throw IllegalArgumentException( OUString::createFromAscii( "Wrong type for Compressed property!\n" ),
                                            Reference< XInterface >(),
                                            2 );
    }
    else
        throw beans::UnknownPropertyException();
}

//--------------------------------------------------------------------------
Any SAL_CALL ZipPackageStream::getPropertyValue( const OUString& PropertyName )
        throw(beans::UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
    {
        aAny <<= sMediaType;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
    {
        aAny <<= aEntry.nSize;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Encrypted" ) ) )
    {
        aAny <<= ( m_nStreamMode == PACKAGE_STREAM_RAW ) ? sal_True : bToBeEncrypted;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "WasEncrypted" ) ) )
    {
        aAny <<= bIsEncrypted;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Compressed" ) ) )
    {
        aAny <<= bToBeCompressed;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "EncryptionKey" ) ) )
    {
        aAny <<= xEncryptionData.isEmpty () ? Sequence < sal_Int8 > () : xEncryptionData->aKey;
        return aAny;
    }
    else
        throw beans::UnknownPropertyException();
}

//--------------------------------------------------------------------------
void ZipPackageStream::setSize (const sal_Int32 nNewSize)
{
    if (aEntry.nCompressedSize != nNewSize )
        aEntry.nMethod = DEFLATED;
    aEntry.nSize = nNewSize;
}
//--------------------------------------------------------------------------
OUString ZipPackageStream::getImplementationName()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZipPackageStream" ) );
}

//--------------------------------------------------------------------------
Sequence< OUString > ZipPackageStream::getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.PackageStream" ) );
    return aNames;
}
//--------------------------------------------------------------------------
sal_Bool SAL_CALL ZipPackageStream::supportsService( OUString const & rServiceName )
    throw (RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}

