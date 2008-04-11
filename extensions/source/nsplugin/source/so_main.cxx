/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: so_main.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
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
#endif
#pragma warning (push,1)
#pragma warning (disable:4668)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#pragma warning (pop)
#endif //end of WNT

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ns_debug.hxx"
#include "so_msg.hxx"
#include "so_instance.hxx"
#include "so_env.hxx"

#include "nsp_func.hxx"



#define MAX_NODE_NUM 1024
SoPluginInstance* lpInstance[MAX_NODE_NUM];


long int NSP_ReadFromPipe(NSP_PIPE_FD fp, void* buf, unsigned long int len)
{
    unsigned long int len_unix = 0, len_wnt = 0;

    len_unix = NSP_Read_Pipe(fp, buf, len, &len_wnt);
#ifdef UNIX
    (void)len_wnt;
    return  len_unix;
#endif //end of UNIX
#ifdef WNT
    return  len_wnt;
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

int find_cur_node(sal_Int32 cur_id)
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

int prepareEnviron()
{
    // if child process inherit the chdir() property from parent process, if yes, no getNewLibraryPath() needed
    const char* pNewLibraryPath = getNewLibraryPath();
    putenv( (char*) pNewLibraryPath );
    SoPluginInstance::SetSODir((char *)findProgramDir());

    return 0;
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

int New_Instance(PLUGIN_MSG* pMsg)
{
    dump_plugin_message(pMsg);
    int free_no;
    if( -1 == (free_no = find_free_node()))
        return -1;
    lpInstance[free_no] = new SoPluginInstance(pMsg->instance_id);
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

int dispatchMsg(PLUGIN_MSG* pMsg)
{
    switch(pMsg->msg_id)
    {
        case SO_SET_WINDOW:
            return Set_Window(pMsg);
        case SO_NEW_INSTANCE:
            return New_Instance(pMsg);
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

sal_Bool start_office(NSP_PIPE_FD read_fd)
{
    int my_sock;
    struct sockaddr_in dst_addr;
    char sCommand[NPP_PATH_MAX];
    sCommand[0] = 0;
#ifdef WNT
    {
        WSADATA wsaData;
        WORD wVersionRequested;

        wVersionRequested = MAKEWORD(2,0);
        if(WSAStartup(wVersionRequested, &wsaData))
        {
           NSP_Close_Pipe(read_fd);
           debug_fprintf(NSP_LOG_APPEND, "Can not init socket in Windows.\n");
           return sal_False;
        }
    }
#endif //end of WNT
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port   = htons(SO_SERVER_PORT);
    dst_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int count = 0;
    int ret = 0;

    my_sock=socket(PF_INET, SOCK_STREAM, 0);

    // if Star Office has been stared, we need not to start it again
    ret = connect(my_sock, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
    if(ret == 0)
    {
        NSP_CloseSocket(my_sock);
        debug_fprintf(NSP_LOG_APPEND, "Staroffice already start\n");
        return sal_True;
    }
    {
        debug_fprintf(NSP_LOG_APPEND, "try to star Staroffice\n");
        char para[128] = {0};
        sprintf(para, "-accept=socket,host=0,port=%d;urp", SO_SERVER_PORT);
#ifdef UNIX
        int nChildPID = fork();
        if( ! nChildPID )  // child process
       {
            NSP_CloseSocket(my_sock);
            NSP_Close_Pipe(read_fd);
            sprintf(sCommand, "/bin/sh soffice -nologo -nodefault %s", para);
            debug_fprintf(NSP_LOG_APPEND,"StarOffice will be started by command: %s\n",sCommand);
            execl("/bin/sh", "/bin/sh", "soffice", "-nologo", "-nodefault", para, NULL);
            _exit(255);
        }
#endif //end of UNIX
#ifdef WNT
            STARTUPINFO NSP_StarInfo;
            memset((void*) &NSP_StarInfo, 0, sizeof(STARTUPINFO));
            NSP_StarInfo.cb = sizeof(STARTUPINFO);
            PROCESS_INFORMATION NSP_ProcessInfo;
            memset((void*)&NSP_ProcessInfo, 0, sizeof(PROCESS_INFORMATION));
            sprintf(para, " -nologo -nodefault -accept=socket,host=0,port=%d;urp", SO_SERVER_PORT);
            //sprintf(para, " -accept=socket,host=0,port=%d;urp\n", SO_SERVER_PORT);
            SECURITY_ATTRIBUTES  NSP_access = { sizeof(SECURITY_ATTRIBUTES), NULL, FALSE}; (void)NSP_access;
            sprintf(sCommand, "\"%s\" %s", findSofficeExecutable(), para);
            debug_fprintf(NSP_LOG_APPEND,"StarOffice will be started by command: %s",sCommand);
            BOOL ret = false;
            ret = CreateProcess(findSofficeExecutable(), sCommand, NULL, NULL, FALSE,
                        0 , NULL, NULL, &NSP_StarInfo, &NSP_ProcessInfo);
            if(ret==false){
                debug_fprintf(NSP_LOG_APPEND,"run staroffice error: %u \n",
                    GetLastError());
            }
            else   debug_fprintf(NSP_LOG_APPEND,"run staroffice success\n");
#endif //end of WNT
    }

    NSP_Sleep(5);
    // try to connect to background SO, thus judge if it is ready
    while(0 > connect(my_sock, (struct sockaddr *)&dst_addr, sizeof(dst_addr)))
    {
        NSP_Sleep(1);
        if (count++ >= 120)
        {
            NSP_CloseSocket(my_sock);
            debug_fprintf(NSP_LOG_APPEND, "print by nsplugin, con star remote StarOffice\n");
            return sal_False;
        }
        debug_fprintf(NSP_LOG_APPEND, "print by nsplugin,  Current count: %d\n", count);
    }
    NSP_CloseSocket(my_sock);
    NSP_Sleep(5);

    prepareEnviron();

    return sal_True;
}


static  NSP_PIPE_FD la_read_fd = 0;

int main(int argc, char** argv)
{
   // Sleep(20*1000);
    debug_fprintf(NSP_LOG_APPEND, "start of main\n");
    memset(lpInstance, 0, sizeof(lpInstance));

    NSP_PIPE_FD fd_pipe[2];
    int iPipe[2];
#ifdef UNIX
     if(argc < 3)
    {
        debug_fprintf(NSP_LOG_APPEND, "print by nsplugin, command error; too little argument to start plugin exec\n");
        return sal_False;
    }
    iPipe[0] = atoi(argv[1]);
    iPipe[1] = atoi(argv[2]);
#endif  //end of UNIX
#ifdef WNT
    (void)argc;
    //sscanf( GetCommandLine(), "%d %d", &iPipe[0],  &iPipe[1] );
    iPipe[0] = atoi(argv[0]);
    iPipe[1] = atoi(argv[1]);
#endif  //end of WNT

    // fd_pipe[0]: read, fd_pipe[0]: write
    fd_pipe[0] = (NSP_PIPE_FD) iPipe[0] ;
    fd_pipe[1] = (NSP_PIPE_FD) iPipe[1] ;
    NSP_Close_Pipe(fd_pipe[1]);

    la_read_fd = fd_pipe[0];
    if(la_read_fd < 0)
    {
        debug_fprintf(NSP_LOG_APPEND, "print by nsplugin, command error: bad read file id:%s \n", la_read_fd);
        return 0;
    }

    if(!start_office(la_read_fd))
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
        if(-1 == dispatchMsg(&nMsg))
        {
            debug_fprintf(NSP_LOG_APPEND, "plugin will shutdown\n");
            break;
        }
    }
    NSP_Close_Pipe(la_read_fd);
    _exit(0);
}

extern "C"{
    sal_Bool restart_office(void){
        return start_office(la_read_fd);
    }
}
