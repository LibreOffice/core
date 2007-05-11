/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: httprequest.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-11 08:55:28 $
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

#ifndef _HTTPREQUEST_HXX_
#define _HTTPREQUEST_HXX_

#define HTTP_INIT               0
#define HTTP_REQUEST_SET        1
#define HTTP_REQUEST_PENDING    2
#define HTTP_REQUEST_DONE       3
#define HTTP_REQUEST_ERROR      4

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

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
