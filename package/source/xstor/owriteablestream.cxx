/*************************************************************************
 *
 *  $RCSfile: owriteablestream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2003-09-11 10:15:28 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "owriteablestream.hxx"

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
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

#include "oseekinstream.hxx"
#include "mutexholder.hxx"
#include "xstorage.hxx"

using namespace ::com::sun::star;

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
void copyInputToOutput_Impl( const uno::Reference< io::XInputStream >& aIn,
                             const uno::Reference< io::XOutputStream >& aOut )
{
    sal_Int32 nRead;
    uno::Sequence < sal_Int8 > aSequence ( n_ConstBufferSize );

    do
    {
        nRead = aIn->readBytes ( aSequence, n_ConstBufferSize );
        if ( nRead < n_ConstBufferSize )
        {
            uno::Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            aOut->writeBytes ( aTempBuf );
        }
        else
            aOut->writeBytes ( aSequence );
    }
    while ( nRead == n_ConstBufferSize );
}

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

// ================================================================

//-----------------------------------------------
OWriteStream_Impl::OWriteStream_Impl( OStorage_Impl* pParent,
                                      uno::Reference< packages::XDataSinkEncrSupport > xPackageStream,
                                      uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > xPackage,
                                      uno::Reference< lang::XMultiServiceFactory > xFactory,
                                      sal_Bool bForceEncrypted )
: m_pAntiImpl( NULL )
, m_bIsModified( sal_False )
, m_bCommited( sal_False )
, m_xPackageStream( xPackageStream )
, m_xFactory( xFactory )
, m_pParent( pParent )
, m_bHasCachedPassword( sal_False )
, m_bForceEncrypted( bForceEncrypted )
, m_xPackage( xPackage )
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
sal_Bool OWriteStream_Impl::IsEncrypted()
{
    if ( m_bForceEncrypted || m_bHasCachedPassword )
        return sal_True;

    if ( m_aTempURL.getLength() )
        return sal_False;

    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    ::rtl::OUString aString_EncryptionKey = ::rtl::OUString::createFromAscii( "Encrypted" );
    sal_Bool bEncrypted = sal_False;
    try {
        if ( !( xPropertySet->getPropertyValue( aString_EncryptionKey ) >>= bEncrypted ) )
            throw io::IOException();
    }
    catch ( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Can't write encryption related properties!\n" );
        throw io::IOException(); // TODO
    }

    return bEncrypted;
}


//-----------------------------------------------
void OWriteStream_Impl::DisposeWrappers()
{
    ::osl::MutexGuard( m_rMutexRef->GetMutex() );
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
::rtl::OUString OWriteStream_Impl::GetNewFilledTempFile()
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
                        copyInputToOutput_Impl( xOrigStream, xTempOutStream );
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
        m_aTempURL = GetNewFilledTempFile();

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

    copyInputToOutput_Impl( xTempInStream, xOutStream );
}

// =================================================================================================

//-----------------------------------------------
void OWriteStream_Impl::Commit()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    // this call can be made only during final commit to root storage
    // the  parent storage is responsible for the correct handling
    // of deleted and renamed contents

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( !m_bIsModified || !m_bCommited )
        return;

    // The stream must be free
    OSL_ENSURE( !m_pAntiImpl || !m_pAntiImpl->m_xOutStream.is(), "Commiting storage while a write stream is open!\n" );

    OSL_ENSURE( m_aTempURL.getLength(), "The temporary must exist!\n" );
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

    // use new file as current persistent representation
    // the new file will be removed after it's stream is closed
    m_xPackageStream->setDataStream( xTempIn );

    // copy properties to the package stream
    uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
    if ( !xPropertySet.is() )
        throw uno::RuntimeException();

    if ( m_bHasCachedPassword )
        xPropertySet->setPropertyValue( ::rtl::OUString::createFromAscii( "EncryptionKey" ), uno::makeAny( m_aKey ) );

    for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
        xPropertySet->setPropertyValue( m_aProps[nInd].Name, m_aProps[nInd].Value );

    // the stream should be free soon, after package is stored
    m_bIsModified = m_bCommited = sal_False;
}

//-----------------------------------------------
void OWriteStream_Impl::Revert()
{
    // can be called only from parent storage
    // means complete reload of the stream

    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    if ( !m_bIsModified )
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

    m_bCommited = m_bIsModified = sal_False;

    m_bHasCachedPassword = sal_False;
    m_aKey.realloc( 0 );
}

//-----------------------------------------------
uno::Sequence< beans::PropertyValue > OWriteStream_Impl::GetStreamProperties()
{
    if ( !m_aProps.getLength() )
    {
        m_aProps.realloc( 4 );
        m_aProps[0].Name = ::rtl::OUString::createFromAscii("MediaType");
        m_aProps[1].Name = ::rtl::OUString::createFromAscii("Size");
        m_aProps[2].Name = ::rtl::OUString::createFromAscii("Encrypted");
        m_aProps[3].Name = ::rtl::OUString::createFromAscii("Compressed");
        // TODO: may be also raw stream should be marked

        uno::Reference< beans::XPropertySet > xPropSet( m_xPackageStream, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            for ( sal_Int32 nInd = 0; nInd < m_aProps.getLength(); nInd++ )
            {
                try {
                    m_aProps[nInd].Value = xPropSet->getPropertyValue( m_aProps[nInd].Name );
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
    }

    return m_aProps;
}

//-----------------------------------------------
uno::Reference< io::XStream > OWriteStream_Impl::GetStream( sal_Int32 nStreamMode, const uno::Sequence< sal_Int8 >& aKey )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    OSL_ENSURE( m_xPackageStream.is(), "No package stream is set!\n" );

    if ( m_pAntiImpl )
        throw io::IOException(); // TODO:

    if ( !IsEncrypted() )
        throw packages::NoEncryptionException();

    uno::Reference< io::XStream > xResultStream;

    if ( m_bHasCachedPassword )
    {
        if ( !SequencesEqual( m_aKey, aKey ) )
            throw packages::WrongPasswordException();

        // the correct key must be set already
        xResultStream = GetStream_Impl( nStreamMode );
    }
    else
    {
        uno::Reference< beans::XPropertySet > xPropertySet( m_xPackageStream, uno::UNO_QUERY );
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

        try {
            xResultStream = GetStream_Impl( nStreamMode );
            m_bHasCachedPassword = sal_True;
            m_aKey = aKey;
        }
        catch( packages::WrongPasswordException& )
        {
            try {
                xPropertySet->setPropertyValue( aString_EncryptionKey, uno::makeAny( uno::Sequence< sal_Int8 >() ) );
            }
            catch ( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Can't write encryption related properties!\n" );
            }

            throw;
        }
        catch( uno::Exception& )
        {
            OSL_ENSURE( sal_False, "Can't open encrypted stream!\n" );
            try {
                xPropertySet->setPropertyValue( aString_EncryptionKey, uno::makeAny( uno::Sequence< sal_Int8 >() ) );
            }
            catch ( uno::Exception& )
            {
                OSL_ENSURE( sal_False, "Can't write encryption related properties!\n" );
            }

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
        uno::Sequence< sal_Int8 > aGlobalKey = GetCommonRootPass();

        if ( m_bHasCachedPassword && !SequencesEqual( m_aKey, aGlobalKey ) )
            throw packages::WrongPasswordException();

        xResultStream = GetStream_Impl( nStreamMode );

        if ( !m_bHasCachedPassword )
        {
            m_bHasCachedPassword = sal_True;
            m_aKey = aGlobalKey;
        }
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

    if ( ( nStreamMode & embed::ElementModes::ELEMENT_READWRITE ) == embed::ElementModes::ELEMENT_READ )
    {
        uno::Reference< io::XInputStream > xInStream;
        if ( m_aTempURL.getLength() )
            xInStream = GetTempFileAsInputStream(); //TODO:
        else
            xInStream = m_xPackageStream->getDataStream();

        if ( !xInStream.is() )
            throw io::IOException();

        OInputCompStream* pStream = new OInputCompStream( *this, xInStream, m_aProps );
        uno::Reference< io::XStream > xCompStream(
                        static_cast< ::cppu::OWeakObject* >( pStream ),
                        uno::UNO_QUERY );
        OSL_ENSURE( xCompStream.is(),
                    "OInputCompStream MUST provide XStream interfaces!\n" );

        m_aInputStreamsList.push_back( pStream );
        return xCompStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::ELEMENT_READWRITE ) == embed::ElementModes::ELEMENT_SEEKABLEREAD )
    {
        uno::Reference< io::XInputStream > xInStream;

        xInStream = GetTempFileAsInputStream(); //TODO:

        if ( !xInStream.is() )
            throw io::IOException();

        OInputSeekStream* pStream = new OInputSeekStream( *this, xInStream, m_aProps );
        uno::Reference< io::XStream > xSeekStream(
                        static_cast< ::cppu::OWeakObject* >( pStream ),
                        uno::UNO_QUERY );
        OSL_ENSURE( xSeekStream.is(),
                    "OInputSeekStream MUST provide XStream interfaces!\n" );

        m_aInputStreamsList.push_back( pStream );
        return xSeekStream;
    }
    else if ( ( nStreamMode & embed::ElementModes::ELEMENT_WRITE ) == embed::ElementModes::ELEMENT_WRITE )
    {
        if ( !m_aInputStreamsList.empty() )
            throw io::IOException(); // TODO:

        uno::Reference< io::XStream > xStream;
        if ( ( nStreamMode & embed::ElementModes::ELEMENT_TRUNCATE ) == embed::ElementModes::ELEMENT_TRUNCATE )
        {
            if ( m_aTempURL.getLength() )
                KillFile( m_aTempURL, GetServiceFactory() );

            // open new empty temp file
            m_aTempURL = GetNewTempFileURL( GetServiceFactory() );

            xStream = GetTempFileAsStream();
        }
        else
            xStream = GetTempFileAsStream();

        m_pAntiImpl = new OWriteStream( this, xStream );
        uno::Reference< io::XStream > xWriteStream =
                                uno::Reference< io::XStream >( static_cast< ::cppu::OWeakObject* >( m_pAntiImpl ),
                                                                uno::UNO_QUERY );

        OSL_ENSURE( xWriteStream.is(), "OWriteStream MUST implement XStream && XComponent interfaces!\n" );

        return xWriteStream;
    }

    throw lang::IllegalArgumentException(); // TODO

    // not reachable
    return uno::Reference< io::XStream >();
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
uno::Sequence< sal_Int8 > OWriteStream_Impl::GetCommonRootPass()
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() ) ;

    uno::Reference< beans::XPropertySet > xGlobalPropertySet( m_xPackage, uno::UNO_QUERY );
    if ( !xGlobalPropertySet.is() )
        throw uno::RuntimeException();

    ::rtl::OUString aString_EncryptionKey = ::rtl::OUString::createFromAscii( "EncryptionKey" );
    uno::Sequence< sal_Int8 > aGlobalKey;
    try {
        if ( !( xGlobalPropertySet->getPropertyValue( aString_EncryptionKey ) >>= aGlobalKey ) )
            throw io::IOException();
    }
    catch ( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "Can't read encryption related properties!\n" );
        throw io::IOException(); // TODO
    }

    return aGlobalKey;
}

void OWriteStream_Impl::InputStreamDisposed( OInputCompStream* pStream )
{
    ::osl::MutexGuard aGuard( m_rMutexRef->GetMutex() );
    m_aInputStreamsList.remove( pStream );
}

//===============================================
// OWriteStream implementation
//===============================================

//-----------------------------------------------
OWriteStream::OWriteStream( OWriteStream_Impl* pImpl, uno::Reference< io::XStream > xStream )
: m_pImpl( pImpl )
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
sal_Int32 SAL_CALL OWriteStream::readBytes( uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

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

    if ( !m_xInStream.is() )
        throw io::NotConnectedException();

    m_xInStream->closeInput();
    m_xInStream = uno::Reference< io::XInputStream >();

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

    if ( !m_xInStream.is() )
        return uno::Reference< io::XInputStream >();

    return uno::Reference< io::XInputStream >( static_cast< io::XInputStream* >( this ), uno::UNO_QUERY );
}

//-----------------------------------------------
uno::Reference< io::XOutputStream > SAL_CALL OWriteStream::getOutputStream()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

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
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    m_xOutStream->writeBytes( aData );
    m_pImpl->m_bIsModified = sal_True;
    m_pImpl->m_pParent->SetModifiedInternally( sal_True );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::flush()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    m_xOutStream->flush();
}

//-----------------------------------------------
void OWriteStream::CloseOutput_Impl()
{
    // all the checks must be done in calling method

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

    m_pImpl->m_bCommited = sal_True;
}

//-----------------------------------------------
void SAL_CALL OWriteStream::closeOutput()
        throw ( io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( !m_xOutStream.is() )
        throw io::NotConnectedException();

    CloseOutput_Impl();

    if ( !m_xInStream.is() )
        dispose();
}

//-----------------------------------------------
void SAL_CALL OWriteStream::seek( sal_Int64 location )
        throw ( lang::IllegalArgumentException,
                io::IOException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

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
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

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

    m_pImpl->m_bIsModified = sal_True;
    m_pImpl->m_pParent->SetModifiedInternally( sal_True );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::dispose()
        throw ( uno::RuntimeException )
{
    // should be an internal method since it can be called only from parent storage

    ::osl::MutexGuard( m_pData->m_rSharedMutexRef->GetMutex() );

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
    m_pImpl = NULL;
}

//-----------------------------------------------
void SAL_CALL OWriteStream::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.addInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    m_pData->m_aListenersContainer.removeInterface( xListener );
}

//-----------------------------------------------
void SAL_CALL OWriteStream::setEncryptionKey( const uno::Sequence< sal_Int8 >& aKey )
    throw( uno::RuntimeException )
{
    ::osl::MutexGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    OSL_ENSURE( m_pImpl->m_xPackageStream.is(), "No package stream is set!\n" );

    uno::Reference< beans::XPropertySet > xPropertySet( m_pImpl->m_xPackageStream, uno::UNO_QUERY );
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

    m_pImpl->m_bHasCachedPassword = sal_True;
    m_pImpl->m_aKey = aKey;
    m_pImpl->m_bIsModified = sal_True;
    m_pImpl->m_pParent->SetModifiedInternally( sal_True );
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
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName.equalsAscii( "MediaType" )
      || aPropertyName.equalsAscii( "Compressed" ) )
    {
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                m_pImpl->m_aProps[nInd].Value = aValue;
        }
    }
    else if ( aPropertyName.equalsAscii( "Encrypted" ) )
    {
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
            {
                sal_Bool bEncr = sal_False;
                if ( aValue >>= bEncr )
                {
                    if ( bEncr )
                    {
                        // if the stream is already encrypted, it must have a cached pass
                        // because it was opened to change this property
                        if ( !m_pImpl->m_bHasCachedPassword )
                        {
                            uno::Sequence< sal_Int8 > aGlobalKey = m_pImpl->GetCommonRootPass();
                            if ( !aGlobalKey.getLength() )
                                throw beans::PropertyVetoException(); // TODO

                            m_pImpl->m_bForceEncrypted = sal_True;
                            m_pImpl->m_bHasCachedPassword = sal_True;
                            m_pImpl->m_aKey = aGlobalKey;
                        }
                    }
                    else
                    {
                        m_pImpl->m_bForceEncrypted = sal_False;
                        m_pImpl->m_bHasCachedPassword = sal_False;
                        m_pImpl->m_aKey.realloc( 0 );
                    }
                }
                else
                    throw lang::IllegalArgumentException(); //TODO

                m_pImpl->m_aProps[nInd].Value = aValue;
            }
        }
    }

    else if ( aPropertyName.equalsAscii( "Size" ) )
        throw beans::PropertyVetoException(); // TODO
    else
        throw beans::UnknownPropertyException(); // TODO

    m_pImpl->m_bIsModified = sal_True;
    m_pImpl->m_pParent->SetModifiedInternally( sal_True );
}


//-----------------------------------------------
uno::Any SAL_CALL OWriteStream::getPropertyValue( const ::rtl::OUString& aPropertyName )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    if ( aPropertyName.equalsAscii( "MediaType" )
      || aPropertyName.equalsAscii( "Encrypted" )
      || aPropertyName.equalsAscii( "Compressed" ) )
    {
        for ( sal_Int32 nInd = 0; nInd < m_pImpl->m_aProps.getLength(); nInd++ )
        {
            if ( aPropertyName.equals( m_pImpl->m_aProps[nInd].Name ) )
                return m_pImpl->m_aProps[nInd].Value;
        }
    }
    else if ( aPropertyName.equalsAscii( "Size" ) )
    {
        if ( !m_xSeekable.is() )
            throw uno::RuntimeException();

        return uno::makeAny( (sal_Int32)m_xSeekable->getLength() );
    }

    throw beans::UnknownPropertyException(); // TODO

    // not reachable
    return uno::Any();
}


//-----------------------------------------------
void SAL_CALL OWriteStream::addPropertyChangeListener(
            const ::rtl::OUString& aPropertyName,
            const uno::Reference< beans::XPropertyChangeListener >& xListener )
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
            const ::rtl::OUString& aPropertyName,
            const uno::Reference< beans::XPropertyChangeListener >& aListener )
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
            const ::rtl::OUString& PropertyName,
            const uno::Reference< beans::XVetoableChangeListener >& aListener )
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
            const ::rtl::OUString& PropertyName,
            const uno::Reference< beans::XVetoableChangeListener >& aListener )
        throw ( beans::UnknownPropertyException,
                lang::WrappedTargetException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_pData->m_rSharedMutexRef->GetMutex() );

    if ( !m_pImpl )
        throw lang::DisposedException();

    //TODO:
}


