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

#include <sal/config.h>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

namespace utl
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

OInputStreamWrapper::OInputStreamWrapper( SvStream& _rStream )
                 :m_pSvStream(&_rStream)
                 ,m_bSvStreamOwner(false)
{
}

OInputStreamWrapper::OInputStreamWrapper( SvStream* pStream, bool bOwner )
                 :m_pSvStream( pStream )
                 ,m_bSvStreamOwner( bOwner )
{
}

OInputStreamWrapper::~OInputStreamWrapper()
{
    if( m_bSvStreamOwner )
        delete m_pSvStream;
}

sal_Int32 SAL_CALL OInputStreamWrapper::readBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
{
    checkConnected();

    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    if (aData.getLength() < nBytesToRead)
        aData.realloc(nBytesToRead);

    sal_uInt32 nRead = m_pSvStream->ReadBytes(static_cast<void*>(aData.getArray()), nBytesToRead);
    checkError();

    // Wenn gelesene Zeichen < MaxLength, css::uno::Sequence anpassen
    if (nRead < (std::size_t)aData.getLength())
        aData.realloc( nRead );

    return nRead;
}

sal_Int32 SAL_CALL OInputStreamWrapper::readSomeBytes(css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
{
    checkError();

    if (nMaxBytesToRead < 0)
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));

    if (m_pSvStream->IsEof())
    {
        aData.realloc(0);
        return 0;
    }
    else
        return readBytes(aData, nMaxBytesToRead);
}

void SAL_CALL OInputStreamWrapper::skipBytes(sal_Int32 nBytesToSkip)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkError();

    m_pSvStream->SeekRel(nBytesToSkip);
    checkError();
}

sal_Int32 SAL_CALL OInputStreamWrapper::available()
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

void SAL_CALL OInputStreamWrapper::closeInput()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    if (m_bSvStreamOwner)
        delete m_pSvStream;

    m_pSvStream = nullptr;
}

void OInputStreamWrapper::checkConnected() const
{
    if (!m_pSvStream)
        throw css::io::NotConnectedException(OUString(), const_cast<css::uno::XWeak*>(static_cast<const css::uno::XWeak*>(this)));
}

void OInputStreamWrapper::checkError() const
{
    checkConnected();

    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
        // TODO: really evaluate the error
        throw css::io::NotConnectedException(OUString(), const_cast<css::uno::XWeak*>(static_cast<const css::uno::XWeak*>(this)));
}

//= OSeekableInputStreamWrapper

OSeekableInputStreamWrapper::~OSeekableInputStreamWrapper() = default;

OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream& _rStream)
{
    SetStream( &_rStream, false );
}

OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream* _pStream, bool _bOwner)
{
    SetStream( _pStream, _bOwner );
}

void SAL_CALL OSeekableInputStreamWrapper::seek( sal_Int64 _nLocation )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    m_pSvStream->Seek((sal_uInt32)_nLocation);
    checkError();
}

sal_Int64 SAL_CALL OSeekableInputStreamWrapper::getPosition(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt32 nPos = m_pSvStream->Tell();
    checkError();
    return (sal_Int64)nPos;
}

sal_Int64 SAL_CALL OSeekableInputStreamWrapper::getLength(  )
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

//= OOutputStreamWrapper

OOutputStreamWrapper::OOutputStreamWrapper(SvStream& _rStream):
    rStream(_rStream)
{}

OOutputStreamWrapper::~OOutputStreamWrapper() {}

void SAL_CALL OOutputStreamWrapper::writeBytes(const css::uno::Sequence< sal_Int8 >& aData)
{
    sal_uInt32 nWritten = rStream.WriteBytes(aData.getConstArray(), aData.getLength());
    ErrCode err = rStream.GetError();
    if  (   (ERRCODE_NONE != err)
        ||  (nWritten != (sal_uInt32)aData.getLength())
        )
    {
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));
    }
}

void SAL_CALL OOutputStreamWrapper::flush()
{
    rStream.Flush();
    checkError();
}

void SAL_CALL OOutputStreamWrapper::closeOutput()
{
}

void OOutputStreamWrapper::checkError() const
{
    if (rStream.GetError() != ERRCODE_NONE)
        // TODO: really evaluate the error
        throw css::io::NotConnectedException(OUString(), const_cast<css::uno::XWeak*>(static_cast<const css::uno::XWeak*>(this)));
}

//= OSeekableOutputStreamWrapper

OSeekableOutputStreamWrapper::OSeekableOutputStreamWrapper(SvStream& _rStream)
    :OOutputStreamWrapper(_rStream)
{
}

OSeekableOutputStreamWrapper::~OSeekableOutputStreamWrapper() {}

Any SAL_CALL OSeekableOutputStreamWrapper::queryInterface( const Type& _rType )
{
    Any aReturn = OOutputStreamWrapper::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OSeekableOutputStreamWrapper_Base::queryInterface(_rType);
    return aReturn;
}

void SAL_CALL OSeekableOutputStreamWrapper::acquire(  ) throw ()
{
    OOutputStreamWrapper::acquire();
}

void SAL_CALL OSeekableOutputStreamWrapper::release(  ) throw ()
{
    OOutputStreamWrapper::release();
}

void SAL_CALL OSeekableOutputStreamWrapper::seek( sal_Int64 _nLocation )
{
    rStream.Seek((sal_uInt32)_nLocation);
    checkError();
}

sal_Int64 SAL_CALL OSeekableOutputStreamWrapper::getPosition(  )
{
    sal_uInt32 nPos = rStream.Tell();
    checkError();
    return (sal_Int64)nPos;
}

sal_Int64 SAL_CALL OSeekableOutputStreamWrapper::getLength(  )
{
    sal_uInt32 nCurrentPos = rStream.Tell();
    checkError();

    rStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt32 nEndPos = rStream.Tell();
    rStream.Seek(nCurrentPos);

    checkError();

    return (sal_Int64)nEndPos;
}

OStreamWrapper::~OStreamWrapper() = default;

OStreamWrapper::OStreamWrapper(SvStream& _rStream)
{
    SetStream( &_rStream, false );
}

css::uno::Reference< css::io::XInputStream > SAL_CALL OStreamWrapper::getInputStream(  )
{
    return this;
}

css::uno::Reference< css::io::XOutputStream > SAL_CALL OStreamWrapper::getOutputStream(  )
{
    return this;
}

void SAL_CALL OStreamWrapper::writeBytes(const css::uno::Sequence< sal_Int8 >& aData)
{
    sal_uInt32 nWritten = m_pSvStream->WriteBytes(aData.getConstArray(), aData.getLength());
    ErrCode err = m_pSvStream->GetError();
    if  (   (ERRCODE_NONE != err)
        ||  (nWritten != (sal_uInt32)aData.getLength())
        )
    {
        throw css::io::BufferSizeExceededException(OUString(),static_cast<css::uno::XWeak*>(this));
    }
}

void SAL_CALL OStreamWrapper::flush()
{
    m_pSvStream->Flush();
    if (m_pSvStream->GetError() != ERRCODE_NONE)
        throw css::io::NotConnectedException(OUString(),static_cast<css::uno::XWeak*>(this));
}

void SAL_CALL OStreamWrapper::closeOutput()
{
}

void SAL_CALL OStreamWrapper::truncate()
{
    m_pSvStream->SetStreamSize(0);
}

} // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
