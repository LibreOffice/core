/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oslstream.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:27:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#include "oslstream.hxx"
#endif

#include "filehelper.hxx"

namespace configmgr
{
    using namespace osl;

    static void raiseIOException(osl::File::RC error, staruno::Reference<staruno::XInterface> const & context)
    {
        const rtl::OUString message = FileHelper::createOSLErrorString(error);
        switch (error)
        {
        case File::E_NOMEM:
            throw stario::BufferSizeExceededException(message, context);

        case File::E_BADF:
            throw stario::NotConnectedException(message, context);

        default:
            throw stario::IOException(message, context);
        }
    }
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

    osl::MutexGuard aGuard( m_aMutex );

    aData.realloc(nBytesToRead);

    sal_uInt64 nRead = 0;
    File::RC eError = m_pFile->read(aData.getArray(), nBytesToRead, nRead);
    if (eError != File::E_None)
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
    File::RC eError = m_pFile->setPos(osl_Pos_Absolut, nNewPos);
    if (eError != File::E_None)
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
    File::RC eError = m_pFile->getPos(nPos);
    if (eError != File::E_None)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nDummy = 0;
    eError = m_pFile->setPos(Pos_End, nDummy);
    if (eError != File::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    sal_uInt64 nAvailable;
    eError = m_pFile->getPos(nAvailable);
    if (eError != File::E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    nAvailable = nAvailable - nPos;
    eError = m_pFile->setPos(Pos_Absolut, nPos);
    if (eError != File::E_None)
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
        File::RC eError = rFile.write(aData.getConstArray(),nLength, nWritten);
        if (eError != File::E_None || nWritten != nLength)
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

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// ----------------------------- Buffered OSLStream -----------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//------------------------------------------------------------------
/*
BufferedFileInputStream::BufferedFileInputStream( File& _rFile )
                 :m_pFile(&_rFile)
                 ,m_bFileOwner(sal_False)
{
}
*/
//------------------------------------------------------------------
BufferedFileInputStream::BufferedFileInputStream( rtl::OUString const & aFileURL)
: m_aFile( aFileURL )
{
    osl::File::RC rc = m_aFile.open(osl_File_OpenFlag_Read);
    if (rc != File::E_None)
        raiseIOException(rc,NULL);
}

//------------------------------------------------------------------
BufferedFileInputStream::~BufferedFileInputStream()
{
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL BufferedFileInputStream::readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    if (nBytesToRead < 0)
        raiseIOException(osl::File::E_INVAL, *this);

    aData.realloc(nBytesToRead);

    sal_uInt64 nSize = sal_uInt64(nBytesToRead);
    sal_uInt64 nRead = 0;

    File::RC rc = m_aFile.read(aData.getArray(), nSize, nRead);
    if (rc != File::E_None)
        raiseIOException(rc,*this);

    OSL_ASSERT(nRead <= nSize);
    return sal_Int32(nRead);
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL BufferedFileInputStream::readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------------------
void SAL_CALL BufferedFileInputStream::skipBytes(sal_Int32 nBytesToSkip)
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    if (nBytesToSkip < 0)
        raiseIOException(osl::File::E_INVAL, *this);

    sal_uInt64 nOffset = sal_uInt64(nBytesToSkip);

    File::RC rc = m_aFile.setPos(osl_Pos_Current, nOffset);
    if (rc != File::E_None)
        raiseIOException(rc,*this);
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL BufferedFileInputStream::available()
    throw( stario::NotConnectedException, stario::IOException, staruno::RuntimeException )
{
    sal_uInt64 avail = 0;
    File::RC rc = m_aFile.available(avail);
    if (rc != File::E_None)
        raiseIOException(rc,*this);

    sal_Int32 result = sal_Int32(avail);
    if (result < 0 || sal_uInt64(result) < avail)
        result = SAL_MAX_INT32;

    return result;
}

//------------------------------------------------------------------------------
void SAL_CALL BufferedFileInputStream::closeInput()
    throw( stario::NotConnectedException, stario::IOException, staruno::RuntimeException )
{
    File::RC rc = m_aFile.close();
    if (rc != File::E_None)
        raiseIOException(rc,*this);
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BufferedFileOutputStream::BufferedFileOutputStream( rtl::OUString const & aFileURL, bool bCreate, sal_uInt32 nBufferSizeHint)
: m_aFile( aFileURL, nBufferSizeHint )
{
    sal_Int32 flags = bCreate ? OpenFlag_Write|OpenFlag_Create : OpenFlag_Write;

    osl::File::RC rc = m_aFile.open(flags);
    if (rc != File::E_None)
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
    if (rc != File::E_None)
        raiseIOException(rc,*this);

    // we don't support special files where multiple write passes are needed
    if (written < size)
        raiseIOException(File::E_IO,*this);
}

void SAL_CALL BufferedFileOutputStream::flush()
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    osl::File::RC rc = m_aFile.sync();
    if (rc != File::E_None)
        raiseIOException(rc,*this);
}

void SAL_CALL BufferedFileOutputStream::closeOutput()
    throw( stario::NotConnectedException, stario::BufferSizeExceededException,
            stario::IOException, staruno::RuntimeException )
{
    osl::File::RC rc = m_aFile.close();
    if (rc != File::E_None)
        raiseIOException(rc,*this);
}

} // namespace configmgr


