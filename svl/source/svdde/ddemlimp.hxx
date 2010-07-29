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

#ifndef _DDEMLIMP_HXX
#define _DDEMLIMP_HXX


#include <string.h>
#include <ctype.h>
#include "ddemlos2.h"

#define DDEMLSERVICETABLE_INISIZE   8

// Bezeichner der systemglobalen DDEML-Tabelle
#define DDEMLDATA   "\\SHAREMEM\\OV_DDEML.DAT"

// vorlaeufig konstante Tabellengroessen
#define CONVTABLECOUNT      2048    /* max count conversations */
#define TRANSTABLECOUNT     2048    /*           transactions  */
#define DDEMLAPPCOUNT       16      /* max count simultaniously running */
                                    /* ddeml (StarDivision) applications */

#define ST_TERMACKREC       0x8000  /* wird im Conversationhandle gesetzt, */
                                    /* wenn die Partner-App DDE_TERMINATE */
                                    /* bestaetigt hat */

#define XST_TIMEOUT         17      /* Trans. hat Timeout ueberschritten */
#define XST_WAITING_ACK     18      /* Trans. wartet auf Acknowledge */
#define XST_WAITING_ADVDATA 19      /* Trans. wartet auf Advise-Daten */
#define XST_WAITING_REQDATA 20      /* Trans. wartet auf angeforderte Daten */


/* User-Flags DDESTRUCT */
#define IMP_HDATAAPPOWNED   0x8000

#define CONVLISTNAME        "DdeConvListId"

#define XTYPF_MASK (XTYPF_NOBLOCK | XTYPF_NODATA | XTYPF_ACKREQ)

//
// DDEML-Messages; werden nur an registrierte DDEML-Apps gesendet
//

// Msg: WM_DDEML_REGISTER
// Empfaenger: wird allen DDEML-Applikationen nach Registrierung
// eines neuen Services gesendet
// Params: nPar1: hszBaseServName
//         nPar2: hszInstServName
#define WM_DDEML_REGISTER               WM_USER+1

// Msg: WM_DDEML_UNREGISTER
// Empfaenger: wird allen DDEML-Applikationen nach Deregistrierung
// eines Services gesendet
// Params: nPar1: hszBaseServName
//         nPar2: hszInstServName
#define WM_DDEML_UNREGISTER             WM_USER+2

//
//
//

struct ImpHCONV
{
    HCONV       hConvPartner;
    HSZ         hszPartner;     // Name of partner application
    HSZ         hszServiceReq;  // Service name
    HSZ         hszTopic;       // Topic name
    sal_uInt16      nStatus;        // ST_* of conversation
    HCONVLIST   hConvList;      // ConvListId , wenn in ConvList
    CONVCONTEXT aConvContext;   // Conversation context

    // private
    HWND        hWndThis;       // 0 == Handle not used
    HWND        hWndPartner;
    PID         pidOwner;       // PID des DdeManagers, der
                                // den Conv-Handle erzeugt hat.
    sal_uInt16      nPrevHCONV;     // 0 == no previous hConv or not in list
    sal_uInt16      nNextHCONV;     // 0 == no next hconv or not in list
};

struct Transaction
{
    HSZ         hszItem;        // Item name
    sal_uInt16      nFormat;        // Data format
    sal_uInt16      nType;          // Transaction type (XTYP_*)
                                // XTYP_ADVREQ [|XTYPF_NODATA] == Advise-Loop
                                //             [|XTYPF_ACKREQ]
                                // XTYP_EXECUTE == laufendes Execute
                                // XTYP_REQUEST
                                // XTYP_POKE
                                // XTYP_ADVSTOP
                                // XTYP_ADVSTART
    sal_uInt16      nConvst;        // Conversation state (XST_*)
                                // 0 == idle
                                // XST_REQSENT (fuer XTYP_ADVREQ)
                                // XST_TIMEOUT (fuer alle Typen!)
                                // XST_WAITING (alle ausser XTYP_ADVREQ)
    sal_uInt16      nLastError;     // last err in transaction
    sal_uIntPtr     nUser;          // Userhandle
    // private
    HCONV       hConvOwner;     // 0 == Transaction not used
};


struct ImpWndProcParams
{
    HWND        hWndReceiver;
    MPARAM      nPar1;
    MPARAM      nPar2;
};

struct ImpService
{
    HSZ         hBaseServName;  // Basis-Name des Service
    HSZ         hInstServName;  // Basis-Name + DDEML-Server-HWND der App
};

class ImpDdeMgr;

// Daten eines Conversation-Windows
struct ImpConvWndData
{
    ImpDdeMgr*  pThis;
    sal_uInt16      nRefCount;  // Zahl Conversations auf diesem Window
};


// systemglobale Daten der Library (liegen in named shared memory)
struct ImpDdeMgrData
{
    sal_uIntPtr     nTotalSize;
    sal_uIntPtr     nOffsAppTable;
    sal_uIntPtr     nOffsConvTable;
    sal_uIntPtr     nOffsTransTable;
    sal_uInt16      nMaxAppCount;
    sal_uInt16      nMaxConvCount;
    sal_uInt16      nMaxTransCount;
    sal_uInt16      nLastErr;
    sal_uInt16      nReserved;
    sal_uInt16      nCurTransCount;
    sal_uInt16      nCurConvCount;
    HWND        aAppTable[ 1 ];     // fuer Broadcast-Messages
    ImpHCONV    aConvTable[ 1 ];
    Transaction aTransTable[ 1 ];
};



class ImpDdeMgr
{
    friend MRESULT EXPENTRY ConvWndProc(HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2);
    friend MRESULT EXPENTRY ServerWndProc(HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2);
    friend void ImpWriteDdeStatus(char*,char*);
    friend void ImpAddHSZ( HSZ, String& );

    static PSZ      AllocAtomName( ATOM hString, sal_uIntPtr& rBufLen );
    static PDDESTRUCT MakeDDEObject( HWND hwnd, ATOM hItemName,
        sal_uInt16 fsStatus, sal_uInt16 usFormat, PVOID pabData, sal_uIntPtr usDataLen );
    static APIRET   AllocNamedSharedMem( PPVOID ppBaseAddress, PSZ pName,
        sal_uIntPtr nElementSize, sal_uIntPtr nElementCount );

    HWND            hWndServer;
    PID             pidThis;
    PFNCALLBACK     pCallback;
    sal_uIntPtr             nTransactFilter;
    CONVCONTEXT     aDefaultContext;
    ImpDdeMgrData*  pData;
    ImpService*     pServices;
    sal_uInt16          nServiceCount;

    ImpHCONV*       pConvTable;         // liegt in pData (nicht deleten!)
    Transaction*    pTransTable;        // liegt in pData (nicht deleten!)
    HWND*           pAppTable;          // liegt in pData (nicht deleten!)

    static ImpHCONV*    GetConvTable( ImpDdeMgrData* );
    static Transaction* GetTransTable( ImpDdeMgrData* );
    static HWND*        GetAppTable( ImpDdeMgrData* );


    static HWND     NextFrameWin( HENUM hEnum );
    void            CreateServerWnd();
    void            DestroyServerWnd();
    HWND            CreateConversationWnd();
    // Fktn. duerfen nur fuer HCONVs aufgerufen werden, die
    // in der eigenen Applikation erzeugt wurden
    static void     DestroyConversationWnd( HWND hWndConv );
    static sal_uInt16   GetConversationWndRefCount( HWND hWndConv );
    static sal_uInt16   IncConversationWndRefCount( HWND hWndConv );

    MRESULT         SrvWndProc(HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2);
    MRESULT         ConvWndProc(HWND hWnd,sal_uIntPtr nMsg,MPARAM nPar1,MPARAM nPar2);
    void            RegisterDDEMLApp();
    void            UnregisterDDEMLApp();
    void            CleanUp();
    ImpDdeMgrData*  InitAll();
    static sal_Bool     MyWinDdePostMsg( HWND, HWND, sal_uInt16, PDDESTRUCT, sal_uIntPtr );
    void            MyInitiateDde( HWND hWndServer, HWND hWndClient,
                        HSZ hszService, HSZ hszTopic, CONVCONTEXT* pCC );
    DDEINIT*        CreateDDEInitData( HWND hWndDest, HSZ hszService,
                        HSZ hszTopic, CONVCONTEXT* pCC );
    // wenn pDDEData==0, muss pCC gesetzt sein
    HCONV           ConnectWithClient( HWND hWndClient, HSZ hszPartner,
                        HSZ hszService, HSZ hszTopic, sal_Bool bSameInst,
                        DDEINIT* pDDEData, CONVCONTEXT* pCC = 0);

    HCONV           CheckIncoming( ImpWndProcParams*, sal_uIntPtr nTransMask,
                        HSZ& rhszItem );
    // fuer Serverbetrieb. Ruft Callback-Fkt fuer alle offenen Advises
    // auf, deren Owner der uebergebene HCONV ist.
    // bFreeTransactions==sal_True: loescht die Transaktionen
    // gibt Anzahl der getrennten Transaktionen zurueck
    sal_uInt16          SendUnadvises( HCONV hConv,
                       sal_uInt16 nFormat, // 0==alle
                       sal_Bool bFreeTransactions );

    sal_Bool            WaitTransState(
                        Transaction* pTrans, sal_uIntPtr nTransId,
                        sal_uInt16 nNewState,
                        sal_uIntPtr nTimeout );

    // DDEML ruft Callback mit XTYP_CONNECT-Transaction nur auf,
    // wenn die App den angeforderten Service registriert hat
    // Standardeinstellung: sal_True
    sal_Bool            bServFilterOn;

    // Fehlercode muss noch systemglobal werden (Atom o. ae.)
    static sal_uInt16   nLastErrInstance; // wenn 0, dann gilt globaler Fehlercode

    static ImpDdeMgrData* AccessMgrData();

    static HCONV    CreateConvHandle( ImpDdeMgrData* pBase,
                        PID pidOwner,
                        HWND hWndThis, HWND hWndPartner,
                        HSZ hszPartner, HSZ hszServiceReq, HSZ hszTopic,
                        HCONV hPrevHCONV = 0 );

    static HCONV    IsConvHandleAvailable( ImpDdeMgrData* pBase );
    static HCONV    GetConvHandle( ImpDdeMgrData* pBase,
                        HWND hWndThis, HWND hWndPartner );
    static void     FreeConvHandle( ImpDdeMgrData*, HCONV,
                        sal_Bool bDestroyHWndThis = sal_True );

    static sal_uIntPtr  CreateTransaction( ImpDdeMgrData* pBase,
                        HCONV hOwner, HSZ hszItem, sal_uInt16 nFormat,
                        sal_uInt16 nTransactionType );
    static sal_uIntPtr  GetTransaction( ImpDdeMgrData* pBase,
                        HCONV hOwner, HSZ hszItem, sal_uInt16 nFormat );

    static void     FreeTransaction( ImpDdeMgrData*, sal_uIntPtr nTransId );

    sal_Bool            DisconnectAll();
    // Transaktionen muessen _vor_ den Konversationen geloescht werden!
    static void     FreeTransactions( ImpDdeMgrData*, HWND hWndThis,
                        HWND hWndPartner );
    static void     FreeTransactions( ImpDdeMgrData*, HCONV hConvOwner );

    static void     FreeConversations( ImpDdeMgrData*,HWND hWndThis,
                        HWND hWndPartner );

    ImpService*     GetService( HSZ hszService );
    ImpService*     PutService( HSZ hszService );
    void            BroadcastService( ImpService*, sal_Bool bRegistered );

    // rh: Startposition(!) & gefundener Handle
    static ImpHCONV* GetFirstServer( ImpDdeMgrData*, HCONVLIST, HCONV& rh);
    static ImpHCONV* GetLastServer( ImpDdeMgrData*, HCONVLIST, HCONV& );
    static sal_Bool     CheckConvListId( HCONVLIST hConvListId );

    sal_Bool            IsSameInstance( HWND hWnd );
    HSZ             GetAppName( HWND hWnd );


    // Transactions
    MRESULT         DdeAck( ImpWndProcParams* pParams );
    MRESULT         DdeAdvise( ImpWndProcParams* pParams );
    MRESULT         DdeData( ImpWndProcParams* pParams );
    MRESULT         DdeExecute( ImpWndProcParams* pParams );
    MRESULT         DdeInitiate( ImpWndProcParams* pParams );
    MRESULT         DdeInitiateAck( ImpWndProcParams* pParams );
    MRESULT         DdePoke( ImpWndProcParams* pParams );
    MRESULT         DdeRequest( ImpWndProcParams* pParams );
    MRESULT         DdeTerminate( ImpWndProcParams* pParams );
    MRESULT         DdeUnadvise( ImpWndProcParams* pParams );
    MRESULT         DdeRegister( ImpWndProcParams* pParams );
    MRESULT         DdeUnregister( ImpWndProcParams* pParams );
    MRESULT         DdeTimeout( ImpWndProcParams* pParams );

    HDDEDATA    Callback(
        sal_uInt16      nTransactionType,
        sal_uInt16      nClipboardFormat,
        HCONV       hConversationHandle,
        HSZ         hsz1,
        HSZ         hsz2,
        HDDEDATA    hData,
        sal_uIntPtr     nData1,
        sal_uIntPtr     nData2 );

    HCONV       DdeConnectImp( HSZ hszService,HSZ hszTopic,CONVCONTEXT* pCC);

    // connection data
    HCONV           hCurConv;       // wird im DdeInitiateAck gesetzt
    HCONVLIST       hCurListId;     // fuer DdeConnectList
    sal_uInt16          nPrevConv;      // .... "" ....
    sal_Bool            bListConnect;

    // synchr. transaction data
    sal_Bool            bInSyncTrans;
    sal_uIntPtr         nSyncTransId;
    HDDEDATA        hSyncResponseData;
    sal_uIntPtr         nSyncResponseMsg; // WM_DDE_ACK, WM_DDE_DATA, WM_TIMER
    // sal_True==nach Ende der synchronen Transaktion eine evtl. benutzte
    // asynchrone Transaktion beenden (typisch synchroner Request auf
    // Advise-Loop)
    sal_Bool            bSyncAbandonTrans;

public:
                    ImpDdeMgr();
                    ~ImpDdeMgr();

    sal_uInt16          DdeInitialize( PFNCALLBACK pCallbackProc, sal_uIntPtr nTransactionFilter );
    sal_uInt16          DdeGetLastError();

    HCONV           DdeConnect( HSZ hszService, HSZ hszTopic, CONVCONTEXT* );
    HCONVLIST       DdeConnectList( HSZ hszService, HSZ hszTopic,
                        HCONVLIST hConvList, CONVCONTEXT* );
    static sal_Bool     DdeDisconnect( HCONV hConv );
    static sal_Bool     DdeDisconnectList( HCONVLIST hConvList );
    static HCONV    DdeReconnect(HCONV hConv);
    static HCONV    DdeQueryNextServer(HCONVLIST hConvList, HCONV hConvPrev);
    static sal_uInt16   DdeQueryConvInfo(HCONV hConv, sal_uIntPtr idTrans,CONVINFO* pCI);
    static sal_Bool     DdeSetUserHandle(HCONV hConv, sal_uIntPtr id, sal_uIntPtr hUser);
    sal_Bool            DdeAbandonTransaction( HCONV hConv, sal_uIntPtr idTransaction);

    sal_Bool            DdePostAdvise( HSZ hszTopic, HSZ hszItem);
    sal_Bool            DdeEnableCallback( HCONV hConv, sal_uInt16 wCmd);

    HDDEDATA        DdeNameService( HSZ hszService, sal_uInt16 afCmd);

    static HDDEDATA DdeClientTransaction(void* pData, sal_uIntPtr cbData,
                        HCONV hConv, HSZ hszItem, sal_uInt16 wFmt, sal_uInt16 wType,
                        sal_uIntPtr dwTimeout, sal_uIntPtr* pdwResult);

    // Data handles

    HDDEDATA        DdeCreateDataHandle( void* pSrc, sal_uIntPtr cb, sal_uIntPtr cbOff,
                        HSZ hszItem, sal_uInt16 wFmt, sal_uInt16 afCmd);
    static sal_uInt8*   DdeAccessData(HDDEDATA hData, sal_uIntPtr* pcbDataSize);
    static sal_Bool     DdeUnaccessData(HDDEDATA hData);
    static sal_Bool     DdeFreeDataHandle(HDDEDATA hData);
    static HDDEDATA DdeAddData(HDDEDATA hData,void* pSrc,sal_uIntPtr cb,sal_uIntPtr cbOff);
    static sal_uIntPtr  DdeGetData(HDDEDATA hData,void* pDst,sal_uIntPtr cbMax,sal_uIntPtr cbOff);

    // String handles

    static HSZ      DdeCreateStringHandle( PSZ pStr, int iCodePage);
    static sal_uIntPtr      DdeQueryString(HSZ hsz,PSZ pStr,sal_uIntPtr cchMax,int iCPage);
    static sal_Bool     DdeFreeStringHandle( HSZ hsz );
    static sal_Bool     DdeKeepStringHandle( HSZ hsz );
    static int      DdeCmpStringHandles(HSZ hsz1, HSZ hsz2);

    // mit dieser Funktion kann geprueft werden, ob eine
    // Applikation schon eine DDEML-Instanz angelegt hat.
    // Die aktuelle Impl. unterstuetzt nur eine DDEML-Instanz
    // pro Applikation (wg. synchroner Transaktionen)
    static ImpDdeMgr* GetImpDdeMgrInstance( HWND hWnd );

    // gibt sal_True zurueck, wenn mind. ein lebender HCONV
    // von diesem DdeMgr erzeugt wurde
    sal_Bool            OwnsConversationHandles();
};

// static
inline ImpHCONV* ImpDdeMgr::GetConvTable( ImpDdeMgrData* pData )
{
    ImpHCONV* pRet;
    if( pData )
        pRet = (ImpHCONV*)((sal_uIntPtr)(pData) + pData->nOffsConvTable);
    else
        pRet = 0;
    return pRet;
}

// static
inline Transaction* ImpDdeMgr::GetTransTable( ImpDdeMgrData* pData )
{
    Transaction* pRet;
    if( pData )
        pRet = (Transaction*)((sal_uIntPtr)(pData) + pData->nOffsTransTable);
    else
        pRet = 0;
    return pRet;
}

// static
inline HWND* ImpDdeMgr::GetAppTable( ImpDdeMgrData* pData )
{
    HWND* pRet;
    if( pData )
        pRet = (HWND*)((sal_uIntPtr)(pData) + pData->nOffsAppTable);
    else
        pRet = 0;
    return pRet;
}




#endif

