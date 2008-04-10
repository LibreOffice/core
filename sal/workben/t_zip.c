/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: t_zip.c,v $
 * $Revision: 1.5 $
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

#include <sal/types.h>
#include <rtl/ustring.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#ifndef _OSL_ZSOCKET_H_
#include <osl/zsocket.h>
#endif

#include <stdio.h>

/*========================================================================
 *
 * internals.
 *
 *======================================================================*/
void SAL_CALL clientSocketReader (void *pData)
{
    oslSocketDescriptor *socket;

    socket = (oslSocketDescriptor*)pData;
    if (socket)
    {
        sal_Char         buffer[128];
        sal_Int32        k, n = sizeof(buffer);
        oslSocketMsgFlag flag = osl_Socket_MsgNormal;

        while ((k = osl_socket_recv (socket, buffer, n, flag)) > 0)
        {
            fwrite (buffer, 1, k, stdout);
            fflush (stdout);
        }
    }
}

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    oslSocketDescriptor *socket;

    socket = osl_socket_createSocketLayer (osl_zlib_getSocketMethods());
    if (socket)
    {
        oslSocketAddr   addr = 0;
        oslSocketResult result;

        result = osl_socket_create (
            socket,
            osl_Socket_FamilyInet,
            osl_Socket_TypeStream,
            osl_Socket_ProtocolIp);
        OSL_ASSERT(result == osl_Socket_Ok);

        if (argc > 1)
        {
            rtl_uString *host = 0;

            rtl_uString_newFromAscii (&host, argv[1]);
            addr = osl_resolveHostname (host);
            rtl_uString_release (host);
        }

        if (addr)
        {
            sal_Char         buffer[128];
            sal_Int32        k, n = sizeof(buffer);
            oslSocketMsgFlag flag = osl_Socket_MsgNormal;
            oslThread        reader;

            osl_setInetPortOfSocketAddr (addr, 7777);

            result = osl_socket_connect (socket, addr);
            OSL_ASSERT(result == osl_Socket_Ok);

            result = osl_socket_connect_handshake (socket, addr);
            osl_destroySocketAddr (addr);
            OSL_ASSERT(result == osl_Socket_Ok);

            reader = osl_createSuspendedThread (clientSocketReader, socket);
            OSL_ASSERT(reader);
            osl_resumeThread (reader);

            while ((k = fread (buffer, 1, n, stdin)) > 0)
            {
                if (osl_socket_send (socket, buffer, k, flag) < 0)
                    break;
            }

            osl_socket_shutdown (socket, osl_Socket_DirReadWrite);
            osl_socket_close (socket);

            osl_joinWithThread (reader);
            osl_destroyThread (reader);
        }
        else
        {
            oslSocketDescriptor *connection;
            oslSocketAddr        from   = 0;
            sal_Int32            option = 1;

            addr = osl_createEmptySocketAddr (osl_Socket_FamilyInet);
            osl_setInetPortOfSocketAddr (addr, 7777);

            result = osl_socket_setOption (
                socket,
                osl_Socket_LevelSocket,
                osl_Socket_OptionReuseAddr,
                &option, sizeof(option));
            OSL_ASSERT(result == osl_Socket_Ok);

            result = osl_socket_bind (socket, addr);
            osl_destroySocketAddr (addr);
            OSL_ASSERT(result == osl_Socket_Ok);

            result = osl_socket_listen (socket, 1);
            OSL_ASSERT(result == osl_Socket_Ok);

            connection = osl_socket_accept (socket, &from);
            if (connection)
            {
                sal_Char         buffer[64];
                sal_Int32        k, n = sizeof(buffer);
                oslSocketMsgFlag flag = osl_Socket_MsgNormal;

                result = osl_socket_accept_handshake (connection, from);
                osl_destroySocketAddr (from);
                OSL_ASSERT(result == osl_Socket_Ok);

                while ((k = osl_socket_recv (connection, buffer, n, flag)) > 0)
                {
                    if (osl_socket_send (connection, buffer, k, flag) < 0)
                        break;
                }

                osl_socket_close (connection);

                osl_socket_delete (connection);
                osl_socket_deleteSocketLayer (connection);
            }

            osl_socket_close (socket);
        }

        osl_socket_delete (socket);
        osl_socket_deleteSocketLayer (socket);
    }

    return 0;
}

