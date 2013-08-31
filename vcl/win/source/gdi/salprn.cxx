/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define WINVER 0x0500

#include <string.h>

#include <svsys.h>

#include <osl/module.h>

#include <tools/urlobj.hxx>

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salinst.h>
#include <win/salgdi.h>
#include <win/salframe.h>
#include <win/salprn.h>

#include <salptype.hxx>
#include <print.h>
#include <jobset.h>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/processfactory.hxx>

#include <malloc.h>

#if defined ( __MINGW32__ )
#include <sehandler.hxx>
#endif

#if defined ( __MINGW32__ ) && !defined ( _WIN64 )
#define CATCH_DRIVER_EX_BEGIN                                               \
    jmp_buf jmpbuf;                                                         \
    __SEHandler han;                                                        \
    if (__builtin_setjmp(jmpbuf) == 0)                                      \
    {                                                                       \
        han.Set(jmpbuf, NULL, (__SEHandler::PF)EXCEPTION_EXECUTE_HANDLER)

#define CATCH_DRIVER_EX_END(mes, p)                                         \
    }                                                                       \
    han.Reset()
#define CATCH_DRIVER_EX_END_2(mes)                                            \
    }                                                                       \
    han.Reset()
#else
#define CATCH_DRIVER_EX_BEGIN                                               \
    __try                                                                   \
    {
#define CATCH_DRIVER_EX_END(mes, p)                                         \
    }                                                                       \
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))\
    {                                                                       \
        OSL_FAIL( mes );                                                   \
        p->markInvalid();                                                   \
    }
#define CATCH_DRIVER_EX_END_2(mes)                                         \
    }                                                                       \
    __except(WinSalInstance::WorkaroundExceptionHandlingInUSER32Lib(GetExceptionCode(), GetExceptionInformation()))\
    {                                                                       \
        OSL_FAIL( mes );                                                   \
    }
#endif

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ui::dialogs;

static char aImplWindows[] = "windows";
static char aImplDevice[]  = "device";

static LPDEVMODEW SAL_DEVMODE_W( const ImplJobSetup* pSetupData )
{
    LPDEVMODEW pRet = NULL;
    SalDriverData* pDrv = (SalDriverData*)pSetupData->mpDriverData;
    if( pSetupData->mnDriverDataLen >= sizeof(DEVMODEW)+sizeof(SalDriverData)-1 )
        pRet = ((LPDEVMODEW)((pSetupData->mpDriverData) + (pDrv->mnDriverOffset)));
    return pRet;
}

static sal_uLong ImplWinQueueStatusToSal( DWORD nWinStatus )
{
    sal_uLong nStatus = 0;
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
    return nStatus;
}

// - WinSalInstance -

void WinSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    DWORD           i;
    DWORD           nBytes = 0;
    DWORD           nInfoPrn4 = 0;
    EnumPrintersW( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, NULL, 0, &nBytes, &nInfoPrn4 );
    if ( nBytes )
    {
        PRINTER_INFO_4W* pWinInfo4 = (PRINTER_INFO_4W*) rtl_allocateMemory( nBytes );
        if ( EnumPrintersW( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 4, (LPBYTE)pWinInfo4, nBytes, &nBytes, &nInfoPrn4 ) )
        {
            for ( i = 0; i < nInfoPrn4; i++ )
            {
                SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
                pInfo->maPrinterName = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo4[i].pPrinterName) );
                pInfo->mnStatus      = 0;
                pInfo->mnJobs        = 0;
                pInfo->mpSysData     = NULL;
                pList->Add( pInfo );
            }
        }
        rtl_freeMemory( pWinInfo4 );
    }
}

void WinSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
    HANDLE hPrinter = 0;
    LPWSTR pPrnName = reinterpret_cast<LPWSTR>(const_cast<sal_Unicode*>(pInfo->maPrinterName.getStr()));
    if( OpenPrinterW( pPrnName, &hPrinter, NULL ) )
    {
        DWORD               nBytes = 0;
        GetPrinterW( hPrinter, 2, NULL, 0, &nBytes );
        if( nBytes )
        {
            PRINTER_INFO_2W* pWinInfo2 = (PRINTER_INFO_2W*)rtl_allocateMemory(nBytes);
            if( GetPrinterW( hPrinter, 2, (LPBYTE)pWinInfo2, nBytes, &nBytes ) )
            {
                if( pWinInfo2->pDriverName )
                    pInfo->maDriver = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo2->pDriverName) );
                XubString aPortName;
                if ( pWinInfo2->pPortName )
                    aPortName = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo2->pPortName) );
                // pLocation can be 0 (the Windows docu doesn't describe this)
                if ( pWinInfo2->pLocation && *pWinInfo2->pLocation )
                    pInfo->maLocation = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo2->pLocation) );
                else
                    pInfo->maLocation = aPortName;
                // pComment can be 0 (the Windows docu doesn't describe this)
                if ( pWinInfo2->pComment )
                    pInfo->maComment = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo2->pComment) );
                pInfo->mnStatus      = ImplWinQueueStatusToSal( pWinInfo2->Status );
                pInfo->mnJobs        = pWinInfo2->cJobs;
                if( ! pInfo->mpSysData )
                    pInfo->mpSysData = new OUString(aPortName);
            }
            rtl_freeMemory(pWinInfo2);
        }
        ClosePrinter( hPrinter );
    }
}

void WinSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo->mpSysData;
    delete pInfo;
}

OUString WinSalInstance::GetDefaultPrinter()
{
    DWORD   nChars = 0;
    GetDefaultPrinterW( NULL, &nChars );
    if( nChars )
    {
        LPWSTR  pStr = (LPWSTR)rtl_allocateMemory(nChars*sizeof(WCHAR));
        XubString aDefPrt;
        if( GetDefaultPrinterW( pStr, &nChars ) )
        {
            aDefPrt = reinterpret_cast<sal_Unicode* >(pStr);
        }
        rtl_freeMemory( pStr );
        if( aDefPrt.Len() )
            return aDefPrt;
    }

    // get default printer from win.ini
    char szBuffer[256];
    GetProfileStringA( aImplWindows, aImplDevice, "", szBuffer, sizeof( szBuffer ) );
    if ( szBuffer[0] )
    {
        // search for printer name
        char* pBuf = szBuffer;
        char* pTmp = pBuf;
        while ( *pTmp && (*pTmp != ',') )
            pTmp++;
        return ImplSalGetUniString( pBuf, static_cast<sal_Int32>(pTmp-pBuf) );
    }
    else
        return OUString();
}

static DWORD ImplDeviceCaps( WinSalInfoPrinter* pPrinter, WORD nCaps,
                             BYTE* pOutput, const ImplJobSetup* pSetupData )
{
    DEVMODEW* pDevMode;
    if ( !pSetupData || !pSetupData->mpDriverData )
        pDevMode = NULL;
    else
        pDevMode = SAL_DEVMODE_W( pSetupData );

    return DeviceCapabilitiesW( reinterpret_cast<LPCWSTR>(pPrinter->maDeviceName.getStr()),
                                reinterpret_cast<LPCWSTR>(pPrinter->maPortName.getStr()),
                                nCaps, (LPWSTR)pOutput, pDevMode );
}

static sal_Bool ImplTestSalJobSetup( WinSalInfoPrinter* pPrinter,
                                 ImplJobSetup* pSetupData, sal_Bool bDelete )
{
    if ( pSetupData && pSetupData->mpDriverData )
    {
        // signature and size must fit to avoid using
        // JobSetups from a wrong system

        // initialize versions from jobsetup
        // those will be overwritten with driver's version
        DEVMODEW* pDevModeW = NULL;
        LONG dmSpecVersion = -1;
        LONG dmDriverVersion = -1;
        SalDriverData* pSalDriverData = (SalDriverData*)pSetupData->mpDriverData;
        BYTE* pDriverData = ((BYTE*)pSalDriverData) + pSalDriverData->mnDriverOffset;
        pDevModeW = (DEVMODEW*)pDriverData;

        long nSysJobSize = -1;
        if( pPrinter && pDevModeW )
        {
            // just too many driver crashes in that area -> check the dmSpecVersion and dmDriverVersion fields always !!!
            // this prevents using the jobsetup between different Windows versions (eg from XP to 9x) but we
            // can avoid potential driver crashes as their jobsetups are often not compatible
            // #110800#, #111151#, #112381#, #i16580#, #i14173# and perhaps #112375#
            HANDLE hPrn;
            LPWSTR pPrinterNameW = reinterpret_cast<LPWSTR>(const_cast<sal_Unicode*>(pPrinter->maDeviceName.getStr()));
            if ( !OpenPrinterW( pPrinterNameW, &hPrn, NULL ) )
                return FALSE;

            // #131642# hPrn==HGDI_ERROR even though OpenPrinter() succeeded!
            if( hPrn == HGDI_ERROR )
                return FALSE;

            nSysJobSize = DocumentPropertiesW( 0, hPrn,
                                               pPrinterNameW,
                                               NULL, NULL, 0 );

            if( nSysJobSize < 0 )
            {
                ClosePrinter( hPrn );
                return FALSE;
            }
            BYTE *pBuffer = (BYTE*)_alloca( nSysJobSize );
            LONG nRet = DocumentPropertiesW( 0, hPrn,
                                        pPrinterNameW,
                                        (LPDEVMODEW)pBuffer, NULL, DM_OUT_BUFFER );
            if( nRet < 0 )
            {
                ClosePrinter( hPrn );
                return FALSE;
            }

            // the spec version differs between the windows platforms, ie 98,NT,2000/XP
            // this allows us to throw away printer settings from other platforms that might crash a buggy driver
            // we check the driver version as well
            dmSpecVersion = ((DEVMODEW*)pBuffer)->dmSpecVersion;
            dmDriverVersion = ((DEVMODEW*)pBuffer)->dmDriverVersion;

            ClosePrinter( hPrn );
        }
        SalDriverData* pSetupDriverData = (SalDriverData*)(pSetupData->mpDriverData);
        if ( (pSetupData->mnSystem == JOBSETUP_SYSTEM_WINDOWS) &&
             (pPrinter->maDriverName == pSetupData->maDriver) &&
             (pSetupData->mnDriverDataLen > sizeof( SalDriverData )) &&
             (long)(pSetupData->mnDriverDataLen - pSetupDriverData->mnDriverOffset) == nSysJobSize &&
             pSetupDriverData->mnSysSignature == SAL_DRIVERDATA_SYSSIGN )
        {
            if( pDevModeW &&
                (dmSpecVersion == pDevModeW->dmSpecVersion) &&
                (dmDriverVersion == pDevModeW->dmDriverVersion) )
                return TRUE;
        }
        if ( bDelete )
        {
            rtl_freeMemory( pSetupData->mpDriverData );
            pSetupData->mpDriverData = NULL;
            pSetupData->mnDriverDataLen = 0;
        }
    }

    return FALSE;
}

static sal_Bool ImplUpdateSalJobSetup( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData,
                                   sal_Bool bIn, WinSalFrame* pVisibleDlgParent )
{
    HANDLE hPrn;
    LPWSTR pPrinterNameW = reinterpret_cast<LPWSTR>(const_cast<sal_Unicode*>(pPrinter->maDeviceName.getStr()));
    if ( !OpenPrinterW( pPrinterNameW, &hPrn, NULL ) )
        return FALSE;
    // #131642# hPrn==HGDI_ERROR even though OpenPrinter() succeeded!
    if( hPrn == HGDI_ERROR )
        return FALSE;

    LONG            nRet;
    HWND            hWnd = 0;
    DWORD           nMode = DM_OUT_BUFFER;
    sal_uLong           nDriverDataLen = 0;
    SalDriverData*  pOutBuffer = NULL;
    BYTE*           pInBuffer = NULL;

    LONG nSysJobSize = DocumentPropertiesW( hWnd, hPrn,
                                       pPrinterNameW,
                                       NULL, NULL, 0 );
    if ( nSysJobSize < 0 )
    {
        ClosePrinter( hPrn );
        return FALSE;
    }

    // make Outputbuffer
    nDriverDataLen              = sizeof(SalDriverData) + nSysJobSize-1;
    pOutBuffer                  = (SalDriverData*)rtl_allocateZeroMemory( nDriverDataLen );
    pOutBuffer->mnSysSignature  = SAL_DRIVERDATA_SYSSIGN;
    // calculate driver data offset including structure padding
    pOutBuffer->mnDriverOffset  = sal::static_int_cast<sal_uInt16>(
                                    (char*)pOutBuffer->maDriverData -
                                    (char*)pOutBuffer );

    // check if we have a suitable input buffer
    if ( bIn && ImplTestSalJobSetup( pPrinter, pSetupData, FALSE ) )
    {
        pInBuffer = (BYTE*)pSetupData->mpDriverData + ((SalDriverData*)pSetupData->mpDriverData)->mnDriverOffset;
        nMode |= DM_IN_BUFFER;
    }

    // check if the dialog should be shown
    if ( pVisibleDlgParent )
    {
        hWnd = pVisibleDlgParent->mhWnd;
        nMode |= DM_IN_PROMPT;
    }

    // Release mutex, in the other case we don't get paints and so on
    sal_uLong nMutexCount=0;
    if ( pVisibleDlgParent )
        nMutexCount = ImplSalReleaseYieldMutex();

    BYTE* pOutDevMode = (((BYTE*)pOutBuffer) + pOutBuffer->mnDriverOffset);
    nRet = DocumentPropertiesW( hWnd, hPrn,
                                pPrinterNameW,
                                (LPDEVMODEW)pOutDevMode, (LPDEVMODEW)pInBuffer, nMode );
    if ( pVisibleDlgParent )
        ImplSalAcquireYieldMutex( nMutexCount );
    ClosePrinter( hPrn );

    if( (nRet < 0) || (pVisibleDlgParent && (nRet == IDCANCEL)) )
    {
        rtl_freeMemory( pOutBuffer );
        return FALSE;
    }

    // fill up string buffers with 0 so they do not influence a JobSetup's memcmp
    if( ((LPDEVMODEW)pOutDevMode)->dmSize >= 64 )
    {
        sal_Int32 nLen = rtl_ustr_getLength( (const sal_Unicode*)((LPDEVMODEW)pOutDevMode)->dmDeviceName );
        if ( sal::static_int_cast<size_t>(nLen) < SAL_N_ELEMENTS( ((LPDEVMODEW)pOutDevMode)->dmDeviceName ) )
            memset( ((LPDEVMODEW)pOutDevMode)->dmDeviceName+nLen, 0, sizeof( ((LPDEVMODEW)pOutDevMode)->dmDeviceName )-(nLen*sizeof(sal_Unicode)) );
    }
    if( ((LPDEVMODEW)pOutDevMode)->dmSize >= 166 )
    {
        sal_Int32 nLen = rtl_ustr_getLength( (const sal_Unicode*)((LPDEVMODEW)pOutDevMode)->dmFormName );
        if ( sal::static_int_cast<size_t>(nLen) < SAL_N_ELEMENTS( ((LPDEVMODEW)pOutDevMode)->dmFormName ) )
            memset( ((LPDEVMODEW)pOutDevMode)->dmFormName+nLen, 0, sizeof( ((LPDEVMODEW)pOutDevMode)->dmFormName )-(nLen*sizeof(sal_Unicode)) );
    }

    // update data
    if ( pSetupData->mpDriverData )
        rtl_freeMemory( pSetupData->mpDriverData );
    pSetupData->mnDriverDataLen = nDriverDataLen;
    pSetupData->mpDriverData    = (BYTE*)pOutBuffer;
    pSetupData->mnSystem        = JOBSETUP_SYSTEM_WINDOWS;

    return TRUE;
}

#define DECLARE_DEVMODE( i )\
    DEVMODEW* pDevModeW = SAL_DEVMODE_W(i);\
    if( pDevModeW == NULL )\
        return

#define CHOOSE_DEVMODE(i)\
    (pDevModeW->i)

static void ImplDevModeToJobSetup( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData, sal_uLong nFlags )
{
    if ( !pSetupData || !pSetupData->mpDriverData )
        return;

    DECLARE_DEVMODE( pSetupData );

    // Orientation
    if ( nFlags & SAL_JOBSET_ORIENTATION )
    {
        if ( CHOOSE_DEVMODE(dmOrientation) == DMORIENT_PORTRAIT )
            pSetupData->meOrientation = ORIENTATION_PORTRAIT;
        else if ( CHOOSE_DEVMODE(dmOrientation) == DMORIENT_LANDSCAPE )
            pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
    }

    // PaperBin
    if ( nFlags & SAL_JOBSET_PAPERBIN )
    {
        sal_uLong nCount = ImplDeviceCaps( pPrinter, DC_BINS, NULL, pSetupData );

        if ( nCount && (nCount != GDI_ERROR) )
        {
            WORD* pBins = (WORD*)rtl_allocateZeroMemory( nCount*sizeof(WORD) );
            ImplDeviceCaps( pPrinter, DC_BINS, (BYTE*)pBins, pSetupData );
            pSetupData->mnPaperBin = 0;

            // search the right bin and assign index to mnPaperBin
            for( sal_uLong i = 0; i < nCount; i++ )
            {
                if( CHOOSE_DEVMODE(dmDefaultSource) == pBins[ i ] )
                {
                    pSetupData->mnPaperBin = (sal_uInt16)i;
                    break;
                }
            }

            rtl_freeMemory( pBins );
        }
    }

    // PaperSize
    if ( nFlags & SAL_JOBSET_PAPERSIZE )
    {
        if( (CHOOSE_DEVMODE(dmFields) & (DM_PAPERWIDTH|DM_PAPERLENGTH)) == (DM_PAPERWIDTH|DM_PAPERLENGTH) )
        {
            pSetupData->mnPaperWidth  = CHOOSE_DEVMODE(dmPaperWidth)*10;
            pSetupData->mnPaperHeight = CHOOSE_DEVMODE(dmPaperLength)*10;
        }
        else
        {
            sal_uLong   nPaperCount = ImplDeviceCaps( pPrinter, DC_PAPERS, NULL, pSetupData );
            WORD*   pPapers = NULL;
            sal_uLong   nPaperSizeCount = ImplDeviceCaps( pPrinter, DC_PAPERSIZE, NULL, pSetupData );
            POINT*  pPaperSizes = NULL;
            if ( nPaperCount && (nPaperCount != GDI_ERROR) )
            {
                pPapers = (WORD*)rtl_allocateZeroMemory(nPaperCount*sizeof(WORD));
                ImplDeviceCaps( pPrinter, DC_PAPERS, (BYTE*)pPapers, pSetupData );
            }
            if ( nPaperSizeCount && (nPaperSizeCount != GDI_ERROR) )
            {
                pPaperSizes = (POINT*)rtl_allocateZeroMemory(nPaperSizeCount*sizeof(POINT));
                ImplDeviceCaps( pPrinter, DC_PAPERSIZE, (BYTE*)pPaperSizes, pSetupData );
            }
            if( nPaperSizeCount == nPaperCount && pPaperSizes && pPapers )
            {
                for( sal_uLong i = 0; i < nPaperCount; i++ )
                {
                    if( pPapers[ i ] == CHOOSE_DEVMODE(dmPaperSize) )
                    {
                        pSetupData->mnPaperWidth  = pPaperSizes[ i ].x*10;
                        pSetupData->mnPaperHeight = pPaperSizes[ i ].y*10;
                        break;
                    }
                }
            }
            if( pPapers )
                rtl_freeMemory( pPapers );
            if( pPaperSizes )
                rtl_freeMemory( pPaperSizes );
        }
        switch( CHOOSE_DEVMODE(dmPaperSize) )
        {
            case( DMPAPER_LETTER ):
                pSetupData->mePaperFormat = PAPER_LETTER;
                break;
            case( DMPAPER_TABLOID ):
                pSetupData->mePaperFormat = PAPER_TABLOID;
                break;
            case( DMPAPER_LEDGER ):
                pSetupData->mePaperFormat = PAPER_LEDGER;
                break;
            case( DMPAPER_LEGAL ):
                pSetupData->mePaperFormat = PAPER_LEGAL;
                break;
            case( DMPAPER_STATEMENT ):
                pSetupData->mePaperFormat = PAPER_STATEMENT;
                break;
            case( DMPAPER_EXECUTIVE ):
                pSetupData->mePaperFormat = PAPER_EXECUTIVE;
                break;
            case( DMPAPER_A3 ):
                pSetupData->mePaperFormat = PAPER_A3;
                break;
            case( DMPAPER_A4 ):
                pSetupData->mePaperFormat = PAPER_A4;
                break;
            case( DMPAPER_A5 ):
                pSetupData->mePaperFormat = PAPER_A5;
                break;
            //See http://wiki.services.openoffice.org/wiki/DefaultPaperSize
            //i.e.
            //http://msdn.microsoft.com/en-us/library/dd319099(VS.85).aspx
            //DMPAPER_B4    12  B4 (JIS) 257 x 364 mm
            //http://partners.adobe.com/public/developer/en/ps/5003.PPD_Spec_v4.3.pdf
            //also says that the MS DMPAPER_B4 is JIS, which makes most sense. And
            //matches our Excel filter's belief about the matching XlPaperSize
            //enumeration.
            //
            //http://msdn.microsoft.com/en-us/library/ms776398(VS.85).aspx said
            ////"DMPAPER_B4     12  B4 (JIS) 250 x 354"
            //which is bogus as it's either JIS 257 x 364 or ISO 250 x 353
            //(cmc)
            case( DMPAPER_B4 ):
                pSetupData->mePaperFormat = PAPER_B4_JIS;
                break;
            case( DMPAPER_B5 ):
                pSetupData->mePaperFormat = PAPER_B5_JIS;
                break;
            case( DMPAPER_QUARTO ):
                pSetupData->mePaperFormat = PAPER_QUARTO;
                break;
            case( DMPAPER_10X14 ):
                pSetupData->mePaperFormat = PAPER_10x14;
                break;
            case( DMPAPER_NOTE ):
                pSetupData->mePaperFormat = PAPER_LETTER;
                break;
            case( DMPAPER_ENV_9 ):
                pSetupData->mePaperFormat = PAPER_ENV_9;
                break;
            case( DMPAPER_ENV_10 ):
                pSetupData->mePaperFormat = PAPER_ENV_10;
                break;
            case( DMPAPER_ENV_11 ):
                pSetupData->mePaperFormat = PAPER_ENV_11;
                break;
            case( DMPAPER_ENV_12 ):
                pSetupData->mePaperFormat = PAPER_ENV_12;
                break;
            case( DMPAPER_ENV_14 ):
                pSetupData->mePaperFormat = PAPER_ENV_14;
                break;
            case( DMPAPER_CSHEET ):
                pSetupData->mePaperFormat = PAPER_C;
                break;
            case( DMPAPER_DSHEET ):
                pSetupData->mePaperFormat = PAPER_D;
                break;
            case( DMPAPER_ESHEET ):
                pSetupData->mePaperFormat = PAPER_E;
                break;
            case( DMPAPER_ENV_DL):
                pSetupData->mePaperFormat = PAPER_ENV_DL;
                break;
            case( DMPAPER_ENV_C5):
                pSetupData->mePaperFormat = PAPER_ENV_C5;
                break;
            case( DMPAPER_ENV_C3):
                pSetupData->mePaperFormat = PAPER_ENV_C3;
                break;
            case( DMPAPER_ENV_C4):
                pSetupData->mePaperFormat = PAPER_ENV_C4;
                break;
            case( DMPAPER_ENV_C6):
                pSetupData->mePaperFormat = PAPER_ENV_C6;
                break;
            case( DMPAPER_ENV_C65):
                pSetupData->mePaperFormat = PAPER_ENV_C65;
                break;
            case( DMPAPER_ENV_ITALY ):
                pSetupData->mePaperFormat = PAPER_ENV_ITALY;
                break;
            case( DMPAPER_ENV_MONARCH ):
                pSetupData->mePaperFormat = PAPER_ENV_MONARCH;
                break;
            case( DMPAPER_ENV_PERSONAL ):
                pSetupData->mePaperFormat = PAPER_ENV_PERSONAL;
                break;
            case( DMPAPER_FANFOLD_US ):
                pSetupData->mePaperFormat = PAPER_FANFOLD_US;
                break;
            case( DMPAPER_FANFOLD_STD_GERMAN ):
                pSetupData->mePaperFormat = PAPER_FANFOLD_DE;
                break;
            case( DMPAPER_FANFOLD_LGL_GERMAN ):
                pSetupData->mePaperFormat = PAPER_FANFOLD_LEGAL_DE;
                break;
            case( DMPAPER_ISO_B4 ):
                pSetupData->mePaperFormat = PAPER_B4_ISO;
                break;
            case( DMPAPER_JAPANESE_POSTCARD ):
                pSetupData->mePaperFormat = PAPER_POSTCARD_JP;
                break;
            case( DMPAPER_9X11 ):
                pSetupData->mePaperFormat = PAPER_9x11;
                break;
            case( DMPAPER_10X11 ):
                pSetupData->mePaperFormat = PAPER_10x11;
                break;
            case( DMPAPER_15X11 ):
                pSetupData->mePaperFormat = PAPER_15x11;
                break;
            case( DMPAPER_ENV_INVITE ):
                pSetupData->mePaperFormat = PAPER_ENV_INVITE;
                break;
            case( DMPAPER_A_PLUS ):
                pSetupData->mePaperFormat = PAPER_A_PLUS;
                break;
            case( DMPAPER_B_PLUS ):
                pSetupData->mePaperFormat = PAPER_B_PLUS;
                break;
            case( DMPAPER_LETTER_PLUS ):
                pSetupData->mePaperFormat = PAPER_LETTER_PLUS;
                break;
            case( DMPAPER_A4_PLUS ):
                pSetupData->mePaperFormat = PAPER_A4_PLUS;
                break;
            case( DMPAPER_A2 ):
                pSetupData->mePaperFormat = PAPER_A2;
                break;
            case( DMPAPER_DBL_JAPANESE_POSTCARD ):
                pSetupData->mePaperFormat = PAPER_DOUBLEPOSTCARD_JP;
                break;
            case( DMPAPER_A6 ):
                pSetupData->mePaperFormat = PAPER_A6;
                break;
            case( DMPAPER_B6_JIS ):
                pSetupData->mePaperFormat = PAPER_B6_JIS;
                break;
            case( DMPAPER_12X11 ):
                pSetupData->mePaperFormat = PAPER_12x11;
                break;
            default:
                pSetupData->mePaperFormat = PAPER_USER;
                break;
        }
    }

    if( nFlags & SAL_JOBSET_DUPLEXMODE )
    {
        DuplexMode eDuplex = DUPLEX_UNKNOWN;
        if( (CHOOSE_DEVMODE(dmFields) & DM_DUPLEX) )
        {
            if( CHOOSE_DEVMODE(dmDuplex) == DMDUP_SIMPLEX )
                eDuplex = DUPLEX_OFF;
            else if( CHOOSE_DEVMODE(dmDuplex) == DMDUP_VERTICAL )
                eDuplex = DUPLEX_LONGEDGE;
            else if( CHOOSE_DEVMODE(dmDuplex) == DMDUP_HORIZONTAL )
                eDuplex = DUPLEX_SHORTEDGE;
        }
        pSetupData->meDuplexMode = eDuplex;
    }
}

static void ImplJobSetupToDevMode( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData, sal_uLong nFlags )
{
    if ( !pSetupData || !pSetupData->mpDriverData )
        return;

    DECLARE_DEVMODE( pSetupData );

    // Orientation
    if ( nFlags & SAL_JOBSET_ORIENTATION )
    {
        CHOOSE_DEVMODE(dmFields) |= DM_ORIENTATION;
        if ( pSetupData->meOrientation == ORIENTATION_PORTRAIT )
            CHOOSE_DEVMODE(dmOrientation) = DMORIENT_PORTRAIT;
        else
            CHOOSE_DEVMODE(dmOrientation) = DMORIENT_LANDSCAPE;
    }

    // PaperBin
    if ( nFlags & SAL_JOBSET_PAPERBIN )
    {
        sal_uLong nCount = ImplDeviceCaps( pPrinter, DC_BINS, NULL, pSetupData );

        if ( nCount && (nCount != GDI_ERROR) )
        {
            WORD* pBins = (WORD*)rtl_allocateZeroMemory(nCount*sizeof(WORD));
            ImplDeviceCaps( pPrinter, DC_BINS, (BYTE*)pBins, pSetupData );
            CHOOSE_DEVMODE(dmFields) |= DM_DEFAULTSOURCE;
            CHOOSE_DEVMODE(dmDefaultSource) = pBins[ pSetupData->mnPaperBin ];
            rtl_freeMemory( pBins );
        }
    }

    // PaperSize
    if ( nFlags & SAL_JOBSET_PAPERSIZE )
    {
        CHOOSE_DEVMODE(dmFields)        |= DM_PAPERSIZE;
        CHOOSE_DEVMODE(dmPaperWidth)     = 0;
        CHOOSE_DEVMODE(dmPaperLength)    = 0;

        switch( pSetupData->mePaperFormat )
        {
            case( PAPER_A2 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A2;
                break;
            case( PAPER_A3 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A3;
                break;
            case( PAPER_A4 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A4;
                break;
            case( PAPER_A5 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A5;
                break;
            case( PAPER_B4_ISO):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ISO_B4;
                break;
            case( PAPER_LETTER ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_LETTER;
                break;
            case( PAPER_LEGAL ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_LEGAL;
                break;
            case( PAPER_TABLOID ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_TABLOID;
                break;
#if 0
            //http://msdn.microsoft.com/en-us/library/ms776398(VS.85).aspx
            //DMPAPER_ENV_B6 is documented as:
            //"DMPAPER_ENV_B6   35  Envelope B6 176 x 125 mm"
            //which is the wrong way around, it is surely 125 x 176, i.e.
            //compare DMPAPER_ENV_B4 and DMPAPER_ENV_B4 as
            //DMPAPER_ENV_B4    33  Envelope B4 250 x 353 mm
            //DMPAPER_ENV_B5    34  Envelope B5 176 x 250 mm
            case( PAPER_B6_ISO ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_B6;
                break;
#endif
            case( PAPER_ENV_C4 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_C4;
                break;
            case( PAPER_ENV_C5 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_C5;
                break;
            case( PAPER_ENV_C6 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_C6;
                break;
            case( PAPER_ENV_C65 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_C65;
                break;
            case( PAPER_ENV_DL ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_DL;
                break;
            case( PAPER_C ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_CSHEET;
                break;
            case( PAPER_D ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_DSHEET;
                break;
            case( PAPER_E ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ESHEET;
                break;
            case( PAPER_EXECUTIVE ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_EXECUTIVE;
                break;
            case( PAPER_FANFOLD_LEGAL_DE ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_FANFOLD_LGL_GERMAN;
                break;
            case( PAPER_ENV_MONARCH ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_MONARCH;
                break;
            case( PAPER_ENV_PERSONAL ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_PERSONAL;
                break;
            case( PAPER_ENV_9 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_9;
                break;
            case( PAPER_ENV_10 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_10;
                break;
            case( PAPER_ENV_11 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_11;
                break;
            case( PAPER_ENV_12 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_12;
                break;
            //See the comments on DMPAPER_B4 above
            case( PAPER_B4_JIS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_B4;
                break;
            case( PAPER_B5_JIS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_B5;
                break;
            case( PAPER_B6_JIS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_B6_JIS;
                break;
            case( PAPER_LEDGER ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_LEDGER;
                break;
            case( PAPER_STATEMENT ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_STATEMENT;
                break;
            case( PAPER_10x14 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_10X14;
                break;
            case( PAPER_ENV_14 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_14;
                break;
            case( PAPER_ENV_C3 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_C3;
                break;
            case( PAPER_ENV_ITALY ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_ITALY;
                break;
            case( PAPER_FANFOLD_US ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_FANFOLD_US;
                break;
            case( PAPER_FANFOLD_DE ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_FANFOLD_STD_GERMAN;
                break;
            case( PAPER_POSTCARD_JP ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_JAPANESE_POSTCARD;
                break;
            case( PAPER_9x11 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_9X11;
                break;
            case( PAPER_10x11 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_10X11;
                break;
            case( PAPER_15x11 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_15X11;
                break;
            case( PAPER_ENV_INVITE ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_ENV_INVITE;
                break;
            case( PAPER_A_PLUS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A_PLUS;
                break;
            case( PAPER_B_PLUS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_B_PLUS;
                break;
            case( PAPER_LETTER_PLUS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_LETTER_PLUS;
                break;
            case( PAPER_A4_PLUS ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A4_PLUS;
                break;
            case( PAPER_DOUBLEPOSTCARD_JP ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_DBL_JAPANESE_POSTCARD;
                break;
            case( PAPER_A6 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_A6;
                break;
            case( PAPER_12x11 ):
                CHOOSE_DEVMODE(dmPaperSize) = DMPAPER_12X11;
                break;
            default:
            {
                short   nPaper = 0;
                sal_uLong   nPaperCount = ImplDeviceCaps( pPrinter, DC_PAPERS, NULL, pSetupData );
                WORD*   pPapers = NULL;
                sal_uLong   nPaperSizeCount = ImplDeviceCaps( pPrinter, DC_PAPERSIZE, NULL, pSetupData );
                POINT*  pPaperSizes = NULL;
                DWORD   nLandscapeAngle = ImplDeviceCaps( pPrinter, DC_ORIENTATION, NULL, pSetupData );
                if ( nPaperCount && (nPaperCount != GDI_ERROR) )
                {
                    pPapers = (WORD*)rtl_allocateZeroMemory(nPaperCount*sizeof(WORD));
                    ImplDeviceCaps( pPrinter, DC_PAPERS, (BYTE*)pPapers, pSetupData );
                }
                if ( nPaperSizeCount && (nPaperSizeCount != GDI_ERROR) )
                {
                    pPaperSizes = (POINT*)rtl_allocateZeroMemory(nPaperSizeCount*sizeof(POINT));
                    ImplDeviceCaps( pPrinter, DC_PAPERSIZE, (BYTE*)pPaperSizes, pSetupData );
                }
                if ( (nPaperSizeCount == nPaperCount) && pPapers && pPaperSizes )
                {
                    PaperInfo aInfo(pSetupData->mnPaperWidth, pSetupData->mnPaperHeight);
                    // compare paper formats and select a good match
                    for ( sal_uLong i = 0; i < nPaperCount; i++ )
                    {
                        if ( aInfo.sloppyEqual(PaperInfo(pPaperSizes[i].x*10, pPaperSizes[i].y*10)))
                        {
                            nPaper = pPapers[i];
                            break;
                        }
                    }

                    // If the printer supports landscape orientation, check paper sizes again
                    // with landscape orientation. This is necessary as a printer driver provides
                    // all paper sizes with portrait orientation only!!
                    if ( !nPaper && nLandscapeAngle != 0 )
                    {
                        PaperInfo aRotatedInfo(pSetupData->mnPaperHeight, pSetupData->mnPaperWidth);
                        for ( sal_uLong i = 0; i < nPaperCount; i++ )
                        {
                            if ( aRotatedInfo.sloppyEqual(PaperInfo(pPaperSizes[i].x*10, pPaperSizes[i].y*10)) )
                            {
                                nPaper = pPapers[i];
                                break;
                            }
                        }
                    }

                    if ( nPaper )
                        CHOOSE_DEVMODE(dmPaperSize) = nPaper;
                }

                if ( !nPaper )
                {
                    CHOOSE_DEVMODE(dmFields)       |= DM_PAPERLENGTH | DM_PAPERWIDTH;
                    CHOOSE_DEVMODE(dmPaperSize)     = DMPAPER_USER;
                    CHOOSE_DEVMODE(dmPaperWidth)    = (short)(pSetupData->mnPaperWidth/10);
                    CHOOSE_DEVMODE(dmPaperLength)   = (short)(pSetupData->mnPaperHeight/10);
                }

                if ( pPapers )
                    rtl_freeMemory(pPapers);
                if ( pPaperSizes )
                    rtl_freeMemory(pPaperSizes);

                break;
            }
        }
    }
    if( (nFlags & SAL_JOBSET_DUPLEXMODE) )
    {
        switch( pSetupData->meDuplexMode )
        {
        case DUPLEX_OFF:
            CHOOSE_DEVMODE(dmFields) |= DM_DUPLEX;
            CHOOSE_DEVMODE(dmDuplex) = DMDUP_SIMPLEX;
            break;
        case DUPLEX_SHORTEDGE:
            CHOOSE_DEVMODE(dmFields) |= DM_DUPLEX;
            CHOOSE_DEVMODE(dmDuplex) = DMDUP_HORIZONTAL;
            break;
        case DUPLEX_LONGEDGE:
            CHOOSE_DEVMODE(dmFields) |= DM_DUPLEX;
            CHOOSE_DEVMODE(dmDuplex) = DMDUP_VERTICAL;
            break;
        case DUPLEX_UNKNOWN:
            break;
        }
    }
}

static HDC ImplCreateICW_WithCatch( LPWSTR pDriver,
                                    LPCWSTR pDevice,
                                    LPDEVMODEW pDevMode )
{
    HDC hDC = 0;
    CATCH_DRIVER_EX_BEGIN;
    hDC = CreateICW( pDriver, pDevice, 0, pDevMode );
    CATCH_DRIVER_EX_END_2( "exception in CreateICW" );
    return hDC;
}

static HDC ImplCreateSalPrnIC( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData )
{
    HDC hDC = 0;
    LPDEVMODEW pDevMode;
    if ( pSetupData && pSetupData->mpDriverData )
        pDevMode = SAL_DEVMODE_W( pSetupData );
    else
        pDevMode = NULL;
    // #95347 some buggy drivers (eg, OKI) write to those buffers in CreateIC, although declared const - so provide some space
    // pl: does this hold true for Unicode functions ?
    if( pPrinter->maDriverName.getLength() > 2048 || pPrinter->maDeviceName.getLength() > 2048 )
        return 0;
    sal_Unicode pDriverName[ 4096 ];
    sal_Unicode pDeviceName[ 4096 ];
    memcpy( pDriverName, pPrinter->maDriverName.getStr(), pPrinter->maDriverName.getLength()*sizeof(sal_Unicode));
    memset( pDriverName+pPrinter->maDriverName.getLength(), 0, 32 );
    memcpy( pDeviceName, pPrinter->maDeviceName.getStr(), pPrinter->maDeviceName.getLength()*sizeof(sal_Unicode));
    memset( pDeviceName+pPrinter->maDeviceName.getLength(), 0, 32 );
    hDC = ImplCreateICW_WithCatch( reinterpret_cast< LPWSTR >(pDriverName),
                                   reinterpret_cast< LPCWSTR >(pDeviceName),
                                   pDevMode );
    return hDC;
}

static WinSalGraphics* ImplCreateSalPrnGraphics( HDC hDC )
{
    WinSalGraphics* pGraphics = new WinSalGraphics;
    pGraphics->SetLayout( 0 );
    pGraphics->setHDC(hDC);
    pGraphics->mhWnd    = 0;
    pGraphics->mbPrinter = TRUE;
    pGraphics->mbVirDev = FALSE;
    pGraphics->mbWindow = FALSE;
    pGraphics->mbScreen = FALSE;
    ImplSalInitGraphics( pGraphics );
    return pGraphics;
}

static sal_Bool ImplUpdateSalPrnIC( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData )
{
    HDC hNewDC = ImplCreateSalPrnIC( pPrinter, pSetupData );
    if ( !hNewDC )
        return FALSE;

    if ( pPrinter->mpGraphics )
    {
        ImplSalDeInitGraphics( pPrinter->mpGraphics );
        DeleteDC( pPrinter->mpGraphics->getHDC() );
        delete pPrinter->mpGraphics;
    }

    pPrinter->mpGraphics = ImplCreateSalPrnGraphics( hNewDC );
    pPrinter->mhDC      = hNewDC;

    return TRUE;
}

// - WinSalInstance -

SalInfoPrinter* WinSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup* pSetupData )
{
    WinSalInfoPrinter* pPrinter = new WinSalInfoPrinter;
    if( ! pQueueInfo->mpSysData )
        GetPrinterQueueState( pQueueInfo );
    pPrinter->maDriverName  = pQueueInfo->maDriver;
    pPrinter->maDeviceName  = pQueueInfo->maPrinterName;
    pPrinter->maPortName    = pQueueInfo->mpSysData ?
                                *pQueueInfo->mpSysData
                              : OUString();

    // check if the provided setup data match the actual printer
    ImplTestSalJobSetup( pPrinter, pSetupData, TRUE );

    HDC hDC = ImplCreateSalPrnIC( pPrinter, pSetupData );
    if ( !hDC )
    {
        delete pPrinter;
        return NULL;
    }

    pPrinter->mpGraphics = ImplCreateSalPrnGraphics( hDC );
    pPrinter->mhDC      = hDC;
    if ( !pSetupData->mpDriverData )
        ImplUpdateSalJobSetup( pPrinter, pSetupData, FALSE, NULL );
    ImplDevModeToJobSetup( pPrinter, pSetupData, SAL_JOBSET_ALL );
    pSetupData->mnSystem = JOBSETUP_SYSTEM_WINDOWS;

    return pPrinter;
}

void WinSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// - WinSalInfoPrinter -

WinSalInfoPrinter::WinSalInfoPrinter() :
    mpGraphics( NULL ),
    mhDC( 0 ),
    mbGraphics( FALSE )
{
    m_bPapersInit = FALSE;
}

WinSalInfoPrinter::~WinSalInfoPrinter()
{
    if ( mpGraphics )
    {
        ImplSalDeInitGraphics( mpGraphics );
        DeleteDC( mpGraphics->getHDC() );
        delete mpGraphics;
    }
}

void WinSalInfoPrinter::InitPaperFormats( const ImplJobSetup* pSetupData )
{
    m_aPaperFormats.clear();

    DWORD nCount = ImplDeviceCaps( this, DC_PAPERSIZE, NULL, pSetupData );
    if( nCount == GDI_ERROR )
        nCount = 0;

    if( nCount )
    {
        POINT* pPaperSizes = (POINT*)rtl_allocateZeroMemory(nCount*sizeof(POINT));
        ImplDeviceCaps( this, DC_PAPERSIZE, (BYTE*)pPaperSizes, pSetupData );

        sal_Unicode* pNamesBuffer = (sal_Unicode*)rtl_allocateMemory(nCount*64*sizeof(sal_Unicode));
        ImplDeviceCaps( this, DC_PAPERNAMES, (BYTE*)pNamesBuffer, pSetupData );
        for( DWORD i = 0; i < nCount; ++i )
        {
            PaperInfo aInfo(pPaperSizes[i].x * 10, pPaperSizes[i].y * 10);
            m_aPaperFormats.push_back( aInfo );
        }
        rtl_freeMemory( pNamesBuffer );
        rtl_freeMemory( pPaperSizes );
    }

    m_bPapersInit = true;
}

int WinSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* pSetupData )
{
    int nRet = ImplDeviceCaps( this, DC_ORIENTATION, NULL, pSetupData );

    if( nRet != sal::static_int_cast<int>( GDI_ERROR ) )
        return nRet * 10;
    else
        return 900; // guess
}

SalGraphics* WinSalInfoPrinter::GetGraphics()
{
    if ( mbGraphics )
        return NULL;

    if ( mpGraphics )
        mbGraphics = TRUE;

    return mpGraphics;
}

void WinSalInfoPrinter::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = FALSE;
}

sal_Bool WinSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
    if ( ImplUpdateSalJobSetup( this, pSetupData, TRUE, static_cast<WinSalFrame*>(pFrame) ) )
    {
        ImplDevModeToJobSetup( this, pSetupData, SAL_JOBSET_ALL );
        return ImplUpdateSalPrnIC( this, pSetupData );
    }

    return FALSE;
}

sal_Bool WinSalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
    if ( !ImplTestSalJobSetup( this, pSetupData, FALSE ) )
        return FALSE;
    return ImplUpdateSalPrnIC( this, pSetupData );
}

sal_Bool WinSalInfoPrinter::SetData( sal_uLong nFlags, ImplJobSetup* pSetupData )
{
    ImplJobSetupToDevMode( this, pSetupData, nFlags );
    if ( ImplUpdateSalJobSetup( this, pSetupData, TRUE, NULL ) )
    {
        ImplDevModeToJobSetup( this, pSetupData, nFlags );
        return ImplUpdateSalPrnIC( this, pSetupData );
    }

    return FALSE;
}

sal_uLong WinSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
    DWORD nRet = ImplDeviceCaps( this, DC_BINS, NULL, pSetupData );
    if ( nRet && (nRet != GDI_ERROR) )
        return nRet;
    else
        return 0;
}

OUString WinSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, sal_uLong nPaperBin )
{
    XubString aPaperBinName;

    DWORD nBins = ImplDeviceCaps( this, DC_BINNAMES, NULL, pSetupData );
    if ( (nPaperBin < nBins) && (nBins != GDI_ERROR) )
    {
        sal_Unicode* pBuffer = new sal_Unicode[nBins*24];
        DWORD nRet = ImplDeviceCaps( this, DC_BINNAMES, (BYTE*)pBuffer, pSetupData );
        if ( nRet && (nRet != GDI_ERROR) )
            aPaperBinName = pBuffer + (nPaperBin*24);
        delete [] pBuffer;
    }

    return aPaperBinName;
}

sal_uLong WinSalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType )
{
    DWORD nRet;

    switch ( nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return TRUE;
        case PRINTER_CAPABILITIES_COPIES:
            nRet = ImplDeviceCaps( this, DC_COPIES, NULL, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return nRet;
            return 0;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
            nRet = ImplDeviceCaps( this, DC_COLLATE, NULL, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
            {
                nRet = ImplDeviceCaps( this, DC_COPIES, NULL, pSetupData );
                if ( nRet && (nRet != GDI_ERROR) )
                    return nRet;
            }
            return 0;

        case PRINTER_CAPABILITIES_SETORIENTATION:
            nRet = ImplDeviceCaps( this, DC_ORIENTATION, NULL, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return TRUE;
            return FALSE;

        case PRINTER_CAPABILITIES_SETPAPERBIN:
            nRet = ImplDeviceCaps( this, DC_BINS, NULL, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return TRUE;
            return FALSE;

        case PRINTER_CAPABILITIES_SETPAPERSIZE:
        case PRINTER_CAPABILITIES_SETPAPER:
            nRet = ImplDeviceCaps( this, DC_PAPERS, NULL, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return TRUE;
            return FALSE;
    }

    return 0;
}

void WinSalInfoPrinter::GetPageInfo( const ImplJobSetup*,
                                  long& rOutWidth, long& rOutHeight,
                                  long& rPageOffX, long& rPageOffY,
                                  long& rPageWidth, long& rPageHeight )
{
    HDC hDC = mhDC;

    rOutWidth   = GetDeviceCaps( hDC, HORZRES );
    rOutHeight  = GetDeviceCaps( hDC, VERTRES );

    rPageOffX   = GetDeviceCaps( hDC, PHYSICALOFFSETX );
    rPageOffY   = GetDeviceCaps( hDC, PHYSICALOFFSETY );
    rPageWidth  = GetDeviceCaps( hDC, PHYSICALWIDTH );
    rPageHeight = GetDeviceCaps( hDC, PHYSICALHEIGHT );
}

// - WinSalInstance -

SalPrinter* WinSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    WinSalPrinter* pPrinter = new WinSalPrinter;
    pPrinter->mpInfoPrinter = static_cast<WinSalInfoPrinter*>(pInfoPrinter);
    return pPrinter;
}

void WinSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

BOOL CALLBACK SalPrintAbortProc( HDC hPrnDC, int /* nError */ )
{
    SalData*    pSalData = GetSalData();
    WinSalPrinter* pPrinter;
    sal_Bool        bWhile = TRUE;
    int         i = 0;

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

        pPrinter = pSalData->mpFirstPrinter;
        while ( pPrinter )
        {
            if( pPrinter->mhDC == hPrnDC )
                break;

            pPrinter = pPrinter->mpNextPrinter;
        }

        if ( !pPrinter || pPrinter->mbAbort )
            return FALSE;
    }
    while ( bWhile );

    return TRUE;
}

static LPDEVMODEW ImplSalSetCopies( LPDEVMODEW pDevMode, sal_uLong nCopies, sal_Bool bCollate )
{
    LPDEVMODEW pNewDevMode = pDevMode;
    if ( pDevMode && (nCopies > 1) )
    {
        if ( nCopies > 32765 )
            nCopies = 32765;
        sal_uLong nDevSize = pDevMode->dmSize+pDevMode->dmDriverExtra;
        pNewDevMode = (LPDEVMODEW)rtl_allocateMemory( nDevSize );
        memcpy( pNewDevMode, pDevMode, nDevSize );
        pDevMode = pNewDevMode;
        pDevMode->dmFields |= DM_COPIES;
        pDevMode->dmCopies  = (short)(sal_uInt16)nCopies;
        pDevMode->dmFields |= DM_COLLATE;
        if ( bCollate )
            pDevMode->dmCollate = DMCOLLATE_TRUE;
        else
            pDevMode->dmCollate = DMCOLLATE_FALSE;
    }

    return pNewDevMode;
}

// - WinSalPrinter -

WinSalPrinter::WinSalPrinter() :
    mpGraphics( NULL ),
    mpInfoPrinter( NULL ),
    mpNextPrinter( NULL ),
    mhDC( 0 ),
    mnError( 0 ),
    mnCopies( 0 ),
    mbCollate( FALSE ),
    mbAbort( FALSE ),
    mbValid( true )
{
    SalData* pSalData = GetSalData();
    // insert printer in printerlist
    mpNextPrinter = pSalData->mpFirstPrinter;
    pSalData->mpFirstPrinter = this;
}

WinSalPrinter::~WinSalPrinter()
{
    SalData* pSalData = GetSalData();

    // release DC if there is one still around because of AbortJob
    HDC hDC = mhDC;
    if ( hDC )
    {
        if ( mpGraphics )
        {
            ImplSalDeInitGraphics( mpGraphics );
            delete mpGraphics;
        }

        DeleteDC( hDC );
    }

    // remove printer from printerlist
    if ( this == pSalData->mpFirstPrinter )
        pSalData->mpFirstPrinter = mpNextPrinter;
    else
    {
        WinSalPrinter* pTempPrinter = pSalData->mpFirstPrinter;

        while( pTempPrinter->mpNextPrinter != this )
            pTempPrinter = pTempPrinter->mpNextPrinter;

        pTempPrinter->mpNextPrinter = mpNextPrinter;
    }
    mbValid = false;
}

void WinSalPrinter::markInvalid()
{
    mbValid = false;
}

// need wrappers for StarTocW/A to use structured exception handling
// since SEH does not mix with standard exception handling's cleanup
static int lcl_StartDocW( HDC hDC, DOCINFOW* pInfo, WinSalPrinter* pPrt )
{
    (void) pPrt;
    int nRet = 0;
    CATCH_DRIVER_EX_BEGIN;
    nRet = ::StartDocW( hDC, pInfo );
    CATCH_DRIVER_EX_END( "exception in StartDocW", pPrt );
    return nRet;
}

sal_Bool WinSalPrinter::StartJob( const OUString* pFileName,
                           const OUString& rJobName,
                           const OUString&,
                           sal_uLong nCopies,
                           bool bCollate,
                           bool /*bDirect*/,
                           ImplJobSetup* pSetupData )
{
    mnError     = 0;
    mbAbort     = FALSE;
    mnCopies        = nCopies;
    mbCollate   = bCollate;

    LPDEVMODEW  pOrgDevModeW = NULL;
    LPDEVMODEW  pDevModeW = NULL;
    HDC hDC = 0;
    if ( pSetupData && pSetupData->mpDriverData )
    {
        pOrgDevModeW = SAL_DEVMODE_W( pSetupData );
        pDevModeW = ImplSalSetCopies( pOrgDevModeW, nCopies, bCollate );
    }

    // #95347 some buggy drivers (eg, OKI) write to those buffers in CreateDC, although declared const - so provide some space
    sal_Unicode aDrvBuf[4096];
    sal_Unicode aDevBuf[4096];
    memcpy( aDrvBuf, mpInfoPrinter->maDriverName.getStr(), (mpInfoPrinter->maDriverName.getLength()+1)*sizeof(sal_Unicode));
    memcpy( aDevBuf, mpInfoPrinter->maDeviceName.getStr(), (mpInfoPrinter->maDeviceName.getLength()+1)*sizeof(sal_Unicode));
    hDC = CreateDCW( reinterpret_cast<LPCWSTR>(aDrvBuf),
                     reinterpret_cast<LPCWSTR>(aDevBuf),
                     NULL,
                     pDevModeW );

    if ( pDevModeW != pOrgDevModeW )
        rtl_freeMemory( pDevModeW );

    if ( !hDC )
    {
        mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    // make sure mhDC is set before the printer driver may call our abortproc
    mhDC = hDC;
    if ( SetAbortProc( hDC, SalPrintAbortProc ) <= 0 )
    {
        mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    mnError = 0;
    mbAbort = FALSE;

    // As the Telocom Balloon Fax driver tends to send messages repeatedly
    // we try to process first all, and then insert a dummy message
    sal_Bool bWhile = TRUE;
    int  i = 0;
    do
    {
        // process messages
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
    ImplPostMessage( GetSalData()->mpFirstInstance->mhComWnd, SAL_MSG_DUMMY, 0, 0 );

    // bring up a file choser if printing to file port but no file name given
    OUString aOutFileName;
    if( mpInfoPrinter->maPortName.equalsIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "FILE:" ) ) && !(pFileName && !pFileName->isEmpty()) )
    {

        uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference< XFilePicker3 > xFilePicker = FilePicker::createWithMode(xContext, TemplateDescription::FILESAVE_SIMPLE);

        if( xFilePicker->execute() == ExecutableDialogResults::OK )
        {
            Sequence< OUString > aPathSeq( xFilePicker->getFiles() );
            INetURLObject aObj( aPathSeq[0] );
            // we're using ansi calls (StartDocA) so convert the string
            aOutFileName = aObj.PathToFileName();
        }
        else
        {
            mnError = SAL_PRINTER_ERROR_ABORT;
            return FALSE;
        }
    }

    DOCINFOW aInfo;
    memset( &aInfo, 0, sizeof( DOCINFOW ) );
    aInfo.cbSize = sizeof( aInfo );
    aInfo.lpszDocName = (LPWSTR)rJobName.getStr();
    if ( pFileName || aOutFileName.getLength() )
    {
        if ( (pFileName && !pFileName->isEmpty()) || aOutFileName.getLength() )
        {
            aInfo.lpszOutput = (LPWSTR)( (pFileName && !pFileName->isEmpty()) ? pFileName->getStr() : aOutFileName.getStr());
        }
        else
            aInfo.lpszOutput = L"FILE:";
    }
    else
        aInfo.lpszOutput = NULL;

    // start Job
    int nRet = lcl_StartDocW( hDC, &aInfo, this );

    if ( nRet <= 0 )
    {
        long nError = GetLastError();
        if ( (nRet == SP_USERABORT) || (nRet == SP_APPABORT) || (nError == ERROR_PRINT_CANCELLED) || (nError == ERROR_CANCELLED) )
            mnError = SAL_PRINTER_ERROR_ABORT;
        else
            mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }

    return TRUE;
}

sal_Bool WinSalPrinter::EndJob()
{
    HDC hDC = mhDC;
    if ( isValid() && hDC )
    {
        if ( mpGraphics )
        {
            ImplSalDeInitGraphics( mpGraphics );
            delete mpGraphics;
            mpGraphics = NULL;
        }

        // #i54419# Windows fax printer brings up a dialog in EndDoc
        // which text previously copied in soffice process can be
        // pasted to -> deadlock due to mutex not released.
        // it should be safe to release the yield mutex over the EndDoc
        // call, however the real solution is supposed to be the threading
        // framework yet to come.
        volatile sal_uLong nAcquire = GetSalData()->mpFirstInstance->ReleaseYieldMutex();
        CATCH_DRIVER_EX_BEGIN;
        if( ::EndDoc( hDC ) <= 0 )
            GetLastError();
        CATCH_DRIVER_EX_END( "exception in EndDoc", this );

        GetSalData()->mpFirstInstance->AcquireYieldMutex( nAcquire );
        DeleteDC( hDC );
        mhDC = 0;
    }

    return TRUE;
}

sal_Bool WinSalPrinter::AbortJob()
{
    mbAbort = TRUE;

    // trigger Abort asynchronously
    HDC hDC = mhDC;
    if ( hDC )
    {
        SalData* pSalData = GetSalData();
        ImplPostMessage( pSalData->mpFirstInstance->mhComWnd,
                         SAL_MSG_PRINTABORTJOB, (WPARAM)hDC, 0 );
    }

    return TRUE;
}

void ImplSalPrinterAbortJobAsync( HDC hPrnDC )
{
    SalData*    pSalData = GetSalData();
    WinSalPrinter* pPrinter = pSalData->mpFirstPrinter;

    // check if printer still exists
    while ( pPrinter )
    {
        if ( pPrinter->mhDC == hPrnDC )
            break;

        pPrinter = pPrinter->mpNextPrinter;
    }

    // if printer still exists, cancel the job
    if ( pPrinter )
    {
        HDC hDC = pPrinter->mhDC;
        if ( hDC )
        {
            if ( pPrinter->mpGraphics )
            {
                ImplSalDeInitGraphics( pPrinter->mpGraphics );
                delete pPrinter->mpGraphics;
                pPrinter->mpGraphics = NULL;
            }

            CATCH_DRIVER_EX_BEGIN;
            ::AbortDoc( hDC );
            CATCH_DRIVER_EX_END( "exception in AbortDoc", pPrinter );

            DeleteDC( hDC );
            pPrinter->mhDC = 0;
        }
    }
}

SalGraphics* WinSalPrinter::StartPage( ImplJobSetup* pSetupData, sal_Bool bNewJobData )
{
    if( ! isValid() || mhDC == 0 )
        return NULL;

    HDC hDC = mhDC;
    if ( pSetupData && pSetupData->mpDriverData && bNewJobData )
    {
        LPDEVMODEW  pOrgDevModeW;
        LPDEVMODEW  pDevModeW;
        pOrgDevModeW = SAL_DEVMODE_W( pSetupData );
        pDevModeW = ImplSalSetCopies( pOrgDevModeW, mnCopies, mbCollate );
        ResetDCW( hDC, pDevModeW );
        if ( pDevModeW != pOrgDevModeW )
            rtl_freeMemory( pDevModeW );
    }
    volatile int nRet = 0;
    CATCH_DRIVER_EX_BEGIN;
    nRet = ::StartPage( hDC );
    CATCH_DRIVER_EX_END( "exception in StartPage", this );

    if ( nRet <= 0 )
    {
        GetLastError();
        mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return NULL;
    }

    // Hack to work around old PostScript printer drivers optimizing away empty pages
    // TODO: move into ImplCreateSalPrnGraphics()?
    HPEN    hTempPen = SelectPen( hDC, GetStockPen( NULL_PEN ) );
    HBRUSH  hTempBrush = SelectBrush( hDC, GetStockBrush( NULL_BRUSH ) );
    WIN_Rectangle( hDC, -8000, -8000, -7999, -7999 );
    SelectPen( hDC, hTempPen );
    SelectBrush( hDC, hTempBrush );

    mpGraphics = ImplCreateSalPrnGraphics( hDC );
    return mpGraphics;
}

sal_Bool WinSalPrinter::EndPage()
{
    HDC hDC = mhDC;
    if ( hDC && mpGraphics )
    {
        ImplSalDeInitGraphics( mpGraphics );
        delete mpGraphics;
        mpGraphics = NULL;
    }

    if( ! isValid() )
        return FALSE;

    volatile int nRet = 0;
    CATCH_DRIVER_EX_BEGIN;
    nRet = ::EndPage( hDC );
    CATCH_DRIVER_EX_END( "exception in EndPage", this );

    if ( nRet > 0 )
        return TRUE;
    else
    {
        GetLastError();
        mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return FALSE;
    }
}

sal_uLong WinSalPrinter::GetErrorCode()
{
    return mnError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
