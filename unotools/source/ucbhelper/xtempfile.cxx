/*************************************************************************
 *
 *  $RCSfile: xtempfile.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mtg $ $Date: 2001-08-01 12:45:12 $
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

#ifndef _UNOTOOLS_XTEMPFILE_HXX
#include <unotools/XTempFile.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

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
using cppu::OWeakObject;
using rtl::OUString;

XTempFile::XTempFile ()
{
    maTempFile.EnableKillingFile ( sal_True );
    mpStream = maTempFile.GetStream ( STREAM_STD_READWRITE );
}
XTempFile::~XTempFile ()
{
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
                                static_cast< XInputStream*  > ( this ),
                                static_cast< XOutputStream* > ( this ),
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
// XInputStream
sal_Int32 SAL_CALL XTempFile::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    checkConnected();
    if (nBytesToRead < 0)
        throw BufferSizeExceededException( OUString(), static_cast<XWeak*>(this));

    ::osl::MutexGuard aGuard( maMutex );

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = mpStream->Read(static_cast < void* > ( aData.getArray() ), nBytesToRead);
    checkError();

    if (nRead < static_cast < sal_uInt32 > ( nBytesToRead ) )
        aData.realloc( nRead );

    return nRead;
}
sal_Int32 SAL_CALL XTempFile::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
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
    ::osl::MutexGuard aGuard( maMutex );
    checkError();
    mpStream->SeekRel(nBytesToSkip);
    checkError();
}
sal_Int32 SAL_CALL XTempFile::available(  )
    throw (NotConnectedException, IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
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
}
// XSeekable
void SAL_CALL XTempFile::seek( sal_Int64 nLocation )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    mpStream->Seek((sal_uInt32) nLocation);
    checkError();
}
sal_Int64 SAL_CALL XTempFile::getPosition(  )
    throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
    checkConnected();

    sal_uInt32 nPos = mpStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}
sal_Int64 SAL_CALL XTempFile::getLength(  )
    throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
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
    ::osl::MutexGuard aGuard( maMutex );
    sal_uInt32 nWritten = mpStream->Write(aData.getConstArray(),aData.getLength());
    checkError();
    if  ( nWritten != (sal_uInt32)aData.getLength())
        throw BufferSizeExceededException( OUString(),static_cast < XWeak* > ( this ) );
}
void SAL_CALL XTempFile::flush(  )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );
    mpStream->Flush();
    checkError();
}
void SAL_CALL XTempFile::closeOutput(  )
    throw (NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
}
void XTempFile::checkError () const
{
    if (mpStream->SvStream::GetError () != ERRCODE_NONE )
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );
}
void XTempFile::checkConnected () const
{
    if (!mpStream)
        throw NotConnectedException ( OUString(), const_cast < XWeak * > ( static_cast < const XWeak* > (this ) ) );
}

OUString XTempFile::getImplementationName ()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.comp.TempFile" ) );
}
Sequence < OUString > XTempFile::getSupportedServiceNames()
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

Reference < XSingleServiceFactory > XTempFile::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory ( rServiceFactory, getImplementationName(),
                                       XTempFile_createInstance,
                                       getSupportedServiceNames());
}
sal_Bool SAL_CALL XTempFile::supportsService(rtl::OUString const & rServiceName)
        throw (com::sun::star::uno::RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
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

extern "C" void SAL_CALL component_getImplementationEnvironment(
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
extern "C" sal_Bool SAL_CALL component_writeInfo( void* pServiceManager, void* pRegistryKey )
{
    return pRegistryKey &&
    writeInfo (pRegistryKey,
               XTempFile::getImplementationName(),
               XTempFile::getSupportedServiceNames() );
}


/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    if (XTempFile::getImplementationName().compareToAscii( pImplName ) == 0)
        xFactory = XTempFile::createServiceFactory ( xSMgr );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}
