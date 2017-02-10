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

#include <sal/config.h>

#include <memory>
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

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ui::dialogs;

static char aImplWindows[] = "windows";
static char aImplDevice[]  = "device";

static DEVMODEW const * SAL_DEVMODE_W( const ImplJobSetup* pSetupData )
{
    DEVMODEW const * pRet = nullptr;
    SalDriverData const * pDrv = reinterpret_cast<SalDriverData const *>(pSetupData->GetDriverData());
    if( pSetupData->GetDriverDataLen() >= sizeof(DEVMODEW)+sizeof(SalDriverData)-1 )
        pRet = reinterpret_cast<DEVMODEW const *>((pSetupData->GetDriverData()) + (pDrv->mnDriverOffset));
    return pRet;
}

static PrintQueueFlags ImplWinQueueStatusToSal( DWORD nWinStatus )
{
    PrintQueueFlags nStatus = PrintQueueFlags::NONE;
    if ( nWinStatus & PRINTER_STATUS_PAUSED )
        nStatus |= PrintQueueFlags::Paused;
    if ( nWinStatus & PRINTER_STATUS_ERROR )
        nStatus |= PrintQueueFlags::Error;
    if ( nWinStatus & PRINTER_STATUS_PENDING_DELETION )
        nStatus |= PrintQueueFlags::PendingDeletion;
    if ( nWinStatus & PRINTER_STATUS_PAPER_JAM )
        nStatus |= PrintQueueFlags::PaperJam;
    if ( nWinStatus & PRINTER_STATUS_PAPER_OUT )
        nStatus |= PrintQueueFlags::PaperOut;
    if ( nWinStatus & PRINTER_STATUS_MANUAL_FEED )
        nStatus |= PrintQueueFlags::ManualFeed;
    if ( nWinStatus & PRINTER_STATUS_PAPER_PROBLEM )
        nStatus |= PrintQueueFlags::PaperProblem;
    if ( nWinStatus & PRINTER_STATUS_OFFLINE )
        nStatus |= PrintQueueFlags::Offline;
    if ( nWinStatus & PRINTER_STATUS_IO_ACTIVE )
        nStatus |= PrintQueueFlags::IOActive;
    if ( nWinStatus & PRINTER_STATUS_BUSY )
        nStatus |= PrintQueueFlags::Busy;
    if ( nWinStatus & PRINTER_STATUS_PRINTING )
        nStatus |= PrintQueueFlags::Printing;
    if ( nWinStatus & PRINTER_STATUS_OUTPUT_BIN_FULL )
        nStatus |= PrintQueueFlags::OutputBinFull;
    if ( nWinStatus & PRINTER_STATUS_WAITING )
        nStatus |= PrintQueueFlags::Waiting;
    if ( nWinStatus & PRINTER_STATUS_PROCESSING )
        nStatus |= PrintQueueFlags::Processing;
    if ( nWinStatus & PRINTER_STATUS_INITIALIZING )
        nStatus |= PrintQueueFlags::Initializing;
    if ( nWinStatus & PRINTER_STATUS_WARMING_UP )
        nStatus |= PrintQueueFlags::WarmingUp;
    if ( nWinStatus & PRINTER_STATUS_TONER_LOW )
        nStatus |= PrintQueueFlags::TonerLow;
    if ( nWinStatus & PRINTER_STATUS_NO_TONER )
        nStatus |= PrintQueueFlags::NoToner;
    if ( nWinStatus & PRINTER_STATUS_PAGE_PUNT )
        nStatus |= PrintQueueFlags::PagePunt;
    if ( nWinStatus & PRINTER_STATUS_USER_INTERVENTION )
        nStatus |= PrintQueueFlags::UserIntervention;
    if ( nWinStatus & PRINTER_STATUS_OUT_OF_MEMORY )
        nStatus |= PrintQueueFlags::OutOfMemory;
    if ( nWinStatus & PRINTER_STATUS_DOOR_OPEN )
        nStatus |= PrintQueueFlags::DoorOpen;
    if ( nWinStatus & PRINTER_STATUS_SERVER_UNKNOWN )
        nStatus |= PrintQueueFlags::StatusUnknown;
    if ( nWinStatus & PRINTER_STATUS_POWER_SAVE )
        nStatus |= PrintQueueFlags::PowerSave;
    if ( nStatus == PrintQueueFlags::NONE && !(nWinStatus & PRINTER_STATUS_NOT_AVAILABLE) )
        nStatus |= PrintQueueFlags::Ready;
    return nStatus;
}


void WinSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    DWORD           i;
    DWORD           nBytes = 0;
    DWORD           nInfoPrn4 = 0;
    EnumPrintersW( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, nullptr, 0, &nBytes, &nInfoPrn4 );
    if ( nBytes )
    {
        PRINTER_INFO_4W* pWinInfo4 = static_cast<PRINTER_INFO_4W*>(rtl_allocateMemory( nBytes ));
        if ( EnumPrintersW( PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, nullptr, 4, reinterpret_cast<LPBYTE>(pWinInfo4), nBytes, &nBytes, &nInfoPrn4 ) )
        {
            for ( i = 0; i < nInfoPrn4; i++ )
            {
                SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
                pInfo->maPrinterName = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo4[i].pPrinterName) );
                pInfo->mnStatus      = PrintQueueFlags::NONE;
                pInfo->mnJobs        = 0;
                pInfo->mpSysData     = nullptr;
                pList->Add( pInfo );
            }
        }
        rtl_freeMemory( pWinInfo4 );
    }
}

void WinSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* pInfo )
{
    HANDLE hPrinter = nullptr;
    LPWSTR pPrnName = reinterpret_cast<LPWSTR>(const_cast<sal_Unicode*>(pInfo->maPrinterName.getStr()));
    if( OpenPrinterW( pPrnName, &hPrinter, nullptr ) )
    {
        DWORD               nBytes = 0;
        GetPrinterW( hPrinter, 2, nullptr, 0, &nBytes );
        if( nBytes )
        {
            PRINTER_INFO_2W* pWinInfo2 = static_cast<PRINTER_INFO_2W*>(rtl_allocateMemory(nBytes));
            if( GetPrinterW( hPrinter, 2, reinterpret_cast<LPBYTE>(pWinInfo2), nBytes, &nBytes ) )
            {
                if( pWinInfo2->pDriverName )
                    pInfo->maDriver = OUString( reinterpret_cast< const sal_Unicode* >(pWinInfo2->pDriverName) );
                OUString aPortName;
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
    GetDefaultPrinterW( nullptr, &nChars );
    if( nChars )
    {
        LPWSTR  pStr = static_cast<LPWSTR>(rtl_allocateMemory(nChars*sizeof(WCHAR)));
        OUString aDefPrt;
        if( GetDefaultPrinterW( pStr, &nChars ) )
        {
            aDefPrt = OUString(reinterpret_cast<sal_Unicode* >(pStr));
        }
        rtl_freeMemory( pStr );
        if( !aDefPrt.isEmpty() )
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
    DEVMODEW const * pDevMode;
    if ( !pSetupData || !pSetupData->GetDriverData() )
        pDevMode = nullptr;
    else
        pDevMode = SAL_DEVMODE_W( pSetupData );

    return DeviceCapabilitiesW( reinterpret_cast<LPCWSTR>(pPrinter->maDeviceName.getStr()),
                                reinterpret_cast<LPCWSTR>(pPrinter->maPortName.getStr()),
                                nCaps, reinterpret_cast<LPWSTR>(pOutput), pDevMode );
}

static bool ImplTestSalJobSetup( WinSalInfoPrinter* pPrinter,
                                 ImplJobSetup* pSetupData, bool bDelete )
{
    if ( pSetupData && pSetupData->GetDriverData() )
    {
        // signature and size must fit to avoid using
        // JobSetups from a wrong system

        // initialize versions from jobsetup
        // those will be overwritten with driver's version
        DEVMODEW const * pDevModeW = nullptr;
        LONG dmSpecVersion = -1;
        LONG dmDriverVersion = -1;
        SalDriverData const * pSalDriverData = reinterpret_cast<SalDriverData const *>(pSetupData->GetDriverData());
        BYTE const * pDriverData = reinterpret_cast<BYTE const *>(pSalDriverData) + pSalDriverData->mnDriverOffset;
        pDevModeW = reinterpret_cast<DEVMODEW const *>(pDriverData);

        long nSysJobSize = -1;
        if( pPrinter && pDevModeW )
        {
            // just too many driver crashes in that area -> check the dmSpecVersion and dmDriverVersion fields always !!!
            // this prevents using the jobsetup between different Windows versions (eg from XP to 9x) but we
            // can avoid potential driver crashes as their jobsetups are often not compatible
            // #110800#, #111151#, #112381#, #i16580#, #i14173# and perhaps #112375#
            HANDLE hPrn;
            LPWSTR pPrinterNameW = reinterpret_cast<LPWSTR>(const_cast<sal_Unicode*>(pPrinter->maDeviceName.getStr()));
            if ( !OpenPrinterW( pPrinterNameW, &hPrn, nullptr ) )
                return FALSE;

            // #131642# hPrn==HGDI_ERROR even though OpenPrinter() succeeded!
            if( hPrn == HGDI_ERROR )
                return FALSE;

            nSysJobSize = DocumentPropertiesW( nullptr, hPrn,
                                               pPrinterNameW,
                                               nullptr, nullptr, 0 );

            if( nSysJobSize < 0 )
            {
                ClosePrinter( hPrn );
                return FALSE;
            }
            DEVMODEW *pBuffer = static_cast<DEVMODEW*>(_alloca( nSysJobSize ));
            LONG nRet = DocumentPropertiesW( nullptr, hPrn,
                                        pPrinterNameW,
                                        pBuffer, nullptr, DM_OUT_BUFFER );
            if( nRet < 0 )
            {
                ClosePrinter( hPrn );
                return FALSE;
            }

            // the spec version differs between the windows platforms, ie 98,NT,2000/XP
            // this allows us to throw away printer settings from other platforms that might crash a buggy driver
            // we check the driver version as well
            dmSpecVersion = pBuffer->dmSpecVersion;
            dmDriverVersion = pBuffer->dmDriverVersion;

            ClosePrinter( hPrn );
        }
        SalDriverData const * pSetupDriverData = reinterpret_cast<SalDriverData const *>(pSetupData->GetDriverData());
        if ( (pSetupData->GetSystem() == JOBSETUP_SYSTEM_WINDOWS) &&
             (pPrinter->maDriverName == pSetupData->GetDriver()) &&
             (pSetupData->GetDriverDataLen() > sizeof( SalDriverData )) &&
             (long)(pSetupData->GetDriverDataLen() - pSetupDriverData->mnDriverOffset) == nSysJobSize &&
             pSetupDriverData->mnSysSignature == SAL_DRIVERDATA_SYSSIGN )
        {
            if( pDevModeW &&
                (dmSpecVersion == pDevModeW->dmSpecVersion) &&
                (dmDriverVersion == pDevModeW->dmDriverVersion) )
                return TRUE;
        }
        if ( bDelete )
        {
            rtl_freeMemory( const_cast<sal_uInt8*>(pSetupData->GetDriverData()) );
            pSetupData->SetDriverData( nullptr );
            pSetupData->SetDriverDataLen( 0 );
        }
    }

    return FALSE;
}

static bool ImplUpdateSalJobSetup( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData,
                                   bool bIn, WinSalFrame* pVisibleDlgParent )
{
    HANDLE hPrn;
    LPWSTR pPrinterNameW = reinterpret_cast<LPWSTR>(const_cast<sal_Unicode*>(pPrinter->maDeviceName.getStr()));
    if ( !OpenPrinterW( pPrinterNameW, &hPrn, nullptr ) )
        return FALSE;
    // #131642# hPrn==HGDI_ERROR even though OpenPrinter() succeeded!
    if( hPrn == HGDI_ERROR )
        return FALSE;

    LONG            nRet;
    HWND            hWnd = nullptr;
    DWORD           nMode = DM_OUT_BUFFER;
    SalDriverData*  pOutBuffer = nullptr;
    BYTE const *    pInBuffer = nullptr;

    LONG nSysJobSize = DocumentPropertiesW( hWnd, hPrn,
                                       pPrinterNameW,
                                       nullptr, nullptr, 0 );
    if ( nSysJobSize < 0 )
    {
        ClosePrinter( hPrn );
        return FALSE;
    }

    // make Outputbuffer
    const std::size_t nDriverDataLen = sizeof(SalDriverData) + nSysJobSize-1;
    pOutBuffer                  = static_cast<SalDriverData*>(rtl_allocateZeroMemory( nDriverDataLen ));
    pOutBuffer->mnSysSignature  = SAL_DRIVERDATA_SYSSIGN;
    // calculate driver data offset including structure padding
    pOutBuffer->mnDriverOffset  = sal::static_int_cast<sal_uInt16>(
                                    reinterpret_cast<char*>(pOutBuffer->maDriverData) -
                                    reinterpret_cast<char*>(pOutBuffer) );

    // check if we have a suitable input buffer
    if ( bIn && ImplTestSalJobSetup( pPrinter, pSetupData, false ) )
    {
        pInBuffer = pSetupData->GetDriverData() + reinterpret_cast<SalDriverData const *>(pSetupData->GetDriverData())->mnDriverOffset;
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

    BYTE* pOutDevMode = (reinterpret_cast<BYTE*>(pOutBuffer) + pOutBuffer->mnDriverOffset);
    nRet = DocumentPropertiesW( hWnd, hPrn,
                                pPrinterNameW,
                                reinterpret_cast<LPDEVMODEW>(pOutDevMode), reinterpret_cast<LPDEVMODEW>(const_cast<BYTE *>(pInBuffer)), nMode );
    if ( pVisibleDlgParent )
        ImplSalAcquireYieldMutex( nMutexCount );
    ClosePrinter( hPrn );

    if( (nRet < 0) || (pVisibleDlgParent && (nRet == IDCANCEL)) )
    {
        rtl_freeMemory( pOutBuffer );
        return FALSE;
    }

    // fill up string buffers with 0 so they do not influence a JobSetup's memcmp
    if( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmSize >= 64 )
    {
        sal_Int32 nLen = rtl_ustr_getLength( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmDeviceName );
        if ( sal::static_int_cast<size_t>(nLen) < SAL_N_ELEMENTS( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmDeviceName ) )
            memset( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmDeviceName+nLen, 0, sizeof( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmDeviceName )-(nLen*sizeof(sal_Unicode)) );
    }
    if( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmSize >= 166 )
    {
        sal_Int32 nLen = rtl_ustr_getLength( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmFormName );
        if ( sal::static_int_cast<size_t>(nLen) < SAL_N_ELEMENTS( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmFormName ) )
            memset( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmFormName+nLen, 0, sizeof( reinterpret_cast<LPDEVMODEW>(pOutDevMode)->dmFormName )-(nLen*sizeof(sal_Unicode)) );
    }

    // update data
    if ( pSetupData->GetDriverData() )
        rtl_freeMemory( const_cast<sal_uInt8*>(pSetupData->GetDriverData()) );
    pSetupData->SetDriverDataLen( nDriverDataLen );
    pSetupData->SetDriverData(reinterpret_cast<BYTE*>(pOutBuffer));
    pSetupData->SetSystem( JOBSETUP_SYSTEM_WINDOWS );

    return TRUE;
}

static void ImplDevModeToJobSetup( WinSalInfoPrinter* pPrinter, ImplJobSetup* pSetupData, JobSetFlags nFlags )
{
    if ( !pSetupData || !pSetupData->GetDriverData() )
        return;

    DEVMODEW const * pDevModeW = SAL_DEVMODE_W(pSetupData);
    if( pDevModeW == nullptr )
        return;

    // Orientation
    if ( nFlags & JobSetFlags::ORIENTATION )
    {
        if ( pDevModeW->dmOrientation == DMORIENT_PORTRAIT )
            pSetupData->SetOrientation( Orientation::Portrait );
        else if ( pDevModeW->dmOrientation == DMORIENT_LANDSCAPE )
            pSetupData->SetOrientation( Orientation::Landscape );
    }

    // PaperBin
    if ( nFlags & JobSetFlags::PAPERBIN )
    {
        const DWORD nCount = ImplDeviceCaps( pPrinter, DC_BINS, nullptr, pSetupData );

        if ( nCount && (nCount != GDI_ERROR) )
        {
            WORD* pBins = static_cast<WORD*>(rtl_allocateZeroMemory( nCount*sizeof(WORD) ));
            ImplDeviceCaps( pPrinter, DC_BINS, reinterpret_cast<BYTE*>(pBins), pSetupData );
            pSetupData->SetPaperBin( 0 );

            // search the right bin and assign index to mnPaperBin
            for( DWORD i = 0; i < nCount; ++i )
            {
                if( pDevModeW->dmDefaultSource == pBins[ i ] )
                {
                    pSetupData->SetPaperBin( (sal_uInt16)i );
                    break;
                }
            }

            rtl_freeMemory( pBins );
        }
    }

    // PaperSize
    if ( nFlags & JobSetFlags::PAPERSIZE )
    {
        if( (pDevModeW->dmFields & (DM_PAPERWIDTH|DM_PAPERLENGTH)) == (DM_PAPERWIDTH|DM_PAPERLENGTH) )
        {
            pSetupData->SetPaperWidth( pDevModeW->dmPaperWidth*10 );
            pSetupData->SetPaperHeight( pDevModeW->dmPaperLength*10 );
        }
        else
        {
            const DWORD nPaperCount = ImplDeviceCaps( pPrinter, DC_PAPERS, nullptr, pSetupData );
            WORD*   pPapers = nullptr;
            const DWORD nPaperSizeCount = ImplDeviceCaps( pPrinter, DC_PAPERSIZE, nullptr, pSetupData );
            POINT*  pPaperSizes = nullptr;
            if ( nPaperCount && (nPaperCount != GDI_ERROR) )
            {
                pPapers = static_cast<WORD*>(rtl_allocateZeroMemory(nPaperCount*sizeof(WORD)));
                ImplDeviceCaps( pPrinter, DC_PAPERS, reinterpret_cast<BYTE*>(pPapers), pSetupData );
            }
            if ( nPaperSizeCount && (nPaperSizeCount != GDI_ERROR) )
            {
                pPaperSizes = static_cast<POINT*>(rtl_allocateZeroMemory(nPaperSizeCount*sizeof(POINT)));
                ImplDeviceCaps( pPrinter, DC_PAPERSIZE, reinterpret_cast<BYTE*>(pPaperSizes), pSetupData );
            }
            if( nPaperSizeCount == nPaperCount && pPaperSizes && pPapers )
            {
                for( DWORD i = 0; i < nPaperCount; ++i )
                {
                    if( pPapers[ i ] == pDevModeW->dmPaperSize )
                    {
                        pSetupData->SetPaperWidth( pPaperSizes[ i ].x*10 );
                        pSetupData->SetPaperHeight( pPaperSizes[ i ].y*10 );
                        break;
                    }
                }
            }
            if( pPapers )
                rtl_freeMemory( pPapers );
            if( pPaperSizes )
                rtl_freeMemory( pPaperSizes );
        }
        switch( pDevModeW->dmPaperSize )
        {
            case DMPAPER_LETTER:
                pSetupData->SetPaperFormat( PAPER_LETTER );
                break;
            case DMPAPER_TABLOID:
                pSetupData->SetPaperFormat( PAPER_TABLOID );
                break;
            case DMPAPER_LEDGER:
                pSetupData->SetPaperFormat( PAPER_LEDGER );
                break;
            case DMPAPER_LEGAL:
                pSetupData->SetPaperFormat( PAPER_LEGAL );
                break;
            case DMPAPER_STATEMENT:
                pSetupData->SetPaperFormat( PAPER_STATEMENT );
                break;
            case DMPAPER_EXECUTIVE:
                pSetupData->SetPaperFormat( PAPER_EXECUTIVE );
                break;
            case DMPAPER_A3:
                pSetupData->SetPaperFormat( PAPER_A3 );
                break;
            case DMPAPER_A4:
                pSetupData->SetPaperFormat( PAPER_A4 );
                break;
            case DMPAPER_A5:
                pSetupData->SetPaperFormat( PAPER_A5 );
                break;
            //See http://wiki.openoffice.org/wiki/DefaultPaperSize
            //i.e.
            //http://msdn.microsoft.com/en-us/library/dd319099(VS.85).aspx
            //DMPAPER_B4    12  B4 (JIS) 257 x 364 mm
            //http://partners.adobe.com/public/developer/en/ps/5003.PPD_Spec_v4.3.pdf
            //also says that the MS DMPAPER_B4 is JIS, which makes most sense. And
            //matches our Excel filter's belief about the matching XlPaperSize
            //enumeration.

            //http://msdn.microsoft.com/en-us/library/ms776398(VS.85).aspx said
            ////"DMPAPER_B4     12  B4 (JIS) 250 x 354"
            //which is bogus as it's either JIS 257 x 364 or ISO 250 x 353
            //(cmc)
            case DMPAPER_B4:
                pSetupData->SetPaperFormat( PAPER_B4_JIS );
                break;
            case DMPAPER_B5:
                pSetupData->SetPaperFormat( PAPER_B5_JIS );
                break;
            case DMPAPER_QUARTO:
                pSetupData->SetPaperFormat( PAPER_QUARTO );
                break;
            case DMPAPER_10X14:
                pSetupData->SetPaperFormat( PAPER_10x14 );
                break;
            case DMPAPER_NOTE:
                pSetupData->SetPaperFormat( PAPER_LETTER );
                break;
            case DMPAPER_ENV_9:
                pSetupData->SetPaperFormat( PAPER_ENV_9 );
                break;
            case DMPAPER_ENV_10:
                pSetupData->SetPaperFormat( PAPER_ENV_10 );
                break;
            case DMPAPER_ENV_11:
                pSetupData->SetPaperFormat( PAPER_ENV_11 );
                break;
            case DMPAPER_ENV_12:
                pSetupData->SetPaperFormat( PAPER_ENV_12 );
                break;
            case DMPAPER_ENV_14:
                pSetupData->SetPaperFormat( PAPER_ENV_14 );
                break;
            case DMPAPER_CSHEET:
                pSetupData->SetPaperFormat( PAPER_C );
                break;
            case DMPAPER_DSHEET:
                pSetupData->SetPaperFormat( PAPER_D );
                break;
            case DMPAPER_ESHEET:
                pSetupData->SetPaperFormat( PAPER_E );
                break;
            case DMPAPER_ENV_DL:
                pSetupData->SetPaperFormat( PAPER_ENV_DL );
                break;
            case DMPAPER_ENV_C5:
                pSetupData->SetPaperFormat( PAPER_ENV_C5 );
                break;
            case DMPAPER_ENV_C3:
                pSetupData->SetPaperFormat( PAPER_ENV_C3 );
                break;
            case DMPAPER_ENV_C4:
                pSetupData->SetPaperFormat( PAPER_ENV_C4 );
                break;
            case DMPAPER_ENV_C6:
                pSetupData->SetPaperFormat( PAPER_ENV_C6 );
                break;
            case DMPAPER_ENV_C65:
                pSetupData->SetPaperFormat( PAPER_ENV_C65 );
                break;
            case DMPAPER_ENV_ITALY:
                pSetupData->SetPaperFormat( PAPER_ENV_ITALY );
                break;
            case DMPAPER_ENV_MONARCH:
                pSetupData->SetPaperFormat( PAPER_ENV_MONARCH );
                break;
            case DMPAPER_ENV_PERSONAL:
                pSetupData->SetPaperFormat( PAPER_ENV_PERSONAL );
                break;
            case DMPAPER_FANFOLD_US:
                pSetupData->SetPaperFormat( PAPER_FANFOLD_US );
                break;
            case DMPAPER_FANFOLD_STD_GERMAN:
                pSetupData->SetPaperFormat( PAPER_FANFOLD_DE );
                break;
            case DMPAPER_FANFOLD_LGL_GERMAN:
                pSetupData->SetPaperFormat( PAPER_FANFOLD_LEGAL_DE );
                break;
            case DMPAPER_ISO_B4:
                pSetupData->SetPaperFormat( PAPER_B4_ISO );
                break;
            case DMPAPER_JAPANESE_POSTCARD:
                pSetupData->SetPaperFormat( PAPER_POSTCARD_JP );
                break;
            case DMPAPER_9X11:
                pSetupData->SetPaperFormat( PAPER_9x11 );
                break;
            case DMPAPER_10X11:
                pSetupData->SetPaperFormat( PAPER_10x11 );
                break;
            case DMPAPER_15X11:
                pSetupData->SetPaperFormat( PAPER_15x11 );
                break;
            case DMPAPER_ENV_INVITE:
                pSetupData->SetPaperFormat( PAPER_ENV_INVITE );
                break;
            case DMPAPER_A_PLUS:
                pSetupData->SetPaperFormat( PAPER_A_PLUS );
                break;
            case DMPAPER_B_PLUS:
                pSetupData->SetPaperFormat( PAPER_B_PLUS );
                break;
            case DMPAPER_LETTER_PLUS:
                pSetupData->SetPaperFormat( PAPER_LETTER_PLUS );
                break;
            case DMPAPER_A4_PLUS:
                pSetupData->SetPaperFormat( PAPER_A4_PLUS );
                break;
            case DMPAPER_A2:
                pSetupData->SetPaperFormat( PAPER_A2 );
                break;
            case DMPAPER_DBL_JAPANESE_POSTCARD:
                pSetupData->SetPaperFormat( PAPER_DOUBLEPOSTCARD_JP );
                break;
            case DMPAPER_A6:
                pSetupData->SetPaperFormat( PAPER_A6 );
                break;
            case DMPAPER_B6_JIS:
                pSetupData->SetPaperFormat( PAPER_B6_JIS );
                break;
            case DMPAPER_12X11:
                pSetupData->SetPaperFormat( PAPER_12x11 );
                break;
            default:
                pSetupData->SetPaperFormat( PAPER_USER );
                break;
        }
    }

    if( nFlags & JobSetFlags::DUPLEXMODE )
    {
        DuplexMode eDuplex = DuplexMode::Unknown;
        if( (pDevModeW->dmFields & DM_DUPLEX) )
        {
            if( pDevModeW->dmDuplex == DMDUP_SIMPLEX )
                eDuplex = DuplexMode::Off;
            else if( pDevModeW->dmDuplex == DMDUP_VERTICAL )
                eDuplex = DuplexMode::LongEdge;
            else if( pDevModeW->dmDuplex == DMDUP_HORIZONTAL )
                eDuplex = DuplexMode::ShortEdge;
        }
        pSetupData->SetDuplexMode( eDuplex );
    }
}

static void ImplJobSetupToDevMode( WinSalInfoPrinter* pPrinter, const ImplJobSetup* pSetupData, JobSetFlags nFlags )
{
    if ( !pSetupData || !pSetupData->GetDriverData() )
        return;

    DEVMODEW* pDevModeW = const_cast<DEVMODEW *>(SAL_DEVMODE_W(pSetupData));
    if( pDevModeW == nullptr )
        return;

    // Orientation
    if ( nFlags & JobSetFlags::ORIENTATION )
    {
        pDevModeW->dmFields |= DM_ORIENTATION;
        if ( pSetupData->GetOrientation() == Orientation::Portrait )
            pDevModeW->dmOrientation = DMORIENT_PORTRAIT;
        else
            pDevModeW->dmOrientation = DMORIENT_LANDSCAPE;
    }

    // PaperBin
    if ( nFlags & JobSetFlags::PAPERBIN )
    {
        const DWORD nCount = ImplDeviceCaps( pPrinter, DC_BINS, nullptr, pSetupData );

        if ( nCount && (nCount != GDI_ERROR) )
        {
            WORD* pBins = static_cast<WORD*>(rtl_allocateZeroMemory(nCount*sizeof(WORD)));
            ImplDeviceCaps( pPrinter, DC_BINS, reinterpret_cast<BYTE*>(pBins), pSetupData );
            pDevModeW->dmFields |= DM_DEFAULTSOURCE;
            pDevModeW->dmDefaultSource = pBins[ pSetupData->GetPaperBin() ];
            rtl_freeMemory( pBins );
        }
    }

    // PaperSize
    if ( nFlags & JobSetFlags::PAPERSIZE )
    {
        pDevModeW->dmFields        |= DM_PAPERSIZE;
        pDevModeW->dmPaperWidth     = 0;
        pDevModeW->dmPaperLength    = 0;

        switch( pSetupData->GetPaperFormat() )
        {
            case PAPER_A2:
                pDevModeW->dmPaperSize = DMPAPER_A2;
                break;
            case PAPER_A3:
                pDevModeW->dmPaperSize = DMPAPER_A3;
                break;
            case PAPER_A4:
                pDevModeW->dmPaperSize = DMPAPER_A4;
                break;
            case PAPER_A5:
                pDevModeW->dmPaperSize = DMPAPER_A5;
                break;
            case PAPER_B4_ISO:
                pDevModeW->dmPaperSize = DMPAPER_ISO_B4;
                break;
            case PAPER_LETTER:
                pDevModeW->dmPaperSize = DMPAPER_LETTER;
                break;
            case PAPER_LEGAL:
                pDevModeW->dmPaperSize = DMPAPER_LEGAL;
                break;
            case PAPER_TABLOID:
                pDevModeW->dmPaperSize = DMPAPER_TABLOID;
                break;
#if 0
            //http://msdn.microsoft.com/en-us/library/ms776398(VS.85).aspx
            //DMPAPER_ENV_B6 is documented as:
            //"DMPAPER_ENV_B6   35  Envelope B6 176 x 125 mm"
            //which is the wrong way around, it is surely 125 x 176, i.e.
            //compare DMPAPER_ENV_B4 and DMPAPER_ENV_B4 as
            //DMPAPER_ENV_B4    33  Envelope B4 250 x 353 mm
            //DMPAPER_ENV_B5    34  Envelope B5 176 x 250 mm
            case PAPER_B6_ISO:
                pDevModeW->dmPaperSize = DMPAPER_ENV_B6;
                break;
#endif
            case PAPER_ENV_C4:
                pDevModeW->dmPaperSize = DMPAPER_ENV_C4;
                break;
            case PAPER_ENV_C5:
                pDevModeW->dmPaperSize = DMPAPER_ENV_C5;
                break;
            case PAPER_ENV_C6:
                pDevModeW->dmPaperSize = DMPAPER_ENV_C6;
                break;
            case PAPER_ENV_C65:
                pDevModeW->dmPaperSize = DMPAPER_ENV_C65;
                break;
            case PAPER_ENV_DL:
                pDevModeW->dmPaperSize = DMPAPER_ENV_DL;
                break;
            case PAPER_C:
                pDevModeW->dmPaperSize = DMPAPER_CSHEET;
                break;
            case PAPER_D:
                pDevModeW->dmPaperSize = DMPAPER_DSHEET;
                break;
            case PAPER_E:
                pDevModeW->dmPaperSize = DMPAPER_ESHEET;
                break;
            case PAPER_EXECUTIVE:
                pDevModeW->dmPaperSize = DMPAPER_EXECUTIVE;
                break;
            case PAPER_FANFOLD_LEGAL_DE:
                pDevModeW->dmPaperSize = DMPAPER_FANFOLD_LGL_GERMAN;
                break;
            case PAPER_ENV_MONARCH:
                pDevModeW->dmPaperSize = DMPAPER_ENV_MONARCH;
                break;
            case PAPER_ENV_PERSONAL:
                pDevModeW->dmPaperSize = DMPAPER_ENV_PERSONAL;
                break;
            case PAPER_ENV_9:
                pDevModeW->dmPaperSize = DMPAPER_ENV_9;
                break;
            case PAPER_ENV_10:
                pDevModeW->dmPaperSize = DMPAPER_ENV_10;
                break;
            case PAPER_ENV_11:
                pDevModeW->dmPaperSize = DMPAPER_ENV_11;
                break;
            case PAPER_ENV_12:
                pDevModeW->dmPaperSize = DMPAPER_ENV_12;
                break;
            //See the comments on DMPAPER_B4 above
            case PAPER_B4_JIS:
                pDevModeW->dmPaperSize = DMPAPER_B4;
                break;
            case PAPER_B5_JIS:
                pDevModeW->dmPaperSize = DMPAPER_B5;
                break;
            case PAPER_B6_JIS:
                pDevModeW->dmPaperSize = DMPAPER_B6_JIS;
                break;
            case PAPER_LEDGER:
                pDevModeW->dmPaperSize = DMPAPER_LEDGER;
                break;
            case PAPER_STATEMENT:
                pDevModeW->dmPaperSize = DMPAPER_STATEMENT;
                break;
            case PAPER_10x14:
                pDevModeW->dmPaperSize = DMPAPER_10X14;
                break;
            case PAPER_ENV_14:
                pDevModeW->dmPaperSize = DMPAPER_ENV_14;
                break;
            case PAPER_ENV_C3:
                pDevModeW->dmPaperSize = DMPAPER_ENV_C3;
                break;
            case PAPER_ENV_ITALY:
                pDevModeW->dmPaperSize = DMPAPER_ENV_ITALY;
                break;
            case PAPER_FANFOLD_US:
                pDevModeW->dmPaperSize = DMPAPER_FANFOLD_US;
                break;
            case PAPER_FANFOLD_DE:
                pDevModeW->dmPaperSize = DMPAPER_FANFOLD_STD_GERMAN;
                break;
            case PAPER_POSTCARD_JP:
                pDevModeW->dmPaperSize = DMPAPER_JAPANESE_POSTCARD;
                break;
            case PAPER_9x11:
                pDevModeW->dmPaperSize = DMPAPER_9X11;
                break;
            case PAPER_10x11:
                pDevModeW->dmPaperSize = DMPAPER_10X11;
                break;
            case PAPER_15x11:
                pDevModeW->dmPaperSize = DMPAPER_15X11;
                break;
            case PAPER_ENV_INVITE:
                pDevModeW->dmPaperSize = DMPAPER_ENV_INVITE;
                break;
            case PAPER_A_PLUS:
                pDevModeW->dmPaperSize = DMPAPER_A_PLUS;
                break;
            case PAPER_B_PLUS:
                pDevModeW->dmPaperSize = DMPAPER_B_PLUS;
                break;
            case PAPER_LETTER_PLUS:
                pDevModeW->dmPaperSize = DMPAPER_LETTER_PLUS;
                break;
            case PAPER_A4_PLUS:
                pDevModeW->dmPaperSize = DMPAPER_A4_PLUS;
                break;
            case PAPER_DOUBLEPOSTCARD_JP:
                pDevModeW->dmPaperSize = DMPAPER_DBL_JAPANESE_POSTCARD;
                break;
            case PAPER_A6:
                pDevModeW->dmPaperSize = DMPAPER_A6;
                break;
            case PAPER_12x11:
                pDevModeW->dmPaperSize = DMPAPER_12X11;
                break;
            default:
            {
                short   nPaper = 0;
                const DWORD nPaperCount = ImplDeviceCaps( pPrinter, DC_PAPERS, nullptr, pSetupData );
                WORD*   pPapers = nullptr;
                const DWORD nPaperSizeCount = ImplDeviceCaps( pPrinter, DC_PAPERSIZE, nullptr, pSetupData );
                POINT*  pPaperSizes = nullptr;
                DWORD   nLandscapeAngle = ImplDeviceCaps( pPrinter, DC_ORIENTATION, nullptr, pSetupData );
                if ( nPaperCount && (nPaperCount != GDI_ERROR) )
                {
                    pPapers = static_cast<WORD*>(rtl_allocateZeroMemory(nPaperCount*sizeof(WORD)));
                    ImplDeviceCaps( pPrinter, DC_PAPERS, reinterpret_cast<BYTE*>(pPapers), pSetupData );
                }
                if ( nPaperSizeCount && (nPaperSizeCount != GDI_ERROR) )
                {
                    pPaperSizes = static_cast<POINT*>(rtl_allocateZeroMemory(nPaperSizeCount*sizeof(POINT)));
                    ImplDeviceCaps( pPrinter, DC_PAPERSIZE, reinterpret_cast<BYTE*>(pPaperSizes), pSetupData );
                }
                if ( (nPaperSizeCount == nPaperCount) && pPapers && pPaperSizes )
                {
                    PaperInfo aInfo(pSetupData->GetPaperWidth(), pSetupData->GetPaperHeight());
                    // compare paper formats and select a good match
                    for ( DWORD i = 0; i < nPaperCount; ++i )
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
                        PaperInfo aRotatedInfo(pSetupData->GetPaperHeight(), pSetupData->GetPaperWidth());
                        for ( DWORD i = 0; i < nPaperCount; ++i )
                        {
                            if ( aRotatedInfo.sloppyEqual(PaperInfo(pPaperSizes[i].x*10, pPaperSizes[i].y*10)) )
                            {
                                nPaper = pPapers[i];
                                break;
                            }
                        }
                    }

                    if ( nPaper )
                        pDevModeW->dmPaperSize = nPaper;
                }

                if ( !nPaper )
                {
                    pDevModeW->dmFields       |= DM_PAPERLENGTH | DM_PAPERWIDTH;
                    pDevModeW->dmPaperSize     = DMPAPER_USER;
                    pDevModeW->dmPaperWidth    = (short)(pSetupData->GetPaperWidth()/10);
                    pDevModeW->dmPaperLength   = (short)(pSetupData->GetPaperHeight()/10);
                }

                if ( pPapers )
                    rtl_freeMemory(pPapers);
                if ( pPaperSizes )
                    rtl_freeMemory(pPaperSizes);

                break;
            }
        }
    }
    if( (nFlags & JobSetFlags::DUPLEXMODE) )
    {
        switch( pSetupData->GetDuplexMode() )
        {
        case DuplexMode::Off:
            pDevModeW->dmFields |= DM_DUPLEX;
            pDevModeW->dmDuplex = DMDUP_SIMPLEX;
            break;
        case DuplexMode::ShortEdge:
            pDevModeW->dmFields |= DM_DUPLEX;
            pDevModeW->dmDuplex = DMDUP_HORIZONTAL;
            break;
        case DuplexMode::LongEdge:
            pDevModeW->dmFields |= DM_DUPLEX;
            pDevModeW->dmDuplex = DMDUP_VERTICAL;
            break;
        case DuplexMode::Unknown:
            break;
        }
    }
}

static HDC ImplCreateICW_WithCatch( LPWSTR pDriver,
                                    LPCWSTR pDevice,
                                    DEVMODEW const * pDevMode )
{
    HDC hDC = nullptr;
    CATCH_DRIVER_EX_BEGIN;
    hDC = CreateICW( pDriver, pDevice, nullptr, pDevMode );
    CATCH_DRIVER_EX_END_2( "exception in CreateICW" );
    return hDC;
}

static HDC ImplCreateSalPrnIC( WinSalInfoPrinter* pPrinter, const ImplJobSetup* pSetupData )
{
    HDC hDC = nullptr;
    DEVMODEW const * pDevMode;
    if ( pSetupData && pSetupData->GetDriverData() )
        pDevMode = SAL_DEVMODE_W( pSetupData );
    else
        pDevMode = nullptr;
    // #95347 some buggy drivers (eg, OKI) write to those buffers in CreateIC, although declared const - so provide some space
    // pl: does this hold true for Unicode functions ?
    if( pPrinter->maDriverName.getLength() > 2048 || pPrinter->maDeviceName.getLength() > 2048 )
        return nullptr;
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
    WinSalGraphics* pGraphics = new WinSalGraphics(WinSalGraphics::PRINTER, false, nullptr, /* CHECKME */ nullptr);
    pGraphics->SetLayout( SalLayoutFlags::NONE );
    pGraphics->setHDC(hDC);
    pGraphics->InitGraphics();
    return pGraphics;
}

static bool ImplUpdateSalPrnIC( WinSalInfoPrinter* pPrinter, const ImplJobSetup* pSetupData )
{
    HDC hNewDC = ImplCreateSalPrnIC( pPrinter, pSetupData );
    if ( !hNewDC )
        return FALSE;

    if ( pPrinter->mpGraphics )
    {
        pPrinter->mpGraphics->DeInitGraphics();
        DeleteDC( pPrinter->mpGraphics->getHDC() );
        delete pPrinter->mpGraphics;
    }

    pPrinter->mpGraphics = ImplCreateSalPrnGraphics( hNewDC );
    pPrinter->mhDC      = hNewDC;

    return TRUE;
}


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
    ImplTestSalJobSetup( pPrinter, pSetupData, true );

    HDC hDC = ImplCreateSalPrnIC( pPrinter, pSetupData );
    if ( !hDC )
    {
        delete pPrinter;
        return nullptr;
    }

    pPrinter->mpGraphics = ImplCreateSalPrnGraphics( hDC );
    pPrinter->mhDC      = hDC;
    if ( !pSetupData->GetDriverData() )
        ImplUpdateSalJobSetup( pPrinter, pSetupData, false, nullptr );
    ImplDevModeToJobSetup( pPrinter, pSetupData, JobSetFlags::ALL );
    pSetupData->SetSystem( JOBSETUP_SYSTEM_WINDOWS );

    return pPrinter;
}

void WinSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}


WinSalInfoPrinter::WinSalInfoPrinter() :
    mpGraphics( nullptr ),
    mhDC( nullptr ),
    mbGraphics( FALSE )
{
    m_bPapersInit = FALSE;
}

WinSalInfoPrinter::~WinSalInfoPrinter()
{
    if ( mpGraphics )
    {
        mpGraphics->DeInitGraphics();
        DeleteDC( mpGraphics->getHDC() );
        delete mpGraphics;
    }
}

void WinSalInfoPrinter::InitPaperFormats( const ImplJobSetup* pSetupData )
{
    m_aPaperFormats.clear();

    DWORD nCount = ImplDeviceCaps( this, DC_PAPERSIZE, nullptr, pSetupData );
    if( nCount == GDI_ERROR )
        nCount = 0;

    if( nCount )
    {
        POINT* pPaperSizes = static_cast<POINT*>(rtl_allocateZeroMemory(nCount*sizeof(POINT)));
        ImplDeviceCaps( this, DC_PAPERSIZE, reinterpret_cast<BYTE*>(pPaperSizes), pSetupData );

        sal_Unicode* pNamesBuffer = static_cast<sal_Unicode*>(rtl_allocateMemory(nCount*64*sizeof(sal_Unicode)));
        ImplDeviceCaps( this, DC_PAPERNAMES, reinterpret_cast<BYTE*>(pNamesBuffer), pSetupData );
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
    const DWORD nRet = ImplDeviceCaps( this, DC_ORIENTATION, nullptr, pSetupData );

    if( nRet != GDI_ERROR )
        return static_cast<int>(nRet) * 10;
    return 900; // guess
}

SalGraphics* WinSalInfoPrinter::AcquireGraphics()
{
    if ( mbGraphics )
        return nullptr;

    if ( mpGraphics )
        mbGraphics = TRUE;

    return mpGraphics;
}

void WinSalInfoPrinter::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = FALSE;
}

bool WinSalInfoPrinter::Setup( SalFrame* pFrame, ImplJobSetup* pSetupData )
{
    if ( ImplUpdateSalJobSetup( this, pSetupData, true, static_cast<WinSalFrame*>(pFrame) ) )
    {
        ImplDevModeToJobSetup( this, pSetupData, JobSetFlags::ALL );
        return ImplUpdateSalPrnIC( this, pSetupData );
    }

    return FALSE;
}

bool WinSalInfoPrinter::SetPrinterData( ImplJobSetup* pSetupData )
{
    if ( !ImplTestSalJobSetup( this, pSetupData, false ) )
        return FALSE;
    return ImplUpdateSalPrnIC( this, pSetupData );
}

bool WinSalInfoPrinter::SetData( JobSetFlags nFlags, ImplJobSetup* pSetupData )
{
    ImplJobSetupToDevMode( this, pSetupData, nFlags );
    if ( ImplUpdateSalJobSetup( this, pSetupData, true, nullptr ) )
    {
        ImplDevModeToJobSetup( this, pSetupData, nFlags );
        return ImplUpdateSalPrnIC( this, pSetupData );
    }

    return FALSE;
}

sal_uInt16 WinSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* pSetupData )
{
    DWORD nRet = ImplDeviceCaps( this, DC_BINS, nullptr, pSetupData );
    if ( nRet && (nRet != GDI_ERROR) )
        return nRet;
    else
        return 0;
}

OUString WinSalInfoPrinter::GetPaperBinName( const ImplJobSetup* pSetupData, sal_uInt16 nPaperBin )
{
    OUString aPaperBinName;

    DWORD nBins = ImplDeviceCaps( this, DC_BINNAMES, nullptr, pSetupData );
    if ( (nPaperBin < nBins) && (nBins != GDI_ERROR) )
    {
        auto pBuffer = std::unique_ptr<sal_Unicode[]>(new sal_Unicode[nBins*24]);
        DWORD nRet = ImplDeviceCaps( this, DC_BINNAMES, reinterpret_cast<BYTE*>(pBuffer.get()), pSetupData );
        if ( nRet && (nRet != GDI_ERROR) )
            aPaperBinName = OUString( pBuffer.get() + (nPaperBin*24) );
    }

    return aPaperBinName;
}

sal_uInt32 WinSalInfoPrinter::GetCapabilities( const ImplJobSetup* pSetupData, PrinterCapType nType )
{
    DWORD nRet;

    switch ( nType )
    {
        case PrinterCapType::SupportDialog:
            return TRUE;
        case PrinterCapType::Copies:
            nRet = ImplDeviceCaps( this, DC_COPIES, nullptr, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return nRet;
            return 0;
        case PrinterCapType::CollateCopies:
            nRet = ImplDeviceCaps( this, DC_COLLATE, nullptr, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
            {
                nRet = ImplDeviceCaps( this, DC_COPIES, nullptr, pSetupData );
                if ( nRet && (nRet != GDI_ERROR) )
                    return nRet;
            }
            return 0;

        case PrinterCapType::SetOrientation:
            nRet = ImplDeviceCaps( this, DC_ORIENTATION, nullptr, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return TRUE;
            return FALSE;

        case PrinterCapType::SetPaperBin:
            nRet = ImplDeviceCaps( this, DC_BINS, nullptr, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return TRUE;
            return FALSE;

        case PrinterCapType::SetPaperSize:
        case PrinterCapType::SetPaper:
            nRet = ImplDeviceCaps( this, DC_PAPERS, nullptr, pSetupData );
            if ( nRet && (nRet != GDI_ERROR) )
                return TRUE;
            return FALSE;

        default:
            break;
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
    bool        bWhile = TRUE;
    int         i = 0;

    do
    {
        // process messages
        MSG aMsg;
        if ( PeekMessageW( &aMsg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &aMsg );
            DispatchMessageW( &aMsg );

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

static DEVMODEW const * ImplSalSetCopies( DEVMODEW const * pDevMode, sal_uLong nCopies, bool bCollate )
{
    if ( pDevMode && (nCopies > 1) )
    {
        if ( nCopies > 32765 )
            nCopies = 32765;
        sal_uLong nDevSize = pDevMode->dmSize+pDevMode->dmDriverExtra;
        LPDEVMODEW pNewDevMode = static_cast<LPDEVMODEW>(rtl_allocateMemory( nDevSize ));
        memcpy( pNewDevMode, pDevMode, nDevSize );
        pNewDevMode->dmFields |= DM_COPIES;
        pNewDevMode->dmCopies  = (short)(sal_uInt16)nCopies;
        pNewDevMode->dmFields |= DM_COLLATE;
        if ( bCollate )
            pNewDevMode->dmCollate = DMCOLLATE_TRUE;
        else
            pNewDevMode->dmCollate = DMCOLLATE_FALSE;
        return pNewDevMode;
    }
    else
    {
        return pDevMode;
    }
}


WinSalPrinter::WinSalPrinter() :
    mpGraphics( nullptr ),
    mpInfoPrinter( nullptr ),
    mpNextPrinter( nullptr ),
    mhDC( nullptr ),
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
            mpGraphics->DeInitGraphics();
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

bool WinSalPrinter::StartJob( const OUString* pFileName,
                           const OUString& rJobName,
                           const OUString&,
                           sal_uInt32 nCopies,
                           bool bCollate,
                           bool /*bDirect*/,
                           ImplJobSetup* pSetupData )
{
    mnError     = 0;
    mbAbort     = FALSE;
    mnCopies        = nCopies;
    mbCollate   = bCollate;

    DEVMODEW const * pOrgDevModeW = nullptr;
    DEVMODEW const * pDevModeW = nullptr;
    HDC hDC = nullptr;
    if ( pSetupData && pSetupData->GetDriverData() )
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
                     nullptr,
                     pDevModeW );

    if ( pDevModeW != pOrgDevModeW )
        rtl_freeMemory( const_cast<DEVMODEW *>(pDevModeW) );

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

    // As the Telecom Balloon Fax driver tends to send messages repeatedly
    // we try to process first all, and then insert a dummy message
    bool bWhile = TRUE;
    int  i = 0;
    do
    {
        // process messages
        MSG aMsg;
        if ( PeekMessageW( &aMsg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &aMsg );
            DispatchMessageW( &aMsg );

            i++;
            if ( i > 15 )
                bWhile = FALSE;
        }
        else
            bWhile = FALSE;
    }
    while ( bWhile );
    BOOL const ret = PostMessageW(GetSalData()->mpFirstInstance->mhComWnd, SAL_MSG_DUMMY, 0, 0);
    SAL_WARN_IF(0 == ret, "vcl", "ERROR: PostMessage() failed!");

    // bring up a file chooser if printing to file port but no file name given
    OUString aOutFileName;
    if( mpInfoPrinter->maPortName.equalsIgnoreAsciiCase( "FILE:" ) && !(pFileName && !pFileName->isEmpty()) )
    {

        uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference< XFilePicker3 > xFilePicker = FilePicker::createWithMode(xContext, TemplateDescription::FILESAVE_SIMPLE);

        if( xFilePicker->execute() == ExecutableDialogResults::OK )
        {
            Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
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
    aInfo.lpszDocName = rJobName.getStr();
    if ( pFileName || aOutFileName.getLength() )
    {
        if ( (pFileName && !pFileName->isEmpty()) || aOutFileName.getLength() )
        {
            aInfo.lpszOutput = (pFileName && !pFileName->isEmpty()) ? pFileName->getStr() : aOutFileName.getStr();
        }
        else
            aInfo.lpszOutput = L"FILE:";
    }
    else
        aInfo.lpszOutput = nullptr;

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

bool WinSalPrinter::EndJob()
{
    HDC hDC = mhDC;
    if ( isValid() && hDC )
    {
        if ( mpGraphics )
        {
            mpGraphics->DeInitGraphics();
            delete mpGraphics;
            mpGraphics = nullptr;
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
        mhDC = nullptr;
    }

    return TRUE;
}

SalGraphics* WinSalPrinter::StartPage( ImplJobSetup* pSetupData, bool bNewJobData )
{
    if( ! isValid() || mhDC == nullptr )
        return nullptr;

    HDC hDC = mhDC;
    if ( pSetupData && pSetupData->GetDriverData() && bNewJobData )
    {
        DEVMODEW const * pOrgDevModeW;
        DEVMODEW const * pDevModeW;
        pOrgDevModeW = SAL_DEVMODE_W( pSetupData );
        pDevModeW = ImplSalSetCopies( pOrgDevModeW, mnCopies, mbCollate );
        ResetDCW( hDC, pDevModeW );
        if ( pDevModeW != pOrgDevModeW )
            rtl_freeMemory( const_cast<DEVMODEW *>(pDevModeW) );
    }
    volatile int nRet = 0;
    CATCH_DRIVER_EX_BEGIN;
    nRet = ::StartPage( hDC );
    CATCH_DRIVER_EX_END( "exception in StartPage", this );

    if ( nRet <= 0 )
    {
        GetLastError();
        mnError = SAL_PRINTER_ERROR_GENERALERROR;
        return nullptr;
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

void WinSalPrinter::EndPage()
{
    HDC hDC = mhDC;
    if ( hDC && mpGraphics )
    {
        mpGraphics->DeInitGraphics();
        delete mpGraphics;
        mpGraphics = nullptr;
    }

    if( ! isValid() )
        return;

    volatile int nRet = 0;
    CATCH_DRIVER_EX_BEGIN;
    nRet = ::EndPage( hDC );
    CATCH_DRIVER_EX_END( "exception in EndPage", this );

    if ( nRet <= 0 )
    {
        GetLastError();
        mnError = SAL_PRINTER_ERROR_GENERALERROR;
    }
}

sal_uLong WinSalPrinter::GetErrorCode()
{
    return mnError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
