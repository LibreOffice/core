/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: oslstream.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:16:47 $
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
#define _CONFIGMGR_OSLSTREAM_HXX_

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
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

    using rtl::OUString;

// -----------------------------------------------------------------------------
    typedef ::cppu::WeakImplHelper1<stario::XInputStream> InputStreamWrapper_Base;

    /// OSLInputStreamWrapper - implementation of XInputStream on an (unbuffered) osl::File
    class OSLInputStreamWrapper : public InputStreamWrapper_Base
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
    /// BufferedFileInputStream - buffered implementation of XInputStream on an osl::File
    class BufferedFileInputStream: public InputStreamWrapper_Base
    {
        BufferedInputFile   m_aFile;

    public:
        BufferedFileInputStream(rtl::OUString const & aFileURL);
        virtual ~BufferedFileInputStream();

    // stario::XInputStream
        virtual sal_Int32   SAL_CALL
            readBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead)
                throw(stario::NotConnectedException, stario::BufferSizeExceededException,
                        stario::IOException,  staruno::RuntimeException);

        virtual sal_Int32   SAL_CALL
            readSomeBytes(staruno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead)
                throw(stario::NotConnectedException, stario::BufferSizeExceededException,
                        stario::IOException,staruno::RuntimeException);

        virtual void        SAL_CALL
            skipBytes(sal_Int32 nBytesToSkip)
                throw(stario::NotConnectedException, stario::BufferSizeExceededException,
                        stario::IOException,staruno::RuntimeException);

        virtual sal_Int32   SAL_CALL    available()
            throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException);

        virtual void        SAL_CALL    closeInput()
            throw(stario::NotConnectedException, stario::IOException, staruno::RuntimeException);
    };

// -----------------------------------------------------------------------------
    typedef ::cppu::WeakImplHelper1<stario::XOutputStream> OutputStreamWrapper_Base;

    /// OSLOutputStreamWrapper - implementation of XOutputStream on an (unbuffered) osl::File
    class OSLOutputStreamWrapper : public OutputStreamWrapper_Base
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
    class BufferedFileOutputStream: public OutputStreamWrapper_Base
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



