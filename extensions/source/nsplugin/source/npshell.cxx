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

#define XP_UNIX 1

#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#endif //end of UNIX

#ifdef WNT

#define _WINDOWS

#ifdef _MSC_VER
#pragma warning (push,1)
#pragma warning (disable:4668)
#endif

#include <windows.h>
#include <direct.h>
#include <stdlib.h>
#include <io.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include "tools/pathutils.hxx"

#endif //end of WNT


#include <stdio.h>

#include <string.h>
#include <errno.h>
#include "boost/scoped_array.hpp"

#include "npapi.h"
#include "npshell.hxx"
#include "so_env.hxx"
#include "so_msg.hxx"
#include "ns_debug.hxx"


#include "nsp_func.hxx"

#include <comphelper/documentconstants.hxx>

#ifdef _MSC_VER
#define NP_DLLPUBLIC
#else
#define NP_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#endif

/***********************************************************************
 *
 * Implementations of plugin API functions
 *
 ***********************************************************************/

static NSP_Mute_Obj send_lock = NSP_New_Mute_Obj();
static NSP_PIPE_FD write_fd = 0;


long int NSP_WriteToPipe(NSP_PIPE_FD fp, void* buf, unsigned long int len)
{
    unsigned long int len_unix = 0, len_wnt = 0;

    len_unix = NSP_Write_Pipe(fp, buf, len, &len_wnt);
#ifdef UNIX
    (void)len_wnt;
    return len_unix;
#endif //end of UNIX
#ifdef WNT
    (void)len_unix;
    return len_wnt;
#endif //end of WNT

}

#ifdef UNIX
static pid_t nChildPID = 0;
#endif

#define MY_LENGTH(s) (sizeof (s) / sizeof *(s) - 1)
#define MY_STRING(s) (s), MY_LENGTH(s)

#if defined WNT
namespace {

bool extendEnvironment(boost::scoped_array< WCHAR > * environment) {
    WCHAR path[MAX_PATH];
    int len = MultiByteToWideChar(
        CP_ACP, MB_PRECOMPOSED, findInstallDir(), -1, path, MAX_PATH);
        //TODO: conversion errors
    if (len == 0 ||
        (tools::buildPath(path, path, path + len - 1, MY_STRING(L"\\ure-link"))
         == NULL))
    {
        return false;
    }
    WCHAR * pathEnd = tools::resolveLink(path);
    if (pathEnd == NULL) {
        return false;
    }
    pathEnd = tools::buildPath(path, path, pathEnd, MY_STRING(L"\\bin"));
    if (pathEnd == NULL) {
        return false;
    }
    WCHAR const * env = GetEnvironmentStringsW();
    if (env == NULL) {
        return false;
    }
    WCHAR const * p = env;
    WCHAR const * p1 = NULL;
    while (*p != L'\0') {
        size_t n = wcslen(p);
        if (p1 == NULL && n >= MY_LENGTH(L"PATH=") &&
            (p[0] == L'P' ||  p[0] == L'p') &&
            (p[1] == L'A' ||  p[1] == L'a') &&
            (p[2] == L'T' ||  p[2] == L't') &&
            (p[3] == L'H' ||  p[3] == L'h') && p[4] == L'=')
        {
            p1 = p + MY_LENGTH(L"PATH=");
            //TODO: check whether the path is already present in PATH (at the
            // beginning of PATH?)
        }
        p += n + 1;
    }
    ++p;
    if (p1 == NULL) {
        environment->reset(
            new WCHAR[MY_LENGTH(L"PATH=") + (pathEnd - path) + 1 + (p - env)]);
            //TODO: overflow
        memcpy(environment->get(), MY_STRING(L"PATH=") * sizeof (WCHAR));
        memcpy(
            environment->get() + MY_LENGTH(L"PATH="), path,
            ((pathEnd - path) + 1) * sizeof (WCHAR));
        memcpy(
            environment->get() + MY_LENGTH(L"PATH=") + (pathEnd - path) + 1,
            env, (p - env) * sizeof (WCHAR));
    } else {
        environment->reset(
            new WCHAR[(p - env) + (pathEnd - path) + MY_LENGTH(L";")]);
            //TODO: overflow
        memcpy(environment->get(), env, (p1 - env) * sizeof (WCHAR));
        memcpy(
            environment->get() + (p1 - env), path,
            (pathEnd - path) * sizeof (WCHAR));
        environment->get()[(p1 - env) + (pathEnd - path)] = L';';
        memcpy(
            environment->get() + (p1 - env) + (pathEnd - path) + 1, p1,
            (p - p1) * sizeof (WCHAR));
    }
    return true;
}

}
#endif

// start nspluin executable in child process, and use pipe to talk with it
int do_init_pipe()
{
    debug_fprintf(NSP_LOG_APPEND, "enter do_init_pipe 1\n");
    NSP_PIPE_FD fd[2];

    if( 0 != NSP_Inherited_Pipe(fd) )
        return NPERR_GENERIC_ERROR;

    write_fd = fd[1];   // write fd
#ifdef UNIX
    // the parent process will wait for the child process in NPP_Shutdown code
    nChildPID = fork();

    if( ! nChildPID )  // child process
#endif //end of UNIX
    {
#ifdef UNIX
        char s_read_fd[16] = {0};
        char s_write_fd[16] = {0};
        sprintf(s_read_fd,  "%d", fd[0]);
        sprintf(s_write_fd, "%d", fd[1]);
        char const * progdir = findProgramDir();
        boost::scoped_array< char > exepath(
            new char[strlen(progdir) + RTL_CONSTASCII_LENGTH("/nsplugin") + 1]);
        sprintf(exepath.get(), "%s/nsplugin", progdir);
        boost::scoped_array< char > inifilepath(
            new char[
                RTL_CONSTASCII_LENGTH(
                    "-env:INIFILENAME=vnd.sun.star.pathname:") +
                strlen(progdir) + RTL_CONSTASCII_LENGTH("/redirectrc") + 1]);
            //TODO: overflow
        sprintf(
            inifilepath.get(),
            "-env:INIFILENAME=vnd.sun.star.pathname:%s/redirectrc", progdir);
        execl(
            exepath.get(), exepath.get(), s_read_fd, s_write_fd,
            inifilepath.get(), progdir, NULL);
        _exit(255);
#endif //end of UNIX
#ifdef WNT
        WCHAR s_read_fd[16] = {0};
        WCHAR s_write_fd[16] = {0};
        wsprintfW(s_read_fd, L"%d", fd[0]);
        wsprintfW(s_write_fd, L"%d", fd[1]);
        boost::scoped_array< WCHAR > env;
        if (!extendEnvironment(&env)) {
            return NPERR_GENERIC_ERROR;
        }
        WCHAR path[MAX_PATH];
        int pathLen = MultiByteToWideChar(
            CP_ACP, MB_PRECOMPOSED, findProgramDir(), -1, path, MAX_PATH);
            //TODO: conversion errors
        if (pathLen == 0) {
            return NPERR_GENERIC_ERROR;
        }
        WCHAR exe[MAX_PATH];
        WCHAR * exeEnd = tools::buildPath(
            exe, path, path + pathLen - 1, MY_STRING(L"\\nsplugin.exe"));
        if (exeEnd == NULL) {
            return NPERR_GENERIC_ERROR;
        }
        WCHAR ini[MAX_PATH];
        WCHAR * iniEnd = tools::buildPath(
            ini, path, path + pathLen - 1, MY_STRING(L"\\redirect.ini"));
        if (iniEnd == NULL) {
            return NPERR_GENERIC_ERROR;
        }
        boost::scoped_array< WCHAR > args(
            new WCHAR[
                MY_LENGTH(L"\"") + (exeEnd - exe) + MY_LENGTH(L"\" ") +
                wcslen(s_read_fd) + MY_LENGTH(L" ") + wcslen(s_write_fd) +
                MY_LENGTH(L" \"-env:INIFILENAME=vnd.sun.star.pathname:") +
                (iniEnd - ini) + MY_LENGTH(L"\"") + 1]); //TODO: overflow
        wsprintfW(
            args.get(),
            L"\"%s\" %s %s \"-env:INIFILENAME=vnd.sun.star.pathname:%s\"", exe,
            s_read_fd, s_write_fd, ini);
        STARTUPINFOW NSP_StarInfo;
        memset((void*) &NSP_StarInfo, 0, sizeof(STARTUPINFOW));
        NSP_StarInfo.cb = sizeof(STARTUPINFOW);
        PROCESS_INFORMATION NSP_ProcessInfo;
        memset((void*)&NSP_ProcessInfo, 0, sizeof(PROCESS_INFORMATION));
        if(!CreateProcessW(
               exe, args.get(), NULL, NULL, TRUE,
               CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, env.get(), path,
               &NSP_StarInfo, &NSP_ProcessInfo))
        {
            DWORD Err = GetLastError();
            (void)Err;
        }
#endif //end of WNT
    }
    NSP_Close_Pipe(fd[0]);
    return NPERR_NO_ERROR;
}

bool sendMsg( PLUGIN_MSG* pMsg, size_t len, int iEnsure)
{
    NSP_Lock_Mute_Obj(send_lock);
    size_t len_w = 0;

    debug_fprintf(NSP_LOG_APPEND, "try to send message type:%d; len: %d\n", pMsg->msg_id, len);
    /*
    debug_fprintf(NSP_LOG_APPEND, "NSPlugin Message: msg_id:%d; instance_id:%d;
        wnd_id:%d;wnd_x:%d;wnd_y:%d;wnd_w:%d;wnd_h:%d; url:%s\n",
        pMsg->msg_id, pMsg->instance_id, pMsg->wnd_id,
        pMsg->wnd_x, pMsg->wnd_y, pMsg->wnd_w, pMsg->wnd_h, pMsg->url);*/
    len_w = NSP_WriteToPipe(write_fd, (void*) pMsg, len);
    if (len_w != len){
        if(errno == EPIPE) // If pipe breaks, then init pipe again and resend the msg
        {
            if(iEnsure){
                debug_fprintf(NSP_LOG_APPEND, "send message error, plugin exec need to be restart\n");
                NSP_Close_Pipe(write_fd);
                do_init_pipe();
                len_w = NSP_WriteToPipe(write_fd, (void*) pMsg, len);
            }
        }
        else if(errno == EINTR) // If interrupted by signal, then continue to send
        {
            long  new_len;
            debug_fprintf(NSP_LOG_APPEND, "send message error, send intrrupted by singal, resend again\n");
            new_len = NSP_WriteToPipe(write_fd, (char*)pMsg+len_w, len-len_w);
            len_w = len_w + new_len;
        }
        else  // else return error
        {
            debug_fprintf(NSP_LOG_APPEND, "send message error :%s.\n", strerror(errno));
            len_w = 0;
        }
    }
    NSP_Unlock_Mute_Obj(send_lock);
    debug_fprintf(NSP_LOG_APPEND, "send message success!\n");
    return len_w == len;
}

extern "C"
{
char* pMimeTypes = const_cast< char* >( "application/vnd.stardivision.calc:sdc:StarCalc 3.0 - 5.0;"
"application/vnd.stardivision.chart:sds:StarChart 3.0 - 5.0;"
"application/vnd.stardivision.draw:sda:StarDraw 3.0 - 5.0;"
"application/vnd.stardivision.impress:sdd:StarImpress 3.0 - 5.0;"
"application/vnd.stardivision.impress-packed:sdp:StarImpress-packed 3.0 - 5.0;"
"application/vnd.stardivision.math:smf:StarMath 3.0 - 5.0;"
"application/vnd.stardivision.writer:vor:StarWriter Template 3.0 - 5.0;"
"application/vnd.stardivision.writer-global:sgl:StarWriter Global 3.0 - 5.0;"
"application/vnd.staroffice.writer:sdw:StarWriter 3.0 - 5.0;"
"application/msword:doc:MS Word document;"
"application/msword:dot:MS Word document template;"
"application/vnd.ms-excel:xls:MS Excel spreadsheet;"
"application/vnd.ms-excel:xlt:MS Excel spreadsheet template;"
"application/vnd.ms-excel:xla:MS Excel spreadsheet;"
MIMETYPE_VND_SUN_XML_CALC_ASCII ":sxc:StarOffice 6.0/7 Spreadsheet;"
MIMETYPE_VND_SUN_XML_CALC_TEMPLATE_ASCII":stc:StarOffice 6.0/7 Spreadsheet Template;"
MIMETYPE_VND_SUN_XML_DRAW_ASCII ":sxd:StarOffice 6.0/7 Drawing;"
MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE_ASCII ":std:StarOffice 6.0/7 Drawing Template;"
MIMETYPE_VND_SUN_XML_IMPRESS_ASCII ":sxi:StarOffice 6.0/7 Presentation;"
MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE_ASCII ":sti:StarOffice 6.0/7 Presentation Template;"
MIMETYPE_VND_SUN_XML_MATH_ASCII ":sxm:StarOffice 6.0/7 Formula;"
MIMETYPE_VND_SUN_XML_WRITER_ASCII ":sxw:StarOffice 6.0/7 Text Document;"
MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII ":sxg:StarOffice 6.0/7 Master Document;"
MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE_ASCII ":stw:StarOffice 6.0/7 Text Document Template;"
MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII ":odt:OpenDocument Text;"
MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII ":ott:OpenDocument Text Template;"
MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII ":odm:OpenDocument Master Document;"
MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII ":oth:HTML Document Template;"
MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII ":ods:OpenDocument Spreadsheet;"
MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII ":ots:OpenDocument Spreadsheet Template;"
MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII ":odg:OpenDocument Drawing;"
MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII ":otg:OpenDocument Drawing Template;"
MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII ":odp:OpenDocument Presentation;"
MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII ":otp:OpenDocument Presentation Template;"
MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII ":odf:OpenDocument Formula;" );

NP_DLLPUBLIC
#ifndef HAVE_NON_CONST_NPP_GETMIMEDESCRIPTION
const
#endif
char*
NPP_GetMIMEDescription(void)
{
    debug_fprintf(NSP_LOG_APPEND, "print by Netscape Plugin,  NPP_GetMIMEDescription:%s.\n", pMimeTypes);
    return(pMimeTypes);
}

#ifdef UNIX
NP_DLLPUBLIC NPError
NPP_GetValue(NPP /*instance*/, NPPVariable variable, void *value)
{
    NPError err = NPERR_NO_ERROR;

    switch (variable) {
        case NPPVpluginNameString:
            // add here, for dynamic productname
            *((char **)value) = NSP_getPluginName();
            break;
        case NPPVpluginDescriptionString:
            // add here, for dynamic product description
            *((char **)value) = NSP_getPluginDesc();
            break;
        default:
            err = NPERR_GENERIC_ERROR;
    }
    debug_fprintf(NSP_LOG_APPEND, "print by Netscape Plugin,  NPP_GetValue return %d.\n", err);
    return err;
}


NPMIMEType
dupMimeType(NPMIMEType type)
{
    NPMIMEType mimetype = (NPMIMEType) NPN_MemAlloc(strlen(type)+1);
    mimetype[strlen(type)] = 0;
    if (mimetype)
        strcpy(mimetype, type);
    return(mimetype);
}
#endif // end of UNIX

NP_DLLPUBLIC NPError
NPP_Initialize(void)
{
    debug_fprintf(NSP_LOG_NEW, "NS Plugin begin initialize.\n");
    return (NPError)do_init_pipe();
}

#ifdef OJI
NP_DLLPUBLIC jref
NPP_GetJavaClass()
{
    return NULL;
}
#endif

NP_DLLPUBLIC void
NPP_Shutdown(void)
{
    PLUGIN_MSG msg;
    memset((char*)&msg, 0, sizeof(PLUGIN_MSG));
    msg.msg_id = SO_SHUTDOWN;
    sendMsg(&msg, sizeof(PLUGIN_MSG), 0);
    NSP_Close_Pipe(write_fd);

#ifdef UNIX
    // on Unix we should wait till the child process is dead
    int nStatus;
    waitpid( nChildPID, &nStatus, 0 );
#endif
}

NP_DLLPUBLIC NPError NP_LOADDS
NPP_New(NPMIMEType pluginType,
    NPP instance,
    uint16_t mode,
    int16_t /*argc*/,
    char* /*argn*/[],
    char* /*argv*/[],
    NPSavedData* /*saved*/)
{
    PluginInstance* This;

    debug_fprintf(NSP_LOG_APPEND, "print by Netscape Plugin, enter NPP_New.\n");
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    instance->pdata = (PluginInstance*)NPN_MemAlloc(sizeof(PluginInstance));

    memset(instance->pdata, 0 , sizeof(PluginInstance));
    This = (PluginInstance*) instance->pdata;

    if (This == NULL)
    {
        return NPERR_OUT_OF_MEMORY_ERROR;
    }

    memset(This, 0, sizeof(PluginInstance));

    /* mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h) */
#ifdef UNIX
    This->mode = mode;
    This->type = dupMimeType(pluginType);
    This->instance = instance;
    This->pluginsPageUrl = NULL;
    This->exists = FALSE;
#endif //end of UNIX
#ifdef WNT
    (void)pluginType;
    This->fWindow = (NPWindow*)NPN_MemAlloc(sizeof(NPWindow));
    memset(This->fWindow, 0, sizeof (NPWindow));
    This->fMode = mode;
  #endif //end of WNT
    PLUGIN_MSG msg;
    memset((char*)&msg, 0, sizeof(PLUGIN_MSG));
    msg.msg_id = SO_NEW_INSTANCE;
    msg.instance_id = (plugin_Int32)instance;
    if (!sendMsg(&msg, sizeof(PLUGIN_MSG), 1))
        return NPERR_GENERIC_ERROR;

    NPN_Status(instance, "......");
    return NPERR_NO_ERROR;
}

NP_DLLPUBLIC NPError NP_LOADDS
NPP_Destroy(NPP instance, NPSavedData** /*save*/)
{
    debug_fprintf(NSP_LOG_APPEND, "print by Nsplugin, enter NPP_Destroy.\n");
    PluginInstance* This;

    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    // Send destroy message
    PLUGIN_MSG msg;
    memset((char*)&msg, 0, sizeof(PLUGIN_MSG));
    msg.msg_id = SO_DESTROY;
    msg.instance_id = (plugin_Int32)instance;
#ifdef UNIX
    msg.wnd_id =(plugin_Int32)((PluginInstance*) instance->pdata)->window;
#endif //end of UNIX
#ifdef WNT
    msg.wnd_id =(plugin_Int32)((PluginInstance*) instance->pdata)->fhWnd;
#endif //end of WNT
    sendMsg(&msg, sizeof(PLUGIN_MSG), 0);

    // Free the instance space
    This = (PluginInstance*) instance->pdata;
    if (This != NULL) {
#ifdef UNIX
        if (This->type)
            NPN_MemFree(This->type);
        if (This->pluginsPageUrl)
            NPN_MemFree(This->pluginsPageUrl);
        if (This->pluginsFileUrl)
                NPN_MemFree(This->pluginsFileUrl);
#endif //end of UNIX
#ifdef WNT
        if(This->fWindow)
            NPN_MemFree(This->fWindow);
#endif //end of WNT
        NPN_MemFree(instance->pdata);
        instance->pdata = NULL;
    }

    return NPERR_NO_ERROR;
}


NP_DLLPUBLIC NPError NP_LOADDS
NPP_SetWindow(NPP instance, NPWindow* window)
{
    PluginInstance* This;
#ifdef UNIX
    NPSetWindowCallbackStruct *ws_info;
#endif //end of UNIX

    debug_fprintf(NSP_LOG_APPEND, "print by Netscape Plugin, received window resize.\n");
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    This = (PluginInstance*) instance->pdata;
    if (This == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    // Prepare the general part of the SET_WINDOW message
    PLUGIN_MSG msg;
    memset((char*)&msg, 0, sizeof(msg));
    msg.msg_id = SO_SET_WINDOW;
    msg.instance_id = (plugin_Int32)instance;

    if ( window )
    {
        // Set window info for instance
#ifdef UNIX
        ws_info        = (NPSetWindowCallbackStruct *)window->ws_info;
        This->window   = (Window) window->window;
        This->x        = window->x;
        This->y        = window->y;
        This->width    = window->width;
        This->height   = window->height;
        This->display  = ws_info->display;
        This->visual   = ws_info->visual;
        This->depth    = ws_info->depth;
        This->colormap = ws_info->colormap;
#endif    //end of UNIX
#ifdef WNT
        This->fhWnd   = (HWND) window->window;
        This->fWindow->x        = window->x;
        This->fWindow->y        = window->y;
        This->fWindow->width    = window->width;
        This->fWindow->height   = window->height;
#endif    //end of WNT
        debug_fprintf(NSP_LOG_APPEND, "begin Set window of Office\n");
        debug_fprintf(NSP_LOG_APPEND, "W=(%d) H=(%d)\n", window->width, window->height);

        // fill the window dependent part of the message
        msg.wnd_id = (plugin_Int32) window->window;
        msg.wnd_x = window->x;
        msg.wnd_y = window->y;
        msg.wnd_w = window->width;
        msg.wnd_h = window->height;
    }
    else
    {
        // empty window pointer usually means closing of the parent window
#ifdef UNIX
        ws_info        = NULL;
        This->window   = (Window) NULL;
        This->x        = 0;
        This->y        = 0;
        This->width    = 0;
        This->height   = 0;
        This->display  = NULL;
        This->visual   = NULL;
#endif    //end of UNIX
#ifdef WNT
        This->fhWnd   = (HWND) NULL;
        This->fWindow->x        = 0;
        This->fWindow->y        = 0;
        This->fWindow->width    = 0;
        This->fWindow->height   = 0;
#endif    //end of WNT
        debug_fprintf(NSP_LOG_APPEND, "Empty window pointer is provided\n");

        // fill the window dependent part of the message
        msg.wnd_id = (plugin_Int32) NULL;
        msg.wnd_x = 0;
        msg.wnd_y = 0;
        msg.wnd_w = 0;
        msg.wnd_h = 0;
    }

    if(!sendMsg(&msg, sizeof(PLUGIN_MSG), 1))
    {
        debug_fprintf(NSP_LOG_APPEND, "NPP_SetWindow return failure \n");
        return NPERR_GENERIC_ERROR;
    }

    return NPERR_NO_ERROR;
}


NP_DLLPUBLIC NPError NP_LOADDS
NPP_NewStream(NPP instance,
          NPMIMEType /*type*/,
          NPStream* /*stream*/,
          NPBool /*seekable*/,
          uint16_t *stype)
{
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;

    // Notify Mozilla to fetch the remote file into local cache directory
    *stype=NP_ASFILEONLY;

    return NPERR_NO_ERROR;
}


int32_t STREAMBUFSIZE = 0X0FFFFFFF;
/* If we are reading from a file in NPAsFile
 * mode so we can take any size stream in our
 * write call (since we ignore it) */

NP_DLLPUBLIC int32_t NP_LOADDS
NPP_WriteReady(NPP /*instance*/, NPStream* /*stream*/)
{
    return STREAMBUFSIZE;
}


NP_DLLPUBLIC int32_t NP_LOADDS
NPP_Write(NPP /*instance*/, NPStream* /*stream*/, int32_t /*offset*/, int32_t len, void* /*buffer*/)
{
    return len;     /* The number of bytes accepted */
}


NP_DLLPUBLIC NPError NP_LOADDS
NPP_DestroyStream(NPP instance, NPStream* /*stream*/, NPError /*reason*/)
{
    if (instance == NULL)
        return NPERR_INVALID_INSTANCE_ERROR;
    return NPERR_NO_ERROR;
}

// save fname to another file with the original file name
NP_DLLPUBLIC void NP_LOADDS
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
    debug_fprintf(NSP_LOG_APPEND, "Into Stream\n");
    char* url = (char*)stream->url;
    char filename[1024] = {0};
    char* pfilename = NULL;
    if (NULL != (pfilename = strrchr(url, '/')))
    {
        strcpy(filename, pfilename+1);
    } else {
        return;
    }

    int length = strlen(url);
    debug_fprintf(NSP_LOG_APPEND, "url: %s; length: %d\n", url, length);
    PluginInstance* This;
    This = (PluginInstance*) instance->pdata;

    debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile\n");

    // copy cached file to another file with original name
    char localPathNew[NPP_PATH_MAX] = {0};
    char localFileNew[NPP_PATH_MAX] = {0};
    // if the file is from local
    if (0 == STRNICMP(url, "file:///", strlen("file:///")))
    {
        sprintf(localPathNew, "%s", fname);
        char* pAskSymbol = NULL;
        if(NULL != (pAskSymbol = strrchr(localPathNew, '?')))
            *pAskSymbol = 0;
    }
    else // from network, on windows, fname is c:\abc123
    {
        strncpy(localPathNew, fname, sizeof(localPathNew));
        char* pRandomFilename = NULL;

#ifdef UNIX
        if(NULL != (pRandomFilename = strrchr(localPathNew, '/')))
#endif //end of UNIX
#ifdef WNT
        if(NULL != (pRandomFilename = strrchr(localPathNew, '\\')))
#endif //end of WNT
        {
            pRandomFilename[1] = 0;
        } else {
            return;
        }
        strcat(localPathNew, filename);
        char* pAskSymbol = NULL;
        if(NULL != (pAskSymbol = strrchr(localPathNew, '?')))
            *pAskSymbol = 0;

        sprintf(localFileNew, "file://%s", localPathNew);
        UnixToDosPath(localFileNew);
        debug_fprintf(NSP_LOG_APPEND, "fname: %s\n localPathNew: %s\nlocalFileNew: %s\n",
            fname, localPathNew, localFileNew);

        restoreUTF8(localPathNew);
        restoreUTF8(localFileNew);
        if(0 != strcmp(fname, localPathNew)) {

#ifdef WNT
        if(FALSE == CopyFile(fname, localPathNew, FALSE))
            return;
#endif //end of WNT

#ifdef UNIX
        int fdSrc, fdDst;
        if((0 > (fdSrc = open(fname, O_RDONLY)))){
                return;
        }
        remove(localPathNew);
        umask(0);
        if  (0 > (fdDst = open(localPathNew, O_WRONLY|O_CREAT,
                        S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))){
            close( fdSrc);
            debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile:can not create cache file %s. error: %s \n",
                localPathNew, strerror(errno));
            return;
        }
        char buffer[NPP_BUFFER_SIZE] = {0};
        ssize_t ret;
        while(0 <= (ret = read(fdSrc, buffer, NPP_BUFFER_SIZE)))
        {
            if (0 == ret)
            {
                if(EINTR == errno)
                    continue;
                else

                    break;
            }
            ssize_t written_bytes = write(fdDst, buffer, ret);
            if (written_bytes != ret)
            {
                debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile:short write to %s. error: %s \n",
                    localPathNew, strerror(errno));
                return;
            }
        }
        close(fdSrc);
        close(fdDst);
#endif //end of UNIX

        debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile:before SetURL\n");
        }
    }

    // send SO_SET_URl message to inform the new URL
    PLUGIN_MSG msg;
    memset((char*)&msg, 0, sizeof(PLUGIN_MSG));
    msg.msg_id = SO_SET_URL;
    msg.instance_id = (plugin_Int32)instance;
#ifdef UNIX
    msg.wnd_id =(plugin_Int32)(This->window);
    sprintf(msg.url, "file://%s", localPathNew);
#endif //end of UNIX
#ifdef WNT
    msg.wnd_id =(int)(This->fhWnd);
    sprintf(msg.url, "file:///%s", localPathNew);
    DosToUnixPath(msg.url);
#endif //endof WNT
    if(!sendMsg(&msg, sizeof(PLUGIN_MSG), 1))
        debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile send SO_SET_URL return failure \n");

    // send SO_SET_WINDOW message
//    memset((char*)&msg, 0, sizeof(PLUGIN_MSG));
    msg.msg_id = SO_SET_WINDOW;
    msg.instance_id = (plugin_Int32)instance;
//  msg.wnd_id =(plugin_Int32)((PluginInstance*) instance->pdata)->window;
#ifdef UNIX
    msg.wnd_x = This->x;
    msg.wnd_y = This->y;
    msg.wnd_w = This->width;
    msg.wnd_h = This->height;
#endif //end of UNIX
#ifdef WNT
    msg.wnd_x = This->fWindow->x;
    msg.wnd_y = This->fWindow->y;
    msg.wnd_w = This->fWindow->width;
    msg.wnd_h = This->fWindow->height;
#endif //endof WNT
    if(!sendMsg(&msg, sizeof(PLUGIN_MSG), 1))
        debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile send SO_SET_WINDOW return failure \n");
}

NP_DLLPUBLIC void NP_LOADDS
NPP_URLNotify(NPP /*instance*/, const char* /*url*/,
                NPReason /*reason*/, void* /*notifyData*/)
{
}


NP_DLLPUBLIC void NP_LOADDS
NPP_Print(NPP instance, NPPrint* printInfo)
{
    if(printInfo == NULL)
        return;

    if (instance != NULL) {
    /***** Insert NPP_Print code here *****/
        PluginInstance* This = (PluginInstance*) instance->pdata;
        (void)This;
        PLUGIN_MSG msg;
        memset((char*)&msg, 0, sizeof(PLUGIN_MSG));
        msg.msg_id = SO_PRINT;
        msg.instance_id = (plugin_Int32)instance;
        if(!sendMsg(&msg, sizeof(PLUGIN_MSG), 1))
            debug_fprintf(NSP_LOG_APPEND, "NPP_StreamAsFile send SO_SET_WINDOW return failure \n");
       printInfo->mode = TRUE;
    /**************************************/

        if (printInfo->mode == NP_FULL) {
            /*
             * PLUGIN DEVELOPERS:
             *  If your plugin would like to take over
             *  printing completely when it is in full-screen mode,
             *  set printInfo->pluginPrinted to TRUE and print your
             *  plugin as you see fit.  If your plugin wants Netscape
             *  to handle printing in this case, set
             *  printInfo->pluginPrinted to FALSE (the default) and
             *  do nothing.  If you do want to handle printing
             *  yourself, printOne is true if the print button
             *  (as opposed to the print menu) was clicked.
             *  On the Macintosh, platformPrint is a THPrint; on
             *  Windows, platformPrint is a structure
             *  (defined in npapi.h) containing the printer name, port,
             *  etc.
             */

    /***** Insert NPP_Print code here *****\
            void* platformPrint =
                printInfo->print.fullPrint.platformPrint;
            NPBool printOne =
                printInfo->print.fullPrint.printOne;
    \**************************************/

            /* Do the default*/
            printInfo->print.fullPrint.pluginPrinted = FALSE;
        }
        else {  /* If not fullscreen, we must be embedded */
            /*
             * PLUGIN DEVELOPERS:
             *  If your plugin is embedded, or is full-screen
             *  but you returned false in pluginPrinted above, NPP_Print
             *  will be called with mode == NP_EMBED.  The NPWindow
             *  in the printInfo gives the location and dimensions of
             *  the embedded plugin on the printed page.  On the
             *  Macintosh, platformPrint is the printer port; on
             *  Windows, platformPrint is the handle to the printing
             *  device context.
             */

    /***** Insert NPP_Print code here *****\
            NPWindow* printWindow =
                &(printInfo->print.embedPrint.window);
            void* platformPrint =
                printInfo->print.embedPrint.platformPrint;
    \**************************************/
        }
    }
}

}// end of extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
