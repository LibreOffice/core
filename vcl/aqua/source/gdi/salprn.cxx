/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salprn.cxx,v $
 * $Revision: 1.15 $
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

#include "salinst.h"
#include "salprn.h"
#include "aquaprintview.h"
#include "salgdi.h"
#include "saldata.hxx"
#include "vcl/jobset.h"
#include "vcl/salptype.hxx"
#include "vcl/impprn.hxx"
#include "vcl/print.hxx"
#include "vcl/unohelp.hxx"

#include <boost/bind.hpp>

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
using namespace rtl;
using namespace vcl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

// =======================================================================

AquaSalInfoPrinter::AquaSalInfoPrinter( const SalPrinterQueueInfo& i_rQueue ) :
    mpGraphics( 0 ),
    mbGraphics( false ),
    mbJob( false ),
    mpPrinter( nil ),
    mpPrintInfo( nil ),
    mePageOrientation( ORIENTATION_PORTRAIT ),
    mnStartPageOffsetX( 0 ),
    mnStartPageOffsetY( 0 )
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
    #if 0
    // FIXME: verify that NSPrintInfo releases the printer
    // else we have a leak here
    if( mpPrinter )
        [mpPrinter release];
    #endif
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

            if( mePageOrientation == ORIENTATION_PORTRAIT )
            {
                double dX = 0, dY = aPaperSize.height;
                dX += [mpPrintInfo leftMargin];
                dY -= [mpPrintInfo topMargin];
                CGContextTranslateCTM( i_rContext, dX + mnStartPageOffsetX, dY - mnStartPageOffsetY );
                CGContextScaleCTM( i_rContext, 0.1, -0.1 );
            }
            else
            {
                CGContextRotateCTM( i_rContext, M_PI/2 );
                double dX = aPaperSize.height, dY = -aPaperSize.width;
                dY += [mpPrintInfo topMargin];
                dX -= [mpPrintInfo rightMargin];

                CGContextTranslateCTM( i_rContext, dX + mnStartPageOffsetY, dY - mnStartPageOffsetX );
                CGContextScaleCTM( i_rContext, -0.1, 0.1 );
            }
            mpGraphics->SetPrinterGraphics( i_rContext, nDPIX, nDPIY, 1.0 );
        }
        else
            DBG_ERROR( "no print info in SetupPrinterGraphics" );
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

BOOL AquaSalInfoPrinter::Setup( SalFrame* i_pFrame, ImplJobSetup* i_pSetupData )
{
    return FALSE;
}

// -----------------------------------------------------------------------

static struct PaperSizeEntry
{
    double fWidth;
    double fHeight;
    Paper        nPaper;
} aPaperSizes[] =
{
    { 842, 1191, PAPER_A3 },
    { 595,  842, PAPER_A4 },
    { 420,  595, PAPER_A5 },
    { 612,  792, PAPER_LETTER },
    { 612, 1008, PAPER_LEGAL },
    { 728, 1032, PAPER_B4 },
    { 516,  729, PAPER_B5 },
    { 792, 1224, PAPER_TABLOID }
};

static bool getPaperSize( double o_fWidth, double o_fHeight, const Paper i_ePaper )
{
    for(unsigned int i = 0; i < sizeof(aPaperSizes)/sizeof(aPaperSizes[0]); i++ )
    {
        if( aPaperSizes[i].nPaper == i_ePaper )
        {
            o_fWidth = aPaperSizes[i].fWidth;
            o_fHeight = aPaperSizes[i].fHeight;
            return true;
        }
    }
    return false;
}

static Paper recognizePaper( double i_fWidth, double i_fHeight )
{
    Paper aPaper = PAPER_USER;
    sal_uInt64 nPaperDesc = 1000000*sal_uInt64(i_fWidth) + sal_uInt64(i_fHeight);
    switch( nPaperDesc )
    {
    case 842001191: aPaper = PAPER_A3;      break;
    case 595000842: aPaper = PAPER_A4;      break;
    case 420000595: aPaper = PAPER_A5;      break;
    case 612000792: aPaper = PAPER_LETTER;  break;
    case 728001032: aPaper = PAPER_B4;      break;
    case 516000729: aPaper = PAPER_B5;      break;
    case 612001008: aPaper = PAPER_LEGAL;   break;
    case 792001224: aPaper = PAPER_TABLOID; break;
    default:
        aPaper = PAPER_USER;
        break;
    }

    if( aPaper == PAPER_USER )
    {
        // search with fuzz factor
        for( unsigned int i = 0; i < sizeof(aPaperSizes)/sizeof(aPaperSizes[0]); i++ )
        {
            double w = (i_fWidth > aPaperSizes[i].fWidth) ? i_fWidth - aPaperSizes[i].fWidth : aPaperSizes[i].fWidth - i_fWidth;
            double h = (i_fHeight > aPaperSizes[i].fHeight) ? i_fHeight - aPaperSizes[i].fHeight : aPaperSizes[i].fHeight - i_fHeight;
            if( w < 3 && h < 3 )
            {
                aPaper = aPaperSizes[i].nPaper;
                break;
            }
        }
    }

    return aPaper;
}

BOOL AquaSalInfoPrinter::SetPrinterData( ImplJobSetup* io_pSetupData )
{
    // FIXME: implement driver data
    if( io_pSetupData && io_pSetupData->mpDriverData )
        return SetData( ~0, io_pSetupData );


    BOOL bSuccess = TRUE;

    // set system type
    io_pSetupData->mnSystem = JOBSETUP_SYSTEM_MAC;

    // get paper format
    if( mpPrintInfo )
    {
        NSSize aPaperSize = [mpPrintInfo paperSize];
        double width = aPaperSize.width, height = aPaperSize.height;
        // set paper
        io_pSetupData->mePaperFormat = recognizePaper( width, height );
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
        io_pSetupData->mpDriverData = reinterpret_cast<BYTE*>(rtl_allocateMemory( 4 ));
        io_pSetupData->mnDriverDataLen = 4;
    }
    else
        bSuccess = FALSE;


    return bSuccess;
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::SetData( ULONG i_nFlags, ImplJobSetup* io_pSetupData )
{
    if( ! io_pSetupData || io_pSetupData->mnSystem != JOBSETUP_SYSTEM_MAC )
        return FALSE;


    if( mpPrintInfo )
    {
        if( (i_nFlags & SAL_JOBSET_PAPERSIZE) !=  0)
        {
            // set paper format
            double width = 0, height = 0;
            if( io_pSetupData->mePaperFormat == PAPER_USER )
            {
                width = TenMuToPt( io_pSetupData->mnPaperWidth );
                height = TenMuToPt( io_pSetupData->mnPaperHeight );
            }
            else
                getPaperSize( width, height, io_pSetupData->mePaperFormat );

            if( width > 0 && height > 0 )
            {
                NSSize aPaperSize = { width, height };
                [mpPrintInfo setPaperSize: aPaperSize];
            }
        }

        if( (i_nFlags & SAL_JOBSET_ORIENTATION) != 0 )
            mePageOrientation = io_pSetupData->meOrientation;
    }

    return mpPrintInfo != nil;
}

// -----------------------------------------------------------------------

ULONG AquaSalInfoPrinter::GetPaperBinCount( const ImplJobSetup* i_pSetupData )
{
    return 0;
}

// -----------------------------------------------------------------------

XubString AquaSalInfoPrinter::GetPaperBinName( const ImplJobSetup* i_pSetupData, ULONG i_nPaperBin )
{
    return XubString();
}

// -----------------------------------------------------------------------

static bool getUseNativeDialog()
{
    bool bNative = true;
    try
    {
        // get service provider
        Reference< XMultiServiceFactory > xSMgr( unohelper::GetMultiServiceFactory() );
        // create configuration hierachical access name
        if( xSMgr.is() )
        {
            try
            {
                Reference< XMultiServiceFactory > xConfigProvider(
                    Reference< XMultiServiceFactory >(
                        xSMgr->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.configuration.ConfigurationProvider" ))),
                        UNO_QUERY )
                    );
                if( xConfigProvider.is() )
                {
                    Sequence< Any > aArgs(1);
                    PropertyValue aVal;
                    aVal.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
                    aVal.Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common/Misc" ) );
                    aArgs.getArray()[0] <<= aVal;
                    Reference< XNameAccess > xConfigAccess(
                        Reference< XNameAccess >(
                            xConfigProvider->createInstanceWithArguments( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                "com.sun.star.configuration.ConfigurationAccess" )),
                                                                            aArgs ),
                            UNO_QUERY )
                        );
                    if( xConfigAccess.is() )
                    {
                        try
                        {
                            sal_Bool bValue = sal_False;
                            Any aAny = xConfigAccess->getByName( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseSystemPrintDialog" ) ) );
                            if( aAny >>= bValue )
                                bNative = bValue;
                        }
                        catch( NoSuchElementException& )
                        {
                        }
                        catch( WrappedTargetException& )
                        {
                        }
                    }
                }
            }
            catch( Exception& )
            {
            }
        }
    }
    catch( WrappedTargetException& )
    {
    }

    return bNative;
}

ULONG AquaSalInfoPrinter::GetCapabilities( const ImplJobSetup* i_pSetupData, USHORT i_nType )
{
    switch( i_nType )
    {
        case PRINTER_CAPABILITIES_SUPPORTDIALOG:
            return 0;
        case PRINTER_CAPABILITIES_COPIES:
            return 0xffff;
        case PRINTER_CAPABILITIES_COLLATECOPIES:
            return 0;
        case PRINTER_CAPABILITIES_SETORIENTATION:
            return 1;
        case PRINTER_CAPABILITIES_SETPAPERBIN:
            return 0;
        case PRINTER_CAPABILITIES_SETPAPERSIZE:
            return 1;
        case PRINTER_CAPABILITIES_SETPAPER:
            return 1;
        case PRINTER_CAPABILITIES_EXTERNALDIALOG:
            return getUseNativeDialog() ? 1 : 0;
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
        long nDPIX = 72, nDPIY = 72;
        mpGraphics->GetResolution( nDPIX, nDPIY );
        const double fXScaling = static_cast<double>(nDPIX)/72.0,
                     fYScaling = static_cast<double>(nDPIY)/72.0;

        NSSize aPaperSize = [mpPrintInfo paperSize];
        o_rPageWidth  = static_cast<long>( double(aPaperSize.width) * fXScaling );
        o_rPageHeight = static_cast<long>( double(aPaperSize.height) * fYScaling );
        o_rPageOffX   = static_cast<long>( [mpPrintInfo leftMargin] * fXScaling );
        o_rPageOffY   = static_cast<long>( [mpPrintInfo topMargin] * fYScaling );
        o_rOutWidth   = static_cast<long>( o_rPageWidth - double([mpPrintInfo leftMargin] + [mpPrintInfo rightMargin]) * fXScaling );
        o_rOutHeight  = static_cast<long>( o_rPageHeight - double([mpPrintInfo topMargin] + [mpPrintInfo bottomMargin]) * fYScaling );
    }
}

BOOL AquaSalInfoPrinter::StartJob( const String* pFileName,
                                   const String& rAppName,
                                   ImplJobSetup* pSetupData,
                                   ImplQPrinter* pQPrinter,
                                   bool bIsQuickJob )
{
    if( mbJob )
        return FALSE;

    BOOL bSuccess = FALSE;
    mnStartPageOffsetX = mnStartPageOffsetY = 0;

    // create view
    NSView* pPrintView = [[AquaPrintView alloc] initWithQPrinter: pQPrinter withInfoPrinter: this];

    NSMutableDictionary* pPrintDict = [mpPrintInfo dictionary];

    // set filename
    if( pFileName )
    {
        [mpPrintInfo setJobDisposition: NSPrintSaveJob];
        NSString* pPath = CreateNSString( *pFileName );
        [pPrintDict setObject: pPath forKey: NSPrintSavePath];
        [pPath release];
    }

    [pPrintDict setObject: [[NSNumber numberWithInt: (int)pQPrinter->GetCopyCount()] autorelease] forKey: NSPrintCopies];
    [pPrintDict setObject: [[NSNumber numberWithBool: YES] autorelease] forKey: NSPrintDetailedErrorReporting];
    [pPrintDict setObject: [[NSNumber numberWithInt: 1] autorelease] forKey: NSPrintFirstPage];
    [pPrintDict setObject: [[NSNumber numberWithInt: (int)pQPrinter->GetPrintPageCount()] autorelease] forKey: NSPrintLastPage];


    // create print operation
    NSPrintOperation* pPrintOperation = [NSPrintOperation printOperationWithView: pPrintView printInfo: mpPrintInfo];

    if( pPrintOperation )
    {
        bool bShowPanel = (! bIsQuickJob && getUseNativeDialog());
        [pPrintOperation setShowsPrintPanel: bShowPanel ? YES : NO ];
        // [pPrintOperation setShowsProgressPanel: NO];
        bSuccess = TRUE;
        mbJob = true;
        [pPrintOperation runOperation];
        mbJob = false;
    }

    return bSuccess;
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::EndJob()
{
    mnStartPageOffsetX = mnStartPageOffsetY = 0;
    mbJob = false;
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::AbortJob()
{
    mbJob = false;

    // FIXME: implementation
    return FALSE;
}

// -----------------------------------------------------------------------

SalGraphics* AquaSalInfoPrinter::StartPage( ImplJobSetup* i_pSetupData, BOOL i_bNewJobData )
{
    if( i_bNewJobData && i_pSetupData )
        SetPrinterData( i_pSetupData );

    CGContextRef rContext = reinterpret_cast<CGContextRef>([[NSGraphicsContext currentContext] graphicsPort]);

    SetupPrinterGraphics( rContext );

    return mpGraphics;
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::EndPage()
{
    return TRUE;
}

// -----------------------------------------------------------------------

ULONG AquaSalInfoPrinter::GetErrorCode() const
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

BOOL AquaSalPrinter::StartJob( const String* pFileName,
                               const String& rAppName,
                               ImplJobSetup* pSetupData,
                               ImplQPrinter* pQPrinter )
{
    bool bIsQuickJob = false;
    std::hash_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator quick_it =
        pSetupData->maValueMap.find( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsQuickJob" ) ) );

    if( quick_it != pSetupData->maValueMap.end() )
    {
        if( quick_it->second.equalsIgnoreAsciiCaseAscii( "true" ) )
            bIsQuickJob = true;
    }

    return mpInfoPrinter->StartJob( pFileName, rAppName, pSetupData, pQPrinter, bIsQuickJob );
}

// -----------------------------------------------------------------------

BOOL AquaSalPrinter::StartJob( const XubString* i_pFileName,
                           const XubString& i_rJobName,
                           const XubString& i_rAppName,
                           ULONG i_nCopies, BOOL i_bCollate,
                           ImplJobSetup* i_pSetupData )
{
    DBG_ERROR( "should never be called" );
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL AquaSalPrinter::EndJob()
{
    return mpInfoPrinter->EndJob();
}

// -----------------------------------------------------------------------

BOOL AquaSalPrinter::AbortJob()
{
    return mpInfoPrinter->AbortJob();
}

// -----------------------------------------------------------------------

SalGraphics* AquaSalPrinter::StartPage( ImplJobSetup* i_pSetupData, BOOL i_bNewJobData )
{
    return mpInfoPrinter->StartPage( i_pSetupData, i_bNewJobData );
}

// -----------------------------------------------------------------------

BOOL AquaSalPrinter::EndPage()
{
    return mpInfoPrinter->EndPage();
}

// -----------------------------------------------------------------------

ULONG AquaSalPrinter::GetErrorCode()
{
    return mpInfoPrinter->GetErrorCode();
}

////////////////////////////
//////   IMPLEMENT US  /////
////////////////////////////

DuplexMode AquaSalInfoPrinter::GetDuplexMode( const ImplJobSetup* i_pSetupData )
{
    return DUPLEX_UNKNOWN;
}

void AquaSalInfoPrinter::InitPaperFormats( const ImplJobSetup* i_pSetupData )
{
}

int AquaSalInfoPrinter::GetLandscapeAngle( const ImplJobSetup* i_pSetupData )
{
    return 0;
}
