/*************************************************************************
 *
 *  $RCSfile: streamwrap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:44:08 $
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

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
    :OInputStreamWrapper(_rStream)
{
}

//------------------------------------------------------------------------------
OSeekableInputStreamWrapper::OSeekableInputStreamWrapper(SvStream* _pStream, sal_Bool _bOwner)
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

} // namespace utl


