/*************************************************************************
 *
 *  $RCSfile: oslfile2streamwrap.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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

#ifndef _COMPHELPER_STREAM_OSLFILEWRAPPER_HXX_
#include <comphelper/oslfile2streamwrap.hxx>
#endif

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
    if (eError != osl_File_E_None)
        throw stario::BufferSizeExceededException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    // Wenn gelesene Zeichen < MaxLength, staruno::Sequence anpassen
    if (nRead < (sal_uInt32)nBytesToRead)
        aData.realloc( nRead );

    return nRead;
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
    if (eError != osl_File_E_None)
    {
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));
    }

#ifdef DBG_UTIL
    m_pFile->getPos(nCurrentPos);
    volatile int dummy = 0;                      // to take a look at last changes ;-)
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
    if (eError != osl_File_E_None)
        throw stario::NotConnectedException(::rtl::OUString(), static_cast<staruno::XWeak*>(this));

    sal_uInt64 nDummy = 0;
    eError = m_pFile->setPos(Pos_End, nDummy);
    if (eError != osl_File_E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    sal_uInt64 nAvailable;
    eError = m_pFile->getPos(nAvailable);
    if (eError != osl_File_E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));

    nAvailable = nAvailable - nPos;
    eError = m_pFile->setPos(Pos_Absolut, nPos);
    if (eError != osl_File_E_None)
       throw stario::NotConnectedException(::rtl::OUString(),static_cast<staruno::XWeak*>(this));
    return nAvailable;
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
    if (eError != osl_File_E_None || nWritten != aData.getLength())
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


