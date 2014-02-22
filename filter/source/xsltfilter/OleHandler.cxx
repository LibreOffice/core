/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
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
        Reference<XStream> tempFile( TempFile::create(m_xContext), UNO_QUERY);
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

                Reference<XNameContainer> cont(
                     Reference<XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW)
                         ->createInstanceWithArguments("com.sun.star.embed.OLESimpleStorage", args), UNO_QUERY);
                m_storage = cont;
            }
    }

    void SAL_CALL OleHandler::initRootStorageFromBase64(const OString& content)
    {
        Sequence<sal_Int8> oleData;
        ::sax::Converter::decodeBase64(oleData, OStringToOUString(
            content, RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS));
        m_rootStream = createTempFile();
        Reference<XOutputStream> xOutput = m_rootStream->getOutputStream();
        xOutput->writeBytes(oleData);
        xOutput->flush();
        
        Reference<XSeekable> xSeek(m_rootStream->getInputStream(), UNO_QUERY);
        xSeek->seek(0);

        
        Sequence<Any> args(1);
        args[0] <<= xSeek;
        Reference<XNameContainer> cont(
             Reference<XMultiServiceFactory>(m_xContext->getServiceManager(), UNO_QUERY_THROW)
                 ->createInstanceWithArguments("com.sun.star.embed.OLESimpleStorage", args), UNO_QUERY);
        m_storage = cont;
    }

    OString SAL_CALL
    OleHandler::encodeSubStorage(const OUString& streamName)
    {
        if (!m_storage->hasByName(streamName))
            {
                return "Not Found:";
            }
        ;
        Reference<XInputStream> subStream((*(Reference< XInterface > *) m_storage->getByName(streamName).getValue()), UNO_QUERY);
        if (!subStream.is())
            {
                return "Not Found:";
            }
        
        Sequence<sal_Int8> pLength(4);
        Reference<XSeekable> xSeek(subStream, UNO_QUERY);
        xSeek->seek(0);
        
        int readbytes = subStream->readBytes(pLength, 4);
        if (4 != readbytes)
            {
                return "Can not read the length.";
            }
        int oleLength = (pLength[0] << 0) + (pLength[1] << 8)
                + (pLength[2] << 16) + (pLength[3] << 24);
        Sequence<sal_Int8> content(oleLength);
        
        readbytes = subStream->readBytes(content, oleLength);
        if (oleLength < readbytes)
            {
                return "oleLength";
            }

        
        ::ZipUtils::Inflater* decompresser = new ::ZipUtils::Inflater(sal_False);
        decompresser->setInput(content);
        Sequence<sal_Int8> result(oleLength);
        decompresser->doInflateSegment(result, 0, oleLength);
        decompresser->end();
        delete decompresser;
        
        OUStringBuffer buf(oleLength);
        ::sax::Converter::encodeBase64(buf, result);
        return OUStringToOString(buf.toString(), RTL_TEXTENCODING_UTF8);
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
            
            Reference<XSeekable> xSeek (m_rootStream, UNO_QUERY);
            int oleLength = (int) xSeek->getLength();
            xSeek->seek(0);
            
            Reference<XInputStream> xInput = m_rootStream->getInputStream();
            Sequence<sal_Int8> oledata(oleLength);
            xInput->readBytes(oledata, oleLength);
            
            OUStringBuffer buf(oleLength);
            ::sax::Converter::encodeBase64(buf, oledata);
            return OUStringToOString(buf.toString(), RTL_TEXTENCODING_UTF8);
        }
        return encodeSubStorage(streamName);
    }

    void SAL_CALL
    OleHandler::insertSubStorage(const OUString& streamName, const OString& content)
    {
        
        Sequence<sal_Int8> oledata;
        ::sax::Converter::decodeBase64(oledata,
                OStringToOUString(content, RTL_TEXTENCODING_ASCII_US));
        
        Reference<XStream> subStream = createTempFile();
        Reference<XInputStream> xInput = subStream->getInputStream();
        Reference<XOutputStream> xOutput = subStream->getOutputStream();
        
        Sequence<sal_Int8> header(4);
        header[0] = (sal_Int8) (oledata.getLength() >> 0) & 0xFF;
        header[1] = (sal_Int8) (oledata.getLength() >> 8) & 0xFF;
        header[2] = (sal_Int8) (oledata.getLength() >> 16) & 0xFF;
        header[3] = (sal_Int8) (oledata.getLength() >> 24) & 0xFF;
        xOutput->writeBytes(header);

        
        Sequence<sal_Int8> output(oledata.getLength());
        ::ZipUtils::Deflater* compresser = new ::ZipUtils::Deflater((sal_Int32) 3, sal_False);
        compresser->setInputSegment(oledata, 0, oledata.getLength());
        compresser->finish();
        int compressedDataLength = compresser->doDeflateSegment(output, 0, oledata.getLength());
        delete(compresser);
        
        Sequence<sal_Int8> compressed(compressedDataLength);
        for (int i = 0; i < compressedDataLength; i++) {
            compressed[i] = output[i];
        }

        
        xOutput->writeBytes(compressed);
        
        Reference<XSeekable> xSeek(xInput, UNO_QUERY);
        xSeek->seek(0);

        
        Reference<XTransactedObject> xTransact(m_storage, UNO_QUERY);
        Any entry;
        entry <<= xInput;
        m_storage->insertByName(streamName, entry);
        xTransact->commit();
    }



}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

