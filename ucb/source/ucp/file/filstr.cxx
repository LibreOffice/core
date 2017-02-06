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

#include "sal/config.h"

#include "com/sun/star/io/BufferSizeExceededException.hpp"
#include "com/sun/star/io/IOException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/diagnose.h"
#include "filstr.hxx"
#include "filtask.hxx"
#include "prov.hxx"
#include <memory>

using namespace fileaccess;
using namespace com::sun::star;
using namespace css::ucb;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

/******************************************************************************/
/*                                                                            */
/*               XStream_impl implementation                                  */
/*                                                                            */
/******************************************************************************/

XStream_impl::XStream_impl( const OUString& aUncPath, bool bLock )
    : m_bInputStreamCalled( false ),
      m_bOutputStreamCalled( false ),
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


uno::Reference< io::XInputStream > SAL_CALL
XStream_impl::getInputStream(  )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bInputStreamCalled = true;
    }
    return uno::Reference< io::XInputStream >( this );
}


uno::Reference< io::XOutputStream > SAL_CALL
XStream_impl::getOutputStream(  )
{
    {
        osl::MutexGuard aGuard( m_aMutex );
        m_bOutputStreamCalled = true;
    }
    return uno::Reference< io::XOutputStream >( this );
}


void SAL_CALL XStream_impl::truncate()
{
    if (osl::FileBase::E_None != m_aFile.setSize(0))
        throw io::IOException( THROW_WHERE );

    if (osl::FileBase::E_None != m_aFile.setPos(osl_Pos_Absolut,sal_uInt64(0)))
        throw io::IOException( THROW_WHERE );
}


// XStream_impl private non interface methods


sal_Int32 SAL_CALL
XStream_impl::readBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nBytesToRead )
{
    if( ! m_nIsOpen )
        throw io::IOException( THROW_WHERE );

    std::unique_ptr<sal_Int8[]> buffer;
    try
    {
        buffer.reset(new sal_Int8[nBytesToRead]);
    }
    catch (const std::bad_alloc&)
    {
        if( m_nIsOpen ) m_aFile.close();
        throw io::BufferSizeExceededException( THROW_WHERE );
    }

    sal_uInt64 nrc(0);
    if(m_aFile.read( buffer.get(),sal_uInt64(nBytesToRead),nrc )
       != osl::FileBase::E_None)
    {
        throw io::IOException( THROW_WHERE );
    }
    aData = uno::Sequence< sal_Int8 > ( buffer.get(), (sal_uInt32)nrc );
    return ( sal_Int32 ) nrc;
}


sal_Int32 SAL_CALL
XStream_impl::readSomeBytes(
    uno::Sequence< sal_Int8 >& aData,
    sal_Int32 nMaxBytesToRead )
{
    return readBytes( aData,nMaxBytesToRead );
}


void SAL_CALL
XStream_impl::skipBytes( sal_Int32 nBytesToSkip )
{
    m_aFile.setPos( osl_Pos_Current, sal_uInt64( nBytesToSkip ) );
}


sal_Int32 SAL_CALL
XStream_impl::available()
{
    return 0;
}


void SAL_CALL
XStream_impl::writeBytes( const uno::Sequence< sal_Int8 >& aData )
{
    sal_uInt32 length = aData.getLength();
    if(length)
    {
        sal_uInt64 nWrittenBytes(0);
        const sal_Int8* p = aData.getConstArray();
        if(osl::FileBase::E_None != m_aFile.write((static_cast<void const *>(p)),sal_uInt64(length),nWrittenBytes) ||
           nWrittenBytes != length )
            throw io::IOException( THROW_WHERE );
    }
}


void SAL_CALL
XStream_impl::closeStream()
{
    if( m_nIsOpen )
    {
        osl::FileBase::RC err = m_aFile.close();

        if( err != osl::FileBase::E_None ) {
            io::IOException ex;
            ex.Message = "could not close file";
            throw ex;
        }

        m_nIsOpen = false;
    }
}

void SAL_CALL
XStream_impl::closeInput()
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bInputStreamCalled = false;

    if( ! m_bOutputStreamCalled )
        closeStream();
}


void SAL_CALL
XStream_impl::closeOutput()
{
    osl::MutexGuard aGuard( m_aMutex );
    m_bOutputStreamCalled = false;

    if( ! m_bInputStreamCalled )
        closeStream();
}


void SAL_CALL
XStream_impl::seek( sal_Int64 location )
{
    if( location < 0 )
        throw lang::IllegalArgumentException( THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );
    if( osl::FileBase::E_None != m_aFile.setPos( osl_Pos_Absolut, sal_uInt64( location ) ) )
        throw io::IOException( THROW_WHERE );
}


sal_Int64 SAL_CALL
XStream_impl::getPosition()
{
    sal_uInt64 uPos;
    if( osl::FileBase::E_None != m_aFile.getPos( uPos ) )
        throw io::IOException( THROW_WHERE );
    return sal_Int64( uPos );
}

sal_Int64 SAL_CALL
XStream_impl::getLength()
{
        sal_uInt64 uEndPos;
        if ( m_aFile.getSize(uEndPos) != osl::FileBase::E_None )
                throw io::IOException( THROW_WHERE );
        else
                return sal_Int64( uEndPos );
}

void SAL_CALL
XStream_impl::flush()
{}

void XStream_impl::waitForCompletion()
{
    // At least on UNIX, to reliably learn about any errors encountered by
    // asynchronous NFS write operations, without closing the file directly
    // afterwards, there appears to be no cheaper way than to call fsync:
    if (m_nIsOpen && m_aFile.sync() != osl::FileBase::E_None) {
        throw io::IOException(
            "could not synchronize file to disc",
            static_cast< OWeakObject * >(this));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
