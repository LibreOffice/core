/*************************************************************************
 *
 *  $RCSfile: ddemlos2.h,v $
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
#ifndef _DDEML_H
#define _DDEML_H

#define INCL_OS2
#define INCL_WIN

#include <tools/svpm.h>
#include <tools/solar.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#ifndef _SYSDEP_HXX
#include <vcl/sysdep.hxx>
#endif

typedef LHANDLE HSTR;

#ifndef CALLBACK
#define CALLBACK
#endif


typedef ULONG       HCONVLIST;
typedef ULONG       HCONV;
typedef ATOM        HSZ;
typedef DDESTRUCT*  HDDEDATA;

struct CONVINFO
{
    USHORT      nSize;                  // sizeof(CONVINFO)
    ULONG       nUser;                  // Userhandle
    HCONV       hConvPartner;           //
    HSZ         hszPartner;             // Name der Partnerapp
    HSZ         hszServiceReq;          // Name des angeforderten Services
    HSZ         hszTopic;               // -- " -- Topics
    HSZ         hszItem;                // -- " -- Items
    USHORT      nFormat;                // Datenformat der akt. Transaktion
    USHORT      nType;                  // Typ der akt. Transaktion (XTYP_*)
    USHORT      nStatus;                // ST_* der Konversation
    USHORT      nConvst;                // XST_* der akt. Transaktion
    USHORT      nLastError;             // letzter Fehler der Transaktion
    HCONVLIST   hConvList;              // ConvListId , wenn in ConvList
    CONVCONTEXT aConvCtxt;              // conversation context
};

/* the following structure is for use with XTYP_WILDCONNECT processing. */

struct HSZPAIR
{
    HSZ hszSvc;
    HSZ hszTopic;
};
typedef HSZPAIR *PHSZPAIR;

/***** conversation states (usState) *****/

#define     XST_NULL              0  /* quiescent states */
#define     XST_INCOMPLETE        1
#define     XST_CONNECTED         2
#define     XST_INIT1             3  /* mid-initiation states */
#define     XST_INIT2             4
#define     XST_REQSENT           5  /* active conversation states */
#define     XST_DATARCVD          6
#define     XST_POKESENT          7
#define     XST_POKEACKRCVD       8
#define     XST_EXECSENT          9
#define     XST_EXECACKRCVD      10
#define     XST_ADVSENT          11
#define     XST_UNADVSENT        12
#define     XST_ADVACKRCVD       13
#define     XST_UNADVACKRCVD     14
#define     XST_ADVDATASENT      15
#define     XST_ADVDATAACKRCVD   16

/* used in LOWORD(dwData1) of XTYP_ADVREQ callbacks... */
#define     CADV_LATEACK         0xFFFF

/***** conversation status bits (fsStatus) *****/

#define     ST_CONNECTED        0x0001
#define     ST_ADVISE           0x0002
#define     ST_ISLOCAL          0x0004
#define     ST_BLOCKED          0x0008
#define     ST_CLIENT           0x0010
#define     ST_TERMINATED       0x0020
#define     ST_INLIST           0x0040
#define     ST_BLOCKNEXT        0x0080
#define     ST_ISSELF           0x0100


/* DDE constants for wStatus field */

//#define DDE_FACK                        0x8000
//#define DDE_FBUSY                       0x4000
//#define DDE_FDEFERUPD           0x4000
//#define DDE_FACKREQ             0x8000
//#define DDE_FRELEASE            0x2000
//#define DDE_FREQUESTED          0x1000
//#define DDE_FACKRESERVED        0x3ff0
//#define DDE_FADVRESERVED        0x3fff
//#define DDE_FDATRESERVED        0x4fff
//#define DDE_FPOKRESERVED        0xdfff
//#define DDE_FAPPSTATUS          0x00ff
#define DDE_FNOTPROCESSED   0x0000

/***** message filter hook types *****/

#define     MSGF_DDEMGR             0x8001

/***** codepage constants ****/

#define CP_WINANSI      1004    /* default codepage for windows & old DDE convs. */

/***** transaction types *****/

#define     XTYPF_NOBLOCK            0x0002  /* CBR_BLOCK will not work */
#define     XTYPF_NODATA             0x0004  /* DDE_FDEFERUPD */
#define     XTYPF_ACKREQ             0x0008  /* DDE_FACKREQ */

#define     XCLASS_MASK              0xFC00
#define     XCLASS_BOOL              0x1000
#define     XCLASS_DATA              0x2000
#define     XCLASS_FLAGS             0x4000
#define     XCLASS_NOTIFICATION      0x8000

#define     XTYP_ERROR              (0x0000 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK )
#define     XTYP_ADVDATA            (0x0010 | XCLASS_FLAGS         )
#define     XTYP_ADVREQ             (0x0020 | XCLASS_DATA | XTYPF_NOBLOCK )
#define     XTYP_ADVSTART           (0x0030 | XCLASS_BOOL          )
#define     XTYP_ADVSTOP            (0x0040 | XCLASS_NOTIFICATION)
#define     XTYP_EXECUTE            (0x0050 | XCLASS_FLAGS         )
#define     XTYP_CONNECT            (0x0060 | XCLASS_BOOL | XTYPF_NOBLOCK)
#define     XTYP_CONNECT_CONFIRM    (0x0070 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_XACT_COMPLETE      (0x0080 | XCLASS_NOTIFICATION  )
#define     XTYP_POKE               (0x0090 | XCLASS_FLAGS         )
#define     XTYP_REGISTER           (0x00A0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_REQUEST            (0x00B0 | XCLASS_DATA          )
#define     XTYP_DISCONNECT         (0x00C0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_UNREGISTER         (0x00D0 | XCLASS_NOTIFICATION | XTYPF_NOBLOCK)
#define     XTYP_WILDCONNECT        (0x00E0 | XCLASS_DATA | XTYPF_NOBLOCK)

#define     XTYP_MASK                0x00F0
#define     XTYP_SHIFT               4  /* shift to turn XTYP_ into an index */

/***** Timeout constants *****/

#define     TIMEOUT_ASYNC           -1L

/***** Transaction ID constants *****/

#define     QID_SYNC                -1L

/****** public strings used in DDE ******/

#define SZDDESYS_TOPIC          "System"
#define SZDDESYS_ITEM_TOPICS    "Topics"
#define SZDDESYS_ITEM_SYSITEMS  "SysItems"
#define SZDDESYS_ITEM_RTNMSG    "ReturnMessage"
#define SZDDESYS_ITEM_STATUS    "Status"
#define SZDDESYS_ITEM_FORMATS   "Formats"
#define SZDDESYS_ITEM_HELP      "Help"
#define SZDDE_ITEM_ITEMLIST     "TopicItemList"


/****** API entry points ******/

typedef HDDEDATA CALLBACK FNCALLBACK(USHORT wType, USHORT wFmt, HCONV hConv,
        HSZ hsz1, HSZ hsz2, HDDEDATA hData, ULONG dwData1, ULONG dwData2);
typedef FNCALLBACK* PFNCALLBACK;

#define     CBR_BLOCK                0xffffffffL

/* DLL registration functions */

USHORT DdeInitialize(ULONG* pidInst, PFNCALLBACK pfnCallback,
                ULONG afCmd, ULONG ulRes);

/*
 * Callback filter flags for use with standard apps.
 */

#define     CBF_FAIL_SELFCONNECTIONS     0x00001000
#define     CBF_FAIL_CONNECTIONS         0x00002000
#define     CBF_FAIL_ADVISES             0x00004000
#define     CBF_FAIL_EXECUTES            0x00008000
#define     CBF_FAIL_POKES               0x00010000
#define     CBF_FAIL_REQUESTS            0x00020000
#define     CBF_FAIL_ALLSVRXACTIONS      0x0003f000

#define     CBF_SKIP_CONNECT_CONFIRMS    0x00040000
#define     CBF_SKIP_REGISTRATIONS       0x00080000
#define     CBF_SKIP_UNREGISTRATIONS     0x00100000
#define     CBF_SKIP_DISCONNECTS         0x00200000
#define     CBF_SKIP_ALLNOTIFICATIONS    0x003c0000

/*
 * Application command flags
 */
#define     APPCMD_CLIENTONLY            0x00000010L
#define     APPCMD_FILTERINITS           0x00000020L
#define     APPCMD_MASK                  0x00000FF0L

/*
 * Application classification flags
 */
#define     APPCLASS_STANDARD            0x00000000L
#define     APPCLASS_MASK                0x0000000FL


BOOL DdeUninitialize(ULONG idInst);

/* conversation enumeration functions */

HCONVLIST DdeConnectList(ULONG idInst, HSZ hszService, HSZ hszTopic,
            HCONVLIST hConvList, CONVCONTEXT* pCC);
HCONV   DdeQueryNextServer(HCONVLIST hConvList, HCONV hConvPrev);
BOOL    DdeDisconnectList(HCONVLIST hConvList);

/* conversation control functions */

HCONV   DdeConnect(ULONG idInst, HSZ hszService, HSZ hszTopic,
            CONVCONTEXT* pCC);
BOOL    DdeDisconnect(HCONV hConv);
HCONV   DdeReconnect(HCONV hConv);

USHORT  DdeQueryConvInfo(HCONV hConv, ULONG idTransaction, CONVINFO* pConvInfo);
BOOL    DdeSetUserHandle(HCONV hConv, ULONG id, ULONG hUser);

BOOL    DdeAbandonTransaction(ULONG idInst, HCONV hConv, ULONG idTransaction);


/* app server interface functions */

BOOL    DdePostAdvise(ULONG idInst, HSZ hszTopic, HSZ hszItem);
BOOL    DdeEnableCallback(ULONG idInst, HCONV hConv, USHORT wCmd);

#define EC_ENABLEALL            0
#define EC_ENABLEONE            ST_BLOCKNEXT
#define EC_DISABLE              ST_BLOCKED
#define EC_QUERYWAITING         2

HDDEDATA DdeNameService(ULONG idInst, HSZ hsz1, HSZ hsz2, USHORT afCmd);

#define DNS_REGISTER        0x0001
#define DNS_UNREGISTER      0x0002
#define DNS_FILTERON        0x0004
#define DNS_FILTEROFF       0x0008

/* app client interface functions */

HDDEDATA DdeClientTransaction(void* pData, ULONG cbData,
        HCONV hConv, HSZ hszItem, USHORT wFmt, USHORT wType,
        ULONG dwTimeout, ULONG* pdwResult);

/* data transfer functions */

HDDEDATA DdeCreateDataHandle(ULONG idInst, void* pSrc, ULONG cb,
            ULONG cbOff, HSZ hszItem, USHORT wFmt, USHORT afCmd);
// HDDEDATA DdeAddData(HDDEDATA hData, void* pSrc, ULONG cb, ULONG cbOff);
ULONG    DdeGetData(HDDEDATA hData, void* pDst, ULONG cbMax, ULONG cbOff);
BYTE*    DdeAccessData(HDDEDATA hData, ULONG* pcbDataSize);
BOOL     DdeUnaccessData(HDDEDATA hData);
BOOL     DdeFreeDataHandle(HDDEDATA hData);

#define     HDATA_APPOWNED          0x0001

USHORT DdeGetLastError(ULONG idInst);

#define     DMLERR_NO_ERROR                    0       /* must be 0 */

#define     DMLERR_FIRST                       0x4000

#define     DMLERR_ADVACKTIMEOUT               0x4000
#define     DMLERR_BUSY                        0x4001
#define     DMLERR_DATAACKTIMEOUT              0x4002
#define     DMLERR_DLL_NOT_INITIALIZED         0x4003
#define     DMLERR_DLL_USAGE                   0x4004
#define     DMLERR_EXECACKTIMEOUT              0x4005
#define     DMLERR_INVALIDPARAMETER            0x4006
#define     DMLERR_LOW_MEMORY                  0x4007
#define     DMLERR_MEMORY_ERROR                0x4008
#define     DMLERR_NOTPROCESSED                0x4009
#define     DMLERR_NO_CONV_ESTABLISHED         0x400a
#define     DMLERR_POKEACKTIMEOUT              0x400b
#define     DMLERR_POSTMSG_FAILED              0x400c
#define     DMLERR_REENTRANCY                  0x400d
#define     DMLERR_SERVER_DIED                 0x400e
#define     DMLERR_SYS_ERROR                   0x400f
#define     DMLERR_UNADVACKTIMEOUT             0x4010
#define     DMLERR_UNFOUND_QUEUE_ID            0x4011

#define     DMLERR_LAST                        0x4011

HSZ     DdeCreateStringHandle(ULONG idInst, PSZ pStr, int iCodePage);
ULONG   DdeQueryString(ULONG idInst, HSZ hsz, PSZ pStr, ULONG cchMax,
                        int iCodePage);
BOOL    DdeFreeStringHandle(ULONG idInst, HSZ hsz);
BOOL    DdeKeepStringHandle(ULONG idInst, HSZ hsz);
int     DdeCmpStringHandles(HSZ hsz1, HSZ hsz2);



/* von OS/2 nicht unterstuetzte Win3.1 Clipboard-Formate */

#define CF_NOTSUPPORTED_BASE    0xff00

#ifndef CF_DIB
#define CF_DIB                  CF_NOTSUPPORTED_BASE+1
#endif

#ifndef CF_DIF
#define CF_DIF                  CF_NOTSUPPORTED_BASE+2
#endif

#ifndef CF_DSPMETAFILEPICT
#define CF_DSPMETAFILEPICT      CF_NOTSUPPORTED_BASE+3
#endif

#ifndef CF_METAFILEPICT
#define CF_METAFILEPICT         CF_NOTSUPPORTED_BASE+4
#endif

#ifndef CF_OEMTEXT
#define CF_OEMTEXT              CF_NOTSUPPORTED_BASE+5
#endif

#ifndef CF_OWNERDISPLAY
#define CF_OWNERDISPLAY         CF_NOTSUPPORTED_BASE+6
#endif

#ifndef CF_PENDATA
#define CF_PENDATA              CF_NOTSUPPORTED_BASE+7
#endif

#ifndef CF_RIFF
#define CF_RIFF                 CF_NOTSUPPORTED_BASE+8
#endif

#ifndef CF_SYLK
#define CF_SYLK                 CF_NOTSUPPORTED_BASE+9
#endif

#ifndef CF_TIFF
#define CF_TIFF                 CF_NOTSUPPORTED_BASE+10
#endif

#ifndef CF_WAVE
#define CF_WAVE                 CF_NOTSUPPORTED_BASE+11
#endif


#endif /* _DDEML_HXX */
