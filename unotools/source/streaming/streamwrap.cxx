/*************************************************************************
 *
 *  $RCSfile: streamwrap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:55 $
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
    if (!m_pSvStream)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    if (nBytesToRead < 0)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt32 nRead = m_pSvStream->Read((void*)aData.getArray(), nBytesToRead);
    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    // Wenn gelesene Zeichen < MaxLength, staruno::Sequence anpassen
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    if (!m_pSvStream)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

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
    if (!m_pSvStream)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

#ifdef DBG_UTIL
    sal_uInt32 nCurrentPos = m_pSvStream->Tell();
#endif

    m_pSvStream->SeekRel(nBytesToSkip);
    if (m_pSvStream->SvStream::GetError() != ERRCODE_NONE)
    {
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));
    }

#ifdef DBG_UTIL
    nCurrentPos = m_pSvStream->Tell();
#endif
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OInputStreamWrapper::available() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_pSvStream)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt32 nPos = m_pSvStream->Tell();
    if (m_pSvStream->SvStream::GetError())
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    m_pSvStream->Seek(STREAM_SEEK_TO_END);
    if (m_pSvStream->SvStream::GetError())
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    sal_Int32 nAvailable = (sal_Int32)m_pSvStream->Tell() - nPos;
    m_pSvStream->Seek(nPos);
    if (m_pSvStream->SvStream::GetError())
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    return nAvailable;
}

//------------------------------------------------------------------------------
void SAL_CALL OInputStreamWrapper::closeInput() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    if (!m_pSvStream)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    if (m_bSvStreamOwner)
        delete m_pSvStream;

    m_pSvStream = NULL;
}

/*************************************************************************/
// stario::XOutputStream
//------------------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    sal_uInt32 nWritten = rStream.Write(aData.getConstArray(),aData.getLength());
    ErrCode err = rStream.GetError();
    if (err != ERRCODE_NONE || nWritten != aData.getLength())
    {
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::flush() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    rStream.Flush();
    if (rStream.GetError() != ERRCODE_NONE)
        throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
}

//------------------------------------------------------------------
void SAL_CALL OOutputStreamWrapper::closeOutput() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
}

} // namespace utl


