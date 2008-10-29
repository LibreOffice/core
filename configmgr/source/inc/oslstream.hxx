/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oslstream.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#define _CONFIGMGR_OSLSTREAM_HXX_

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>

#include "bufferedfile.hxx"

namespace osl
{
    class File;
}

namespace configmgr
{
    namespace stario    = ::com::sun::star::io;
    namespace staruno   = ::com::sun::star::uno;

// -----------------------------------------------------------------------------
    /// OSLInputStreamWrapper - implementation of XInputStream on an (unbuffered) osl::File
    class OSLInputStreamWrapper : public ::cppu::WeakImplHelper1<stario::XInputStream>
    {
        ::osl::Mutex      m_aMutex;
        ::osl::File*    m_pFile;
        sal_Bool        m_bFileOwner : 1;

    public:
        /// c'tor. _rStream must live at least until closeInput() is called on this stream
        OSLInputStreamWrapper(::osl::File& _rStream);
        /// c'tor. if bOwner is <FALSE/> *pStream must live at least until closeInput() is called on this stream
        OSLInputStreamWrapper(::osl::File* pStream, sal_Bool bOwner=sal_False);
        virtual ~OSLInputStreamWrapper();

    // stario::XInputStream
        virtual sal_Int32   SAL_CALL
            readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

        virtual sal_Int32   SAL_CALL
            readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
                throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

        virtual void        SAL_CALL
            skipBytes(sal_Int32 nBytesToSkip)
                throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

        virtual sal_Int32   SAL_CALL    available() throw(stario::NotConnectedException, staruno::RuntimeException);
        virtual void        SAL_CALL    closeInput() throw(stario::NotConnectedException, staruno::RuntimeException);
    };

// -----------------------------------------------------------------------------
    /// OSLOutputStreamWrapper - implementation of XOutputStream on an (unbuffered) osl::File
    class OSLOutputStreamWrapper : public ::cppu::WeakImplHelper1<stario::XOutputStream>
    {
        ::osl::Mutex      m_aMutex;
        ::osl::File&        rFile;

    public:
        /// c'tor. _rStream must live at least until closeOutput() is called on this stream
        OSLOutputStreamWrapper(::osl::File& _rFile) :rFile(_rFile) { }

    // stario::XOutputStream
        virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData)
            throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

        virtual void SAL_CALL flush()
            throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);

        virtual void SAL_CALL closeOutput()
            throw(stario::NotConnectedException, stario::BufferSizeExceededException, staruno::RuntimeException);
    };
// -----------------------------------------------------------------------------
    /// BufferedFileOutputStream - buffered implementation of XOutputStream on an osl::File
    class BufferedFileOutputStream: public ::cppu::WeakImplHelper1<stario::XOutputStream>
    {
        BufferedOutputFile  m_aFile;

    public:
        BufferedFileOutputStream(rtl::OUString const & aFileURL, bool bCreate=true, sal_uInt32 nBufferSizeHint=0);
        virtual ~BufferedFileOutputStream();

    // stario::XOutputStream
        virtual void SAL_CALL writeBytes(const staruno::Sequence< sal_Int8 >& aData)
            throw(stario::NotConnectedException, stario::BufferSizeExceededException,
                    stario::IOException, staruno::RuntimeException);

        virtual void SAL_CALL flush()
            throw(stario::NotConnectedException, stario::BufferSizeExceededException,
                    stario::IOException, staruno::RuntimeException);

        virtual void SAL_CALL closeOutput()
            throw(stario::NotConnectedException, stario::BufferSizeExceededException,
                   stario::IOException, staruno::RuntimeException);
    };
// -----------------------------------------------------------------------------

}   // namespace configmgr

#endif // _CONFIGMGR_OSLSTREAM_HXX_



