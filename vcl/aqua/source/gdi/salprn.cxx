/*************************************************************************
 *
 *  $RCSfile: salprn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pluby $ $Date: 2000-11-01 03:12:45 $
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

#include <string.h>
#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#define _SV_SALPRN_CXX

#ifndef _SV_SALAQUA_HXX
#include <salaqua.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif

#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif

#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif

// =======================================================================

static char aImplWindows[] = "windows";
static char aImplDevices[] = "devices";
static char aImplDevice[]  = "device";

// =======================================================================

static ULONG ImplWinQueueStatusToSal( DWORD nWinStatus )
{
    ULONG nStatus = 0;
#ifdef WIN
    if ( nWinStatus & PRINTER_STATUS_PAUSED )
        nStatus |= QUEUE_STATUS_PAUSED;
    if ( nWinStatus & PRINTER_STATUS_ERROR )
        nStatus |= QUEUE_STATUS_ERROR;
    if ( nWinStatus & PRINTER_STATUS_PENDING_DELETION )
        nStatus |= QUEUE_STATUS_PENDING_DELETION;
    if ( nWinStatus & PRINTER_STATUS_PAPER_JAM )
        nStatus |= QUEUE_STATUS_PAPER_JAM;
    if ( nWinStatus & PRINTER_STATUS_PAPER_OUT )
        nStatus |= QUEUE_STATUS_PAPER_OUT;
    if ( nWinStatus & PRINTER_STATUS_MANUAL_FEED )
        nStatus |= QUEUE_STATUS_MANUAL_FEED;
    if ( nWinStatus & PRINTER_STATUS_PAPER_PROBLEM )
        nStatus |= QUEUE_STATUS_PAPER_PROBLEM;
    if ( nWinStatus & PRINTER_STATUS_OFFLINE )
        nStatus |= QUEUE_STATUS_OFFLINE;
    if ( nWinStatus & PRINTER_STATUS_IO_ACTIVE )
        nStatus |= QUEUE_STATUS_IO_ACTIVE;
    if ( nWinStatus & PRINTER_STATUS_BUSY )
        nStatus |= QUEUE_STATUS_BUSY;
    if ( nWinStatus & PRINTER_STATUS_PRINTING )
        nStatus |= QUEUE_STATUS_PRINTING;
    if ( nWinStatus & PRINTER_STATUS_OUTPUT_BIN_FULL )
        nStatus |= QUEUE_STATUS_OUTPUT_BIN_FULL;
    if ( nWinStatus & PRINTER_STATUS_WAITING )
        nStatus |= QUEUE_STATUS_WAITING;
    if ( nWinStatus & PRINTER_STATUS_PROCESSING )
        nStatus |= QUEUE_STATUS_PROCESSING;
    if ( nWinStatus & PRINTER_STATUS_INITIALIZING )
        nStatus |= QUEUE_STATUS_INITIALIZING;
    if ( nWinStatus & PRINTER_STATUS_WARMING_UP )
        nStatus |= QUEUE_STATUS_WARMING_UP;
    if ( nWinStatus & PRINTER_STATUS_TONER_LOW )
        nStatus |= QUEUE_STATUS_TONER_LOW;
    if ( nWinStatus & PRINTER_STATUS_NO_TONER )
        nStatus |= QUEUE_STATUS_NO_TONER;
    if ( nWinStatus & PRINTER_STATUS_PAGE_PUNT )
        nStatus |= QUEUE_STATUS_PAGE_PUNT;
    if ( nWinStatus & PRINTER_STATUS_USER_INTERVENTION )
        nStatus |= QUEUE_STATUS_USER_INTERVENTION;
    if ( nWinStatus & PRINTER_STATUS_OUT_OF_MEMORY )
        nStatus |= QUEUE_STATUS_OUT_OF_MEMORY;
    if ( nWinStatus & PRINTER_STATUS_DOOR_OPEN )
        nStatus |= QUEUE_STATUS_DOOR_OPEN;
    if ( nWinStatus & PRINTER_STATUS_SERVER_UNKNOWN )
        nStatus |= QUEUE_STATUS_SERVER_UNKNOWN;
    if ( nWinStatus & PRINTER_STATUS_POWER_SAVE )
        nStatus |= QUEUE_STATUS_POWER_SAVE;
    if ( !nStatus && !(nWinStatus & PRINTER_STATUS_NOT_AVAILABLE) )
        nStatus |= QUEUE_STATUS_READY;
#endif
    return nStatus;
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
// !!! UNICODE - NT Optimierung !!!
    DWORD           i;
    DWORD           n;
    DWORD           nBytes = 0;
//    DWORD           nInfoRet;
    DWORD           nInfoPrn2;
    BOOL            bFound = FALSE;
#ifdef WIN
    PRINTER_INFO_2* pWinInfo2 = NULL;
    PRINTER_INFO_2* pGetInfo2;
    EnumPrintersA( PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &nBytes, &nInfoPrn2 );
#endif
    if ( nBytes )
    {
#ifdef WIN
        pWinInfo2 = (PRINTER_INFO_2*)new BYTE[nBytes];
        if ( EnumPrintersA( PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE)pWinInfo2, nBytes, &nBytes, &nInfoPrn2 ) )
        {
            pGetInfo2 = pWinInfo2;
            for ( i = 0; i < nInfoPrn2; i++ )
            {
                SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
                pInfo->maPrinterName = ImplSalGetUniString( pGetInfo2->pPrinterName );
                pInfo->maDriver      = ImplSalGetUniString( pGetInfo2->pDriverName );
                XubString aPortName;
                if ( pGetInfo2->pPortName )
                    aPortName = ImplSalGetUniString( pGetInfo2->pPortName );
                // pLocation can be 0 (the Windows docu doesn't describe this)
                if ( pGetInfo2->pLocation && strlen( pGetInfo2->pLocation ) )
                    pInfo->maLocation = ImplSalGetUniString( pGetInfo2->pLocation );
                else
                    pInfo->maLocation = aPortName;
                // pComment can be 0 (the Windows docu doesn't describe this)
                if ( pGetInfo2->pComment )
                    pInfo->maComment = ImplSalGetUniString( pGetInfo2->pComment );
                pInfo->mnStatus      = ImplWinQueueStatusToSal( pGetInfo2->Status );
                pInfo->mnJobs        = pGetInfo2->cJobs;
                pInfo->mpSysData     = new XubString( aPortName );
                pList->Add( pInfo );
                pGetInfo2++;
            }

            bFound = TRUE;
        }
#endif
    }

/* Siehe Kommentar unten !!!
    EnumPrinters( PRINTER_ENUM_NETWORK | PRINTER_ENUM_REMOTE, NULL, 1, NULL, 0, &nBytes, &nInfoRet );
    if ( nBytes )
    {
        PRINTER_INFO_1* pWinInfo1 = (PRINTER_INFO_1*)new BYTE[nBytes];
        if ( EnumPrinters( PRINTER_ENUM_NETWORK | PRINTER_ENUM_REMOTE, NULL, 1, (LPBYTE)pWinInfo1, nBytes, &nBytes, &nInfoRet ) )
        {
            PRINTER_INFO_1* pGetInfo1 = pWinInfo1;
            for ( i = 0; i < nInfoRet; i++ )
            {
                // Feststellen, ob Printer durch erste Abfrage schon gefunden
                // wurde
                BOOL bAdd = TRUE;
#ifdef WIN
                if ( pWinInfo2 )
                {
                    pGetInfo2 = pWinInfo2;
                    for ( n = 0; n < nInfoPrn2; n++ )
                    {
                        if ( strcmp( pGetInfo1->pName, pGetInfo2->pPrinterName ) == 0 )
                        {
                            bAdd = FALSE;
                            break;
                        }
                        pGetInfo2++;
                    }
                }
#endif
                // Wenn neuer Printer, dann aufnehmen
                if ( bAdd )
                {
                    SalPrinterQueueInfo*    pInfo = new SalPrinterQueueInfo;
                    XubString               aPrnName( pGetInfo1->pName );
                    pInfo->maPrinterName = aPrnName;
                    pInfo->maDriver      = "winspool";
                    pInfo->maComment     = pGetInfo1->pComment;
                    pInfo->mnStatus      = 0;
                    pInfo->mnJobs        = QUEUE_JOBS_DONTKNOW;
                    pInfo->mpSysData     = new String();
                    pList->Add( pInfo );
                }
                pGetInfo1++;
            }

            bFound = TRUE;
        }

        delete pWinInfo1;
    }
*/

//    if ( bFound )
//        return;

#ifdef WIN
// !!! UNICODE - NT Optimierung !!!
    // Drucker aus WIN.INI lesen
    UINT    nSize = 4096;
    char*   pBuf = new char[nSize];
    UINT    nRead = GetProfileStringA( aImplDevices, NULL, "", pBuf, nSize );
    while ( nRead >= nSize-2 )
    {
        nSize += 2048;
        delete pBuf;
        pBuf = new char[nSize];
        nRead = GetProfileStringA( aImplDevices, NULL, "", pBuf, nSize );
    }

    // Druckernamen aus Buffer extrahieren und Liste aufbauen
    char* pName = pBuf;
    while ( *pName )
    {
        char*   pPortName;
        char*   pTmp;
        char    aPortBuf[256];
        GetProfileStringA( aImplDevices, pName, "", aPortBuf, sizeof( aPortBuf ) );

        pPortName = aPortBuf;

        // Namen anlegen
        xub_StrLen nNameLen = strlen( pName );
        XubString aName( ImplSalGetUniString( pName, nNameLen ) );

        // Treibernamen rausfischen
        pTmp = pPortName;
        while ( *pTmp != ',' )
            pTmp++;
        XubString aDriver( ImplSalGetUniString( pPortName, (USHORT)(pTmp-pPortName) ) );
        pPortName = pTmp;

        // Alle Portnamen raussuchen
        do
        {
            pPortName++;
            pTmp = pPortName;
            while ( *pTmp && (*pTmp != ',') )
                pTmp++;

            String aPortName( ImplSalGetUniString( pPortName, (USHORT)(pTmp-pPortName) ) );

            // Neuen Eintrag anlegen
            // !!! Da ich zu bloeb bin, die Netzwerk-Printer zur 5.0
            // !!! richtig zu integrieren, gehen wir zusaetzlich
            // !!! noch ueber das W16-Interface, da uns dort die
            // !!! Drucker noch einfach und schnell geliefert werden
            // !!! ohne das wir jetzt zu grossen Aufwand treiben muessen.
            // !!! Somit sollten wir dann jedenfalls nicht schlechter sein
            // !!! als in einer 4.0 SP2.
            // Feststellen, ob Printer durch erste Abfrage schon gefunden
            // wurde
            BOOL bAdd = TRUE;
            if ( pWinInfo2 )
            {
                pGetInfo2 = pWinInfo2;
                for ( n = 0; n < nInfoPrn2; n++ )
                {
                    if ( aName.EqualsIgnoreCaseAscii( pGetInfo2->pPrinterName ) )
                    {
                        bAdd = FALSE;
                        break;
                    }
                    pGetInfo2++;
                }
            }
            // Wenn neuer Printer, dann aufnehmen
            if ( bAdd )
            {
                SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
                pInfo->maPrinterName = aName;
                pInfo->maDriver      = aDriver;
                pInfo->maLocation    = aPortName;
                pInfo->mnStatus      = 0;
                pInfo->mnJobs        = QUEUE_JOBS_DONTKNOW;
                pInfo->mpSysData     = new XubString( aPortName );
                pList->Add( pInfo );
            }
        }
        while ( *pTmp == ',' );

        pName += nNameLen + 1;
    }

    delete pBuf;
    delete pWinInfo2;
#endif
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
// !!! UNICODE - NT Optimierung !!!
    DWORD               nBytes = 0;
    DWORD               nInfoRet;
#ifdef WIN
    PRINTER_INFO_2*     pWinInfo2;
    EnumPrintersA( PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &nBytes, &nInfoRet );
#endif
    if ( nBytes )
    {
#ifdef WIN
        pWinInfo2 = (PRINTER_INFO_2*)new BYTE[nBytes];
        if ( EnumPrintersA( PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE)pWinInfo2, nBytes, &nBytes, &nInfoRet ) )
        {
            PRINTER_INFO_2* pGetInfo2 = pWinInfo2;
            for ( DWORD i = 0; i < nInfoRet; i++ )
            {
                if ( pInfo->maPrinterName.EqualsAscii( pGetInfo2->pPrinterName ) &&
                     pInfo->maDriver.EqualsAscii( pGetInfo2->pDriverName ) )
                {
                    if ( pGetInfo2->pLocation && strlen( pGetInfo2->pLocation ) )
                        pInfo->maLocation = ImplSalGetUniString( pGetInfo2->pLocation );
                    else
                        pInfo->maLocation = ImplSalGetUniString( pGetInfo2->pPortName );
                    pInfo->mnStatus = ImplWinQueueStatusToSal( pGetInfo2->Status );
                    pInfo->mnJobs   = pGetInfo2->cJobs;
                    break;
                }

                pGetInfo2++;
            }
        }

        delete pWinInfo2;
#endif
    }
}

// -----------------------------------------------------------------------

void SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete (String*)(pInfo->mpSysData);
    delete pInfo;
}

// -----------------------------------------------------------------------

// !!! UNICODE - NT Optimierung !!!
XubString SalInstance::GetDefaultPrinter()
{
    // Default-Printer-String aus win.ini holen
    char szBuffer[256];
#ifdef WIN
    GetProfileStringA( aImplWindows, aImplDevice, "", szBuffer, sizeof( szBuffer ) );
#endif
    if ( szBuffer[0] )
    {
        // Printername suchen
        char* pBuf = szBuffer;
        char* pTmp = pBuf;
        while ( *pTmp && (*pTmp != ',') )
            pTmp++;
        return ImplSalGetUniString( pBuf, (xub_StrLen)(pTmp-pBuf) );
    }
    else
        return XubString();
}

// =======================================================================

static DWORD ImplDeviceCaps( SalInfoPrinter* pPrinter, WORD nCaps,
                             LPTSTR pOutput, const ImplJobSetup* pSetupData )
{
#ifdef WIN
    DEVMODE* pDevMode;
    if ( !pSetupData || !pSetupData->mpDriverData )
        pDevMode = NULL;
    else
        pDevMode = SAL_DEVMODE( pSetupData );

// !!! UNICODE - NT Optimierung !!!
    return DeviceCapabilitiesA( ImplSalGetWinAnsiString( pPrinter->maPrinterData.maDeviceName, TRUE ).GetBuffer(),
                                ImplSalGetWinAnsiString( pPrinter->maPrinterData.maPortName, TRUE ).GetBuffer(),
                                nCaps, (LPSTR)pOutput, pDevMode );
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------

static BOOL ImplTestSalJobSetup( SalInfoPrinter* pPrinter,
                                 ImplJobSetup* pSetupData, BOOL bDelete )
{
    if ( pSetupData && pSetupData->mpDriverData )
    {
        // Signature und Groesse muss uebereinstimmen, damit wir keine
        // JobSetup's von anderen Systemen setzen
        if ( (pSetupData->mnSystem == JOBSETUP_SYSTEM_WINDOWS) &&
             (pSetupData->mnDriverDataLen > sizeof( SalDriverData )) &&
             (((SalDriverData*)(pSetupData->mpDriverData))->mnSysSignature == SAL_DRIVERDATA_SYSSIGN) )
            return TRUE;
        else if ( bDelete )
        {
            delete pSetupData->mpDriverData;
            pSetupData->mpDriverData = NULL;
            pSetupData->mnDriverDataLen = 0;
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static BOOL ImplUpdateSalJobSetup( SalInfoPrinter* pPrinter, ImplJobSetup* pSetupData,
                                   BOOL bIn, SalFrame* pVisibleDlgParent )
{
#ifdef WIN
    HANDLE hPrn;
// !!! UNICODE - NT Optimierung !!!
    if ( !OpenPrinterA( (LPSTR)ImplSalGetWinAnsiString( pPrinter->maPrinterData.maDeviceName, TRUE ).GetBuffer(), &hPrn, NULL ) )
        return FALSE;

    LONG            nRet;
    LONG            nSysJobSize;
    VCLWINDOW           hWnd = 0;
    DWORD           nMode = DM_OUT_BUFFER;
    ULONG           nDriverDataLen = 0;
    SalDriverData*  pOutBuffer = NULL;
    DEVMODE*        pInDevBuffer = NULL;
    DEVMODE*        pOutDevBuffer = NULL;

// !!! UNICODE - NT Optimierung !!!
    nSysJobSize = DocumentPropertiesA( hWnd, hPrn,
                                       (LPSTR)ImplSalGetWinAnsiString( pPrinter->maPrinterData.maDeviceName, TRUE ).GetBuffer(),
                                       NULL, NULL, 0 );
    if ( nSysJobSize < 0 )
    {
        ClosePrinter( hPrn );
        return FALSE;
    }

    // Outputbuffer anlegen
    nDriverDataLen              = sizeof(SalDriverData)+nSysJobSize-1;
    pOutBuffer                  = (SalDriverData*)SvMemAlloc( nDriverDataLen );
    memset( pOutBuffer, 0, nDriverDataLen );
    pOutDevBuffer               = (LPDEVMODE)(pOutBuffer->maDriverData);
    pOutBuffer->mnSysSignature  = SAL_DRIVERDATA_SYSSIGN;
    pOutBuffer->mnVersion       = SAL_DRIVERDATA_VERSION;
    pOutBuffer->mnDriverOffset  = (USHORT)(((SalDriverData*)NULL)->maDriverData);

    // Testen, ob wir einen geeigneten Inputbuffer haben
    if ( bIn && ImplTestSalJobSetup( pPrinter, pSetupData, FALSE ) )
    {
        pInDevBuffer = SAL_DEVMODE( pSetupData );
        nMode |= DM_IN_BUFFER;
    }

    // Testen, ob Dialog angezeigt werden soll
    if ( pVisibleDlgParent )
    {
        hWnd = pVisibleDlgParent->maFrameData.mhWnd;
        nMode |= DM_IN_PROMPT;
    }

// !!! UNICODE - NT Optimierung !!!
    // Release mutex, in the other case we don't get paints and so on
    ULONG nMutexCount = ImplSalReleaseYieldMutex();
    nRet = DocumentPropertiesA( hWnd, hPrn,
                                (LPSTR)ImplSalGetWinAnsiString( pPrinter->maPrinterData.maDeviceName, TRUE ).GetBuffer(),
                                pOutDevBuffer, pInDevBuffer, nMode );
    ImplSalAcquireYieldMutex( nMutexCount );
    ClosePrinter( hPrn );

    if ( (nRet < 0) || (pVisibleDlgParent && (nRet == IDCANCEL)) )
    {
        SvMemFree( pOutBuffer );
        return FALSE;
    }

    // String-Buffer am Ende immer mit 0 initialisieren, damit
    // die JobSetups nach Moeglichkeit bei memcmp immer
    // identisch sind
    if ( pOutDevBuffer->dmSize >= 32 )
    {
        USHORT nLen = strlen( (const char*)pOutDevBuffer->dmDeviceName );
        if ( nLen < sizeof( pOutDevBuffer->dmDeviceName ) )
            memset( pOutDevBuffer->dmDeviceName+nLen, 0, sizeof( pOutDevBuffer->dmDeviceName )-nLen );
    }
    if ( pOutDevBuffer->dmSize >= 102 )
    {
        USHORT nLen = strlen( (const char*)pOutDevBuffer->dmFormName );
        if ( nLen < sizeof( pOutDevBuffer->dmFormName ) )
            memset( pOutDevBuffer->dmFormName+nLen, 0, sizeof( pOutDevBuffer->dmFormName )-nLen );
    }

    // Daten updaten
    if ( pSetupData->mpDriverData )
        delete pSetupData->mpDriverData;
    pSetupData->mnDriverDataLen = nDriverDataLen;
    pSetupData->mpDriverData    = (BYTE*)pOutBuffer;
    pSetupData->mnSystem        = JOBSETUP_SYSTEM_WINDOWS;

    return TRUE;
#else
    return FALSE;
#endif
}

// -----------------------------------------------------------------------

static void ImplDevModeToJobSetup( SalInfoPrinter* pPrinter, ImplJobSetup* pSetupData, ULONG nFlags )
{
#ifdef WIN
    if ( !pSetupData || !pSetupData->mpDriverData )
        return;

    DEVMODE* pDevMode = SAL_DEVMODE( pSetupData );

    // Orientation
    if ( nFlags & SAL_JOBSET_ORIENTATION )
    {
        if ( pDevMode->dmOrientation == DMORIENT_PORTRAIT )
            pSetupData->meOrientation = ORIENTATION_PORTRAIT;
        else if ( pDevMode->dmOrientation == DMORIENT_LANDSCAPE )
            pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
    }

    // PaperBin
    if ( nFlags & SAL_JOBSET_PAPERBIN )
    {
        ULONG nCount = ImplDeviceCaps( pPrinter, DC_BINS, NULL, pSetupData );

        if ( nCount && (nCount != ((ULONG)-1)) )
        {
            WORD* pBins = new WORD[nCount];
            memset( (BYTE*)pBins, 0, nCount*sizeof(WORD) );
            ImplDeviceCaps( pPrinter, DC_BINS, (LPTSTR)pBins, pSetupData );
            pSetupData->mnPaperBin = 0;

            // search the right bin and assign index to mnPaperBin
            for( ULONG i = 0; i < nCount; i++ )
            {
                if( pDevMode->dmDefaultSource == pBins[ i ] )
                {
                    pSetupData->mnPaperBin = (USHORT)i;
                    break;
                }
            }

            delete[] pBins;
        }
    }

    // PaperSize
    if ( nFlags & SAL_JOBSET_PAPERSIZE )
    {
        pSetupData->mnPaperWidth  = pDevMode->dmPaperWidth*10;
        pSetupData->mnPaperHeight = pDevMode->dmPaperLength*10;
        switch( pDevMode->dmPaperSize )
        {
            case( DMPAPER_A3 ):
                pSetupData->mePaperFormat = PAPER_A3;
                break;
            case( DMPAPER_A4 ):
                pSetupData->mePaperFormat = PAPER_A4;
                break;
            case( DMPAPER_A5 ):
                pSetupData->mePaperFormat = PAPER_A5;
                break;
            case( DMPAPER_B4 ):
                pSetupData->mePaperFormat = PAPER_B4;
                break;
            case( DMPAPER_B5 ):
                pSetupData->mePaperFormat = PAPER_B5;
                break;
            case( DMPAPER_LETTER ):
                pSetupData->mePaperFormat = PAPER_LETTER;
                break;
            case( DMPAPER_LEGAL ):
                pSetupData->mePaperFormat = PAPER_LEGAL;
                break;
            case( DMPAPER_TABLOID ):
                pSetupData->mePaperFormat = PAPER_TABLOID;
                break;
            default:
                pSetupData->mePaperFormat = PAPER_USER;
                break;
        }
    }
#endif
}

// -----------------------------------------------------------------------

static BOOL ImplPaperSizeEqual( short nPaperWidth1, short nPaperHeight1,
                                short nPaperWidth2, short nPaperHeight2 )
{
    return (((nPaperWidth1 >= nPaperWidth2-1) && (nPaperWidth1 <= nPaperWidth2+1)) &&
            ((nPaperHeight1 >= nPaperHeight2-1) && (nPaperHeight1 <= nPaperHeight2+1)));
}

// -----------------------------------------------------------------------

static void ImplJobSetupToDevMode( SalInfoPrinter* pPrinter, ImplJobSetup* pSetupData, ULONG nFlags )
{
#ifdef WIN
    if ( !pSetupData || !pSetupData->mpDriverData )
        return;

    DEVMODE* pDevMode = SAL_DEVMODE( pSetupData );

    // Orientation
    if ( nFlags & SAL_JOBSET_ORIENTATION )
    {
        pDevMode->dmFields |= DM_ORIENTATION;
        if ( pSetupData->meOrientation == ORIENTATION_PORTRAIT )
            pDevMode->dmOrientation = DMORIENT_PORTRAIT;
        else
            pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
    }

    // PaperBin
    if ( nFlags & SAL_JOBSET_PAPERBIN )
    {
        ULONG nCount = ImplDeviceCaps( pPrinter, DC_BINS, NULL, pSetupData );

        if ( nCount && (nCount != ((ULONG)-1)) )
        {
            WORD* pBins = new WORD[nCount];
            memset( pBins, 0, nCount*sizeof(WORD) );
            ImplDeviceCaps( pPrinter, DC_BINS, (LPTSTR)pBins, pSetupData );
            pDevMode->dmFields |= DM_DEFAULTSOURCE;
            pDevMode->dmDefaultSource = pBins[ pSetupData->mnPaperBin ];
            delete[] pBins;
        }
    }

    // PaperSize
    if ( nFlags & SAL_JOBSET_PAPERSIZE )
    {
        pDevMode->dmFields      |= DM_PAPERSIZE;
        pDevMode->dmPaperWidth   = 0;
        pDevMode->dmPaperLength  = 0;

        switch( pDevMode->dmPaperSize )
        {
            case( PAPER_A3 ):
                pDevMode->dmPaperSize = DMPAPER_A3;
                break;
            case( PAPER_A4 ):
                pDevMode->dmPaperSize = DMPAPER_A4;
                break;
            case( PAPER_A5 ):
                pDevMode->dmPaperSize = DMPAPER_A5;
                break;
            case( PAPER_B4 ):
                pDevMode->dmPaperSize = DMPAPER_B4;
                break;
            case( PAPER_B5 ):
                pDevMode->dmPaperSize = DMPAPER_B5;
                break;
            case( PAPER_LETTER ):
                pDevMode->dmPaperSize = DMPAPER_LETTER;
                break;
            case( PAPER_LEGAL ):
                pDevMode->dmPaperSize = DMPAPER_LEGAL;
                break;
            case( PAPER_TABLOID ):
                pDevMode->dmPaperSize = DMPAPER_TABLOID;
                break;
            default:
                {
                short   nPaper = 0;
                ULONG   nPaperCount = ImplDeviceCaps( pPrinter, DC_PAPERS, NULL, pSetupData );
                WORD*   pPapers = NULL;
                ULONG   nPaperSizeCount = ImplDeviceCaps( pPrinter, DC_PAPERSIZE, NULL, pSetupData );
                POINT*  pPaperSizes = NULL;
                if ( nPaperCount && (nPaperCount != ((ULONG)-1)) )
                {
                    pPapers = new WORD[nPaperCount];
                    memset( pPapers, 0, nPaperCount*sizeof(WORD) );
                    ImplDeviceCaps( pPrinter, DC_PAPERS, (LPTSTR)pPapers, pSetupData );
                }
                if ( nPaperSizeCount && (nPaperSizeCount != ((ULONG)-1)) )
                {
                    pPaperSizes = new POINT[nPaperSizeCount];
                    memset( pPaperSizes, 0, nPaperSizeCount*sizeof(POINT) );
                    ImplDeviceCaps( pPrinter, DC_PAPERSIZE, (LPTSTR)pPaperSizes, pSetupData );
                }
                if ( (nPaperSizeCount == nPaperCount) && pPapers && pPaperSizes )
                {
                    // Alle Papierformate vergleichen und ein passendes
                    // raussuchen
                    for ( ULONG i = 0; i < nPaperCount; i++ )
                    {
                        if ( ImplPaperSizeEqual( (short)(pSetupData->mnPaperWidth/10),
                                                 (short)(pSetupData->mnPaperHeight/10),
                                                 (short)pPaperSizes[i].x,
                                                 (short)pPaperSizes[i].y ) )
                        {
                            nPaper = pPapers[i];
                            break;
                        }
                    }
                }
                if ( pPapers )
                    delete pPapers;
                if ( pPaperSizes )
                    delete pPaperSizes;

                if ( nPaper )
                    pDevMode->dmPaperSize = nPaper;
                else
                {
                    pDevMode->dmFields     |= DM_PAPERLENGTH | DM_PAPERWIDTH;
                    pDevMode->dmPaperSize   = DMPAPER_USER;
                    pDevMode->dmPaperWidth  = pSetupData->mnPaperWidth/10;
                    pDevMode->dmPaperLength = pSetupData->mnPaperHeight/10;
                }
                }
                break;
        }
    }
#endif
}

// -----------------------------------------------------------------------

static VCLVIEW ImplCreateSalPrnIC( SalInfoPrinter* pPrinter, ImplJobSetup* pSetupData )
{
#ifdef WIN
    LPDEVMODE pDevMode;
    if ( pSetupData && pSetupData->mpDriverData )
        pDevMode = SAL_DEVMODE( pSetupData );
    else
        pDevMode = NULL;
// !!! UNICODE - NT Optimierung !!!
    VCLVIEW hDC = CreateICA( ImplSalGetWinAnsiString( pPrinter->maPrinterData.maDriverName, TRUE ).GetBuffer(),
                         ImplSalGetWinAnsiString( pPrinter->maPrinterData.maDeviceName, TRUE ).GetBuffer(),
                         0,
                         (LPDEVMODE)pDevMode );
    return hDC;
#else
    return NULL;
#endif
}

// -----------------------------------------------------------------------

static SalGraphics* ImplCreateSalPrnGraphics( VCLVIEW hDC )
{
    SalGraphics* pGraphics = new SalGraphics;
    pGraphics->maGraphicsData.mhDC      = hDC;
    pGraphics->maGraphicsData.mhWnd     = 0;
    pGraphics->maGraphicsData.mbPrinter = TRUE;
    pGraphics->maGraphicsData.mbVirDev  = FALSE;
    pGraphics->maGraphicsData.mbWindow  = FALSE;
    pGraphics->maGraphicsData.mbScreen  = FALSE;
    ImplSalInitGraphics( &(pGraphics->maGraphicsData) );
    return pGraphics;
}

// -----------------------------------------------------------------------

static BOOL ImplUpdateSalPrnIC( SalInfoPrinter* pPrinter, ImplJobSetup* pSetupData )
{
    VCLVIEW hNewDC = ImplCreateSalPrnIC( pPrinter, pSetupData );
    if ( !hNewDC )
        return FALSE;

    if ( pPrinter->maPrinterData.mpGraphics )
    {
        ImplSalDeInitGraphics( &(pPrinter->maPrinterData.mpGraphics->maGraphicsData) );
#ifdef WIN
        DeleteDC( pPrinter->maPrinterData.mpGraphics->maGraphicsData.mhDC );
#endif
        delete pPrinter->maPrinterData.mpGraphics;
    }

    SalGraphics* pGraphics = ImplCreateSalPrnGraphics( hNewDC );
    pPrinter->maPrinterData.mhDC        = hNewDC;
    pPrinter->maPrinterData.mpGraphics  = pGraphics;

    return TRUE;
}

// =======================================================================

SalInfoPrinter* SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
    SalInfoPrinter* pPrinter = new SalInfoPrinter;
    pPrinter->maPrinterData.maDriverName    = pQueueInfo->maDriver;
    pPrinter->maPrinterData.maDeviceName    = pQueueInfo->maPrinterName;
    pPrinter->maPrinterData.maPortName      = *(String*)(pQueueInfo->mpSysData);

    // Testen, ob Setupdaten zum Drucker gehoeren (erst aufrufen, nachdem
    // die Member gesetzt sind, da diese in dieser Routine abgefragt werden)
    ImplTestSalJobSetup( pPrinter, pSetupData, TRUE );

    VCLVIEW hDC = ImplCreateSalPrnIC( pPrinter, pSetupData );
    if ( !hDC )
    {
        delete pPrinter;
        return NULL;
    }

    SalGraphics* pGraphics = ImplCreateSalPrnGraphics( hDC );
    pPrinter->maPrinterData.mhDC        = hDC;
    pPrinter->maPrinterData.mpGraphics  = pGraphics;
    if ( !pSetupData->mpDriverData )
        ImplUpdateSalJobSetup( pPrinter, pSetupData, FALSE, NULL );
    ImplDevModeToJobSetup( pPrinter, pSetupData, SAL_JOBSET_ALL );
    pSetupData->mnSystem = JOBSETUP_SYSTEM_WINDOWS;

    return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// =======================================================================

SalInfoPrinter::SalInfoPrinter()
{
    maPrinterData.mhDC          = 0;
    maPrinterData.mpGraphics    = NULL;
    maPrinterData.mbGraphics    = FALSE;
}

// -----------------------------------------------------------------------

SalInfoPrinter::~SalInfoPrinter()
{
    if ( maPrinterData.mpGraphics )
    {
        ImplSalDeInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
#ifdef WIN
        DeleteDC( maPrinterData.mpGraphics->maGraphicsData.mhDC );
#endif
        delete maPrinterData.mpGraphics;
    }
}

// -----------------------------------------------------------------------

SalGraphics* SalInfoPrinter::GetGraphics()
{
    if ( maPrinterData.mbGraphics )
        return NULL;

    if ( maPrinterData.mpGraphics )
        maPrinterData.mbGraphics = TRUE;

    return maPrinterData.mpGraphics;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::ReleaseGraphics( SalGraphics* )
{
    maPrinterData.mbGraphics = FALSE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
    if ( ImplUpdateSalJobSetup( this, pSetupData, TRUE, pFrame ) )
    {
        ImplDevModeToJobSetup( this, pSetupData, SAL_JOBSET_ALL );
        return ImplUpdateSalPrnIC( this, pSetupData );
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
    if ( !ImplTestSalJobSetup( this, pSetupData, FALSE ) )
        return FALSE;
    return ImplUpdateSalPrnIC( this, pSetupData );
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
    ImplJobSetupToDevMode( this, pSetupData, nFlags );
    if ( ImplUpdateSalJobSetup( this, pSetupData, TRUE, NULL ) )
    {
        ImplDevModeToJobSetup( this, pSetupData, nFlags );
        return ImplUpdateSalPrnIC( this, pSetupData );
    }

    return FALSE;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
#ifdef WIN
    DWORD nRet = ImplDeviceCaps( this, DC_BINS, NULL, pSetupData );
    if ( nRet && (nRet != ((ULONG)-1)) )
        return nRet;
    else
#endif
        return 0;
}

// -----------------------------------------------------------------------

XubString SalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, ULONG nPaperBin )
{
// !!! UNICODE - NT Optimierung !!!
    XubString aPaperBinName;

#ifdef WIN
    DWORD nBins = ImplDeviceCaps( this, DC_BINNAMES, NULL, pSetupData );
    if ( (nPaperBin < nBins) && (nBins != ((ULONG)-1)) )
    {
        char* pBuffer = new char[nBins*24];
        DWORD nRet = ImplDeviceCaps( this, DC_BINNAMES, pBuffer, pSetupData );
        if ( nRet && (nRet != ((ULONG)-1)) )
            aPaperBinName = ImplSalGetUniString( (const char*)(pBuffer + (nPaperBin*24)) );
        delete pBuffer;
    }
#endif

    return aPaperBinName;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, USHORT nType )
{
#ifdef WIN
    DWORD nRet;

    switch ( nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return TRUE;
        case PRINTER_CAPABILITIES_COPIES:
            nRet = ImplDeviceCaps( this, DC_COPIES, NULL, pSetupData );
            if ( nRet && (nRet != ((ULONG)-1)) )
                return nRet;
            return 0;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
            nRet = ImplDeviceCaps( this, DC_COLLATE, NULL, pSetupData );
            if ( nRet && (nRet != ((ULONG)-1)) )
            {
                nRet = ImplDeviceCaps( this, DC_COPIES, NULL, pSetupData );
                if ( nRet && (nRet != ((ULONG)-1)) )
                     return nRet;
            }
            return 0;

        case PRINTER_CAPABILITIES_SETORIENTATION:
            nRet = ImplDeviceCaps( this, DC_ORIENTATION, NULL, pSetupData );
            if ( nRet && (nRet != ((ULONG)-1)) )
                return TRUE;
            return FALSE;

        case PRINTER_CAPABILITIES_SETPAPERBIN:
            nRet = ImplDeviceCaps( this, DC_BINS, NULL, pSetupData );
            if ( nRet && (nRet != ((ULONG)-1)) )
                return TRUE;
            return FALSE;

        case PRINTER_CAPABILITIES_SETPAPERSIZE:
        case PRINTER_CAPABILITIES_SETPAPER:
            nRet = ImplDeviceCaps( this, DC_PAPERS, NULL, pSetupData );
            if ( nRet && (nRet != ((ULONG)-1)) )
                return TRUE;
            return FALSE;
    }
#endif

    return 0;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::GetPageInfo( const ImplJobSetup*,
                                  long& rOutWidth, long& rOutHeight,
                                  long& rPageOffX, long& rPageOffY,
                                  long& rPageWidth, long& rPageHeight )
{
    VCLVIEW hDC = maPrinterData.mhDC;

#ifdef WIN
    rOutWidth   = GetDeviceCaps( hDC, HORZRES );
    rOutHeight  = GetDeviceCaps( hDC, VERTRES );

    rPageOffX   = GetDeviceCaps( hDC, PHYSICALOFFSETX );
    rPageOffY   = GetDeviceCaps( hDC, PHYSICALOFFSETY );
    rPageWidth  = GetDeviceCaps( hDC, PHYSICALWIDTH );
    rPageHeight = GetDeviceCaps( hDC, PHYSICALHEIGHT );
#endif
}

// =======================================================================

SalPrinter* SalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    SalPrinter* pPrinter = new SalPrinter;
    pPrinter->maPrinterData.mpInfoPrinter = pInfoPrinter;
    return pPrinter;
}

// -----------------------------------------------------------------------

void SalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// =======================================================================

WIN_BOOL CALLBACK SalPrintAbortProc( VCLVIEW hPrnDC, int /* nError */ )
{
    SalData*    pSalData = GetSalData();
    SalPrinter* pPrinter;
    BOOL        bWhile = TRUE;
    int         i = 0;

    do
    {
#ifdef WIN
        // Messages verarbeiten
        MSG aMsg;
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &aMsg );
            ImplDispatchMessage( &aMsg );
            i++;
            if ( i > 15 )
                bWhile = FALSE;
        }
        else
#endif
            bWhile = FALSE;

        pPrinter = pSalData->mpFirstPrinter;
        while ( pPrinter )
        {
            if( pPrinter->maPrinterData.mhDC == hPrnDC )
                break;

            pPrinter = pPrinter->maPrinterData.mpNextPrinter;
        }

        if ( !pPrinter || pPrinter->maPrinterData.mbAbort )
            return FALSE;
    }
    while ( bWhile );

    return TRUE;
}

// -----------------------------------------------------------------------

#ifdef WIN
static LPDEVMODE ImplSalSetCopies( LPDEVMODE pDevMode, ULONG nCopies, BOOL bCollate )
{
    LPDEVMODE pNewDevMode = pDevMode;
    if ( pDevMode && (nCopies > 1) )
    {
        if ( nCopies > 32765 )
            nCopies = 32765;
        ULONG nDevSize = pDevMode->dmSize+pDevMode->dmDriverExtra;
        pNewDevMode = (LPDEVMODE)new BYTE[nDevSize];
        memcpy( pNewDevMode, pDevMode, nDevSize );
        pDevMode = pNewDevMode;
        pDevMode->dmFields |= DM_COPIES;
        pDevMode->dmCopies  = (short)(USHORT)nCopies;
        if ( aSalShlData.mbW40 )
        {
            pDevMode->dmFields |= DM_COLLATE;
            if ( bCollate )
                pDevMode->dmCollate = DMCOLLATE_TRUE;
            else
                pDevMode->dmCollate = DMCOLLATE_FALSE;
        }
    }

    return pNewDevMode;
}
#endif

// -----------------------------------------------------------------------

SalPrinter::SalPrinter()
{
    SalData* pSalData = GetSalData();

    maPrinterData.mhDC          = 0;
    maPrinterData.mpGraphics    = NULL;
    maPrinterData.mbAbort       = FALSE;
    maPrinterData.mnCopies      = 0;
    maPrinterData.mbCollate     = FALSE;

    // insert frame in framelist
    maPrinterData.mpNextPrinter = pSalData->mpFirstPrinter;
    pSalData->mpFirstPrinter = this;
}

// -----------------------------------------------------------------------

SalPrinter::~SalPrinter()
{
    SalData* pSalData = GetSalData();

    // DC freigeben, wenn er noch durch ein AbortJob existiert
    VCLVIEW hDC = maPrinterData.mhDC;
    if ( hDC )
    {
        if ( maPrinterData.mpGraphics )
        {
            ImplSalDeInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
            delete maPrinterData.mpGraphics;
        }

#ifdef WIN
        DeleteDC( hDC );
#endif
    }

    // remove printer from printerlist
    if ( this == pSalData->mpFirstPrinter )
        pSalData->mpFirstPrinter = maPrinterData.mpNextPrinter;
    else
    {
        SalPrinter* pTempPrinter = pSalData->mpFirstPrinter;

        while( pTempPrinter->maPrinterData.mpNextPrinter != this )
            pTempPrinter = pTempPrinter->maPrinterData.mpNextPrinter;

        pTempPrinter->maPrinterData.mpNextPrinter = maPrinterData.mpNextPrinter;
    }
}

// -----------------------------------------------------------------------

BOOL SalPrinter::StartJob( const XubString* pFileName,
                           const XubString& rJobName,
                           const XubString&,
                           ULONG nCopies, BOOL bCollate,
                           ImplJobSetup* pSetupData )
{
#ifdef WIN
    maPrinterData.mnError       = 0;
    maPrinterData.mbAbort       = FALSE;
    maPrinterData.mnCopies      = nCopies;
    maPrinterData.mbCollate     = bCollate;

    LPDEVMODE   pOrgDevMode = NULL;
    LPDEVMODE   pDevMode;
    BOOL        bOwnDevMode = FALSE;
    if ( pSetupData && pSetupData->mpDriverData )
    {
        pOrgDevMode = SAL_DEVMODE( pSetupData );
        pDevMode = ImplSalSetCopies( pOrgDevMode, nCopies, bCollate );
    }
    else
        pDevMode = NULL;

// !!! UNICODE - NT Optimierung !!!
    VCLVIEW hDC = CreateDCA( ImplSalGetWinAnsiString( maPrinterData.mpInfoPrinter->maPrinterData.maDriverName, TRUE ).GetBuffer(),
                         ImplSalGetWinAnsiString( maPrinterData.mpInfoPrinter->maPrinterData.maDeviceName, TRUE ).GetBuffer(),
                         0,
                         (LPDEVMODEA)pDevMode );

    if ( pDevMode != pOrgDevMode )
        delete pDevMode;

    if ( !hDC )
    {
        maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    if ( SetAbortProc( hDC, SalPrintAbortProc ) <= 0 )
    {
        maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    maPrinterData.mnError   = 0;
    maPrinterData.mbAbort   = FALSE;

// !!! UNICODE - NT Optimierung !!!
    // Both strings must be exist, if StartJob() is called
    ByteString aJobName( ImplSalGetWinAnsiString( rJobName, TRUE ) );
    ByteString aFileName;

    DOCINFO aInfo;
    memset( &aInfo, 0, sizeof( DOCINFO ) );
    aInfo.cbSize = sizeof( aInfo );
    aInfo.lpszDocName = (LPCSTR)aJobName.GetBuffer();
    if ( pFileName )
    {
        if ( pFileName->Len() )
        {
            aFileName = ImplSalGetWinAnsiString( *pFileName, TRUE );
            aInfo.lpszOutput = (LPCSTR)aFileName.GetBuffer();
        }
        else
            aInfo.lpszOutput = "FILE:";
    }
    else
        aInfo.lpszOutput = NULL;

    // Wegen Telocom Balloon Fax-Treiber, der uns unsere Messages
    // ansonsten oefters schickt, versuchen wir vorher alle
    // zu verarbeiten und dann eine Dummy-Message reinstellen
    BOOL bWhile = TRUE;
    int  i = 0;
    do
    {
        // Messages verarbeiten
        MSG aMsg;
        if ( ImplPeekMessage( &aMsg, 0, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &aMsg );
            ImplDispatchMessage( &aMsg );
            i++;
            if ( i > 15 )
                bWhile = FALSE;
        }
        else
            bWhile = FALSE;
    }
    while ( bWhile );
    ImplPostMessage( GetSalData()->mpFirstInstance->maInstData.mhComWnd, SAL_MSG_DUMMY, 0, 0 );

    // Job starten
    int nRet = ::StartDoc( hDC, &aInfo );
    if ( nRet <= 0 )
    {
        if ( (nRet == SP_USERABORT) || (nRet == SP_APPABORT) || (GetLastError() == ERROR_PRINT_CANCELLED) )
            maPrinterData.mnError = SAL_PRINTER_ERROR_ABORT;
        else
            maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    maPrinterData.mhDC = hDC;
    return TRUE;
#else
    return FALSE;
#endif
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
    VCLVIEW hDC = maPrinterData.mhDC;
    if ( hDC )
    {
        if ( maPrinterData.mpGraphics )
        {
            ImplSalDeInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
            delete maPrinterData.mpGraphics;
            maPrinterData.mpGraphics = NULL;
        }

#ifdef WIN
        ::EndDoc( hDC );
        DeleteDC( hDC );
#endif
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::AbortJob()
{
    maPrinterData.mbAbort = TRUE;

    // Abort asyncron ausloesen
    VCLVIEW hDC = maPrinterData.mhDC;
    if ( hDC )
    {
        SalData* pSalData = GetSalData();
#ifdef WIN
        ImplPostMessage( pSalData->mpFirstInstance->maInstData.mhComWnd,
                         SAL_MSG_PRINTABORTJOB, (WPARAM)hDC, 0 );
#endif
    }

    return TRUE;
}

// -----------------------------------------------------------------------

void ImplSalPrinterAbortJobAsync( VCLVIEW hPrnDC )
{
    SalData*    pSalData = GetSalData();
    SalPrinter* pPrinter = pSalData->mpFirstPrinter;

    // Feststellen, ob Printer noch existiert
    while ( pPrinter )
    {
        if ( pPrinter->maPrinterData.mhDC == hPrnDC )
            break;

        pPrinter = pPrinter->maPrinterData.mpNextPrinter;
    }

    // Wenn Printer noch existiert, dann den Job abbrechen
    if ( pPrinter )
    {
        VCLVIEW hDC = pPrinter->maPrinterData.mhDC;
        if ( hDC )
        {
            if ( pPrinter->maPrinterData.mpGraphics )
            {
                ImplSalDeInitGraphics( &(pPrinter->maPrinterData.mpGraphics->maGraphicsData) );
                delete pPrinter->maPrinterData.mpGraphics;
                pPrinter->maPrinterData.mpGraphics = NULL;
            }

#ifdef WIN
            ::AbortDoc( hDC );
            DeleteDC( hDC );
#endif
        }
    }
}

// -----------------------------------------------------------------------

SalGraphics* SalPrinter::StartPage( ImplJobSetup* pSetupData, BOOL bNewJobData )
{
    VCLVIEW hDC = maPrinterData.mhDC;
#ifdef WIN
    if ( pSetupData && pSetupData->mpDriverData && bNewJobData )
    {
        LPDEVMODE   pOrgDevMode;
        LPDEVMODE   pDevMode;
        pOrgDevMode = SAL_DEVMODE( pSetupData );
        pDevMode = ImplSalSetCopies( pOrgDevMode, maPrinterData.mnCopies, maPrinterData.mbCollate );
        ResetDC( hDC, pDevMode );
        if ( pDevMode != pOrgDevMode )
            delete pDevMode;
    }
    int nRet = ::StartPage( hDC );
    if ( nRet <= 0 )
    {
        maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return NULL;
    }

    // Hack, damit alte PS-Treiber Leerseiten nicht wegoptimieren
    HPEN    hTempPen = SelectPen( hDC, GetStockPen( NULL_PEN ) );
    HBRUSH  hTempBrush = SelectBrush( hDC, GetStockBrush( NULL_BRUSH ) );
    WIN_Rectangle( hDC, -8000, -8000, -7999, -7999 );
    SelectPen( hDC, hTempPen );
    SelectBrush( hDC, hTempBrush );
#endif

    SalGraphics* pGraphics = new SalGraphics;
    pGraphics->maGraphicsData.mhDC      = hDC;
    pGraphics->maGraphicsData.mhWnd     = 0;
    pGraphics->maGraphicsData.mbPrinter = TRUE;
    pGraphics->maGraphicsData.mbVirDev  = FALSE;
    pGraphics->maGraphicsData.mbWindow  = FALSE;
    pGraphics->maGraphicsData.mbScreen  = FALSE;
    ImplSalInitGraphics( &(pGraphics->maGraphicsData) );
    maPrinterData.mpGraphics = pGraphics;
    return pGraphics;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndPage()
{
    VCLVIEW hDC = maPrinterData.mhDC;
    if ( hDC && maPrinterData.mpGraphics )
    {
        ImplSalDeInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
        delete maPrinterData.mpGraphics;
        maPrinterData.mpGraphics = NULL;
    }
#ifdef WIN
    int nRet = ::EndPage( hDC );
    if ( nRet > 0 )
        return TRUE;
    else
    {
        maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }
#else
    return FALSE;
#endif
}

// -----------------------------------------------------------------------

ULONG SalPrinter::GetErrorCode()
{
    return maPrinterData.mnError;
}
