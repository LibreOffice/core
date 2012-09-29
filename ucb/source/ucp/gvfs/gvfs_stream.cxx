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

#include "gvfs_stream.hxx"
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>

#include <libgnomevfs/gnome-vfs-ops.h>

using namespace cppu;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace gvfs;

using ::rtl::OUString;

Stream::Stream( GnomeVFSHandle         *handle,
        const GnomeVFSFileInfo *aInfo ) :
    m_eof (sal_False),
    m_bInputStreamCalled( sal_False ),
    m_bOutputStreamCalled( sal_False )
{
    m_handle = handle;
    gnome_vfs_file_info_copy (&m_info, aInfo);
}

Stream::~Stream( void )
{
    if (m_handle) {
        gnome_vfs_close (m_handle);
        m_handle = NULL;
    }
}

Any Stream::queryInterface( const Type &type )
    throw( RuntimeException )
{
    Any aRet = ::cppu::queryInterface
        ( type,
          static_cast< XStream * >( this ),
          static_cast< XInputStream * >( this ),
          static_cast< XOutputStream * >( this ),
          static_cast< XSeekable * >( this ),
          static_cast< XTruncate * >( this ) );

    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( type );
}

// -------------------------------------------------------------------
//                            XStream
// -------------------------------------------------------------------

com::sun::star::uno::Reference< com::sun::star::io::XInputStream > SAL_CALL
Stream::getInputStream(  )
    throw( com::sun::star::uno::RuntimeException )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bInputStreamCalled = true;
    }
    return Reference< XInputStream >( this );
}

com::sun::star::uno::Reference< com::sun::star::io::XOutputStream > SAL_CALL
Stream::getOutputStream(  )
    throw( com::sun::star::uno::RuntimeException )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bOutputStreamCalled = true;
    }
    return Reference< XOutputStream >( this );
}

// -------------------------------------------------------------------
//                            XInputStream
// -------------------------------------------------------------------

sal_Int32 SAL_CALL Stream::readBytes(
    Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( NotConnectedException,
               BufferSizeExceededException,
               IOException,
               RuntimeException )
{
    GnomeVFSResult   result;
    GnomeVFSFileSize nBytesRead = 0;

    if( ! m_handle )
        throw IOException();

    if( m_eof ) {
        aData.realloc( 0 );
        return 0;
    }

    try {
        aData.realloc( nBytesToRead );
    } catch ( const Exception &e ) {
        throw BufferSizeExceededException();
    }

    do {
        result = gnome_vfs_read( m_handle, aData.getArray(),
                     nBytesToRead, &nBytesRead );
    } while( result == GNOME_VFS_ERROR_INTERRUPTED );

    if (result != GNOME_VFS_OK &&
        result != GNOME_VFS_ERROR_EOF)
        throwOnError( result );

    if (result == GNOME_VFS_ERROR_EOF)
        m_eof = sal_True;

    aData.realloc( sal::static_int_cast<sal_uInt32>(nBytesRead) );

    return sal::static_int_cast<sal_Int32>(nBytesRead);
}

sal_Int32 SAL_CALL Stream::readSomeBytes(
    Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
            throw( NotConnectedException,
               BufferSizeExceededException,
               IOException,
               RuntimeException )
{
    // Again - having 2 methods here just sucks; cf. filinpstr.cxx
    // This can never be an effective non-blocking API - so why bother ?
    return readBytes( aData, nMaxBytesToRead );
}

void SAL_CALL Stream::skipBytes( sal_Int32 nBytesToSkip )
        throw( NotConnectedException,
               BufferSizeExceededException,
               IOException,
               RuntimeException )
{
    GnomeVFSResult result;

    if( ! m_handle )
        throw IOException();

    result = gnome_vfs_seek( m_handle, GNOME_VFS_SEEK_CURRENT, nBytesToSkip );

    if ( result == GNOME_VFS_ERROR_BAD_PARAMETERS ||
         result == GNOME_VFS_ERROR_NOT_SUPPORTED )
        g_warning ("FIXME: just read them in ...");

    throwOnError( result );
}

sal_Int32 SAL_CALL Stream::available(  )
        throw( NotConnectedException,
               IOException,
               RuntimeException )
{
    return 0; // cf. filinpstr.cxx
}

void SAL_CALL Stream::closeInput( void )
        throw( NotConnectedException,
                   IOException,
                   RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bInputStreamCalled = false;

    if( ! m_bOutputStreamCalled )
        closeStream();
}

// -------------------------------------------------------------------
//                            XSeekable
// -------------------------------------------------------------------

void SAL_CALL Stream::seek( sal_Int64 location )
        throw( ::com::sun::star::lang::IllegalArgumentException,
               IOException,
               RuntimeException )
{
    GnomeVFSResult result;

    if( ! m_handle )
        throw IOException();

    if ( location < 0 )
        throw ::com::sun::star::lang::IllegalArgumentException();

    m_eof = sal_False;
    result = gnome_vfs_seek( m_handle, GNOME_VFS_SEEK_START, location );

    if (result == GNOME_VFS_ERROR_EOF)
        throw ::com::sun::star::lang::IllegalArgumentException();

    throwOnError( result );
}

sal_Int64 SAL_CALL Stream::getPosition()
        throw( IOException,
               RuntimeException )
{
    GnomeVFSFileSize nBytesIn = 0;

    if( ! m_handle )
        throw IOException();

    throwOnError( gnome_vfs_tell( m_handle, &nBytesIn ) );

    return nBytesIn;
}

sal_Int64 SAL_CALL Stream::getLength()
    throw( IOException, RuntimeException )
{
    // FIXME: so this sucks; it may be stale but ...
    if (m_info.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE)
        return m_info.size;
    else {
        g_warning ("FIXME: No valid length");
        return 0;
    }
}

// -------------------------------------------------------------------
//                            XTruncate
// -------------------------------------------------------------------

void SAL_CALL Stream::truncate( void )
    throw( com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    if( ! m_handle )
        throw IOException();

    throwOnError( gnome_vfs_truncate_handle( m_handle, 0 ) );
}

// -------------------------------------------------------------------
//                            XOutputStream
// -------------------------------------------------------------------

void SAL_CALL Stream::writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& aData )
    throw( com::sun::star::io::NotConnectedException,
           com::sun::star::io::BufferSizeExceededException,
           com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException)
{
    GnomeVFSResult   result = GNOME_VFS_OK;
    GnomeVFSFileSize toWrite = aData.getLength();
    const sal_Int8 *p = aData.getConstArray();

    if( ! m_handle )
        throw IOException();

    while( toWrite > 0) {
        GnomeVFSFileSize bytesWritten = 0;

        result = gnome_vfs_write( m_handle, p, toWrite, &bytesWritten );
        if( result == GNOME_VFS_ERROR_INTERRUPTED )
            continue;
        throwOnError( result );
        g_assert( bytesWritten <= toWrite );
        toWrite -= bytesWritten;
        p += bytesWritten;
    }
}

void SAL_CALL Stream::flush( void )
    throw( NotConnectedException, BufferSizeExceededException,
           IOException, RuntimeException )
{
}

void SAL_CALL Stream::closeOutput( void )
    throw( com::sun::star::io::NotConnectedException,
           com::sun::star::io::IOException,
           com::sun::star::uno::RuntimeException )
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bOutputStreamCalled = false;

    if( ! m_bInputStreamCalled )
        closeStream();
}

// -------------------------------------------------------------------
//                            Misc.
// -------------------------------------------------------------------

void Stream::closeStream( void )
    throw( ::com::sun::star::io::NotConnectedException,
           ::com::sun::star::io::IOException,
           ::com::sun::star::uno::RuntimeException )
{
    if (m_handle) {
        gnome_vfs_close (m_handle);
        m_handle = NULL;
    } else
        throw IOException();
}

void Stream::throwOnError( GnomeVFSResult result )
    throw( NotConnectedException,
           BufferSizeExceededException,
           IOException,
           RuntimeException )
{
    if( result != GNOME_VFS_OK ) {
        ::rtl::OUString aMsg = ::rtl::OUString::createFromAscii
              ( gnome_vfs_result_to_string( result ) );

        g_warning( "Input Stream exceptional result '%s' (%d)",
               gnome_vfs_result_to_string( result ), result );

        throw IOException( aMsg, static_cast< cppu::OWeakObject * >( this ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
