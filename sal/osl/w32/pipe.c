/*************************************************************************
 *
 *  $RCSfile: pipe.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:23 $
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


#include "system.h"

#include <osl/pipe.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/mutex.h>
#include <osl/semaphor.h>
#include <osl/conditn.h>
#include <osl/interlck.h>
#include <osl/process.h>

#include <rtl/alloc.h>
#include <rtl/memory.h>

#define PIPEWINDOWCLASS "__OSL_PIPE_WNDCLASS__"
#define PIPESYSTEM      "\\\\.\\pipe\\"
#define PIPEPREFIX    "OSL_PIPE_"

#define WND_PIPE_TIMERID    1
#define WND_PIPE_INTERVALL  1000

typedef enum {
    MSG_OFF = 0xDEADBEEF,
    MSG_SYN,
    MSG_FIN,
    MSG_DATA,
    MSG_UNKNOWN
} MessageType;

typedef struct
{
    sal_uInt32 m_Size;
    sal_uInt32 m_ReadPos;
    sal_uInt32 m_WritePos;
    BYTE   m_Data[1];

} oslPipeBuffer;

/*****************************************************************************/
/* oslPipeImpl */
/*****************************************************************************/

typedef struct _oslPipePacket {
    struct _oslPipePacket* m_Next;
    DWORD m_Bytes;
    BYTE  m_Data[1];
} oslPipePacket;

typedef struct _oslPipeImpl {
    oslInterlockedCount  m_Reference;
    HANDLE               m_File;
    HWND                 m_SrcWnd;
    HWND                 m_DstWnd;
    PSECURITY_ATTRIBUTES m_Security;
    HANDLE               m_ReadEvent;
    HANDLE               m_WriteEvent;
    HANDLE               m_AcceptEvent;
    rtl_uString*         m_Name;
    rtl_String*          m_NameA;
    struct _oslPipeImpl* m_Next;
    oslMutex             m_Mutex;
    oslThread            m_Thread;
    oslSemaphore         m_Acception;
    oslCondition         m_ThreadStartUpCond;
    struct _oslPipeImpl* m_Acceptions;
    oslSemaphore         m_Packet;
    struct _oslPipePacket* m_Packets;
    oslPipeError         m_Error;
    sal_Bool             *m_pbAbortAccept;
} oslPipeImpl;


/*****************************************************************************/
/* __osl_PipeWndProc */
/*****************************************************************************/

LRESULT CALLBACK __osl_PipeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    sal_Int32 processMessage(oslPipeImpl *, HWND, COPYDATASTRUCT *);
    void __osl_destroyPipeImpl(oslPipeImpl *);

    switch (Msg)
    {
        case WM_COPYDATA:
        {
            oslPipeImpl* pPipeImpl;

            if (pPipeImpl = (oslPipeImpl*)GetWindowLong(hWnd, 0))
                return (processMessage(pPipeImpl, (HWND)wParam, (COPYDATASTRUCT *)lParam));

            break;
        }

        case WM_CREATE:
        {
            LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
            oslPipeImpl* pPipeImpl = (oslPipeImpl*)lpCS->lpCreateParams;
            SetWindowLong(hWnd, 0, (LONG)pPipeImpl);

            /* if we are not an accepted pipe, signal creation of window;
               for any accepted pipe start a timer for alive checking */
            if (pPipeImpl->m_DstWnd == NULL)
                osl_releaseSemaphore(pPipeImpl->m_Acception);
            else
                SetTimer(hWnd, WND_PIPE_TIMERID, WND_PIPE_INTERVALL, NULL);

            return 0;
        }

        case WM_DESTROY:
        {
            oslPipeImpl* pPipeImpl;

            KillTimer(hWnd, WND_PIPE_TIMERID);

            if (pPipeImpl = (oslPipeImpl*)GetWindowLong(hWnd, 0))
            {
                osl_acquireMutex(pPipeImpl->m_Mutex);

                /* release any waiting threads */
                do
                {
                    osl_releaseSemaphore(pPipeImpl->m_Packet);
                }
                while (! osl_tryToAcquireSemaphore(pPipeImpl->m_Packet));

                pPipeImpl->m_SrcWnd = NULL;

                osl_releaseMutex(pPipeImpl->m_Mutex);
            }

            PostQuitMessage(0);
            return 0;
        }

        case WM_TIMER:
        {
            oslPipeImpl* pPipeImpl;

            /* check if we have lost our partner */
            if ((pPipeImpl = (oslPipeImpl*)GetWindowLong(hWnd, 0)) &&
                (pPipeImpl->m_DstWnd != NULL) && (! IsWindow(pPipeImpl->m_DstWnd)))
            {
                COPYDATASTRUCT CopyData;

                CopyData.dwData = (DWORD)MSG_FIN;
                CopyData.cbData = 0;
                CopyData.lpData = NULL;

                SendMessage(hWnd, WM_COPYDATA,
                            (WPARAM)hWnd, (LPARAM)&CopyData);

            }
            break;
        }
    }

    return DefWindowProc(hWnd, Msg, wParam, lParam);
}


/*****************************************************************************/
/* PipeThreadProc */
/*****************************************************************************/

static void SAL_CALL PipeThreadProc(void *pData)
{
    oslPipeImpl* pPipeImpl = (oslPipeImpl*)pData;

    pPipeImpl->m_SrcWnd = CreateWindow(PIPEWINDOWCLASS, pPipeImpl->m_NameA->buffer,
                                       WS_OVERLAPPEDWINDOW,
                                       0, 0, 0, 0,
                                       (HWND)NULL,        /* no parent */
                                       (HMENU)NULL,       /* use class menu */
                                       (HANDLE)GetModuleHandle(NULL),
                                       (LPSTR)pData);

    /* if we are an accepted pipe, notify creator of new window handle */
    if ((pPipeImpl->m_DstWnd != NULL) && IsWindow(pPipeImpl->m_DstWnd))
    {
        COPYDATASTRUCT CopyData;

        CopyData.dwData = (DWORD)MSG_SYN;
        CopyData.cbData = 0;
        CopyData.lpData = NULL;

        SendMessage(pPipeImpl->m_DstWnd, WM_COPYDATA,
                    (WPARAM)pPipeImpl->m_SrcWnd, (LPARAM)&CopyData);
    }

    /* notify pipe client that this thread starts up and source window was created */
    osl_setCondition(pPipeImpl->m_ThreadStartUpCond);

    while (osl_scheduleThread(pPipeImpl->m_Thread))
    {
        MSG Msg;

        if (IsWindow(pPipeImpl->m_SrcWnd) && GetMessage(&Msg, pPipeImpl->m_SrcWnd, 0, 0))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        else
            break;
    }
}

/*****************************************************************************/
/* osl_create/destroy-PipeImpl */
/*****************************************************************************/

static oslInterlockedCount nPipes = 0;

oslPipeImpl* __osl_createPipeImpl()
{
    oslPipeImpl* pPipeImpl;

    pPipeImpl = (oslPipeImpl*) rtl_allocateZeroMemory(sizeof(oslPipeImpl));

    pPipeImpl->m_Reference = 0;
    pPipeImpl->m_Name = NULL;
    pPipeImpl->m_NameA = NULL;
    pPipeImpl->m_File = INVALID_HANDLE_VALUE;
    pPipeImpl->m_pbAbortAccept = 0;

    if (!IS_NT)
    {
        if (osl_incrementInterlockedCount(&nPipes) == 1)
        {
            WNDCLASS Class;

            Class.hCursor        = NULL;    // this window never shown, so no
            Class.hIcon          = NULL;    // cursor or icon are necessary
            Class.lpszMenuName   = NULL;
            Class.lpszClassName  = PIPEWINDOWCLASS;
            Class.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
            Class.hInstance      = GetModuleHandle(NULL);
            Class.style          = 0;
            Class.lpfnWndProc    = __osl_PipeWndProc;
            Class.cbWndExtra     = sizeof(void *);
            Class.cbClsExtra     = 0;

            OSL_VERIFY(RegisterClass(&Class));
        }
    }

    pPipeImpl->m_Mutex = osl_createMutex();
    pPipeImpl->m_Acception = osl_createSemaphore(0);
    pPipeImpl->m_Packet = osl_createSemaphore(0);
    pPipeImpl->m_ThreadStartUpCond = osl_createCondition();

    pPipeImpl->m_ReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    pPipeImpl->m_WriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    pPipeImpl->m_AcceptEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    return (pPipeImpl);
}

void __osl_destroyPipeImpl(oslPipeImpl *pImpl)
{
    if (pImpl != NULL)
    {
        osl_destroyMutex(pImpl->m_Mutex);
        // terminate the accept
        osl_releaseSemaphore(pImpl->m_Acception);
        osl_destroySemaphore(pImpl->m_Acception);
        osl_destroySemaphore(pImpl->m_Packet);
        osl_destroyCondition(pImpl->m_ThreadStartUpCond);

        if (pImpl->m_Security != NULL)
        {
            rtl_freeMemory(pImpl->m_Security->lpSecurityDescriptor);
            rtl_freeMemory(pImpl->m_Security);
        }

        if (!IS_NT)
        {
            CloseHandle(pImpl->m_ReadEvent);
            CloseHandle(pImpl->m_WriteEvent);
            CloseHandle(pImpl->m_AcceptEvent);

            if (osl_decrementInterlockedCount(&nPipes) == 0)
                UnregisterClass(PIPEWINDOWCLASS, GetModuleHandle(NULL));
        }

        if (pImpl->m_Name)
            rtl_uString_release(pImpl->m_Name);
        if (pImpl->m_NameA)
            rtl_string_release(pImpl->m_NameA);

        rtl_freeMemory(pImpl);
    }
}


/*****************************************************************************/
/* processMessage */
/*****************************************************************************/

static sal_Int32 processMessage(oslPipeImpl *pPipeImpl, HWND Sender, COPYDATASTRUCT *pData)
{
    sal_Int32 Result = -1;

    osl_acquireMutex(pPipeImpl->m_Mutex);

    switch (pData->dwData)
    {
        case MSG_SYN:
            /* new connection */
            if (pPipeImpl->m_DstWnd == NULL)
            {
                oslPipeImpl *pAccept;

                pAccept = __osl_createPipeImpl();

                rtl_string_assign(&( pAccept->m_NameA ) , pPipeImpl->m_NameA );
                pAccept->m_DstWnd = Sender;

                if (pAccept->m_Thread = osl_createSuspendedThread(PipeThreadProc, pAccept))
                    osl_resumeThread(pAccept->m_Thread);

                if (pPipeImpl->m_Acceptions == NULL)
                    pPipeImpl->m_Acceptions = pAccept;
                else
                {
                    oslPipeImpl *pInsert = pPipeImpl->m_Acceptions;
                    while (pInsert->m_Next != NULL)
                        pInsert = pInsert->m_Next;

                    pInsert->m_Next = pAccept;
                }
            }
            else
                pPipeImpl->m_DstWnd = Sender;

            Result = 0;
            osl_releaseSemaphore(pPipeImpl->m_Acception);
            break;

        case MSG_FIN:
            DestroyWindow(pPipeImpl->m_SrcWnd);
            Result = 0;
            break;


        case MSG_DATA:
            if (pData->cbData > 0)
            {
                oslPipePacket *pPacket = calloc(1, sizeof(oslPipePacket) + pData->cbData);

                if (pPacket)
                {
                    pPacket->m_Bytes = pData->cbData;
                    rtl_copyMemory(pPacket->m_Data, pData->lpData, pData->cbData);

                    if (pPipeImpl->m_Packets == NULL)
                        pPipeImpl->m_Packets = pPacket;
                    else
                    {
                        oslPipePacket *pInsert = pPipeImpl->m_Packets;

                        while (pInsert->m_Next != NULL)
                            pInsert = pInsert->m_Next;

                        pInsert->m_Next = pPacket;
                    }

                    Result = pData->cbData;

                    osl_releaseSemaphore(pPipeImpl->m_Packet);
                }
                else
                    OSL_TRACE("Failed to allocate pipe data buffer\n");
            }
            break;

        default:
            OSL_TRACE("Invalid pipe message\n");
            break;
    }

    osl_releaseMutex(pPipeImpl->m_Mutex);

    return (Result);
}

/*****************************************************************************/
/* osl_createPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_createPipe(rtl_uString *strPipeName, oslPipeOptions Options,
                       oslSecurity Security)
{
    rtl_uString* name = NULL;
    rtl_uString* path = NULL;
    rtl_uString* temp = NULL;
    oslPipeImpl* pPipeImpl;

       PSECURITY_ATTRIBUTES  pSecAttr = NULL;

    rtl_uString_newFromAscii(&path, PIPESYSTEM);
    rtl_uString_newFromAscii(&name, PIPEPREFIX);

    if (Security)
    {
        rtl_uString *Ident = NULL;
        rtl_uString *Delim = NULL;

        OSL_VERIFY(osl_getUserIdent(Security, &Ident));
        rtl_uString_newFromAscii(&Delim, "_");

        rtl_uString_newConcat(&temp, name, Ident);
        rtl_uString_newConcat(&name, temp, Delim);

        rtl_uString_release(Ident);
        rtl_uString_release(Delim);
    }
    else
    {
        if (Options & osl_Pipe_CREATE)
        {
            PSECURITY_DESCRIPTOR pSecDesc;

            pSecDesc = (PSECURITY_DESCRIPTOR) rtl_allocateMemory(SECURITY_DESCRIPTOR_MIN_LENGTH);

            /* add a NULL disc. ACL to the security descriptor */
            OSL_VERIFY(InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION));
            OSL_VERIFY(SetSecurityDescriptorDacl(pSecDesc, TRUE, (PACL) NULL, FALSE));

            pSecAttr = rtl_allocateMemory(sizeof(SECURITY_ATTRIBUTES));
            pSecAttr->nLength = sizeof(SECURITY_ATTRIBUTES);
            pSecAttr->lpSecurityDescriptor = pSecDesc;
            pSecAttr->bInheritHandle = TRUE;
        }
    }

    rtl_uString_assign(&temp, name);
    rtl_uString_newConcat(&name, temp, strPipeName);

    /* alloc memory */
    pPipeImpl= __osl_createPipeImpl();
    osl_incrementInterlockedCount(&(pPipeImpl->m_Reference));

    /* build system pipe name */
    rtl_uString_assign(&temp, path);
    rtl_uString_newConcat(&path, temp, name);
    rtl_uString_release(temp);
    temp = NULL;

    if (Options & osl_Pipe_CREATE)
    {
        pPipeImpl->m_Security = pSecAttr;
        rtl_uString_assign(&pPipeImpl->m_Name, name);

        if (IS_NT)
        {
            /* try to open system pipe */
            pPipeImpl->m_File = CreateNamedPipeW(
                path->buffer,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                PIPE_UNLIMITED_INSTANCES,
                4096, 4096,
                NMPWAIT_WAIT_FOREVER,
                pPipeImpl->m_Security);

            if (pPipeImpl->m_File != INVALID_HANDLE_VALUE)
                return pPipeImpl;
        }
        else
        {
            rtl_uString2String(
                &pPipeImpl->m_NameA,
                name->buffer,
                name->length,
                RTL_TEXTENCODING_UTF8,
                OUSTRING_TO_OSTRING_CVTFLAGS);

            /* check if pipe already exists */
            if (FindWindow(PIPEWINDOWCLASS, pPipeImpl->m_NameA->buffer) == NULL)
            {
                if (pPipeImpl->m_Thread = osl_createSuspendedThread(PipeThreadProc, pPipeImpl))
                {
                    osl_resumeThread(pPipeImpl->m_Thread);

                    /* wait for creation */
                    osl_acquireSemaphore(pPipeImpl->m_Acception);

                    /* wait for thread start ups */
                    osl_waitCondition(pPipeImpl->m_ThreadStartUpCond, NULL);

                    return (pPipeImpl);
                }
                else
                    OSL_TRACE("osl_createPipe failed to start thread.\n");
            }
        }
    }
    else
    {
        if (IS_NT)
        {
            /* first try to open system pipe */
            if ((pPipeImpl->m_File = CreateFileW(
                    path->buffer,
                    GENERIC_READ|GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL)) != INVALID_HANDLE_VALUE)
                return (pPipeImpl);
        }
        else
        {
            rtl_uString2String(
                &pPipeImpl->m_NameA,
                name->buffer,
                name->length,
                RTL_TEXTENCODING_UTF8,
                OUSTRING_TO_OSTRING_CVTFLAGS);

            /* next try window emulation */
            if ((pPipeImpl->m_DstWnd = FindWindow(PIPEWINDOWCLASS, pPipeImpl->m_NameA->buffer)) != NULL)
            {
                if (pPipeImpl->m_Thread = osl_createSuspendedThread(PipeThreadProc, pPipeImpl))
                {
                    osl_resumeThread(pPipeImpl->m_Thread);

                    /* wait for creation */
                    osl_acquireSemaphore(pPipeImpl->m_Acception);

                    return (pPipeImpl);
                }
                else
                    OSL_TRACE("osl_createPipe failed to start thread.\n");
            }
        }
    }

    /* if we reach here something went wrong */
    __osl_destroyPipeImpl(pPipeImpl);

    return NULL;
}

/*****************************************************************************/
/* osl_copyPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_copyPipe(oslPipe Pipe)
{
    oslPipeImpl* pPipeImpl;
    oslPipeImpl* pParamPipeImpl;

    OSL_ASSERT(Pipe);

    /* alloc memory */
    pPipeImpl= __osl_createPipeImpl();

    OSL_ASSERT(pPipeImpl);

    if(pPipeImpl==NULL)
        return NULL;

    pParamPipeImpl= (oslPipeImpl*)Pipe;

    osl_incrementInterlockedCount(&(pPipeImpl->m_Reference));

    return (oslPipe)pPipeImpl;
}


/*****************************************************************************/
/* osl_destroyPipe  */
/*****************************************************************************/
void SAL_CALL osl_destroyPipe(oslPipe Pipe)
{
    oslPipeImpl* pPipeImpl, *pAccept;
    oslPipePacket *pData;

    /* socket already invalid */
    if(Pipe==NULL)
        return;

    pPipeImpl= (oslPipeImpl*)Pipe;

    if (osl_decrementInterlockedCount(&(pPipeImpl->m_Reference)) == 0)
    {
        if (IS_NT)
        {
            /* if we have a system pipe close it */
            if (pPipeImpl->m_File != INVALID_HANDLE_VALUE)
            {
                /*          FlushFileBuffers(pPipeImpl->m_File); */
                DisconnectNamedPipe(pPipeImpl->m_File);
                CloseHandle(pPipeImpl->m_File);
            }
        }
        else
        {
            /* send finito message outher side */
            if( pPipeImpl->m_pbAbortAccept )
            {
                *(pPipeImpl->m_pbAbortAccept) = sal_True;
            }

            if ((pPipeImpl->m_DstWnd != 0) && IsWindow(pPipeImpl->m_DstWnd))
            {
                COPYDATASTRUCT CopyData;
                oslPipeImpl*   pPipeImpl;

                OSL_ASSERT(Pipe);

                pPipeImpl= (oslPipeImpl*)Pipe;

                CopyData.dwData = (DWORD)MSG_FIN;
                CopyData.cbData = 0;
                CopyData.lpData = NULL;

                SendMessage(pPipeImpl->m_DstWnd, WM_COPYDATA,
                            (WPARAM)pPipeImpl->m_SrcWnd, (LPARAM)&CopyData);
            }

            /* send finito message himself */
            if ((pPipeImpl->m_SrcWnd!= 0) && IsWindow(pPipeImpl->m_SrcWnd))
            {
                COPYDATASTRUCT CopyData;
                oslPipeImpl*   pPipeImpl;

                OSL_ASSERT(Pipe);

                pPipeImpl= (oslPipeImpl*)Pipe;

                CopyData.dwData = (DWORD)MSG_FIN;
                CopyData.cbData = 0;
                CopyData.lpData = NULL;

                SendMessage(pPipeImpl->m_SrcWnd, WM_COPYDATA,
                        (WPARAM)pPipeImpl->m_SrcWnd, (LPARAM)&CopyData);
            }
        }

        osl_acquireMutex(pPipeImpl->m_Mutex);

        /* delete any outstanding packets */
        while (pData = pPipeImpl->m_Packets)
        {
            pPipeImpl->m_Packets = pData->m_Next;
            rtl_freeMemory(pData);
        }

        /* delete any outstanding connections */
        while (pAccept = pPipeImpl->m_Acceptions)
        {
            pPipeImpl->m_Acceptions = pAccept->m_Next;
            osl_destroyPipe(pAccept);
        }

        osl_releaseMutex(pPipeImpl->m_Mutex);

        __osl_destroyPipeImpl(pPipeImpl);
    }
}

/*****************************************************************************/
/* osl_acceptPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_acceptPipe(oslPipe Pipe)
{
    DWORD        i;
    oslPipeImpl* pPipeImpl = NULL;
    oslPipeImpl* pParamPipeImpl;
    HANDLE       Event;
    OVERLAPPED   os;

    OSL_ASSERT(Pipe);

    pParamPipeImpl= (oslPipeImpl*)Pipe;


    if (IS_NT)
    {
        DWORD nBytesTransfered;
        rtl_uString* path = NULL;
        rtl_uString* temp = NULL;

        OSL_ASSERT (pParamPipeImpl->m_File != INVALID_HANDLE_VALUE);

        Event = pParamPipeImpl->m_AcceptEvent;
        rtl_zeroMemory(&os, sizeof(OVERLAPPED));
        os.hEvent = pParamPipeImpl->m_AcceptEvent;
        ResetEvent(pParamPipeImpl->m_AcceptEvent);

        ConnectNamedPipe(pParamPipeImpl->m_File, &os);

        // blocking call to accept
        if( ! GetOverlappedResult(
                pParamPipeImpl->m_File,
                &os,
                &nBytesTransfered,
                TRUE ) )
        {
            // check, if between ConnectNamePipe and GetOverlappedResult
            // a successful connect took place
            DWORD dw = GetLastError();
            if( ERROR_PIPE_CONNECTED != dw )
            {
                // no successful connect (in general, pipe has been destroyed)
                return 0;
            }
        }

        pPipeImpl= __osl_createPipeImpl();
        OSL_ASSERT(pPipeImpl);

        osl_incrementInterlockedCount(&(pPipeImpl->m_Reference));
        rtl_uString_assign(&pPipeImpl->m_Name, pParamPipeImpl->m_Name);
        pPipeImpl->m_File = pParamPipeImpl->m_File;

        rtl_uString_newFromAscii(&temp, PIPESYSTEM);
        rtl_uString_newConcat(&path, temp, pParamPipeImpl->m_Name);
        rtl_uString_release(temp);

        // prepare for next accept
        pParamPipeImpl->m_File =
            CreateNamedPipeW(path->buffer,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                PIPE_UNLIMITED_INSTANCES,
                4096, 4096,
                NMPWAIT_WAIT_FOREVER,
                pPipeImpl->m_Security);

    }
    else
    {
        sal_Bool bAbort = sal_False;
        Event = (HANDLE)pParamPipeImpl->m_Acception;

        pParamPipeImpl->m_pbAbortAccept = &bAbort;
        while ((i = MsgWaitForMultipleObjects(1, &Event, FALSE, INFINITE, QS_SENDMESSAGE))
               == (WAIT_OBJECT_0 + 1))
        {
            MSG msg;

              while (PeekMessage(&msg, NULL, WM_USER - 1, WM_USER - 1, PM_REMOVE))
               {
                TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }
        }
        pParamPipeImpl->m_pbAbortAccept = 0;

        if( bAbort )
        {
            return 0;
        }

        OSL_ASSERT(pParamPipeImpl->m_DstWnd == 0);

        osl_acquireMutex(pParamPipeImpl->m_Mutex);

        if (pParamPipeImpl->m_Acceptions)
        {
            pPipeImpl = pParamPipeImpl->m_Acceptions;
            pParamPipeImpl->m_Acceptions = pPipeImpl->m_Next;

            /* wait for thread start ups */
            osl_waitCondition(pPipeImpl->m_ThreadStartUpCond, NULL);
        }

        osl_releaseMutex(pParamPipeImpl->m_Mutex);
    }

    return (oslPipe)pPipeImpl;
}

/*****************************************************************************/
/* osl_receivePipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receivePipe(oslPipe Pipe,
                        void* pBuffer,
                        sal_uInt32 BytesToRead)
{
    sal_Int32    nBytes;
    oslPipeImpl* pPipeImpl;

    OSL_ASSERT(Pipe);

    pPipeImpl= (oslPipeImpl*)Pipe;

    /* if we have a system pipe use it */
    if (pPipeImpl->m_File != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED   os;
        rtl_zeroMemory(&os,sizeof(OVERLAPPED));
        os.hEvent = pPipeImpl->m_ReadEvent;

        ResetEvent(pPipeImpl->m_ReadEvent);

        if (! ReadFile(pPipeImpl->m_File, pBuffer, BytesToRead, &nBytes, &os) &&
            ((GetLastError() != ERROR_IO_PENDING) ||
             ! GetOverlappedResult(pPipeImpl->m_File, &os, &nBytes, TRUE)))
        {
            DWORD lastError = GetLastError();

            if (lastError == ERROR_MORE_DATA)
                nBytes = BytesToRead;
              else
              {
                  if (lastError == ERROR_PIPE_NOT_CONNECTED)
                    nBytes = 0;
                else
                    nBytes = -1;

                 pPipeImpl->m_Error = osl_Pipe_E_ConnectionAbort;
            }
        }
    }
    else
    {
        if (osl_tryToAcquireSemaphore(pPipeImpl->m_Packet) ||
            (IsWindow(pPipeImpl->m_SrcWnd) && IsWindow(pPipeImpl->m_DstWnd) &&
              osl_acquireSemaphore(pPipeImpl->m_Packet)))
        {
            osl_acquireMutex(pPipeImpl->m_Mutex);

            if (pPipeImpl->m_Packets != NULL)
            {
                oslPipePacket *pData;

                pData = pPipeImpl->m_Packets;

                nBytes = min(BytesToRead, pData->m_Bytes);

                rtl_copyMemory(pBuffer, pData->m_Data, nBytes);

                if (nBytes < (sal_Int32)pData->m_Bytes)
                {
                    rtl_copyMemory(pData->m_Data, &pData->m_Data[nBytes], pData->m_Bytes - nBytes);
                    pData->m_Bytes -= nBytes;

                    osl_releaseSemaphore(pPipeImpl->m_Packet);
                }
                else
                {
                    pPipeImpl->m_Packets = pData->m_Next;
                    rtl_freeMemory(pData);
                }
            }
            else
                nBytes = 0;

            osl_releaseMutex(pPipeImpl->m_Mutex);
        }
        else
            nBytes = 0;
    }

    return (nBytes);
}

/*****************************************************************************/
/* osl_sendPipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendPipe(oslPipe Pipe,
                       const void* pBuffer,
                       sal_uInt32 BytesToSend)
{
    sal_Int32        nBytes = -1;
    oslPipeImpl*   pPipeImpl;

    OSL_ASSERT(Pipe);

    pPipeImpl= (oslPipeImpl*)Pipe;

    if (pPipeImpl->m_File != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED   os;
        rtl_zeroMemory(&os, sizeof(OVERLAPPED));
        os.hEvent = pPipeImpl->m_WriteEvent;
        ResetEvent(pPipeImpl->m_WriteEvent);

        if (! WriteFile(pPipeImpl->m_File, pBuffer, BytesToSend, &nBytes, &os) &&
            ((GetLastError() != ERROR_IO_PENDING) ||
              ! GetOverlappedResult(pPipeImpl->m_File, &os, &nBytes, TRUE)))
        {
              if (GetLastError() == ERROR_PIPE_NOT_CONNECTED)
                nBytes = 0;
            else
                nBytes = -1;

             pPipeImpl->m_Error = osl_Pipe_E_ConnectionAbort;
        }
    }
    else
    {
        COPYDATASTRUCT CopyData;

        CopyData.dwData = (DWORD)MSG_DATA;
        CopyData.cbData = BytesToSend;
        CopyData.lpData = (void *)pBuffer;

        if (IsWindow(pPipeImpl->m_DstWnd))
            return (SendMessage(pPipeImpl->m_DstWnd, WM_COPYDATA,
                                (WPARAM)pPipeImpl->m_SrcWnd, (LPARAM)&CopyData));

        pPipeImpl->m_Error = osl_Pipe_E_ConnectionAbort;
    }

    return (nBytes);
}


/*****************************************************************************/
/* osl_getLastPipeError  */
/*****************************************************************************/
oslPipeError SAL_CALL osl_getLastPipeError(oslPipe Pipe)
{
    oslPipeError Error;
    oslPipeImpl* pPipeImpl;

    pPipeImpl= (oslPipeImpl*)Pipe;

    if (pPipeImpl != NULL)
    {
        Error = pPipeImpl->m_Error;
        pPipeImpl->m_Error = osl_Pipe_E_None;
    }
    else
        Error = osl_Pipe_E_None;

    return (Error);
}

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.21  2000/09/18 14:29:02  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.20  2000/08/17 16:55:06  jbu
*    #77354# WIN98 pipe runs now, NT acceptPipe changed, so that accept retuns a null pipe on closing accepting pipe
*
*    Revision 1.19  2000/05/29 16:35:50  hro
*    SRC591: Explicite SAL_CALL calling convention
*
*    Revision 1.18  2000/05/22 15:29:08  obr
*    unicode changes channel/pipe
*
*    Revision 1.17  2000/04/07 13:41:04  obr
*    Unicode changes
*
*    Revision 1.16  1999/10/27 15:13:35  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.15  1999/04/29 10:09:11  ts
*    #65460# osl_acceptPipe() warted auf des anlegen des Windows in der PipeWindowProc
*
*    Revision 1.14  1999/04/22 16:25:45  tlx
*    #64892# Flush beim destroyPipe rausgenommen wg. Deadlock
*
*    Revision 1.13  1999/04/22 13:25:30  rh
*    #64126# Implemented TIMER for alice checking, and releasing of receiving threads
*
*    Revision 1.12  1999/03/11 14:47:20  ts
*    #58701# RefCounting an oslPipeImpl gerichtet, DestroyWindow() im richtigen Thread, nPipes geschuetzt
*
*    Revision 1.11  1999/03/04 10:01:23  rh
*    #62429 code cleanup
*
*    Revision 1.10  1999/03/02 15:21:25  rh
*    #62321 use "UNKNOWN" as user name, if it could not be determined
*
*    Revision 1.9  1999/02/09 09:34:32  rh
*    #61503, #61593 Bugfix: receiving any message on pipe window -> acceptPipe fails
*
*    Revision 1.8  1999/01/20 19:09:49  jsc
*    #61011# Typumstellung
*
*    Revision 1.7  1998/12/08 13:41:05  ts
*    #52712# Pipe-Bugs unter NT behoben
*
*    Revision 1.6  1998/12/07 17:44:13  ts
*    #52712# pipes unter NT
*
*    Revision 1.5  1998/12/07 16:04:09  rh
*    #53351# added system pipes and emulation for pipes via messages
*
*    Revision 1.4  1998/12/07 14:07:01  ts
*    #52712# systempipes
*
*    Revision 1.3  1998/12/04 15:11:51  ts
*    #52712# pipes
*
*    Revision 1.2  1998/03/22 13:36:14  rh
*    Chnages for Win16
*
*    Revision 1.1  1998/03/13 15:07:44  rh
*    Cleanup of enum chaos and implemntation of pipes
*
*************************************************************************/
