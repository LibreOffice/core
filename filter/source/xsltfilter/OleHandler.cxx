/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Peter Jentsch <pjotr@guineapics.de> ]
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Peter Jentsch <pjotr@guineapics.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */


#include <cstdio>
#include <cstring>
#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/variables.h>

#include <rtl/ustrbuf.hxx>

#include <sax/tools/converter.hxx>

#include <package/Inflater.hxx>
#include <package/Deflater.hxx>

#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <OleHandler.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::embed;
using namespace ::rtl;


namespace XSLT
{
    Reference<XStream> SAL_CALL OleHandler::createTempFile() {
        Reference<XStream> tempFile(
                        TempFile::create(comphelper::getComponentContext(m_msf)),
                        UNO_QUERY);
        OSL_ASSERT(tempFile.is());
        return tempFile;
    }

    void SAL_CALL OleHandler::ensureCreateRootStorage()
    {
        if (m_storage == NULL || m_rootStream == NULL)
            {
                m_rootStream = createTempFile();
                Sequence<Any> args(1);
                args[0] <<= m_rootStream->getInputStream();
                OUString serviceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.OLESimpleStorage"));

                Reference<XNameContainer> cont(m_msf->createInstanceWithArguments(serviceName, args), UNO_QUERY);
                m_storage = cont;
            }
    }

    void SAL_CALL OleHandler::initRootStorageFromBase64(const OString& content)
    {
        Sequence<sal_Int8> oleData;
        ::sax::Converter::decodeBase64(oleData, ::rtl::OStringToOUString(
            content, RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS));
        m_rootStream = createTempFile();
        Reference<XOutputStream> xOutput = m_rootStream->getOutputStream();
        xOutput->writeBytes(oleData);
        xOutput->flush();
        //Get the input stream and seek to begin
        Reference<XSeekable> xSeek(m_rootStream->getInputStream(), UNO_QUERY);
        xSeek->seek(0);

        //create an com.sun.star.embed.OLESimpleStorage from the temp stream
        Sequence<Any> args(1);
        args[0] <<= xSeek;
        Reference<XNameContainer> cont(m_msf->createInstanceWithArguments(OUString(
                RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.OLESimpleStorage" )), args), UNO_QUERY);
        m_storage = cont;
    }

    OString SAL_CALL
    OleHandler::encodeSubStorage(const OUString& streamName)
    {
        if (!m_storage->hasByName(streamName))
            {
                return "Not Found:";// + streamName;
            }
        ;
        Reference<XInputStream> subStream((*(Reference< XInterface > *) m_storage->getByName(streamName).getValue()), UNO_QUERY);
        if (!subStream.is())
            {
                return "Not Found:";// + streamName;
            }
        //The first four byte are the length of the uncompressed data
        Sequence<sal_Int8> pLength(4);
        Reference<XSeekable> xSeek(subStream, UNO_QUERY);
        xSeek->seek(0);
        //Get the uncompressed length
        int readbytes = subStream->readBytes(pLength, 4);
        if (4 != readbytes)
            {
                return "Can not read the length.";
            }
        int oleLength = (pLength[0] << 0) + (pLength[1] << 8)
                + (pLength[2] << 16) + (pLength[3] << 24);
        Sequence<sal_Int8> content(oleLength);
        //Read all bytes. The compressed length should less then the uncompressed length
        readbytes = subStream->readBytes(content, oleLength);
        if (oleLength < readbytes)
            {
                return "oleLength";// +oleLength + readBytes;
            }

        // Decompress the bytes
        ::ZipUtils::Inflater* decompresser = new ::ZipUtils::Inflater(sal_False);
        decompresser->setInput(content);
        Sequence<sal_Int8> result(oleLength);
        decompresser->doInflateSegment(result, 0, oleLength);
        decompresser->end();
        delete decompresser;
        //return the base64 string of the uncompressed data
        OUStringBuffer buf(oleLength);
        ::sax::Converter::encodeBase64(buf, result);
        return ::rtl::OUStringToOString(buf.toString(), RTL_TEXTENCODING_UTF8);
    }

    void SAL_CALL
    OleHandler::insertByName(const OUString& streamName, const OString& content)
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

    const OString
    SAL_CALL OleHandler::getByName(const OUString& streamName)
    {
        if ( streamName == "oledata.mso" )
        {
            //get the length and seek to 0
            Reference<XSeekable> xSeek (m_rootStream, UNO_QUERY);
            int oleLength = (int) xSeek->getLength();
            xSeek->seek(0);
            //read all bytes
            Reference<XInputStream> xInput = m_rootStream->getInputStream();
            Sequence<sal_Int8> oledata(oleLength);
            xInput->readBytes(oledata, oleLength);
            //return the base64 encoded string
            OUStringBuffer buf(oleLength);
            ::sax::Converter::encodeBase64(buf, oledata);
            return ::rtl::OUStringToOString(buf.toString(), RTL_TEXTENCODING_UTF8);
        }
        return encodeSubStorage(streamName);
    }

    void SAL_CALL
    OleHandler::insertSubStorage(const OUString& streamName, const OString& content)
    {
        //decode the base64 string
        Sequence<sal_Int8> oledata;
        ::sax::Converter::decodeBase64(oledata,
                rtl::OStringToOUString(content, RTL_TEXTENCODING_ASCII_US));
        //create a temp stream to write data to
        Reference<XStream> subStream = createTempFile();
        Reference<XInputStream> xInput = subStream->getInputStream();
        Reference<XOutputStream> xOutput = subStream->getOutputStream();
        //write the length to the temp stream
        Sequence<sal_Int8> header(4);
        header[0] = (sal_Int8) (oledata.getLength() >> 0) & 0xFF;
        header[1] = (sal_Int8) (oledata.getLength() >> 8) & 0xFF;
        header[2] = (sal_Int8) (oledata.getLength() >> 16) & 0xFF;
        header[3] = (sal_Int8) (oledata.getLength() >> 24) & 0xFF;
        xOutput->writeBytes(header);

        // Compress the bytes
        Sequence<sal_Int8> output(oledata.getLength());
        ::ZipUtils::Deflater* compresser = new ::ZipUtils::Deflater((sal_Int32) 3, sal_False);
        compresser->setInputSegment(oledata, 0, oledata.getLength());
        compresser->finish();
        int compressedDataLength = compresser->doDeflateSegment(output, 0, oledata.getLength());
        delete(compresser);
        //realloc the data length
        Sequence<sal_Int8> compressed(compressedDataLength);
        for (int i = 0; i < compressedDataLength; i++) {
            compressed[i] = output[i];
        }

        //write the compressed data to the temp stream
        xOutput->writeBytes(compressed);
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

