/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binarywriter.cxx,v $
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

#include "binarywriter.hxx"
#include "valuenode.hxx"
#include "filehelper.hxx"
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Any.hxx>
#include "oslstream.hxx"
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XDataOutputStream.hpp>
#include "typeconverter.hxx"
#include "binarytype.hxx"
#include "simpletypehelper.hxx"

#define ASCII(x) rtl::OUString::createFromAscii(x)
namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace uno = com::sun::star::uno;
        namespace io = com::sun::star::io;

        BinaryWriter::BinaryWriter(rtl::OUString const &_aFileURL, uno::Reference<lang::XMultiServiceFactory> const& _xServiceProvider)
        : m_aFileURL(_aFileURL)
        , m_xServiceProvider(_xServiceProvider)
        , m_xDataOutputStream()
        {}

        bool BinaryWriter::open() SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            if (m_aFileURL.getLength() == 0)
                return false;

            OSL_ENSURE(!m_xDataOutputStream.is(), "Binary Writer: already open");
            if ( m_xDataOutputStream.is())
                return false;

            if (FileHelper::fileExists(m_aFileURL))
            {
                if (osl::File::RC errorCode = osl::File::remove(m_aFileURL))
                {
                    // creating the file will fail later
                    OSL_TRACE("Binary Cache: Cannot remove existing file [%d]",int(errorCode));
                }
            }
            else
            {
                //create missing directories
                rtl::OUString parentDirectory = FileHelper::getParentDir(m_aFileURL) ;

                if (osl::File::RC errorCode = FileHelper::mkdirs(parentDirectory))
                {
                    // creating the file will fail later
                    OSL_TRACE("Binary Cache: Cannot create package cache directory [%d]",int(errorCode));
                }
            }

            uno::Reference<io::XOutputStream> xOutput = new BufferedFileOutputStream(m_aFileURL, true, 1024);

            uno::Reference< io::XActiveDataSource > xFormattingStream(
                    m_xServiceProvider->createInstance(ASCII("com.sun.star.io.DataOutputStream")),
                    uno::UNO_QUERY_THROW);

            xFormattingStream->setOutputStream(xOutput);

            m_xDataOutputStream.set(xFormattingStream, uno::UNO_QUERY_THROW);

            OSL_ASSERT(m_xDataOutputStream.is());
            return m_xDataOutputStream.is();
        }

        void BinaryWriter::close() SAL_THROW( (io::IOException, uno::RuntimeException) )
        {
            if (m_xDataOutputStream.is())
                m_xDataOutputStream->closeOutput();

            m_xDataOutputStream.clear();
        }

        BinaryWriter::~BinaryWriter()
        {
            try
            {
                close();
            }
            catch (uno::Exception& e)
            {
                            (void)e;
                OSL_ENSURE(false, rtl::OUStringToOString(e.Message,RTL_TEXTENCODING_ASCII_US).getStr());
            }
        }

    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    void BinaryWriter::write(sal_Bool _aValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // write one byte
        m_xDataOutputStream->writeBoolean(_aValue);
    }
    void BinaryWriter::write(sal_Int8 _aValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // write one byte
        m_xDataOutputStream->writeByte(_aValue);
    }
    void BinaryWriter::write(sal_Int16 _aValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // write two bytes
        m_xDataOutputStream->writeShort(_aValue);
    }
    void BinaryWriter::write(sal_Int32 _aValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // write four byte
        m_xDataOutputStream->writeLong(_aValue);
    }
    void BinaryWriter::write(sal_Int64 _aValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // write eight byte
        m_xDataOutputStream->writeHyper(_aValue);
    }
    void BinaryWriter::write(double _aValue)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // write eight byte
        m_xDataOutputStream->writeDouble(_aValue);
    }

    // -----------------------------------------------------------------------------
    bool isAsciiEncoding(rtl::OUString const& _aStr)
    {
        const sal_Unicode *pStr = _aStr.getStr();
        sal_Int32 nLen = _aStr.getLength();
        while (nLen--)
        {
            if (*pStr++ > 127)
                return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    void BinaryWriter::write(rtl::OUString const& _aStr)
        SAL_THROW( (io::IOException, uno::RuntimeException) )
    {
        // @@@ OBinaryBaseReader_Impl::readUTF() @@@

        rtl::OString aUTF;
        // to fasten the conversion for ascii data, we mask the length
        bool bIsAscii = isAsciiEncoding(_aStr);
        if (bIsAscii)
            rtl_uString2String (
                &(aUTF.pData), _aStr.getStr(), _aStr.getLength(),
                RTL_TEXTENCODING_ASCII_US, OUSTRING_TO_OSTRING_CVTFLAGS);
        else
            rtl_uString2String (
                &(aUTF.pData), _aStr.getStr(), _aStr.getLength(),
                RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        sal_Int32 nLength = aUTF.getLength();
        uno::Sequence<sal_Int8> aData (nLength);
        memcpy (aData.getArray(), aUTF.getStr(), nLength);

        OSL_ENSURE((nLength & binary::STR_ASCII_MASK) == 0,"String too long");
        if (bIsAscii)
        {
            nLength |= binary::STR_ASCII_MASK;
            OSL_ASSERT((nLength &  binary::STR_ASCII_MASK) == binary::STR_ASCII_MASK);
            OSL_ASSERT(sal_Int32(nLength & ~binary::STR_ASCII_MASK) == aData.getLength());
        }
        m_xDataOutputStream->writeLong (nLength);

        m_xDataOutputStream->writeBytes (aData);
    }

    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
   }
// -----------------------------------------------------------------------------
}
