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



#ifndef _HTTPREQUEST_HXX_
#define _HTTPREQUEST_HXX_

#define HTTP_INIT               0
#define HTTP_REQUEST_SET        1
#define HTTP_REQUEST_PENDING    2
#define HTTP_REQUEST_DONE       3
#define HTTP_REQUEST_ERROR      4

#include <tools/string.hxx>
#include <tools/stream.hxx>

namespace vos
{
    class OStreamSocket;
    class OConnectorSocket;
}

class HttpRequest
{
    ByteString aRequestPath;
    ByteString aRequestHost;
    sal_uInt16 nRequestPort;
    ByteString aProxyHost;
    sal_uInt16 nProxyPort;

    sal_uInt16 nStatus;
    vos::OConnectorSocket *pOutSocket;

    ByteString aHeader;
    sal_uInt16 nResultId;
    ByteString aContentType;
    SvMemoryStream* pStream;

    void SendString( vos::OStreamSocket* pSocket, ByteString aText );
    sal_Bool IsItem( ByteString aItem, ByteString aLine );
    void Init();
public:
    HttpRequest();
    ~HttpRequest();

    void SetRequest( ByteString aHost, ByteString aPath, sal_uInt16 nPort );
    void SetProxy( ByteString aHost, sal_uInt16 nPort );

    sal_Bool Execute();
    void Abort();

    ByteString GetHeader() { return aHeader; }
    SvMemoryStream* GetBody();

    ByteString GetContentType() { return aContentType; }
    sal_uInt16 GetResultId() { return nResultId; }

    sal_uInt16 GetStatus();

};

#endif
