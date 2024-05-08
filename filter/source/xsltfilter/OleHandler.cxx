/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <rtl/ustrbuf.hxx>

#include <package/Inflater.hxx>
#include <package/Deflater.hxx>

#include <cppuhelper/factory.hxx>
#include <comphelper/base64.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "OleHandler.hxx"
#include <optional>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::embed;


namespace XSLT
{
    Reference<XStream> OleHandler::createTempFile() {
        Reference<XStream> tempFile = TempFile::create(m_xContext);
        OSL_ASSERT(tempFile.is());
        return tempFile;
    }

    void OleHandler::ensureCreateRootStorage()
    {
        if (m_storage == nullptr || m_rootStream == nullptr)
        {
            m_rootStream = createTempFile();
            Sequence<Any> args{ Any(m_rootStream->getInputStream()) };

            m_storage.set(
                 Reference<XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW)
                     ->createInstanceWithArguments(u"com.sun.star.embed.OLESimpleStorage"_ustr, args), UNO_QUERY);
        }
    }

    void OleHandler::initRootStorageFromBase64(std::string_view content)
    {
        Sequence<sal_Int8> oleData;
        ::comphelper::Base64::decode(oleData, OStringToOUString(
            content, RTL_TEXTENCODING_UTF8));
        m_rootStream = createTempFile();
        Reference<XOutputStream> xOutput = m_rootStream->getOutputStream();
        xOutput->writeBytes(oleData);
        xOutput->flush();
        //Get the input stream and seek to begin
        Reference<XSeekable> xSeek(m_rootStream->getInputStream(), UNO_QUERY);
        xSeek->seek(0);

        //create a com.sun.star.embed.OLESimpleStorage from the temp stream
        Sequence<Any> args{ Any(xSeek) };
        m_storage.set(
             Reference<XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW)
                 ->createInstanceWithArguments(u"com.sun.star.embed.OLESimpleStorage"_ustr, args), UNO_QUERY);
    }

    OString
    OleHandler::encodeSubStorage(const OUString& streamName)
    {
        if (!m_storage || !m_storage->hasByName(streamName))
        {
            return "Not Found:"_ostr;// + streamName;
        }

        Reference<XInputStream> subStream(m_storage->getByName(streamName), UNO_QUERY);
        if (!subStream.is())
        {
            return "Not Found:"_ostr;// + streamName;
        }
        //The first four byte are the length of the uncompressed data
        Sequence<sal_Int8> aLength(4);
        Reference<XSeekable> xSeek(subStream, UNO_QUERY);
        xSeek->seek(0);
        //Get the uncompressed length
        int readbytes = subStream->readBytes(aLength, 4);
        if (4 != readbytes)
        {
            return "Can not read the length."_ostr;
        }
        sal_Int32 const oleLength = (static_cast<sal_uInt8>(aLength[0]) <<  0U)
                                  | (static_cast<sal_uInt8>(aLength[1]) <<  8U)
                                  | (static_cast<sal_uInt8>(aLength[2]) << 16U)
                                  | (static_cast<sal_uInt8>(aLength[3]) << 24U);
        if (oleLength < 0)
        {
            return "invalid oleLength"_ostr;
        }
        Sequence<sal_Int8> content(oleLength);
        //Read all bytes. The compressed length should be less than the uncompressed length
        readbytes = subStream->readBytes(content, oleLength);
        if (oleLength < readbytes)
        {
            return "oleLength"_ostr;// +oleLength + readBytes;
        }

        // Decompress the bytes
        std::optional< ::ZipUtils::Inflater> decompresser(std::in_place, false);
        decompresser->setInput(content);
        Sequence<sal_Int8> result(oleLength);
        decompresser->doInflateSegment(result, 0, oleLength);
        decompresser->end();
        decompresser.reset();
        //return the base64 string of the uncompressed data
        OUStringBuffer buf(oleLength);
        ::comphelper::Base64::encode(buf, result);
        return OUStringToOString(buf, RTL_TEXTENCODING_ASCII_US);
    }

    void
    OleHandler::insertByName(const OUString& streamName, std::string_view content)
    {
        if ( streamName == "oledata.mso" )
        {
            initRootStorageFromBase64(content);
        }
        else
        {
            ensureCreateRootStorage();
            insertSubStorage(streamName, content);
        }
    }

    OString
    OleHandler::getByName(const OUString& streamName)
    {
        if ( streamName == "oledata.mso" )
        {
            //get the length and seek to 0
            Reference<XSeekable> xSeek (m_rootStream, UNO_QUERY);
            int oleLength = static_cast<int>(xSeek->getLength());
            xSeek->seek(0);
            //read all bytes
            Reference<XInputStream> xInput = m_rootStream->getInputStream();
            Sequence<sal_Int8> oledata(oleLength);
            xInput->readBytes(oledata, oleLength);
            //return the base64 encoded string
            OUStringBuffer buf(oleLength);
            ::comphelper::Base64::encode(buf, oledata);
            return OUStringToOString(buf, RTL_TEXTENCODING_ASCII_US);
        }
        return encodeSubStorage(streamName);
    }

    void
    OleHandler::insertSubStorage(const OUString& streamName, std::string_view content)
    {
        //decode the base64 string
        Sequence<sal_Int8> oledata;
        ::comphelper::Base64::decode(oledata,
                OStringToOUString(content, RTL_TEXTENCODING_ASCII_US));
        //create a temp stream to write data to
        Reference<XStream> subStream = createTempFile();
        Reference<XInputStream> xInput = subStream->getInputStream();
        Reference<XOutputStream> xOutput = subStream->getOutputStream();
        //write the length to the temp stream
        Sequence<sal_Int8> header{
            static_cast<sal_Int8>((oledata.getLength() >> 0) & 0xFF),
            static_cast<sal_Int8>((oledata.getLength() >> 8) & 0xFF),
            static_cast<sal_Int8>((oledata.getLength() >> 16) & 0xFF),
            static_cast<sal_Int8>((oledata.getLength() >> 24) & 0xFF)
        };
        xOutput->writeBytes(header);

        // Compress the bytes
        Sequence<sal_Int8> output(oledata.getLength());
        std::optional< ::ZipUtils::Deflater> compresser(std::in_place, sal_Int32(3), false);
        compresser->setInputSegment(oledata);
        compresser->finish();
        int compressedDataLength = compresser->doDeflateSegment(output, oledata.getLength());
        compresser.reset();
        //realloc the data length
        output.realloc(compressedDataLength);

        //write the compressed data to the temp stream
        xOutput->writeBytes(output);
        //seek to 0
        Reference<XSeekable> xSeek(xInput, UNO_QUERY);
        xSeek->seek(0);

        //insert the temp stream as a sub stream and use an XTransactedObject to commit it immediately
        Reference<XTransactedObject> xTransact(m_storage, UNO_QUERY);
        Any entry;
        entry <<= xInput;
        m_storage->insertByName(streamName, entry);
        xTransact->commit();
    }


}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

