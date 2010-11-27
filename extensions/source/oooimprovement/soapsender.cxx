/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_extensions.hxx"

#include "soapsender.hxx"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <osl/socket.hxx>
#include <rtl/strbuf.hxx>
#include <boost/shared_ptr.hpp>


using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using boost::shared_ptr;
using com::sun::star::io::XTempFile;
using com::sun::star::ucb::XSimpleFileAccess;
using com::sun::star::util::URL;
using com::sun::star::util::XURLTransformer;
using osl::ConnectorSocket;
using rtl::OString;
using rtl::OUString;
using rtl::OStringBuffer;


namespace
{
    static OString getHttpPostHeader(OString path, sal_Int32 length)
    {
        OStringBuffer result =
            "POST " + path + " HTTP/1.0\r\n"
            "Content-Type: text/xml; charset=\"utf-8\"\r\n"
            "Content-Length: ";
        result.append(length);
        result.append("\r\nSOAPAction: \"\"\r\n\r\n");
        return result.makeStringAndClear();
    };
}

namespace oooimprovement
{
    SoapSender::SoapSender(const Reference<XMultiServiceFactory> sf, const OUString& url)
        : m_ServiceFactory(sf)
        , m_Url(url)
    { }

    void SoapSender::send(const SoapRequest& request) const
    {
        Reference<XTempFile> temp_file(
            m_ServiceFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.TempFile"))),
            UNO_QUERY_THROW);
        Reference<XSimpleFileAccess> file_access(
            m_ServiceFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
            UNO_QUERY_THROW);
        Reference<XURLTransformer> url_trans(
            m_ServiceFactory->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))),
            UNO_QUERY_THROW);

        // writing request to tempfile
        {
            Reference<XOutputStream> temp_stream = temp_file->getOutputStream();
            request.writeTo(temp_stream);
            temp_stream->flush();
            temp_stream->closeOutput();
        }

        // parse Url
        URL url;
        {
            url.Complete = m_Url;
            url_trans->parseStrict(url);
        }

        // connect socket
        shared_ptr<ConnectorSocket> socket(new ConnectorSocket(osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream));
        {
            ::osl::SocketAddr addr(url.Server, url.Port);
            oslSocketResult result = socket->connect(addr);
            if(result != osl_Socket_Ok)
                throw RuntimeException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("unable to connect to SOAP server")),
                    Reference<XInterface>());
        }

        // send header
        {
            OStringBuffer path_on_server =
                OUStringToOString(url.Path, RTL_TEXTENCODING_ASCII_US) +
                OUStringToOString(url.Name, RTL_TEXTENCODING_ASCII_US);
            const OString header = getHttpPostHeader(path_on_server.makeStringAndClear(), file_access->getSize(temp_file->getUri()));
            if(socket->write(header.getStr(), header.getLength()) != static_cast<sal_Int32>(header.getLength()))
                throw RuntimeException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("error while sending HTTP header")),
                    Reference<XInterface>());
        }

        // send soap request
        {
            Reference<XInputStream> temp_stream = file_access->openFileRead(temp_file->getUri());
            const sal_Int32 bufsize = 1024;
            sal_Int32 bytes_read;
            Sequence<sal_Int8> buf(bufsize);
            char buf2[bufsize];
            do
            {
                bytes_read = temp_stream->readBytes(buf, bufsize);
                buf.realloc(bytes_read);
                for(sal_Int32 idx = 0; idx < bytes_read; idx++)
                    buf2[idx] = buf[idx];
                if(socket->write(buf2, bytes_read) != bytes_read)
                    throw RuntimeException(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("error while sending SOAP request")),
                        Reference<XInterface>());
            } while(bytes_read == bufsize);
        }

        // receive answer
        {
            const sal_Int32 bufsize = 1024;
            char buf[bufsize];
            sal_Int32 bytes_read = socket->read(buf, bufsize);
            OString answer(buf, bytes_read);
            const sal_Int32 returncode_start = answer.indexOf(' ');
            if(returncode_start==-1 || !answer.copy(returncode_start, 4).equals(OString(" 200")))
                throw RuntimeException(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("SOAP server returns a error")),
                    Reference<XInterface>());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
