/*************************************************************************
 *
 *  $RCSfile: pipe.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-14 16:30:30 $
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
    sal_Int32 m_Bytes;
    BYTE  m_Data[1];
} oslPipePacket;

struct oslPipeImpl {
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
    oslPipe              m_Next;
    oslMutex             m_Mutex;
    oslThread            m_Thread;
    oslSemaphore         m_Acception;
    oslCondition         m_ThreadStartUpCond;
    oslPipe              m_Acceptions;
    oslSemaphore         m_Packet;
    struct _oslPipePacket* m_Packets;
    oslPipeError         m_Error;
    sal_Bool             *m_pbAbortAccept;  // TODO : remove this after osl_closePipe MUST change
    sal_Bool             m_bClosed;
};


/*****************************************************************************/
/* __osl_PipeWndProc */
/*****************************************************************************/

LRESULT CALLBACK __osl_PipeWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    sal_Int32 processMessage(oslPipe, HWND, COPYDATASTRUCT *);
    void __osl_destroyPipeImpl(oslPipe);

    switch (Msg)
    {
        case WM_COPYDATA:
        {
            oslPipe pPipe;

            if (pPipe = (oslPipe)GetWindowLong(hWnd, 0))
                return (processMessage(pPipe, (HWND)wParam, (COPYDATASTRUCT *)lParam));

            break;
        }

        case WM_CREATE:
        {
            LPCREATESTRUCT lpCS = (LPCREATESTRUCT)lParam;
            oslPipe pPipe = (oslPipe)lpCS->lpCreateParams;
            SetWindowLong(hWnd, 0, (LONG)pPipe);

            /* if we are not an accepted pipe, signal creation of window;
               for any accepted pipe start a timer for alive checking */
            if (pPipe->m_DstWnd == NULL)
                osl_releaseSemaphore(pPipe->m_Acception);
            else
                SetTimer(hWnd, WND_PIPE_TIMERID, WND_PIPE_INTERVALL, NULL);

            return 0;
        }

        case WM_DESTROY:
        {
            oslPipe pPipe;

            KillTimer(hWnd, WND_PIPE_TIMERID);

            if (pPipe = (oslPipe)GetWindowLong(hWnd, 0))
            {
                osl_acquireMutex(pPipe->m_Mutex);

                /* release any waiting threads */
                do
                {
                    osl_releaseSemaphore(pPipe->m_Packet);
                }
                while (! osl_tryToAcquireSemaphore(pPipe->m_Packet));

                pPipe->m_SrcWnd = NULL;

                osl_releaseMutex(pPipe->m_Mutex);
            }

            PostQuitMessage(0);
            return 0;
        }

        case WM_TIMER:
        {
            oslPipe pPipe;

            /* check if we have lost our partner */
            if ((pPipe = (oslPipe)GetWindowLong(hWnd, 0)) &&
                (pPipe->m_DstWnd != NULL) && (! IsWindow(pPipe->m_DstWnd)))
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
    oslPipe pPipe = (oslPipe)pData;

    pPipe->m_SrcWnd = CreateWindow(PIPEWINDOWCLASS, pPipe->m_NameA->buffer,
                                   WS_OVERLAPPEDWINDOW,
                                   0, 0, 0, 0,
                                   (HWND)NULL,        /* no parent */
                                   (HMENU)NULL,       /* use class menu */
                                   (HANDLE)GetModuleHandle(NULL),
                                   (LPSTR)pData);

    /* if we are an accepted pipe, notify creator of new window handle */
    if ((pPipe->m_DstWnd != NULL) && IsWindow(pPipe->m_DstWnd))
    {
        COPYDATASTRUCT CopyData;

        CopyData.dwData = (DWORD)MSG_SYN;
        CopyData.cbData = 0;
        CopyData.lpData = NULL;

        SendMessage(pPipe->m_DstWnd, WM_COPYDATA,
                    (WPARAM)pPipe->m_SrcWnd, (LPARAM)&CopyData);
    }

    /* notify pipe client that this thread starts up and source window was created */
    osl_setCondition(pPipe->m_ThreadStartUpCond);

    while (osl_scheduleThread(pPipe->m_Thread))
    {
        MSG Msg;

        if (IsWindow(pPipe->m_SrcWnd) && GetMessage(&Msg, pPipe->m_SrcWnd, 0, 0))
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

oslPipe __osl_createPipeImpl()
{
    oslPipe pPipe;

    pPipe = (oslPipe) rtl_allocateZeroMemory(sizeof(struct oslPipeImpl));

    pPipe->m_bClosed = sal_False;
    pPipe->m_Reference = 0;
    pPipe->m_Name = NULL;
    pPipe->m_NameA = NULL;
    pPipe->m_File = INVALID_HANDLE_VALUE;
    pPipe->m_pbAbortAccept = 0;

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

    pPipe->m_Mutex = osl_createMutex();
    pPipe->m_Acception = osl_createSemaphore(0);
    pPipe->m_Packet = osl_createSemaphore(0);
    pPipe->m_ThreadStartUpCond = osl_createCondition();

    pPipe->m_ReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    pPipe->m_WriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    pPipe->m_AcceptEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    return pPipe;
}

void __osl_destroyPipeImpl(oslPipe pPipe)
{
    if (pPipe != NULL)
    {
        osl_destroyMutex(pPipe->m_Mutex);
        // terminate the accept
        osl_releaseSemaphore(pPipe->m_Acception);
        osl_destroySemaphore(pPipe->m_Acception);
        osl_destroySemaphore(pPipe->m_Packet);
        osl_destroyCondition(pPipe->m_ThreadStartUpCond);

        if (pPipe->m_Security != NULL)
        {
            rtl_freeMemory(pPipe->m_Security->lpSecurityDescriptor);
            rtl_freeMemory(pPipe->m_Security);
        }

        if (!IS_NT)
        {
            CloseHandle(pPipe->m_ReadEvent);
            CloseHandle(pPipe->m_WriteEvent);
            CloseHandle(pPipe->m_AcceptEvent);

            if (osl_decrementInterlockedCount(&nPipes) == 0)
                UnregisterClass(PIPEWINDOWCLASS, GetModuleHandle(NULL));
        }

        if (pPipe->m_Name)
            rtl_uString_release(pPipe->m_Name);
        if (pPipe->m_NameA)
            rtl_string_release(pPipe->m_NameA);

        rtl_freeMemory(pPipe);
    }
}


/*****************************************************************************/
/* processMessage */
/*****************************************************************************/

static sal_Int32 processMessage(oslPipe pPipe, HWND Sender, COPYDATASTRUCT *pData)
{
    sal_Int32 Result = -1;

    osl_acquireMutex(pPipe->m_Mutex);

    switch (pData->dwData)
    {
        case MSG_SYN:
            /* new connection */
            if (pPipe->m_DstWnd == NULL)
            {
                oslPipe pAccept;

                pAccept = __osl_createPipeImpl();

                rtl_string_assign(&( pAccept->m_NameA ) , pPipe->m_NameA );
                pAccept->m_DstWnd = Sender;

                if (pAccept->m_Thread = osl_createSuspendedThread(PipeThreadProc, pAccept))
                    osl_resumeThread(pAccept->m_Thread);

                if (pPipe->m_Acceptions == NULL)
                    pPipe->m_Acceptions = pAccept;
                else
                {
                    oslPipe pInsert = pPipe->m_Acceptions;
                    while (pInsert->m_Next != NULL)
                        pInsert = pInsert->m_Next;

                    pInsert->m_Next = pAccept;
                }
            }
            else
                pPipe->m_DstWnd = Sender;

            Result = 0;
            osl_releaseSemaphore(pPipe->m_Acception);
            break;

        case MSG_FIN:
            DestroyWindow(pPipe->m_SrcWnd);
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

                    if (pPipe->m_Packets == NULL)
                        pPipe->m_Packets = pPacket;
                    else
                    {
                        oslPipePacket *pInsert = pPipe->m_Packets;

                        while (pInsert->m_Next != NULL)
                            pInsert = pInsert->m_Next;

                        pInsert->m_Next = pPacket;
                    }

                    Result = pData->cbData;

                    osl_releaseSemaphore(pPipe->m_Packet);
                }
                else
                    OSL_TRACE("Failed to allocate pipe data buffer\n");
            }
            break;

        default:
            OSL_TRACE("Invalid pipe message\n");
            break;
    }

    osl_releaseMutex(pPipe->m_Mutex);

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
    oslPipe pPipe;

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
    pPipe= __osl_createPipeImpl();
    osl_incrementInterlockedCount(&(pPipe->m_Reference));

    /* build system pipe name */
    rtl_uString_assign(&temp, path);
    rtl_uString_newConcat(&path, temp, name);
    rtl_uString_release(temp);
    temp = NULL;

    if (Options & osl_Pipe_CREATE)
    {
        pPipe->m_Security = pSecAttr;
        rtl_uString_assign(&pPipe->m_Name, name);

        if (IS_NT)
        {
            /* try to open system pipe */
            pPipe->m_File = CreateNamedPipeW(
                path->buffer,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                PIPE_UNLIMITED_INSTANCES,
                4096, 4096,
                NMPWAIT_WAIT_FOREVER,
                pPipe->m_Security);

            if (pPipe->m_File != INVALID_HANDLE_VALUE)
                return pPipe;
        }
        else
        {
            rtl_uString2String(
                &pPipe->m_NameA,
                name->buffer,
                name->length,
                RTL_TEXTENCODING_UTF8,
                OUSTRING_TO_OSTRING_CVTFLAGS);

            /* check if pipe already exists */
            if (FindWindow(PIPEWINDOWCLASS, pPipe->m_NameA->buffer) == NULL)
            {
                if (pPipe->m_Thread = osl_createSuspendedThread(PipeThreadProc, pPipe))
                {
                    osl_resumeThread(pPipe->m_Thread);

                    /* wait for creation */
                    osl_acquireSemaphore(pPipe->m_Acception);

                    /* wait for thread start ups */
                    osl_waitCondition(pPipe->m_ThreadStartUpCond, NULL);

                    return (pPipe);
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
            /* free instance should be available first */
            WaitNamedPipeW(path->buffer, NMPWAIT_WAIT_FOREVER);

            /* first try to open system pipe */
            if ((pPipe->m_File = CreateFileW(
                    path->buffer,
                    GENERIC_READ|GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    NULL)) != INVALID_HANDLE_VALUE)
                return (pPipe);
        }
        else
        {
            rtl_uString2String(
                &pPipe->m_NameA,
                name->buffer,
                name->length,
                RTL_TEXTENCODING_UTF8,
                OUSTRING_TO_OSTRING_CVTFLAGS);

            /* next try window emulation */
            if ((pPipe->m_DstWnd = FindWindow(PIPEWINDOWCLASS, pPipe->m_NameA->buffer)) != NULL)
            {
                if (pPipe->m_Thread = osl_createSuspendedThread(PipeThreadProc, pPipe))
                {
                    osl_resumeThread(pPipe->m_Thread);

                    /* wait for creation */
                    osl_acquireSemaphore(pPipe->m_Acception);

                    return (pPipe);
                }
                else
                    OSL_TRACE("osl_createPipe failed to start thread.\n");
            }
        }
    }

    /* if we reach here something went wrong */
    __osl_destroyPipeImpl(pPipe);

    return NULL;
}

void SAL_CALL osl_acquirePipe( oslPipe pPipe )
{
    osl_incrementInterlockedCount( &(pPipe->m_Reference) );
}

void SAL_CALL osl_releasePipe( oslPipe pPipe )
{
//      OSL_ASSERT( pPipe );

    if( 0 == pPipe )
        return;

    if( 0 == osl_decrementInterlockedCount( &(pPipe->m_Reference) ) )
    {
        if( ! pPipe->m_bClosed )
            osl_closePipe( pPipe );

        __osl_destroyPipeImpl( pPipe );
    }
}
/*****************************************************************************/
/* osl_copyPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_copyPipe(oslPipe pPipe)
{
    osl_acquirePipe( pPipe );
    return pPipe;
}


/*****************************************************************************/
/* osl_destroyPipe  */
/*****************************************************************************/
void SAL_CALL osl_destroyPipe(oslPipe pPipe)
{
    osl_releasePipe( pPipe );
}

void SAL_CALL osl_closePipe( oslPipe pPipe )
{
    if( pPipe && ! pPipe->m_bClosed )
    {
        oslPipe pAccept;
        oslPipePacket *pData;

        pPipe->m_bClosed = sal_True;
        if (IS_NT)
        {
            /* if we have a system pipe close it */
            if (pPipe->m_File != INVALID_HANDLE_VALUE)
            {
                /*          FlushFileBuffers(pPipe->m_File); */
                DisconnectNamedPipe(pPipe->m_File);
                CloseHandle(pPipe->m_File);
            }
        }
        else
        {
            /* send finito message outher side */
            if( pPipe->m_pbAbortAccept )
            {
                *(pPipe->m_pbAbortAccept) = sal_True;
            }

            if ((pPipe->m_DstWnd != 0) && IsWindow(pPipe->m_DstWnd))
            {
                COPYDATASTRUCT CopyData;

                CopyData.dwData = (DWORD)MSG_FIN;
                CopyData.cbData = 0;
                CopyData.lpData = NULL;

                SendMessage(pPipe->m_DstWnd, WM_COPYDATA,
                            (WPARAM)pPipe->m_SrcWnd, (LPARAM)&CopyData);
            }

            /* send finito message himself */
            if ((pPipe->m_SrcWnd!= 0) && IsWindow(pPipe->m_SrcWnd))
            {
                COPYDATASTRUCT CopyData;

                CopyData.dwData = (DWORD)MSG_FIN;
                CopyData.cbData = 0;
                CopyData.lpData = NULL;

                SendMessage(pPipe->m_SrcWnd, WM_COPYDATA,
                        (WPARAM)pPipe->m_SrcWnd, (LPARAM)&CopyData);
            }
        }

        osl_acquireMutex(pPipe->m_Mutex);

        /* delete any outstanding packets */
        while (pData = pPipe->m_Packets)
        {
            pPipe->m_Packets = pData->m_Next;
            rtl_freeMemory(pData);
        }

        /* delete any outstanding connections */
        while (pAccept = pPipe->m_Acceptions)
        {
            pPipe->m_Acceptions = pAccept->m_Next;
            osl_destroyPipe(pAccept);
        }

        osl_releaseMutex(pPipe->m_Mutex);
    }
}

/*****************************************************************************/
/* osl_acceptPipe  */
/*****************************************************************************/
oslPipe SAL_CALL osl_acceptPipe(oslPipe pPipe)
{
    DWORD        i;
    oslPipe  pAcceptedPipe = NULL;

    HANDLE       Event;
    OVERLAPPED   os;

    OSL_ASSERT(pPipe);

    if (IS_NT)
    {
        DWORD nBytesTransfered;
        rtl_uString* path = NULL;
        rtl_uString* temp = NULL;

        OSL_ASSERT (pPipe->m_File != INVALID_HANDLE_VALUE);

        Event = pPipe->m_AcceptEvent;
        rtl_zeroMemory(&os, sizeof(OVERLAPPED));
        os.hEvent = pPipe->m_AcceptEvent;
        ResetEvent(pPipe->m_AcceptEvent);

        ConnectNamedPipe(pPipe->m_File, &os);

        // blocking call to accept
        if( ! GetOverlappedResult(
                pPipe->m_File,
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


        pAcceptedPipe = __osl_createPipeImpl();
        OSL_ASSERT(pAcceptedPipe);

        osl_incrementInterlockedCount(&(pAcceptedPipe->m_Reference));
        rtl_uString_assign(&pAcceptedPipe->m_Name, pPipe->m_Name);
        pAcceptedPipe->m_File = pPipe->m_File;

        rtl_uString_newFromAscii(&temp, PIPESYSTEM);
        rtl_uString_newConcat(&path, temp, pPipe->m_Name);
        rtl_uString_release(temp);

        // prepare for next accept
        pPipe->m_File =
            CreateNamedPipeW(path->buffer,
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                PIPE_UNLIMITED_INSTANCES,
                4096, 4096,
                NMPWAIT_WAIT_FOREVER,
                pAcceptedPipe->m_Security);
        rtl_uString_release( path );
    }
    else
    {
        sal_Bool bAbort = sal_False;
        Event = (HANDLE)pPipe->m_Acception;

        pPipe->m_pbAbortAccept = &bAbort;
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
        pPipe->m_pbAbortAccept = 0;

        if( bAbort )
        {
            return 0;
        }

        OSL_ASSERT(pPipe->m_DstWnd == 0);

        osl_acquireMutex(pPipe->m_Mutex);

        if (pPipe->m_Acceptions)
        {
            pAcceptedPipe = pPipe->m_Acceptions;
            pPipe->m_Acceptions = pAcceptedPipe->m_Next;

            /* wait for thread start ups */
            osl_waitCondition(pAcceptedPipe->m_ThreadStartUpCond, NULL);
        }

        osl_releaseMutex(pPipe->m_Mutex);
    }

    return pAcceptedPipe;
}

/*****************************************************************************/
/* osl_receivePipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_receivePipe(oslPipe pPipe,
                        void* pBuffer,
                        sal_Int32 BytesToRead)
{
    sal_Int32    nBytes;

    OSL_ASSERT(pPipe);

    /* if we have a system pipe use it */
    if (pPipe->m_File != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED   os;
        rtl_zeroMemory(&os,sizeof(OVERLAPPED));
        os.hEvent = pPipe->m_ReadEvent;

        ResetEvent(pPipe->m_ReadEvent);

        if (! ReadFile(pPipe->m_File, pBuffer, BytesToRead, &nBytes, &os) &&
            ((GetLastError() != ERROR_IO_PENDING) ||
             ! GetOverlappedResult(pPipe->m_File, &os, &nBytes, TRUE)))
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

                 pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
            }
        }
    }
    else
    {
        if (osl_tryToAcquireSemaphore(pPipe->m_Packet) ||
            (IsWindow(pPipe->m_SrcWnd) && IsWindow(pPipe->m_DstWnd) &&
              osl_acquireSemaphore(pPipe->m_Packet)))
        {
            osl_acquireMutex(pPipe->m_Mutex);

            if (pPipe->m_Packets != NULL)
            {
                oslPipePacket *pData;

                pData = pPipe->m_Packets;

                nBytes = min( BytesToRead, pData->m_Bytes);

                rtl_copyMemory(pBuffer, pData->m_Data, nBytes);

                if (nBytes < (sal_Int32)pData->m_Bytes)
                {
                    rtl_copyMemory(pData->m_Data, &pData->m_Data[nBytes], pData->m_Bytes - nBytes);
                    pData->m_Bytes -= nBytes;

                    osl_releaseSemaphore(pPipe->m_Packet);
                }
                else
                {
                    pPipe->m_Packets = pData->m_Next;
                    rtl_freeMemory(pData);
                }
            }
            else
                nBytes = 0;

            osl_releaseMutex(pPipe->m_Mutex);
        }
        else
            nBytes = 0;
    }

    return (nBytes);
}

/*****************************************************************************/
/* osl_sendPipe  */
/*****************************************************************************/
sal_Int32 SAL_CALL osl_sendPipe(oslPipe pPipe,
                       const void* pBuffer,
                       sal_Int32 BytesToSend)
{
    sal_Int32        nBytes = -1;
    OSL_ASSERT(pPipe);

    if (pPipe->m_File != INVALID_HANDLE_VALUE)
    {
        OVERLAPPED   os;
        rtl_zeroMemory(&os, sizeof(OVERLAPPED));
        os.hEvent = pPipe->m_WriteEvent;
        ResetEvent(pPipe->m_WriteEvent);

        if (! WriteFile(pPipe->m_File, pBuffer, BytesToSend, &nBytes, &os) &&
            ((GetLastError() != ERROR_IO_PENDING) ||
              ! GetOverlappedResult(pPipe->m_File, &os, &nBytes, TRUE)))
        {
              if (GetLastError() == ERROR_PIPE_NOT_CONNECTED)
                nBytes = 0;
            else
                nBytes = -1;

             pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
        }
    }
    else
    {
        COPYDATASTRUCT CopyData;

        CopyData.dwData = (DWORD)MSG_DATA;
        CopyData.cbData = BytesToSend;
        CopyData.lpData = (void *)pBuffer;

        if (IsWindow(pPipe->m_DstWnd))
            return (SendMessage(pPipe->m_DstWnd, WM_COPYDATA,
                                (WPARAM)pPipe->m_SrcWnd, (LPARAM)&CopyData));

        pPipe->m_Error = osl_Pipe_E_ConnectionAbort;
    }

    return (nBytes);
}

sal_Int32 SAL_CALL osl_writePipe( oslPipe pPipe, const void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were send or an error occured */
    sal_Int32 BytesSend= 0;
    sal_Int32 BytesToSend= n;

    OSL_ASSERT(pPipe);
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendPipe(pPipe, pBuffer, BytesToSend);

        /* error occured? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }

    return BytesSend;
}

sal_Int32 SAL_CALL osl_readPipe( oslPipe pPipe, void *pBuffer , sal_Int32 n )
{
    /* loop until all desired bytes were read or an error occured */
    sal_Int32 BytesRead= 0;
    sal_Int32 BytesToRead= n;

    OSL_ASSERT( pPipe );
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receivePipe(pPipe, pBuffer, BytesToRead);

        /* error occured? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        pBuffer= (sal_Char*)pBuffer + RetVal;
    }
    return BytesRead;
}


/*****************************************************************************/
/* osl_getLastPipeError  */
/*****************************************************************************/
oslPipeError SAL_CALL osl_getLastPipeError(oslPipe pPipe)
{
    oslPipeError Error;

    if (pPipe != NULL)
    {
        Error = pPipe->m_Error;
        pPipe->m_Error = osl_Pipe_E_None;
    }
    else
        Error = osl_Pipe_E_None;

    return (Error);
}

