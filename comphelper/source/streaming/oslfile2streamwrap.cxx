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
#include "precompiled_comphelper.hxx"
#include <comphelper/oslfile2streamwrap.hxx>

#include <algorithm>

namespace comphelper
{
    using namespace osl;

//------------------------------------------------------------------
OSLInputStreamWrapper::OSLInputStreamWrapper( File& _rFile )
                 :m_pFile(&_rFile)
                 ,m_bFileOwner(sal_False)
{
}

//------------------------------------------------------------------
OSLInputStreamWrapper::OSLInputStreamWrapper( File* pStream, sal_Bool bOwner )
                 :m_pFile( pStream )
                 ,m_bFileOwner( bOwner )
{
}

//------------------------------------------------------------------
OSLInputStreamWrapper::~OSLInputStreamWrapper()
{
    if( m_bFileOwner )
        delete m_pFile;
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OSLInputStreamWrapper::readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    if (nBytesToRead < 0)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    ::osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt64 nRead = 0;
    FileBase::RC eError = m_pFile->read((void*)aData.getArray(), nBytesToRead, nRead);
    if (eError != FileBase::E_None)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    // Wenn gelesene Zeichen < MaxLength, staruno::Sequence anpassen
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( sal::static_int_cast< sal_Int32 >(nRead) );

    return sal::static_int_cast< sal_Int32 >(nRead);
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OSLInputStreamWrapper::readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    if (nMaxBytesToRead < 0)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    /*
      if (m_pFile->IsEof())
      {
      aData.realloc(0);
      return 0;
      }
      else
    */
    return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------------------
void SAL_CALL OSLInputStreamWrapper::skipBytes(sal_Int32 nBytesToSkip) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nCurrentPos;
    m_pFile->getPos(nCurrentPos);

    sal_uInt64 nNewPos = nCurrentPos + nBytesToSkip;
    FileBase::RC eError = m_pFile->setPos(osl_Pos_Absolut, nNewPos);
    if (eError != FileBase::E_None)
    {
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));
    }

#ifdef DBG_UTIL
    m_pFile->getPos(nCurrentPos);
//  volatile int dummy = 0;                      // to take a look at last changes ;-)
#endif
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OSLInputStreamWrapper::available() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nPos;
    FileBase::RC eError = m_pFile->getPos(nPos);
    if (eError != FileBase::E_None)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nDummy = 0;
    eError = m_pFile->setPos(Pos_End, nDummy);
    if (eError != FileBase::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    sal_uInt64 nAvailable;
    eError = m_pFile->getPos(nAvailable);
    if (eError != FileBase::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    nAvailable = nAvailable - nPos;
    eError = m_pFile->setPos(Pos_Absolut, nPos);
    if (eError != FileBase::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    return sal::static_int_cast< sal_Int32 >(
        std::max(nAvailable, sal::static_int_cast< sal_uInt64 >(SAL_MAX_INT32)));
}

//------------------------------------------------------------------------------
void SAL_CALL OSLInputStreamWrapper::closeInput() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    m_pFile->close();
    if (m_bFileOwner)
        delete m_pFile;

    m_pFile = NULL;
}

/*************************************************************************/
// stario::XOutputStream
//------------------------------------------------------------------------------
void SAL_CALL OSLOutputStreamWrapper::writeBytes(const staruno::Sequence< sal_Int8 >& aData) throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    sal_uInt64 nWritten;
    FileBase::RC eError = rFile.write(aData.getConstArray(),aData.getLength(), nWritten);
    if (eError != FileBase::E_None
        || nWritten != sal::static_int_cast< sal_uInt32 >(aData.getLength()))
    {
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------
void SAL_CALL OSLOutputStreamWrapper::flush() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
}

//------------------------------------------------------------------
void SAL_CALL OSLOutputStreamWrapper::closeOutput() throw( stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException )
{
    rFile.close();
}

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
