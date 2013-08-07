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


#include "vcl/svapp.hxx"
#include "vcl/timer.hxx"
#include "vcl/printerinfomanager.hxx"

#include "jobset.h"
#include "print.h"
#include "salptype.hxx"
#include "saldatabasic.hxx"

#include "generic/genpspgraphics.h"

#include "headless/svpprn.hxx"
#include "headless/svpinst.hxx"

using namespace psp;

/*
 *  static helpers
 */

static OUString getPdfDir( const PrinterInfo& rInfo )
{
    OUString aDir;
    sal_Int32 nIndex = 0;
    while( nIndex != -1 )
    {
        OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
        if( aToken.startsWith( "pdf=" ) )
        {
            sal_Int32 nPos = 0;
            aDir = aToken.getToken( 1, '=', nPos );
            if( aDir.isEmpty() )
                aDir = OStringToOUString( OString( getenv( "HOME" ) ), osl_getThreadTextEncoding() );
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
    OUString aPaper;
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
        pKey                    = rData.m_pParser->getKey( String( "InputSlot"  ) );
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
        pKey = rData.m_pParser->getKey( String( "Duplex"  ) );
    if( pKey )
        pValue = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        if( pValue->m_aOption.equalsIgnoreAsciiCase( "None" ) ||
            pValue->m_aOption.startsWithIgnoreAsciiCase( "Simplex" )
           )
        {
            pJobSetup->meDuplexMode = DUPLEX_OFF;
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexNoTumble" ) )
        {
            pJobSetup->meDuplexMode = DUPLEX_LONGEDGE;
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexTumble" ) )
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
        pJobSetup->mpDriverData = (sal_uInt8*)pBuffer;
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

SalInfoPrinter* SvpSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup*            pJobSetup )
{
    // create and initialize SalInfoPrinter
    SvpSalInfoPrinter* pPrinter = new SvpSalInfoPrinter();

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
    }


    return pPrinter;
}

void SvpSalInstance::DestroyInfoPrinter( SalInfoPrinter* pPrinter )
{
    delete pPrinter;
}

SalPrinter* SvpSalInstance::CreatePrinter( SalInfoPrinter* pInfoPrinter )
{
    // create and initialize SalPrinter
    SvpSalPrinter* pPrinter = new SvpSalPrinter( pInfoPrinter );
    pPrinter->m_aJobData = static_cast<SvpSalInfoPrinter*>(pInfoPrinter)->m_aJobData;

    return pPrinter;
}

void SvpSalInstance::DestroyPrinter( SalPrinter* pPrinter )
{
    delete pPrinter;
}

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

void SvpSalInstance::DeletePrinterQueueInfo( SalPrinterQueueInfo* pInfo )
{
    delete pInfo;
}

void SvpSalInstance::GetPrinterQueueState( SalPrinterQueueInfo* )
{
}

OUString SvpSalInstance::GetDefaultPrinter()
{
    PrinterInfoManager& rManager( PrinterInfoManager::get() );
    return rManager.getDefaultPrinter();
}

void SvpSalInstance::PostPrintersChanged()
{
    const std::list< SalFrame* >& rList = SvpSalInstance::s_pDefaultInstance->getFrames();
    for( std::list< SalFrame* >::const_iterator it = rList.begin();
         it != rList.end(); ++it )
        SvpSalInstance::s_pDefaultInstance->PostEvent( *it, NULL, SALEVENT_PRINTERCHANGED );
}

GenPspGraphics *SvpSalInstance::CreatePrintGraphics()
{
    return new GenPspGraphics();
}

sal_Bool SvpSalInfoPrinter::Setup( SalFrame*, ImplJobSetup* )
{
    return sal_False;
}

SvpSalPrinter::SvpSalPrinter( SalInfoPrinter* pInfoPrinter )
    : PspSalPrinter( pInfoPrinter )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
