/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.io.TempFile")),
            UNO_QUERY_THROW);
        Reference<XSimpleFileAccess> file_access(
            m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.ucb.SimpleFileAccess")),
            UNO_QUERY_THROW);
        Reference<XURLTransformer> url_trans(
            m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.util.URLTransformer")),
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
                    OUString::createFromAscii("unable to connect to SOAP server"),
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
                    OUString::createFromAscii("error while sending HTTP header"),
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
                        OUString::createFromAscii("error while sending SOAP request"),
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
                    OUString::createFromAscii("SOAP server returns a error"),
                    Reference<XInterface>());
        }
    }
}
