/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryreader.cxx,v $
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

#include "binaryreader.hxx"
#include "binarytype.hxx"
#include "valuenode.hxx"
#include "filehelper.hxx"
#include "oslstream.hxx"


#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XDataOutputStream.hpp>
#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/UnexpectedEOFException.hpp>

#include <cppuhelper/implbase1.hxx>
#include <osl/file.hxx>

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif
#include "tracer.hxx"

#define ASCII(x) rtl::OUString::createFromAscii(x)

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace uno = com::sun::star::uno;
        namespace io = com::sun::star::io;


        // --------------------------------------------------------------------------

        inline rtl::OUString ErrorToMessage_Impl (osl::FileBase::RC eError)
        {
            return FileHelper::createOSLErrorString (eError);
        }

        // --------------------------------------------------------------------------

        class BinaryReader_Impl :
            public cppu::WeakImplHelper1< com::sun::star::io::XDataInputStream >
        {
        public:
            /** Construction.
            */
            explicit BinaryReader_Impl (rtl::OUString const & rFileUrl)
                SAL_THROW( (io::IOException, uno::RuntimeException) );

            /** XInputStream.
            */
            virtual sal_Int32 SAL_CALL readBytes (
                uno::Sequence<sal_Int8> & rData, sal_Int32 nBytesToRead)
                throw (
                    io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException);

            virtual sal_Int32 SAL_CALL readSomeBytes (
                uno::Sequence<sal_Int8> & rData, sal_Int32 nBytesToRead)
                throw (
                    io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException);

            virtual void SAL_CALL skipBytes (sal_Int32 nBytesToSkip)
                throw (
                    io::NotConnectedException,
                    io::BufferSizeExceededException,
                    io::IOException, uno::RuntimeException);

            virtual sal_Int32 SAL_CALL available()
                throw (
                    io::NotConnectedException,
                    io::IOException, uno::RuntimeException);

            virtual void SAL_CALL closeInput()
                throw (
                    io::NotConnectedException,
                    io::IOException, uno::RuntimeException);


            /** XDataInputStream.
            */
            virtual sal_Int8 SAL_CALL readBoolean()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual sal_Int8 SAL_CALL readByte()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual sal_Unicode SAL_CALL readChar()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual sal_Int16 SAL_CALL readShort()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual sal_Int32 SAL_CALL readLong()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual sal_Int64 SAL_CALL readHyper()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual float SAL_CALL readFloat()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual double SAL_CALL readDouble()
                throw (
                    io::IOException, uno::RuntimeException);

            virtual rtl::OUString SAL_CALL readUTF()
                throw (
                    io::IOException, uno::RuntimeException);

        protected:
            /** Destruction.
            */
            virtual ~BinaryReader_Impl();

        private:
            sal_uInt32 checkAvail(); // may throw NotConnectedException
            sal_uInt32 getMaxAvail(sal_Int32 nRequest); // may throw NotConnectedException, BufferSizeExceededException
            sal_uInt8 const * readBuffer(sal_uInt32 nRequired); // may throw NotConnectedException, UnexpectedEOFException
        private:
            /** Representation.
            */
            sal_uInt8 * m_pBuffer;
            sal_uInt32  m_nLength;
            sal_uInt32  m_nOffset;

            /** Not implemented.
            */
            BinaryReader_Impl (const BinaryReader_Impl&);
            BinaryReader_Impl& operator= (const BinaryReader_Impl&);
    };

    // --------------------------------------------------------------------------
    static inline void checkIOError(osl::File::RC errcode)
    {
        if (errcode != osl::FileBase::E_None)
        {
            throw io::IOException (ErrorToMessage_Impl (errcode), NULL);
        }
    }
    static void raiseBufferError()
    {
        rtl::OUString sMsg = rtl::OUString::createFromAscii("Cannot allocate Buffer: Too large");
        throw io:: BufferSizeExceededException(sMsg, NULL);
    }
    // -------------------------------------------------------------------------
        BinaryReader_Impl::BinaryReader_Impl (rtl::OUString const & rFileUrl)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        : m_pBuffer (0)
        , m_nLength (0)
        , m_nOffset (0)
        {
            osl::File aFile (rFileUrl);

            checkIOError( aFile.open (OpenFlag_Read) );
            sal_uInt64 nLength = 0;
            checkIOError( aFile.getSize (nLength) );
            if (nLength > 0xffffffff)
                raiseBufferError();
            m_nLength = sal_uInt32(nLength);

            sal_uInt8 *pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory (m_nLength));
            if (!pBuffer)
                raiseBufferError();

            sal_uInt64 nRead = 0;
            osl::File::RC result = aFile.read (pBuffer, nLength, nRead);
            if (result != osl::FileBase::E_None)
            {
                rtl_freeMemory (pBuffer);
                checkIOError( result );
            }
            if (nRead != nLength)
            {
                rtl_freeMemory (pBuffer);
                rtl::OUString sMsg = rtl::OUString::createFromAscii("BinaryCache - Could not read entire size of file: ");
                throw io::UnexpectedEOFException(sMsg.concat(rFileUrl),NULL);
            }
            m_pBuffer = pBuffer;
        }

        // --------------------------------------------------------------------------

        BinaryReader_Impl::~BinaryReader_Impl()
        {
            if (m_pBuffer) rtl_freeMemory (m_pBuffer);
        }

        // --------------------------------------------------------------------------
        // XInputStream implementation.
        // --------------------------------------------------------------------------
        sal_uInt32 BinaryReader_Impl::checkAvail ()
        {
            if (!m_pBuffer)
            {
                rtl::OUString sMsg = rtl::OUString::createFromAscii("BinaryCache - Stream is not open. No data available for reading.");
                throw io::NotConnectedException(sMsg,*this);
            }
            OSL_ASSERT(m_nLength >= m_nOffset);
              return m_nLength - m_nOffset;
        }
        // --------------------------------------------------------------------------

        sal_uInt32 BinaryReader_Impl::getMaxAvail (sal_Int32 nRequest)
        {
            if (nRequest < 0)
            {
                rtl::OUString sMsg = rtl::OUString::createFromAscii("BinaryCache - Invalid read request - negative byte count requested.");
                throw io::BufferSizeExceededException(sMsg,*this);
            }
            sal_uInt32 const uRequest = sal_uInt32(nRequest);
            sal_uInt32 const uAvail = checkAvail ();
            return std::min(uRequest,uAvail);
        }
        // --------------------------------------------------------------------------

        sal_uInt8 const * BinaryReader_Impl::readBuffer (sal_uInt32 nRequest)
        {
            sal_uInt32 const nAvail = checkAvail ();
            if (nRequest > nAvail)
            {
                rtl::OUString sMsg = rtl::OUString::createFromAscii("BinaryCache - Invalid file format - read past end-of-file.");
                throw io::UnexpectedEOFException(sMsg,*this);
            }
            sal_uInt8 const * pData = m_pBuffer + m_nOffset;
            m_nOffset += nRequest;
            return pData;
        }
        // --------------------------------------------------------------------------

        sal_Int32 SAL_CALL BinaryReader_Impl::readBytes (
            uno::Sequence<sal_Int8> & rData, sal_Int32 nBytesToRead)
            throw (
                io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException, uno::RuntimeException)
        {
            sal_uInt32 nRead = getMaxAvail(nBytesToRead);
            if (nRead > 0)
            {
                rData.realloc (nRead);
                memcpy (rData.getArray(), readBuffer(nRead), nRead);
            }
            return sal_Int32(nRead);
        }

        // --------------------------------------------------------------------------

        sal_Int32 SAL_CALL BinaryReader_Impl::readSomeBytes (
            uno::Sequence<sal_Int8> & rData, sal_Int32 nBytesToRead)
            throw (
                io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException, uno::RuntimeException)
        {
            return readBytes(rData,nBytesToRead);
        }

        // --------------------------------------------------------------------------

        void SAL_CALL BinaryReader_Impl::skipBytes (sal_Int32 nBytesToSkip)
            throw (
                io::NotConnectedException,
                io::BufferSizeExceededException,
                io::IOException, uno::RuntimeException)
        {
            (void) readBuffer(sal_uInt32(nBytesToSkip));
        }

        // --------------------------------------------------------------------------

        sal_Int32 SAL_CALL BinaryReader_Impl::available()
            throw (
                io::NotConnectedException,
                io::IOException, uno::RuntimeException)
        {
            const sal_uInt32 nMaxAvail = 0x7FFFFFFF;
            const sal_uInt32 nAvail = checkAvail();
            return sal_Int32(std::min(nAvail,nMaxAvail));
        }

        // --------------------------------------------------------------------------

        void SAL_CALL BinaryReader_Impl::closeInput()
            throw (
                io::NotConnectedException,
                io::IOException, uno::RuntimeException)
        {
            OSL_ENSURE(m_pBuffer,"BinaryCache - Closing stream that is already closed");
            if (m_pBuffer)
            {
                rtl_freeMemory (m_pBuffer);
                m_pBuffer = 0;
            }
        }

        // --------------------------------------------------------------------------
        // XDataInputStream implementation.
        // --------------------------------------------------------------------------

        sal_Int8 SAL_CALL BinaryReader_Impl::readBoolean()
            throw (io::IOException, uno::RuntimeException)
        {
            return this->readByte();
        }

        // --------------------------------------------------------------------------

        sal_Int8 SAL_CALL BinaryReader_Impl::readByte()
            throw (io::IOException, uno::RuntimeException)
        {
            sal_Int8 result = sal_Int8(*readBuffer(1));

            return result;
        }

        // --------------------------------------------------------------------------

        sal_Unicode SAL_CALL BinaryReader_Impl::readChar()
            throw (io::IOException, uno::RuntimeException)
        {
            sal_Unicode result;

            sal_uInt8 const * pData = readBuffer(sizeof result);

            result = sal_Unicode(
                (sal_uInt16(pData[0]) << 8) |
                (sal_uInt16(pData[1]) << 0)   );

            return result;
        }

        // --------------------------------------------------------------------------

        sal_Int16 SAL_CALL BinaryReader_Impl::readShort()
            throw (io::IOException, uno::RuntimeException)
        {
            sal_Int16 result;

            sal_uInt8 const * pData = readBuffer(sizeof result);

            result = sal_Int16(
                (sal_uInt16(pData[0]) << 8) |
                (sal_uInt16(pData[1]) << 0)   );

            return result;
        }

        // --------------------------------------------------------------------------

        sal_Int32 SAL_CALL BinaryReader_Impl::readLong()
            throw (io::IOException, uno::RuntimeException)
        {
            sal_Int32 result;

            sal_uInt8 const * pData = readBuffer(sizeof result);

            result = sal_Int32(
                (sal_uInt32(pData[0]) << 24) |
                (sal_uInt32(pData[1]) << 16) |
                (sal_uInt32(pData[2]) <<  8) |
                (sal_uInt32(pData[3]) <<  0)   );

            return result;
        }

        // --------------------------------------------------------------------------

        sal_Int64 SAL_CALL BinaryReader_Impl::readHyper()
            throw (io::IOException, uno::RuntimeException)
        {
            sal_Int64 result;

            sal_uInt8 const * pData = readBuffer(sizeof result);

            result = sal_Int64(
                (sal_uInt64(pData[0]) << 56) |
                (sal_uInt64(pData[1]) << 48) |
                (sal_uInt64(pData[2]) << 40) |
                (sal_uInt64(pData[3]) << 32) |
                (sal_uInt64(pData[4]) << 24) |
                (sal_uInt64(pData[5]) << 16) |
                (sal_uInt64(pData[6]) <<  8) |
                (sal_uInt64(pData[7]) <<  0)   );

            return result;
        }

        // --------------------------------------------------------------------------

        float SAL_CALL BinaryReader_Impl::readFloat()
            throw (io::IOException, uno::RuntimeException)
        {
            union { float f; sal_uInt32 n; } result;

            sal_uInt8 const * pData = readBuffer(sizeof result.n);

            result.n = sal_uInt32(
                (sal_uInt32(pData[0]) << 24) |
                (sal_uInt32(pData[1]) << 16) |
                (sal_uInt32(pData[2]) <<  8) |
                (sal_uInt32(pData[3]) <<  0)   );

            return result.f;
        }

        // --------------------------------------------------------------------------

        double SAL_CALL BinaryReader_Impl::readDouble()
            throw (io::IOException, uno::RuntimeException)
        {
            union { double d; sal_uInt64 n; } result;

            sal_uInt8 const * pData = readBuffer(sizeof result.n);

            result.n = sal_uInt64(
                (sal_uInt64(pData[0]) << 56) |
                (sal_uInt64(pData[1]) << 48) |
                (sal_uInt64(pData[2]) << 40) |
                (sal_uInt64(pData[3]) << 32) |
                (sal_uInt64(pData[4]) << 24) |
                (sal_uInt64(pData[5]) << 16) |
                (sal_uInt64(pData[6]) <<  8) |
                (sal_uInt64(pData[7]) <<  0)   );

            return result.d;
        }

        // --------------------------------------------------------------------------

        rtl::OUString SAL_CALL BinaryReader_Impl::readUTF()
            throw (io::IOException, uno::RuntimeException)
        {
            sal_uInt32    nLength;

            sal_uInt8 const * const pData = readBuffer(sizeof nLength);

            nLength = sal_uInt32(
                (sal_uInt32(pData[0]) << 24) |
                (sal_uInt32(pData[1]) << 16) |
                (sal_uInt32(pData[2]) <<  8) |
                (sal_uInt32(pData[3]) <<  0)   );

            bool bIsAscii = (nLength & binary::STR_ASCII_MASK) == binary::STR_ASCII_MASK;
            nLength &=~binary::STR_ASCII_MASK;

            rtl::OUString result;
            if (nLength != 0)
            {
                sal_Char const * const pUTF = reinterpret_cast<sal_Char const * >(readBuffer(nLength));

                sal_Int32 const nStrLength = sal_Int32(nLength);
                OSL_ASSERT(nStrLength >= 0);

                rtl_TextEncoding const enc = bIsAscii ? RTL_TEXTENCODING_ASCII_US : RTL_TEXTENCODING_UTF8;

                rtl_uString_internConvert(&result.pData, pUTF, nStrLength, enc,
                                          OSTRING_TO_OUSTRING_CVTFLAGS, NULL);
            }

            return result;
        }

        // --------------------------------------------------------------------------
        // BinaryReader implementation.
        // --------------------------------------------------------------------------

        bool BinaryReader::open()
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            OSL_PRECOND(!m_xDataInputStream.is(),"Binary Reader: already open");
            if (m_xDataInputStream.is())
                return false;

            if (m_sFileURL.getLength() == 0)
                return false;

            if (!FileHelper::fileExists(m_sFileURL))
                return false;

            m_xDataInputStream.set(new BinaryReader_Impl (m_sFileURL));
            return true;
        }

        // --------------------------------------------------------------------------

        inline uno::Reference<io::XDataInputStream> BinaryReader::getDataInputStream()
        {
            OSL_ENSURE(m_xDataInputStream.is(),"Binary Cache: Reader was not opened - no input stream");
            return m_xDataInputStream;
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(sal_Bool &_bValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _bValue = getDataInputStream()->readBoolean();
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(sal_Int8 &_nValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _nValue = getDataInputStream()->readByte();
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(sal_Int16 &_nValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _nValue = getDataInputStream()->readShort();
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(sal_Int32 &_nValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _nValue = getDataInputStream()->readLong();
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(sal_Int64 &_nValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _nValue = getDataInputStream()->readHyper();
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(double &_nValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _nValue = getDataInputStream()->readDouble();
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read(rtl::OUString& _aStr)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            _aStr = getDataInputStream()->readUTF();
        }

        // -----------------------------------------------------------------------------
        template <class Element>
        void readSequence(BinaryReader& _rReader, uno::Sequence< Element > & aSequence)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            // PRE: the Sequence must exist
            sal_Int32 nLength;
            _rReader.read(nLength);

            aSequence.realloc(nLength);

            Element* const pElement = aSequence.getArray();  // fill the hole array
            for(sal_Int32 i=0; i<nLength; ++i)
            {
                _rReader.read(pElement[i]);                  // read one element
            }
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read (uno::Sequence< sal_Int8 > &_aValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            readSequence(*this, _aValue);
        }

        // --------------------------------------------------------------------------

        void BinaryReader::read (uno::Sequence< rtl::OUString > &_aValue)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            readSequence(*this, _aValue);
        }

        // --------------------------------------------------------------------------

        uno::Sequence< sal_Int8 > const * const for_binary = 0;

        #define CASE_READ_SEQUENCE(TYPE_CLASS, DATA_TYPE)   \
            case TYPE_CLASS:                                \
            {                                               \
                OSL_ENSURE( ::getCppuType(static_cast<DATA_TYPE const*>(0)).getTypeClass() == (TYPE_CLASS), "Typeclass does not match element type" );    \
                uno::Sequence< DATA_TYPE > aData;                \
                readSequence(_rReader, aData);                   \
                _aValue <<= aData;                               \
            }   break

        bool readSequenceValue (
            BinaryReader & _rReader,
            uno::Any          & _aValue,
            uno::Type const   & _aElementType)
            SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            switch(_aElementType.getTypeClass())
            {
            CASE_READ_SEQUENCE( uno::TypeClass_BOOLEAN, sal_Bool );

            CASE_READ_SEQUENCE( uno::TypeClass_SHORT, sal_Int16 );

            CASE_READ_SEQUENCE( uno::TypeClass_LONG, sal_Int32 );

            CASE_READ_SEQUENCE( uno::TypeClass_HYPER, sal_Int64 );

            CASE_READ_SEQUENCE( uno::TypeClass_DOUBLE, double );

            CASE_READ_SEQUENCE( uno::TypeClass_STRING, rtl::OUString );

            case uno::TypeClass_SEQUENCE:
                if (_aElementType == ::getCppuType(for_binary))
                {
                    uno::Sequence< sal_Int8 > aData;
                    readSequence(_rReader, aData);
                    _aValue <<= aData;
                    break;
                }
                // else fall through

            default:
                OSL_ENSURE(false, "Unexpected type for sequence elements");
                return false;
            }
            return true;
        }

        #undef CASE_READ_SEQUENCE

        // --------------------------------------------------------------------------


    }
}
