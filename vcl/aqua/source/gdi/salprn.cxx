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


#include <boost/bind.hpp>

#include "officecfg/Office/Common.hxx"

#include "vcl/print.hxx"
#include <sal/macros.h>

#include "aqua/salinst.h"
#include "aqua/salprn.h"
#include "aqua/aquaprintview.h"
#include "aqua/salgdi.h"
#include "aqua/saldata.hxx"

#include "jobset.h"
#include "salptype.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "com/sun/star/uno/Sequence.hxx"

#include <algorithm>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::beans;

using ::rtl::OUString;
using ::rtl::OStringToOUString;

// =======================================================================

AquaSalInfoPrinter::AquaSalInfoPrinter( const SalPrinterQueueInfo& i_rQueue ) :
    mpGraphics( 0 ),
    mbGraphics( false ),
    mbJob( false ),
    mpPrinter( nil ),
    mpPrintInfo( nil ),
    mePageOrientation( ORIENTATION_PORTRAIT ),
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
        mePageOrientation = ([mpPrintInfo orientation] == NSLandscapeOrientation) ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT;
        [mpPrintInfo setOrientation: NSPortraitOrientation];
    }

    mpGraphics = new AquaSalGraphics();

    const int nWidth = 100, nHeight = 100;
    maContextMemory.reset( reinterpret_cast<sal_uInt8*>( rtl_allocateMemory( nWidth * 4 * nHeight ) ),
                           boost::bind( rtl_freeMemory, _1 ) );

    if( maContextMemory )
    {
        mrContext = CGBitmapContextCreate( maContextMemory.get(), nWidth, nHeight, 8, nWidth * 4, GetSalData()->mxRGBSpace, kCGImageAlphaNoneSkipFirst );
        if( mrContext )
            SetupPrinterGraphics( mrContext );
    }
}

// -----------------------------------------------------------------------

AquaSalInfoPrinter::~AquaSalInfoPrinter()
{
    delete mpGraphics;
    if( mpPrintInfo )
        [mpPrintInfo release];
    if( mrContext )
        CFRelease( mrContext );
}

// -----------------------------------------------------------------------

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
            if( mePageOrientation == ORIENTATION_PORTRAIT )
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
            mpGraphics->SetPrinterGraphics( i_rContext, nDPIX, nDPIY, 1.0 );
        }
        else
            OSL_FAIL( "no print info in SetupPrinterGraphics" );
    }
}

// -----------------------------------------------------------------------

SalGraphics* AquaSalInfoPrinter::GetGraphics()
{
    SalGraphics* pGraphics = mbGraphics ? NULL : mpGraphics;
    mbGraphics = true;
    return pGraphics;
}

// -----------------------------------------------------------------------

void AquaSalInfoPrinter::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = false;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalInfoPrinter::Setup( SalFrame*, ImplJobSetup* )
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalInfoPrinter::SetPrinterData( ImplJobSetup* io_pSetupData )
{
    // FIXME: implement driver data
    if( io_pSetupData && io_pSetupData->mpDriverData )
        return SetData( ~0, io_pSetupData );


    sal_Bool bSuccess = sal_True;

    // set system type
    io_pSetupData->mnSystem = JOBSETUP_SYSTEM_MAC;

    // get paper format
    if( mpPrintInfo )
    {
        NSSize aPaperSize = [mpPrintInfo paperSize];
        double width = aPaperSize.width, height = aPaperSize.height;
        // set paper
        PaperInfo aInfo( PtTo10Mu( width ), PtTo10Mu( height ) );
        aInfo.doSloppyFit();
        io_pSetupData->mePaperFormat = aInfo.getPaper();
        if( io_pSetupData->mePaperFormat == PAPER_USER )
        {
            io_pSetupData->mnPaperWidth = PtTo10Mu( width );
            io_pSetupData->mnPaperHeight = PtTo10Mu( height );
        }
        else
        {
            io_pSetupData->mnPaperWidth = 0;
            io_pSetupData->mnPaperHeight = 0;
        }

        // set orientation
        io_pSetupData->meOrientation = mePageOrientation;

        io_pSetupData->mnPaperBin = 0;
        io_pSetupData->mpDriverData = reinterpret_cast<sal_uInt8*>(rtl_allocateMemory( 4 ));
        io_pSetupData->mnDriverDataLen = 4;
    }
    else
        bSuccess = sal_False;


    return bSuccess;
}

// -----------------------------------------------------------------------

void AquaSalInfoPrinter::setPaperSize( long i_nWidth, long i_nHeight, Orientation i_eSetOrientation )
{

    Orientation ePaperOrientation = ORIENTATION_PORTRAIT;
    const PaperInfo* pPaper = matchPaper( i_nWidth, i_nHeight, ePaperOrientation );

    if( pPaper )
    {
        NSString* pPaperName = [CreateNSString( rtl::OStringToOUString(PaperInfo::toPSName(pPaper->getPaper()), RTL_TEXTENCODING_ASCII_US) ) autorelease];
        [mpPrintInfo setPaperName: pPaperName];
    }
    else if( i_nWidth > 0 && i_nHeight > 0 )
    {
        NSSize aPaperSize = { TenMuToPt(i_nWidth), TenMuToPt(i_nHeight) };
        [mpPrintInfo setPaperSize: aPaperSize];
    }
    // this seems counterintuitive
    mePageOrientation = i_eSetOrientation;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalInfoPrinter::SetData( sal_uLong i_nFlags, ImplJobSetup* io_pSetupData )
{
    if( ! io_pSetupData || io_pSetupData->mnSystem != JOBSETUP_SYSTEM_MAC )
        return sal_False;


    if( mpPrintInfo )
    {
        if( (i_nFlags & SAL_JOBSET_ORIENTATION) != 0 )
            mePageOrientation = io_pSetupData->meOrientation;

        if( (i_nFlags & SAL_JOBSET_PAPERSIZE) !=  0)
        {
            // set paper format
            long width = 21000, height = 29700;
            if( io_pSetupData->mePaperFormat == PAPER_USER )
            {
                // #i101108# sanity check
                if( io_pSetupData->mnPaperWidth && io_pSetupData->mnPaperHeight )
                {
                    width = io_pSetupData->mnPaperWidth;
                    height = io_pSetupData->mnPaperHeight;
                }
            }
            else
            {
                PaperInfo aInfo( io_pSetupData->mePaperFormat );
                width = aInfo.getWidth();
                height = aInfo.getHeight();
            }

            setPaperSize( width, height, mePageOrientation );
        }
    }

    return mpPrintInfo != nil;
}

// -----------------------------------------------------------------------

sal_uLong AquaSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* )
{
    return 0;
}

// -----------------------------------------------------------------------

rtl::OUString AquaSalInfoPrinter::GetPaperBinName( const ImplJobSetup*, sal_uLong )
{
    return rtl::OUString();
}

// -----------------------------------------------------------------------

sal_uLong AquaSalInfoPrinter::GetCapabilities( const ImplJobSetup*, sal_uInt16 i_nType )
{
    switch( i_nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return 0;
        case PRINTER_CAPABILITIES_COPIES:
            return 0xffff;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
            return 0xffff;
        case PRINTER_CAPABILITIES_SETORIENTATION:
            return 1;
        case PRINTER_CAPABILITIES_SETDUPLEX:
            return 0;
        case PRINTER_CAPABILITIES_SETPAPERBIN:
            return 0;
        case PRINTER_CAPABILITIES_SETPAPERSIZE:
            return 1;
        case PRINTER_CAPABILITIES_SETPAPER:
            return 1;
        case PRINTER_CAPABILITIES_EXTERNALDIALOG:
            return officecfg::Office::Common::Misc::UseSystemPrintDialog::get()
                ? 1 : 0;
        case PRINTER_CAPABILITIES_PDF:
            return 1;
        case PRINTER_CAPABILITIES_USEPULLMODEL:
            return 1;
        default: break;
    };
    return 0;
}

// -----------------------------------------------------------------------

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

        if( mePageOrientation == ORIENTATION_LANDSCAPE )
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

sal_Bool AquaSalInfoPrinter::StartJob( const rtl::OUString* i_pFileName,
                                   const rtl::OUString& i_rJobName,
                                   const rtl::OUString& /*i_rAppName*/,
                                   ImplJobSetup* i_pSetupData,
                                   vcl::PrinterController& i_rController
                                   )
{
    if( mbJob )
        return sal_False;

    sal_Bool bSuccess = sal_False;
    bool bWasAborted = false;
    AquaSalInstance* pInst = GetSalData()->mpFirstInstance;
    PrintAccessoryViewState aAccViewState;
    sal_Int32 nAllPages = 0;

    // reset IsLastPage
    i_rController.setLastPage( sal_False );

    // update job data
    if( i_pSetupData )
        SetData( ~0, i_pSetupData );

    // do we want a progress panel ?
    sal_Bool bShowProgressPanel = sal_True;
    beans::PropertyValue* pMonitor = i_rController.getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MonitorVisible" ) ) );
    if( pMonitor )
        pMonitor->Value >>= bShowProgressPanel;
    if( ! i_rController.isShowDialogs() )
        bShowProgressPanel = sal_False;

    // possibly create one job for collated output
    sal_Bool bSinglePrintJobs = sal_False;
    beans::PropertyValue* pSingleValue = i_rController.getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PrintCollateAsSingleJobs" ) ) );
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
            boost::shared_ptr<Printer> pPrinter( i_rController.getPrinter() );
            pPrinter->SetMapMode( MapMode( MAP_100TH_MM ) );
            pPrinter->SetPaperSizeUser( aCurSize, true );

            // create view
            NSView* pPrintView = [[AquaPrintView alloc] initWithController: &i_rController withInfoPrinter: this];

            NSMutableDictionary* pPrintDict = [mpPrintInfo dictionary];

            // set filename
            if( i_pFileName )
            {
                [mpPrintInfo setJobDisposition: NSPrintSaveJob];
                NSString* pPath = CreateNSString( *i_pFileName );
                [pPrintDict setObject: pPath forKey: NSPrintSavePath];
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

                if( bShowPanel && mnCurPageRangeStart == 0 && nCurJob == 0) // only the first range of pages (in the first job) gets the accesory view
                    pReleaseAfterUse = [AquaPrintAccessoryView setupPrinterPanel: pPrintOperation withController: &i_rController withState: &aAccViewState];

                bSuccess = sal_True;
                mbJob = true;
                pInst->startedPrintJob();
                [pPrintOperation runOperation];
                pInst->endedPrintJob();
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
    // so we need to call XRenderadble::render one last time with IsLastPage = sal_True
    i_rController.setLastPage( sal_True );
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

// -----------------------------------------------------------------------

sal_Bool AquaSalInfoPrinter::EndJob()
{
    mnStartPageOffsetX = mnStartPageOffsetY = 0;
    mbJob = false;
    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalInfoPrinter::AbortJob()
{
    mbJob = false;

    // FIXME: implementation
    return sal_False;
}

// -----------------------------------------------------------------------

SalGraphics* AquaSalInfoPrinter::StartPage( ImplJobSetup* i_pSetupData, sal_Bool i_bNewJobData )
{
    if( i_bNewJobData && i_pSetupData )
        SetPrinterData( i_pSetupData );

    CGContextRef rContext = reinterpret_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);

    SetupPrinterGraphics( rContext );

    return mpGraphics;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalInfoPrinter::EndPage()
{
    mpGraphics->InvalidateContext();
    return sal_True;
}

// -----------------------------------------------------------------------

sal_uLong AquaSalInfoPrinter::GetErrorCode() const
{
    return 0;
}

// =======================================================================

AquaSalPrinter::AquaSalPrinter( AquaSalInfoPrinter* i_pInfoPrinter ) :
    mpInfoPrinter( i_pInfoPrinter )
{
}

// -----------------------------------------------------------------------

AquaSalPrinter::~AquaSalPrinter()
{
}

// -----------------------------------------------------------------------

sal_Bool AquaSalPrinter::StartJob( const rtl::OUString* i_pFileName,
                               const rtl::OUString& i_rJobName,
                               const rtl::OUString& i_rAppName,
                               ImplJobSetup* i_pSetupData,
                               vcl::PrinterController& i_rController )
{
    return mpInfoPrinter->StartJob( i_pFileName, i_rJobName, i_rAppName, i_pSetupData, i_rController );
}

// -----------------------------------------------------------------------

sal_Bool AquaSalPrinter::StartJob( const rtl::OUString* /*i_pFileName*/,
                               const rtl::OUString& /*i_rJobName*/,
                               const rtl::OUString& /*i_rAppName*/,
                               sal_uLong /*i_nCopies*/,
                               bool /*i_bCollate*/,
                               bool /*i_bDirect*/,
                               ImplJobSetup* )
{
    OSL_FAIL( "should never be called" );
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool AquaSalPrinter::EndJob()
{
    return mpInfoPrinter->EndJob();
}

// -----------------------------------------------------------------------

sal_Bool AquaSalPrinter::AbortJob()
{
    return mpInfoPrinter->AbortJob();
}

// -----------------------------------------------------------------------

SalGraphics* AquaSalPrinter::StartPage( ImplJobSetup* i_pSetupData, sal_Bool i_bNewJobData )
{
    return mpInfoPrinter->StartPage( i_pSetupData, i_bNewJobData );
}

// -----------------------------------------------------------------------

sal_Bool AquaSalPrinter::EndPage()
{
    return mpInfoPrinter->EndPage();
}

// -----------------------------------------------------------------------

sal_uLong AquaSalPrinter::GetErrorCode()
{
    return mpInfoPrinter->GetErrorCode();
}

void AquaSalInfoPrinter::InitPaperFormats( const ImplJobSetup* )
{
    m_aPaperFormats.clear();
    m_bPapersInit = true;

    if( mpPrinter )
    {
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
                    rtl::OString aPaperName( [pPaper UTF8String] );
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
    }
}

const PaperInfo* AquaSalInfoPrinter::matchPaper( long i_nWidth, long i_nHeight, Orientation& o_rOrientation ) const
{
    if( ! m_bPapersInit )
        const_cast<AquaSalInfoPrinter*>(this)->InitPaperFormats( NULL );

    const PaperInfo* pMatch = NULL;
    o_rOrientation = ORIENTATION_PORTRAIT;
    for( int n = 0; n < 2 ; n++ )
    {
        for( size_t i = 0; i < m_aPaperFormats.size(); i++ )
        {
            if( abs( m_aPaperFormats[i].getWidth() - i_nWidth ) < 50 &&
                abs( m_aPaperFormats[i].getHeight() - i_nHeight ) < 50 )
            {
                pMatch = &m_aPaperFormats[i];
                return pMatch;
            }
        }
        o_rOrientation = ORIENTATION_LANDSCAPE;
        std::swap( i_nWidth, i_nHeight );
    }
    return pMatch;
}

int AquaSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* )
{
    return 900;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
