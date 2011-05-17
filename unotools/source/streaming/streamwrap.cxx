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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
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
                 ,m_bSvStreamOwner(sal_False)
{
    DBG_CTOR(OInputStreamWrapper,NULL);

}

//------------------------------------------------------------------
OInputStreamWrapper::OInputStreamWrapper( SvStream* pStream, sal_Bool bOwner )
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
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

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
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

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

#ifdef DBG_UTIL
    sal_uInt32 nCurrentPos = m_pSvStream->Tell();
#endif

    m_pSvStream->SeekRel(nBytesToSkip);
    checkError();

#ifdef DBG_UTIL
    nCurrentPos = m_pSvStream->Tell();
#endif
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
        throw stario::NotConnectedException(::rtl::OUString(), const_cast<staruno::XWeak*>(static_cast<const staruno::XWeak*>(this)));
}

//------------------------------------------------------------------------------
void OInputStreamWrapper::checkError() const
{
    checkConnected();

    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
        // TODO: really evaluate the error
        throw stario::NotConnectedException(::rtl::OUString(), const_cast<staruno::XWeak*>(static_cast<const staruno::XWeak*>(this)));
}

//==================================================================
//= OSeekableInputStreamWrapper
//==================================================================
//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream& _rStream)
{
    SetStream( &_rStream, sal_False );
}

//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream* _pStream, sal_Bool _bOwner)
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
//------------------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    sal_uInt32 nWritten = rStream.Write(aData.getConstArray(),aData.getLength());
    ErrCode err = rStream.GetError();
    if  (   (ERRCODE_NONE != err)
        ||  (nWritten != (sal_uInt32)aData.getLength())
        )
    {
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
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
        throw stario::NotConnectedException(::rtl::OUString(), const_cast<staruno::XWeak*>(static_cast<const staruno::XWeak*>(this)));
}

//==================================================================
//= OSeekableOutputStreamWrapper
//==================================================================
//------------------------------------------------------------------------------
OSeekableOutputStreamWrapper::OSeekableOutputStreamWrapper(SvStream& _rStream)
    :OOutputStreamWrapper(_rStream)
{
}

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
    SetStream( &_rStream, sal_False );
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
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OStreamWrapper::flush() throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException)
{
    m_pSvStream->Flush();
    if (m_pSvStream->GetError() != ERRCODE_NONE)
        throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
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
