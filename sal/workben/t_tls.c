/*************************************************************************
 *
 *  $RCSfile: t_tls.c,v $
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
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif
#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_ISOCKET_H_
#include <osl/isocket.h>
#endif

#ifndef _OSL_TLS_H_
#include <osl/tls.h>
#endif

#include <string.h>
#include <stdio.h>

/*========================================================================
 *
 * Internals.
 *
 *======================================================================*/
void SAL_CALL __rtl_sequence_release (sal_Sequence *value)
{
    if (value)
    {
        if (osl_decrementInterlockedCount (&(value->nRefCount)) == 0)
            rtl_freeMemory (value);
    }
}

void SAL_CALL __rtl_sequence_new (sal_Sequence **value, sal_Int32 length)
{
    if (value)
    {
        __rtl_sequence_release (*value);

        *value = (sal_Sequence*)rtl_allocateMemory (
            SAL_SEQUENCE_HEADER_SIZE + length);

        (*value)->nRefCount = 1;
        (*value)->nElements = 0;
    }
}

void load_sequence (sal_Sequence **ppSequence, const sal_Char *name)
{
    oslFileError  result;
    rtl_uString  *pName, *pPath;

    if (ppSequence)
    {
        __rtl_sequence_release (*ppSequence);
        *ppSequence = NULL;
    }

    pName = NULL;
    rtl_uString_newFromAscii (&pName, name);

    pPath = NULL;
    result = osl_searchNormalizedPath (pName, NULL, &pPath);
    if (result == osl_File_E_None)
    {
        oslFileHandle hFile;

        result = osl_openFile (pPath, &hFile, osl_File_OpenFlag_Read);
        if (result == osl_File_E_None)
        {
            sal_uInt64 nSize = 0, nRead = 0;

            osl_setFilePos (hFile, osl_Pos_End, 0);
            osl_getFilePos (hFile, &nSize);
            osl_setFilePos (hFile, osl_Pos_Absolut, 0);

            __rtl_sequence_new (ppSequence, (sal_Int32)nSize);

            osl_readFile (hFile, (*ppSequence)->elements, nSize, &nRead);
            (*ppSequence)->nElements = (sal_Int32)nRead;
        }
        osl_closeFile (hFile);

        rtl_uString_release (pPath);
    }

    rtl_uString_release (pName);
}

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    static const sal_Char p[] = "GET / HTTP/1.0\015\012\015\012";
    oslSocketDescriptor  *socket;


    socket = osl_socket_createSocketLayer (osl_tls_getSocketMethods());
    if (socket)
    {
        oslSocketAddr     target;
        oslSocketResult   result;
        oslSocketMsgFlag  flag   = osl_Socket_MsgNormal;
        rtl_uString      *dotted = NULL;


        rtl_uString_newFromAscii (&dotted, argv[1]);
        target = osl_createInetSocketAddr (dotted, 443);
        rtl_uString_release (dotted);

        result = osl_socket_create (
            socket,
            osl_Socket_FamilyInet,
            osl_Socket_TypeStream,
            osl_Socket_ProtocolIp);
        OSL_ASSERT(result == osl_Socket_Ok);

        result = osl_socket_connect (socket, target);
        osl_destroySocketAddr (target);
        OSL_ASSERT(result == osl_Socket_Ok);

        result = osl_socket_connect_handshake (socket, target);
        OSL_ASSERT(result == osl_Socket_Ok);

        if (osl_socket_send (socket, p, strlen(p), flag) > 0)
        {
            sal_Char  buffer[1024];
            sal_Int32 k, n = sizeof(buffer);

            while ((k = osl_socket_recv (socket, buffer, n, flag)) > 0)
                fwrite (buffer, 1, k, stdout);
        }

        osl_socket_shutdown (socket, osl_Socket_DirReadWrite);
        osl_socket_close (socket);

        osl_socket_delete (socket);
        osl_socket_deleteSocketLayer (socket);
    }


    socket = osl_socket_createSocketLayer (osl_tls_getSocketMethods());
    if (socket)
    {
        oslSocketDescriptor *connection;
        oslSocketResult      result;
        oslSocketAddr        addr, from;
        sal_Int32            option = 1;
        sal_Sequence        *data = NULL;


        result = osl_socket_create (
            socket,
            osl_Socket_FamilyInet,
            osl_Socket_TypeStream,
            osl_Socket_ProtocolIp);
        OSL_ASSERT(result == osl_Socket_Ok);

        result = osl_socket_setOption (
            socket,
            osl_Socket_LevelSocket,
            osl_Socket_OptionReuseAddr,
            &option, sizeof(option));
        OSL_ASSERT(result == osl_Socket_Ok);

        addr = osl_createEmptySocketAddr (osl_Socket_FamilyInet);
        osl_setInetPortOfSocketAddr (addr, 7777);

        result = osl_socket_bind (socket, addr);
        OSL_ASSERT(result == osl_Socket_Ok);

        load_sequence (&data, "s_cert.der");
        if (data && data->nElements)
        {
            result = osl_socket_setOption (
                socket,
                osl_Socket_LevelTLS,
                osl_Socket_OptionCertificate,
                data->elements, data->nElements);
            OSL_ASSERT(result == osl_Socket_Ok);
        }

        load_sequence (&data, "s_pkey.der");
        if (data && data->nElements)
        {
            result = osl_socket_setOption (
                socket,
                osl_Socket_LevelTLS,
                osl_Socket_OptionPrivateKey,
                data->elements, data->nElements);
            OSL_ASSERT(result == osl_Socket_Ok);
        }
        __rtl_sequence_release (data);

        result = osl_socket_listen (socket, 1);
        OSL_ASSERT(result == osl_Socket_Ok);

        connection = osl_socket_accept (socket, &from);
        if (connection)
        {
            sal_Char  buffer[1024];
            sal_Int32 k, n = sizeof(buffer);
            oslSocketMsgFlag flag = osl_Socket_MsgNormal;

            result = osl_socket_accept_handshake (connection, from);
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

        osl_socket_delete (socket);
        osl_socket_deleteSocketLayer (socket);
    }

    return 0;
}

