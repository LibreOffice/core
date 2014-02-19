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

#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>

namespace utl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

//==================================================================
//= OInputStreamWrapper
//==================================================================
DBG_NAME(OInputStreamWrapper)
//------------------------------------------------------------------
OInputStreamWrapper::OInputStreamWrapper( SvStream& _rStream )
                 :m_pSvStream(&_rStream)
                 ,m_bSvStreamOwner(false)
{
    DBG_CTOR(OInputStreamWrapper,NULL);

}

//------------------------------------------------------------------
OInputStreamWrapper::OInputStreamWrapper( SvStream* pStream, bool bOwner )
                 :m_pSvStream( pStream )
                 ,m_bSvStreamOwner( bOwner )
{
    DBG_CTOR(OInputStreamWrapper,NULL);

}

//------------------------------------------------------------------
OInputStreamWrapper::~OInputStreamWrapper()
{
    if( m_bSvStreamOwner )
        delete m_pSvStream;

    DBG_DTOR(OInputStreamWrapper,NULL);
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    checkConnected();

    if (nBytesToRead < 0)
        throw stario::BufferSizeExceededException(OUString(),static_cast<staruno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = m_pSvStream->Read((void*)aData.getArray(), nBytesToRead);
    checkError();

    // Wenn gelesene Zeichen < MaxLength, staruno::Sequence anpassen
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    checkError();

    if (nMaxBytesToRead < 0)
        throw stario::BufferSizeExceededException(OUString(),static_cast<staruno::XWeak*>(this));

    if (m_pSvStream->IsEof())
    {
        aData.realloc(0);
        return 0;
    }
    else
        return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------------------
void SAL_CALL OInputStreamWrapper::skipBytes(sal_Int32 nBytesToSkip) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkError();

    m_pSvStream->SeekRel(nBytesToSkip);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::available() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();

    m_pSvStream->Seek(STREAM_SEEK_TO_END);
    checkError();

    sal_Int32 nAvailable = (sal_Int32)m_pSvStream->Tell() - nPos;
    m_pSvStream->Seek(nPos);
    checkError();

    return nAvailable;
}

//------------------------------------------------------------------------------
void SAL_CALL OInputStreamWrapper::closeInput() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    if (m_bSvStreamOwner)
        delete m_pSvStream;

    m_pSvStream = NULL;
}

//------------------------------------------------------------------------------
void OInputStreamWrapper::checkConnected() const
{
    if (!m_pSvStream)
        throw stario::NotConnectedException(OUString(), const_cast<staruno::XWeak*>(static_cast<const staruno::XWeak*>(this)));
}

//------------------------------------------------------------------------------
void OInputStreamWrapper::checkError() const
{
    checkConnected();

    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
        // TODO: really evaluate the error
        throw stario::NotConnectedException(OUString(), const_cast<staruno::XWeak*>(static_cast<const staruno::XWeak*>(this)));
}

//==================================================================
//= OSeekableInputStreamWrapper
//==================================================================
//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream& _rStream)
{
    SetStream( &_rStream, false );
}

//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream* _pStream, bool _bOwner)
{
    SetStream( _pStream, _bOwner );
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableInputStreamWrapper::seek( sal_Int64 _nLocation ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    m_pSvStream->Seek((sal_uInt32)_nLocation);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OSeekableInputStreamWrapper::getPosition(  ) throw (IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OSeekableInputStreamWrapper::getLength(  ) throw (IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nCurrentPos = m_pSvStream->Tell();
    checkError();

    m_pSvStream->Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nEndPos = m_pSvStream->Tell();
    m_pSvStream->Seek(nCurrentPos);

    checkError();

    return (sal_Int64)nEndPos;
}

//==================================================================
//= OOutputStreamWrapper
//==================================================================

OOutputStreamWrapper::OOutputStreamWrapper(SvStream& _rStream):
    rStream(_rStream)
{}

OOutputStreamWrapper::~OOutputStreamWrapper() {}

void SAL_CALL OOutputStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    sal_uInt32 nWritten = rStream.Write(aData.getConstArray(),aData.getLength());
    ErrCode err = rStream.GetError();
    if  (   (ERRCODE_NONE != err)
        ||  (nWritten != (sal_uInt32)aData.getLength())
        )
    {
        throw stario::BufferSizeExceededException(OUString(),static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::flush() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    rStream.Flush();
    checkError();
}

//------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::closeOutput() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
}

//------------------------------------------------------------------------------
void OOutputStreamWrapper::checkError() const
{
    if (rStream.GetError() != ERRCODE_NONE)
        // TODO: really evaluate the error
        throw stario::NotConnectedException(OUString(), const_cast<staruno::XWeak*>(static_cast<const staruno::XWeak*>(this)));
}

//==================================================================
//= OSeekableOutputStreamWrapper
//==================================================================
//------------------------------------------------------------------------------
OSeekableOutputStreamWrapper::OSeekableOutputStreamWrapper(SvStream& _rStream)
    :OOutputStreamWrapper(_rStream)
{
}

OSeekableOutputStreamWrapper::~OSeekableOutputStreamWrapper() {}

//------------------------------------------------------------------------------
Any SAL_CALL OSeekableOutputStreamWrapper::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = OOutputStreamWrapper::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OSeekableOutputStreamWrapper_Base::queryInterface(_rType);
    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableOutputStreamWrapper::acquire(  ) throw ()
{
    OOutputStreamWrapper::acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableOutputStreamWrapper::release(  ) throw ()
{
    OOutputStreamWrapper::release();
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableOutputStreamWrapper::seek( sal_Int64 _nLocation ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    rStream.Seek((sal_uInt32)_nLocation);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OSeekableOutputStreamWrapper::getPosition(  ) throw (IOException, RuntimeException)
{
    sal_uInt32 nPos = rStream.Tell();
    checkError();
    return (sal_Int64)nPos;
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OSeekableOutputStreamWrapper::getLength(  ) throw (IOException, RuntimeException)
{
    sal_uInt32 nCurrentPos = rStream.Tell();
    checkError();

    rStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nEndPos = rStream.Tell();
    rStream.Seek(nCurrentPos);

    checkError();

    return (sal_Int64)nEndPos;
}

//------------------------------------------------------------------------------
OStreamWrapper::OStreamWrapper(SvStream& _rStream)
{
    SetStream( &_rStream, false );
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL OStreamWrapper::getInputStream(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return this;
}

//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL OStreamWrapper::getOutputStream(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return this;
}

//------------------------------------------------------------------------------
void SAL_CALL OStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException)
{
    sal_uInt32 nWritten = m_pSvStream->Write(aData.getConstArray(),aData.getLength());
    ErrCode err = m_pSvStream->GetError();
    if  (   (ERRCODE_NONE != err)
        ||  (nWritten != (sal_uInt32)aData.getLength())
        )
    {
        throw stario::BufferSizeExceededException(OUString(),static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OStreamWrapper::flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException)
{
    m_pSvStream->Flush();
    if (m_pSvStream->GetError() != ERRCODE_NONE)
        throw stario::NotConnectedException(OUString(),static_cast<staruno::XWeak*>(this));
}

//------------------------------------------------------------------------------
void SAL_CALL OStreamWrapper::closeOutput() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException)
{
}

//------------------------------------------------------------------------------
void SAL_CALL OStreamWrapper::truncate() throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    m_pSvStream->SetStreamSize(0);
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
