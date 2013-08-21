/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifdef UNIX
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif //end of UNIX

#ifdef WNT
#ifdef _MSC_VER
#pragma once
#pragma warning (push,1)
#pragma warning (disable:4668)
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#ifdef _MSC_VER
#pragma warning (pop)
#endif
#endif //end of WNT

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "boost/scoped_array.hpp"

#include "ns_debug.hxx"
#include "so_msg.hxx"
#include "so_instance.hxx"
#include "so_env.hxx"

#include "nsp_func.hxx"

#include "sal/main.h"
#include <sal/macros.h>

#include "rtl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"

#include "osl/security.hxx"
#include "osl/thread.hxx"

#include "cppuhelper/bootstrap.hxx"



#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


#define MAX_NODE_NUM 1024

SoPluginInstance* lpInstance[MAX_NODE_NUM];

static  NSP_PIPE_FD la_read_fd = 0;
static char const * progdir = NULL;


long int NSP_ReadFromPipe(NSP_PIPE_FD fp, void* buf, unsigned long int len)
{
    unsigned long int len_unix = 0, len_wnt = 0;

    len_unix = NSP_Read_Pipe(fp, buf, len, &len_wnt);
#ifdef UNIX
    (void)len_wnt;
    return len_unix;
#endif //end of UNIX
#ifdef WNT
    (void)len_unix;
    return len_wnt;
#endif //end of WNT

}

int find_free_node()
{
    for(int i=0; i<MAX_NODE_NUM; i++)
    {
        if(NULL == lpInstance[i])
            return i;
    }
    return -1;
}

int find_cur_node(long cur_id)
{
    for(int i=0; i<MAX_NODE_NUM; i++)
    {
        if(lpInstance[i] == NULL)
            continue;
        if(cur_id == lpInstance[i]->GetParent())
            return i;
    }
    return -1;
}

sal_Bool dump_plugin_message(PLUGIN_MSG* pMsg)
{
    if (!pMsg)
        return sal_False;
    debug_fprintf(NSP_LOG_APPEND, "NSPlugin Message: msg_id:%d; instance_id:%d;wnd_id:%d;wnd_x:%d;wnd_y:%d;wnd_w:%d;wnd_h:%d; url:%s\n",
        pMsg->msg_id, pMsg->instance_id, pMsg->wnd_id,
        pMsg->wnd_x, pMsg->wnd_y, pMsg->wnd_w, pMsg->wnd_h, pMsg->url);
    return sal_True;
}

int Set_Window(PLUGIN_MSG* pMsg)
{
    dump_plugin_message(pMsg);
    int cur_no;
    if( -1 == (cur_no = find_cur_node(pMsg->instance_id)))
        return -1;
    if(lpInstance[cur_no]->SetWindow(pMsg->wnd_id,
        pMsg->wnd_x, pMsg->wnd_y, pMsg->wnd_w, pMsg->wnd_h))
        return 0;
    else
        return -1;
}

int Set_URL(PLUGIN_MSG* pMsg)
{
    dump_plugin_message(pMsg);
    int cur_no;
    if( -1 == (cur_no = find_cur_node(pMsg->instance_id)))
        return -1;
    if(lpInstance[cur_no]->SetURL(pMsg->url))
        return 0;
    else
        return -1;
}

int New_Instance(PLUGIN_MSG* pMsg, Reference< lang::XMultiServiceFactory > xMSF)
{
    dump_plugin_message(pMsg);
    int free_no;
    if( -1 == (free_no = find_free_node()))
        return -1;
    lpInstance[free_no] = new SoPluginInstance(pMsg->instance_id, xMSF);
    return 0;
}

int Destroy(PLUGIN_MSG* pMsg)
{
    dump_plugin_message(pMsg);
    int cur_no;
    if( -1 == (cur_no = find_cur_node(pMsg->instance_id)))
        return -1;
    if(lpInstance[cur_no] != NULL)
    {
        lpInstance[cur_no]->Destroy();
        debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, begin delete.\n");
        delete(lpInstance[cur_no]);
        lpInstance[cur_no] = NULL;
    }
    return 0;
}

int Print(PLUGIN_MSG* pMsg)
{
    dump_plugin_message(pMsg);
    int cur_no;
    if( -1 == (cur_no = find_cur_node(pMsg->instance_id)))
        return -1;
    if(lpInstance[cur_no] != NULL)
    {
        lpInstance[cur_no]->Print();
    }
    return 0;
}

int Shutdown()
{
    for(int cur_no=0; cur_no<MAX_NODE_NUM; cur_no++)
    {
        if(lpInstance[cur_no] == NULL)
            continue;
        lpInstance[cur_no]->Destroy();
        debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, begin delete.\n");
        delete(lpInstance[cur_no]);
        lpInstance[cur_no] = NULL;
    }
    return -1;
}

int dispatchMsg(PLUGIN_MSG* pMsg, Reference< lang::XMultiServiceFactory > xMSF)
{
    switch(pMsg->msg_id)
    {
        case SO_SET_WINDOW:
            return Set_Window(pMsg);
        case SO_NEW_INSTANCE:
            if(xMSF.is())
                return New_Instance(pMsg, xMSF);
        case SO_SET_URL:
            return Set_URL(pMsg);
        case SO_DESTROY:
            return Destroy(pMsg);
        case SO_SHUTDOWN:
            Shutdown();
            return -1;
         case SO_PRINT:
            Print(pMsg);
            return 0;
        default:
            return -1;
    }
}

Reference< lang::XMultiServiceFactory > SAL_CALL start_office(NSP_PIPE_FD read_fd)
{
    Reference< XComponentContext > xRemoteContext;

    try
    {
        OUString aOfficePath;

#ifdef UNIX
        boost::scoped_array< char > exepath(
            new char[( progdir ? strlen( progdir ) : 0 ) + RTL_CONSTASCII_LENGTH( "/soffice" ) + 1] );
        if ( progdir )
            sprintf( exepath.get(), "%s/soffice", progdir );
        else
            sprintf( exepath.get(), "soffice" );
        if (!rtl_convertStringToUString(
            &aOfficePath.pData, exepath.get(), strlen(exepath.get()), osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
            RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
            RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            debug_fprintf(NSP_LOG_APPEND,"bad characters in soffice installation path!\n");
            return Reference< lang::XMultiServiceFactory >(NULL);
        }
#endif //end of UNIX
#ifdef WNT
        char sPath[NPP_PATH_MAX];
        sPath[0] = 0;

        // The quotes will be added in osl_executeProcess
        sprintf(sPath, "%s", findSofficeExecutable() );
        if (!rtl_convertStringToUString(
            &aOfficePath.pData, sPath, strlen(sPath), osl_getThreadTextEncoding(),
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
            RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
            RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            debug_fprintf(NSP_LOG_APPEND,"bad characters in soffice installation path!\n");
            return Reference< lang::XMultiServiceFactory >(NULL);
        }
#endif //end of WNT

        // create default local component context
        Reference< XComponentContext > xLocalContext(
            defaultBootstrap_InitialComponentContext() );
        if ( !xLocalContext.is() )
        {
            debug_fprintf(NSP_LOG_APPEND,"no local component context!\n");
            return Reference< lang::XMultiServiceFactory >(NULL);
        }

        // env string
        OUStringBuffer buf;
        OUString aPath, aPluginPipeName;

        if(!Bootstrap::get("BRAND_BASE_DIR", aPath))
        {
            debug_fprintf(NSP_LOG_APPEND,"failed to get BRAND_BASE_DIR!\n");
            return Reference< lang::XMultiServiceFactory >(NULL);
        }

        aPluginPipeName = OUString::number( aPath.hashCode() );

        // accept string
        OSL_ASSERT( buf.isEmpty() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "--accept=pipe,name=" ) );
        buf.append( aPluginPipeName );    //user installation path as pipe name
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ";urp;" ) );
        OUString sConnectStartString( buf.makeStringAndClear() );

        // arguments
        OUString args [] = {
                OUString("--nologo"),
                OUString("--nodefault"),
                OUString("--nolockcheck"),
                sConnectStartString,
        };

        // create a URL resolver
        Reference< bridge::XUnoUrlResolver > xUrlResolver(
            bridge::UnoUrlResolver::create( xLocalContext ) );

        // connection string
        OSL_ASSERT( buf.isEmpty() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "uno:pipe,name=" ) );
        buf.append( aPluginPipeName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
            ";urp;StarOffice.ComponentContext" ) );
        OUString sConnectString( buf.makeStringAndClear() );

        try
        {
            // try to connect to office, no need to start instance again if office already started
            xRemoteContext.set(
                xUrlResolver->resolve( sConnectString ), UNO_QUERY_THROW );
            debug_fprintf(NSP_LOG_APPEND, "Staroffice already start\n");
            return Reference< lang::XMultiServiceFactory >(xRemoteContext->getServiceManager(), UNO_QUERY);
        }
        catch ( const connection::NoConnectException & )
        {
        }

        // start office process
#ifdef UNIX
        // a temporary solution
        // in future the process should be started using the osl_executeProcess call
        int nChildPID = fork();
        if( ! nChildPID )  // child process
        {
            NSP_Close_Pipe(read_fd);
            execl( "/bin/sh",
                   "/bin/sh",
                   OUStringToOString( aOfficePath, osl_getThreadTextEncoding() ).getStr(),
                   OUStringToOString( args[0], osl_getThreadTextEncoding() ).getStr(),
                   OUStringToOString( args[1], osl_getThreadTextEncoding() ).getStr(),
                   OUStringToOString( args[2], osl_getThreadTextEncoding() ).getStr(),
                   OUStringToOString( args[3], osl_getThreadTextEncoding() ).getStr(),
                   NULL);
            _exit(255);
        }
#else
        (void) read_fd; /* avoid warning about unused parameter */
        Security sec;
        oslProcess hProcess = 0;
        rtl_uString * ar_args [] = {
                args[ 0 ].pData,
                args[ 1 ].pData,
                args[ 2 ].pData,
                args[ 3 ].pData,
        };

        oslProcessError rc = osl_executeProcess(
            aOfficePath.pData,
            ar_args,
            SAL_N_ELEMENTS( ar_args ),
            osl_Process_DETACHED,
            sec.getHandle(),
            0, // => current working dir
            0,
            0, // => no env vars
            &hProcess );
        switch ( rc )
        {
            case osl_Process_E_None:
                osl_freeProcessHandle( hProcess );
                break;
            default:
                debug_fprintf(NSP_LOG_APPEND, "unmapped error!\n");
                return Reference< lang::XMultiServiceFactory >(NULL);
        }
#endif

        // wait until office is started
        for ( int i = 0; i < 240 /* stop the connection after 240 * 500ms */; ++i )
        {
            try
            {
                // try to connect to office
                xRemoteContext.set(
                    xUrlResolver->resolve( sConnectString ), UNO_QUERY_THROW );
                return Reference< lang::XMultiServiceFactory >(xRemoteContext->getServiceManager(), UNO_QUERY);
            }
            catch ( const connection::NoConnectException & )
            {
                // wait 500 ms, then try to connect again
                TimeValue tv = { 0 /* secs */, 500000000 /* nanosecs */ };
                ::osl::Thread::wait( tv );
            }
        }
        debug_fprintf(NSP_LOG_APPEND, "Failed to connect to Staroffice in 2 minutes\n");
        return Reference< lang::XMultiServiceFactory >(NULL);
    }
    catch (const Exception & e)
    {
        debug_fprintf(NSP_LOG_APPEND, "unexpected UNO exception caught: ");
        debug_fprintf(NSP_LOG_APPEND, (sal_Char *)e.Message.getStr());
        return Reference< lang::XMultiServiceFactory >(NULL);
    }

}


SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    // Sleep(20*1000);
    debug_fprintf(NSP_LOG_APPEND, "start of main\n");
    memset(lpInstance, 0, sizeof(lpInstance));

    // MessageBox( NULL, "nsplugin has been started", "Info", MB_OK );

    NSP_PIPE_FD fd_pipe[2];
    int iPipe[2];
     if(argc < 3)
    {
        debug_fprintf(NSP_LOG_APPEND, "print by nsplugin, command error; too little argument to start plugin exec\n");
        return EXIT_FAILURE;
    }
    iPipe[0] = atoi(argv[1]);
    iPipe[1] = atoi(argv[2]);

    // fd_pipe[0]: read, fd_pipe[0]: write
    fd_pipe[0] = (NSP_PIPE_FD) (sal_IntPtr) iPipe[0] ;
    fd_pipe[1] = (NSP_PIPE_FD) (sal_IntPtr) iPipe[1] ;
    NSP_Close_Pipe(fd_pipe[1]);

    if(iPipe[0] < 0)
    {
        debug_fprintf(NSP_LOG_APPEND, "print by nsplugin, command error: bad read file id:%s \n", iPipe[0]);
        return 0;
    }
    la_read_fd = fd_pipe[0];

    // the program path is provided only on unix, on windows the registry entry is used
    if ( argc > 4 )
        progdir = argv[4];

    Reference< lang::XMultiServiceFactory > xFactory = start_office(la_read_fd);
    if(!xFactory.is())
    {
        NSP_Close_Pipe(la_read_fd);
        return -1;
    }
    PLUGIN_MSG nMsg;
    int len;
    while(1)
    {
        memset(&nMsg, 0, sizeof(PLUGIN_MSG));
        len = NSP_ReadFromPipe(la_read_fd, (char*)&nMsg, sizeof(PLUGIN_MSG));
        if(len != sizeof(PLUGIN_MSG))
            break;
        debug_fprintf(NSP_LOG_APPEND, "Read message from pipe type %d \n", nMsg.msg_id);
        if(-1 == dispatchMsg(&nMsg, xFactory))
        {
            debug_fprintf(NSP_LOG_APPEND, "plugin will shutdown\n");
            break;
        }
    }
    NSP_Close_Pipe(la_read_fd);
    _exit(0);
#ifndef _MSC_VER
    return EXIT_SUCCESS; // avoid warnings
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
