/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salprn.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-09-13 16:31:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <salinst.h>
#include <salprn.h>
#include <salgdi.h>
#include <vcl/jobset.h>
#include <vcl/salptype.hxx>

#include <rtl/math.hxx>

// =======================================================================

AquaSalInfoPrinter::AquaSalInfoPrinter( const SalPrinterQueueInfo& i_rQueue ) :
    mrSession( 0 ),
    mrSettings( 0 ),
    mrPrinter( 0 ),
    mrPageFormat( 0 ),
    mpGraphics( 0 ),
    mbGraphics( false ),
    mbJob( false )
{
    DBG_ASSERT( i_rQueue.mpSysData, "no printer id in queue" );

    if( PMCreateSession( &mrSession ) == noErr )
    {
        PMCreatePrintSettings( &mrSettings );
        PMCreatePageFormat( &mrPageFormat );

        mrPrinter = PMPrinterCreateFromPrinterID( reinterpret_cast<CFStringRef>(i_rQueue.mpSysData) );
        if( mrPrinter )
            PMSessionSetCurrentPMPrinter( mrSession, mrPrinter );

        PMSessionDefaultPrintSettings( mrSession, mrSettings );
        PMSessionDefaultPageFormat( mrSession, mrPageFormat );

        // note: kPMDestinationInvalid does not do nothing, but results
        // in an actual print job; is this by design ?
        CFStringRef rFile = CreateCFString( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/dev/null" ) ) );
        CFURLRef rURL = CFURLCreateWithFileSystemPath( NULL, rFile, kCFURLPOSIXPathStyle, false );
        if ( rFile )
            CFRelease( rFile );

        OSStatus err =
        PMSessionSetDestination( mrSession,
                                 mrSettings,
                                 kPMDestinationFile,
                                 NULL,
                                 rURL );
        if( rURL )
            CFRelease( rURL );

        if( err == noErr )
            err = PMSessionBeginCGDocumentNoDialog( mrSession, mrSettings, mrPageFormat );
        if( err == noErr )
            err = PMSessionBeginPageNoDialog( mrSession, mrPageFormat, NULL );
        CGContextRef rContext = 0;
        if( err == noErr )
            err = PMSessionGetCGGraphicsContext( mrSession, &rContext );
        mpGraphics = new AquaSalGraphics();
        SetupPrinterGraphics( rContext );
    }
    else
        mrSession = 0;
}

// -----------------------------------------------------------------------

AquaSalInfoPrinter::~AquaSalInfoPrinter()
{
    delete mpGraphics;
    if( mrPrinter )
        PMRelease( mrPrinter );
    if( mrPageFormat )
        PMRelease( mrPageFormat );
    if( mrSettings )
        PMRelease( mrSettings );
    if( mrSession )
        PMRelease( mrSession );
}

// -----------------------------------------------------------------------

void AquaSalInfoPrinter::SetupPrinterGraphics( CGContextRef i_rContext ) const
{
    if( mpGraphics )
    {
        if( mrPageFormat )
        {
            PMResolution aRes;
            if( PMGetResolution( mrPageFormat, &aRes ) != noErr )
                aRes.hRes = aRes.vRes = 72.0;

            // a reported resolution of 72 dpi ist most likely not good for
            // formatting and downscaling images
            // so assume a reasonable default resolution for a printer
            if( aRes.hRes == 72.0 )
                aRes.hRes = 720.0;
            if( aRes.vRes == 72.0 )
                aRes.vRes = 720.0;

            // mirror context so it fits OOo's coordinate space
            // get page height
            PMRect aRect;
            if( PMGetUnadjustedPaperRect( mrPageFormat, &aRect ) == noErr )
            {
                PMOrientation aOrient;
                OSStatus err = PMGetOrientation( mrPageFormat, &aOrient );
                DBG_ASSERT( err == noErr, "error in PMGetOrientation" );
                (void)err; // make product build happy
                switch( aOrient )
                {
                case kPMLandscape:
                    CGContextTranslateCTM( i_rContext, aRect.bottom, -aRect.left );
                    CGContextScaleCTM( i_rContext, -72.0/aRes.vRes, 72.0/aRes.hRes );
                    break;
                default:
                    DBG_ERROR( "unhandled orientation, defaulting to portrait" );
                case kPMPortrait:
                    CGContextTranslateCTM( i_rContext, -aRect.left, aRect.bottom );
                    CGContextScaleCTM( i_rContext, 72.0/aRes.hRes, -72.0/aRes.vRes );
                    break;
                }
            }

            mpGraphics->SetPrinterGraphics( i_rContext, static_cast<long>(aRes.hRes), static_cast<long>(aRes.vRes) );
        }
        else
            DBG_ERROR( "no page format in SetupPrinterGraphics" );
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

static PMPaper findBestPaper( const ImplJobSetup& i_rSetupData, PMPrinter i_rPrinter )
{
    PMPaper rPaper = 0;
    DBG_ASSERT( i_rPrinter, "no printer for findBestPaper" );
    CFArrayRef rPaperList = 0;
    if( PMPrinterGetPaperList( i_rPrinter, &rPaperList ) == noErr )
    {
        const CFIndex nPapers = CFArrayGetCount( rPaperList );
        for( CFIndex n = 0; n < nPapers; n++ )
        {
            PMPaper aPaper = reinterpret_cast<PMPaper>(const_cast<void*>(CFArrayGetValueAtIndex( rPaperList, n )));

            double width = 0, height = 0;
            PMPaperGetWidth( aPaper, &width );
            PMPaperGetHeight( aPaper, &height );
            if( i_rSetupData.mePaperFormat == recognizePaper( width, height ) )
            {
                if( i_rSetupData.mePaperFormat == PAPER_USER )
                {
                    if( rtl::math::approxEqual( width, TenMuToPt( i_rSetupData.mnPaperWidth ) ) &&
                        rtl::math::approxEqual( height, TenMuToPt( i_rSetupData.mnPaperHeight ) ) )
                    {
                        rPaper = aPaper;
                        break;
                    }
                }
                else
                {
                    rPaper = aPaper;
                    break;
                }
            }
        }
        if( ! rPaper && nPapers > 0 )
        {
            // take the first paper as fallback
            rPaper = reinterpret_cast<PMPaper>(const_cast<void*>(CFArrayGetValueAtIndex( rPaperList, 0 )));
        }
    }
    return rPaper;
}

BOOL AquaSalInfoPrinter::SetPrinterData( ImplJobSetup* io_pSetupData )
{
    // FIXME: implement driver data
    if( io_pSetupData && io_pSetupData->mpDriverData )
        return SetData( ~0, io_pSetupData );


    OSStatus err = noErr;
    BOOL bSuccess = TRUE;

    // set system type
    io_pSetupData->mnSystem = JOBSETUP_SYSTEM_MAC;

    // get paper format
    if( mrPageFormat )
    {
        PMPaper rPaper = 0;
        err = PMGetPageFormatPaper( mrPageFormat, &rPaper );
        if( err == noErr )
        {
            double width, height;
            PMPaperGetWidth( rPaper, &width );
            PMPaperGetHeight( rPaper, &height );
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
        }
        else
            bSuccess = FALSE;

        // get orientation
        PMOrientation aOrient;
        err = PMGetOrientation( mrPageFormat, &aOrient );
        if( err == noErr )
        {
            // set orientation
            io_pSetupData->meOrientation = (aOrient == kPMLandscape || aOrient == kPMReverseLandscape) ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT;
        }
        else
            bSuccess = FALSE;

        io_pSetupData->mnPaperBin = 0;
        io_pSetupData->mpDriverData = reinterpret_cast<BYTE*>(rtl_allocateMemory( 4 ));
        io_pSetupData->mnDriverDataLen = 4;
    }
    else
        bSuccess = FALSE;


    return (err == noErr);
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::SetData( ULONG i_nFlags, ImplJobSetup* io_pSetupData )
{
    if( ! io_pSetupData || io_pSetupData->mnSystem != JOBSETUP_SYSTEM_MAC )
        return FALSE;

    OSStatus err = noErr;

    if( (i_nFlags & SAL_JOBSET_PAPERSIZE) !=  0)
    {
        if( mrPageFormat )
        {
            // check whether we need to update the paper format
            bool bNewPaper = true;

            // get paper format
            PMPaper rPaper = 0;
            err = PMGetPageFormatPaper( mrPageFormat, &rPaper );
            if( err == noErr )
            {
                double width, height;
                PMPaperGetWidth( rPaper, &width );
                PMPaperGetHeight( rPaper, &height );
                if( io_pSetupData->mePaperFormat == recognizePaper( width, height ) )
                {
                    if( io_pSetupData->mePaperFormat == PAPER_USER )
                    {
                        if( rtl::math::approxEqual( width, TenMuToPt( io_pSetupData->mnPaperWidth ) ) &&
                            rtl::math::approxEqual( height, TenMuToPt( io_pSetupData->mnPaperHeight ) ) )
                        {
                            bNewPaper = false;
                        }
                    }
                    else
                        bNewPaper = false;
                }
            }
            if( bNewPaper )
            {
                PMRelease( mrPageFormat );
                mrPageFormat = 0;
            }
        }

        if( ! mrPageFormat && mrPrinter )
        {
            PMPaper rPaper = findBestPaper( *io_pSetupData, mrPrinter );
            if( rPaper )
                err = PMCreatePageFormatWithPMPaper( &mrPageFormat, rPaper );
        }
    }

    if( (i_nFlags & SAL_JOBSET_ORIENTATION) != 0 )
    {
        if( err == noErr && mrPageFormat )
            err = PMSetOrientation( mrPageFormat, (io_pSetupData->meOrientation == ORIENTATION_PORTRAIT) ? kPMPortrait : kPMLandscape, false );
    }

    return err == noErr && mrPageFormat;
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
    if( mrPageFormat )
    {
        PMRect aPageRect, aPaperRect;
        if( PMGetAdjustedPageRect( mrPageFormat, &aPageRect ) == noErr &&
            PMGetAdjustedPaperRect( mrPageFormat, &aPaperRect ) == noErr )
        {
            long nDPIX = 72, nDPIY = 72;
            mpGraphics->GetResolution( nDPIX, nDPIY );
            const double fXScaling = static_cast<double>(nDPIX)/72.0,
                         fYScaling = static_cast<double>(nDPIY)/72.0;
            o_rPageWidth  = (aPaperRect.right - aPaperRect.left) * fXScaling;
            o_rPageHeight = (aPaperRect.bottom - aPaperRect.top ) * fYScaling;
            o_rPageOffX   = (aPageRect.left - aPaperRect.left) * fXScaling;
            o_rPageOffY   = (aPageRect.top - aPaperRect.top) * fYScaling;
            o_rOutWidth   = (aPageRect.right - aPageRect.left) * fXScaling;
            o_rOutHeight  = (aPageRect.bottom - aPageRect.top) * fYScaling;
        }
    }
}

BOOL AquaSalInfoPrinter::StartJob( const XubString* i_pFileName,
                           const XubString& i_rJobName,
                           const XubString& i_rAppName,
                           ULONG i_nCopies, BOOL i_bCollate,
                           ImplJobSetup* i_pSetupData )
{
    if( mbJob || ! mrPrinter )
        return FALSE;

    BOOL bSuccess = FALSE;

    // end the info printer session to kPMDestinationInvalid
    if( mrSession )
    {
        PMSessionEndPage( mrSession );
        PMSessionEndDocumentNoDialog( mrSession );
        PMRelease( mrSession );
        mrSession = 0;
    }

    // start a real session for the job
    if( PMCreateSession( &mrSession ) == noErr )
    {
        OSStatus err = PMSessionSetCurrentPMPrinter( mrSession, mrPrinter );

        CFURLRef rURL = 0;
        if( i_pFileName )
        {
            CFStringRef rFile = CreateCFString( *i_pFileName );
            rURL = CFURLCreateWithFileSystemPath( NULL, rFile, kCFURLPOSIXPathStyle, false );
            if ( rFile )
                CFRelease( rFile );
        }

        err =
        PMSessionSetDestination( mrSession,
                                 mrSettings,
                                 rURL ? kPMDestinationFile : kPMDestinationPrinter,
                                 NULL,
                                 rURL );
        if( rURL )
            CFRelease( rURL );

        if( i_pSetupData )
            SetPrinterData( i_pSetupData );

        if( i_nCopies > 1 && err == noErr )
        {
            err = PMSetCopies( mrSettings, i_nCopies, false );

            if( err == noErr )
                err = PMSetCollate( mrSettings, i_bCollate );
        }

        if( i_rJobName.Len() && err == noErr )
        {
            CFStringRef rName = CreateCFString( i_rJobName );
            err = PMPrintSettingsSetJobName( mrSettings, rName );
            if ( rName )
                CFRelease( rName );
        }

        if( err == noErr )
            err = PMSessionBeginCGDocumentNoDialog( mrSession, mrSettings, mrPageFormat );
        bSuccess = (err == noErr);
    }

    mbJob = bSuccess;
    return bSuccess;
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::EndJob()
{
    OSStatus err = 1;
    if( mrSession )
    {
        err = PMSessionEndDocumentNoDialog( mrSession );
        PMRelease( mrSession );
        mrSession = 0;
    }

    mbJob = false;
    return err == noErr;
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
    if( ! mrSession )
        return NULL;

    if( i_bNewJobData && i_pSetupData )
        SetPrinterData( i_pSetupData );

    OSStatus err = PMSessionError( mrSession );
    if( err != noErr )
        return NULL;

    CGContextRef rContext = 0;
    err = PMSessionBeginPageNoDialog( mrSession, mrPageFormat, NULL );
    if( err == noErr )
        err = PMSessionGetCGGraphicsContext( mrSession, &rContext );

    SetupPrinterGraphics( rContext );

    return mpGraphics;
}

// -----------------------------------------------------------------------

BOOL AquaSalInfoPrinter::EndPage()
{
    OSStatus err = PMSessionEndPageNoDialog( mrSession );
    return err == noErr;
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

BOOL AquaSalPrinter::StartJob( const XubString* i_pFileName,
                           const XubString& i_rJobName,
                           const XubString& i_rAppName,
                           ULONG i_nCopies, BOOL i_bCollate,
                           ImplJobSetup* i_pSetupData )
{
    return mpInfoPrinter->StartJob( i_pFileName, i_rJobName, i_rAppName, i_nCopies, i_bCollate, i_pSetupData );
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
