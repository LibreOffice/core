/*************************************************************************
 *
 *  $RCSfile: xtempfile.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:24:27 $
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

#ifndef _XTEMPFILE_HXX
#include <XTempFile.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star;

using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertyChangeListener;
using com::sun::star::beans::XVetoableChangeListener;
using com::sun::star::beans::UnknownPropertyException;
using com::sun::star::beans::PropertyVetoException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::registry::XRegistryKey;
using com::sun::star::registry::InvalidRegistryException;
using com::sun::star::uno::Any;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Type;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::XWeak;
using com::sun::star::uno::Reference;
using com::sun::star::io::NotConnectedException;
using com::sun::star::io::BufferSizeExceededException;
using com::sun::star::io::IOException;
using com::sun::star::io::XInputStream;
using com::sun::star::io::XOutputStream;
using com::sun::star::io::XSeekable;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::lang::XTypeProvider;
using cppu::OWeakObject;
using rtl::OUString;
using osl::FileBase;
using osl::MutexGuard;
using utl::TempFile;
using namespace utl;

// copy define from desktop\source\app\appinit.cxx

#define DESKTOP_TEMPNAMEBASE_DIR    "/temp/soffice.tmp"

XTempFile::XTempFile ()
: mpStream( NULL )
, mbRemoveFile( sal_True )
, mbInClosed( sal_False )
, mbOutClosed( sal_False )
, mnCachedPos( 0 )
, mbHasCachedPos( sal_False )
{
    mpTempFile = new TempFile;
    mpTempFile->EnableKillingFile ( sal_True );
}

XTempFile::~XTempFile ()
{
    if ( mpTempFile )
        delete mpTempFile;
}

uno::Sequence< beans::Property > XTempFile::GetProps()
{
    static uno::Sequence< beans::Property >* pProps = NULL;
    if ( pProps == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pProps == NULL )
        {
            static uno::Sequence< beans::Property > aProps( 3 );

            aProps[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RemoveFile" ) );
            aProps[0].Type = getCppuType( static_cast< sal_Bool* >( NULL ) );
            aProps[0].Attributes = beans::PropertyAttribute::TRANSIENT;
            aProps[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceName" ) );
            aProps[1].Type = getCppuType( static_cast< ::rtl::OUString* >( NULL ) );
            aProps[1].Attributes = beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::READONLY;
            aProps[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Uri" ) );
            aProps[2].Type = getCppuType( static_cast< ::rtl::OUString* >( NULL ) );
            aProps[2].Attributes = beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::READONLY;

            pProps = &aProps;
        }
    }

    return *pProps;
}

// XInterface
Any SAL_CALL XTempFile::queryInterface( const Type& rType )
    throw (RuntimeException)
{
    return ::cppu::queryInterface ( rType                                       ,
                                // OWeakObject interfaces
                                reinterpret_cast< XInterface*> ( this ),
                                static_cast< XWeak*         > ( this ),
                                // my own interfaces
                                static_cast< XTypeProvider* > ( this ),
                                static_cast< XInputStream*  > ( this ),
                                static_cast< XOutputStream* > ( this ),
                                static_cast< XStream*       > ( this ),
                                static_cast< XTruncate*     > ( this ),
                                static_cast< XPropertySet*  > ( this ),
                                static_cast< XPropertySetInfo*  > ( this ),
                                static_cast< XServiceInfo*  > ( this ),
                                static_cast< XSeekable*     > ( this ) );
}
void SAL_CALL XTempFile::acquire(  )
    throw ()
{
    OWeakObject::acquire();
}
void SAL_CALL XTempFile::release(  )
        throw ()
{
    OWeakObject::release();
}

// XTypeProvider
Sequence< Type > SAL_CALL XTempFile::getTypes()
        throw( RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL;
    if ( pTypeCollection == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pTypeCollection == NULL )
        {
            static ::cppu::OTypeCollection aTypeCollection
                                    ( ::getCppuType( ( const Reference< XTypeProvider >* )NULL )
                                    , ::getCppuType( ( const Reference< XWeak >* )NULL )
                                    , ::getCppuType( ( const Reference< XInputStream >* )NULL )
                                    , ::getCppuType( ( const Reference< XOutputStream >* )NULL )
                                    , ::getCppuType( ( const Reference< XStream >* )NULL )
                                    , ::getCppuType( ( const Reference< XTruncate >* )NULL )
                                    , ::getCppuType( ( const Reference< XPropertySet >* )NULL )
                                    , ::getCppuType( ( const Reference< XPropertySetInfo >* )NULL )
                                    , ::getCppuType( ( const Reference< XServiceInfo >* )NULL )
                                    , ::getCppuType( ( const Reference< XSeekable >* )NULL ) );
            pTypeCollection = &aTypeCollection;
        }
    }

    return pTypeCollection->getTypes() ;
}

Sequence< sal_Int8 > SAL_CALL XTempFile::getImplementationId()
        throw( RuntimeException )
{
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() ) ;

        if ( pID == NULL )
        {
            static ::cppu::OImplementationId aID( sal_False ) ;
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}

// XInputStream

sal_Int32 SAL_CALL XTempFile::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    checkConnected();
    if (nBytesToRead < 0)
        throw BufferSizeExceededException( OUString(), static_cast<XWeak*>(this));

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = mpStream->Read(static_cast < void* > ( aData.getArray() ), nBytesToRead);
    checkError();

    if (nRead < static_cast < sal_uInt32 > ( nBytesToRead ) )
        aData.realloc( nRead );

    if ( nBytesToRead > nRead )
    {
        // usually that means that the stream was read till the end
        // TODO/LATER: it is better to get rid of this optimization by avoiding using of multiple temporary files ( there should be only one temporary file? )
        mnCachedPos = mpStream->Tell();
        mbHasCachedPos = sal_True;

        mpStream = NULL;
        if ( mpTempFile )
            mpTempFile->CloseStream();
    }

    return nRead;
}
sal_Int32 SAL_CALL XTempFile::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    checkConnected();
    checkError();

    if (nMaxBytesToRead < 0)
        throw BufferSizeExceededException(OUString(), static_cast < XWeak* >( this ) );

    if (mpStream->IsEof())
    {
        aData.realloc(0);
        return 0;
    }
    else
        return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL XTempFile::skipBytes( sal_Int32 nBytesToSkip )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    checkConnected();
    checkError();
    mpStream->SeekRel(nBytesToSkip);
    checkError();
}
sal_Int32 SAL_CALL XTempFile::available(  )
    throw (NotConnectedException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();

    mpStream->Seek(STREAM_SEEK_TO_END);
    checkError();

    sal_Int32 nAvailable = (sal_Int32)mpStream->Tell() - nPos;
    mpStream->Seek(nPos);
    checkError();

    return nAvailable;
}
void SAL_CALL XTempFile::closeInput(  )
    throw (NotConnectedException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbInClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    mbInClosed = sal_True;

    if ( mbOutClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = NULL;

        if ( mpTempFile )
        {
            delete mpTempFile;
            mpTempFile = NULL;
        }
    }
}

// XSeekable

void SAL_CALL XTempFile::seek( sal_Int64 nLocation )
    throw (IllegalArgumentException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    checkConnected();
    if ( nLocation < 0 || nLocation > getLength() )
        throw IllegalArgumentException();

    mpStream->Seek((sal_uInt32) nLocation);
    checkError();
}
sal_Int64 SAL_CALL XTempFile::getPosition(  )
    throw (IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}
sal_Int64 SAL_CALL XTempFile::getLength(  )
    throw (IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nCurrentPos = mpStream->Tell();
    checkError();

    mpStream->Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nEndPos = mpStream->Tell();
    mpStream->Seek(nCurrentPos);

    checkError();

    return (sal_Int64)nEndPos;
}

// XOutputStream

void SAL_CALL XTempFile::writeBytes( const Sequence< sal_Int8 >& aData )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    checkConnected();
    sal_uInt32 nWritten = mpStream->Write(aData.getConstArray(),aData.getLength());
    checkError();
    if  ( nWritten != (sal_uInt32)aData.getLength())
        throw BufferSizeExceededException( OUString(),static_cast < XWeak* > ( this ) );
}
void SAL_CALL XTempFile::flush(  )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    checkConnected();
    mpStream->Flush();
    checkError();
}
void SAL_CALL XTempFile::closeOutput(  )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    if ( mbOutClosed )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );

    mbOutClosed = sal_True;

    // TODO/LATER: it is better to get rid of this optimization by avoiding using of multiple temporary files ( there should be only one temporary file? )
    if ( mpStream )
    {
        mnCachedPos = mpStream->Tell();
        mbHasCachedPos = sal_True;

        mpStream = NULL;
        if ( mpTempFile )
            mpTempFile->CloseStream();
    }

    if ( mbInClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = NULL;

        if ( mpTempFile )
        {
            delete mpTempFile;
            mpTempFile = NULL;
        }
    }
}


void XTempFile::checkError () const
{
    if (!mpStream || mpStream->SvStream::GetError () != ERRCODE_NONE )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );
}
void XTempFile::checkConnected ()
{
    if (!mpStream && mpTempFile)
    {
        mpStream = mpTempFile->GetStream( STREAM_STD_READWRITE );
        if ( mpStream && mbHasCachedPos )
        {
            mpStream->Seek( mnCachedPos );
            if ( mpStream->SvStream::GetError () == ERRCODE_NONE )
            {
                mbHasCachedPos = sal_False;
                mnCachedPos = 0;
            }
            else
            {
                mpStream = NULL;
                mpTempFile->CloseStream();
            }
        }
    }

    if (!mpStream)
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );
}

// XStream

Reference< XInputStream > SAL_CALL XTempFile::getInputStream()
    throw (RuntimeException)
{
    return Reference< XInputStream >( static_cast< OWeakObject* >( this ), ::com::sun::star::uno::UNO_QUERY );
}

Reference< XOutputStream > SAL_CALL XTempFile::getOutputStream()
    throw (RuntimeException)
{
    return Reference< XOutputStream >( static_cast< OWeakObject* >( this ), ::com::sun::star::uno::UNO_QUERY );
}

// XTruncate

void SAL_CALL XTempFile::truncate()
    throw (IOException, RuntimeException)
{
    MutexGuard aGuard( maMutex );
    checkConnected();
    // SetStreamSize() call does not change the position
    mpStream->Seek( 0 );
    mpStream->SetStreamSize( 0 );
    checkError();
}

// XPropertySetInfo
uno::Sequence< beans::Property > SAL_CALL XTempFile::getProperties()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    return GetProps();
}

beans::Property SAL_CALL XTempFile::getPropertyByName( const ::rtl::OUString& aName )
    throw (beans::UnknownPropertyException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    uno::Sequence< beans::Property > aProps = XTempFile::GetProps();
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
        if ( aName.equals( aProps[nInd].Name ) )
            return aProps[nInd];

    throw beans::UnknownPropertyException();
}

::sal_Bool SAL_CALL XTempFile::hasPropertyByName( const ::rtl::OUString& aName )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    uno::Sequence< beans::Property > aProps = XTempFile::GetProps();
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
        if ( aName.equals( aProps[nInd].Name ) )
            return sal_True;

    return sal_False;
}


// XPropertySet

Reference< XPropertySetInfo > SAL_CALL XTempFile::getPropertySetInfo(  )
    throw (RuntimeException)
{
    MutexGuard aGuard( maMutex );

    return Reference< XPropertySetInfo > ( static_cast< XPropertySetInfo* >( this ) );
}

void SAL_CALL XTempFile::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
    throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        // the stream is already disconnected
        throw RuntimeException();
    }

    if ( aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "RemoveFile" ) ) )
    {
        if ( !( aValue >>= mbRemoveFile ) )
            throw IllegalArgumentException();

        mpTempFile->EnableKillingFile( mbRemoveFile );
    }
    // All other properties are read-only
    else if ( aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "ResourceName" ) ) ||
         aPropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Uri" ) ) )
        throw IllegalArgumentException();
    else
        throw UnknownPropertyException();
}
Any SAL_CALL XTempFile::getPropertyValue( const OUString& PropertyName )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    MutexGuard aGuard( maMutex );

    if ( !mpTempFile )
    {
        // the stream is already disconnected
        throw RuntimeException();
    }

    Any aRet;

    if ( PropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "RemoveFile" ) ) )
        aRet <<= mbRemoveFile;
    else if ( PropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "ResourceName" ) ) )
        aRet <<= OUString ( mpTempFile->GetFileName() );
    else if ( PropertyName.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Uri" ) ) )
        aRet <<= OUString ( mpTempFile->GetURL() );
    else
        throw UnknownPropertyException();
    return aRet;
}
void SAL_CALL XTempFile::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_ASSERT ( sal_False, "Listeners not implemented" );
}
void SAL_CALL XTempFile::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_ASSERT ( sal_False, "Listeners not implemented" );
}
void SAL_CALL XTempFile::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_ASSERT ( sal_False, "Listeners not implemented" );
}
void SAL_CALL XTempFile::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
    throw (UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    DBG_ASSERT ( sal_False, "Listeners not implemented" );
}

// XServiceInfo
::rtl::OUString SAL_CALL XTempFile::getImplementationName()
        throw (::com::sun::star::uno::RuntimeException)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL XTempFile::supportsService(rtl::OUString const & rServiceName)
        throw (com::sun::star::uno::RuntimeException)
{
    return rServiceName == getSupportedServiceNames_Static()[0];
}

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL XTempFile::getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}



OUString XTempFile::getImplementationName_Static ()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.comp.TempFile" ) );
}
Sequence < OUString > XTempFile::getSupportedServiceNames_Static()
{
    Sequence < OUString > aNames ( 1 );
    aNames[0] = OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
    return aNames;
}
Reference < XInterface >SAL_CALL XTempFile_createInstance(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference< XInterface >( *new XTempFile );
}

Reference < XSingleServiceFactory > XTempFile::createServiceFactory_Static( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory ( rServiceFactory, getImplementationName_Static(),
                                       XTempFile_createInstance,
                                       getSupportedServiceNames_Static());
}

static sal_Bool writeInfo( void * pRegistryKey,
                           const OUString & rImplementationName,
                              Sequence< OUString > const & rServiceNames )
{
    OUString aKeyName( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
    aKeyName += rImplementationName;
    aKeyName += OUString( RTL_CONSTASCII_USTRINGPARAM ( "/UNO/SERVICES" ) );

    Reference< XRegistryKey > xKey;
    try
    {
        xKey = static_cast< XRegistryKey * >(
                                    pRegistryKey )->createKey( aKeyName );
    }
    catch ( InvalidRegistryException const & )
    {
    }

    if ( !xKey.is() )
        return sal_False;

    sal_Bool bSuccess = sal_True;

    for ( sal_Int32 n = 0; n < rServiceNames.getLength(); ++n )
    {
        try
        {
            xKey->createKey( rServiceNames[ n ] );
        }
        catch ( InvalidRegistryException const & )
        {
            bSuccess = sal_False;
            break;
        }
    }
    return bSuccess;
}
// C functions to implement this as a component

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
                const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 * for each supported service.
 * @param pServiceManager generic uno interface providing a service manager
 * @param pRegistryKey generic uno interface providing registry key to write
 */
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    return pRegistryKey &&
    writeInfo (pRegistryKey,
               XTempFile::getImplementationName_Static(),
               XTempFile::getSupportedServiceNames_Static() );
}


/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    if (XTempFile::getImplementationName_Static().compareToAscii( pImplName ) == 0)
        xFactory = XTempFile::createServiceFactory_Static ( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}
