/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/main.h>
#include <rtl/ustring.h>
#include <osl/thread.h>
#include <osl/conditn.h>
#include <osl/socket.h>

#include <cstdio>

oslThread serverThread;
oslCondition serverReady;

void server(void*);
void client();

SAL_IMPLEMENT_MAIN()
{
    fprintf(stdout, "Demonstrates sockets.\n");

    serverReady = osl_createCondition();
    serverThread = osl_createThread(server, nullptr);
    osl_waitCondition(serverReady, nullptr);
    client();
    osl_joinWithThread(serverThread);

    return 0;
}

void client()
{
    oslSocket socket = osl_createSocket(osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp);

    rtl_uString *pstrLocalHostAddr = nullptr;
    rtl_string2UString(&pstrLocalHostAddr, "127.0.0.1", 9, osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);

    // create high socket on localhost address
    oslSocketAddr addr = osl_createInetSocketAddr(pstrLocalHostAddr, 30000);

    if (osl_connectSocketTo(socket, addr, nullptr) != osl_Socket_Ok)
    {
        fprintf(stderr, "**Client**    Could not bind address to socket.\n");
        exit(1);
    }

    char sendBuffer = 'c';
    sal_Int32 nSentChar = osl_sendSocket(socket, &sendBuffer, 1, osl_Socket_MsgNormal);
    fprintf(stdout, "**Client**    Sent %d character.\n", nSentChar);
}

void server(void* /* pData */)
{
    oslSocket socket = osl_createSocket(osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp);

    rtl_uString *pstrLocalHostAddr = nullptr;
    rtl_string2UString(&pstrLocalHostAddr, "127.0.0.1", 9, osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);

    fprintf(stdout, "**Server**    Listening on localhost...\n");
    // create high socket on localhost address
    fprintf(stdout, "**Server**    Create socket\n");
    oslSocketAddr addr = osl_createInetSocketAddr(pstrLocalHostAddr, 30000);

    fprintf(stdout, "**Server**    Bind address to socket\n");
    if (osl_bindAddrToSocket(socket, addr) == sal_False)
    {
        fprintf(stderr, "Could not bind address to socket.\n");
        exit(1);
    }

    fprintf(stdout, "**Server**    Listen on socket...\n");
    if (osl_listenOnSocket(socket, -1) == sal_False)
    {
        fprintf(stderr, "**Client** Could not listen on socket.\n");
        exit(1);
    }

    osl_setCondition(serverReady);

    fprintf(stdout, "**Server**    Accept connection...\n");
    oslSocket inboundSocket = osl_acceptConnectionOnSocket(socket, &addr);

    fprintf(stdout, "**Server**    Receive data...\n");
    char buffer;
    osl_receiveSocket(inboundSocket, &buffer, 1, osl_Socket_MsgNormal);

    fprintf(stdout, "**Server**    Received character %c\n", buffer);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
