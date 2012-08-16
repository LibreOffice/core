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

#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"
#include "filstr.hxx"
#include "shell.hxx"
#include "prov.hxx"


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



/******************************************************************************/
/*                                                                            */
/*               XStream_impl implementation                                  */
/*                                                                            */
/******************************************************************************/


uno::Any SAL_CALL
XStream_impl::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException)
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          (static_cast< lang::XTypeProvider* >(this)),
                                          (static_cast< io::XStream* >(this)),
                                          (static_cast< io::XInputStream* >(this)),
                                          (static_cast< io::XOutputStream* >(this)),
                                          (static_cast< io::XSeekable* >(this)),
                                          (static_cast< io::XTruncate* >(this)),
                                          (static_cast< io::XAsyncOutputMonitor* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


void SAL_CALL
XStream_impl::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
XStream_impl::release(
    void )
    throw()
{
    OWeakObject::release();
}


//////////////////////////////////////////////////////////////////////////////////////////
//  XTypeProvider
//////////////////////////////////////////////////////////////////////////////////////////


XTYPEPROVIDER_IMPL_7( XStream_impl,
                      lang::XTypeProvider,
                      io::XStream,
                      io::XSeekable,
                      io::XInputStream,
                      io::XOutputStream,
                      io::XTruncate,
                      io::XAsyncOutputMonitor )



XStream_impl::XStream_impl( shell* pMyShell,const rtl::OUString& aUncPath, sal_Bool bLock )
    : m_bInputStreamCalled( false ),
      m_bOutputStreamCalled( false ),
      m_pMyShell( pMyShell ),
      m_xProvider( m_pMyShell->m_pProvider ),
      m_aFile( aUncPath ),
      m_nErrorCode( TASKHANDLER_NO_ERROR ),
      m_nMinorErrorCode( TASKHANDLER_NO_ERROR )
{
    sal_uInt32 nFlags = ( osl_File_OpenFlag_Read | osl_File_OpenFlag_Write );
    if ( !bLock )
        nFlags |= osl_File_OpenFlag_NoLock;

    osl::FileBase::RC err = m_aFile.open( nFlags );
    if(  err != osl::FileBase::E_None )
    {
        m_nIsOpen = false;
        m_aFile.close();

        m_nErrorCode = TASKHANDLING_OPEN_FOR_STREAM;
        m_nMinorErrorCode = err;
    }
    else
        m_nIsOpen = true;
}


XStream_impl::~XStream_impl()
{
    try
    {
        closeStream();
    }
    catch (const io::IOException&)
    {
        OSL_FAIL("unexpected situation");
    }
    catch (const uno::RuntimeException&)
    {
        OSL_FAIL("unexpected situation");
    }
}


sal_Int32 SAL_CALL XStream_impl::CtorSuccess()
{
    return m_nErrorCode;
}



sal_Int32 SAL_CALL XStream_impl::getMinorError()
{
    return m_nMinorErrorCode;
}



uno::Reference< io::XInputStream > SAL_CALL
XStream_impl::getInputStream(  )
    throw( uno::RuntimeException)
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bInputStreamCalled = true;
    }
    return uno::Reference< io::XInputStream >( this );
}


uno::Reference< io::XOutputStream > SAL_CALL
XStream_impl::getOutputStream(  )
    throw( uno::RuntimeException )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bOutputStreamCalled = true;
    }
    return uno::Reference< io::XOutputStream >( this );
}


void SAL_CALL XStream_impl::truncate(void)
    throw( io::IOException, uno::RuntimeException )
{
    if (osl::FileBase::E_None != m_aFile.setSize(0))
        throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

    if (osl::FileBase::E_None != m_aFile.setPos(osl_Pos_Absolut,sal_uInt64(0)))
        throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
}



//===========================================================================
// XStream_impl private non interface methods
//===========================================================================

sal_Int32 SAL_CALL
XStream_impl::readBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    if( ! m_nIsOpen )
        throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );

    sal_Int8 * buffer;
    try
    {
        buffer = new sal_Int8[nBytesToRead];
    }
    catch (const std::bad_alloc&)
    {
        if( m_nIsOpen ) m_aFile.close();
        throw io::BufferSizeExceededException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
    }

    sal_uInt64 nrc(0);
    if(m_aFile.read( (void* )buffer,sal_uInt64(nBytesToRead),nrc )
       != osl::FileBase::E_None)
    {
        delete[] buffer;
        throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
    }
    aData = uno::Sequence< sal_Int8 > ( buffer, (sal_uInt32)nrc );
    delete[] buffer;
    return ( sal_Int32 ) nrc;
}


sal_Int32 SAL_CALL
XStream_impl::readSomeBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nMaxBytesToRead )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL
XStream_impl::skipBytes(
    sal_Int32 nBytesToSkip )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XStream_impl::available(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException)
{
    return 0;
}


void SAL_CALL
XStream_impl::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException)
{
    sal_uInt32 length = aData.getLength();
    if(length)
    {
        sal_uInt64 nWrittenBytes(0);
        const sal_Int8* p = aData.getConstArray();
        if(osl::FileBase::E_None != m_aFile.write(((void*)(p)),sal_uInt64(length),nWrittenBytes) ||
           nWrittenBytes != length )
            throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
    }
}


void SAL_CALL
XStream_impl::closeStream(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    if( m_nIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();

        if( err != osl::FileBase::E_None ) {
            io::IOException ex;
            ex.Message = rtl::OUString( "could not close file");
            throw ex;
        }

        m_nIsOpen = false;
    }
}

void SAL_CALL
XStream_impl::closeInput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bInputStreamCalled = false;

    if( ! m_bOutputStreamCalled )
        closeStream();
}


void SAL_CALL
XStream_impl::closeOutput(
    void )
    throw( io::NotConnectedException,
           io::IOException,
           uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bOutputStreamCalled = false;

    if( ! m_bInputStreamCalled )
        closeStream();
}


void SAL_CALL
XStream_impl::seek(
    sal_Int64 location )
    throw( lang::IllegalArgumentException,
           io::IOException,
           uno::RuntimeException )
{
    if( location < 0 )
        throw lang::IllegalArgumentException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >(), 0 );
    if( osl::FileBase::E_None != m_aFile.setPos( osl_Pos_Absolut, sal_uInt64( location ) ) )
        throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
}


sal_Int64 SAL_CALL
XStream_impl::getPosition(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
    sal_uInt64 uPos;
    if( osl::FileBase::E_None != m_aFile.getPos( uPos ) )
        throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
    return sal_Int64( uPos );
}

sal_Int64 SAL_CALL
XStream_impl::getLength(
    void )
    throw( io::IOException,
           uno::RuntimeException )
{
        sal_uInt64 uEndPos;
        if ( m_aFile.getSize(uEndPos) != osl::FileBase::E_None )
                throw io::IOException( ::rtl::OUString(  OSL_LOG_PREFIX  ), uno::Reference< uno::XInterface >() );
        else
                return sal_Int64( uEndPos );
}

void SAL_CALL
XStream_impl::flush()
    throw( io::NotConnectedException,
           io::BufferSizeExceededException,
           io::IOException,
           uno::RuntimeException )
{}

void XStream_impl::waitForCompletion()
    throw (io::IOException, uno::RuntimeException)
{
    // At least on UNIX, to reliably learn about any errors encountered by
    // asynchronous NFS write operations, without closing the file directly
    // afterwards, there appears to be no cheaper way than to call fsync:
    if (m_nIsOpen && m_aFile.sync() != osl::FileBase::E_None) {
        throw io::IOException(
            rtl::OUString( "could not synchronize file to disc"),
            static_cast< OWeakObject * >(this));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
