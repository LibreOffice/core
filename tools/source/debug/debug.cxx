/*************************************************************************
 *
 *  $RCSfile: debug.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:06 $
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

#define _TOOLS_DEBUG_CXX

#ifndef MAC
#if defined (UNX) || defined (GCC)
#include <unistd.h>
#else
#include <direct.h>
#endif
#endif

#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef OS2
#define INCL_DOSSEMAPHORES
#define INCL_DOSMISC
#define INCL_WINDIALOGS
#include <svpm.h>
#endif

#if defined ( WNT )
#include <svwin.h>
#endif

#include <debug.hxx>

// =======================================================================

#ifdef DBG_UTIL

// --- DbgErrors ---

static sal_Char const DbgError_ProfEnd1[]   = "DBG_PROF...() without DBG_PROFSTART(): ";
static sal_Char const DbgError_Xtor1[]      = "DBG_DTOR() or DBG_CHKTHIS() without DBG_CTOR(): ";

static sal_Char const DbgError_CtorDtor1[]  = "this == NULL in class ";
static sal_Char const DbgError_CtorDtor2[]  = "invalid this-Pointer %p in class ";
static sal_Char const DbgError_CtorDtor3[]  = "Error-Msg from Object %p in class ";

static sal_Char const DbgTrace_EnterCtor[]  = "Enter Ctor from class ";
static sal_Char const DbgTrace_LeaveCtor[]  = "Leave Ctor from class ";
static sal_Char const DbgTrace_EnterDtor[]  = "Enter Dtor from class ";
static sal_Char const DbgTrace_LeaveDtor[]  = "Leave Dtor from class ";
static sal_Char const DbgTrace_EnterMeth[]  = "Enter method from class ";
static sal_Char const DbgTrace_LeaveMeth[]  = "Leave method from class ";

// --- PointerList ---

#define PBLOCKCOUNT     1024

struct PBlock
{
    void*       aData[PBLOCKCOUNT];
    USHORT      nCount;
    PBlock*     pPrev;
    PBlock*     pNext;
};

class PointerList
{
private:
    PBlock*     pFirst;
    PBlock*     pLast;
    ULONG       nCount;

public:
                PointerList() { pFirst = NULL; pLast = NULL; nCount = 0; }
                ~PointerList();

    void        Add( const void* p );
    BOOL        Remove( const void* p );

    const void* Get( ULONG nPos ) const;
    BOOL        IsIn( const void* p ) const;
    ULONG       Count() const { return nCount; }
};

// --- Datentypen ---

#define DBG_MAXNAME     28

struct ProfType
{
    ULONG                   nCount;
    ULONG                   nTime;
    ULONG                   nMinTime;
    ULONG                   nMaxTime;
    ULONG                   nStart;
    ULONG                   nContinueTime;
    ULONG                   nContinueStart;
    sal_Char                aName[DBG_MAXNAME+1];
};

struct XtorType
{
    ULONG                   nCtorCalls;
    ULONG                   nDtorCalls;
    ULONG                   nMaxCount;
    ULONG                   nStatics;
    sal_Char                aName[DBG_MAXNAME+1];
    BOOL                    bTest;
    PointerList             aThisList;
};

struct DebugData
{
    DbgData                 aDbgData;
    USHORT                  bInit;
    DbgPrintLine            pDbgPrintMsgBox;
    DbgPrintLine            pDbgPrintWindow;
    DbgPrintLine            pDbgPrintShell;
    DbgPrintLine            pDbgPrintTestTool;
    PointerList*            pProfList;
    PointerList*            pXtorList;
    DbgTestSolarMutexProc   pDbgTestSolarMutex;
};

#define DBG_TEST_XTOR_EXTRA (DBG_TEST_XTOR_THIS |  DBG_TEST_XTOR_FUNC |               \
                             DBG_TEST_XTOR_EXIT |  DBG_TEST_XTOR_REPORT )

// ------------------------------
// - statische Verwaltungsdaten -
// ------------------------------

static DebugData aDebugData =
{
    {
    DBG_TEST_RESOURCE | DBG_TEST_MEM_INIT,
    TRUE,
    DBG_OUT_NULL,
    DBG_OUT_NULL,
    DBG_OUT_MSGBOX,
    0x77,
    0x55,
    0x33,
    "",
    "",
    "",
    "",
    "",
    "",
    },
    FALSE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

#ifndef MAC
static sal_Char aCurPath[260];
#endif

static int      bDbgImplInMain = FALSE;
static sal_Char aBuf[DBG_BUF_MAXLEN];
static sal_Char aBufOut[DBG_BUF_MAXLEN];

// =======================================================================

#if defined( WNT )
static CRITICAL_SECTION aImplCritDbgSection;
#elif defined( OS2 )
static HMTX             hImplCritDbgSection = 0;
#endif
static BOOL             bImplCritDbgSectionInit = FALSE;

// -----------------------------------------------------------------------

void ImplDbgInitLock()
{
#if defined( WNT )
    InitializeCriticalSection( &aImplCritDbgSection );
#elif defined( OS2 )
    DosCreateMutexSem( NULL, &hImplCritDbgSection, 0, FALSE );
#endif
    bImplCritDbgSectionInit = TRUE;
}

// -----------------------------------------------------------------------

void ImplDbgDeInitLock()
{
#if defined( WNT )
    DeleteCriticalSection( &aImplCritDbgSection );
#elif defined( OS2 )
    DosCloseMutexSem( hImplCritDbgSection );
#endif
    bImplCritDbgSectionInit = FALSE;
}

// -----------------------------------------------------------------------

void ImplDbgLock()
{
    if ( !bImplCritDbgSectionInit )
        return;

#if defined( WNT )
    EnterCriticalSection( &aImplCritDbgSection );
#elif defined( OS2 )
    DosRequestMutexSem( hImplCritDbgSection, SEM_INDEFINITE_WAIT );
#endif
}

// -----------------------------------------------------------------------

void ImplDbgUnlock()
{
    if ( !bImplCritDbgSectionInit )
        return;

#if defined( WNT )
    LeaveCriticalSection( &aImplCritDbgSection );
#elif defined( OS2 )
    DosReleaseMutexSem( hImplCritDbgSection );
#endif
}

// =======================================================================

// Aus DBMEM.CXX
#if (defined( WNT ) || defined( OS2 ) || defined( MAC )) && !defined ( SVX_LIGHT )
#define SV_MEMMGR
#endif
#ifdef SV_MEMMGR
void DbgImpCheckMemory( void* p = NULL );
void DbgImpCheckMemoryDeInit();
void DbgImpMemoryInfo( sal_Char* pBuf );
#endif

#if defined( OS2 )
#define FILE_LINEEND    "\r\n"
#else
#define FILE_LINEEND    "\n"
#endif

// =======================================================================

static BOOL ImplActivateDebugger( const sal_Char* pMsg )
{
#if defined( WNT )
    static sal_Char aImplDbgOutBuf[DBG_BUF_MAXLEN];
    strcpy( aImplDbgOutBuf, pMsg );
    strcat( aImplDbgOutBuf, "\r\n" );
    OutputDebugString( aImplDbgOutBuf );
    DebugBreak();
    return TRUE;
#elif defined( MAC )
    debugstr( (sal_Char*)pLine );
    return TRUE;
#else
    return FALSE;
#endif
}

// -----------------------------------------------------------------------

static BOOL ImplCoreDump( const sal_Char* pMsg )
{
#if defined( WNT )
    DebugBreak();
#else
    long* pTemp = 0;
    *pTemp = 0xCCCC;
#endif
    return TRUE;
}

// =======================================================================

static ULONG ImplGetPerfTime()
{
#if defined( WNT )
    return (ULONG)GetTickCount();
#elif defined( OS2 )
    PM_ULONG nClock;
    DosQuerySysInfo( QSV_MS_COUNT, QSV_MS_COUNT, &nClock, sizeof( nClock ) );
    return (ULONG)nClock;
#elif defined( MAC )
    long long millisec;
    Microseconds((UnsignedWide *)&millisec);
    millisec = ( millisec + 500L ) / 1000L;
    return (ULONG)millisec;
#else
    static ULONG    nImplTicksPerSecond = 0;
    static double   dImplTicksPerSecond;
    ULONG           nTicks = (ULONG)clock();

    if ( !nImplTicksPerSecond )
    {
        nImplTicksPerSecond = CLOCKS_PER_SEC;
        dImplTicksPerSecond = nImplTicksPerSecond;
    }

    double fTicks = nTicks;
    fTicks *= 1000;
    fTicks /= dImplTicksPerSecond;
    return (ULONG)fTicks;
#endif
}

// -----------------------------------------------------------------------

#if defined( OS2 )

typedef HFILE FILETYPE;

static FILETYPE FileOpen( const sal_Char* pFileName, const sal_Char* pOpenMode )
{
    HFILE   hFile = 0;
    ULONG   lAction = 0;
    ULONG   nOpen1 = FILE_OPEN;
    ULONG   nOpen2 = OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY;

    if ( *pOpenMode == 'w' )
    {
        nOpen1 = OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW;
        nOpen2 = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREADWRITE;
    }
    else if ( *pOpenMode == 'a' )
    {
        nOpen1 = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
        nOpen2 = OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYREADWRITE;
    }

    APIRET nRet = DosOpen( pFileName, &hFile, &lAction, 0,
                           FILE_NORMAL, nOpen1, nOpen2, 0L );

    if ( nRet )
        return NULL;
    else
    {
        if ( *pOpenMode == 'a' )
        {
            ULONG nTemp;
            DosSetFilePtr( hFile, 0, FILE_END, &nTemp );
        }

        return hFile;
    }
}

static ULONG FileRead( void* pData, int n1, int n2, FILETYPE pFile )
{
    ULONG nRead;
    DosRead( pFile, pData, n1*n2, &nRead );
    return nRead;
}

static ULONG FileWrite( void* pData, int n1, int n2, FILETYPE pFile )
{
    ULONG nWritten;
    DosWrite( pFile, pData, n1*n2, &nWritten );
    return nWritten;
}

static void FilePrintF( FILETYPE pFile, const sal_Char* pFStr, ... )
{
    static sal_Char aTempBuf[DBG_BUF_MAXLEN];

    va_list pList;

    va_start( pList, pFStr );
    vsprintf( aTempBuf, pFStr, pList );
    va_end( pList );

    FileWrite( aTempBuf, strlen( aTempBuf ), 1, pFile );
}

static void FileClose( FILETYPE pFile )
{
    DosClose( pFile );
}

#else

typedef FILE*       FILETYPE;
#define FileOpen    fopen
#define FileRead    fread
#define FileWrite   fwrite
#define FilePrintF  fprintf
#define FileClose   fclose

#endif

// =======================================================================

PointerList::~PointerList()
{
    PBlock* pBlock = pFirst;
    while ( pBlock )
    {
        PBlock* pNextBlock = pBlock->pNext;
        delete pBlock;
        pBlock = pNextBlock;
    }
}

// -----------------------------------------------------------------------

void PointerList::Add( const void* p )
{
    if ( !pFirst )
    {
        pFirst = new PBlock;
        memset( pFirst->aData, 0, PBLOCKCOUNT * sizeof( void* ) );
        pFirst->nCount = 0;
        pFirst->pPrev  = NULL;
        pFirst->pNext  = NULL;
        pLast = pFirst;
    }

    PBlock* pBlock = pFirst;
    while ( pBlock && (pBlock->nCount == PBLOCKCOUNT) )
        pBlock = pBlock->pNext;

    if ( !pBlock )
    {
        pBlock = new PBlock;
        memset( pBlock->aData, 0, PBLOCKCOUNT * sizeof( void* ) );
        pBlock->nCount = 0;
        pBlock->pPrev  = pLast;
        pBlock->pNext  = NULL;
        pLast->pNext   = pBlock;
        pLast          = pBlock;
    }

    USHORT i = 0;
    while ( pBlock->aData[i] )
        i++;

    pBlock->aData[i] = (void*)p;
    pBlock->nCount++;
    nCount++;
}

// -----------------------------------------------------------------------

BOOL PointerList::Remove( const void* p )
{
    if ( !p )
       return FALSE;

    PBlock* pBlock = pFirst;
    while ( pBlock )
    {
        USHORT i = 0;
        while ( i < PBLOCKCOUNT )
        {
            if ( ((ULONG)p) == ((ULONG)pBlock->aData[i]) )
            {
                pBlock->aData[i] = NULL;
                pBlock->nCount--;
                nCount--;

                if ( !pBlock->nCount )
                {
                    if ( pBlock->pPrev )
                        pBlock->pPrev->pNext = pBlock->pNext;
                    if ( pBlock->pNext )
                        pBlock->pNext->pPrev = pBlock->pPrev;
                    if ( pBlock == pFirst )
                        pFirst = pBlock->pNext;
                    if ( pBlock == pLast )
                        pLast = pBlock->pPrev;
                    delete pBlock;
                }

                return TRUE;
            }
            i++;
        }

        pBlock = pBlock->pNext;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

const void* PointerList::Get( ULONG nPos ) const
{
    if ( nCount <= nPos )
        return NULL;

    PBlock* pBlock = pFirst;
    ULONG   nStart = 0;
    while ( pBlock )
    {
        USHORT i = 0;
        while ( i < PBLOCKCOUNT )
        {
            if ( pBlock->aData[i] )
            {
                nStart++;
                if ( (nStart-1) == nPos )
                    return pBlock->aData[i];
            }

            i++;
        }

        pBlock = pBlock->pNext;
    }

    return NULL;
}

// -----------------------------------------------------------------------

BOOL PointerList::IsIn( const void* p ) const
{
    if ( !p )
       return FALSE;

    PBlock* pBlock = pFirst;
    while ( pBlock )
    {
        USHORT i = 0;
        while ( i < PBLOCKCOUNT )
        {
            if ( ((ULONG)p) == ((ULONG)pBlock->aData[i]) )
                return TRUE;
            i++;
        }

        pBlock = pBlock->pNext;
    }

    return FALSE;
}


// =======================================================================

static void DbgGetDbgFileName( sal_Char* pStr )
{
#if defined( UNX )
    const sal_Char* pName = getenv("DBGSV_INIT");
    if ( !pName )
        pName = ".dbgsv.init";
    strcpy( pStr, pName );
#elif defined( WNT )
    const sal_Char* pName = getenv("DBGSV_INIT");
    if ( pName )
        strcpy( pStr, pName );
    else
        GetProfileStringA( "sv", "dbgsv", "dbgsv.ini", pStr, 200 );
#elif defined( OS2 )
    PrfQueryProfileString( HINI_PROFILE, (PSZ)"SV", (PSZ)"DBGSV",
                           "dbgsv.ini", (PSZ)pStr, 200 );
#else
    strcpy( pStr, "dbgsv.ini" );
#endif
}

// -----------------------------------------------------------------------

static void DbgGetLogFileName( sal_Char* pStr )
{
#if defined( UNX )
    const sal_Char* pName = getenv("DBGSV_LOG");
    if ( !pName )
        pName = "dbgsv.log";
    strcpy( pStr, pName );
#elif defined( WNT )
    const sal_Char* pName = getenv("DBGSV_LOG");
    if ( pName )
        strcpy( pStr, pName );
    else
        GetProfileStringA( "sv", "dbgsvlog", "dbgsv.log", pStr, 200 );
#elif defined( OS2 )
    PrfQueryProfileString( HINI_PROFILE, (PSZ)"SV", (PSZ)"DBGSVLOG",
                           "dbgsv.log", (PSZ)pStr, 200 );
#else
    strcpy( pStr, "dbgsv.log" );
#endif
}

// -----------------------------------------------------------------------

static int DbgImplIsAllErrorOut()
{
#if defined( WNT )
    const sal_Char* pName = getenv("UPDATER");
    if ( pName && (strcmp( pName, "YES" ) == 0) )
        return TRUE;
    if ( GetProfileInt( "sv", "dbgallerrorout", 0 ) )
        return TRUE;
    else
        return FALSE;
#elif defined( OS2 )
    const sal_Char* pName = getenv("UPDATER");
    if ( pName && (strcmp( pName, "YES" ) == 0) )
        return TRUE;
    if ( PrfQueryProfileInt( HINI_PROFILE, (PSZ)"SV", (PSZ)"DBGALLERROROUT", 0 ) )
        return TRUE;
    else
        return FALSE;
#else
    return TRUE;
#endif
}

// -----------------------------------------------------------------------

static void DbgDebugBeep()
{
#if defined( WNT )
    MessageBeep( MB_ICONHAND );
#elif defined( OS2 )
    WinAlarm( HWND_DESKTOP, WA_ERROR );
#endif
}

// -----------------------------------------------------------------------

static DebugData* GetDebugData()
{
    if ( !aDebugData.bInit )
    {
        aDebugData.bInit = TRUE;

        // Default Debug-Namen setzen
        DbgGetLogFileName( aDebugData.aDbgData.aDebugName );

        // DEBUG.INI-File
        FILETYPE pDbgFile;
        DbgGetDbgFileName( aBuf );
        if ( (pDbgFile = FileOpen( aBuf, "r" )) != NULL )
        {
            FileRead( &(aDebugData.aDbgData), sizeof( DbgData ), 1, pDbgFile );
            FileClose( pDbgFile );
        }

#ifndef MAC
        getcwd( aCurPath, sizeof( aCurPath ) );
#endif

        // Daten initialisieren
        if ( aDebugData.aDbgData.nTestFlags & DBG_TEST_XTOR )
            aDebugData.pXtorList = new PointerList;
        if ( aDebugData.aDbgData.nTestFlags & DBG_TEST_PROFILING )
            aDebugData.pProfList = new PointerList;
        if ( aDebugData.aDbgData.nErrorOut < DBG_OUT_MSGBOX )
        {
            if ( !DbgImplIsAllErrorOut() )
                aDebugData.aDbgData.nErrorOut = DBG_OUT_MSGBOX;
        }
    }

    return &aDebugData;
}

// -----------------------------------------------------------------------

inline DebugData* ImplGetDebugData()
{
    if ( !aDebugData.bInit )
        return GetDebugData();
    else
        return &aDebugData;
}

// -----------------------------------------------------------------------

static FILETYPE ImplDbgInitFile()
{
    static BOOL bFileInit = FALSE;

#ifndef MAC
    getcwd( aBuf, sizeof( aBuf ) );
    chdir( aCurPath );
#endif

    DebugData*  pData = GetDebugData();
    FILETYPE    pDebugFile;

    if ( !bFileInit )
    {
        bFileInit = TRUE;

        if ( pData->aDbgData.bOverwrite )
            pDebugFile = FileOpen( pData->aDbgData.aDebugName, "w" );
        else
            pDebugFile = FileOpen( pData->aDbgData.aDebugName, "a" );

        if ( pDebugFile )
        {
            time_t  nTime = time( 0 );
            tm*     pTime;
#ifdef UNX
            tm      aTime;
            pTime = localtime_r( &nTime, &aTime );
#else
            pTime = localtime( &nTime );
#endif

            // Header ausgeben
            FilePrintF( pDebugFile, "******************************************************************************%s", FILE_LINEEND );
            FilePrintF( pDebugFile, "%s%s", pData->aDbgData.aDebugName, FILE_LINEEND );
            if ( pTime )
                FilePrintF( pDebugFile, "%s%s", asctime( pTime ), FILE_LINEEND );
        }
    }
    else
        pDebugFile = FileOpen( pData->aDbgData.aDebugName, "a" );

#ifndef MAC
    chdir( aBuf );
#endif

    return pDebugFile;
}

// -----------------------------------------------------------------------

static void ImplDbgPrintFile( const sal_Char* pLine )
{
    FILETYPE pDebugFile = ImplDbgInitFile();

    if ( pDebugFile )
    {
        FilePrintF( pDebugFile, "%s%s", pLine, FILE_LINEEND );
        FileClose( pDebugFile );
    }
}

// -----------------------------------------------------------------------

static int ImplStrSearch( const sal_Char* pSearchStr, int nSearchLen,
                          const sal_Char* pStr, int nLen )
{
    int nPos = 0;
    while ( nPos+nSearchLen <= nLen )
    {
        if ( strncmp( pStr+nPos, pSearchStr, nSearchLen ) == 0 )
            return 1;
        nPos++;
    }

    return 0;
}

// -----------------------------------------------------------------------

static int ImplDbgFilter( const sal_Char* pFilter, const sal_Char* pMsg,
                          int bEmpty )
{
    int nStrLen = strlen( pFilter );
    if ( !nStrLen )
        return bEmpty;

    int nMsgLen = strlen( pMsg );
    const sal_Char* pTok = pFilter;
    int         nTok = 0;
    while ( pTok[nTok] )
    {
        if ( pTok[nTok] == ';' )
        {
            if ( nTok && ImplStrSearch( pTok, nTok, pMsg, nMsgLen ) )
                return TRUE;

            pTok += nTok+1;
            nTok = 0;
        }

        nTok++;
    }

    if ( nTok && ImplStrSearch( pTok, nTok, pMsg, nMsgLen ) )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

static void DebugInit()
{
    bDbgImplInMain = TRUE;
    ImplDbgInitLock();
}

// -----------------------------------------------------------------------

static void DebugDeInit()
{
    DebugData*  pData = GetDebugData();
    ULONG       i;
    ULONG       nCount;
    ULONG       nOldOut;

    // Statistik-Ausgaben immer in File
    nOldOut = pData->aDbgData.nTraceOut;
    pData->aDbgData.nTraceOut = DBG_OUT_FILE;

    // Xtor-Liste ausgeben
    if ( pData->pXtorList && pData->pXtorList->Count() &&
         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_REPORT) )
    {
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "Object Report" );
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "%-27s : %-9s : %-9s : %-7s : %-3s : %-6s :",
                 "XTor-List", "Ctor", "Dtor", "MaxInst", "St.", "Diff." );
        DbgOutf( "----------------------------:-----------:-----------:---------:----:---------:" );
        for( i = 0, nCount = pData->pXtorList->Count(); i < nCount; i++ )
        {
            XtorType* pXtorData = (XtorType*)pData->pXtorList->Get( i );
            if ( pXtorData->bTest )
            {
                // Static-Objekte dazurechnen
                pXtorData->nDtorCalls += pXtorData->nStatics;
                if ( pXtorData->nStatics && (pXtorData->nDtorCalls > pXtorData->nCtorCalls) )
                    pXtorData->nDtorCalls = pXtorData->nCtorCalls;
                DbgOutf( "%-27s : %9lu : %9lu : %7lu : %3lu : %4lu %-1s :",
                         pXtorData->aName, pXtorData->nCtorCalls, pXtorData->nDtorCalls,
                         pXtorData->nMaxCount, pXtorData->nStatics,
                         pXtorData->nCtorCalls - pXtorData->nDtorCalls,
                         (pXtorData->nCtorCalls - pXtorData->nDtorCalls) ? "!" : " " );
            }
        }
        DbgOutf( "==============================================================================" );
    }

    // Aufraeumen
    if ( pData->pXtorList )
    {
        for( i = 0, nCount = pData->pXtorList->Count(); i < nCount; i++ )
        {
            XtorType* pXtorData = (XtorType*)pData->pXtorList->Get( i );
            delete pXtorData;
        }
        delete pData->pXtorList;
        pData->pXtorList = NULL;
    }

    // Alles auf FALSE setzen, damit globale Variablen nicht das
    // System zum Abstuerzen bringt. Dabei muessen aber die
    // Memory-Flags erhalten bleiben, da sonst new/delete in globalen
    // Variablen abstuerzen, da die Pointeranpassung dann nicht mehr richtig
    // funktioniert
    pData->aDbgData.nTraceOut   = nOldOut;
    pData->aDbgData.nTestFlags &= (DBG_TEST_MEM | DBG_TEST_PROFILING);
    pData->pDbgPrintTestTool    = NULL;
    pData->pDbgPrintShell       = NULL;
    pData->pDbgPrintWindow      = NULL;
    pData->pDbgPrintShell       = NULL;
    ImplDbgDeInitLock();
}

// -----------------------------------------------------------------------

static void DebugGlobalDeInit()
{
    DebugData*  pData = GetDebugData();
    ULONG       i;
    ULONG       nCount;
    ULONG       nOldOut;

    // Statistik-Ausgaben immer in File
    nOldOut = pData->aDbgData.nTraceOut;
    pData->aDbgData.nTraceOut = DBG_OUT_FILE;

    // Profileliste ausgeben
    if ( pData->pProfList && pData->pProfList->Count() )
    {
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "Profiling Report" );
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "%-25s : %-9s : %-6s : %-6s : %-6s : %-9s :",
                 "Prof-List (ms)", "Time", "Min", "Max", "Ave", "Count" );
        DbgOutf( "--------------------------:-----------:--------:--------:--------:-----------:" );
        for( i = 0, nCount = pData->pProfList->Count(); i < nCount; i++ )
        {
            ProfType* pProfData = (ProfType*)pData->pProfList->Get( i );
            ULONG nAve = pProfData->nTime / pProfData->nCount;
            DbgOutf( "%-25s : %9lu : %6lu : %6lu : %6lu : %9lu :",
                     pProfData->aName, pProfData->nTime,
                     pProfData->nMinTime, pProfData->nMaxTime, nAve,
                     pProfData->nCount );
        }
        DbgOutf( "==============================================================================" );
    }

    // Aufraeumen
    if ( pData->pProfList )
    {
        for( i = 0, nCount = pData->pProfList->Count(); i < nCount; i++ )
        {
            ProfType* pProfData = (ProfType*)pData->pProfList->Get( i );
            delete pProfData;
        }
        delete pData->pProfList;
        pData->pProfList = NULL;
    }

#ifdef SV_MEMMGR
    DbgImpCheckMemoryDeInit();
#endif

    // Profiling-Flags ausschalten
    pData->aDbgData.nTraceOut   = nOldOut;
    pData->aDbgData.nTestFlags &= ~DBG_TEST_PROFILING;
}

// -----------------------------------------------------------------------

void ImpDbgOutfBuf( sal_Char* pBuf, const sal_Char* pFStr, ... )
{
    va_list pList;

    va_start( pList, pFStr );
    vsprintf( aBuf, pFStr, pList );
    va_end( pList );

    strcat( pBuf, aBuf );
    strcat( pBuf, "\n" );
}

// -----------------------------------------------------------------------

static void DebugXTorInfo( sal_Char* pBuf )
{
    DebugData*  pData = GetDebugData();
    ULONG       i;
    ULONG       nCount;

    // Xtor-Liste ausgeben
    if ( pData->pXtorList && pData->pXtorList->Count() &&
         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_REPORT) )
    {
        ImpDbgOutfBuf( pBuf, "------------------------------------------------------------------------------" );
        ImpDbgOutfBuf( pBuf, "Object Report" );
        ImpDbgOutfBuf( pBuf, "------------------------------------------------------------------------------" );
        ImpDbgOutfBuf( pBuf, "%-27s : %-9s : %-9s : %-7s : %-3s : %-6s :",
                       "XTor-List", "Ctor", "Dtor", "MaxInst", "St.", "Diff." );
        ImpDbgOutfBuf( pBuf, "----------------------------:-----------:-----------:---------:----:---------:" );
        for( i = 0, nCount = pData->pXtorList->Count(); i < nCount; i++ )
        {
            XtorType* pXtorData = (XtorType*)pData->pXtorList->Get( i );
            if ( pXtorData->bTest )
            {
                ImpDbgOutfBuf( pBuf, "%-27s : %9lu : %9lu : %7lu : %3lu : %6lu :",
                               pXtorData->aName, pXtorData->nCtorCalls, pXtorData->nDtorCalls,
                               pXtorData->nMaxCount, pXtorData->nStatics,
                               pXtorData->nCtorCalls - pXtorData->nDtorCalls );
            }
        }
        ImpDbgOutfBuf( pBuf, "==============================================================================" );
        ImpDbgOutfBuf( pBuf, "" );
    }
}

// -----------------------------------------------------------------------

void* DbgFunc( USHORT nAction, void* pParam )
{
    DebugData* pData = ImplGetDebugData();

    if ( nAction == DBG_FUNC_GETDATA )
        return (void*)&(pData->aDbgData);
    else
    {
        switch ( nAction )
        {
            case DBG_FUNC_DEBUGSTART:
                DebugInit();
                break;

            case DBG_FUNC_DEBUGEND:
                DebugDeInit();
                break;

            case DBG_FUNC_GLOBALDEBUGEND:
                DebugGlobalDeInit();
                break;

            case DBG_FUNC_SETPRINTMSGBOX:
                pData->pDbgPrintMsgBox = (DbgPrintLine)pParam;
                break;

            case DBG_FUNC_SETPRINTWINDOW:
                pData->pDbgPrintWindow = (DbgPrintLine)pParam;
                break;

            case DBG_FUNC_SETPRINTSHELL:
                pData->pDbgPrintShell = (DbgPrintLine)pParam;
                break;

            case DBG_FUNC_SETPRINTTESTTOOL:
                pData->pDbgPrintTestTool = (DbgPrintLine)pParam;
                break;

            case DBG_FUNC_SAVEDATA:
                {
                FILETYPE pDbgFile;
                DbgGetDbgFileName( aBuf );
                if ( (pDbgFile = FileOpen( aBuf, "w" )) != NULL )
                {
                    FileWrite( pParam, sizeof( DbgData ), 1, pDbgFile );
                    FileClose( pDbgFile );
                }
                }
                break;

            case DBG_FUNC_MEMTEST:
#ifdef SV_MEMMGR
                DbgImpCheckMemory( pParam );
#endif
                break;

            case DBG_FUNC_XTORINFO:
                DebugXTorInfo( (sal_Char*)pParam );
                break;

            case DBG_FUNC_MEMINFO:
#ifdef SV_MEMMGR
                DbgImpMemoryInfo( (sal_Char*)pParam );
#endif
                break;

            case DBG_FUNC_COREDUMP:
                ImplCoreDump( NULL );
                break;

            case DBG_FUNC_ALLERROROUT:
                return (void*)(ULONG)DbgImplIsAllErrorOut();

            case DBG_FUNC_SETTESTSOLARMUTEX:
                pData->pDbgTestSolarMutex = (DbgTestSolarMutexProc)pParam;
                break;

            case DBG_FUNC_TESTSOLARMUTEX:
                if ( pData->pDbgTestSolarMutex )
                    pData->pDbgTestSolarMutex();
                break;

            case DBG_FUNC_PRINTFILE:
                ImplDbgPrintFile( (const sal_Char*)pParam );
                break;
       }

        return NULL;
    }
}

// -----------------------------------------------------------------------

void DbgProf( USHORT nAction, DbgDataType* pDbgData )
{
    // Ueberhaupt Profiling-Test an
    DebugData*  pData = ImplGetDebugData();
    ProfType*   pProfData = (ProfType*)pDbgData->pData;
    ULONG       nTime;

    if ( !(pData->aDbgData.nTestFlags & DBG_TEST_PROFILING) )
        return;

    if ( (nAction != DBG_PROF_START) && !pProfData )
    {
        strcpy( aBuf, DbgError_ProfEnd1 );
        strcat( aBuf, pDbgData->pName );
        DbgError( aBuf );
        return;
    }

    switch ( nAction )
    {
        case DBG_PROF_START:
            if ( !pDbgData->pData )
            {
                pDbgData->pData = (void*)new ProfType;
                pProfData = (ProfType*)pDbgData->pData;
                strncpy( pProfData->aName, pDbgData->pName, DBG_MAXNAME );
                pProfData->aName[DBG_MAXNAME] = '\0';
                pProfData->nCount           = 0;
                pProfData->nTime            = 0;
                pProfData->nMinTime         = 0xFFFFFFFF;
                pProfData->nMaxTime         = 0;
                pProfData->nStart           = 0xFFFFFFFF;
                pProfData->nContinueTime    = 0;
                pProfData->nContinueStart   = 0xFFFFFFFF;
                pData->pProfList->Add( (void*)pProfData );
            }

            if ( pProfData->nStart == 0xFFFFFFFF )
            {
                pProfData->nStart = ImplGetPerfTime();
                pProfData->nCount++;
            }
            break;

        case DBG_PROF_STOP:
            nTime = ImplGetPerfTime();

            if ( pProfData->nStart == 0xFFFFFFFF )
            {
                DbgError( DbgError_ProfEnd1 );
                return;
            }

            if ( pProfData->nContinueStart != 0xFFFFFFFF )
            {
                pProfData->nContinueTime += ImplGetPerfTime() - pProfData->nContinueStart;
                pProfData->nContinueStart = 0xFFFFFFFF;
            }

            nTime -= pProfData->nStart;
            nTime -= pProfData->nContinueTime;

            if ( nTime < pProfData->nMinTime )
                pProfData->nMinTime = nTime;

            if ( nTime > pProfData->nMaxTime )
                pProfData->nMaxTime = nTime;

            pProfData->nTime += nTime;

            pProfData->nStart         = 0xFFFFFFFF;
            pProfData->nContinueTime  = 0;
            pProfData->nContinueStart = 0xFFFFFFFF;
            break;

        case DBG_PROF_CONTINUE:
            if ( pProfData->nContinueStart != 0xFFFFFFFF )
            {
                pProfData->nContinueTime += ImplGetPerfTime() - pProfData->nContinueStart;
                pProfData->nContinueStart = 0xFFFFFFFF;
            }
            break;

        case DBG_PROF_PAUSE:
            if ( pProfData->nContinueStart == 0xFFFFFFFF )
                pProfData->nContinueStart = ImplGetPerfTime();
            break;
    }
}

// -----------------------------------------------------------------------

void DbgXtor( DbgDataType* pDbgData, USHORT nAction, const void* pThis,
              DbgUsr fDbgUsr )
{
    DebugData* pData = ImplGetDebugData();

    // Verbindung zu Debug-Memory-Manager testen
#ifdef SV_MEMMGR
    if ( pData->aDbgData.nTestFlags & DBG_TEST_MEM_XTOR )
        DbgImpCheckMemory();
#endif

    // Schnell-Test
    if ( !(pData->aDbgData.nTestFlags & DBG_TEST_XTOR) )
        return;

    XtorType* pXtorData = (XtorType*)pDbgData->pData;
    if ( !pXtorData )
    {
        pDbgData->pData = (void*)new XtorType;
        pXtorData = (XtorType*)pDbgData->pData;
        strncpy( pXtorData->aName, pDbgData->pName, DBG_MAXNAME );
        pXtorData->aName[DBG_MAXNAME] = '\0';
        pXtorData->nCtorCalls   = 0;
        pXtorData->nDtorCalls   = 0;
        pXtorData->nMaxCount    = 0;
        pXtorData->nStatics     = 0;
        pXtorData->bTest        = TRUE;
        pData->pXtorList->Add( (void*)pXtorData );

        if ( !ImplDbgFilter( pData->aDbgData.aInclClassFilter, pXtorData->aName, TRUE ) )
            pXtorData->bTest = FALSE;
        if ( ImplDbgFilter( pData->aDbgData.aExclClassFilter, pXtorData->aName, FALSE ) )
            pXtorData->bTest = FALSE;
    }
    if ( !pXtorData->bTest )
        return;

    USHORT nAct = nAction & ~DBG_XTOR_DTOROBJ;

    // Trace (Enter)
    if ( (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_TRACE) &&
         !(nAction & DBG_XTOR_DTOROBJ) )
    {
        if ( nAct != DBG_XTOR_CHKOBJ )
        {
            if ( nAct == DBG_XTOR_CTOR )
                strcpy( aBuf, DbgTrace_EnterCtor );
            else if ( nAct == DBG_XTOR_DTOR )
                strcpy( aBuf, DbgTrace_EnterDtor );
            else
                strcpy( aBuf, DbgTrace_EnterMeth );
            strcat( aBuf, pDbgData->pName );
            DbgTrace( aBuf );
        }
    }

    // Sind noch Xtor-Tests als Trace an
    if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXTRA )
    {
        // DBG_CTOR-Aufruf vor allen anderen DBG_XTOR-Aufrufen
        if ( ((nAction & ~DBG_XTOR_DTOROBJ) != DBG_XTOR_CTOR) && !pDbgData->pData )
        {
            strcpy( aBuf, DbgError_Xtor1 );
            strcat( aBuf, pDbgData->pName );
            DbgError( aBuf );
            return;
        }

        // Testen, ob This-Pointer gueltig
        if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_THIS )
        {
            if ( (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXIT) ||
                 !(nAction & DBG_XTOR_DTOROBJ) )
            {
                // This-Pointer == NULL
                if ( !pThis )
                {
                    strcpy( aBuf, DbgError_CtorDtor1 );
                    strcat( aBuf, pDbgData->pName );
                    DbgError( aBuf );
                    return;
                }

                if ( (nAction & ~DBG_XTOR_DTOROBJ) != DBG_XTOR_CTOR )
                {
                    if ( !pXtorData->aThisList.IsIn( pThis ) )
                    {
                        sprintf( aBuf, DbgError_CtorDtor2, pThis );
                        strcat( aBuf, pDbgData->pName );
                        DbgError( aBuf );
                    }
                }
            }
        }

        // Function-Test durchfuehren und Verwaltungsdaten updaten
        const sal_Char* pMsg = NULL;
        switch ( nAction & ~DBG_XTOR_DTOROBJ )
        {
            case DBG_XTOR_CTOR:
                if ( nAction & DBG_XTOR_DTOROBJ )
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXIT) &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                else
                {
                    pXtorData->nCtorCalls++;
                    if ( !bDbgImplInMain )
                        pXtorData->nStatics++;
                    if ( (pXtorData->nCtorCalls-pXtorData->nDtorCalls) > pXtorData->nMaxCount )
                        pXtorData->nMaxCount = pXtorData->nCtorCalls - pXtorData->nDtorCalls;

                    if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_THIS )
                        pXtorData->aThisList.Add( pThis );
                }
                break;

            case DBG_XTOR_DTOR:
                if ( nAction & DBG_XTOR_DTOROBJ )
                {
                    pXtorData->nDtorCalls++;
                    if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_THIS )
                        pXtorData->aThisList.Remove( pThis );
                }
                else
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                break;

            case DBG_XTOR_CHKTHIS:
            case DBG_XTOR_CHKOBJ:
                if ( nAction & DBG_XTOR_DTOROBJ )
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXIT) &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                else
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                break;
        }

        // Gegebenenfalls Fehlermeldung ausgeben
        if ( pMsg )
        {
            sprintf( aBuf, DbgError_CtorDtor3, pThis );
            strcat( aBuf, pDbgData->pName );
            strcat( aBuf, ": \n" );
            strcat( aBuf, pMsg );
            DbgError( aBuf );
        }
    }

    // Trace (Leave)
    if ( (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_TRACE) &&
         (nAction & DBG_XTOR_DTOROBJ) )
    {
        if ( nAct != DBG_XTOR_CHKOBJ )
        {
            if ( nAct == DBG_XTOR_CTOR )
                strcpy( aBuf, DbgTrace_LeaveCtor );
            else if ( nAct == DBG_XTOR_DTOR )
                strcpy( aBuf, DbgTrace_LeaveDtor );
            else
                strcpy( aBuf, DbgTrace_LeaveMeth );
            strcat( aBuf, pDbgData->pName );
            DbgTrace( aBuf );
        }
    }
}

// -----------------------------------------------------------------------

void DbgOut( const sal_Char* pMsg, USHORT nDbgOut, const sal_Char* pFile, USHORT nLine )
{
    static BOOL bIn = FALSE;
    DebugData*  pData = GetDebugData();
    sal_Char*   pStr;
    ULONG       nOut;
    int         nBufLen = 0;

    if ( bIn )
        return;
    bIn = TRUE;

    if ( nDbgOut == DBG_OUT_ERROR )
    {
        nOut = pData->aDbgData.nErrorOut;
        pStr = "Error: ";
        if ( pData->aDbgData.nErrorOut == DBG_OUT_FILE )
            DbgDebugBeep();
    }
    else if ( nDbgOut == DBG_OUT_WARNING )
    {
        nOut = pData->aDbgData.nWarningOut;
        pStr = "Warning: ";
    }
    else
    {
        nOut = pData->aDbgData.nTraceOut;
        pStr = NULL;
    }

    if ( nOut == DBG_OUT_NULL )
    {
        bIn = FALSE;
        return;
    }

    if ( (nDbgOut != DBG_OUT_ERROR) || DbgImplIsAllErrorOut() )
    {
        if ( !ImplDbgFilter( pData->aDbgData.aInclFilter, pMsg, TRUE ) )
        {
            bIn = FALSE;
            return;
        }
        if ( ImplDbgFilter( pData->aDbgData.aExclFilter, pMsg, FALSE ) )
        {
            bIn = FALSE;
            return;
        }
    }

    ImplDbgLock();

    if ( pStr )
    {
        strcpy( aBufOut, pStr );
        nBufLen = strlen( pStr );
    }
    else
        aBufOut[0] = '\0';

    int nMsgLen = strlen( pMsg );
    if ( nBufLen+nMsgLen > DBG_BUF_MAXLEN )
    {
        int nCopyLen = DBG_BUF_MAXLEN-nBufLen-3;
        strncpy( &(aBufOut[nBufLen]), pMsg, nCopyLen );
        strcpy( &(aBufOut[nBufLen+nCopyLen]), "..." );
    }
    else
        strcpy( &(aBufOut[nBufLen]), pMsg );

    if ( pFile && nLine && (nBufLen+nMsgLen < DBG_BUF_MAXLEN) )
    {
        if ( nOut == DBG_OUT_MSGBOX )
            strcat( aBufOut, "\n" );
        else
            strcat( aBufOut, " " );
        strcat( aBufOut, "From File " );
        strcat( aBufOut, pFile );
        strcat( aBufOut, " at Line " );

        // Line in String umwandeln und dranhaengen
        sal_Char aLine[9];
        sal_Char*  pLine = &aLine[7];
        USHORT i;
        memset( aLine, 0, sizeof( aLine ) );
        do
        {
            i = nLine % 10;
            pLine--;
            *(pLine) = (sal_Char)i + 48;
            nLine /= 10;
        }
        while ( nLine );
        strcat( aBufOut, pLine );
    }

    if ( nOut == DBG_OUT_COREDUMP )
    {
        if ( !ImplCoreDump( aBufOut ) )
            nOut = DBG_OUT_DEBUGGER;
    }

    if ( nOut == DBG_OUT_DEBUGGER )
    {
        if ( !ImplActivateDebugger( aBufOut ) )
            nOut = DBG_OUT_TESTTOOL;
    }

    if ( nOut == DBG_OUT_TESTTOOL )
    {
        if ( pData->pDbgPrintTestTool )
            pData->pDbgPrintTestTool( aBufOut );
        else
            nOut = DBG_OUT_MSGBOX;
    }

    if ( nOut == DBG_OUT_MSGBOX )
    {
        if ( pData->pDbgPrintMsgBox )
            pData->pDbgPrintMsgBox( aBufOut );
        else
            nOut = DBG_OUT_SHELL;
    }

    if ( nOut == DBG_OUT_SHELL )
    {
        if ( pData->pDbgPrintShell )
            pData->pDbgPrintShell( aBufOut );
        else
            nOut = DBG_OUT_WINDOW;
    }

    if ( nOut == DBG_OUT_WINDOW )
    {
        if ( pData->pDbgPrintWindow )
            pData->pDbgPrintWindow( aBufOut );
        else
            nOut = DBG_OUT_FILE;
    }

    if ( nOut == DBG_OUT_FILE )
        ImplDbgPrintFile( aBufOut );

    ImplDbgUnlock();

    bIn = FALSE;
}

// -----------------------------------------------------------------------

void DbgOutTypef( USHORT nDbgOut, const sal_Char* pFStr, ... )
{
    va_list pList;

    va_start( pList, pFStr );
    vsprintf( aBuf, pFStr, pList );
    va_end( pList );

    DbgOut( aBuf, nDbgOut );
}

// -----------------------------------------------------------------------

void DbgOutf( const sal_Char* pFStr, ... )
{
    va_list pList;

    va_start( pList, pFStr );
    vsprintf( aBuf, pFStr, pList );
    va_end( pList );

    DbgOut( aBuf );
}

// =======================================================================

#else

void* DbgFunc( USHORT, void* ) { return NULL; }

void DbgProf( USHORT, DbgDataType* ) {}
void DbgXtor( DbgDataType*, USHORT, const void*, DbgUsr ) {}

void DbgOut( const sal_Char*, USHORT, const sal_Char*, USHORT ) {}
void DbgOutTypef( USHORT, const sal_Char*, ... ) {}
void DbgOutf( const sal_Char*, ... ) {}

#endif
