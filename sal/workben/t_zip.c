/*************************************************************************
 *
 *  $RCSfile: t_zip.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
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

