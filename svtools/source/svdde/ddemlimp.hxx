/*************************************************************************
 *
 *  $RCSfile: ddemlimp.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:05 $
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
    USHORT      nStatus;        // ST_* of conversation
    HCONVLIST   hConvList;      // ConvListId , wenn in ConvList
    CONVCONTEXT aConvContext;   // Conversation context

    // private
    HWND        hWndThis;       // 0 == Handle not used
    HWND        hWndPartner;
    PID         pidOwner;       // PID des DdeManagers, der
                                // den Conv-Handle erzeugt hat.
    USHORT      nPrevHCONV;     // 0 == no previous hConv or not in list
    USHORT      nNextHCONV;     // 0 == no next hconv or not in list
};

struct Transaction
{
    HSZ         hszItem;        // Item name
    USHORT      nFormat;        // Data format
    USHORT      nType;          // Transaction type (XTYP_*)
                                // XTYP_ADVREQ [|XTYPF_NODATA] == Advise-Loop
                                //             [|XTYPF_ACKREQ]
                                // XTYP_EXECUTE == laufendes Execute
                                // XTYP_REQUEST
                                // XTYP_POKE
                                // XTYP_ADVSTOP
                                // XTYP_ADVSTART
    USHORT      nConvst;        // Conversation state (XST_*)
                                // 0 == idle
                                // XST_REQSENT (fuer XTYP_ADVREQ)
                                // XST_TIMEOUT (fuer alle Typen!)
                                // XST_WAITING (alle ausser XTYP_ADVREQ)
    USHORT      nLastError;     // last err in transaction
    ULONG       nUser;          // Userhandle
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
    USHORT      nRefCount;  // Zahl Conversations auf diesem Window
};


// systemglobale Daten der Library (liegen in named shared memory)
struct ImpDdeMgrData
{
    ULONG       nTotalSize;
    ULONG       nOffsAppTable;
    ULONG       nOffsConvTable;
    ULONG       nOffsTransTable;
    USHORT      nMaxAppCount;
    USHORT      nMaxConvCount;
    USHORT      nMaxTransCount;
    USHORT      nLastErr;
    USHORT      nReserved;
    USHORT      nCurTransCount;
    USHORT      nCurConvCount;
    HWND        aAppTable[ 1 ];     // fuer Broadcast-Messages
    ImpHCONV    aConvTable[ 1 ];
    Transaction aTransTable[ 1 ];
};



class ImpDdeMgr
{
    friend MRESULT EXPENTRY ConvWndProc(HWND hWnd,ULONG nMsg,MPARAM nPar1,MPARAM nPar2);
    friend MRESULT EXPENTRY ServerWndProc(HWND hWnd,ULONG nMsg,MPARAM nPar1,MPARAM nPar2);
    friend void ImpWriteDdeStatus(char*,char*);
    friend void ImpAddHSZ( HSZ, String& );

    static PSZ      AllocAtomName( ATOM hString, ULONG& rBufLen );
    static PDDESTRUCT MakeDDEObject( HWND hwnd, ATOM hItemName,
        USHORT fsStatus, USHORT usFormat, PVOID pabData, ULONG usDataLen );
    static APIRET   AllocNamedSharedMem( PPVOID ppBaseAddress, PSZ pName,
        ULONG nElementSize, ULONG nElementCount );

    HWND            hWndServer;
    PID             pidThis;
    PFNCALLBACK     pCallback;
    ULONG           nTransactFilter;
    CONVCONTEXT     aDefaultContext;
    ImpDdeMgrData*  pData;
    ImpService*     pServices;
    USHORT          nServiceCount;

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
    static USHORT   GetConversationWndRefCount( HWND hWndConv );
    static USHORT   IncConversationWndRefCount( HWND hWndConv );

    MRESULT         SrvWndProc(HWND hWnd,ULONG nMsg,MPARAM nPar1,MPARAM nPar2);
    MRESULT         ConvWndProc(HWND hWnd,ULONG nMsg,MPARAM nPar1,MPARAM nPar2);
    void            RegisterDDEMLApp();
    void            UnregisterDDEMLApp();
    void            CleanUp();
    ImpDdeMgrData*  InitAll();
    static BOOL     MyWinDdePostMsg( HWND, HWND, USHORT, PDDESTRUCT, ULONG );
    void            MyInitiateDde( HWND hWndServer, HWND hWndClient,
                        HSZ hszService, HSZ hszTopic, CONVCONTEXT* pCC );
    DDEINIT*        CreateDDEInitData( HWND hWndDest, HSZ hszService,
                        HSZ hszTopic, CONVCONTEXT* pCC );
    // wenn pDDEData==0, muss pCC gesetzt sein
    HCONV           ConnectWithClient( HWND hWndClient, HSZ hszPartner,
                        HSZ hszService, HSZ hszTopic, BOOL bSameInst,
                        DDEINIT* pDDEData, CONVCONTEXT* pCC = 0);

    HCONV           CheckIncoming( ImpWndProcParams*, ULONG nTransMask,
                        HSZ& rhszItem );
    // fuer Serverbetrieb. Ruft Callback-Fkt fuer alle offenen Advises
    // auf, deren Owner der uebergebene HCONV ist.
    // bFreeTransactions==TRUE: loescht die Transaktionen
    // gibt Anzahl der getrennten Transaktionen zurueck
    USHORT          SendUnadvises( HCONV hConv,
                       USHORT nFormat, // 0==alle
                       BOOL bFreeTransactions );

    BOOL            WaitTransState(
                        Transaction* pTrans, ULONG nTransId,
                        USHORT nNewState,
                        ULONG nTimeout );

    // DDEML ruft Callback mit XTYP_CONNECT-Transaction nur auf,
    // wenn die App den angeforderten Service registriert hat
    // Standardeinstellung: TRUE
    BOOL            bServFilterOn;

    // Fehlercode muss noch systemglobal werden (Atom o. ae.)
    static USHORT   nLastErrInstance; // wenn 0, dann gilt globaler Fehlercode

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
                        BOOL bDestroyHWndThis = TRUE );

    static ULONG    CreateTransaction( ImpDdeMgrData* pBase,
                        HCONV hOwner, HSZ hszItem, USHORT nFormat,
                        USHORT nTransactionType );
    static ULONG    GetTransaction( ImpDdeMgrData* pBase,
                        HCONV hOwner, HSZ hszItem, USHORT nFormat );

    static void     FreeTransaction( ImpDdeMgrData*, ULONG nTransId );

    BOOL            DisconnectAll();
    // Transaktionen muessen _vor_ den Konversationen geloescht werden!
    static void     FreeTransactions( ImpDdeMgrData*, HWND hWndThis,
                        HWND hWndPartner );
    static void     FreeTransactions( ImpDdeMgrData*, HCONV hConvOwner );

    static void     FreeConversations( ImpDdeMgrData*,HWND hWndThis,
                        HWND hWndPartner );

    ImpService*     GetService( HSZ hszService );
    ImpService*     PutService( HSZ hszService );
    void            BroadcastService( ImpService*, BOOL bRegistered );

    // rh: Startposition(!) & gefundener Handle
    static ImpHCONV* GetFirstServer( ImpDdeMgrData*, HCONVLIST, HCONV& rh);
    static ImpHCONV* GetLastServer( ImpDdeMgrData*, HCONVLIST, HCONV& );
    static BOOL     CheckConvListId( HCONVLIST hConvListId );

    BOOL            IsSameInstance( HWND hWnd );
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
        USHORT      nTransactionType,
        USHORT      nClipboardFormat,
        HCONV       hConversationHandle,
        HSZ         hsz1,
        HSZ         hsz2,
        HDDEDATA    hData,
        ULONG       nData1,
        ULONG       nData2 );

    HCONV       DdeConnectImp( HSZ hszService,HSZ hszTopic,CONVCONTEXT* pCC);

    // connection data
    HCONV           hCurConv;       // wird im DdeInitiateAck gesetzt
    HCONVLIST       hCurListId;     // fuer DdeConnectList
    USHORT          nPrevConv;      // .... "" ....
    BOOL            bListConnect;

    // synchr. transaction data
    BOOL            bInSyncTrans;
    ULONG           nSyncTransId;
    HDDEDATA        hSyncResponseData;
    ULONG           nSyncResponseMsg; // WM_DDE_ACK, WM_DDE_DATA, WM_TIMER
    // TRUE==nach Ende der synchronen Transaktion eine evtl. benutzte
    // asynchrone Transaktion beenden (typisch synchroner Request auf
    // Advise-Loop)
    BOOL            bSyncAbandonTrans;

public:
                    ImpDdeMgr();
                    ~ImpDdeMgr();

    USHORT          DdeInitialize( PFNCALLBACK pCallbackProc, ULONG nTransactionFilter );
    USHORT          DdeGetLastError();

    HCONV           DdeConnect( HSZ hszService, HSZ hszTopic, CONVCONTEXT* );
    HCONVLIST       DdeConnectList( HSZ hszService, HSZ hszTopic,
                        HCONVLIST hConvList, CONVCONTEXT* );
    static BOOL     DdeDisconnect( HCONV hConv );
    static BOOL     DdeDisconnectList( HCONVLIST hConvList );
    static HCONV    DdeReconnect(HCONV hConv);
    static HCONV    DdeQueryNextServer(HCONVLIST hConvList, HCONV hConvPrev);
    static USHORT   DdeQueryConvInfo(HCONV hConv, ULONG idTrans,CONVINFO* pCI);
    static BOOL     DdeSetUserHandle(HCONV hConv, ULONG id, ULONG hUser);
    BOOL            DdeAbandonTransaction( HCONV hConv, ULONG idTransaction);

    BOOL            DdePostAdvise( HSZ hszTopic, HSZ hszItem);
    BOOL            DdeEnableCallback( HCONV hConv, USHORT wCmd);

    HDDEDATA        DdeNameService( HSZ hszService, USHORT afCmd);

    static HDDEDATA DdeClientTransaction(void* pData, ULONG cbData,
                        HCONV hConv, HSZ hszItem, USHORT wFmt, USHORT wType,
                        ULONG dwTimeout, ULONG* pdwResult);

    // Data handles

    HDDEDATA        DdeCreateDataHandle( void* pSrc, ULONG cb, ULONG cbOff,
                        HSZ hszItem, USHORT wFmt, USHORT afCmd);
    static BYTE*    DdeAccessData(HDDEDATA hData, ULONG* pcbDataSize);
    static BOOL     DdeUnaccessData(HDDEDATA hData);
    static BOOL     DdeFreeDataHandle(HDDEDATA hData);
    static HDDEDATA DdeAddData(HDDEDATA hData,void* pSrc,ULONG cb,ULONG cbOff);
    static ULONG    DdeGetData(HDDEDATA hData,void* pDst,ULONG cbMax,ULONG cbOff);

    // String handles

    static HSZ      DdeCreateStringHandle( PSZ pStr, int iCodePage);
    static ULONG    DdeQueryString(HSZ hsz,PSZ pStr,ULONG cchMax,int iCPage);
    static BOOL     DdeFreeStringHandle( HSZ hsz );
    static BOOL     DdeKeepStringHandle( HSZ hsz );
    static int      DdeCmpStringHandles(HSZ hsz1, HSZ hsz2);

    // mit dieser Funktion kann geprueft werden, ob eine
    // Applikation schon eine DDEML-Instanz angelegt hat.
    // Die aktuelle Impl. unterstuetzt nur eine DDEML-Instanz
    // pro Applikation (wg. synchroner Transaktionen)
    static ImpDdeMgr* GetImpDdeMgrInstance( HWND hWnd );

    // gibt TRUE zurueck, wenn mind. ein lebender HCONV
    // von diesem DdeMgr erzeugt wurde
    BOOL            OwnsConversationHandles();
};

// static
inline ImpHCONV* ImpDdeMgr::GetConvTable( ImpDdeMgrData* pData )
{
    ImpHCONV* pRet;
    if( pData )
        pRet = (ImpHCONV*)((ULONG)(pData) + pData->nOffsConvTable);
    else
        pRet = 0;
    return pRet;
}

// static
inline Transaction* ImpDdeMgr::GetTransTable( ImpDdeMgrData* pData )
{
    Transaction* pRet;
    if( pData )
        pRet = (Transaction*)((ULONG)(pData) + pData->nOffsTransTable);
    else
        pRet = 0;
    return pRet;
}

// static
inline HWND* ImpDdeMgr::GetAppTable( ImpDdeMgrData* pData )
{
    HWND* pRet;
    if( pData )
        pRet = (HWND*)((ULONG)(pData) + pData->nOffsAppTable);
    else
        pRet = 0;
    return pRet;
}




#endif

