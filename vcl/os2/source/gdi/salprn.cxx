/*************************************************************************
 *
 *  $RCSfile: salprn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:34 $
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

// use this define to disable the DJP support
// #define NO_DJP

#define INCL_DOSMODULEMGR
#define INCL_DEV
#define INCL_SPL
#define INCL_SPLDOSPRINT
#define INCL_DEVDJP

#define BOOL     PM_BOOL
#define BYTE     PM_BYTE
#define USHORT   PM_USHORT
#define ULONG    PM_ULONG

#define INCL_PM
#include <os2.h>
#include "pmdjp.h"

#undef BOOL
#undef BYTE
#undef USHORT
#undef ULONG

#include <string.h>

#define _SV_SALPRN_CXX

#ifndef _NEW_HXX
#include <tools/new.hxx>
#endif
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
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

#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif

// =======================================================================

// -----------------------
// - struct ImplFormInfo -
// -----------------------

struct ImplFormInfo
{
    long                    mnPaperWidth;
    long                    mnPaperHeight;
    DJPT_PAPERSIZE          mnId;
};

// =======================================================================

// -----------------------
// - struct ImplTrayInfo -
// -----------------------

struct ImplTrayInfo
{
    XubString               maName;
    XubString               maDisplayName;
    DJPT_TRAYTYPE           mnId;

                            ImplTrayInfo( const char* pTrayName,
                                          const char* pTrayDisplayName ) :
                                maName( pTrayName ),
                                maDisplayName( pTrayDisplayName )
                            {}
};

// =======================================================================

struct ImplQueueSalSysData
{
    String          maPrinterName;          // pszPrinters
    String          maName;                 // pszName bzw. LogAddress
    String          maOrgDriverName;        // pszDriverName (maDriverName.maDeviceName)
    String          maDriverName;           // pszDriverName bis .
    String          maDeviceName;           // pszDriverName nach .
    PDRIVDATA       mpDrivData;

                    ImplQueueSalSysData( const String& rPrinterName,
                                         const String& rName,
                                         const String& rDriverName,
                                         const String& rDeviceName,
                                         const String& rOrgDriverName,
                                         PDRIVDATA pDrivData  );
                    ~ImplQueueSalSysData();
};

// -----------------------------------------------------------------------

ImplQueueSalSysData::ImplQueueSalSysData( const String& rPrinterName,
                                          const String& rName,
                                          const String& rOrgDriverName,
                                          const String& rDriverName,
                                          const String& rDeviceName,
                                          PDRIVDATA pDrivData ) :
    maPrinterName( rPrinterName ),
    maName( rName ),
    maOrgDriverName( rName ),
    maDriverName( rDriverName ),
    maDeviceName( rDeviceName )
{
    if ( pDrivData )
    {
        mpDrivData = (PDRIVDATA)new BYTE[pDrivData->cb];
        memcpy( mpDrivData, pDrivData, pDrivData->cb );
    }
    else
        mpDrivData = NULL;
}

// -----------------------------------------------------------------------

ImplQueueSalSysData::~ImplQueueSalSysData()
{
    delete mpDrivData;
}

// =======================================================================

static ULONG ImplPMQueueStatusToSal( PM_USHORT nPMStatus )
{
    ULONG nStatus = 0;
    if ( nPMStatus & PRQ3_PAUSED )
        nStatus |= QUEUE_STATUS_PAUSED;
    if ( nPMStatus & PRQ3_PENDING )
        nStatus |= QUEUE_STATUS_PENDING_DELETION;
    if ( !nStatus )
        nStatus |= QUEUE_STATUS_READY;
    return nStatus;
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    APIRET rc;
    ULONG  nNeeded;
    ULONG  nReturned;
    ULONG  nTotal;

    // query needed size of the buffer for the QueueInfo
    rc = SplEnumQueue( (PSZ)NULL, 3, NULL, 0, &nReturned, &nTotal, &nNeeded, NULL );
    if( nNeeded == 0 )
        return;

    // create the buffer for the QueueInfo
    PCHAR pQueueData = new CHAR[nNeeded];

    // query QueueInfos
    rc = SplEnumQueue( (PSZ)NULL, 3, pQueueData, nNeeded, &nReturned, &nTotal, &nNeeded, NULL );

    PPRQINFO3 pPrqInfo = (PPRQINFO3)pQueueData;
    for ( int i = 0; i < nReturned; i++ )
    {
        // create entry for the QueueInfo array
        SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;

        String aOrgDriverName( pPrqInfo->pszDriverName );
        String aName( pPrqInfo->pszName );
        pInfo->maDriver      = aOrgDriverName;
        pInfo->maPrinterName = pPrqInfo->pszComment;
        pInfo->maLocation    = aName;
        pInfo->mnStatus      = ImplPMQueueStatusToSal( pPrqInfo->fsStatus );
        pInfo->mnJobs        = pPrqInfo->cJobs;
        // pInfo->maComment = !!!

        // Feststellen, ob Name doppelt
        PPRQINFO3 pTempPrqInfo = (PPRQINFO3)pQueueData;
        for ( int j = 0; j < nReturned; j++ )
        {
            // Wenn Name doppelt, erweitern wir diesen um die Location
            if ( (j != i) &&
                 (strcmp( pPrqInfo->pszComment, pTempPrqInfo->pszComment ) == 0) )
            {
                pInfo->maPrinterName += ';';
                pInfo->maPrinterName += pInfo->maLocation;
            }
            pTempPrqInfo++;
        }

        // pszDriver in DriverName (bis .) und DeviceName (nach .) aufsplitten
        PSZ pDriverName;
        PSZ pDeviceName;
        if ( (pDriverName = strchr( pPrqInfo->pszDriverName, '.' )) != 0 )
        {
           *pDriverName = 0;
           pDeviceName  = pDriverName + 1;
        }
        else
            pDeviceName = NULL;

        // Alle Bytes hinter dem DeviceNamen auf 0 initialisieren, damit
        // ein memcmp vom JobSetup auch funktioniert
        if ( pPrqInfo->pDriverData &&
             (pPrqInfo->pDriverData->cb >= sizeof( pPrqInfo->pDriverData )) )
        {
            int nDeviceNameLen = strlen( pPrqInfo->pDriverData->szDeviceName );
            memset( pPrqInfo->pDriverData->szDeviceName+nDeviceNameLen,
                    0,
                    sizeof( pPrqInfo->pDriverData->szDeviceName )-nDeviceNameLen );
        }

        // save driver data and driver names
        String aPrinterName( pPrqInfo->pszPrinters );
        String aDriverName( pPrqInfo->pszDriverName );
        String aDeviceName;
        if ( pDeviceName )
            aDeviceName = pDeviceName;
        pInfo->mpSysData = new ImplQueueSalSysData( aPrinterName, aName,
                                                    aOrgDriverName,
                                                    aDriverName, aDeviceName,
                                                    pPrqInfo->pDriverData );

        // add queue to the list
        pList->Add( pInfo );

        // increment to next element of the QueueInfo array
        pPrqInfo++;
    }

    delete [] pQueueData;
}

// -----------------------------------------------------------------------

void SalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
    APIRET rc;
    ULONG  nNeeded;
    ULONG  nReturned;
    ULONG  nTotal;

    // query needed size of the buffer for the QueueInfo
    rc = SplEnumQueue( (PSZ)NULL, 3, NULL, 0, &nReturned, &nTotal, &nNeeded, NULL );
    if( nNeeded == 0 )
        return;

    // create the buffer for the QueueInfo
    PCHAR pQueueData = new CHAR[nNeeded];

    // query QueueInfos
    rc = SplEnumQueue( (PSZ)NULL, 3, pQueueData, nNeeded, &nReturned, &nTotal, &nNeeded, NULL );

    PPRQINFO3 pPrqInfo = (PPRQINFO3)pQueueData;
    for ( int i = 0; i < nReturned; i++ )
    {
        ImplQueueSalSysData* pSysData = (ImplQueueSalSysData*)(pInfo->mpSysData);
        if ( (strcmp( pSysData->maPrinterName, pPrqInfo->pszPrinters ) == 0) &&
             (strcmp( pSysData->maName, pPrqInfo->pszName ) == 0) &&
             (strcmp( pSysData->maOrgDriverName, pPrqInfo->pszDriverName ) == 0) )
        {
            pInfo->mnStatus = ImplPMQueueStatusToSal( pPrqInfo->fsStatus );
            pInfo->mnJobs   = pPrqInfo->cJobs;
            break;
        }

        // increment to next element of the QueueInfo array
        pPrqInfo++;
    }

    delete [] pQueueData;
}

// -----------------------------------------------------------------------

void SalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete ((ImplQueueSalSysData*)(pInfo->mpSysData));
    delete pInfo;
}

// -----------------------------------------------------------------------

XubString SalInstance::GetDefaultPrinter()
{
    APIRET      rc;
    ULONG       nNeeded;
    ULONG       nReturned;
    ULONG       nTotal;
    char        szQueueName[255];
    XubString   aDefaultName;

    // query default queue
    if ( !PrfQueryProfileString( HINI_PROFILE, SPL_INI_SPOOLER, "QUEUE", 0, szQueueName, sizeof( szQueueName ) ) )
        return aDefaultName;

    // extract first queue name
    PSZ pStr;
    if ( (pStr = strchr( szQueueName, ';' )) != 0 )
        *pStr = 0;

    // query needed size of the buffer for the QueueInfo
    rc = SplEnumQueue( (PSZ)NULL, 3, NULL, 0, &nReturned, &nTotal, &nNeeded, NULL );
    if ( nNeeded == 0 )
        return aDefaultName;

    // create the buffer for the QueueInfo
    PCHAR pQueueData = new CHAR[ nNeeded ];

    // query QueueInfos
    rc = SplEnumQueue ((PSZ)NULL, 3, pQueueData, nNeeded, &nReturned, &nTotal, &nNeeded, NULL );

    // find printer name for default queue
    PPRQINFO3 pPrqInfo = (PPRQINFO3) pQueueData;
    for ( int i = 0; i < nReturned; i++ )
    {
        if ( strcmp( pPrqInfo->pszName, szQueueName ) == 0 )
        {
            aDefaultName = pPrqInfo->pszComment;

            // Feststellen, ob Name doppelt
            PPRQINFO3 pTempPrqInfo = (PPRQINFO3)pQueueData;
            for ( int j = 0; j < nReturned; j++ )
            {
                // Wenn Name doppelt, erweitern wir diesen um die Location
                if ( (j != i) &&
                     (strcmp( pPrqInfo->pszComment, pTempPrqInfo->pszComment ) == 0) )
                {
                    aDefaultName += ';';
                    aDefaultName += pPrqInfo->pszName;
                }
                pTempPrqInfo++;
            }
            break;
        }

        // increment to next element of the QueueInfo array
        pPrqInfo++;
    }

    delete [] pQueueData;

    return aDefaultName;
}

// =======================================================================

static void* ImplAllocPrnMemory( size_t n )
{
    PVOID pVoid = 0;

    if ( DosAllocMem( &pVoid, n, PAG_COMMIT | PAG_READ | PAG_WRITE ) )
        return 0;

    return pVoid;
}

// -----------------------------------------------------------------------

inline void ImplFreePrnMemory( void* p )
{
    DosFreeMem( p );
}

// -----------------------------------------------------------------------

static PDRIVDATA ImplPrnDrivData( const ImplJobSetup* pSetupData )
{
    // Diese Funktion wird eingesetzt, damit Druckertreiber nicht auf
    // unseren Daten arbeiten, da es durch Konfigurationsprobleme
    // sein kann, das der Druckertreiber bei uns Daten ueberschreibt.
    // Durch diese vorgehensweise werden einige Abstuerze vermieden, bzw.
    // sind dadurch leichter zu finden

    if ( !pSetupData->mpDriverData )
        return NULL;

    DBG_ASSERT( ((PRIVDATA)(pSetupData->mpDriverData))->cb == pSetupData->mnDriverDataLen,
                "ImplPrnDrivData() - SetupDataLen != DriverDataLen" );

    PDRIVDATA pDrivData = (PDRIVDATA)ImplAllocPrnMemory( pSetupData->mnDriverDataLen );
    memcpy( pDrivData, pSetupData->mpDriverData, pSetupData->mnDriverDataLen );
    return pDrivData;
}

// -----------------------------------------------------------------------

static void ImplUpdateSetupData( const PDRIVDATA pDrivData, ImplJobSetup* pSetupData )
{
    // Diese Funktion wird eingesetzt, damit Druckertreiber nicht auf
    // unseren Daten arbeiten, da es durch Konfigurationsprobleme
    // sein kann, das der Druckertreiber bei uns Daten ueberschreibt.
    // Durch diese vorgehensweise werden einige Abstuerze vermieden, bzw.
    // sind dadurch leichter zu finden

    if ( !pDrivData || !pDrivData->cb )
    {
        if ( pSetupData->mpDriverData )
            delete pSetupData->mpDriverData;
        pSetupData->mnDriverDataLen = 0;
    }
    else
    {
        // Alle Bytes hinter dem DeviceNamen auf 0 initialisieren, damit
        // ein memcmp vom JobSetup auch funktioniert
        if ( pDrivData->cb >= sizeof( pDrivData ) )
        {
            int nDeviceNameLen = strlen( pDrivData->szDeviceName );
            memset( pDrivData->szDeviceName+nDeviceNameLen,
                    0,
                    sizeof( pDrivData->szDeviceName )-nDeviceNameLen );
        }

        if ( pSetupData->mpDriverData )
        {
            if ( pSetupData->mnDriverDataLen != pDrivData->cb )
                delete pSetupData->mpDriverData;
            pSetupData->mpDriverData = new BYTE[pDrivData->cb];
        }
        else
            pSetupData->mpDriverData = new BYTE[pDrivData->cb];
        pSetupData->mnDriverDataLen = pDrivData->cb;
        memcpy( pSetupData->mpDriverData, pDrivData, pDrivData->cb );
    }

    if ( pDrivData )
        ImplFreePrnMemory( pDrivData );
}

// -----------------------------------------------------------------------

static BOOL ImplPaperSizeEqual( long nPaperWidth1, long nPaperHeight1,
                                long nPaperWidth2, long nPaperHeight2 )
{
    return (((nPaperWidth1 >= nPaperWidth2-1) && (nPaperWidth1 <= nPaperWidth2+1)) &&
            ((nPaperHeight1 >= nPaperHeight2-1) && (nPaperHeight1 <= nPaperHeight2+1)));
}

// -----------------------------------------------------------------------

static BOOL ImplIsDriverDJPEnabled( HDC hDC )
{
#ifdef NO_DJP
    return FALSE;
#else
    // Ueber OS2-Ini kann DJP disablte werden
    if ( !PrfQueryProfileInt( HINI_PROFILE, SAL_PROFILE_APPNAME, SAL_PROFILE_USEDJP, 1 ) )
        return FALSE;

    // Testen, ob DJP-Interface am Drucker vorhanden
    LONG   lQuery;
    APIRET rc;

    lQuery = DEVESC_QUERYSIZE;
    rc = DevEscape( hDC,
                    DEVESC_QUERYESCSUPPORT,
                    sizeof( lQuery ),
                    (PBYTE)&lQuery,
                    0,
                    (PBYTE)NULL );
    if ( DEV_OK != rc )
        return FALSE;

    lQuery = DEVESC_QUERYJOBPROPERTIES;
    rc = DevEscape( hDC,
                    DEVESC_QUERYESCSUPPORT,
                    sizeof( lQuery ),
                    (PBYTE)&lQuery,
                    0,
                    (PBYTE)NULL );
    if ( DEV_OK != rc )
        return FALSE;

    lQuery = DEVESC_SETJOBPROPERTIES;
    rc = DevEscape( hDC,
                    DEVESC_QUERYESCSUPPORT,
                    sizeof( lQuery ),
                    (PBYTE)&lQuery,
                    0,
                    (PBYTE)NULL );
    if ( DEV_OK != rc )
        return FALSE;

    return TRUE;
#endif
}

// -----------------------------------------------------------------------

static void ImplFormatInputList( PDJP_ITEM pDJP, PQUERYTUPLE pTuple )
{
   // Loop through the query elements
   BOOL fContinue = TRUE;
   do
   {
      pDJP->cb            = sizeof (DJP_ITEM);
      pDJP->ulProperty    = pTuple->ulProperty;
      pDJP->lType         = pTuple->lType;
      pDJP->ulNumReturned = 0;
      pDJP->ulValue       = DJP_NONE;

      // at EOL?
      fContinue = DJP_NONE != pTuple->ulProperty;

      // Move to next item structure and tuplet
      pDJP++;
      pTuple++;
   }
   while ( fContinue );
}

// -----------------------------------------------------------------------

static void ImplFreeFormAndTrayList( SalInfoPrinter* pSalInfoPrinter )
{
    if ( pSalInfoPrinter->maPrinterData.mnFormCount )
    {
        for ( USHORT i = 0; i < pSalInfoPrinter->maPrinterData.mnFormCount; i++ )
            delete pSalInfoPrinter->maPrinterData.mpFormArray[i];
        delete [] pSalInfoPrinter->maPrinterData.mpFormArray;
        pSalInfoPrinter->maPrinterData.mnFormCount = 0;
    }

    if ( pSalInfoPrinter->maPrinterData.mnTrayCount )
    {
        for ( USHORT i = 0; i < pSalInfoPrinter->maPrinterData.mnTrayCount; i++ )
            delete pSalInfoPrinter->maPrinterData.mpTrayArray[i];
        delete [] pSalInfoPrinter->maPrinterData.mpTrayArray;
        pSalInfoPrinter->maPrinterData.mnTrayCount = 0;
    }
}

// -----------------------------------------------------------------------

static void ImplGetFormAndTrayList( SalInfoPrinter* pSalInfoPrinter, const ImplJobSetup* pSetupData )
{
    ImplFreeFormAndTrayList( pSalInfoPrinter );

    LONG alQuery[] =
    {
        0,                  0,              // First two members of QUERYSIZE
        DJP_CJ_FORM,        DJP_ALL,
        DJP_CJ_TRAYNAME,    DJP_ALL,
        DJP_NONE,           DJP_NONE        // EOL marker
    };

    APIRET      rc;
    PQUERYSIZE  pQuerySize          = (PQUERYSIZE)alQuery;
    PBYTE       pBuffer             = NULL;
    LONG        nAlloc              = 0;
    PDRIVDATA   pCopyDrivData       = ImplPrnDrivData( pSetupData );
    LONG        nDrivDataSize       = pCopyDrivData->cb;
    PBYTE       pDrivData           = (PBYTE)pCopyDrivData;

    // find out how many bytes to allocate
    pQuerySize->cb = sizeof( alQuery );
    rc = DevEscape( pSalInfoPrinter->maPrinterData.mhDC,
                    DEVESC_QUERYSIZE,
                    sizeof( alQuery ),
                    (PBYTE)pQuerySize,
                    &nDrivDataSize,
                    pDrivData );
    if ( DEV_OK != rc )
    {
        ImplFreePrnMemory( pCopyDrivData );
        return;
    }

    // allocate the memory
    nAlloc = pQuerySize->ulSizeNeeded;
    pBuffer = (PBYTE)new BYTE[nAlloc];

    // set up the input
    PDJP_ITEM pDJP = (PDJP_ITEM)pBuffer;
    ImplFormatInputList( pDJP, pQuerySize->aTuples );

    // do it!
    rc = DevEscape( pSalInfoPrinter->maPrinterData.mhDC,
                    DEVESC_QUERYJOBPROPERTIES,
                    nAlloc,
                    pBuffer,
                    &nDrivDataSize,
                    pDrivData );
    ImplFreePrnMemory( pCopyDrivData );

    if ( (DEV_OK == rc) || (DEV_WARNING == rc) )
    {
        // Loop through the query elements
        PQUERYTUPLE pTuple = pQuerySize->aTuples;
        while ( DJP_NONE != pTuple->ulProperty )
        {
            if ( pDJP->ulProperty == DJP_CJ_FORM )
            {
                if ( pDJP->ulNumReturned )
                {
                    PDJPT_FORM pElm = DJP_ELEMENTP( *pDJP, DJPT_FORM );

                    pSalInfoPrinter->maPrinterData.mnFormCount = pDJP->ulNumReturned;
                    pSalInfoPrinter->maPrinterData.mpFormArray = new PIMPLFORMINFO[pSalInfoPrinter->maPrinterData.mnFormCount];
                    for( int i = 0; i < pDJP->ulNumReturned; i++, pElm++ )
                    {
                        ImplFormInfo* pInfo     = new ImplFormInfo;
                        pInfo->mnPaperWidth     = pElm->hcInfo.cx;
                        pInfo->mnPaperHeight    = pElm->hcInfo.cy;
                        pInfo->mnId             = pElm->djppsFormID;
                        pSalInfoPrinter->maPrinterData.mpFormArray[i] = pInfo;
                    }
                }
            }
            else if ( pDJP->ulProperty == DJP_CJ_TRAYNAME )
            {
                if ( pDJP->ulNumReturned )
                {
                    PDJPT_TRAYNAME pElm = DJP_ELEMENTP( *pDJP, DJPT_TRAYNAME );

                    pSalInfoPrinter->maPrinterData.mnTrayCount = pDJP->ulNumReturned;
                    pSalInfoPrinter->maPrinterData.mpTrayArray = new PIMPLTRAYINFO[pSalInfoPrinter->maPrinterData.mnTrayCount];
                    for( int i = 0; i < pDJP->ulNumReturned; i++, pElm++ )
                    {
                        ImplTrayInfo* pInfo     = new ImplTrayInfo( pElm->szTrayname, pElm->szDisplayTrayname );
                        pInfo->mnId             = pElm->djpttTrayID;
                        pSalInfoPrinter->maPrinterData.mpTrayArray[i] = pInfo;
                    }
                }
            }

            pDJP = DJP_NEXT_STRUCTP( pDJP );
            pTuple++;
        }
    }

    delete [] pBuffer;
}

// -----------------------------------------------------------------------

static BOOL ImplGetCurrentSettings( SalInfoPrinter* pSalInfoPrinter, ImplJobSetup* pSetupData )
{
    // Um den aktuellen Tray zu ermitteln, brauchen wir auch die Listen dazu
    if ( !pSalInfoPrinter->maPrinterData.mnFormCount )
        ImplGetFormAndTrayList( pSalInfoPrinter, pSetupData );

    LONG alQuery[] =
    {
        0,                      0,              // First two members of QUERYSIZE
        DJP_SJ_ORIENTATION,     DJP_CURRENT,
        DJP_CJ_FORM,            DJP_CURRENT,
        DJP_NONE,               DJP_NONE        // EOL marker
    };

    APIRET      rc;
    PQUERYSIZE  pQuerySize          = (PQUERYSIZE)alQuery;
    PBYTE       pBuffer             = NULL;
    LONG        nAlloc              = 0;
    PDRIVDATA   pCopyDrivData       = ImplPrnDrivData( pSetupData );
    LONG        nDrivDataSize       = pCopyDrivData->cb;
    PBYTE       pDrivData           = (PBYTE)pCopyDrivData;
    BOOL        bResult;

    // find out how many bytes to allocate
    pQuerySize->cb = sizeof( alQuery );
    rc = DevEscape( pSalInfoPrinter->maPrinterData.mhDC,
                    DEVESC_QUERYSIZE,
                    sizeof( alQuery ),
                    (PBYTE)pQuerySize,
                    &nDrivDataSize,
                    pDrivData );
    if ( DEV_OK != rc )
    {
        ImplFreePrnMemory( pCopyDrivData );
        return FALSE;
    }

    // allocate the memory
    nAlloc = pQuerySize->ulSizeNeeded;
    pBuffer = (PBYTE)new BYTE[nAlloc];

    // set up the input
    PDJP_ITEM pDJP = (PDJP_ITEM)pBuffer;
    ImplFormatInputList( pDJP, pQuerySize->aTuples );

    rc = DevEscape( pSalInfoPrinter->maPrinterData.mhDC,
                    DEVESC_QUERYJOBPROPERTIES,
                    nAlloc,
                    pBuffer,
                    &nDrivDataSize,
                    pDrivData );
    if ( (DEV_OK == rc) || (DEV_WARNING == rc) )
    {
        // aktuelle Setup-Daten uebernehmen
        ImplUpdateSetupData( pCopyDrivData, pSetupData );

        // Loop through the query elements
        PQUERYTUPLE pTuple = pQuerySize->aTuples;
        while ( DJP_NONE != pTuple->ulProperty )
        {
            if ( pDJP->ulProperty == DJP_SJ_ORIENTATION )
            {
                if ( pDJP->ulNumReturned )
                {
                    PDJPT_ORIENTATION pElm = DJP_ELEMENTP( *pDJP, DJPT_ORIENTATION );
                    if ( (DJP_ORI_PORTRAIT == *pElm) || (DJP_ORI_REV_PORTRAIT == *pElm) )
                        pSetupData->meOrientation = ORIENTATION_PORTRAIT;
                    else
                        pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
                }
            }
            else if ( pDJP->ulProperty == DJP_CJ_FORM )
            {
                if ( pDJP->ulNumReturned )
                {
                    PDJPT_FORM pElm = DJP_ELEMENTP( *pDJP, DJPT_FORM );

                    pSetupData->mnPaperWidth  = pElm->hcInfo.cx*100;
                    pSetupData->mnPaperHeight = pElm->hcInfo.cy*100;
                    switch( pElm->djppsFormID )
                    {
                        case DJP_PSI_A3:
                            pSetupData->mePaperFormat = PAPER_A3;
                            break;

                        case DJP_PSI_A4:
                            pSetupData->mePaperFormat = PAPER_A4;
                            break;

                        case DJP_PSI_A5:
                            pSetupData->mePaperFormat = PAPER_A5;
                            break;

                        case DJP_PSI_B4:
                            pSetupData->mePaperFormat = PAPER_B4;
                            break;

                        case DJP_PSI_B5:
                            pSetupData->mePaperFormat = PAPER_B5;
                            break;

                        case DJP_PSI_LETTER:
                            pSetupData->mePaperFormat = PAPER_LETTER;
                            break;

                        case DJP_PSI_LEGAL:
                            pSetupData->mePaperFormat = PAPER_LEGAL;
                            break;

                        case DJP_PSI_TABLOID:
                            pSetupData->mePaperFormat = PAPER_TABLOID;
                            break;

                        default:
                            pSetupData->mePaperFormat = PAPER_USER;
                            break;
                    }

                    // Wir suchen zuerst ueber den Namen/Id und dann ueber die Id
                    BOOL    bTrayFound = FALSE;
                    USHORT  j;
                    for ( j = 0; j < pSalInfoPrinter->maPrinterData.mnTrayCount; j++ )
                    {
                        if ( (pSalInfoPrinter->maPrinterData.mpTrayArray[j]->mnId == pElm->djpttTrayID) &&
                             (pSalInfoPrinter->maPrinterData.mpTrayArray[j]->maName == pElm->szTrayname) )
                        {
                            pSetupData->mnPaperBin = j;
                            bTrayFound = TRUE;
                            break;
                        }
                    }
                    if ( !bTrayFound )
                    {
                        for ( j = 0; j < pSalInfoPrinter->maPrinterData.mnTrayCount; j++ )
                        {
                            if ( pSalInfoPrinter->maPrinterData.mpTrayArray[j]->mnId == pElm->djpttTrayID )
                            {
                                pSetupData->mnPaperBin = j;
                                bTrayFound = TRUE;
                                break;
                            }
                        }
                    }
                    // Wenn wir Ihn immer noch nicht gefunden haben, setzen
                    // wir ihn auf DontKnow
                    if ( !bTrayFound )
                        pSetupData->mnPaperBin = 0xFFFF;
                }
            }

            pDJP = DJP_NEXT_STRUCTP( pDJP );
            pTuple++;
        }

        bResult = TRUE;
    }
    else
    {
        ImplFreePrnMemory( pCopyDrivData );
        bResult = FALSE;
    }

    delete [] pBuffer;

    return bResult;
}

// -----------------------------------------------------------------------

static BOOL ImplSetOrientation( HDC hPrinterDC, PDRIVDATA pDriverData,
                                Orientation eOrientation )
{
    LONG alQuery[] =
    {
        0,                      0,              // First two members of QUERYSIZE
        DJP_SJ_ORIENTATION,     DJP_CURRENT,
        DJP_NONE,               DJP_NONE        // EOL marker
    };

    APIRET      rc;
    PQUERYSIZE  pQuerySize      = (PQUERYSIZE)alQuery;
    PBYTE       pBuffer         = NULL;
    LONG        nAlloc          = 0;
    LONG        nDrivDataSize   = pDriverData->cb;

    // find out how many bytes to allocate
    pQuerySize->cb = sizeof( alQuery );
    rc = DevEscape( hPrinterDC,
                    DEVESC_QUERYSIZE,
                    sizeof( alQuery ),
                    (PBYTE)pQuerySize,
                    &nDrivDataSize,
                    (PBYTE)pDriverData );
    if ( DEV_OK != rc )
        return FALSE;

    // allocate the memory
    nAlloc = pQuerySize->ulSizeNeeded;
    pBuffer = (PBYTE)new BYTE[nAlloc];

    // set up the input
    PDJP_ITEM pDJP = (PDJP_ITEM)pBuffer;
    ImplFormatInputList( pDJP, pQuerySize->aTuples );

    pDJP->cb         = sizeof( DJP_ITEM );
    pDJP->ulProperty = DJP_SJ_ORIENTATION;
    pDJP->lType      = DJP_CURRENT;
    pDJP->ulValue    = (eOrientation == ORIENTATION_PORTRAIT)
                           ? DJP_ORI_PORTRAIT
                           : DJP_ORI_LANDSCAPE;

    // do it!
    rc = DevEscape( hPrinterDC,
                    DEVESC_SETJOBPROPERTIES,
                    nAlloc,
                    pBuffer,
                    &nDrivDataSize,
                    (PBYTE)pDriverData );

    delete [] pBuffer;

    return ((DEV_OK == rc) || (DEV_WARNING == rc));
}

// -----------------------------------------------------------------------

static BOOL ImplSetPaperSize( HDC hPrinterDC, PDRIVDATA pDriverData,
                              DJPT_PAPERSIZE nOS2PaperFormat )
{
    LONG alQuery[] =
    {
        0,                      0,              // First two members of QUERYSIZE
        DJP_SJ_PAPERSIZE,       DJP_CURRENT,
        DJP_NONE,               DJP_NONE        // EOL marker
    };

    APIRET      rc;
    PQUERYSIZE  pQuerySize      = (PQUERYSIZE)alQuery;
    PBYTE       pBuffer         = NULL;
    LONG        nAlloc          = 0;
    LONG        nDrivDataSize   = pDriverData->cb;

    // find out how many bytes to allocate
    pQuerySize->cb = sizeof( alQuery );
    rc = DevEscape( hPrinterDC,
                    DEVESC_QUERYSIZE,
                    sizeof( alQuery ),
                    (PBYTE)pQuerySize,
                    &nDrivDataSize,
                    (PBYTE)pDriverData );
    if ( DEV_OK != rc )
        return FALSE;

    // allocate the memory
    nAlloc = pQuerySize->ulSizeNeeded;
    pBuffer = (PBYTE)new BYTE[nAlloc];

    // set up the input
    PDJP_ITEM pDJP = (PDJP_ITEM)pBuffer;
    PDJP_ITEM pStartDJP = pDJP;
    ImplFormatInputList( pDJP, pQuerySize->aTuples );

    // Neue Daten zuweisen
    pDJP->cb         = sizeof( DJP_ITEM );
    pDJP->ulProperty = DJP_SJ_PAPERSIZE;
    pDJP->lType      = DJP_CURRENT;
    pDJP->ulValue    = nOS2PaperFormat;

    // und setzen
    rc = DevEscape( hPrinterDC,
                    DEVESC_SETJOBPROPERTIES,
                    nAlloc,
                    pBuffer,
                    &nDrivDataSize,
                    (PBYTE)pDriverData );

    delete [] pBuffer;

    return ((DEV_OK == rc) || (DEV_WARNING == rc));
}

// -----------------------------------------------------------------------

static BOOL ImplSetPaperBin( HDC hPrinterDC, PDRIVDATA pDriverData,
                             ImplTrayInfo* pTrayInfo )
{
    LONG alQuery[] =
    {
        0,                      0,              // First two members of QUERYSIZE
        DJP_SJ_TRAYTYPE,        DJP_CURRENT,
        DJP_NONE,               DJP_NONE        // EOL marker
    };

    APIRET      rc;
    PQUERYSIZE  pQuerySize      = (PQUERYSIZE)alQuery;
    PBYTE       pBuffer         = NULL;
    LONG        nAlloc          = 0;
    LONG        nDrivDataSize   = pDriverData->cb;

    // find out how many bytes to allocate
    pQuerySize->cb = sizeof( alQuery );
    rc = DevEscape( hPrinterDC,
                    DEVESC_QUERYSIZE,
                    sizeof( alQuery ),
                    (PBYTE)pQuerySize,
                    &nDrivDataSize,
                    (PBYTE)pDriverData );
    if ( DEV_OK != rc )
        return FALSE;

    // allocate the memory
    nAlloc = pQuerySize->ulSizeNeeded;
    pBuffer = (PBYTE)new BYTE[nAlloc];

    // set up the input
    PDJP_ITEM pDJP = (PDJP_ITEM)pBuffer;
    ImplFormatInputList( pDJP, pQuerySize->aTuples );

    // Neue Daten zuweisen
    pDJP->cb         = sizeof( DJP_ITEM );
    pDJP->ulProperty = DJP_SJ_TRAYTYPE;
    pDJP->lType      = DJP_CURRENT;
    pDJP->ulValue    = pTrayInfo->mnId;

    // und setzen
    rc = DevEscape( hPrinterDC,
                    DEVESC_SETJOBPROPERTIES,
                    nAlloc,
                    pBuffer,
                    &nDrivDataSize,
                    (PBYTE)pDriverData );

    delete [] pBuffer;

    return ((DEV_OK == rc) || (DEV_WARNING == rc));
}

// =======================================================================

static BOOL ImplSalCreateInfoPrn( SalInfoPrinter* pPrinter, PDRIVDATA pDriverData,
                                  HDC& rDC, HPS& rPS )
{
    SalData* pSalData = GetSalData();

    // create info context
    DEVOPENSTRUC  devOpenStruc;
    memset( &devOpenStruc, 0, sizeof( devOpenStruc ) );
    devOpenStruc.pszLogAddress      = (char*)(const char*)pPrinter->maPrinterData.maName;
    devOpenStruc.pszDriverName      = (char*)(const char*)pPrinter->maPrinterData.maDriverName;
    devOpenStruc.pdriv              = pDriverData;
    devOpenStruc.pszDataType        = "PM_Q_STD";

    HDC hDC = DevOpenDC( pSalData->mhAB, OD_INFO, "*",
                         4, (PDEVOPENDATA)&devOpenStruc, (HDC)NULL);
    if ( !hDC )
        return FALSE;

    // create presentation space
    SIZEL sizel;
    sizel.cx = 0;
    sizel.cy = 0;
    HPS hPS = GpiCreatePS( pSalData->mhAB, hDC, &sizel, GPIA_ASSOC | GPIT_MICRO | PU_PELS );
    if ( !hPS )
    {
        DevCloseDC( hDC );
        return FALSE;
    }

    rDC = hDC;
    rPS = hPS;
    return TRUE;
}

// -----------------------------------------------------------------------

static void ImplSalDestroyInfoPrn( SalInfoPrinter* pPrinter )
{
    ImplSalDeInitGraphics( &(pPrinter->maPrinterData.mpGraphics->maGraphicsData) );
    GpiAssociate( pPrinter->maPrinterData.mhPS, 0 );
    GpiDestroyPS( pPrinter->maPrinterData.mhPS );
    DevCloseDC( pPrinter->maPrinterData.mhDC );
}

// =======================================================================

SalInfoPrinter* SalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                ImplJobSetup* pSetupData )
{
    ImplQueueSalSysData* pSysQueueData = (ImplQueueSalSysData*)(pQueueInfo->mpSysData);
    SalInfoPrinter* pPrinter = new SalInfoPrinter;
    pPrinter->maPrinterData.maPrinterName   = pSysQueueData->maPrinterName;
    pPrinter->maPrinterData.maName          = pSysQueueData->maName;
    pPrinter->maPrinterData.maDriverName    = pSysQueueData->maDriverName;
    pPrinter->maPrinterData.maDeviceName    = pSysQueueData->maDeviceName;

    // Nur Setup-Daten uebernehmen, wenn Treiber und Laenge der Treiberdaten
    // uebereinstimmt
    PDRIVDATA   pDriverData;
    BOOL        bUpdateDriverData;
    if ( pSetupData->mpDriverData && pSysQueueData->mpDrivData &&
         (pSetupData->mnSystem == JOBSETUP_SYSTEM_OS2) &&
         (pSetupData->mnDriverDataLen == pSysQueueData->mpDrivData->cb) &&
         (strcmp( ((PDRIVDATA)pSetupData->mpDriverData)->szDeviceName,
                  pSysQueueData->mpDrivData->szDeviceName ) == 0) )
    {
        pDriverData = PDRIVDATA( pSetupData->mpDriverData );
        bUpdateDriverData = FALSE;
    }
    else
    {
        pDriverData = pSysQueueData->mpDrivData;
        bUpdateDriverData = TRUE;
    }
    if ( pDriverData )
        pPrinter->maPrinterData.maJobSetupDeviceName = pDriverData->szDeviceName;

    if ( !ImplSalCreateInfoPrn( pPrinter, pDriverData,
                                pPrinter->maPrinterData.mhDC,
                                pPrinter->maPrinterData.mhPS ) )
    {
        delete pPrinter;
        return NULL;
    }

    // create graphics object for output
    SalGraphics* pGraphics = new SalGraphics;
    pGraphics->maGraphicsData.mhDC              = pPrinter->maPrinterData.mhDC;
    pGraphics->maGraphicsData.mhPS              = pPrinter->maPrinterData.mhPS;
    pGraphics->maGraphicsData.mhWnd             = 0;
    pGraphics->maGraphicsData.mbPrinter         = TRUE;
    pGraphics->maGraphicsData.mbVirDev          = FALSE;
    pGraphics->maGraphicsData.mbWindow          = FALSE;
    pGraphics->maGraphicsData.mbScreen          = FALSE;

    ImplSalInitGraphics( &(pGraphics->maGraphicsData) );
    pPrinter->maPrinterData.mpGraphics          = pGraphics;

    // check printer driver for DJP support
    pPrinter->maPrinterData.mbDJPSupported = ImplIsDriverDJPEnabled( pPrinter->maPrinterData.mhDC );

    if ( bUpdateDriverData )
    {
        if ( pSetupData->mpDriverData )
            delete pSetupData->mpDriverData;
        pSetupData->mpDriverData = new BYTE[pDriverData->cb];
        memcpy( pSetupData->mpDriverData, pDriverData, pDriverData->cb );
        pSetupData->mnDriverDataLen = pDriverData->cb;
    }

    // retrieve current settings from printer driver and store them to system independend data!
    if ( pPrinter->maPrinterData.mbDJPSupported )
        ImplGetCurrentSettings( pPrinter, pSetupData );
    pSetupData->mnSystem = JOBSETUP_SYSTEM_OS2;

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
    maPrinterData.mhDC                  = 0;
    maPrinterData.mhPS                  = 0;
    maPrinterData.mpGraphics            = NULL;
    maPrinterData.mbGraphics            = FALSE;
    maPrinterData.mbDJPSupported        = FALSE;
    maPrinterData.mnFormCount           = 0;
    maPrinterData.mpFormArray           = NULL;
    maPrinterData.mnTrayCount           = 0;
    maPrinterData.mpTrayArray           = NULL;
}

// -----------------------------------------------------------------------

SalInfoPrinter::~SalInfoPrinter()
{
    if ( maPrinterData.mpGraphics )
    {
        ImplSalDestroyInfoPrn( this );
        delete maPrinterData.mpGraphics;
    }

    ImplFreeFormAndTrayList( this );
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
    PDRIVDATA pDrivData = ImplPrnDrivData( pSetupData );
    if ( !pDrivData )
        return FALSE;

    APIRET rc = DevPostDeviceModes( GetSalData()->mhAB, pDrivData,
                                    maPrinterData.maDriverName.GetStr(),
                                    maPrinterData.maDeviceName.GetStr(),
                                    maPrinterData.maPrinterName.GetStr(),
                                    DPDM_POSTJOBPROP );
    if ( rc == DEV_OK )
    {
        ImplUpdateSetupData( pDrivData, pSetupData );

        // update DC and PS
        HDC hDC;
        HPS hPS;
        if ( !ImplSalCreateInfoPrn( this, (PDRIVDATA)(pSetupData->mpDriverData), hDC, hPS ) )
            return FALSE;

        // Alten Printer DC/PS zerstoeren
        ImplSalDestroyInfoPrn( this );

        // Neue Daten setzen und initialisieren
        maPrinterData.mhDC = hDC;
        maPrinterData.mhPS = hPS;
        maPrinterData.mpGraphics->maGraphicsData.mhDC = maPrinterData.mhDC;
        maPrinterData.mpGraphics->maGraphicsData.mhPS = maPrinterData.mhPS;
        ImplSalInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );

        // retrieve current settings from printer driver and store them to system independend data!
        ImplFreeFormAndTrayList( this );
        if ( maPrinterData.mbDJPSupported )
            ImplGetCurrentSettings( this, pSetupData );

        return TRUE;
    }
    else
    {
        ImplFreePrnMemory( pDrivData );
        return FALSE;
    }
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
    // Wir koennen nur Treiberdaten von OS2 setzen
    if ( pSetupData->mnSystem != JOBSETUP_SYSTEM_OS2 )
        return FALSE;

    PDRIVDATA pNewDrivData = (PDRIVDATA)(pSetupData->mpDriverData);
    if ( !pNewDrivData )
        return FALSE;

    // Testen, ob Printerdaten fuer den gleichen Printer uebergeben werden,
    // da einige Treiber zu Abstuerzen neigen, wenn Daten von einem anderen
    // Printer gesetzt werden
    if ( strcmp( maPrinterData.maJobSetupDeviceName, pNewDrivData->szDeviceName ) != 0 )
        return FALSE;

    // update DC and PS
    HDC hDC;
    HPS hPS;
    if ( !ImplSalCreateInfoPrn( this, pNewDrivData, hDC, hPS ) )
        return FALSE;

    // Alten Printer DC/PS zerstoeren
    ImplSalDestroyInfoPrn( this );

    // Neue Daten setzen und initialisieren
    maPrinterData.mhDC = hDC;
    maPrinterData.mhPS = hPS;
    maPrinterData.mpGraphics->maGraphicsData.mhDC = maPrinterData.mhDC;
    maPrinterData.mpGraphics->maGraphicsData.mhPS = maPrinterData.mhPS;
    ImplSalInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );

    // retrieve current settings from printer driver and store them to system independend data!
    ImplFreeFormAndTrayList( this );
    if ( maPrinterData.mbDJPSupported )
        ImplGetCurrentSettings( this, pSetupData );

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalInfoPrinter::SetData( ULONG nFlags, ImplJobSetup* pSetupData )
{
    // needs DJP support
    if ( !maPrinterData.mbDJPSupported )
        return FALSE;

    PDRIVDATA pDrivData = ImplPrnDrivData( pSetupData );

    if ( !pDrivData )
        return FALSE;

    BOOL bOK = FALSE;

    // set orientation
    if ( nFlags & SAL_JOBSET_ORIENTATION )
    {
        if ( ImplSetOrientation( maPrinterData.mhDC, pDrivData, pSetupData->meOrientation ) )
            bOK = TRUE;
    }

    // set paper size
    if ( nFlags & SAL_JOBSET_PAPERSIZE )
    {
        // Papierformat ermitteln
        DJPT_PAPERSIZE nOS2PaperFormat;
        switch ( pSetupData->mePaperFormat )
        {
            case PAPER_A3:
                nOS2PaperFormat = DJP_PSI_A3;
                break;

            case PAPER_A4:
                nOS2PaperFormat = DJP_PSI_A4;
                break;

            case PAPER_A5:
                nOS2PaperFormat = DJP_PSI_A5;
                break;

            case PAPER_B4:
                nOS2PaperFormat = DJP_PSI_B4;
                break;

            case PAPER_B5:
                nOS2PaperFormat = DJP_PSI_B5;
                break;

            case PAPER_LETTER:
                nOS2PaperFormat = DJP_PSI_LETTER;
                break;

            case PAPER_LEGAL:
                nOS2PaperFormat = DJP_PSI_LEGAL;
                break;

            case PAPER_TABLOID:
                nOS2PaperFormat = DJP_PSI_TABLOID;
                break;

            default:
                {
                nOS2PaperFormat = DJP_PSI_NONE;
                // OS2 rechnet in Millimetern
                long nPaperWidth     = pSetupData->mnPaperWidth  / 100;
                long nPaperHeight    = pSetupData->mnPaperHeight / 100;
                // Ansonsten ueber die Papiergroesse suchen
                for( int i = 0; i < maPrinterData.mnFormCount; i++ )
                {
                    ImplFormInfo* pFormInfo = maPrinterData.mpFormArray[i];
                    if ( ImplPaperSizeEqual( nPaperWidth, nPaperHeight,
                                             pFormInfo->mnPaperWidth, pFormInfo->mnPaperHeight ) )
                    {
                        nOS2PaperFormat = pFormInfo->mnId;
                        break;
                    }
                }
                }
                break;
        }

        if ( nOS2PaperFormat != DJP_PSI_NONE )
        {
            if ( ImplSetPaperSize( maPrinterData.mhDC, pDrivData, nOS2PaperFormat ) )
                bOK = TRUE;
        }
    }

    // set paper tray
    if ( (nFlags & SAL_JOBSET_PAPERBIN) && (pSetupData->mnPaperBin < maPrinterData.mnTrayCount) )
    {
        if ( ImplSetPaperBin( maPrinterData.mhDC, pDrivData,
                              maPrinterData.mpTrayArray[pSetupData->mnPaperBin] ) )
            bOK = TRUE;
    }

    if ( bOK )
    {
        ImplUpdateSetupData( pDrivData, pSetupData );

        // query current driver settings
        ImplFreeFormAndTrayList( this );
        if ( ImplGetCurrentSettings( this, pSetupData ) )
        {
            // update DC and PS
            HDC hDC;
            HPS hPS;
            if ( ImplSalCreateInfoPrn( this, (PDRIVDATA)(pSetupData->mpDriverData), hDC, hPS ) )
            {
                // Alten Printer DC/PS zerstoeren
                ImplSalDestroyInfoPrn( this );

                // Neue Daten setzen und initialisieren
                maPrinterData.mhDC = hDC;
                maPrinterData.mhPS = hPS;
                maPrinterData.mpGraphics->maGraphicsData.mhDC = maPrinterData.mhDC;
                maPrinterData.mpGraphics->maGraphicsData.mhPS = maPrinterData.mhPS;
                ImplSalInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
            }
            else
                bOK = FALSE;
        }
        else
            bOK = FALSE;
    }

    return bOK;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pJobSetup )
{
    if ( !maPrinterData.mbDJPSupported )
        return 1;

    // init paperbinlist if empty
    if ( !maPrinterData.mnTrayCount )
        ImplGetFormAndTrayList( this, pJobSetup );

    // Wir haben immer einen PaperTray und wenn, das eben einen ohne
    // Namen
    if ( !maPrinterData.mnTrayCount )
        return 1;
    else
        return maPrinterData.mnTrayCount;
}

// -----------------------------------------------------------------------

XubString SalInfoPrinter::GetPaperBinName( const ImplJobSetup* pJobSetup,
                                          ULONG nPaperBin )
{
    XubString aPaperBinName;

    if ( maPrinterData.mbDJPSupported )
    {
        // init paperbinlist if empty
        if ( !maPrinterData.mnTrayCount )
            ImplGetFormAndTrayList( this, pJobSetup );

        if ( nPaperBin < maPrinterData.mnTrayCount )
            aPaperBinName = maPrinterData.mpTrayArray[nPaperBin]->maDisplayName;
    }

    return aPaperBinName;
}

// -----------------------------------------------------------------------

ULONG SalInfoPrinter::GetCapabilities( const ImplJobSetup*, USHORT nType )
{
    switch ( nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return TRUE;
        case PRINTER_CAPABILITIES_COPIES:
            return 0xFFFF;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
            return 0;
        case PRINTER_CAPABILITIES_SETORIENTATION:
        case PRINTER_CAPABILITIES_SETPAPERBIN:
        case PRINTER_CAPABILITIES_SETPAPERSIZE:
        case PRINTER_CAPABILITIES_SETPAPER:
            return maPrinterData.mbDJPSupported;
    }

    return 0;
}

// -----------------------------------------------------------------------

void SalInfoPrinter::GetPageInfo( const ImplJobSetup*,
                                  long& rOutWidth, long& rOutHeight,
                                  long& rPageOffX, long& rPageOffY,
                                  long& rPageWidth, long& rPageHeight )
{
    HDC hDC = maPrinterData.mhDC;

    // search current form
    HCINFO  aInfo;
    int nForms = DevQueryHardcopyCaps( hDC, 0, 0, &aInfo );
    for( int i = 0; i < nForms; i++ )
    {
        if ( DevQueryHardcopyCaps( hDC, i, 1, &aInfo ) >= 0 )
        {
            if ( aInfo.flAttributes & HCAPS_CURRENT )
            {
                // query resolution
                long nXResolution;
                long nYResolution;
                DevQueryCaps( hDC, CAPS_HORIZONTAL_RESOLUTION, 1, &nXResolution );
                DevQueryCaps( hDC, CAPS_VERTICAL_RESOLUTION, 1, &nYResolution );
                rPageOffX   = aInfo.xLeftClip * nXResolution / 1000;
                rPageOffY   = (aInfo.cy-aInfo.yTopClip) * nYResolution / 1000;
                rPageWidth  = aInfo.cx * nXResolution / 1000;
                rPageHeight = aInfo.cy * nYResolution / 1000;
                rOutWidth   = aInfo.xPels;
                rOutHeight  = aInfo.yPels;
                return;
            }
        }
    }

    // use device caps if no form selected/found
    long lCapsWidth = 0;
    long lCapsHeight = 0;
    DevQueryCaps( hDC, CAPS_WIDTH, 1L, &lCapsWidth );
    DevQueryCaps( hDC, CAPS_HEIGHT, 1L, &lCapsHeight );
    rPageOffX    = 0;
    rPageOffY    = 0;
    rOutWidth    = lCapsWidth;
    rOutHeight   = lCapsHeight;
    rPageWidth   = rOutWidth;
    rPageHeight  = rOutHeight;
}

// =======================================================================

static BOOL ImplIsDriverPrintDJPEnabled( HDC hDC )
{
#ifdef NO_DJP
    return FALSE;
#else
    // Ueber OS2-Ini kann DJP disablte werden
    if ( !PrfQueryProfileInt( HINI_PROFILE, SAL_PROFILE_APPNAME, SAL_PROFILE_PRINTDJP, 1 ) )
        return FALSE;

    // Testen, ob DJP-Interface am Drucker vorhanden
    LONG   lQuery;
    APIRET rc;

    lQuery = DEVESC_QUERYSIZE;
    rc = DevEscape( hDC,
                    DEVESC_QUERYESCSUPPORT,
                    sizeof( lQuery ),
                    (PBYTE)&lQuery,
                    0,
                    (PBYTE)NULL );
    if ( DEV_OK != rc )
        return FALSE;

    return TRUE;
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

SalPrinter::SalPrinter()
{
    maPrinterData.mhDC                  = 0;
    maPrinterData.mhPS                  = 0;
    maPrinterData.mpGraphics            = NULL;
    maPrinterData.mbAbort               = FALSE;
    maPrinterData.mbPrintDJPSupported   = FALSE;
}

// -----------------------------------------------------------------------

SalPrinter::~SalPrinter()
{
}

// -----------------------------------------------------------------------

BOOL SalPrinter::StartJob( const XubString* pFileName,
                           const XubString& rJobName,
                           const XubString& rAppName,
                           ULONG nCopies, BOOL bCollate,
                           ImplJobSetup* pSetupData )
{
    DEVOPENSTRUC    aDevOpenStruc;
    LONG            lType;
    APIRET          rc;

    // prepare queue information
    memset( &aDevOpenStruc, 0, sizeof( aDevOpenStruc ) );
    aDevOpenStruc.pszDriverName = (PSZ)(maPrinterData.mpInfoPrinter->maPrinterData.maDriverName.GetStr());

    // print into file?
    if ( pFileName )
    {
        aDevOpenStruc.pszLogAddress = (PSZ)pFileName->GetStr();
        aDevOpenStruc.pszDataType = "PM_Q_RAW";
        lType = OD_DIRECT;
    }
    else
    {
        aDevOpenStruc.pszLogAddress = (PSZ)(maPrinterData.mpInfoPrinter->maPrinterData.maName.GetStr());
        if ( PrfQueryProfileInt( HINI_PROFILE, SAL_PROFILE_APPNAME, SAL_PROFILE_PRINTRAW, 0 ) )
            aDevOpenStruc.pszDataType = "PM_Q_RAW";
        else
            aDevOpenStruc.pszDataType = "PM_Q_STD";
        lType = OD_QUEUED;
    }

    // Set comment (AppName nur bis zum 1. Space-Zeichen nehmen)
    const xub_Unicode*  pComment = rAppName;
    USHORT          nCommentLen = 0;
    memset( maPrinterData.maCommentBuf, 0, sizeof( maPrinterData.maCommentBuf ) );
    while ( (nCommentLen < 32) &&
            (((*pComment >= 'a') && (*pComment <= 'z')) ||
             ((*pComment >= 'A') && (*pComment <= 'Z')) ||
             ((*pComment >= '0') && (*pComment <= '9')) ||
             (*pComment == '-')))
    {
        maPrinterData.maCommentBuf[nCommentLen] = (char)(*pComment);
        nCommentLen++;
        pComment++;
    }
    aDevOpenStruc.pszComment = (PSZ)maPrinterData.maCommentBuf;

    // Kopien
    if ( nCopies > 1 )
    {
        // OS2 kann maximal 999 Kopien
        if ( nCopies > 999 )
            nCopies = 999;
        String aCopyStr( nCopies );
        strcpy( maPrinterData.maCopyBuf, "COP=" );
        strcat( maPrinterData.maCopyBuf+4, aCopyStr.GetStr() );
        aDevOpenStruc.pszQueueProcParams = (PSZ)maPrinterData.maCopyBuf;
    }

    // open device context
    SalData*    pSalData = GetSalData();
    HAB         hAB = pSalData->mhAB;
    aDevOpenStruc.pdriv = (PDRIVDATA)pSetupData->mpDriverData;
    maPrinterData.mhDC = DevOpenDC( hAB,
                                    lType,
                                    "*",
                                    7,
                                    (PDEVOPENDATA)&aDevOpenStruc,
                                    0 );
    if ( maPrinterData.mhDC == 0 )
    {
        ERRORID nLastError = WinGetLastError( hAB );
        if ( (nLastError & 0xFFFF) == PMERR_SPL_PRINT_ABORT )
            maPrinterData.mnError = SAL_PRINTER_ERROR_ABORT;
        else
            maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    // open presentation space
    SIZEL sizel;
    sizel.cx = 0;
    sizel.cy = 0;
    maPrinterData.mhPS = GpiCreatePS( hAB, maPrinterData.mhDC, &sizel, GPIA_ASSOC | GPIT_MICRO | PU_PELS );
    if ( !maPrinterData.mhPS )
    {
        DevCloseDC( maPrinterData.mhDC );
        maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return NULL;
    }

    // Can we print with DJP
    maPrinterData.mbPrintDJPSupported = ImplIsDriverPrintDJPEnabled( maPrinterData.mhDC );

    // JobName ermitteln und Job starten
    PSZ pszJobName = NULL;
    int nJobNameLen = 0;
    if ( rJobName.Len() > 0 )
    {
        pszJobName = (PSZ)rJobName.GetStr();
        nJobNameLen = rJobName.Len();
    }
    rc = DevEscape( maPrinterData.mhDC,
                    DEVESC_STARTDOC,
                    nJobNameLen, pszJobName,
                    0, NULL );

    if ( rc != DEV_OK )
    {
        ERRORID nLastError = WinGetLastError( hAB );
        if ( (nLastError & 0xFFFF) == PMERR_SPL_PRINT_ABORT )
            maPrinterData.mnError = SAL_PRINTER_ERROR_ABORT;
        else
            maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
        GpiAssociate( maPrinterData.mhPS, NULL );
        GpiDestroyPS( maPrinterData.mhPS );
        DevCloseDC( maPrinterData.mhDC );
        return FALSE;
    }

    // init for first page
    maPrinterData.mbFirstPage = TRUE;
    maPrinterData.mnError = 0;

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndJob()
{
    APIRET rc;
    rc = DevEscape( maPrinterData.mhDC,
                    DEVESC_ENDDOC,
                    0, NULL,
                    0, NULL);

    // destroy presentation space and device context
    GpiAssociate( maPrinterData.mhPS, NULL );
    GpiDestroyPS( maPrinterData.mhPS );
    DevCloseDC( maPrinterData.mhDC );
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::AbortJob()
{
    APIRET rc;

    rc = DevEscape( maPrinterData.mhDC,
                    DEVESC_ABORTDOC,
                    0, NULL,
                    0, NULL );

    // destroy SalGraphics
    if ( maPrinterData.mpGraphics )
    {
        ImplSalDeInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
        delete maPrinterData.mpGraphics;
        maPrinterData.mpGraphics = NULL;
    }

    // destroy presentation space and device context
    GpiAssociate( maPrinterData.mhPS, NULL );
    GpiDestroyPS( maPrinterData.mhPS );
    DevCloseDC( maPrinterData.mhDC );
    return TRUE;
}

// -----------------------------------------------------------------------

SalGraphics* SalPrinter::StartPage( ImplJobSetup* pSetupData, BOOL bNewJobSetup )
{
    APIRET rc;

    if ( maPrinterData.mbFirstPage )
        maPrinterData.mbFirstPage = FALSE;
    else
    {
        PBYTE   pJobData;
        LONG    nJobDataSize;
        LONG    nEscape;
        if ( maPrinterData.mbPrintDJPSupported && bNewJobSetup )
        {
            nEscape         = DEVESC_NEWFRAME_WPROP;
            nJobDataSize    = ((PDRIVDATA)(pSetupData->mpDriverData))->cb;
            pJobData        = (PBYTE)(pSetupData->mpDriverData);
        }
        else
        {
            nEscape         = DEVESC_NEWFRAME;
            nJobDataSize    = 0;
            pJobData        = NULL;
        }
        rc = DevEscape( maPrinterData.mhDC,
                        nEscape,
                        0, NULL,
                        &nJobDataSize, pJobData );

        if ( rc != DEV_OK )
        {
            DevEscape( maPrinterData.mhDC, DEVESC_ENDDOC, 0, NULL, 0, NULL);
            GpiAssociate( maPrinterData.mhPS, NULL );
            GpiDestroyPS( maPrinterData.mhPS );
            DevCloseDC( maPrinterData.mhDC );
            maPrinterData.mnError = SAL_PRINTER_ERROR_GENERALERROR;
            return NULL;
        }
    }

    // create SalGraphics with copy of hPS
    SalGraphics* pGraphics = new SalGraphics;
    pGraphics->maGraphicsData.mhDC              = maPrinterData.mhDC;
    pGraphics->maGraphicsData.mhPS              = maPrinterData.mhPS;
    pGraphics->maGraphicsData.mhWnd             = 0;
    pGraphics->maGraphicsData.mbPrinter         = TRUE;
    pGraphics->maGraphicsData.mbVirDev          = FALSE;
    pGraphics->maGraphicsData.mbWindow          = FALSE;
    pGraphics->maGraphicsData.mbScreen          = FALSE;
    pGraphics->maGraphicsData.mnHeight          = 0;
    // search current form for actual page height
    HCINFO  aInfo;
    int     nForms = DevQueryHardcopyCaps( maPrinterData.mhDC, 0, 0, &aInfo );
    for( int i = 0; i < nForms; i++ )
    {
        if ( DevQueryHardcopyCaps( maPrinterData.mhDC, i, 1, &aInfo ) >= 0 )
        {
            if ( aInfo.flAttributes & HCAPS_CURRENT )
                pGraphics->maGraphicsData.mnHeight  = aInfo.yPels;
        }
    }
    // use device caps if no form selected/found
    if ( !pGraphics->maGraphicsData.mnHeight )
        DevQueryCaps( maPrinterData.mhDC, CAPS_HEIGHT, 1L, &pGraphics->maGraphicsData.mnHeight );

    ImplSalInitGraphics( &(pGraphics->maGraphicsData) );
    maPrinterData.mpGraphics = pGraphics;

    return pGraphics;
}

// -----------------------------------------------------------------------

BOOL SalPrinter::EndPage()
{
    if ( maPrinterData.mpGraphics )
    {
        // destroy SalGraphics
        ImplSalDeInitGraphics( &(maPrinterData.mpGraphics->maGraphicsData) );
        delete maPrinterData.mpGraphics;
        maPrinterData.mpGraphics = NULL;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

ULONG SalPrinter::GetErrorCode()
{
    return maPrinterData.mnError;
}
