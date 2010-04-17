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
#include "precompiled_xmerge.hxx"

#ifndef _OSL_STREAM_WRAPPER_HXX_
#include "streamwrap.hxx"
#endif
#include <osl/file.hxx>

namespace foo
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::osl;

//==================================================================
//= OInputStreamWrapper
//==================================================================
//------------------------------------------------------------------
OInputStreamWrapper::OInputStreamWrapper( File& _rStream )
                 :m_pSvStream(&_rStream)
                 ,m_bSvStreamOwner(sal_False)
{
}

//------------------------------------------------------------------
OInputStreamWrapper::OInputStreamWrapper( File* pStream, sal_Bool bOwner )
                 :m_pSvStream( pStream )
                 ,m_bSvStreamOwner( bOwner )
{
}

//------------------------------------------------------------------
OInputStreamWrapper::~OInputStreamWrapper()
{
    if( m_bSvStreamOwner )
        delete m_pSvStream;

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

    sal_uInt64 nRead = 0;
    m_pSvStream->read((void*)aData.getArray(), nBytesToRead,nRead);

    checkError();

    // Wenn gelesene Zeichen < MaxLength, staruno::Sequence anpassen
    if (nRead < nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    checkError();

    if (nMaxBytesToRead < 0)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------------------
void SAL_CALL OInputStreamWrapper::skipBytes(sal_Int32 nBytesToSkip) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkError();

    m_pSvStream->setPos(osl_Pos_Current,nBytesToSkip);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::available() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt64 nPos = 0;
    m_pSvStream->getPos(nPos);
    checkError();

    m_pSvStream->setPos(Pos_End,0);
    checkError();

    sal_uInt64 nAvailable = 0;
    m_pSvStream->getPos(nAvailable);
    nAvailable -= nPos;

    m_pSvStream->setPos(Pos_Absolut,nPos);
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
}

//==================================================================
//= OSeekableInputStreamWrapper
//==================================================================
//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(File& _rStream)
    :OInputStreamWrapper(_rStream)
{
}

//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(File* _pStream, sal_Bool _bOwner)
    :OInputStreamWrapper(_pStream, _bOwner)
{
}

//------------------------------------------------------------------------------
Any SAL_CALL OSeekableInputStreamWrapper::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = OInputStreamWrapper::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = OSeekableInputStreamWrapper_Base::queryInterface(_rType);
    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableInputStreamWrapper::acquire(  ) throw ()
{
    OInputStreamWrapper::acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableInputStreamWrapper::release(  ) throw ()
{
    OInputStreamWrapper::release();
}

//------------------------------------------------------------------------------
void SAL_CALL OSeekableInputStreamWrapper::seek( sal_Int64 _nLocation ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    m_pSvStream->setPos(Pos_Current,(sal_uInt32)_nLocation);
    checkError();
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OSeekableInputStreamWrapper::getPosition(  ) throw (IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt64 nPos = 0;
    nPos = m_pSvStream->getPos(nPos);
    checkError();
    return nPos;
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL OSeekableInputStreamWrapper::getLength(  ) throw (IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkConnected();

    sal_uInt64 nCurrentPos = 0;
    m_pSvStream->getPos(nCurrentPos);
    checkError();

    m_pSvStream->setPos(osl_Pos_End,0);
    sal_uInt64 nEndPos = 0;
    m_pSvStream->getPos(nEndPos);
    m_pSvStream->setPos(osl_Pos_Absolut,nCurrentPos);

    checkError();

    return nEndPos;
}

//==================================================================
//= OOutputStreamWrapper
//==================================================================
//------------------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    sal_uInt64 nWritten = 0;
    rStream.write(aData.getConstArray(),aData.getLength(),nWritten);
    if (nWritten != aData.getLength())
    {
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::flush() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
}

//------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::closeOutput() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
}

} // namespace utl


