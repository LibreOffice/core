/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oslstream.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_configmgr.hxx"
#include "oslstream.hxx"

#include "filehelper.hxx"

namespace configmgr
{
    static void raiseIOException(osl::File::RC error, staruno::Reference<staruno::XInterface> const & context)
    {
        const rtl::OUString message = FileHelper::createOSLErrorString(error);
        switch (error)
        {
        case osl::File::E_NOMEM:
            throw stario::BufferSizeExceededException(message, context);

        case osl::File::E_BADF:
            throw stario::NotConnectedException(message, context);

        default:
            throw stario::IOException(message, context);
        }
    }
//------------------------------------------------------------------
OSLInputStreamWrapper::OSLInputStreamWrapper( osl::File& _rFile )
                 :m_pFile(&_rFile)
                 ,m_bFileOwner(sal_False)
{
}

//------------------------------------------------------------------
OSLInputStreamWrapper::OSLInputStreamWrapper( osl::File* pStream, sal_Bool bOwner )
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

    osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt64 nRead = 0;
    osl::File::RC eError = m_pFile->read(aData.getArray(), nBytesToRead, nRead);
    if (eError != osl::File::E_None)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    // Wenn gelesene Zeichen < MaxLength, staruno::Sequence anpassen
    if (nRead < (sal_uInt64)nBytesToRead)
        aData.realloc( sal::static_int_cast<sal_Int32>( nRead ));

    return sal::static_int_cast<sal_Int32>( nRead );
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
    osl::MutexGuard aGuard( m_aMutex );
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nCurrentPos;
    m_pFile->getPos(nCurrentPos);

    sal_uInt64 nNewPos = nCurrentPos + nBytesToSkip;
    osl::File::RC eError = m_pFile->setPos(osl_Pos_Absolut, nNewPos);
    if (eError != osl::File::E_None)
    {
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));
    }
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL OSLInputStreamWrapper::available() throw( stario::NotConnectedException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (!m_pFile)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nPos;
    osl::File::RC eError = m_pFile->getPos(nPos);
    if (eError != osl::File::E_None)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nDummy = 0;
    eError = m_pFile->setPos(Pos_End, nDummy);
    if (eError != osl::File::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    sal_uInt64 nAvailable;
    eError = m_pFile->getPos(nAvailable);
    if (eError != osl::File::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    nAvailable = nAvailable - nPos;
    eError = m_pFile->setPos(Pos_Absolut, nPos);
    if (eError != osl::File::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    return sal::static_int_cast<sal_Int32>( nAvailable );
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
    sal_uInt32 const nLength = sal_uInt32(aData.getLength());
    if (nLength != 0)
    {
        sal_uInt64 nWritten;
        osl::File::RC eError = rFile.write(aData.getConstArray(),nLength, nWritten);
        if (eError != osl::File::E_None || nWritten != nLength)
        {
            throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
        }
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

//------------------------------------------------------------------
//------------------------------------------------------------------
BufferedFileOutputStream::BufferedFileOutputStream( rtl::OUString const & aFileURL, bool bCreate, sal_uInt32 nBufferSizeHint)
: m_aFile( aFileURL, nBufferSizeHint )
{
    sal_Int32 flags = bCreate ? OpenFlag_Write|OpenFlag_Create : OpenFlag_Write;

    osl::File::RC rc = m_aFile.open(flags);
    if (rc != osl::File::E_None)
        raiseIOException(rc,NULL);
}

//------------------------------------------------------------------
BufferedFileOutputStream::~BufferedFileOutputStream()
{
}

//------------------------------------------------------------------------------
void SAL_CALL BufferedFileOutputStream::writeBytes(const staruno::Sequence< sal_Int8 >& aData)
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    const sal_uInt64 size = sal_uInt64(aData.getLength());
    sal_uInt64 written = 0;

    osl::File::RC rc = m_aFile.write(aData.getConstArray(), size, written);
    if (rc != osl::File::E_None)
        raiseIOException(rc,*this);

    // we don't support special files where multiple write passes are needed
    if (written < size)
        raiseIOException(osl::File::E_IO,*this);
}

void SAL_CALL BufferedFileOutputStream::flush()
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    osl::File::RC rc = m_aFile.sync();
    if (rc != osl::File::E_None)
        raiseIOException(rc,*this);
}

void SAL_CALL BufferedFileOutputStream::closeOutput()
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    osl::File::RC rc = m_aFile.close();
    if (rc != osl::File::E_None)
        raiseIOException(rc,*this);
}

} // namespace configmgr


