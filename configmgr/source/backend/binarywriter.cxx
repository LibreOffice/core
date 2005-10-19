/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: binarywriter.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-19 12:15:51 $
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

#include "binarywriter.hxx"

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
#include <com/sun/star/uno/Type.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#include "oslstream.hxx"
#endif

#ifndef _COM_SUN_STAR_IO_IOEXCEPTION_HPP_
#include <com/sun/star/io/IOException.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XDataOutputStream.hpp>
#endif

#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_BINARYTYPE_HXX
#include "binarytype.hxx"
#endif
#ifndef CONFIGMGR_SIMPLETYPEHELPER_HXX
#include "simpletypehelper.hxx"
#endif

#define ASCII(x) rtl::OUString::createFromAscii(x)
namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        using namespace ::rtl;
        using namespace ::std;
        using namespace ::com::sun::star;
        using namespace ::com::sun::star::uno;
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
