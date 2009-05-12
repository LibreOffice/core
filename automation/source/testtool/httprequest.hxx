/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: httprequest.hxx,v $
 * $Revision: 1.3 $
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
    USHORT nRequestPort;
    ByteString aProxyHost;
    USHORT nProxyPort;

    USHORT nStatus;
    vos::OConnectorSocket *pOutSocket;

    ByteString aHeader;
    USHORT nResultId;
    ByteString aContentType;
    SvMemoryStream* pStream;

    void SendString( vos::OStreamSocket* pSocket, ByteString aText );
    BOOL IsItem( ByteString aItem, ByteString aLine );
    void Init();
public:
    HttpRequest();
    ~HttpRequest();

    void SetRequest( ByteString aHost, ByteString aPath, USHORT nPort );
    void SetProxy( ByteString aHost, USHORT nPort );

    BOOL Execute();
    void Abort();

    ByteString GetHeader() { return aHeader; }
    SvMemoryStream* GetBody();

    ByteString GetContentType() { return aContentType; }
    USHORT GetResultId() { return nResultId; }

    USHORT GetStatus();

};

#endif
