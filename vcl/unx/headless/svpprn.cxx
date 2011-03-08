/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_vcl.hxx"

#include "vcl/svapp.hxx"
#include "vcl/jobset.h"
#include "vcl/print.h"
#include "vcl/salptype.hxx"
#include "vcl/timer.hxx"
#include "vcl/printerinfomanager.hxx"

#include "svpprn.hxx"
#include "svppspgraphics.hxx"
#include "svpinst.hxx"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

using namespace psp;

using ::rtl::OUString;
using ::rtl::OUStringToOString;

/*
 *  static helpers
 */

static String getPdfDir( const PrinterInfo& rInfo )
{
    String aDir;
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( ! aToken.compareToAscii( "pdf=", 4 ) )
        {
            sal_Int32 nPos = 0;
            aDir = aToken.getToken( 1, '=', nPos );
            if( ! aDir.Len() )
                aDir = String( ByteString( getenv( "HOME" ) ), osl_getThreadTextEncoding() );
            break;
        }
    }
    return aDir;
}

inline int PtTo10Mu( int nPoints ) { return (int)((((double)nPoints)*35.27777778)+0.5); }

inline int TenMuToPt( int nUnits ) { return (int)((((double)nUnits)/35.27777778)+0.5); }

static void copyJobDataToJobSetup( ImplJobSetup* pJobSetup, JobData& rData )
{
    pJobSetup->meOrientation    = (Orientation)(rData.m_eOrientation == orientation::Landscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT);

    // copy page size
    String aPaper;
    int width, height;

    rData.m_aContext.getPageSize( aPaper, width, height );
    pJobSetup->mePaperFormat    = PaperInfo::fromPSName(OUStringToOString( aPaper, RTL_TEXTENCODING_ISO_8859_1 ));
    pJobSetup->mnPaperWidth     = 0;
    pJobSetup->mnPaperHeight    = 0;
    if( pJobSetup->mePaperFormat == PAPER_USER )
    {
        // transform to 100dth mm
        width               = PtTo10Mu( width );
        height              = PtTo10Mu( height );

        if( rData.m_eOrientation == psp::orientation::Portrait )
        {
            pJobSetup->mnPaperWidth = width;
            pJobSetup->mnPaperHeight= height;
        }
        else
        {
            pJobSetup->mnPaperWidth = height;
            pJobSetup->mnPaperHeight= width;
        }
    }

    // copy input slot
    const PPDKey* pKey = NULL;
    const PPDValue* pValue = NULL;

    pJobSetup->mnPaperBin = 0xffff;
    if( rData.m_pParser )
        pKey                    = rData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "InputSlot" ) ) );
    if( pKey )
        pValue                  = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        for( pJobSetup->mnPaperBin = 0;
             pValue != pKey->getValue( pJobSetup->mnPaperBin ) &&
                 pJobSetup->mnPaperBin < pKey->countValues();
             pJobSetup->mnPaperBin++ )
            ;
        if( pJobSetup->mnPaperBin >= pKey->countValues() || pValue == pKey->getDefaultValue() )
            pJobSetup->mnPaperBin = 0xffff;
    }

    // copy duplex
    pKey = NULL;
    pValue = NULL;

    pJobSetup->meDuplexMode = DUPLEX_UNKNOWN;
    if( rData.m_pParser )
        pKey = rData.m_pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Duplex" ) ) );
    if( pKey )
        pValue = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        if( pValue->m_aOption.EqualsIgnoreCaseAscii( "None" ) ||
            pValue->m_aOption.EqualsIgnoreCaseAscii( "Simplex", 0, 7 )
           )
        {
            pJobSetup->meDuplexMode = DUPLEX_OFF;
        }
        else if( pValue->m_aOption.EqualsIgnoreCaseAscii( "DuplexNoTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_LONGEDGE;
        }
        else if( pValue->m_aOption.EqualsIgnoreCaseAscii( "DuplexTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_SHORTEDGE;
        }
    }

    // copy the whole context
    if( pJobSetup->mpDriverData )
        rtl_freeMemory( pJobSetup->mpDriverData );

    int nBytes;
    void* pBuffer = NULL;
    if( rData.getStreamBuffer( pBuffer, nBytes ) )
    {
        pJobSetup->mnDriverDataLen = nBytes;
        pJobSetup->mpDriverData = (BYTE*)pBuffer;
    }
    else
    {
        pJobSetup->mnDriverDataLen = 0;
        pJobSetup->mpDriverData = NULL;
    }
}

/*
 *  SalInstance
 */

// -----------------------------------------------------------------------

SalInfoPrinter* SvpSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup*            pJobSetup )
{
    // create and initialize SalInfoPrinter
    SvpSalInfoPrinter* pPrinter = new SvpSalInfoPrinter;

    if( pJobSetup )
    {
        PrinterInfoManager& rManager( PrinterInfoManager::get() );
        PrinterInfo aInfo( rManager.getPrinterInfo( pQueueInfo->maPrinterName ) );
        pPrinter->m_aJobData = aInfo;
        pPrinter->m_aPrinterGfx.Init( pPrinter->m_aJobData );

        if( pJobSetup->mpDriverData )
            JobData::constructFromStreamBuffer( pJobSetup->mpDriverData, pJobSetup->mnDriverDataLen, aInfo );

        pJobSetup->mnSystem         = JOBSETUP_SYSTEM_UNIX;
        pJobSetup->maPrinterName    = pQueueInfo->maPrinterName;
        pJobSetup->maDriver         = aInfo.m_aDriverName;
        copyJobDataToJobSetup( pJobSetup, aInfo );

        // set/clear backwards compatibility flag
        bool bStrictSO52Compatibility = false;
        boost::unordered_map<rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator compat_it =
            pJobSetup->maValueMap.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "StrictSO52Compatibility" ) ) );
        if( compat_it != pJobSetup->maValueMap.end() )
        {
            if( compat_it->second.equalsIgnoreAsciiCaseAscii( "true" ) )
                bStrictSO52Compatibility = true;
        }
        pPrinter->m_aPrinterGfx.setStrictSO52Compatibility( bStrictSO52Compatibility );
    }


    return pPrinter;
}

// -----------------------------------------------------------------------

void SvpSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

SalPrinter* SvpSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    // create and initialize SalPrinter
    SvpSalPrinter* pPrinter = new SvpSalPrinter( pInfoPrinter );
    pPrinter->m_aJobData = static_cast<SvpSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return pPrinter;
}

// -----------------------------------------------------------------------

void SvpSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

// -----------------------------------------------------------------------

void SvpSalInstance::GetPrinterQueueInfo( ImplPrnQueueList* pList )
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    static const char* pNoSyncDetection = getenv( "SAL_DISABLE_SYNCHRONOUS_PRINTER_DETECTION" );
    if( ! pNoSyncDetection || ! *pNoSyncDetection )
    {
        // #i62663# synchronize possible asynchronouse printer detection now
        rManager.checkPrintersChanged( true );
    }
    ::std::list< OUString > aPrinters;
    rManager.listPrinters( aPrinters );

    for( ::std::list< OUString >::iterator it = aPrinters.begin(); it != aPrinters.end(); ++it )
    {
        const PrinterInfo& rInfo( rManager.getPrinterInfo( *it ) );
        // Neuen Eintrag anlegen
        SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
        pInfo->maPrinterName    = *it;
        pInfo->maDriver         = rInfo.m_aDriverName;
        pInfo->maLocation       = rInfo.m_aLocation;
        pInfo->maComment        = rInfo.m_aComment;
        pInfo->mpSysData        = NULL;

        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            String aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
            if( aToken.CompareToAscii( "pdf=", 4 ) == COMPARE_EQUAL )
            {
                pInfo->maLocation = getPdfDir( rInfo );
                break;
            }
        }

        pList->Add( pInfo );
    }
}

// -----------------------------------------------------------------------

void SvpSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

// -----------------------------------------------------------------------

void SvpSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
}

// -----------------------------------------------------------------------

String SvpSalInstance::GetDefaultPrinter()
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    return rManager.getDefaultPrinter();
}

// -----------------------------------------------------------------------

BOOL SvpSalInfoPrinter::Setup( SalFrame*, ImplJobSetup* )
{
    return FALSE;
}

/*
 *  svp::PrinterUpdate
 */

namespace svp
{
    class PrinterUpdate
    {
        static Timer*                       pPrinterUpdateTimer;
        static int                          nActiveJobs;

        static void doUpdate();
        DECL_STATIC_LINK( PrinterUpdate, UpdateTimerHdl, void* );
    public:
        static void update();
        static void jobStarted() { nActiveJobs++; }
        static void jobEnded();
    };
}

Timer* svp::PrinterUpdate::pPrinterUpdateTimer = NULL;
int svp::PrinterUpdate::nActiveJobs = 0;

void svp::PrinterUpdate::doUpdate()
{
    ::psp::PrinterInfoManager& rManager( ::psp::PrinterInfoManager::get() );
    if( rManager.checkPrintersChanged( false ) && SvpSalInstance::s_pDefaultInstance )
    {
        const std::list< SalFrame* >& rList = SvpSalInstance::s_pDefaultInstance->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rList.begin();
             it != rList.end(); ++it )
             SvpSalInstance::s_pDefaultInstance->PostEvent( *it, NULL, SALEVENT_PRINTERCHANGED );
    }
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( svp::PrinterUpdate, UpdateTimerHdl, void*, )
{
    if( nActiveJobs < 1 )
    {
        doUpdate();
        delete pPrinterUpdateTimer;
        pPrinterUpdateTimer = NULL;
    }
    else
        pPrinterUpdateTimer->Start();

    return 0;
}

// -----------------------------------------------------------------------

void svp::PrinterUpdate::update()
{
    if( Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        return;

    static bool bOnce = false;
    if( ! bOnce )
    {
        bOnce = true;
        // start background printer detection
        psp::PrinterInfoManager::get();
        return;
    }

    if( nActiveJobs < 1 )
        doUpdate();
    else if( ! pPrinterUpdateTimer )
    {
        pPrinterUpdateTimer = new Timer();
        pPrinterUpdateTimer->SetTimeout( 500 );
        pPrinterUpdateTimer->SetTimeoutHdl( STATIC_LINK( NULL, svp::PrinterUpdate, UpdateTimerHdl ) );
        pPrinterUpdateTimer->Start();
    }
}

void SvpSalInstance::updatePrinterUpdate()
{
    svp::PrinterUpdate::update();
}

// -----------------------------------------------------------------------

void svp::PrinterUpdate::jobEnded()
{
    nActiveJobs--;
    if( nActiveJobs < 1 )
    {
        if( pPrinterUpdateTimer )
        {
            pPrinterUpdateTimer->Stop();
            delete pPrinterUpdateTimer;
            pPrinterUpdateTimer = NULL;
            doUpdate();
        }
    }
}

void SvpSalInstance::jobStartedPrinterUpdate()
{
    svp::PrinterUpdate::jobStarted();
}

void SvpSalInstance::jobEndedPrinterUpdate()
{
    svp::PrinterUpdate::jobEnded();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
