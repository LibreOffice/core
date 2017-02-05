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

#include "officecfg/Office/Common.hxx"

#include <vcl/print.hxx>
#include <sal/macros.h>

#include "osx/salinst.h"
#include "osx/salprn.h"
#include "osx/printview.h"
#include "quartz/salgdi.h"
#include "osx/saldata.hxx"
#include "quartz/utils.h"

#include "jobset.h"
#include "salptype.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "com/sun/star/uno/Sequence.hxx"

#include <algorithm>
#include <cstdlib>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::beans;

AquaSalInfoPrinter::AquaSalInfoPrinter( const SalPrinterQueueInfo& i_rQueue ) :
    mpGraphics( nullptr ),
    mbGraphics( false ),
    mbJob( false ),
    mpPrinter( nil ),
    mpPrintInfo( nil ),
    mePageOrientation( Orientation::Portrait ),
    mnStartPageOffsetX( 0 ),
    mnStartPageOffsetY( 0 ),
    mnCurPageRangeStart( 0 ),
    mnCurPageRangeCount( 0 )
{
    NSString* pStr = CreateNSString( i_rQueue.maPrinterName );
    mpPrinter = [NSPrinter printerWithName: pStr];
    [pStr release];

    NSPrintInfo* pShared = [NSPrintInfo sharedPrintInfo];
    if( pShared )
    {
        mpPrintInfo = [pShared copy];
        [mpPrintInfo setPrinter: mpPrinter];
#if MACOSX_SDK_VERSION >= 1090
        mePageOrientation = ([mpPrintInfo orientation] == NSPaperOrientationPortrait) ? Orientation::Landscape : Orientation::Portrait;
        [mpPrintInfo setOrientation: NSPaperOrientationPortrait];
#else
        mePageOrientation = ([mpPrintInfo orientation] == NSLandscapeOrientation) ? Orientation::Landscape : Orientation::Portrait;
        [mpPrintInfo setOrientation: NSPortraitOrientation];
#endif
    }

    mpGraphics = new AquaSalGraphics();

    const int nWidth = 100, nHeight = 100;
    mpContextMemory.reset(static_cast<sal_uInt8*>(rtl_allocateMemory(nWidth * 4 * nHeight)),
                          &rtl_freeMemory);

    if (mpContextMemory)
    {
        mrContext = CGBitmapContextCreate(mpContextMemory.get(),
                nWidth, nHeight, 8, nWidth * 4,
                GetSalData()->mxRGBSpace, kCGImageAlphaNoneSkipFirst);
        if( mrContext )
            SetupPrinterGraphics( mrContext );
    }
}

AquaSalInfoPrinter::~AquaSalInfoPrinter()
{
    delete mpGraphics;
    if( mpPrintInfo )
        [mpPrintInfo release];
    if( mrContext )
        CFRelease( mrContext );
}

void AquaSalInfoPrinter::SetupPrinterGraphics( CGContextRef i_rContext ) const
{
    if( mpGraphics )
    {
        if( mpPrintInfo )
        {
            // FIXME: get printer resolution
            long nDPIX = 720, nDPIY = 720;
            NSSize aPaperSize = [mpPrintInfo paperSize];

            NSRect aImageRect = [mpPrintInfo imageablePageBounds];
            if( mePageOrientation == Orientation::Portrait )
            {
                // move mirrored CTM back into paper
                double dX = 0, dY = aPaperSize.height;
                // move CTM to reflect imageable area
                dX += aImageRect.origin.x;
                dY -= aPaperSize.height - aImageRect.size.height - aImageRect.origin.y;
                CGContextTranslateCTM( i_rContext, dX + mnStartPageOffsetX, dY - mnStartPageOffsetY );
                // scale to be top/down and reflect our "virtual" DPI
                CGContextScaleCTM( i_rContext, 72.0/double(nDPIX), -(72.0/double(nDPIY)) );
            }
            else
            {
                // move CTM to reflect imageable area
                double dX = aImageRect.origin.x, dY = aPaperSize.height - aImageRect.size.height - aImageRect.origin.y;
                CGContextTranslateCTM( i_rContext, -dX, -dY );
                // turn by 90 degree
                CGContextRotateCTM( i_rContext, M_PI/2 );
                // move turned CTM back into paper
                dX = aPaperSize.height;
                dY = -aPaperSize.width;
                CGContextTranslateCTM( i_rContext, dX + mnStartPageOffsetY, dY - mnStartPageOffsetX );
                // scale to be top/down and reflect our "virtual" DPI
                CGContextScaleCTM( i_rContext, -(72.0/double(nDPIY)), (72.0/double(nDPIX)) );
            }
            mpGraphics->SetPrinterGraphics( i_rContext, nDPIX, nDPIY );
        }
        else
            OSL_FAIL( "no print info in SetupPrinterGraphics" );
    }
}

SalGraphics* AquaSalInfoPrinter::AcquireGraphics()
{
    SalGraphics* pGraphics = mbGraphics ? nullptr : mpGraphics;
    mbGraphics = true;
    return pGraphics;
}

void AquaSalInfoPrinter::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = false;
}

bool AquaSalInfoPrinter::Setup( SalFrame*, ImplJobSetup* )
{
    return false;
}

bool AquaSalInfoPrinter::SetPrinterData( ImplJobSetup* io_pSetupData )
{
    // FIXME: implement driver data
    if( io_pSetupData && io_pSetupData->GetDriverData() )
        return SetData( JobSetFlags::ALL, io_pSetupData );

    bool bSuccess = true;

    // set system type
    io_pSetupData->SetSystem( JOBSETUP_SYSTEM_MAC );

    // get paper format
    if( mpPrintInfo )
    {
        NSSize aPaperSize = [mpPrintInfo paperSize];
        double width = aPaperSize.width, height = aPaperSize.height;
        // set paper
        PaperInfo aInfo( PtTo10Mu( width ), PtTo10Mu( height ) );
        aInfo.doSloppyFit();
        io_pSetupData->SetPaperFormat( aInfo.getPaper() );
        if( io_pSetupData->GetPaperFormat() == PAPER_USER )
        {
            io_pSetupData->SetPaperWidth( PtTo10Mu( width ) );
            io_pSetupData->SetPaperHeight( PtTo10Mu( height ) );
        }
        else
        {
            io_pSetupData->SetPaperWidth( 0 );
            io_pSetupData->SetPaperHeight( 0 );
        }

        // set orientation
        io_pSetupData->SetOrientation( mePageOrientation );

        io_pSetupData->SetPaperBin( 0 );
        io_pSetupData->SetDriverData( static_cast<sal_uInt8*>(rtl_allocateMemory( 4 )) );
        io_pSetupData->SetDriverDataLen( 4 );
    }
    else
        bSuccess = false;

    return bSuccess;
}

void AquaSalInfoPrinter::setPaperSize( long i_nWidth, long i_nHeight, Orientation i_eSetOrientation )
{

    Orientation ePaperOrientation = Orientation::Portrait;
    const PaperInfo* pPaper = matchPaper( i_nWidth, i_nHeight, ePaperOrientation );

    if( pPaper )
    {
        NSString* pPaperName = [CreateNSString( OStringToOUString(PaperInfo::toPSName(pPaper->getPaper()), RTL_TEXTENCODING_ASCII_US) ) autorelease];
        [mpPrintInfo setPaperName: pPaperName];
    }
    else if( i_nWidth > 0 && i_nHeight > 0 )
    {
        NSSize aPaperSize = { static_cast<CGFloat>(TenMuToPt(i_nWidth)), static_cast<CGFloat>(TenMuToPt(i_nHeight)) };
        [mpPrintInfo setPaperSize: aPaperSize];
    }
    // this seems counterintuitive
    mePageOrientation = i_eSetOrientation;
}

bool AquaSalInfoPrinter::SetData( JobSetFlags i_nFlags, ImplJobSetup* io_pSetupData )
{
    if( ! io_pSetupData || io_pSetupData->GetSystem() != JOBSETUP_SYSTEM_MAC )
        return false;

    if( mpPrintInfo )
    {
        if( i_nFlags & JobSetFlags::ORIENTATION )
            mePageOrientation = io_pSetupData->GetOrientation();

        if( i_nFlags & JobSetFlags::PAPERSIZE )
        {
            // set paper format
            long width = 21000, height = 29700;
            if( io_pSetupData->GetPaperFormat() == PAPER_USER )
            {
                // #i101108# sanity check
                if( io_pSetupData->GetPaperWidth() && io_pSetupData->GetPaperHeight() )
                {
                    width = io_pSetupData->GetPaperWidth();
                    height = io_pSetupData->GetPaperHeight();
                }
            }
            else
            {
                PaperInfo aInfo( io_pSetupData->GetPaperFormat() );
                width = aInfo.getWidth();
                height = aInfo.getHeight();
            }

            setPaperSize( width, height, mePageOrientation );
        }
    }

    return mpPrintInfo != nil;
}

sal_uInt16 AquaSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* )
{
    return 0;
}

OUString AquaSalInfoPrinter::GetPaperBinName( const ImplJobSetup*, sal_uInt16 )
{
    return OUString();
}

sal_uInt32 AquaSalInfoPrinter::GetCapabilities( const ImplJobSetup*, PrinterCapType i_nType )
{
    switch( i_nType )
    {
        case PrinterCapType::SupportDialog:
            return 0;
        case PrinterCapType::Copies:
            return 0xffff;
        case PrinterCapType::CollateCopies:
            return 0xffff;
        case PrinterCapType::SetOrientation:
            return 1;
        case PrinterCapType::SetDuplex:
            return 0;
        case PrinterCapType::SetPaperBin:
            return 0;
        case PrinterCapType::SetPaperSize:
            return 1;
        case PrinterCapType::SetPaper:
            return 1;
        case PrinterCapType::ExternalDialog:
            return officecfg::Office::Common::Misc::UseSystemPrintDialog::get()
                ? 1 : 0;
        case PrinterCapType::PDF:
            return 1;
        case PrinterCapType::UsePullModel:
            return 1;
        default: break;
    }
    return 0;
}

void AquaSalInfoPrinter::GetPageInfo( const ImplJobSetup*,
                                  long& o_rOutWidth, long& o_rOutHeight,
                                  long& o_rPageOffX, long& o_rPageOffY,
                                  long& o_rPageWidth, long& o_rPageHeight )
{
    if( mpPrintInfo )
    {
        sal_Int32 nDPIX = 72, nDPIY = 72;
        mpGraphics->GetResolution( nDPIX, nDPIY );
        const double fXScaling = static_cast<double>(nDPIX)/72.0,
                     fYScaling = static_cast<double>(nDPIY)/72.0;

        NSSize aPaperSize = [mpPrintInfo paperSize];
        o_rPageWidth  = static_cast<long>( double(aPaperSize.width) * fXScaling );
        o_rPageHeight = static_cast<long>( double(aPaperSize.height) * fYScaling );

        NSRect aImageRect = [mpPrintInfo imageablePageBounds];
        o_rPageOffX   = static_cast<long>( aImageRect.origin.x * fXScaling );
        o_rPageOffY   = static_cast<long>( (aPaperSize.height - aImageRect.size.height - aImageRect.origin.y) * fYScaling );
        o_rOutWidth   = static_cast<long>( aImageRect.size.width * fXScaling );
        o_rOutHeight  = static_cast<long>( aImageRect.size.height * fYScaling );

        if( mePageOrientation == Orientation::Landscape )
        {
            std::swap( o_rOutWidth, o_rOutHeight );
            std::swap( o_rPageWidth, o_rPageHeight );
            std::swap( o_rPageOffX, o_rPageOffY );
        }
    }
}

static Size getPageSize( vcl::PrinterController& i_rController, sal_Int32 i_nPage )
{
    Size aPageSize;
    uno::Sequence< PropertyValue > aPageParms( i_rController.getPageParameters( i_nPage ) );
    for( sal_Int32 nProperty = 0, nPropertyCount = aPageParms.getLength(); nProperty < nPropertyCount; ++nProperty )
    {
        if ( aPageParms[ nProperty ].Name == "PageSize" )
        {
            awt::Size aSize;
            aPageParms[ nProperty].Value >>= aSize;
            aPageSize.Width() = aSize.Width;
            aPageSize.Height() = aSize.Height;
            break;
        }
    }
    return aPageSize;
}

bool AquaSalInfoPrinter::StartJob( const OUString* i_pFileName,
                                   const OUString& i_rJobName,
                                   const OUString& /*i_rAppName*/,
                                   ImplJobSetup* i_pSetupData,
                                   vcl::PrinterController& i_rController
                                   )
{
    if( mbJob )
        return false;

    bool bSuccess = false;
    bool bWasAborted = false;
    AquaSalInstance* pInst = GetSalData()->mpFirstInstance;
    PrintAccessoryViewState aAccViewState;
    sal_Int32 nAllPages = 0;

    // reset IsLastPage
    i_rController.setLastPage( false );

    // update job data
    if( i_pSetupData )
        SetData( JobSetFlags::ALL, i_pSetupData );

    // do we want a progress panel ?
    bool bShowProgressPanel = true;
    beans::PropertyValue* pMonitor = i_rController.getValue( OUString( "MonitorVisible" ) );
    if( pMonitor )
        pMonitor->Value >>= bShowProgressPanel;
    if( ! i_rController.isShowDialogs() )
        bShowProgressPanel = false;

    // possibly create one job for collated output
    bool bSinglePrintJobs = false;
    beans::PropertyValue* pSingleValue = i_rController.getValue( OUString( "PrintCollateAsSingleJobs" ) );
    if( pSingleValue )
    {
        pSingleValue->Value >>= bSinglePrintJobs;
    }

    // FIXME: jobStarted() should be done after the print dialog has ended (if there is one)
    // how do I know when that might be ?
    i_rController.jobStarted();

    int nCopies = i_rController.getPrinter()->GetCopyCount();
    int nJobs = 1;
    if( bSinglePrintJobs )
    {
        nJobs = nCopies;
        nCopies = 1;
    }

    for( int nCurJob = 0; nCurJob < nJobs; nCurJob++ )
    {
        aAccViewState.bNeedRestart = true;
        do
        {
            if( aAccViewState.bNeedRestart )
            {
                mnCurPageRangeStart = 0;
                mnCurPageRangeCount = 0;
                nAllPages = i_rController.getFilteredPageCount();
            }

            aAccViewState.bNeedRestart = false;

            Size aCurSize( 21000, 29700 );
            if( nAllPages > 0 )
            {
                mnCurPageRangeCount = 1;
                aCurSize = getPageSize( i_rController, mnCurPageRangeStart );
                Size aNextSize( aCurSize );

                // print pages up to a different size
                while( mnCurPageRangeCount + mnCurPageRangeStart < nAllPages )
                {
                    aNextSize = getPageSize( i_rController, mnCurPageRangeStart + mnCurPageRangeCount );
                    if( aCurSize == aNextSize // same page size
                        ||
                        (aCurSize.Width() == aNextSize.Height() && aCurSize.Height() == aNextSize.Width()) // same size, but different orientation
                        )
                    {
                        mnCurPageRangeCount++;
                    }
                    else
                        break;
                }
            }
            else
                mnCurPageRangeCount = 0;

            // now for the current run
            mnStartPageOffsetX = mnStartPageOffsetY = 0;
            // setup the paper size and orientation
            // do this on our associated Printer object, since that is
            // out interface to the applications which occasionally rely on the paper
            // information (e.g. brochure printing scales to the found paper size)
            // also SetPaperSizeUser has the advantage that we can share a
            // platform independent paper matching algorithm
            VclPtr<Printer> pPrinter( i_rController.getPrinter() );
            pPrinter->SetMapMode( MapMode( MapUnit::Map100thMM ) );
            pPrinter->SetPaperSizeUser( aCurSize, true );

            // create view
            NSView* pPrintView = [[AquaPrintView alloc] initWithController: &i_rController withInfoPrinter: this];

            NSMutableDictionary* pPrintDict = [mpPrintInfo dictionary];

            // set filename
            if( i_pFileName )
            {
                [mpPrintInfo setJobDisposition: NSPrintSaveJob];
                NSString* pPath = CreateNSString( *i_pFileName );
                [pPrintDict setObject:[NSURL fileURLWithPath:pPath] forKey:NSPrintJobSavingURL];
                [pPath release];
            }

            [pPrintDict setObject: [[NSNumber numberWithInt: nCopies] autorelease] forKey: NSPrintCopies];
            if( nCopies > 1 )
                [pPrintDict setObject: [[NSNumber numberWithBool: pPrinter->IsCollateCopy()] autorelease] forKey: NSPrintMustCollate];
            [pPrintDict setObject: [[NSNumber numberWithBool: YES] autorelease] forKey: NSPrintDetailedErrorReporting];
            [pPrintDict setObject: [[NSNumber numberWithInt: 1] autorelease] forKey: NSPrintFirstPage];
            // #i103253# weird: for some reason, autoreleasing the value below like the others above
            // leads do a double free malloc error. Why this value should behave differently from all the others
            // is a mystery.
            [pPrintDict setObject: [NSNumber numberWithInt: mnCurPageRangeCount] forKey: NSPrintLastPage];

            // create print operation
            NSPrintOperation* pPrintOperation = [NSPrintOperation printOperationWithView: pPrintView printInfo: mpPrintInfo];

            if( pPrintOperation )
            {
                NSObject* pReleaseAfterUse = nil;
                bool bShowPanel = !i_rController.isDirectPrint()
                    && (officecfg::Office::Common::Misc::UseSystemPrintDialog::
                        get())
                    && i_rController.isShowDialogs();
                [pPrintOperation setShowsPrintPanel: bShowPanel ? YES : NO ];
                [pPrintOperation setShowsProgressPanel: bShowProgressPanel ? YES : NO];

                // set job title (since MacOSX 10.5)
                if( [pPrintOperation respondsToSelector: @selector(setJobTitle:)] )
                    [pPrintOperation performSelector: @selector(setJobTitle:) withObject: [CreateNSString( i_rJobName ) autorelease]];

                if( bShowPanel && mnCurPageRangeStart == 0 && nCurJob == 0) // only the first range of pages (in the first job) gets the accessory view
                    pReleaseAfterUse = [AquaPrintAccessoryView setupPrinterPanel: pPrintOperation withController: &i_rController withState: &aAccViewState];

                bSuccess = true;
                mbJob = true;
                pInst->startedPrintJob();
                BOOL wasSuccessful = [pPrintOperation runOperation];
                pInst->endedPrintJob();
                bSuccess = wasSuccessful;
                bWasAborted = [[[pPrintOperation printInfo] jobDisposition] compare: NSPrintCancelJob] == NSOrderedSame;
                mbJob = false;
                if( pReleaseAfterUse )
                    [pReleaseAfterUse release];
            }

            mnCurPageRangeStart += mnCurPageRangeCount;
            mnCurPageRangeCount = 1;
        } while( aAccViewState.bNeedRestart || mnCurPageRangeStart + mnCurPageRangeCount < nAllPages );
    }

    // inform application that it can release its data
    // this is awkward, but the XRenderable interface has no method for this,
    // so we need to call XRenderadble::render one last time with IsLastPage = true
    i_rController.setLastPage( true );
    GDIMetaFile aPageFile;
    if( mrContext )
        SetupPrinterGraphics( mrContext );
    i_rController.getFilteredPageFile( 0, aPageFile );

    i_rController.setJobState( bWasAborted
                             ? view::PrintableState_JOB_ABORTED
                             : view::PrintableState_JOB_SPOOLED );

    mnCurPageRangeStart = mnCurPageRangeCount = 0;

    return bSuccess;
}

bool AquaSalInfoPrinter::EndJob()
{
    mnStartPageOffsetX = mnStartPageOffsetY = 0;
    mbJob = false;
    return true;
}

bool AquaSalInfoPrinter::AbortJob()
{
    mbJob = false;

    // FIXME: implementation
    return false;
}

SalGraphics* AquaSalInfoPrinter::StartPage( ImplJobSetup* i_pSetupData, bool i_bNewJobData )
{
    if( i_bNewJobData && i_pSetupData )
        SetPrinterData( i_pSetupData );

    CGContextRef rContext = static_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);

    SetupPrinterGraphics( rContext );

    return mpGraphics;
}

bool AquaSalInfoPrinter::EndPage()
{
    mpGraphics->InvalidateContext();
    return true;
}

sal_uLong AquaSalInfoPrinter::GetErrorCode()
{
    return 0;
}

AquaSalPrinter::AquaSalPrinter( AquaSalInfoPrinter* i_pInfoPrinter ) :
    mpInfoPrinter( i_pInfoPrinter )
{
}

AquaSalPrinter::~AquaSalPrinter()
{
}

bool AquaSalPrinter::StartJob( const OUString* i_pFileName,
                               const OUString& i_rJobName,
                               const OUString& i_rAppName,
                               ImplJobSetup* i_pSetupData,
                               vcl::PrinterController& i_rController )
{
    return mpInfoPrinter->StartJob( i_pFileName, i_rJobName, i_rAppName, i_pSetupData, i_rController );
}

bool AquaSalPrinter::StartJob( const OUString* /*i_pFileName*/,
                               const OUString& /*i_rJobName*/,
                               const OUString& /*i_rAppName*/,
                               sal_uInt32 /*i_nCopies*/,
                               bool /*i_bCollate*/,
                               bool /*i_bDirect*/,
                               ImplJobSetup* )
{
    OSL_FAIL( "should never be called" );
    return false;
}

bool AquaSalPrinter::EndJob()
{
    return mpInfoPrinter->EndJob();
}

SalGraphics* AquaSalPrinter::StartPage( ImplJobSetup* i_pSetupData, bool i_bNewJobData )
{
    return mpInfoPrinter->StartPage( i_pSetupData, i_bNewJobData );
}

void AquaSalPrinter::EndPage()
{
    mpInfoPrinter->EndPage();
}

sal_uLong AquaSalPrinter::GetErrorCode()
{
    return AquaSalInfoPrinter::GetErrorCode();
}

void AquaSalInfoPrinter::InitPaperFormats( const ImplJobSetup* )
{
    m_aPaperFormats.clear();
    m_bPapersInit = true;

    if( mpPrinter )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
            //TODO: 10.9 statusForTable:, stringListForKey:inTable:
        if( [mpPrinter statusForTable: @"PPD"] == NSPrinterTableOK )
        {
            NSArray* pPaperNames = [mpPrinter stringListForKey: @"PageSize" inTable: @"PPD"];
            if( pPaperNames )
            {
                unsigned int nPapers = [pPaperNames count];
                for( unsigned int i = 0; i < nPapers; i++ )
                {
                    NSString* pPaper = [pPaperNames objectAtIndex: i];
                    // first try to match the name
                    OString aPaperName( [pPaper UTF8String] );
                    Paper ePaper = PaperInfo::fromPSName( aPaperName );
                    if( ePaper != PAPER_USER )
                    {
                        m_aPaperFormats.push_back( PaperInfo( ePaper ) );
                    }
                    else
                    {
                        NSSize aPaperSize = [mpPrinter pageSizeForPaper: pPaper];
                        if( aPaperSize.width > 0 && aPaperSize.height > 0 )
                        {
                            PaperInfo aInfo( PtTo10Mu( aPaperSize.width ),
                                             PtTo10Mu( aPaperSize.height ) );
                            if( aInfo.getPaper() == PAPER_USER )
                                aInfo.doSloppyFit();
                            m_aPaperFormats.push_back( aInfo );
                        }
                    }
                }
            }
        }
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }
}

const PaperInfo* AquaSalInfoPrinter::matchPaper( long i_nWidth, long i_nHeight, Orientation& o_rOrientation ) const
{
    if( ! m_bPapersInit )
        const_cast<AquaSalInfoPrinter*>(this)->InitPaperFormats( nullptr );

    const PaperInfo* pMatch = nullptr;
    o_rOrientation = Orientation::Portrait;
    for( int n = 0; n < 2 ; n++ )
    {
        for( size_t i = 0; i < m_aPaperFormats.size(); i++ )
        {
            if( std::abs( m_aPaperFormats[i].getWidth() - i_nWidth ) < 50 &&
                std::abs( m_aPaperFormats[i].getHeight() - i_nHeight ) < 50 )
            {
                pMatch = &m_aPaperFormats[i];
                return pMatch;
            }
        }
        o_rOrientation = Orientation::Landscape;
        std::swap( i_nWidth, i_nHeight );
    }
    return pMatch;
}

int AquaSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* )
{
    return 900;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
