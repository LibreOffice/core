/*************************************************************************
 *
 *  $RCSfile: t_layer.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:30 $
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
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_ISOCKET_H_
#include <osl/isocket.h>
#endif

/*========================================================================
 *
 * osl/debug.c
 *
 *======================================================================*/
/*
 * __osl_debug_create.
 */
static oslSocketResult SAL_CALL __osl_debug_create (
    oslSocketDescriptor *socket,
    oslAddrFamily        family,
    oslSocketType        type,
    oslProtocol          protocol)
{
    oslSocketResult result;

    if (!(socket->m_lower))
    {
        oslSocketDescriptor *lower;
        lower = osl_socket_createSocketLayer (osl_socket_getSocketMethods());
        osl_socket_pushSocketLayer (socket, lower);
    }

    result = (socket->m_lower->m_methods->m_create)(
        socket->m_lower, family, type, protocol);
    if (!(result == osl_Socket_Ok))
        return (result);

    if (!(socket->m_private))
    {
        /* ... */
    }
    return (osl_Socket_Ok);
}

/*
 * __osl_debug_delete.
 */
static void SAL_CALL __osl_debug_delete (oslSocketDescriptor *socket)
{
    if (socket->m_private)
    {
        /* ... */
    }
    if (socket->m_lower)
    {
        (socket->m_lower->m_methods->m_delete)(socket->m_lower);
        osl_socket_deleteSocketLayer (socket->m_lower);
    }
}

/*
 * __osl_debug_getOption.
 */
static oslSocketResult SAL_CALL __osl_debug_getOption (
    oslSocketDescriptor  *socket,
    oslSocketOptionLevel  level,
    oslSocketOption       option,
    void                 *buffer,
    sal_uInt32            length)
{
    if (level == osl_Socket_LevelSocketLayer)
    {
        /* ... */
        return (osl_Socket_Ok);
    }

    return (socket->m_lower->m_methods->m_getoption)(
        socket->m_lower, level, option, buffer, length);
}

/*
 * __osl_debug_setOption.
 */
static oslSocketResult SAL_CALL __osl_debug_setOption (
    oslSocketDescriptor  *socket,
    oslSocketOptionLevel  level,
    oslSocketOption       option,
    const void           *buffer,
    sal_uInt32            length)
{
    if (level == osl_Socket_LevelSocketLayer)
    {
        /* ... */
        return (osl_Socket_Ok);
    }

    return (socket->m_lower->m_methods->m_setoption)(
        socket->m_lower, level, option, buffer, length);
}

/*
 * __osl_debug_accept.
 */
static oslSocketDescriptor* SAL_CALL __osl_debug_accept (
    oslSocketDescriptor *socket, oslSocketAddr *fromaddr)
{
    oslSocketDescriptor *lower;
    lower = (socket->m_lower->m_methods->m_accept)(socket->m_lower, fromaddr);
    if (lower)
    {
        oslSocketDescriptor *upper;
        upper = osl_socket_createSocketLayer (socket->m_methods);
        osl_socket_pushSocketLayer (upper, lower);
        return (upper);
    }
    return ((oslSocketDescriptor*)NULL);
}

/*
 * __osl_debug_connect.
 */
static oslSocketResult SAL_CALL __osl_debug_connect (
    oslSocketDescriptor *socket, oslSocketAddr addr)
{
    return (socket->m_lower->m_methods->m_connect)(socket->m_lower, addr);
}

/*
 * __osl_debug_close.
 */
static void SAL_CALL __osl_debug_close (oslSocketDescriptor *socket)
{
    (socket->m_lower->m_methods->m_close)(socket->m_lower);
}

/*
 * __osl_shutdown.
 */
static void SAL_CALL __osl_debug_shutdown (
    oslSocketDescriptor *socket, oslSocketDirection direction)
{
    (socket->m_lower->m_methods->m_shutdown)(socket->m_lower, direction);
}

/*
 * __osl_debug_methods.
 */
static oslSocketMethods __osl_debug_methods =
{
    NULL /* initialized on demand */
};

/*
 * osl_debug_getSocketMethods.
 */
const oslSocketMethods* SAL_CALL osl_debug_getSocketMethods (void)
{
    static oslInterlockedCount nonce = 0;
    if (osl_incrementInterlockedCount (&nonce) == 1)
    {
        const oslSocketMethods *methods = osl_socket_getDefaultSocketMethods();
        __osl_debug_methods = *methods;

        __osl_debug_methods.m_create    = __osl_debug_create;
        __osl_debug_methods.m_delete    = __osl_debug_delete;

        __osl_debug_methods.m_getoption = __osl_debug_getOption;
        __osl_debug_methods.m_setoption = __osl_debug_setOption;

        __osl_debug_methods.m_accept    = __osl_debug_accept;
        __osl_debug_methods.m_connect   = __osl_debug_connect;
        __osl_debug_methods.m_close     = __osl_debug_close;

        __osl_debug_methods.m_shutdown  = __osl_debug_shutdown;

        osl_incrementInterlockedCount (&nonce);
    }

    osl_decrementInterlockedCount (&nonce);
    return (&__osl_debug_methods);
}

/*========================================================================
 *
 * osl/socket.c (physical, bottom layer).
 *
 *======================================================================*/
#if 0  /* NYI */

static oslSocketResult SAL_CALL __osl_socket_create (
    oslSocketDescriptor *socket,
    oslAddrFamily        family,
    oslSocketType        type,
    oslProtocol          protocol)
{
#ifdef DEBUG
    const oslSocketMethods *debug = osl_debug_getSocketMethods();
    if (!(socket->m_upper || socket->m_upper->m_methods == debug))
    {
        oslSocketDescriptor *upper;

        upper = osl_socket_createSocketLayer (debug);
        osl_socket_pushSocketLayer (upper, socket);

        return (upper->m_methods->m_create)(upper, family, type, protocol);
    }
#endif /* DEBUG */
    if (!(socket->m_private))
    {
        /* ... */
        socket->m_private = osl_createSocket (family, type, protocol);
    }
    return (osl_Socket_Ok);
}

#endif /* NYI */

/*========================================================================
 *
 * main.
 *
 *======================================================================*/
int SAL_CALL main (int argc, char **argv)
{
    oslSocketDescriptor *socket;

    socket = osl_socket_createSocketLayer (osl_debug_getSocketMethods());
    if (socket)
    {
        oslSocketDescriptor *connection;
        oslSocketResult      result;
        oslSocketAddr        addr, from;
        sal_Int32            option = 1;

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

        result = osl_socket_listen (socket, 1);
        OSL_ASSERT(result == osl_Socket_Ok);

        connection = osl_socket_accept (socket, &from);
        if (connection)
        {
            sal_Char         buffer[1024];
            sal_Int32        k, n = sizeof(buffer);
            oslSocketMsgFlag flag = osl_Socket_MsgNormal;

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
        osl_socket_delete (socket);

        osl_socket_deleteSocketLayer (socket);
    }

    return 0;
}

