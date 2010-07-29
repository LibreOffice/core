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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

/*
    ToDo / Bugs:

    - DdeInitiate: Eigener Thread?
    - Timeout bei Disconnects (IBM:nicht auf Ack warten!)
    - Konzept Errorhandling (globale/lokale Fehler)
    - Bedeutung/Anwendung Conversation-Contexte
    - Bei Zugriffen auf ConversationHandles WindowHandles checken
    - Namen der Partner-App ermitteln
    - Codepage-Geraffel
*/
#define INCL_DOSPROCESS

#include "ddemlimp.hxx"

#define LOGFILE
#define STATUSFILE
#define DDEDATAFILE
#include "ddemldeb.hxx"


#if defined (OS2) && defined (__BORLANDC__)
#pragma option -w-par
#endif

// static
inline sal_Bool ImpDdeMgr::MyWinDdePostMsg( HWND hWndTo, HWND hWndFrom,
    sal_uInt16 nMsg, PDDESTRUCT pData, sal_uIntPtr nFlags )
{
    sal_Bool bSuccess = WinDdePostMsg( hWndTo,hWndFrom,nMsg,pData,nFlags);
    if( !bSuccess )
    {
        WRITELOG("WinDdePostMsg:Failed!")
        if ( !(nFlags & DDEPM_NOFREE) )
        {
            MyDosFreeMem( pData,"MyWinDdePostMsg" );
        }
    }
    return bSuccess;
}


// *********************************************************************
// ImpDdeMgr
// *********************************************************************

sal_uInt16 ImpDdeMgr::nLastErrInstance = 0;

//
// Conversation-WndProc
// Steuert Transaktionen eines Conversationhandles
//
MRESULT EXPENTRY ConvWndProc(HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2)
{
#if defined(DBG_UTIL) && defined(OV_DEBUG)
    if( nMsg >= WM_DDE_FIRST && nMsg <= WM_DDE_LAST)
    {
        ////WRITELOG("::ConvWndProc:DDE-Msg received")
    }
#endif
    ImpConvWndData* pObj = (ImpConvWndData*)WinQueryWindowULong( hWnd, 0 );
    return pObj->pThis->ConvWndProc( hWnd, nMsg, nPar1, nPar2 );
}

//
// Server-WndProc
// DDE-Server-Window der App
//
MRESULT EXPENTRY ServerWndProc(HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2)
{
#if defined(DBG_UTIL) && defined(OV_DEBUG)
    if( nMsg >= WM_DDE_FIRST && nMsg <= WM_DDE_LAST)
    {
        ////WRITELOG("::ServerWndProc:DDE-Msg received")
    }
#endif
    ImpDdeMgr* pObj = (ImpDdeMgr*)WinQueryWindowULong( hWnd, 0 );
    return pObj->SrvWndProc( hWnd, nMsg, nPar1, nPar2 );
}


inline HDDEDATA ImpDdeMgr::Callback( sal_uInt16 nTransactionType,
    sal_uInt16 nClipboardFormat, HCONV hConversationHandle, HSZ hsz1,
    HSZ hsz2, HDDEDATA hData, sal_uIntPtr nData1, sal_uIntPtr nData2 )
{
    HDDEDATA hRet = (HDDEDATA)0;
    if( pCallback )
        hRet = (*pCallback)(nTransactionType, nClipboardFormat,
            hConversationHandle, hsz1, hsz2, hData, nData1, nData2);
    return hRet;
}



ImpDdeMgr::ImpDdeMgr()
{
    nLastErrInstance    = DMLERR_NO_ERROR;
    pCallback           = 0;
    nTransactFilter     = 0;
    nServiceCount       = 0;
    pServices           = 0;
    pAppTable           = 0;
    pConvTable          = 0;
    pTransTable         = 0;
    bServFilterOn       = sal_True;
    bInSyncTrans        = sal_False;

    CreateServerWnd();
    pData = InitAll();
    if ( !pData )
        nLastErrInstance = DMLERR_MEMORY_ERROR;
    else
        RegisterDDEMLApp();
}

ImpDdeMgr::~ImpDdeMgr()
{
    CleanUp();
    DestroyServerWnd();
// Named Shared Mem vom BS loeschen lassen, da nicht bekannt ist,
// wieviele DDEML-Instanzen die App erzeugt hat, und OS/2
// keinen App-Referenzzaehler fuer shared mem fuehrt.
//  if ( pData )
//      DosFreeMem( pData );
}


sal_Bool ImpDdeMgr::IsSameInstance( HWND hWnd )
{
    TID tid; PID pid;
    WinQueryWindowProcess( hWnd, &pid, &tid );
    return (sal_Bool)(pid == pidThis);
}

HSZ ImpDdeMgr::GetAppName( HWND hWnd )
{
    return 0;
}

// static
ImpDdeMgr* ImpDdeMgr::GetImpDdeMgrInstance( HWND hWnd )
{
    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if( !pData )
        return 0;

    ImpDdeMgr* pResult = 0;
    TID tid; PID pidApp;
    WinQueryWindowProcess( hWnd, &pidApp, &tid );
    HWND* pApp = ImpDdeMgr::GetAppTable( pData );
    sal_uInt16 nCurApp = 0;
    while( nCurApp < pData->nMaxAppCount )
    {
        HWND hCurWin = *pApp;
        if( hCurWin )
        {
            PID pidCurApp;
            WinQueryWindowProcess( hCurWin, &pidCurApp, &tid );
            if( pidCurApp == pidApp )
            {
                pResult = (ImpDdeMgr*)WinQueryWindowULong( hCurWin, 0 );
                break;
            }
        }
        pApp++;
        nCurApp++;
    }
    return pResult;
}





void ImpDdeMgr::CleanUp()
{
    DisconnectAll();
    ImpService* pPtr = pServices;
    if( pPtr )
    {
        for( sal_uInt16 nIdx = 0; nIdx < nServiceCount; nIdx++, pPtr++ )
        {
            HSZ hStr = pPtr->hBaseServName;
            if( hStr )
                DdeFreeStringHandle( hStr );
            hStr = pPtr->hInstServName;
            if( hStr )
                DdeFreeStringHandle( hStr );
        }
        nServiceCount = 0;
        delete pServices;
        pServices = 0;
    }
    bServFilterOn = sal_True;  // default setting DDEML
    UnregisterDDEMLApp();
}

void ImpDdeMgr::RegisterDDEMLApp()
{
    HWND* pPtr = pAppTable;
    HWND hCur;
    sal_uInt16 nPos = 0;
    while( nPos < pData->nMaxAppCount )
    {
        hCur = *pPtr;
        if (hCur == (HWND)0 )
        {
            // in Tabelle stellen
            *pPtr = hWndServer;
            break;
        }
        nPos++;
        pPtr++;
    }
}

void ImpDdeMgr::UnregisterDDEMLApp()
{
    HWND* pPtr = pAppTable;
    sal_uInt16 nPos = 0;
    while( nPos < pData->nMaxAppCount )
    {
        if (*pPtr == hWndServer )
        {
            *pPtr = 0;
            break;
        }
        nPos++;
        pPtr++;
    }
}

// static
ImpDdeMgrData* ImpDdeMgr::AccessMgrData()
{
    ImpDdeMgrData* pData = 0;
    APIRET nRet = DosGetNamedSharedMem((PPVOID)&pData,DDEMLDATA,PAG_READ|PAG_WRITE);
    DBG_ASSERT(!nRet,"DDE:AccessMgrData failed");
    return pData;
}

sal_uInt16 ImpDdeMgr::DdeGetLastError()
{
    sal_uInt16 nErr;
    if ( !pData )
        nErr = DMLERR_DLL_NOT_INITIALIZED;
    else if ( nLastErrInstance )
        nErr = nLastErrInstance;
    else
        nErr = pData->nLastErr;

    nLastErrInstance = 0;
    pData->nLastErr = 0;
    return nErr;
}



sal_uInt16 ImpDdeMgr::DdeInitialize( PFNCALLBACK pCallbackProc, sal_uIntPtr nTransactionFilter )
{
    if ( !nLastErrInstance )
    {
        if ( !pCallbackProc )
        {
            nLastErrInstance = DMLERR_INVALIDPARAMETER;
            return nLastErrInstance;
        }
        pCallback = pCallbackProc;
        nTransactFilter = nTransactionFilter;
        nTransactFilter |= CBF_FAIL_SELFCONNECTIONS;
    }
    return nLastErrInstance;
}


// static
HWND ImpDdeMgr::NextFrameWin( HENUM hEnum )
{
    char aBuf[ 10 ];

    HWND hWnd = WinGetNextWindow( hEnum );
    while( hWnd )
    {
        WinQueryClassName( hWnd, sizeof(aBuf)-1, (PCH)aBuf );
        // Frame-Window ?
        if( !strcmp( aBuf, "#1" ) ) // #define WC_FRAME ((PSZ)0xffff0001L)
            break;
        hWnd = WinGetNextWindow( hEnum );
    }
    return hWnd;
}


HCONV ImpDdeMgr::DdeConnectImp( HSZ hszService,HSZ hszTopic,CONVCONTEXT* pCC)
{
    hCurConv = 0;
    if( !pCC )
        pCC = &aDefaultContext;

    sal_uIntPtr nBufLen;
    PSZ pService = AllocAtomName( (ATOM)hszService, nBufLen );
    PSZ pTopic = AllocAtomName( (ATOM)hszTopic, nBufLen );
#if 0 && defined(OV_DEBUG)
    String aStr("DdeConnectImp Service:");
    aStr += pService;
    aStr += " Topic:";
    aStr += pTopic;
    WRITELOG((char*)(const char*)aStr)
#endif

#if defined(OV_DEBUG)
    if( !strcmp(pService,"oliver voeltz") )
    {
        WRITESTATUS("Table of connections");
        MyDosFreeMem( pTopic,"DdeConnectImp" );
        MyDosFreeMem( pService,"DdeConnectImp" );
        return 0;
    }
#endif

#if 0
    // original pm-fkt benutzen
    HWND hWndCurClient = CreateConversationWnd();
    WinDdeInitiate( hWndCurClient, pService, pTopic, pCC );
    if( GetConversationWndRefCount(hWndCurClient) == 0)
        DestroyConversationWnd( hWndCurClient );
#else
    // eigener Verbindungsaufbau
    HENUM hEnum = WinBeginEnumWindows( HWND_DESKTOP );
    HWND hWndCurSrv = NextFrameWin( hEnum );
    HWND hWndCurClient = CreateConversationWnd();
    while( hWndCurSrv && !hCurConv )
    {
        if( hWndCurSrv != hWndServer ||
            ((nTransactFilter & CBF_FAIL_SELFCONNECTIONS)==0 ))
        {
            // pro DDE-Server ein Conversation-Window erzeugen
            if( GetConversationWndRefCount(hWndCurClient) >= 2)
            {
                DestroyConversationWnd( hWndCurClient );
                hWndCurClient = CreateConversationWnd();
            }
            MyInitiateDde(hWndCurSrv,hWndCurClient,hszService,hszTopic,pCC);
            if( !bListConnect && hCurConv )
                break;
        }
        hWndCurSrv = NextFrameWin( hEnum );
    }

    if( GetConversationWndRefCount(hWndCurClient) == 0)
        DestroyConversationWnd( hWndCurClient );
    WinEndEnumWindows( hEnum );
#endif

    if( !hCurConv )
        nLastErrInstance = DMLERR_NO_CONV_ESTABLISHED;

#if 0 && defined(OV_DEBUG)
    String aCStr( "DdeConnectImp:End ");
    if( nLastErrInstance != DMLERR_NO_CONV_ESTABLISHED )
        aCStr += "(Success)";
    else
        aCStr += "(Failed)";
    WRITELOG((char*)aCStr.GetStr())
#endif

    MyDosFreeMem( pTopic,"DdeConnectImp" );
    MyDosFreeMem( pService,"DdeConnectImp" );
    return hCurConv;
}

HCONV ImpDdeMgr::DdeConnect( HSZ hszService, HSZ hszTopic, CONVCONTEXT* pCC)
{
    ////WRITELOG("DdeConnect:Start")
    bListConnect = sal_False;
    HCONV hResult = DdeConnectImp( hszService, hszTopic, pCC );
    ////WRITELOG("DdeConnect:End")
    ////WRITESTATUS("DdeConnect:End")
    return hResult;
}


HCONVLIST ImpDdeMgr::DdeConnectList( HSZ hszService, HSZ hszTopic,
    HCONVLIST hConvList, CONVCONTEXT* pCC )
{
    nPrevConv = 0;
    ////WRITESTATUS("Before DdeConnectList")
    if( hConvList )
    {
        HCONV hLastConvInList;

        hCurListId      = hConvList;
        ImpHCONV* pConv = pConvTable;
        pConv          += (sal_uInt16)hConvList;
        if( (sal_uInt16)hConvList >= pData->nMaxConvCount ||pConv->hWndThis==0 )
        {
            nLastErrInstance = DMLERR_INVALIDPARAMETER;
            return 0;
        }
        GetLastServer(pData, hConvList, hLastConvInList);
        nPrevConv = (sal_uInt16)hLastConvInList;
    }
    else
        hCurListId = (HCONVLIST)WinCreateWindow( HWND_OBJECT, WC_FRAME,
            CONVLISTNAME, 0,0,0,0,0, HWND_DESKTOP, HWND_BOTTOM, 0,0,0);

    bListConnect = sal_True;
    DdeConnectImp( hszService, hszTopic, pCC );
#if 0 && defined(OV_DEBUG)
    WRITELOG("DdeConnectList:ConnectionList:")
    HCONV hDebug = 0;
    do
    {
        hDebug = DdeQueryNextServer( hCurListId, hDebug);
        String aStr( (sal_uIntPtr)hDebug );
        WRITELOG((char*)(const char*)aStr)
    } while( hDebug );
#endif
    ////WRITESTATUS("After DdeConnectList")
    return (HCONVLIST)hCurListId;
}

DDEINIT* ImpDdeMgr::CreateDDEInitData( HWND hWndDestination, HSZ hszService,
    HSZ hszTopic, CONVCONTEXT* pCC )
{
    sal_uIntPtr nLen1 = 0, nLen2 = 0;
    HATOMTBL hAtomTable = WinQuerySystemAtomTable();

    if( hszService )
        nLen1 = WinQueryAtomLength( hAtomTable, hszService );
    if( hszTopic )
        nLen2 = WinQueryAtomLength( hAtomTable, hszTopic );
    nLen1++; nLen2++;

    DDEINIT* pBuf = 0;

    sal_uIntPtr nLen = sizeof(DDEINIT) + nLen1+ nLen2 + sizeof(CONVCONTEXT);
    if( !(MyDosAllocSharedMem((PPVOID)&pBuf, NULL, nLen,
            PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_GIVEABLE | OBJ_ANY,
            "CreateDDEInitData")))
    {
        memset( pBuf, 0, nLen );

        /*
        PID pid; TID tid;
        WinQueryWindowProcess( hWndDestination, &pid, &tid );
        APIRET nRet = DosGiveSharedMem( pBuf, pid, PAG_READ | PAG_WRITE );
        */

        pBuf->cb = nLen;
        pBuf->offConvContext = sizeof( DDEINIT );
        char* pBase = (char*)pBuf;
        pBase += sizeof(DDEINIT);
        if( pCC )
            memcpy( pBase, pCC, sizeof(CONVCONTEXT) );
        pBase += sizeof(CONVCONTEXT);
        pBuf->pszAppName = pBase;
        if( hszService )
            WinQueryAtomName( hAtomTable, hszService, pBase, nLen1 );
        pBase += nLen1;
        pBuf->pszTopic = pBase;
        if( hszTopic )
            WinQueryAtomName( hAtomTable, hszTopic, pBase, nLen2 );
    }
    return pBuf;
}



void ImpDdeMgr::MyInitiateDde( HWND hWndSrv, HWND hWndClient,
    HSZ hszService, HSZ hszTopic, CONVCONTEXT* pCC )
{
    DDEINIT* pBuf = CreateDDEInitData( hWndSrv, hszService, hszTopic, pCC );
    if( pBuf )
    {
        PID pid; TID tid;
        WinQueryWindowProcess( hWndSrv, &pid, &tid );
        APIRET nRet = DosGiveSharedMem( pBuf, pid, PAG_READ | PAG_WRITE );
        WinSendMsg( hWndSrv,WM_DDE_INITIATE,(MPARAM)hWndClient,(MPARAM)pBuf);
        MyDosFreeMem( pBuf,"MyInitiateDde" );
    }
}

// static
ImpHCONV* ImpDdeMgr::GetFirstServer(ImpDdeMgrData* pData, HCONVLIST hConvList,
    HCONV& rhConv )
{
    ImpHCONV* pPtr = GetConvTable( pData );
    HCONV hConv;
    if( !rhConv )
    {
        pPtr++;
        hConv = 1;
    }
    else
    {
        // Startposition
        pPtr += (sal_uInt16)rhConv;
        hConv = rhConv;
        pPtr++; hConv++;    // auf den naechsten
    }
    while( hConv < pData->nMaxConvCount )
    {
        if( pPtr->hConvList == hConvList )
        {
            rhConv = hConv;
            return pPtr;
        }
        pPtr++;
        hConv++;
    }
    rhConv = 0;
    return 0;
}

// static
ImpHCONV* ImpDdeMgr::GetLastServer(ImpDdeMgrData* pData, HCONVLIST hConvList,
    HCONV& rhConv )
{
    ImpHCONV* pPtr = GetConvTable( pData );
    pPtr += pData->nMaxConvCount;
    pPtr--;
    HCONV hConv = pData->nMaxConvCount;
    hConv--;
    while( hConv > 0 )
    {
        if( pPtr->hConvList == hConvList )
        {
            rhConv = hConv;
            return pPtr;
        }
        pPtr--;
        hConv--;
    }
    rhConv = 0;
    return 0;
}

// static
sal_Bool ImpDdeMgr::CheckConvListId( HCONVLIST hConvListId )
{
    HAB hAB = WinQueryAnchorBlock( (HWND)hConvListId );
    if( hAB )
        return WinIsWindow( hAB, (HWND)hConvListId );
    return sal_False;
    /*
    HAB hAB = WinQueryAnchorBlock( (HWND)hConvListId );
    if( hAB )
    {
        char aBuf[ 16 ];
        WinQueryWindowText( (HWND)hConvListId, sizeof(aBuf), aBuf );
        if( strcmp(aBuf, CONVLISTNAME ) == 0 )
            return sal_True;
    }
    return sal_False;
    */
}

// static
HCONV ImpDdeMgr::DdeQueryNextServer(HCONVLIST hConvList, HCONV hConvPrev)
{
    if( !CheckConvListId( hConvList ) )
        return (HCONV)0;
    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    GetFirstServer( pData, hConvList, hConvPrev );
    return hConvPrev;
}

// static

// Idee: DisconnectAll uebergibt das ServerWindow. Zu jedem HCONV
// wird das Creator-Server-Wnd gespeichert. Disconnect braucht
// dann nur noch die Window-Handles zu vergleichen
sal_Bool ImpDdeMgr::DdeDisconnect( HCONV hConv )
{
    WRITELOG("DdeDisconnect:Start")
    ////WRITESTATUS("DdeDisconnect:Start")

    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if ( !pData )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return sal_False;
    }
    ImpHCONV* pConv = GetConvTable(pData) + (sal_uInt16)hConv;

    if( (sal_uInt16)hConv >= pData->nMaxConvCount || pConv->hWndThis==0 )
    {
        nLastErrInstance = DMLERR_NO_CONV_ESTABLISHED;
        return sal_False;
    }

    PID pidApp; TID tid;
    HWND hWndDummy = WinCreateWindow( HWND_OBJECT, WC_FRAME,
        "Bla", 0, 0,0,0,0, HWND_DESKTOP, HWND_BOTTOM, 0, 0,  0 );
    WinQueryWindowProcess( hWndDummy, &pidApp, &tid );
    WinDestroyWindow( hWndDummy );
    PID pidThis; PID pidPartner;

    HWND hWndThis = pConv->hWndThis;
    HWND hWndPartner = pConv->hWndPartner;

    WinQueryWindowProcess( hWndThis, &pidThis, &tid );
    WinQueryWindowProcess( hWndPartner, &pidPartner, &tid );
    if( pidApp != pidThis && pidApp != pidPartner )
        return sal_True;  // gehoert nicht der App -> ueberspringen

    HCONV hConvPartner = pConv->hConvPartner;

    // die App benachrichtigen, dass alle offenen Advise-Loops
    // beendet werden, egal ob sie vom Server oder Client
    // initiiert wurden. Die Dinger aber nicht loeschen, da sie evtl.
    // noch vom Partner gebraucht werden.
    ImpConvWndData* pObj =
        (ImpConvWndData*)WinQueryWindowULong( pConv->hWndThis, 0 );
    ImpDdeMgr* pThis = pObj->pThis;
    pThis->SendUnadvises( hConv, 0, sal_False ); // alle Formate & NICHT loeschen
    pThis->SendUnadvises( hConvPartner, 0, sal_False ); // alle Formate & NICHT loeschen

    pConv->nStatus |= ST_TERMINATED;

    HAB hAB = WinQueryAnchorBlock( pConv->hWndThis );
    // um die MessageQueue inne Gaenge zu halten
    sal_uIntPtr nTimerId = WinStartTimer( hAB, 0, 0, 50 );

    /*
       Die Partner-App muss ein DDE_TERMINATE posten, auf das
       wir warten muessen, um alle Messages zu bearbeiten, die
       _vor_ dem DdeDisconnect von der Partner-App gepostet
       wurden.
    */
    WRITELOG("DdeDisconnect:Waiting for acknowledge...")
    WinDdePostMsg( hWndPartner, hWndThis, WM_DDE_TERMINATE,
        (PDDESTRUCT)0,DDEPM_RETRY);

    QMSG aQueueMsg;
    sal_Bool bContinue = sal_True;
    while( bContinue )
    {
        if( WinGetMsg( hAB, &aQueueMsg, 0, 0, 0 ))
        {
            WinDispatchMsg( hAB, &aQueueMsg );
            if( (!WinIsWindow( hAB, hWndPartner)) ||
                (pConv->nStatus & ST_TERMACKREC) )
            {
                bContinue = sal_False;
                if( pConv->nStatus & ST_TERMACKREC )
                {
                    WRITELOG("DdeDisconnect: TermAck received")
                }
                else
                {
                    WRITELOG("DdeDisconnect: Partner died")
                }
            }
        }
        else
            bContinue = sal_False;
    }

    WinStopTimer( hAB, 0, nTimerId );

    // WRITELOG("DdeDisconnect:Freeing data")
    // Transaktionstabelle aufraeumen
    FreeTransactions( pData, hConv );
    if( hConvPartner )
        FreeTransactions( pData, hConvPartner );

    FreeConvHandle( pData, hConv );

    WRITELOG("DdeDisconnect:End")
    //WRITESTATUS("DdeDisconnect:End")
    return sal_True;
}

// static
sal_Bool ImpDdeMgr::DdeDisconnectList( HCONVLIST hConvList )
{
    if( !CheckConvListId( hConvList ) )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_INVALIDPARAMETER;
        return sal_False;
    }

    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if ( !pData )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return sal_False;
    }
    HCONV hConv = 0;
    GetFirstServer( pData, hConvList, hConv );
    while( hConv )
    {
        DdeDisconnect( hConv );
        GetFirstServer( pData, hConvList, hConv );
    }
    WinDestroyWindow( (HWND)hConvList );
    return sal_True;
}



// static
HCONV ImpDdeMgr::DdeReconnect(HCONV hConv)
{
    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if ( !pData )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return 0;
    }
    return 0;
}

// static
sal_uInt16 ImpDdeMgr::DdeQueryConvInfo(HCONV hConv, sal_uIntPtr nTransId, CONVINFO* pCI)
{
    if( !pCI || pCI->nSize == 0)
        return 0;
    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if ( !pData )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return 0;
    }
    Transaction* pTrans;
    if( nTransId != QID_SYNC )
    {
        pTrans = ImpDdeMgr::GetTransTable( pData );
        pTrans += nTransId;
        if( nTransId >= pData->nMaxTransCount || pTrans->hConvOwner == 0 )
        {
            ImpDdeMgr::nLastErrInstance = DMLERR_UNFOUND_QUEUE_ID;
            return 0;
        }
    }
    else
        pTrans = 0;

    ImpHCONV* pConv = ImpDdeMgr::GetConvTable( pData );
    pConv += (sal_uIntPtr)hConv;
    if( hConv >= pData->nMaxConvCount || pConv->hWndThis == 0 )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_NO_CONV_ESTABLISHED;
        return 0;
    }

    sal_uInt16 nSize = pCI->nSize;
    if( nSize > sizeof(CONVINFO) )
        nSize = sizeof(CONVINFO);
    CONVINFO aTempInfo;
    memset( &aTempInfo, 0, sizeof(CONVINFO) );
    aTempInfo.nSize         = pCI->nSize;
    aTempInfo.hConvPartner  = pConv->hConvPartner;
    aTempInfo.hszPartner    = pConv->hszPartner;
    aTempInfo.hszServiceReq = pConv->hszServiceReq;
    aTempInfo.hszTopic      = pConv->hszTopic;
    aTempInfo.nStatus       = pConv->nStatus;
    aTempInfo.hConvList     = pConv->hConvList;
    aTempInfo.aConvCtxt     = pConv->aConvContext;
    if( pTrans )
    {
        aTempInfo.nUser     = pTrans->nUser;
        aTempInfo.hszItem   = pTrans->hszItem;
        aTempInfo.nFormat   = pTrans->nFormat;
        aTempInfo.nType     = pTrans->nType;
        aTempInfo.nConvst   = pTrans->nConvst;
        aTempInfo.nLastError= pTrans->nLastError;
    }
    memcpy( pCI, &aTempInfo, nSize );

    return nSize;
}

// static
sal_Bool ImpDdeMgr::DdeSetUserHandle(HCONV hConv, sal_uIntPtr nTransId, sal_uIntPtr hUser)
{
    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if ( !pData )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return sal_False;
    }
    Transaction* pTrans = GetTransTable( pData );
    pTrans += nTransId;
    if( !nTransId || !hConv || nTransId >= pData->nMaxTransCount ||
        pTrans->hConvOwner != hConv )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_INVALIDPARAMETER;
        return sal_False;
    }
    if( !pTrans->hConvOwner)
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_UNFOUND_QUEUE_ID;
        return sal_False;
    }
    pTrans->nUser = hUser;
    return sal_True;
}

sal_Bool ImpDdeMgr::DdeAbandonTransaction( HCONV hConv, sal_uIntPtr nTransId )
{
    ////WRITELOG("DdeAbandonTransaction:Start")
    if( !pData )
    {
        nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return sal_False;
    }
    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hConv;
    if( nTransId < 1 || nTransId >= pData->nMaxTransCount ||
        hConv < 1 || hConv >= pData->nMaxConvCount || !pConv->hWndThis)
    {
        nLastErrInstance = DMLERR_INVALIDPARAMETER;
        return sal_False;
    }
    if( !hConv )
    {
        DBG_ASSERT(0,"DdeAbandonTransaction:NULL-hConv not supported");
        nLastErrInstance = DMLERR_INVALIDPARAMETER;
        return sal_False;
    }
    Transaction* pTrans = pTransTable;
    pTrans += (sal_uInt16)nTransId;
    if( pTrans->hConvOwner != hConv )
    {
        nLastErrInstance = DMLERR_UNFOUND_QUEUE_ID;
        return sal_False;
    }

    if( bInSyncTrans && nTransId == nSyncTransId )
    {
        bSyncAbandonTrans = sal_True;
        return sal_True;
    }
    sal_uInt16 nTempType = pTrans->nType;
    nTempType &= (~XTYPF_MASK);
    if( nTempType == (XTYP_ADVREQ & ~(XTYPF_NOBLOCK)))
    {
        ////WRITELOG("DdeAbandTrans:Advise Loop")

// ----------------------------------------------------------------------
// Der von der Deutschen Bank eingesetzte DDE-Server
// "Invision V2.71 Build 36 Mar 12 1999 V4.8.2" hat einen Bug, der
// dazu fuehrt, dass auf per WM_DDE_TERMINATE geschlossene Verbindungen
// nicht mit einem WM_DDE_TERMINATE geantwortet wird, wenn der
// entsprechende Link vorher per WM_DDE_UNADVISE beendet wurde. Dieser
// Bug tritt ab zwei parallel laufenden Links auf. Auf Wunsch der DB
// wurde das folgende Workaround eingebaut.
// ----------------------------------------------------------------------
#define DEUTSCHE_BANK
#ifndef DEUTSCHE_BANK

// Acknowledge ist beim Unadvise nicht ueblich
//#define SO_DDE_ABANDON_TRANSACTION_WAIT_ACK
#ifdef SO_DDE_ABANDON_TRANSACTION_WAIT_ACK
        DDESTRUCT* pOutDDEData = MakeDDEObject( pConv->hWndPartner,
            pTrans->hszItem, DDE_FACKREQ, 0 /*pTrans->nFormat*/, 0, 0);
#else
        DDESTRUCT* pOutDDEData = MakeDDEObject( pConv->hWndPartner,
            pTrans->hszItem, 0, 0 /*pTrans->nFormat*/, 0, 0);
#endif
        WRITELOG("DdeAbandTrans:Waiting for acknowledge...")
        pTrans->nConvst = XST_UNADVSENT;
        if ( !MyWinDdePostMsg( pConv->hWndPartner, pConv->hWndThis,
            WM_DDE_UNADVISE, pOutDDEData, DDEPM_RETRY ) )
        {
            WRITELOG("DdeAbandTrans:PostMsg Failed")
            return sal_False;
        }
#ifdef SO_DDE_ABANDON_TRANSACTION_WAIT_ACK
        WaitTransState( pTrans, nTransId, XST_UNADVACKRCVD, 0 );
#else
        pTrans->nConvst = XST_UNADVACKRCVD;
#endif

#endif // DEUTSCHE_BANK

        WRITELOG("DdeAbandTrans:Ack received->Freeing transaction")
        FreeTransaction( pData, nTransId );
    }
    WRITELOG("DdeAbandonTransaction:End")
    return sal_True;
}

// wird von einem Server aufgerufen, wenn sich die Daten des
// Topic/Item-Paars geaendert haben. Diese Funktion fordert
// dann den Server auf, die Daten zu rendern (bei Hotlinks) und
// benachrichtigt die Clients
sal_Bool ImpDdeMgr::DdePostAdvise( HSZ hszTopic, HSZ hszItem)
{
    ////WRITELOG("DdePostAdvise:Start")
    ////WRITESTATUS("DdePostAdvise:Start")

#if 0 && defined( OV_DEBUG )
    String aDebStr("DdePostAdvise:Item ");
    aDebStr += (sal_uIntPtr)hszItem;
    WRITELOG((char*)(const char*)aDebStr)
#endif

    Transaction* pTrans = pTransTable;
    pTrans++;
    sal_uInt16 nCurTrans = 1;
    sal_uInt16 nUsedTransactions = pData->nCurTransCount;
    while( nUsedTransactions && nCurTrans < pData->nMaxTransCount )
    {
        HCONV hOwner = pTrans->hConvOwner;
        if( hOwner )
        {
            nUsedTransactions--;
            sal_uInt16 nTempType = pTrans->nType;
            nTempType &= (~XTYPF_MASK);
            if( nTempType == (XTYP_ADVREQ & (~XTYPF_NOBLOCK) ) )
            {
                ImpHCONV* pConv = pConvTable;
                pConv += (sal_uInt16)hOwner;
                if(hszItem == pTrans->hszItem && pConv->hszTopic == hszTopic)
                {
                    if( pConv->hConvPartner )
                    {
                        // Transaktionen werden immer vom Client erzeugt
                        // -> auf Server-HCONV umschalten
                        hOwner = pConv->hConvPartner;
                        pConv = pConvTable;
                        pConv += (sal_uInt16)hOwner;
                    }
                    HWND hWndClient = pConv->hWndPartner;
                    HWND hWndServer = pConv->hWndThis;
#if 0 && defined( OV_DEBUG )
                    String aDebStr("DdePostAdvise: Server:");
                    aDebStr += (sal_uIntPtr)hWndServer;
                    aDebStr += " Client:";
                    aDebStr += (sal_uIntPtr)hWndClient;
                    WRITELOG((char*)(const char*)aDebStr)
#endif
                    DDESTRUCT* pOutDDEData;
                    if ( pTrans->nType & XTYPF_NODATA )
                    {
                        // Warm link
                        ////WRITELOG("DdePostAdvise:Warm link found")
                        pOutDDEData = MakeDDEObject( hWndClient, hszItem,
                            DDE_FNODATA, pTrans->nFormat, 0, 0  );
                    }
                    else
                    {
                        // Hot link
                        ////WRITELOG("DdePostAdvise:Hot link found")
                        pOutDDEData = Callback( XTYP_ADVREQ,
                            pTrans->nFormat, hOwner, hszTopic,
                            hszItem, (HDDEDATA)0, 1, 0 );
                    }
                    if( pOutDDEData )
                    {
                        // todo: FACK_REQ in Out-Data setzen, wenn pTrans->nType & XTYPF_ACKREQ
                        ////WRITELOG("DdePostAdvise:Sending data/notification")
                        sal_Bool bSuccess = MyWinDdePostMsg( hWndClient,
                            hWndServer,WM_DDE_DATA, pOutDDEData, DDEPM_RETRY);
                        if( bSuccess )
                        {
                            // auf Acknowledge des Partners warten ?
                            if( pTrans->nType & XTYPF_ACKREQ )
                            {
                                pTrans->nConvst = XST_ADVDATASENT;
                                // Impl. ist falsch! => korrekt: XST_ADVDATAACKRCVD
                                WaitTransState(pTrans, nCurTrans,
                                    XST_UNADVACKRCVD, 0);
                            }
                        }
                        else
                        {
                            ////WRITELOG("DdePostAdvise:PostMsg failed")
                            nLastErrInstance = DMLERR_POSTMSG_FAILED;
                        }
                    }
                    else
                    {
                        ////WRITELOG("DdePostAdvise:No data to send")
                    }
                }
            }
        }
        nCurTrans++;
        pTrans++;
    }
    ////WRITELOG("DdePostAdvise:End")
    return sal_True;
}

sal_Bool ImpDdeMgr::DdeEnableCallback( HCONV hConv, sal_uInt16 wCmd)
{
    return sal_False;
}

// Rueckgabe: 0==Service nicht registriert; sonst Pointer auf Service-Eintrag
ImpService* ImpDdeMgr::GetService( HSZ hszService )
{
    ImpService* pPtr = pServices;
    if( !pPtr || !hszService )
        return 0;
    for( sal_uIntPtr nIdx = 0; nIdx < nServiceCount; nIdx++, pPtr++ )
    {
        if(( hszService == pPtr->hBaseServName )    ||
           ( hszService == pPtr->hInstServName ) )
            return pPtr;
    }
    return 0;
}


// legt Service in Service-Tabelle ab. Tabelle wird ggf. expandiert
ImpService* ImpDdeMgr::PutService( HSZ hszService )
{
    if( !pServices )
    {
        DBG_ASSERT(nServiceCount==0,"DDE:Bad ServiceCount");
        pServices = new ImpService[ DDEMLSERVICETABLE_INISIZE ];
        memset( pServices, 0, DDEMLSERVICETABLE_INISIZE* sizeof(ImpService));
        nServiceCount = DDEMLSERVICETABLE_INISIZE;
    }
    ImpService* pPtr = pServices;
    sal_uInt16 nCurPos = 0;
    while( pPtr )
    {
        if( pPtr->hBaseServName == 0 )
            break;
        nCurPos++;
        if( nCurPos < nServiceCount )
            pPtr++;
        else
            pPtr = 0;
    }
    if( !pPtr )
    {
        // Tabelle vergroessern
        pPtr = new ImpService[ nServiceCount + DDEMLSERVICETABLE_INISIZE ];
        memset( pPtr, 0, DDEMLSERVICETABLE_INISIZE* sizeof(ImpService));
        memcpy( pPtr, pServices, nServiceCount * sizeof(ImpService) );
#ifdef DBG_UTIL
        memset( pServices, 0, nServiceCount * sizeof(ImpService) );
#endif
        delete pServices;
        pServices = pPtr;
        pPtr += nServiceCount; // zeigt auf erste neue Position
        nServiceCount += DDEMLSERVICETABLE_INISIZE;
    }
    DBG_ASSERT(pPtr->hBaseServName==0,"DDE:Service not empty");
    DBG_ASSERT(pPtr->hInstServName==0,"DDE:Service not empty");

    DdeKeepStringHandle( hszService );

    sal_uInt16 nStrLen = (sal_uInt16)DdeQueryString( hszService, 0, 0, 0);
    char* pBuf = new char[ nStrLen + 1 ];
    DdeQueryString(hszService, pBuf, nStrLen, 850 /* CodePage*/ );
    pBuf[ nStrLen ] = 0;
    String aStr( (sal_uIntPtr)hWndServer );
    aStr += pBuf;
    HSZ hszInstServ = DdeCreateStringHandle( (PSZ)(const char*)pBuf, 850 );
    delete [] pBuf;

    pPtr->hBaseServName = hszService;
    pPtr->hInstServName = hszInstServ;
    return pPtr;
}

void ImpDdeMgr::BroadcastService( ImpService* pService, sal_Bool bRegistered )
{
    DBG_ASSERT(pService,"DDE:No Service");
    if( !pService )
        return;
    MPARAM aMp1 = (MPARAM)(pService->hBaseServName);
    MPARAM aMp2 = (MPARAM)(pService->hInstServName);
    sal_uIntPtr nMsg;
    if( bRegistered )
        nMsg = WM_DDEML_REGISTER;
    else
        nMsg = WM_DDEML_UNREGISTER;

    HWND* pPtr = pAppTable;
    for( sal_uInt16 nPos = 0; nPos < pData->nMaxAppCount; nPos++, pPtr++ )
    {
        HWND hWndCurWin = *pPtr;
        if ( hWndCurWin && hWndCurWin != hWndServer )
            WinSendMsg( hWndCurWin, nMsg, aMp1, aMp2 );
    }
}

HDDEDATA ImpDdeMgr::DdeNameService( HSZ hszService, sal_uInt16 afCmd )
{
    HDDEDATA hRet = (HDDEDATA)1;

    if( afCmd & DNS_FILTERON )
        bServFilterOn = sal_True;
    else if( afCmd & DNS_FILTEROFF )
        bServFilterOn = sal_False;
    ImpService* pService = GetService( hszService );
    sal_Bool bRegister = (sal_Bool)(afCmd & DNS_REGISTER);
    if( bRegister )
    {
        if( !pService )
        {
            pService = PutService( hszService );
            BroadcastService( pService, sal_True );
        }
    }
    else
    {
        if( pService )
        {
            BroadcastService( pService, sal_False );
            DdeFreeStringHandle( pService->hBaseServName );
            pService->hBaseServName = 0;
            DdeFreeStringHandle( pService->hInstServName );
            pService->hInstServName = 0;
        }
            hRet = (HDDEDATA)0;  // Service nicht gefunden
    }
    return hRet;
}


// static
HDDEDATA ImpDdeMgr::DdeClientTransaction(void* pDdeData, sal_uIntPtr cbData,
    HCONV hConv, HSZ hszItem, sal_uInt16 nFormat, sal_uInt16 nType,
    sal_uIntPtr nTimeout, sal_uIntPtr* pResult)
{
    //WRITELOG("DdeClientTransaction:Start")

#if 0 && defined(OV_DEBUG)
    if( nType == XTYP_REQUEST )
    {
        WRITELOG("Start XTYP_REQUEST");
        WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
            "Start XTYP_REQUEST","DdeClientTransaction",
            HWND_DESKTOP,MB_OK);
    }
#endif

    if( pResult )
        *pResult = 0;

    ImpDdeMgrData* pData = ImpDdeMgr::AccessMgrData();
    if ( !pData )
    {
        ImpDdeMgr::nLastErrInstance = DMLERR_DLL_NOT_INITIALIZED;
        return (HDDEDATA)0;
    }

    sal_Bool bIsDdeHandle = (sal_Bool)(pDdeData && cbData==0xffffffff);
    sal_Bool bAppOwnsHandle = (sal_Bool)( bIsDdeHandle &&
        (((DDESTRUCT*)pDdeData)->fsStatus & IMP_HDATAAPPOWNED) );

    sal_Bool bNoData  = (sal_Bool)(nType & XTYPF_NODATA)!=0;
    sal_Bool bAckReq  = (sal_Bool)(nType & XTYPF_ACKREQ)!=0;
    sal_uInt16 nTypeFlags = nType & XTYPF_MASK;
    nType &= (~XTYPF_MASK);

    sal_Bool bSync = (sal_Bool)( nTimeout != TIMEOUT_ASYNC ) != 0;
    if( nType == XTYP_ADVSTART )
        bSync = sal_True;

    // Mapping transaction -> OS/2-Message
    sal_uInt16 nTimeoutErr, nMsg;
    switch ( nType )
    {
        case XTYP_ADVSTART:
            nMsg = WM_DDE_ADVISE;
            nTimeoutErr = DMLERR_ADVACKTIMEOUT;
{
            nTimeout = 60000;
#if 0 && defined(OV_DEBUG)
            char aBuf[ 128 ];
            ImpDdeMgr::DdeQueryString( hszItem,aBuf,127,850);
            String aXXStr("Establishing hotlink ");
            aXXStr += aBuf;
            WRITELOG((char*)aXXStr.GetStr());
#endif

}
            break;

        case XTYP_ADVSTOP:
            nMsg = WM_DDE_UNADVISE;
            nTimeoutErr = DMLERR_UNADVACKTIMEOUT;
            break;

        case XTYP_REQUEST:
            nMsg = WM_DDE_REQUEST;
            nTimeoutErr = DMLERR_DATAACKTIMEOUT;
            break;

        case XTYP_POKE:
            nMsg = WM_DDE_POKE;
            nTimeoutErr = DMLERR_POKEACKTIMEOUT;
            break;

        case XTYP_EXECUTE:
            nMsg = WM_DDE_EXECUTE;
            nTimeoutErr = DMLERR_EXECACKTIMEOUT;
            break;

        default:
            nMsg = 0;
    }
    if(!hConv || (sal_uInt16)hConv>= pData->nMaxConvCount || !nType || !nMsg ||
       (nType != XTYP_EXECUTE && (!hszItem || !nFormat)) )
    {
        WRITELOG("DdeClientTransaction:Invalid parameter")
        ImpDdeMgr::nLastErrInstance = DMLERR_INVALIDPARAMETER;
        if( bIsDdeHandle && !bAppOwnsHandle )
            DdeFreeDataHandle( (HDDEDATA)pDdeData );
        return (HDDEDATA)0;
    }

    // ueber den Conversation handle das ImpDdeMgr-Objekt holen
    ImpHCONV* pConv = GetConvTable( pData );
    pConv += (sal_uInt16)hConv;
    ImpConvWndData* pObj =
        (ImpConvWndData*)WinQueryWindowULong( pConv->hWndThis, 0 );
    ImpDdeMgr* pThis = pObj->pThis;

    if( bSync && pThis->bInSyncTrans )
    {
        WRITELOG("DdeClientTransaction:Already in sync. transaction")
        ImpDdeMgr::nLastErrInstance = DMLERR_REENTRANCY;
        if( bIsDdeHandle && !bAppOwnsHandle )
            DdeFreeDataHandle( (HDDEDATA)pDdeData );
        return (HDDEDATA)0;
    }

    Transaction* pTrans;

    sal_Bool bReqOnAdvLoop = sal_False;
    sal_uIntPtr nTransId = GetTransaction( pData, hConv, hszItem, nFormat );
    if( nTransId )
    {
        // WRITELOG("DdeClientTransaction:Transaction found")
        pTrans = GetTransTable( pData );
        pTrans += (sal_uInt16)nTransId;
        sal_uInt16 nTransType = pTrans->nType;
        nTransType &= (~XTYPF_MASK);
        if( (nType != XTYP_REQUEST && nTransType == nType) ||
             // wird Advise-Loop schon zum requesten missbraucht ?
             (nType == XTYP_REQUEST &&
             nTransType == XTYP_ADVREQ &&
             pTrans->nConvst == XST_WAITING_REQDATA))
        {
            // dieser Kanal ist dicht!
            WRITELOG("DdeClientTransaction:Transaction already used")
            ImpDdeMgr::nLastErrInstance = DMLERR_REENTRANCY;
            if( bIsDdeHandle && !bAppOwnsHandle )
                DdeFreeDataHandle( (HDDEDATA)pDdeData );
            return (HDDEDATA)0;
        }
        else if( nTransType == XTYP_ADVREQ )
        {
            switch( nType )
            {
                case XTYP_ADVSTOP:
                    //WRITELOG("DdeClientTransaction:Stopping advise trans")
                    pTrans->nType = XTYP_ADVSTOP;
                    break;

                case XTYP_ADVSTART:
                    //WRITELOG("DdeClientTransaction:Adj. Advise-Params")
                    pTrans->nType = XTYP_ADVREQ;
                    if( bNoData )
                        pTrans->nType |= XTYPF_NODATA;
                    if( bAckReq )
                        pTrans->nType |= XTYPF_ACKREQ;
                    if( pResult )
                        *pResult = nTransId;
                    return (HDDEDATA)sal_True;

                case XTYP_REQUEST:
                    // WRITELOG("DdeClientTransaction:Using adv trans for req")
                    // nConvst wird unten auf XST_WAITING_REQDATA gesetzt
                    bReqOnAdvLoop = sal_True;
                    break;

                default:
                    WRITELOG("DdeClientTransaction:Invalid parameter")
                    ImpDdeMgr::nLastErrInstance = DMLERR_INVALIDPARAMETER;
                    if( bIsDdeHandle && !bAppOwnsHandle )
                        DdeFreeDataHandle( (HDDEDATA)pDdeData );
                    return (HDDEDATA)sal_False;
            }
        }
    }
    else
    {
        // WRITELOG("DdeClientTransaction:Creating transaction")
        nTransId = CreateTransaction(pData, hConv, hszItem, nFormat, nType);
    }

    pTrans = GetTransTable( pData );
    pTrans += (sal_uInt16)nTransId;
    pTrans->nConvst = XST_WAITING_ACK;
    if( nType == XTYP_REQUEST )
        pTrans->nConvst = XST_WAITING_REQDATA;

    HWND hWndServer = pConv->hWndPartner;
    HWND hWndClient = pConv->hWndThis;

    HDDEDATA pOutDDEData;
    if( bIsDdeHandle )
    {
        if( bAppOwnsHandle )
        {
            // wir muessen leider leider duplizieren, da uns OS/2
            // keine Chance laesst, diesen Status im Datenobjekt
            // zu versenken.
            ////WRITELOG("DdeClientTransaction:Copying handle")
            HDDEDATA pNew;
            HDDEDATA pData = (HDDEDATA)pDdeData;
            if( !(MyDosAllocSharedMem((PPVOID)&pNew, NULL, pData->cbData,
                PAG_COMMIT | PAG_READ | PAG_WRITE | OBJ_GIVEABLE | OBJ_ANY,
                "MakeDDEObject")))
            {
                memcpy( pNew, pData, pData->cbData );
                pOutDDEData = pNew;
            }
            else
            {
                WRITELOG("DdeClientTransaction:No Memory")
                ImpDdeMgr::nLastErrInstance = DMLERR_LOW_MEMORY;
                return (HDDEDATA)0;
            }
        }
        else
            pOutDDEData = (HDDEDATA)pDdeData;
    }
    else
        pOutDDEData=MakeDDEObject(hWndServer,hszItem,0,nFormat,pDdeData,cbData);

    pOutDDEData->fsStatus |= nTypeFlags;

    HDDEDATA pDDEInData = 0;
    if( bSync )
    {
        if( nType != XTYP_REQUEST )
            pOutDDEData->fsStatus |= DDE_FACKREQ;

        // WRITELOG("DdeClientTransaction:Starting sync. trans.")
        pThis->hSyncResponseData = (HDDEDATA)0;
        pThis->nSyncResponseMsg  = 0;
        pThis->bInSyncTrans      = sal_True;
        pThis->nSyncTransId      = nTransId;
        pThis->bSyncAbandonTrans = sal_False;

        if ( !MyWinDdePostMsg( hWndServer, hWndClient, nMsg, pOutDDEData,
                DDEPM_RETRY) )
        {
            WRITELOG("DdeClientTransaction:PostMsg failed")
            nLastErrInstance = DMLERR_POSTMSG_FAILED;
            if( !bReqOnAdvLoop )
                FreeTransaction( pData, nTransId );
            else
            {
                DBG_ASSERT(pTrans->nType==XTYP_ADVREQ,"DDE:Error!")
                pTrans->nConvst = 0;
            }
            return sal_False;
        }
        HAB hAB = WinQueryAnchorBlock( hWndClient );
        sal_uIntPtr nDummyTimer = WinStartTimer( hAB, 0, 0, 50 );
        sal_uIntPtr nTimeoutId = TID_USERMAX - nTransId;
        WinStartTimer( hAB, hWndClient, nTimeoutId, nTimeout );
        QMSG aQueueMsg;
        sal_Bool bLoop = sal_True;
        while( bLoop )
        {
            if( pThis->nSyncResponseMsg )
                bLoop = sal_False;
            else
            {
                if( WinGetMsg(hAB,&aQueueMsg,0,0,0 ))
                {
                    WinDispatchMsg( hAB, &aQueueMsg );
                }
                else
                    bLoop = sal_False;
            }
        }

        WinStopTimer( hAB, hWndClient, nTimeoutId );
        WinStopTimer( hAB, 0, nDummyTimer );

        //
        // Der Speicherblock pOutDDEData muss vom Server geloescht worden sein!
        // Ueberlegen: Nochmal loeschen, falls Server buggy ist, ansonsten
        // platzt uns bald der Adressraum!
        //
        if( !pThis->nSyncResponseMsg )
        {
            // unsere App wurde beendet
            ////WRITELOG("DdeClientTransaction:App terminated")
            return 0;
        }
        pDDEInData = pThis->hSyncResponseData;
        nMsg = pThis->nSyncResponseMsg;
        pThis->hSyncResponseData= 0;
        pThis->nSyncResponseMsg = 0;
        pThis->bInSyncTrans     = sal_False;
        pThis->nSyncTransId     = 0;
        if( !pDDEInData && nMsg != WM_TIMER )
        {
            DBG_ASSERT(0,"Dde:No data!");
            WRITELOG("DdeClientTransaction: No Data!")
            return (HDDEDATA)0;
        }
        switch( nMsg )
        {
            case WM_TIMER:
                WRITELOG("DdeClientTransaction:Timeout!")
                nLastErrInstance = nTimeoutErr;
                if( bReqOnAdvLoop )
                    // auf normalen Loop-Betrieb zurueckschalten
                    pTrans->nConvst = XST_WAITING_ADVDATA;
                break;

            case WM_DDE_ACK:
                {
                // WRITELOG("DdeClientTransaction:Ack received")
                sal_Bool bPositive = (sal_Bool)(pDDEInData->fsStatus & DDE_FACK);
                MyDosFreeMem( pDDEInData,"DdeClientTransaction" );
                pDDEInData = (HDDEDATA)bPositive;
                if( nType == XTYP_ADVSTART && pDDEInData )
                {

#if 0 && defined(OV_DEBUG)
                    char aBuf[ 128 ];
                    ImpDdeMgr::DdeQueryString( pTrans->hszItem,aBuf,128,850);
                    String aXXStr("Hotlink ");
#endif

                    if( bPositive )
                    {
                        pTrans->nType = XTYP_ADVREQ;
                        // Hot/Warmlink, Ack
                        pTrans->nType |= nTypeFlags;
                        // XST_WAITING_ACK ==> XST_WAITING_ADVDATA
                        pTrans->nConvst = XST_WAITING_ADVDATA;

#if 0 && defined(OV_DEBUG)
                        aXXStr += "established ";
                        aXXStr += aBuf;
#endif

                    }

#if 0 && defined(OV_DEBUG)
                    else
                    {
                        aXXStr += "failed ";
                        aXXStr += aBuf;
                    }
                    WRITELOG((char*)aXXStr.GetStr());
#endif

                }
                }
                break;

            case WM_DDE_DATA:
                // WRITELOG("DdeClientTransaction:Data received")
                // WRITEDATA(pDDEInData)
                if( bReqOnAdvLoop )
                {
                    DBG_ASSERT(pTrans->nConvst==XST_WAITING_REQDATA,"DDE:Bad state");
                    DBG_ASSERT(pTrans->nType==XTYP_ADVREQ,"DDE:Bad state");
                    // auf Loop-Betrieb umschalten
                    pTrans->nConvst = XST_WAITING_ADVDATA;
                }
                break;

            default:
                WRITELOG("DdeClientTransaction:Unexpected msg")
                MyDosFreeMem( pDDEInData,"DdeClientTransaction"  );
                pDDEInData = 0;
        }
        pThis->bSyncAbandonTrans = sal_False;
        pThis->bInSyncTrans = sal_False;
        if( pThis->bSyncAbandonTrans && bReqOnAdvLoop )
            pThis->DdeAbandonTransaction( hConv, nTransId );
    }
    else
    {
        // WRITELOG("DdeClientTransaction:Starting async. trans.")
        pDDEInData = (HDDEDATA)MyWinDdePostMsg( hWndServer, hWndClient, nMsg,
            pOutDDEData, DDEPM_RETRY);
        if( !pDDEInData )
        {
            WRITELOG("DdeClientTransaction:PostMsg failed")
            nLastErrInstance = DMLERR_POSTMSG_FAILED;
            if( !bReqOnAdvLoop )
                FreeTransaction( pData, nTransId );
            else
            {
                DBG_ASSERT(pTrans->nType==XTYP_ADVREQ,"DDE:Error!")
                pTrans->nConvst = 0;
            }
        }
        else
        {
            // WRITELOG("DdeClientTransaction:Async trans. success")
            if( pResult )
                *pResult = nTransId;
        }
    }
#if 0 && defined( OV_DEBUG )
    if( nType == XTYP_REQUEST )
    {
        WRITELOG("End XTYP_REQUEST");
        WinMessageBox(HWND_DESKTOP,HWND_DESKTOP,
            "End XTYP_REQUEST","DdeClientTransaction",
            HWND_DESKTOP,MB_OK);
    }
#endif
    //WRITELOG("DdeClientTransaction:End")
    //WRITESTATUS("DdeClientTransaction:End")
    return pDDEInData;
}

MRESULT ImpDdeMgr::DdeRegister( ImpWndProcParams* pParams )
{
    MRESULT nRet = (MRESULT)0;
    if ( !(nTransactFilter & CBF_SKIP_REGISTRATIONS) )
    {
        HSZ hSBaseName = (HSZ)pParams->nPar1;
        HSZ hIBaseName = (HSZ)pParams->nPar2;
        nRet=(MRESULT)Callback(XTYP_REGISTER,0,0,hSBaseName,hIBaseName,0,0,0);
    }
    return nRet;
}

MRESULT ImpDdeMgr::DdeUnregister( ImpWndProcParams* pParams )
{
    MRESULT nRet = (MRESULT)0;
    if ( !(nTransactFilter & CBF_SKIP_UNREGISTRATIONS) )
    {
        HSZ hSBaseName = (HSZ)pParams->nPar1;
        HSZ hIBaseName = (HSZ)pParams->nPar2;
        nRet=(MRESULT)Callback(XTYP_UNREGISTER,0,0,hSBaseName,hIBaseName,0,0,0);
    }
    return nRet;
}

MRESULT ImpDdeMgr::DdeTimeout( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeTimeout:Received")
    if( nSyncResponseMsg )
    {
        ////WRITELOG("DdeTimeout:Trans already processed->ignoring timeout")
        return (MRESULT)1;
    }
    sal_uIntPtr nTimerId = (sal_uIntPtr)pParams->nPar1;
    sal_uIntPtr nTransId = TID_USERMAX - nTimerId;
    Transaction* pTrans = pTransTable;
    pTrans += (sal_uInt16)nTransId;
    if( nTransId < 1 || nTransId >= pData->nMaxTransCount ||
        pTrans->hConvOwner == 0 )
    {
        DBG_ASSERT(0,"DdeTimeout:Invalid TransactionId");
        return (MRESULT)1;
    }
    if( bInSyncTrans && nTransId == nSyncTransId )
    {
        sal_uInt16 nTempType = pTrans->nType;
        nTempType &= (~XTYPF_MASK);
        // advise-loops koennen nur innerhalb synchroner
        // requests timeouts bekommen. die transaktion wird
        // in diesem fall nicht geloescht.
        if( nTempType != (XTYP_ADVREQ & (~XTYPF_NOBLOCK) ))
        {
            ////WRITELOG("DdeTimeout:Freeing transaction")
            FreeTransaction( pData, nTransId );
        }
        nSyncResponseMsg = WM_TIMER;
#if 0 && defined( OV_DEBUG )
        String aMsg("DdeTimeout:Transaction=");
        aMsg += nTransId;
        WRITELOG((char*)(const char*)aMsg)
#endif
    }
    else
    {
        ////WRITELOG("DdeTimeout:Async transaction timed out")
        pTrans->nConvst = XST_TIMEOUT;
    }
    return (MRESULT)1;
}



MRESULT ImpDdeMgr::DdeTerminate( ImpWndProcParams* pParams )
{
    WRITELOG("DdeTerminate:Received")
    HWND hWndThis = pParams->hWndReceiver;
    HWND hWndPartner = (HWND)(pParams->nPar1);

    HCONV hConv = GetConvHandle( pData, hWndThis, hWndPartner );
#if 0 && defined( OV_DEBUG )
    String strDebug("DdeTerminate:ConvHandle=");
    strDebug += (sal_uInt16)hConv;
    WRITELOG((char*)(const char*)strDebug)
#endif
    ImpHCONV* pConv = pConvTable + (sal_uInt16)hConv;
    if( hConv )
    {
        // warten wir auf ein DDE_TERMINATE Acknowledge ?
        if( pConv->nStatus & ST_TERMINATED )
        {
            ////WRITELOG("DdeTerminate:TERMINATE-Ack received")
            pConv->nStatus |= ST_TERMACKREC;
            return (MRESULT)0;  // DdeDisconnect raeumt jetzt auf
        }

        // sind wir Server?, wenn ja: die App benachrichtigen,
        // dass die Advise loops gestoppt wurden und die
        // Transaktionen loeschen

        // OV 26.07.96: Die das TERMINATE empfangende App muss
        // die Transaction-Tabelle abraeumen, egal ob Server oder Client!!
        // Es muessen alle Trans geloescht werden, die als Owner den
        // Client oder den Server haben!
        // if( !(pConv->nStatus & ST_CLIENT ) )
        SendUnadvises( hConv, 0, sal_False );  // alle Formate & nicht loeschen
        SendUnadvises( pConv->hConvPartner, 0, sal_False );

        // wir werden von draussen gekillt
        if ( !(nTransactFilter & CBF_SKIP_DISCONNECTS) )
        {
            Callback( XTYP_DISCONNECT, 0, hConv, 0, 0, 0,
                0, (sal_uIntPtr)IsSameInstance(hWndPartner));
        }

        // kann unsere Partner-App DDEML ?
        if( !(pConv->hConvPartner) )
        {
            // nein, deshalb Transaktionstabelle selbst loeschen
            ////WRITELOG("DdeTerminate:Freeing transactions")
            FreeTransactions( pData, hConv );
        }
    }
    else
        nLastErrInstance = DMLERR_NO_CONV_ESTABLISHED;

#if 0 && defined(OV_DEBUG)
    if( !WinIsWindow(0,hWndPartner))
    {
        WRITELOG("DdeTerminate:hWndPartner not valid")
    }
    if(!WinIsWindow(0,hWndThis))
    {
        WRITELOG("DdeTerminate:hWndThis not valid")
    }
#endif

    if( hConv )
    {
        // hWndThis nicht loeschen, da wir den Handle noch fuer
        // das Acknowledge brauchen
        ////WRITELOG("DdeTerminate:Freeing conversation")
        FreeConvHandle( pData, hConv, sal_False );
    }

    ////WRITELOG("DdeTerminate:Acknowledging DDE_TERMINATE")

#ifdef OV_DEBUG
    DBG_ASSERT(WinIsWindow( 0, hWndThis ),"hWndThis not valid");
#endif

    if( !WinDdePostMsg( hWndPartner, hWndThis, WM_DDE_TERMINATE, 0, DDEPM_RETRY ))
    {
        ////WRITELOG("DdeTerminate:Acknowledging DDE_TERMINATE failed")
    }
    // jetzt hWndThis loeschen
    DestroyConversationWnd( hWndThis );

    return (MRESULT)0;
}


/*
    Zuordnung des Conversationhandles:

    Verbindungsaufbau:
        Client: DdeInitiate( HWNDClient )
        Server: Post( WM_DDE_INITIATEACK( HWNDServer ))
        Client: CreateConvHandle( HWNDClient, HWNDServer )

    Datenaustausch:
        Server: Post(WM_DDE_ACK( HWNDSender ))
        Client: GetConvHandle( HWNDClient, HWNDSender )
*/

MRESULT ImpDdeMgr::ConvWndProc( HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2 )
{
    ImpWndProcParams    aParams;

    MRESULT nRet        = (MRESULT)0;
    aParams.hWndReceiver= hWnd;
    aParams.nPar1       = nPar1;
    aParams.nPar2       = nPar2;

    switch( nMsg )
    {

#ifdef DBG_UTIL
        case WM_DDE_INITIATE :
            DBG_ASSERT(0,"dde:unexpected msg");
            nRet = (MRESULT)sal_True;
            break;
#endif

        case WM_DDE_INITIATEACK : nRet = DdeInitiateAck(&aParams);  break;
        case WM_DDE_ACK         : nRet = DdeAck( &aParams );        break;
        case WM_DDE_ADVISE      : nRet = DdeAdvise( &aParams );     break;
        case WM_DDE_DATA        : nRet = DdeData( &aParams );       break;
        case WM_DDE_EXECUTE     : nRet = DdeExecute( &aParams );    break;
        case WM_DDE_POKE        : nRet = DdePoke( &aParams );       break;
        case WM_DDE_REQUEST     : nRet = DdeRequest( &aParams );    break;
        case WM_DDE_TERMINATE   : nRet = DdeTerminate( &aParams );  break;
        case WM_DDE_UNADVISE    : nRet = DdeUnadvise( &aParams );   break;
        case WM_TIMER           : nRet = DdeTimeout( &aParams );    break;
    }
    return nRet;
}

MRESULT ImpDdeMgr::SrvWndProc( HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2 )
{
    MRESULT nRet = (MRESULT)0;

    ImpWndProcParams aParams;
    aParams.hWndReceiver= hWnd;
    aParams.nPar1       = nPar1;
    aParams.nPar2       = nPar2;

    switch( nMsg )
    {
#ifdef DBG_UTIL
        case WM_DDE_ACK :
        case WM_DDE_ADVISE :
        case WM_DDE_EXECUTE :
        case WM_DDE_POKE :
        case WM_DDE_REQUEST :
        case WM_DDE_UNADVISE :
        case WM_DDE_DATA :
        case WM_DDE_INITIATEACK :
            DBG_ASSERT(0,"dde:unexpected msg");
            nRet = (MRESULT)sal_True;
            break;
#endif

        case WM_DDE_TERMINATE :
            break; // DDE_INITIATE wurde im DDE_INITIATEACK terminiert

        // ein Client will was von uns
        case WM_DDE_INITIATE :
            nRet = DdeInitiate( &aParams );
            break;

        // eine ddeml-faehige App. hat einen Service (typ. AppName) [de]reg.
        case WM_DDEML_REGISTER :
            nRet = DdeRegister( &aParams );
            break;

        case WM_DDEML_UNREGISTER :
            nRet = DdeUnregister( &aParams );
            break;
    };
    return nRet;
}


MRESULT ImpDdeMgr::DdeAck( ImpWndProcParams* pParams )
{
    //WRITELOG("DdeAck:Start")
    HSZ hszItem;
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    if( pInDDEData )
    {
        sal_Bool bPositive = (sal_Bool)(pInDDEData->fsStatus & DDE_FACK ) != 0;
        sal_Bool bBusy = bPositive ? sal_False : (sal_Bool)(pInDDEData->fsStatus & DDE_FBUSY ) != 0;
        sal_Bool bNotProcessed = (sal_Bool)(pInDDEData->fsStatus & DDE_NOTPROCESSED ) != 0;
#if 0 && defined( OV_DEBUG )
        String aDebStr("DdeAck:Received ");
        if( bPositive )
            aDebStr += "(positive)";
        else
            aDebStr += "(negative)";
        if( bBusy )
            aDebStr += "(busy)";
        if( bNotProcessed )
            aDebStr += "(not processed)";
        WRITELOG((char*)(const char*)aDebStr)
#endif
        // ein DDE_ACK niemals bestaetigen (um endlosschleifen zu vermeiden)
        pInDDEData->fsStatus &= (~DDE_FACKREQ);
    }
    else
    {
        //WRITELOG("DdeAck:Received (no data!)")
        return (MRESULT)0;
    }

    HCONV hConv = CheckIncoming(pParams, 0, hszItem);
#ifdef OV_DEBUG
    if( !hConv )
    {
        WRITELOG("DdeAck:HCONV not found")
    }
#endif
    sal_uIntPtr nTransId=GetTransaction(pData,hConv,hszItem,pInDDEData->usFormat);
    if( !nTransId )
    {
        WRITELOG("DdeAck:Transaction not found")
        MyDosFreeMem( pInDDEData,"DdeAck" );
        return (MRESULT)0;
    }

    sal_Bool bThisIsSync = (sal_Bool)( bInSyncTrans && nTransId == nSyncTransId );
#if 0 && defined( OV_DEBUG )
    if( bThisIsSync)
        WRITELOG("DdeAck: sync transaction")
    else
        WRITELOG("DdeAck: async transaction")
#endif
    // pruefen, ob die Transaktion abgeschlossen ist.
    Transaction* pTrans = pTransTable;
    pTrans += (sal_uInt16)nTransId;
    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hConv;

    if( pTrans->nConvst == XST_UNADVSENT )
    {
        //WRITELOG("DdeAck:Unadvise-Ack received")
        pTrans->nConvst = XST_UNADVACKRCVD;
        MyDosFreeMem( pInDDEData,"DdeAck" );
        DdeFreeStringHandle( hszItem );
        return (MRESULT)0;
    }
    if( pTrans->nConvst == XST_ADVDATASENT )
    {
        //WRITELOG("DdeAck:AdvData-Ack received")
        pTrans->nConvst = XST_ADVDATAACKRCVD;
        MyDosFreeMem( pInDDEData,"DdeAck" );
        DdeFreeStringHandle( hszItem );
        return (MRESULT)0;
    }

    sal_uInt16 nType = pTrans->nType;
    nType &= (~XTYPF_MASK);
    // beginn einer advise-loop oder request auf advise-loop ?
    // wenn ja: transaktion nicht loeschen
    sal_Bool bFinished = (sal_Bool)(nType != XTYP_ADVSTART &&
                            nType != (XTYP_ADVREQ & (~XTYPF_NOBLOCK)) );
    if( bFinished )
    {
        if( !bThisIsSync )
        {
            ////WRITELOG("DdeAck:Transaction completed")
            Callback( XTYP_XACT_COMPLETE, pInDDEData->usFormat, hConv,
                pConv->hszTopic, hszItem, (HDDEDATA)0, nTransId, 0 );
        }
        ////WRITELOG("DdeAck:Freeing transaction")
        FreeTransaction( pData, nTransId );
    }

    if( bThisIsSync )
    {
        hSyncResponseData = pInDDEData;
        nSyncResponseMsg = WM_DDE_ACK;
    }
    else
    {
        MyDosFreeMem( pInDDEData,"DdeAck" );
    }

    DdeFreeStringHandle( hszItem );

    return (MRESULT)0;
}


sal_uInt16 ImpDdeMgr::SendUnadvises(HCONV hConvServer,sal_uInt16 nFormat,sal_Bool bFree)
{
    sal_uInt16 nTransFound = 0;
    sal_Bool bCallApp = (sal_Bool)(!(nTransactFilter & CBF_FAIL_ADVISES));
#if 0 && defined( OV_DEBUG )
    String aStr("Unadvising transactions for HCONV=");
    aStr += (sal_uIntPtr)hConvServer;
    aStr += " CallApp:"; aStr += (sal_uInt16)bCallApp;
    WRITELOG((char*)aStr.GetStr())
#endif


    // wenn wir weder loeschen noch die App benachrichtigen sollen,
    // koennen wir gleich wieder returnen
    if( !hConvServer || ( !bFree && !bCallApp ) )
        return 0;

    ImpHCONV* pConvSrv = pConvTable;
    pConvSrv += (sal_uInt16)hConvServer;
    HSZ hszTopic = pConvSrv->hszTopic;

    Transaction* pTrans = pTransTable;
    pTrans++;
    sal_uInt16 nCurTransId = 1;
    sal_uInt16 nCurTransactions = pData->nCurTransCount;
    while( nCurTransactions && nCurTransId < pData->nMaxTransCount )
    {
        if( pTrans->hConvOwner )
            nCurTransactions--;
        if( pTrans->hConvOwner == hConvServer &&
            (pTrans->nType & XTYP_ADVREQ) )
        {
            if( !nFormat || (nFormat == pTrans->nFormat) )
            {
                nTransFound++;
                if( bCallApp )
                {
                    //WRITELOG("SendUnadvises:Notifying App")
                    Callback( XTYP_ADVSTOP, pTrans->nFormat, hConvServer,
                        hszTopic, pTrans->hszItem, 0,0,0 );
                }
                if( bFree )
                    FreeTransaction( pData, (sal_uIntPtr)nCurTransId );
            }
        }
        nCurTransId++;
        pTrans++;
    }
    return nTransFound;
}



HCONV ImpDdeMgr::CheckIncoming( ImpWndProcParams* pParams, sal_uIntPtr nTransMask,
    HSZ& rhszItem )
{
//  ////WRITELOG("CheckIncoming")
    rhszItem = 0;
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    if( !pInDDEData )
    {
        // ////WRITELOG("CheckIncoming:PDDESTRUCT==0")
        return (HCONV)0;
    }

    HWND hWndThis = pParams->hWndReceiver;
    HWND hWndClient = (HWND)pParams->nPar1;

    sal_Bool bReject = (sal_Bool)(nTransactFilter & nTransMask);
    HCONV hConv;
    if( !bReject )
        hConv = GetConvHandle( pData, hWndThis, hWndClient );
    if ( bReject || !hConv )
        return (HCONV)0;

    rhszItem = DdeCreateStringHandle(
        ((char*)(pInDDEData)+pInDDEData->offszItemName), 850 );

    // ////WRITELOG("CheckIncoming:OK");
    return hConv;
}


MRESULT ImpDdeMgr::DdeAdvise( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeAdvise:Received")
    HSZ hszItem;
    HCONV hConv           = CheckIncoming(pParams, CBF_FAIL_ADVISES, hszItem);
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    HWND hWndThis         = pParams->hWndReceiver;
    HWND hWndClient       = (HWND)pParams->nPar1;
    if( !hConv )
    {
        ////WRITELOG("DdeAdvise:Conversation not found")
        pInDDEData->fsStatus &= (~DDE_FACK);
        MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
        DdeFreeStringHandle( hszItem );
        return (MRESULT)0;
    }

    Transaction* pTrans = pTransTable;
    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hConv;

    // existiert schon ein Link auf Topic/Item/Format-Vektor ?

    sal_uIntPtr nTransId=GetTransaction(pData,hConv,hszItem,pInDDEData->usFormat);
    if( nTransId )
    {
        ////WRITELOG("DdeAdvise:Transaction already exists")
        pTrans += (sal_uInt16)nTransId;
        // ist es eine AdviseLoop ?
        sal_uInt16 nTempType = pTrans->nType;
        nTempType &= (~XTYPF_MASK);
        if( nTempType == XTYP_ADVREQ )
        {
            // Flags der laufenden Advise-Loop aktualisieren
            ////WRITELOG("DdeAdvise:Adjusting Advise-Params")
            pTrans->nType = XTYP_ADVREQ;
            if( pInDDEData->fsStatus & DDE_FNODATA )
                pTrans->nType |= XTYPF_NODATA;
            if( pInDDEData->fsStatus & DDE_FACKREQ )
                pTrans->nType |= XTYPF_ACKREQ;
            pInDDEData->fsStatus |= DDE_FACK;
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
            DdeFreeStringHandle( hszItem );
            return (MRESULT)0;
        }
        else if( nTempType != XTYP_ADVSTART )
        {
            ////WRITELOG("DdeAdvise:Not a advise transaction")
            pInDDEData->fsStatus &= (~DDE_FACK);
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
            DdeFreeStringHandle( hszItem );
            return (MRESULT)0;
        }
    }

    if( !nTransId )
    {
        ////WRITELOG("DdeAdvise:Creating Transaction")
        ////WRITESTATUS("DdeAdvise:Creating Transaction")
        nTransId = CreateTransaction( pData, hConv, hszItem,
            pInDDEData->usFormat, XTYP_ADVREQ );
        ////WRITESTATUS("DdeAdvise:Created Transaction")
    }
    if( nTransId )
    {
        pTrans = pTransTable;
        pTrans += (sal_uInt16)nTransId;
        if( pInDDEData->fsStatus & DDE_FNODATA )
            pTrans->nType |= XTYPF_NODATA;
        if( pInDDEData->fsStatus & DDE_FACKREQ )
            pTrans->nType |= XTYPF_ACKREQ;
    }
    else
    {
        ////WRITELOG("DdeAdvise:Cannot create Transaction")
        pInDDEData->fsStatus &= (~DDE_FACK);
        MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
        DdeFreeStringHandle( hszItem );
        return (MRESULT)0;
    }

    ////WRITELOG("DdeAdvise:Calling Server")

    if ( Callback( XTYP_ADVSTART, pInDDEData->usFormat,
        hConv, pConv->hszTopic, hszItem, (HDDEDATA)0, 0, 0 ) )
    {
        //
        // ServerApp erlaubt AdviseLoop
        //
        ////WRITELOG("DdeAdvise:Advise loop established")
        pInDDEData->fsStatus |= DDE_FACK;
        MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
    }
    else
    {
        ////WRITELOG("DdeAdvise:Advise loop not established")
        FreeTransaction( pData, nTransId );
        pInDDEData->fsStatus &= (~DDE_FACK); // DDE_FNOTPROCESSED;
        MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
    }
    ////WRITESTATUS("DdeAdvise:End")
    ////WRITELOG("DdeAdvise:End")
    DdeFreeStringHandle( hszItem );
    return (MRESULT)0;
}

MRESULT ImpDdeMgr::DdeData( ImpWndProcParams* pParams )
{
    WRITELOG("DdeData:Received")
    HSZ hszItem;
    HCONV hConv           = CheckIncoming(pParams, 0, hszItem);
    HWND hWndThis         = pParams->hWndReceiver;
    HWND hWndClient       = (HWND)pParams->nPar1;
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
#if 0 && defined( OV_DEBUG )
    {
        String aStr("DdeData Address:");
        aStr += (sal_uIntPtr)pInDDEData;
        WRITELOG((char*)aStr.GetStr())
    }
#endif

    sal_Bool bSendAck;
    if( pInDDEData && (pInDDEData->fsStatus & DDE_FACKREQ ))
    {
        WRITELOG("DdeData: Ackn requested")
        bSendAck = sal_True;
    }
    else
    {
        WRITELOG("DdeData: Ackn not requested")
        bSendAck = sal_False;
    }

    sal_uIntPtr nTransId = GetTransaction(pData,hConv,hszItem,pInDDEData->usFormat);
    if( !nTransId )
    {
        WRITELOG("DdeData:Transaction not found")
        WRITEDATA(pInDDEData)
        if( bSendAck )
        {
            WRITELOG("DdeData: Posting Ackn")
            pInDDEData->fsStatus &= (~DDE_FACK); // NOTPROCESSED;
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
        }
        else
        {
            MyDosFreeMem( pInDDEData,"DdeData" );
        }
        return (MRESULT)0;
    }

#if 0 && defined( OV_DEBUG )
    if( pInDDEData )
    {
        WRITEDATA(pInDDEData)
    }
#endif

    sal_Bool bThisIsSync = (sal_Bool)( bInSyncTrans && nTransId == nSyncTransId );

    // pruefen, ob die Transaktion abgeschlossen ist.
    Transaction* pTrans = pTransTable;
    pTrans += (sal_uInt16)nTransId;

    if( pTrans->nConvst == XST_WAITING_ACK )
    {
        // dieser Fall kann eintreten, wenn ein Server innerhalb
        // einer WM_DDE_ADVISE-Msg. oder bevor beim Client das
        // Ack eintrifft, Advise-Daten sendet.
        WRITELOG("DdeData:Ignoring unexpected data")
        if( bSendAck )
        {
            WRITELOG("DdeData: Posting Ackn")
            pInDDEData->fsStatus &= (~DDE_FACK); // NOTPROCESSED;
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
        }
        else
        {
            MyDosFreeMem( pInDDEData,"DdeData" );
        }
        return (MRESULT)0;
    }

    ImpHCONV* pConv = pConvTable;
    pConv  += (sal_uInt16)hConv;

    sal_uInt16 nType = pTrans->nType;
    nType &= (~XTYPF_MASK);
    sal_Bool bNotAdviseLoop = (sal_Bool)(nType != (XTYP_ADVREQ & (~XTYPF_NOBLOCK)));
    if( !bThisIsSync )
    {
        // WRITELOG("DdeData:Is async transaction")
        if( bNotAdviseLoop )
        {
            // WRITELOG("DdeData:Transaction completed -> calling client")
            Callback( XTYP_XACT_COMPLETE, pInDDEData->usFormat, hConv,
                pConv->hszTopic, hszItem, pInDDEData, nTransId, 0 );
            // WRITELOG("DdeData:Freeing transaction")
            FreeTransaction( pData, nTransId );
        }
        else
        {
            WRITELOG("DdeData:Advise-Loop -> calling client")
            HDDEDATA pToSend = pInDDEData;
            if( pTrans->nType & XTYPF_NODATA )
            {
                pToSend = 0;
                // WRITELOG("DdeData:Is warm link")
            }
            Callback( XTYP_ADVDATA, pInDDEData->usFormat, hConv,
                pConv->hszTopic, hszItem, pToSend, nTransId, 0 );
        }
        if( bSendAck )
        {
            WRITELOG("DdeData: Posting Ackn")
            pInDDEData->fsStatus = DDE_FACK;
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
        }
        else
            MyDosFreeMem( pInDDEData,"DdeData" );
    }
    else  // synchrone Transaktion (Datenhandle nicht freigeben!)
    {
        // WRITELOG("DdeData:Is sync transaction")
        hSyncResponseData = pInDDEData;
        nSyncResponseMsg = WM_DDE_DATA;
        if( bSendAck )
        {
            pInDDEData->fsStatus |= DDE_FACK;
            WRITELOG("DdeData: Posting Ackn")
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,
                DDEPM_RETRY | DDEPM_NOFREE );
        }
    }

    DdeFreeStringHandle( hszItem );
    // WRITELOG("DdeData:End")
    return (MRESULT)0;
}

MRESULT ImpDdeMgr::DdeExecute( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeExecute:Received")
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    HSZ hszItem;
    HCONV hConv = CheckIncoming(pParams, 0, hszItem);
    sal_Bool bSuccess = sal_False;
    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hConv;
    if ( hConv && !(nTransactFilter & CBF_FAIL_EXECUTES) && pInDDEData )
    {
        if ( Callback( XTYP_EXECUTE, pInDDEData->usFormat, hConv,
            pConv->hszTopic, hszItem, pInDDEData, 0, 0 )
                == (DDESTRUCT*)DDE_FACK )
            bSuccess = sal_True;
    }
    else
    {
        ////WRITELOG("DdeExecute:Not processed")
    }
    if( pInDDEData )
    {
        if( bSuccess )
            pInDDEData->fsStatus |= DDE_FACK;
        else
            pInDDEData->fsStatus &= (~DDE_FACK);
        MyWinDdePostMsg( pConv->hWndPartner, pConv->hWndThis, WM_DDE_ACK,
            pInDDEData, DDEPM_RETRY );
    }
    DdeFreeStringHandle( hszItem );
    return (MRESULT)0;
}

HCONV ImpDdeMgr::ConnectWithClient( HWND hWndClient,
    HSZ hszPartner, HSZ hszService, HSZ hszTopic, sal_Bool bSameInst,
    DDEINIT* pDDEData, CONVCONTEXT* pCC )
{
    ////WRITELOG("ConnectWithClient:Start")
    HWND hWndSrv = CreateConversationWnd();
    IncConversationWndRefCount( hWndSrv );
    HCONV hConv  = CreateConvHandle( pData, pidThis, hWndSrv, hWndClient,
                    hszPartner, hszService, hszTopic );
    if(!hConv )
        return 0;
    sal_Bool bFreeDdeData = sal_False;
    if( !pDDEData )
    {
        bFreeDdeData = sal_True;
        pDDEData = CreateDDEInitData( hWndClient,hszService,hszTopic, pCC );
        PID pid; TID tid;
        WinQueryWindowProcess( hWndClient, &pid, &tid );
        DosGiveSharedMem( pDDEData, pid, PAG_READ | PAG_WRITE);
    }
    HAB hAB = WinQueryAnchorBlock( hWndSrv );
    WinGetLastError( hAB ); // fehlercode zuruecksetzen
    WinSendMsg(hWndClient,WM_DDE_INITIATEACK,(MPARAM)hWndSrv,(MPARAM)pDDEData);
    if( WinGetLastError( hAB ) )
    {
        // ////WRITELOG("DdeConnectWithClient:Client died")
        if( bFreeDdeData )
        {
            MyDosFreeMem( pDDEData,"ConnectWithClient" );
        }
        FreeConvHandle( pData, hConv );
        return (HCONV)0;
    }

    if( !(nTransactFilter & CBF_SKIP_CONNECT_CONFIRMS) )
    {
        Callback( XTYP_CONNECT_CONFIRM, 0, hConv, hszTopic, hszService,
            0, 0, (sal_uIntPtr)bSameInst );
    }

    if( bFreeDdeData )
    {
        MyDosFreeMem( pDDEData,"ConnectWithClient" );
    }
    // HCONV der PartnerApp suchen & bei uns eintragen
    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hConv;
    pConv->hConvPartner = GetConvHandle( pData, hWndClient, hWndSrv );
#if 0 && defined(OV_DEBUG)
    if( !pConv->hConvPartner )
    {
        WRITELOG("DdeConnectWithClient:Partner not found")
    }
#endif
    pConv->nStatus = ST_CONNECTED;
    //WRITESTATUS("Server:Connected with client")
    //WRITELOG("ConnectWithClient:End")
    return hConv;
}

MRESULT ImpDdeMgr::DdeInitiate( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeInitiate:Received")
    HWND hWndClient = (HWND)(pParams->nPar1);
//  sal_Bool bSameInst = IsSameInstance( hWndClient );
    sal_Bool bSameInst = (sal_Bool)(hWndClient==hWndServer);
    DDEINIT* pDDEData = (DDEINIT*)pParams->nPar2;

    if ( ( nTransactFilter & (CBF_FAIL_CONNECTIONS | APPCMD_CLIENTONLY)) ||
         (( nTransactFilter & CBF_FAIL_SELFCONNECTIONS) && bSameInst )
    )
    {
        MyDosFreeMem( pDDEData,"DdeInitiate" );
        return (MRESULT)sal_False;   // narda
    }

    HSZ hszService = (HSZ)0;
    if( *(pDDEData->pszAppName) != '\0' )
    {
        hszService = DdeCreateStringHandle( pDDEData->pszAppName, 850 );
        ////WRITELOG(pDDEData->pszAppName);
    }
    HSZ hszTopic = (HSZ)0;
    if( *(pDDEData->pszTopic) != '\0' )
    {
        hszTopic = DdeCreateStringHandle( pDDEData->pszTopic, 850 );
        ////WRITELOG(pDDEData->pszTopic);
    }
    HSZ hszPartner = GetAppName( hWndClient );

    // nur weitermachen, wenn Service registriert oder
    // Service-Name-Filtering ausgeschaltet.
    if( !bServFilterOn || GetService(hszService) )
    {
        // XTYP_CONNECT-Transaktionen erfolgen nur mit
        // Services & Topics ungleich 0!
        if( hszService && hszTopic )
        {
            if( IsConvHandleAvailable(pData) && Callback( XTYP_CONNECT,
                 0, 0, hszTopic,hszService, 0, 0, (sal_uIntPtr)bSameInst))
            {
                // App erlaubt Verbindung mit Client
                ConnectWithClient( hWndClient, hszPartner,
                    hszService, hszTopic, bSameInst, pDDEData );
            }
        }
        else
        {
            // ** Wildcard-Connect **
            ////WRITELOG("DdeInitiate:Wildconnect")
            // vom Server eine Liste aller Service/Topic-Paare anfordern
            CONVCONTEXT* pCC=(CONVCONTEXT*)(pDDEData+pDDEData->offConvContext);
            DDESTRUCT* hList = Callback( XTYP_WILDCONNECT, 0, (HCONV)0,
                hszTopic,hszService, (HDDEDATA)0, (sal_uIntPtr)pCC, (sal_uIntPtr)bSameInst );
            if( hList )
            {
                HSZPAIR* pPairs = (HSZPAIR*)((char*)hList+hList->offabData);
                while( pPairs->hszSvc )
                {
                    ////WRITELOG("DdeInitiate:Wildconnect.Connecting")
                    ConnectWithClient( hWndClient, hszPartner,
                        pPairs->hszSvc, pPairs->hszTopic, bSameInst, 0, pCC);
                    // Stringhandles gehoeren der App! (nicht free-en)
                    pPairs++;
                }
                DdeFreeDataHandle( hList );
            }
        }
    }
#if 0 && defined(OV_DEBUG)
    else
    {
        WRITELOG("DdeInitiate:Service filtered")
    }
#endif
    DdeFreeStringHandle( hszTopic );
    DdeFreeStringHandle( hszService );
    DdeFreeStringHandle( hszPartner );
    MyDosFreeMem( pDDEData,"DdeInitiate" );
    ////WRITELOG("DdeInitiate:End")
    return (MRESULT)sal_True;
}

MRESULT ImpDdeMgr::DdeInitiateAck( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeInitiateAck:Received")
    DDEINIT* pDDEData = (DDEINIT*)(pParams->nPar2);

    if( !bListConnect && hCurConv )
    {
        ////WRITELOG("DdeInitiateAck:Already connected")
        MyDosFreeMem( pDDEData,"DdeInitiateAck" );
        WinPostMsg( hWndServer, WM_DDE_TERMINATE, (MPARAM)hWndServer, 0 );
        return (MRESULT)sal_False;
    }

    HWND hWndThis = pParams->hWndReceiver;
    // Referenz-Count unseres Client-Windows inkrementieren
    IncConversationWndRefCount( hWndThis );

    HWND hWndSrv        = (HWND)(pParams->nPar1);
    HSZ hszService      = DdeCreateStringHandle( pDDEData->pszAppName, 850 );
    HSZ hszTopic        = DdeCreateStringHandle( pDDEData->pszTopic, 850 );
    HSZ hszPartnerApp   = GetAppName( hWndSrv );

    hCurConv = CreateConvHandle( pData, pidThis, hWndThis, hWndSrv,
                hszPartnerApp, hszService, hszTopic, 0 );

    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hCurConv;

    // HCONV der PartnerApp suchen & bei uns eintragen
    pConv->hConvPartner = GetConvHandle( pData, hWndSrv, hWndThis );
    // nicht asserten, da ja non-ddeml-Partner moeglich
    // DBG_ASSERT(pConv->hConvPartner,"DDE:Partner not found");
    pConv->nStatus = ST_CONNECTED | ST_CLIENT;

    if( bListConnect )
    {
        ////WRITELOG("DdeInitiateAck:ListConnect/Connecting hConvs")
        // Status setzen & verketten
        pConv->hConvList = hCurListId;
        pConv->nPrevHCONV = nPrevConv;
        pConv->nStatus |= ST_INLIST;
        if( nPrevConv )
        {
            pConv = pConvTable;
            pConv += nPrevConv;
            pConv->nNextHCONV = (sal_uInt16)hCurConv;
        }
        nPrevConv = (sal_uInt16)hCurConv;
    }

    DdeFreeStringHandle( hszService );
    DdeFreeStringHandle( hszTopic );
    DdeFreeStringHandle( hszPartnerApp );
    MyDosFreeMem( pDDEData,"DdeInitiateAck" );
    ////WRITESTATUS("After DdeInitiateAck")
    ////WRITELOG("DdeInitiateAck:End")
    return (MRESULT)sal_True;
}

MRESULT ImpDdeMgr::DdePoke( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdePoke:Received")
    HSZ hszItem = 0;
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    HCONV hConv = CheckIncoming( pParams, CBF_FAIL_REQUESTS, hszItem );
    sal_Bool bSuccess =sal_False;
    ImpHCONV* pConv = pConvTable;
    pConv += (sal_uInt16)hConv;
    if ( hConv && !(nTransactFilter & CBF_FAIL_POKES) && pInDDEData )
    {
        if( Callback( XTYP_POKE, pInDDEData->usFormat, hConv,
            pConv->hszTopic, hszItem, pInDDEData, 0, 0 )
                == (DDESTRUCT*)DDE_FACK )
            bSuccess = sal_True;
    }
#if 0 && defined( OV_DEBUG )
    else
    {
        WRITELOG("DdePoke:Not processed")
    }
#endif
    if( pInDDEData )
    {
        if( bSuccess )
            pInDDEData->fsStatus |= DDE_FACK;
        else
            pInDDEData->fsStatus &= (~DDE_FACK);

        MyWinDdePostMsg( pConv->hWndPartner, pConv->hWndThis, WM_DDE_ACK,
            pInDDEData, DDEPM_RETRY );
    }
    DdeFreeStringHandle( hszItem );
    return (MRESULT)0;
}

MRESULT ImpDdeMgr::DdeRequest( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeRequest:Received")
    HSZ hszItem = 0;
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    if( pInDDEData )
        // ist fuer Requests nicht definiert
        pInDDEData->fsStatus = 0;
    HCONV hConv = CheckIncoming( pParams, CBF_FAIL_REQUESTS, hszItem );
    HWND hWndThis = pParams->hWndReceiver;
    HWND hWndClient = (HWND)pParams->nPar1;
    if( hConv )
    {
        ImpHCONV* pConv = pConvTable;
        pConv += (sal_uInt16)hConv;

        DDESTRUCT* pOutDDEData = Callback( XTYP_REQUEST, pInDDEData->usFormat,
            hConv, pConv->hszTopic, hszItem, (HDDEDATA)0, 0, 0 );

        if ( !pOutDDEData )
        {
            ////WRITELOG("DdeRequest:Not processed")
            pInDDEData->fsStatus &= (~DDE_FACK);
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
        }
        else
        {
            ////WRITELOG("DdeRequest:Success")
            MyDosFreeMem( pInDDEData,"DdeRequest" );
            pOutDDEData->fsStatus |= DDE_FRESPONSE;
            MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_DATA,pOutDDEData,DDEPM_RETRY);
        }
    }
    else
    {
        pInDDEData->fsStatus &= (~DDE_FACK);
        MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
    }

    DdeFreeStringHandle( hszItem );
    ////WRITELOG("DdeRequest:End")
    return (MRESULT)0;
}


MRESULT ImpDdeMgr::DdeUnadvise( ImpWndProcParams* pParams )
{
    ////WRITELOG("DdeUnadvise:Received")

    HSZ hszItem;
    HCONV hConv           = CheckIncoming( pParams, 0, hszItem );
    DDESTRUCT* pInDDEData = (DDESTRUCT*)(pParams->nPar2);
    HWND hWndThis         = pParams->hWndReceiver;
    HWND hWndClient       = (HWND)pParams->nPar1;
    sal_uInt16 nClosedTransactions = 0;
    if( hConv )
    {
        sal_uInt16 nFormat = pInDDEData->usFormat;
        // alle Transaktionen des HCONVs loeschen ?
        if( !hszItem )
        {
            // App benachrichtigen & Transaktionen loeschen
            nClosedTransactions = SendUnadvises( hConv, nFormat, sal_True );
        }
        else
        {
            sal_uIntPtr nTransId = GetTransaction(pData, hConv, hszItem, nFormat);
            if( nTransId )
            {
                ////WRITELOG("DdeUnadvise:Transaction found")
                Transaction* pTrans = pTransTable;
                pTrans += (sal_uInt16)nTransId;
                ImpHCONV* pConv = pConvTable;
                pConv += (sal_uInt16)hConv;
                nClosedTransactions = 1;
                if( !(nTransactFilter & CBF_FAIL_ADVISES) )
                    Callback( XTYP_ADVSTOP, nFormat, hConv,
                        pConv->hszTopic, hszItem, 0, 0, 0 );
                if( !pConv->hConvPartner )
                    FreeTransaction( pData, nTransId );
            }
#if defined(OV_DEBUG)
            else
            {
                WRITELOG("DdeUnadvise:Transaction not found")
            }
#endif
        }
    }
#if defined(OV_DEBUG)
    else
    {
       WRITELOG("DdeUnadvise:Conversation not found")
    }
#endif

    if( !nClosedTransactions )
        pInDDEData->fsStatus &= (~DDE_FACK);
    else
        pInDDEData->fsStatus |= DDE_FACK;

    MyWinDdePostMsg(hWndClient,hWndThis,WM_DDE_ACK,pInDDEData,DDEPM_RETRY);
    DdeFreeStringHandle( hszItem );
    return (MRESULT)0;
}

sal_Bool ImpDdeMgr::WaitTransState( Transaction* pTrans, sal_uIntPtr nTransId,
    sal_uInt16 nNewState, sal_uIntPtr nTimeout )
{
    ////WRITELOG("WaitTransState:Start")
    ImpHCONV* pConv = pConvTable;
    pConv += pTrans->hConvOwner;
    HAB hAB = WinQueryAnchorBlock( pConv->hWndThis );
    sal_uIntPtr nTimerId = WinStartTimer( hAB, 0, 0, 50 );
    QMSG aQueueMsg;

//  while( WinGetMsg( hAB, &aQueueMsg, 0, 0, 0 )    &&
//          WinIsWindow( hAB, pConv->hWndPartner)   &&
//          pTrans->nConvst != nNewState )
//  {
//      WinDispatchMsg( hAB, &aQueueMsg );
//  }

    sal_Bool bContinue = sal_True;
    while( bContinue )
    {
        if( WinGetMsg( hAB, &aQueueMsg, 0, 0, 0 ))
        {
            WinDispatchMsg( hAB, &aQueueMsg );
            if( (!WinIsWindow( hAB, pConv->hWndPartner)) ||
                (pTrans->nConvst == nNewState) )
            {
                bContinue = sal_False;
            }
        }
        else
            bContinue = sal_False;
    }

    WinStopTimer( hAB, 0, nTimerId );
    ////WRITELOG("WaitTransState:End")
    return sal_True;
}




