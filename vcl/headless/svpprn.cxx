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

#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include "printerinfomanager.hxx"

#include "jobset.h"
#include "print.h"
#include "salptype.hxx"
#include "saldatabasic.hxx"

#include "unx/genpspgraphics.h"

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

static void copyJobDataToJobSetup( ImplJobSetup* pJobSetup, JobData& rData )
{
    pJobSetup->SetOrientation( rData.m_eOrientation == orientation::Landscape ? Orientation::Landscape : Orientation::Portrait );

    // copy page size
    OUString aPaper;
    int width, height;

    rData.m_aContext.getPageSize( aPaper, width, height );
    pJobSetup->SetPaperFormat( PaperInfo::fromPSName(OUStringToOString( aPaper, RTL_TEXTENCODING_ISO_8859_1 )) );
    pJobSetup->SetPaperWidth( 0 );
    pJobSetup->SetPaperHeight( 0 );
    if( pJobSetup->GetPaperFormat() == PAPER_USER )
    {
        // transform to 100dth mm
        width               = PtTo10Mu( width );
        height              = PtTo10Mu( height );

        if( rData.m_eOrientation == psp::orientation::Portrait )
        {
            pJobSetup->SetPaperWidth( width );
            pJobSetup->SetPaperHeight( height );
        }
        else
        {
            pJobSetup->SetPaperWidth( height );
            pJobSetup->SetPaperHeight( width );
        }
    }

    // copy input slot
    const PPDKey* pKey = nullptr;
    const PPDValue* pValue = nullptr;

    pJobSetup->SetPaperBin( 0xffff );
    if( rData.m_pParser )
        pKey                    = rData.m_pParser->getKey( OUString( "InputSlot"  ) );
    if( pKey )
        pValue                  = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        int nPaperBin;
        for( nPaperBin = 0;
             pValue != pKey->getValue( nPaperBin ) &&
                 nPaperBin < pKey->countValues();
             nPaperBin++ );
        pJobSetup->SetPaperBin(
            (nPaperBin == pKey->countValues()
             || pValue == pKey->getDefaultValue())
            ? 0xffff : nPaperBin);
    }

    // copy duplex
    pKey = nullptr;
    pValue = nullptr;

    pJobSetup->SetDuplexMode( DuplexMode::Unknown );
    if( rData.m_pParser )
        pKey = rData.m_pParser->getKey( OUString( "Duplex"  ) );
    if( pKey )
        pValue = rData.m_aContext.getValue( pKey );
    if( pKey && pValue )
    {
        if( pValue->m_aOption.equalsIgnoreAsciiCase( "None" ) ||
            pValue->m_aOption.startsWithIgnoreAsciiCase( "Simplex" )
           )
        {
            pJobSetup->SetDuplexMode( DuplexMode::Off );
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexNoTumble" ) )
        {
            pJobSetup->SetDuplexMode( DuplexMode::LongEdge );
        }
        else if( pValue->m_aOption.equalsIgnoreAsciiCase( "DuplexTumble" ) )
        {
            pJobSetup->SetDuplexMode( DuplexMode::ShortEdge );
        }
    }

    // copy the whole context
    if( pJobSetup->GetDriverData() )
        rtl_freeMemory( const_cast<sal_uInt8*>(pJobSetup->GetDriverData()) );

    sal_uInt32 nBytes;
    void* pBuffer = nullptr;
    if( rData.getStreamBuffer( pBuffer, nBytes ) )
    {
        pJobSetup->SetDriverDataLen( nBytes );
        pJobSetup->SetDriverData( static_cast<sal_uInt8*>(pBuffer) );
    }
    else
    {
        pJobSetup->SetDriverDataLen( 0 );
        pJobSetup->SetDriverData( nullptr );
    }
}

// SalInstance

SalInfoPrinter* SvpSalInstance::CreateInfoPrinter( SalPrinterQueueInfo* pQueueInfo,
                                                   ImplJobSetup*        pJobSetup )
{
    // create and initialize SalInfoPrinter
    SvpSalInfoPrinter* pPrinter = new SvpSalInfoPrinter;

    if( pJobSetup )
    {
        PrinterInfoManager& rManager( PrinterInfoManager::get() );
        PrinterInfo aInfo( rManager.getPrinterInfo( pQueueInfo->maPrinterName ) );
        pPrinter->m_aJobData = aInfo;
        pPrinter->m_aPrinterGfx.Init( pPrinter->m_aJobData );

        if( pJobSetup->GetDriverData() )
            JobData::constructFromStreamBuffer( pJobSetup->GetDriverData(),
                                                pJobSetup->GetDriverDataLen(), aInfo );

        pJobSetup->SetSystem( JOBSETUP_SYSTEM_UNIX );
        pJobSetup->SetPrinterName( pQueueInfo->maPrinterName );
        pJobSetup->SetDriver( aInfo.m_aDriverName );
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
    ::std::vector< OUString > aPrinters;
    rManager.listPrinters( aPrinters );

    for( ::std::vector< OUString >::iterator it = aPrinters.begin(); it != aPrinters.end(); ++it )
    {
        const PrinterInfo& rInfo( rManager.getPrinterInfo( *it ) );
        // create new entry
        SalPrinterQueueInfo* pInfo = new SalPrinterQueueInfo;
        pInfo->maPrinterName    = *it;
        pInfo->maDriver         = rInfo.m_aDriverName;
        pInfo->maLocation       = rInfo.m_aLocation;
        pInfo->maComment        = rInfo.m_aComment;
        pInfo->mpSysData        = nullptr;

        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aToken( rInfo.m_aFeatures.getToken( 0, ',', nIndex ) );
            if( aToken.startsWith( "pdf=" ) )
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
    SvpSalInstance *pInst = SvpSalInstance::s_pDefaultInstance;
    for (auto pSalFrame : pInst->getFrames() )
        pInst->PostEvent( const_cast<SalFrame*>( pSalFrame ), nullptr, SalEvent::PrinterChanged );
}

GenPspGraphics *SvpSalInstance::CreatePrintGraphics()
{
    return new GenPspGraphics();
}

bool SvpSalInfoPrinter::Setup( SalFrame*, ImplJobSetup* )
{
    return false;
}

SvpSalPrinter::SvpSalPrinter( SalInfoPrinter* pInfoPrinter )
    : PspSalPrinter( pInfoPrinter )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
