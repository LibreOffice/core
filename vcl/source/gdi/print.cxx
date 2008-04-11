/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: print.cxx,v $
 * $Revision: 1.65 $
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

#define _SPOOLPRINTER_EXT
#define _RMPRINTER_EXT
#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <list>

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salinst.hxx>
#include <vcl/salgdi.hxx>
#include <vcl/salptype.hxx>
#include <vcl/salprn.hxx>

#include <vcl/unohelp.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/jobset.h>
#include <vcl/outdev.h>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/print.h>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/impprn.hxx>
#include <vcl/print.hxx>

#include <comphelper/processfactory.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

int nImplSysDialog = 0;

// =======================================================================

#define PAPER_SLOPPY    50  // Bigger sloppy value as PaperInfo uses only mm accuracy!
#define PAPER_COUNT     9

// Use more accurate metric values for Letter/Legal/Tabloid paper formats
static long ImplPaperFormats[PAPER_COUNT*2] =
{
    29700, 42000,   // A3
    21000, 29700,   // A4
    14800, 21000,   // A5
    25000, 35300,   // B4
    17600, 25000,   // B5
    21590, 27940,   // Letter
    21590, 35570,   // Legal
    27960, 43130,   // Tabloid
    0,     0        // USER
};

// =======================================================================

Paper ImplGetPaperFormat( long nWidth100thMM, long nHeight100thMM )
{
    USHORT i;

    for( i = 0; i < PAPER_COUNT; i++ )
    {
        if ( (ImplPaperFormats[i*2] == nWidth100thMM) &&
             (ImplPaperFormats[i*2+1] == nHeight100thMM) )
            return (Paper)i;
    }

    for( i = 0; i < PAPER_COUNT; i++ )
    {
        if ( (Abs( ImplPaperFormats[i*2]-nWidth100thMM ) < PAPER_SLOPPY) &&
             (Abs( ImplPaperFormats[i*2+1]-nHeight100thMM ) < PAPER_SLOPPY) )
            return (Paper)i;
    }

    return PAPER_USER;
}

// =======================================================================

void ImplUpdateJobSetupPaper( JobSetup& rJobSetup )
{
    const ImplJobSetup* pConstData = rJobSetup.ImplGetConstData();

    if ( !pConstData->mnPaperWidth || !pConstData->mnPaperHeight )
    {
        if ( pConstData->mePaperFormat != PAPER_USER )
        {
            ImplJobSetup* pData  = rJobSetup.ImplGetData();
            pData->mnPaperWidth  = ImplPaperFormats[((USHORT)pConstData->mePaperFormat)*2];
            pData->mnPaperHeight = ImplPaperFormats[((USHORT)pConstData->mePaperFormat)*2+1];
        }
    }
    else if ( pConstData->mePaperFormat == PAPER_USER )
    {
        Paper ePaper = ImplGetPaperFormat( pConstData->mnPaperWidth, pConstData->mnPaperHeight );
        if ( ePaper != PAPER_USER )
            rJobSetup.ImplGetData()->mePaperFormat = ePaper;
    }
}

// ------------------
// - PrinterOptions -
// ------------------

PrinterOptions::PrinterOptions() :
    mbReduceTransparency( FALSE ),
    meReducedTransparencyMode( PRINTER_TRANSPARENCY_AUTO ),
    mbReduceGradients( FALSE ),
    meReducedGradientsMode( PRINTER_GRADIENT_STRIPES ),
    mnReducedGradientStepCount( 64 ),
    mbReduceBitmaps( FALSE ),
    meReducedBitmapMode( PRINTER_BITMAP_NORMAL ),
    mnReducedBitmapResolution( 200 ),
    mbReducedBitmapsIncludeTransparency( TRUE ),
    mbConvertToGreyscales( FALSE )
{
}

// -----------------------------------------------------------------------

PrinterOptions::~PrinterOptions()
{
}

// -------------
// - QueueInfo -
// -------------

QueueInfo::QueueInfo()
{
    mnStatus    = 0;
    mnJobs      = 0;
}

// -----------------------------------------------------------------------

QueueInfo::QueueInfo( const QueueInfo& rInfo ) :
    maPrinterName( rInfo.maPrinterName ),
    maDriver( rInfo.maDriver ),
    maLocation( rInfo.maLocation ),
    maComment( rInfo.maComment ),
    mnStatus( rInfo.mnStatus ),
    mnJobs( rInfo.mnJobs )
{
}

// -----------------------------------------------------------------------

QueueInfo::~QueueInfo()
{
}

// -----------------------------------------------------------------------

bool QueueInfo::operator==( const QueueInfo& rInfo ) const
{
    return
        maPrinterName   == rInfo.maPrinterName  &&
        maDriver        == rInfo.maDriver       &&
        maLocation      == rInfo.maLocation     &&
        maComment       == rInfo.maComment      &&
        mnStatus        == rInfo.mnStatus       &&
        mnJobs          == rInfo.mnJobs;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const QueueInfo& rInfo )
{
    VersionCompat aCompat( rOStream, STREAM_WRITE, 1 );

    rOStream.WriteByteString( rInfo.maPrinterName, RTL_TEXTENCODING_UTF8 );
    rOStream.WriteByteString( rInfo.maDriver, RTL_TEXTENCODING_UTF8 );
    rOStream.WriteByteString( rInfo.maLocation, RTL_TEXTENCODING_UTF8 );
    rOStream.WriteByteString( rInfo.maComment, RTL_TEXTENCODING_UTF8 );
    rOStream << rInfo.mnStatus;
    rOStream << rInfo.mnJobs;

    return rOStream;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, QueueInfo& rInfo )
{
    VersionCompat aCompat( rIStream, STREAM_READ );

    rIStream.ReadByteString( rInfo.maPrinterName, RTL_TEXTENCODING_UTF8 );
    rIStream.ReadByteString( rInfo.maDriver, RTL_TEXTENCODING_UTF8 );
    rIStream.ReadByteString( rInfo.maLocation, RTL_TEXTENCODING_UTF8 );
    rIStream.ReadByteString( rInfo.maComment, RTL_TEXTENCODING_UTF8 );
    rIStream >> rInfo.mnStatus;
    rIStream >> rInfo.mnJobs;

    return rIStream;
}

// =======================================================================

SalPrinterQueueInfo::SalPrinterQueueInfo()
{
    mnStatus    = 0;
    mnJobs      = QUEUE_JOBS_DONTKNOW;
    mpSysData   = NULL;
}

// -----------------------------------------------------------------------

SalPrinterQueueInfo::~SalPrinterQueueInfo()
{
}

// -----------------------------------------------------------------------

ImplPrnQueueList::~ImplPrnQueueList()
{
    ImplSVData*         pSVData = ImplGetSVData();
    for( unsigned int i = 0; i < m_aQueueInfos.size(); i++ )
    {
        delete m_aQueueInfos[i].mpQueueInfo;
        pSVData->mpDefInst->DeletePrinterQueueInfo( m_aQueueInfos[i].mpSalQueueInfo );
    }
}

// -----------------------------------------------------------------------

void ImplPrnQueueList::Add( SalPrinterQueueInfo* pData )
{
    std::hash_map< rtl::OUString, sal_Int32, rtl::OUStringHash >::iterator it =
        m_aNameToIndex.find( pData->maPrinterName );
    if( it == m_aNameToIndex.end() )
    {
        m_aNameToIndex[ pData->maPrinterName ] = m_aQueueInfos.size();
        m_aQueueInfos.push_back( ImplPrnQueueData() );
        m_aQueueInfos.back().mpQueueInfo = NULL;
        m_aQueueInfos.back().mpSalQueueInfo = pData;
        m_aPrinterList.push_back( pData->maPrinterName );
    }
    else // this should not happen, but ...
    {
        ImplPrnQueueData& rData = m_aQueueInfos[ it->second ];
        delete rData.mpQueueInfo;
        rData.mpQueueInfo = NULL;
        ImplGetSVData()->mpDefInst->DeletePrinterQueueInfo( rData.mpSalQueueInfo );
        rData.mpSalQueueInfo = pData;
    }
}

// -----------------------------------------------------------------------

ImplPrnQueueData* ImplPrnQueueList::Get( const rtl::OUString& rPrinter )
{
    ImplPrnQueueData* pData = NULL;
    std::hash_map<rtl::OUString,sal_Int32,rtl::OUStringHash>::iterator it =
        m_aNameToIndex.find( rPrinter );
    if( it != m_aNameToIndex.end() )
        pData = &m_aQueueInfos[it->second];
    return pData;
}

// =======================================================================

static void ImplInitPrnQueueList()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maGDIData.mpPrinterQueueList = new ImplPrnQueueList;

    pSVData->mpDefInst->GetPrinterQueueInfo( pSVData->maGDIData.mpPrinterQueueList );
}

// -----------------------------------------------------------------------

void ImplDeletePrnQueueList()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplPrnQueueList*   pPrnList = pSVData->maGDIData.mpPrinterQueueList;

    if ( pPrnList )
    {
        delete pPrnList;
        pSVData->maGDIData.mpPrinterQueueList = NULL;
    }
}

// -----------------------------------------------------------------------

const std::vector<rtl::OUString>& Printer::GetPrinterQueues()
{
    ImplSVData*         pSVData = ImplGetSVData();
    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();
    return pSVData->maGDIData.mpPrinterQueueList->m_aPrinterList;
}

// -----------------------------------------------------------------------
const QueueInfo* Printer::GetQueueInfo( const String& rPrinterName, bool bStatusUpdate )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    ImplPrnQueueData* pInfo = pSVData->maGDIData.mpPrinterQueueList->Get( rPrinterName );
    if( pInfo )
    {
        if( !pInfo->mpQueueInfo || bStatusUpdate )
            pSVData->mpDefInst->GetPrinterQueueState( pInfo->mpSalQueueInfo );

        if ( !pInfo->mpQueueInfo )
            pInfo->mpQueueInfo = new QueueInfo;

        pInfo->mpQueueInfo->maPrinterName   = pInfo->mpSalQueueInfo->maPrinterName;
        pInfo->mpQueueInfo->maDriver        = pInfo->mpSalQueueInfo->maDriver;
        pInfo->mpQueueInfo->maLocation      = pInfo->mpSalQueueInfo->maLocation;
        pInfo->mpQueueInfo->maComment       = pInfo->mpSalQueueInfo->maComment;
        pInfo->mpQueueInfo->mnStatus        = pInfo->mpSalQueueInfo->mnStatus;
        pInfo->mpQueueInfo->mnJobs          = pInfo->mpSalQueueInfo->mnJobs;
        return pInfo->mpQueueInfo;
    }
    return NULL;
}

// -----------------------------------------------------------------------

XubString Printer::GetDefaultPrinterName()
{
    ImplSVData* pSVData = ImplGetSVData();

    return pSVData->mpDefInst->GetDefaultPrinter();
}

// =======================================================================

void Printer::ImplInitData()
{
    mpPrinterData       = new ImplPrivatePrinterData();
    mbDevOutput         = FALSE;
    meOutDevType        = OUTDEV_PRINTER;
    mbDefPrinter        = FALSE;
    mnError             = 0;
    mnCurPage           = 0;
    mnCurPrintPage      = 0;
    mnPageQueueSize     = 0;
    mnCopyCount         = 1;
    mbCollateCopy       = FALSE;
    mbPrinting          = FALSE;
    mbJobActive         = FALSE;
    mbPrintFile         = FALSE;
    mbInPrintPage       = FALSE;
    mbNewJobSetup       = FALSE;
    mpInfoPrinter       = NULL;
    mpPrinter           = NULL;
    mpDisplayDev        = NULL;
    mpQPrinter          = NULL;
    mpQMtf              = NULL;
    mbIsQueuePrinter    = FALSE;
    mpPrinterOptions    = new PrinterOptions;

    // Printer in die Liste eintragen
    ImplSVData* pSVData = ImplGetSVData();
    mpNext = pSVData->maGDIData.mpFirstPrinter;
    mpPrev = NULL;
    if ( mpNext )
        mpNext->mpPrev = this;
    else
        pSVData->maGDIData.mpLastPrinter = this;
    pSVData->maGDIData.mpFirstPrinter = this;
}

// -----------------------------------------------------------------------

void Printer::ImplInit( SalPrinterQueueInfo* pInfo )
{
    ImplSVData* pSVData = ImplGetSVData();
    // #i74084# update info for this specific SalPrinterQueueInfo
    pSVData->mpDefInst->GetPrinterQueueState( pInfo );

    // Testen, ob Treiber ueberhaupt mit dem JobSetup uebereinstimmt
    ImplJobSetup* pJobSetup = maJobSetup.ImplGetData();

    if ( pJobSetup->mpDriverData )
    {
        if ( (pJobSetup->maPrinterName != pInfo->maPrinterName) ||
             (pJobSetup->maDriver != pInfo->maDriver) )
        {
            rtl_freeMemory( pJobSetup->mpDriverData );
            pJobSetup->mpDriverData = NULL;
            pJobSetup->mnDriverDataLen = 0;
        }
    }

    // Printernamen merken
    maPrinterName = pInfo->maPrinterName;
    maDriver = pInfo->maDriver;

    // In JobSetup den Printernamen eintragen
    pJobSetup->maPrinterName = maPrinterName;
    pJobSetup->maDriver = maDriver;

    mpInfoPrinter   = pSVData->mpDefInst->CreateInfoPrinter( pInfo, pJobSetup );
    mpPrinter       = NULL;
    mpJobPrinter    = NULL;
    mpJobGraphics   = NULL;
    ImplUpdateJobSetupPaper( maJobSetup );

    if ( !mpInfoPrinter )
    {
        ImplInitDisplay( NULL );
        return;
    }

    // we need a graphics
    if ( !ImplGetGraphics() )
    {
        ImplInitDisplay( NULL );
        return;
    }

    // Daten initialisieren
    ImplUpdatePageData();
    mpFontList = new ImplDevFontList();
    mpFontCache = new ImplFontCache( TRUE );
    mpGraphics->GetDevFontList( mpFontList );
}

// -----------------------------------------------------------------------

void Printer::ImplInitDisplay( const Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    mpInfoPrinter       = NULL;
    mpPrinter           = NULL;
    mpJobPrinter        = NULL;
    mpJobGraphics       = NULL;

    if ( pWindow )
        mpDisplayDev = new VirtualDevice( *pWindow );
    else
        mpDisplayDev = new VirtualDevice();
    mpFontList          = pSVData->maGDIData.mpScreenFontList;
    mpFontCache         = pSVData->maGDIData.mpScreenFontCache;
    mnDPIX              = mpDisplayDev->mnDPIX;
    mnDPIY              = mpDisplayDev->mnDPIY;
}

// -----------------------------------------------------------------------

SalPrinterQueueInfo* Printer::ImplGetQueueInfo( const XubString& rPrinterName,
                                                const XubString* pDriver )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    ImplPrnQueueList* pPrnList = pSVData->maGDIData.mpPrinterQueueList;
    if ( pPrnList && pPrnList->m_aQueueInfos.size() )
    {
        // first search for the printer name driectly
        ImplPrnQueueData* pInfo = pPrnList->Get( rPrinterName );
        if( pInfo )
            return pInfo->mpSalQueueInfo;

        // then search case insensitive
        for( unsigned int i = 0; i < pPrnList->m_aQueueInfos.size(); i++ )
        {
            if( pPrnList->m_aQueueInfos[i].mpSalQueueInfo->maPrinterName.EqualsIgnoreCaseAscii( rPrinterName ) )
                return pPrnList->m_aQueueInfos[i].mpSalQueueInfo;
        }

        // then search for driver name
        if ( pDriver )
        {
            for( unsigned int i = 0; i < pPrnList->m_aQueueInfos.size(); i++ )
            {
                if( pPrnList->m_aQueueInfos[i].mpSalQueueInfo->maDriver == *pDriver )
                    return pPrnList->m_aQueueInfos[i].mpSalQueueInfo;
            }
        }

        // then the default printer
        pInfo = pPrnList->Get( GetDefaultPrinterName() );
        if( pInfo )
            return pInfo->mpSalQueueInfo;

        // last chance: the first available printer
        return pPrnList->m_aQueueInfos[0].mpSalQueueInfo;
    }

    return NULL;
}

// -----------------------------------------------------------------------

void Printer::ImplUpdatePageData()
{
    // we need a graphics
    if ( !ImplGetGraphics() )
        return;

    mpGraphics->GetResolution( mnDPIX, mnDPIY );
    mpInfoPrinter->GetPageInfo( maJobSetup.ImplGetConstData(),
                                mnOutWidth, mnOutHeight,
                                maPageOffset.X(), maPageOffset.Y(),
                                maPaperSize.Width(), maPaperSize.Height() );
}

// -----------------------------------------------------------------------

void Printer::ImplUpdateFontList()
{
    ImplUpdateFontData( TRUE );
}

// -----------------------------------------------------------------------

Printer::Printer()
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( GetDefaultPrinterName(), NULL );
    if ( pInfo )
    {
        ImplInit( pInfo );
        if ( !IsDisplayPrinter() )
            mbDefPrinter = TRUE;
    }
    else
        ImplInitDisplay( NULL );
}

// -----------------------------------------------------------------------

Printer::Printer( const Window* pWindow )
{
    ImplInitData();
    ImplInitDisplay( pWindow );
}

// -----------------------------------------------------------------------

Printer::Printer( const JobSetup& rJobSetup ) :
    maJobSetup( rJobSetup )
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rJobSetup.mpData->maPrinterName,
                                                   &rJobSetup.mpData->maDriver );
    if ( pInfo )
    {
        ImplInit( pInfo );
        SetJobSetup( rJobSetup );
    }
    else
    {
        ImplInitDisplay( NULL );
        maJobSetup = JobSetup();
    }
}

// -----------------------------------------------------------------------

Printer::Printer( const QueueInfo& rQueueInfo )
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rQueueInfo.GetPrinterName(),
                                                   &rQueueInfo.GetDriver() );
    if ( pInfo )
        ImplInit( pInfo );
    else
        ImplInitDisplay( NULL );
}

// -----------------------------------------------------------------------

Printer::Printer( const XubString& rPrinterName )
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rPrinterName, NULL );
    if ( pInfo )
        ImplInit( pInfo );
    else
        ImplInitDisplay( NULL );
}

// -----------------------------------------------------------------------

Printer::~Printer()
{
    DBG_ASSERT( !IsPrinting(), "Printer::~Printer() - Job is printing" );
    DBG_ASSERT( !IsJobActive(), "Printer::~Printer() - Job is active" );
    DBG_ASSERT( !mpQPrinter, "Printer::~Printer() - QueuePrinter not destroyed" );
    DBG_ASSERT( !mpQMtf, "Printer::~Printer() - QueueMetafile not destroyed" );

    delete mpPrinterData;
    mpPrinterData = NULL;

    delete mpPrinterOptions;

    ImplReleaseGraphics();
    if ( mpInfoPrinter )
        ImplGetSVData()->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
    if ( mpDisplayDev )
        delete mpDisplayDev;
    else
    {
        // OutputDevice-Dtor versucht das gleiche, deshalb muss hier
        // der FontEntry auch auf NULL gesetzt werden
        // TODO: consolidate duplicate cleanup by Printer and OutputDevice
        if ( mpFontEntry )
        {
            mpFontCache->Release( mpFontEntry );
            mpFontEntry = NULL;
        }
        if ( mpGetDevFontList )
        {
            delete mpGetDevFontList;
            mpGetDevFontList = NULL;
        }
        if ( mpGetDevSizeList )
        {
            delete mpGetDevSizeList;
            mpGetDevSizeList = NULL;
        }
        delete mpFontCache;
        mpFontCache = NULL;
        // font list deleted by OutputDevice dtor
    }

    // Printer aus der Liste eintragen
    ImplSVData* pSVData = ImplGetSVData();
    if ( mpPrev )
        mpPrev->mpNext = mpNext;
    else
        pSVData->maGDIData.mpFirstPrinter = mpNext;
    if ( mpNext )
        mpNext->mpPrev = mpPrev;
    else
        pSVData->maGDIData.mpLastPrinter = mpPrev;
}

// -----------------------------------------------------------------------
void Printer::SetNextJobIsQuick()
{
    mpPrinterData->mbNextJobIsQuick = true;
    if( mpQPrinter )
        mpQPrinter->SetNextJobIsQuick();
}

// -----------------------------------------------------------------------
void Printer::Compat_OldPrinterMetrics( bool bSet )
{
    // propagate flag
    if( mpInfoPrinter )
        mpInfoPrinter->m_bCompatMetrics = bSet;
    if( mpQPrinter )
        mpQPrinter->Compat_OldPrinterMetrics( bSet );

    // get new font data
    ImplUpdateFontData( TRUE );
}

// -----------------------------------------------------------------------

ULONG Printer::GetCapabilities( USHORT nType ) const
{
    if ( IsDisplayPrinter() )
        return FALSE;

    if( mpInfoPrinter )
        return mpInfoPrinter->GetCapabilities( maJobSetup.ImplGetConstData(), nType );
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::HasSupport( PrinterSupport eFeature ) const
{
    switch ( eFeature )
    {
        case SUPPORT_SET_ORIENTATION:
            return (BOOL)GetCapabilities( PRINTER_CAPABILITIES_SETORIENTATION );
        case SUPPORT_SET_PAPERBIN:
            return (BOOL)GetCapabilities( PRINTER_CAPABILITIES_SETPAPERBIN );
        case SUPPORT_SET_PAPERSIZE:
            return (BOOL)GetCapabilities( PRINTER_CAPABILITIES_SETPAPERSIZE );
        case SUPPORT_SET_PAPER:
            return (BOOL)GetCapabilities( PRINTER_CAPABILITIES_SETPAPER );
        case SUPPORT_COPY:
            return (GetCapabilities( PRINTER_CAPABILITIES_COPIES ) != 0);
        case SUPPORT_COLLATECOPY:
            return (GetCapabilities( PRINTER_CAPABILITIES_COLLATECOPIES ) != 0);
        case SUPPORT_SETUPDIALOG:
            return (BOOL)GetCapabilities( PRINTER_CAPABILITIES_SUPPORTDIALOG );
        case SUPPORT_FAX:
            return (BOOL) GetCapabilities( PRINTER_CAPABILITIES_FAX );
        case SUPPORT_PDF:
            return (BOOL) GetCapabilities( PRINTER_CAPABILITIES_PDF );
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Printer::SetJobSetup( const JobSetup& rSetup )
{
    if ( IsDisplayPrinter() || mbInPrintPage )
        return FALSE;

    JobSetup aJobSetup = rSetup;

    ImplReleaseGraphics();
    if ( mpInfoPrinter->SetPrinterData( aJobSetup.ImplGetData() ) )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = TRUE;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------


BOOL Printer::Setup( Window* pWindow )
{
    if ( IsDisplayPrinter() )
        return FALSE;

    if ( IsJobActive() || IsPrinting() )
        return FALSE;

    JobSetup aJobSetup = maJobSetup;
    SalFrame* pFrame;
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    if( !pWindow )
        return FALSE;

    pFrame = pWindow->ImplGetFrame();
    ImplReleaseGraphics();
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode++;
    nImplSysDialog++;
    BOOL bSetup = mpInfoPrinter->Setup( pFrame, aJobSetup.ImplGetData() );
    pSVData->maAppData.mnModalMode--;
    nImplSysDialog--;
    if ( bSetup )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = TRUE;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
        return TRUE;
    }
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::SetPrinterProps( const Printer* pPrinter )
{
    if ( IsJobActive() || IsPrinting() )
        return FALSE;

    ImplSVData* pSVData = ImplGetSVData();

    mbDefPrinter        = pPrinter->mbDefPrinter;
    maPrintFile         = pPrinter->maPrintFile;
    mbPrintFile         = pPrinter->mbPrintFile;
    mnCopyCount         = pPrinter->mnCopyCount;
    mbCollateCopy       = pPrinter->mbCollateCopy;
    mnPageQueueSize     = pPrinter->mnPageQueueSize;
    *mpPrinterOptions   = *pPrinter->mpPrinterOptions;

    if ( pPrinter->IsDisplayPrinter() )
    {
        // Alten Printer zerstoeren
        if ( !IsDisplayPrinter() )
        {
            ImplReleaseGraphics();
            pSVData->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
            if ( mpFontEntry )
            {
                mpFontCache->Release( mpFontEntry );
                mpFontEntry = NULL;
            }
            if ( mpGetDevFontList )
            {
                delete mpGetDevFontList;
                mpGetDevFontList = NULL;
            }
            if ( mpGetDevSizeList )
            {
                delete mpGetDevSizeList;
                mpGetDevSizeList = NULL;
            }
            // clean up font list
            delete mpFontCache;
            delete mpFontList;
            mpFontCache = NULL;
            mpFontList = NULL;

            mbInitFont = TRUE;
            mbNewFont = TRUE;
            mpInfoPrinter = NULL;
        }

        // Neuen Printer bauen
        ImplInitDisplay( NULL );
        return TRUE;
    }

    // Alten Printer zerstoeren?
    if ( GetName() != pPrinter->GetName() )
    {
        ImplReleaseGraphics();
        if ( mpDisplayDev )
        {
            delete mpDisplayDev;
            mpDisplayDev = NULL;
        }
        else
        {
            pSVData->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );

            if ( mpFontEntry )
            {
                mpFontCache->Release( mpFontEntry );
                mpFontEntry = NULL;
            }
            if ( mpGetDevFontList )
            {
                delete mpGetDevFontList;
                mpGetDevFontList = NULL;
            }
            if ( mpGetDevSizeList )
            {
                delete mpGetDevSizeList;
                mpGetDevSizeList = NULL;
            }
            delete mpFontCache;
            delete mpFontList;
            mpFontCache = NULL;
            mpFontList = NULL;
            mbInitFont = TRUE;
            mbNewFont = TRUE;
            mpInfoPrinter = NULL;
        }

        // Neuen Printer bauen
        XubString aDriver = pPrinter->GetDriverName();
        SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( pPrinter->GetName(), &aDriver );
        if ( pInfo )
        {
            ImplInit( pInfo );
            SetJobSetup( pPrinter->GetJobSetup() );
        }
        else
            ImplInitDisplay( NULL );
    }
    else
        SetJobSetup( pPrinter->GetJobSetup() );

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::SetOrientation( Orientation eOrientation )
{
    if ( mbInPrintPage )
        return FALSE;

    if ( maJobSetup.ImplGetConstData()->meOrientation != eOrientation )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->meOrientation = eOrientation;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            return TRUE;
        }

        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return TRUE;
        }
        else
            return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

Orientation Printer::GetOrientation() const
{
    return maJobSetup.ImplGetConstData()->meOrientation;
}

// -----------------------------------------------------------------------

BOOL Printer::SetPaperBin( USHORT nPaperBin )
{
    if ( mbInPrintPage )
        return FALSE;

    if ( (maJobSetup.ImplGetConstData()->mnPaperBin != nPaperBin) &&
         (nPaperBin < GetPaperBinCount()) )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mnPaperBin = nPaperBin;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            return TRUE;
        }

        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERBIN, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return TRUE;
        }
        else
            return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

USHORT Printer::GetPaperBin() const
{
    return maJobSetup.ImplGetConstData()->mnPaperBin;
}

// -----------------------------------------------------------------------

static BOOL ImplPaperSizeEqual( unsigned long nPaperWidth1, unsigned long nPaperHeight1,
                                unsigned long nPaperWidth2, unsigned long nPaperHeight2 )
{
    const long PAPER_ACCURACY = 1; // 1.0 mm accuracy

    return ( (Abs( long(nPaperWidth1)-long(nPaperWidth2) ) <= PAPER_ACCURACY ) &&
             (Abs( long(nPaperHeight1)-long(nPaperHeight2) ) <= PAPER_ACCURACY ) );
}

// -----------------------------------------------------------------------

// Map user paper format to a available printer paper formats
void Printer::ImplFindPaperFormatForUserSize( JobSetup& aJobSetup )
{
    ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();

    int     nLandscapeAngle = GetLandscapeAngle();
    int     nPaperCount     = GetPaperInfoCount();

    unsigned long nPaperWidth   = pSetupData->mnPaperWidth/100;
    unsigned long nPaperHeight  = pSetupData->mnPaperHeight/100;

    // Alle Papierformate vergleichen und ein passendes raussuchen
    for ( int i = 0; i < nPaperCount; i++ )
    {
        const vcl::PaperInfo& rPaperInfo = GetPaperInfo( i );

        if ( ImplPaperSizeEqual( rPaperInfo.m_nPaperWidth,
                                 rPaperInfo.m_nPaperHeight,
                                 nPaperWidth,
                                 nPaperHeight ) )
        {
            pSetupData->mePaperFormat = ImplGetPaperFormat( rPaperInfo.m_nPaperWidth*100,
                                                            rPaperInfo.m_nPaperHeight*100 );
            break;
        }
    }

    // If the printer supports landscape orientation, check paper sizes again
    // with landscape orientation. This is necessary as a printer driver provides
    // all paper sizes with portrait orientation only!!
    if ( pSetupData->mePaperFormat == PAPER_USER &&
         nLandscapeAngle != 0 &&
         HasSupport( SUPPORT_SET_ORIENTATION ))
    {
        for ( int i = 0; i < nPaperCount; i++ )
        {
            const vcl::PaperInfo& rPaperInfo = GetPaperInfo( i );

            if ( ImplPaperSizeEqual( rPaperInfo.m_nPaperWidth,
                                     rPaperInfo.m_nPaperHeight,
                                     nPaperHeight,
                                     nPaperWidth ))
            {
                pSetupData->mePaperFormat = ImplGetPaperFormat( rPaperInfo.m_nPaperWidth*100,
                                                                rPaperInfo.m_nPaperHeight*100 );
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

BOOL Printer::SetPaper( Paper ePaper )
{
    if ( mbInPrintPage )
        return FALSE;

    if ( maJobSetup.ImplGetConstData()->mePaperFormat != ePaper )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mePaperFormat = ePaper;
        if ( ePaper != PAPER_USER )
        {
            pSetupData->mnPaperWidth  = ImplPaperFormats[((USHORT)ePaper)*2];
            pSetupData->mnPaperHeight = ImplPaperFormats[((USHORT)ePaper)*2+1];
        }

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            return TRUE;
        }

        ImplReleaseGraphics();
        if ( ePaper == PAPER_USER )
            ImplFindPaperFormatForUserSize( aJobSetup );
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return TRUE;
        }
        else
            return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Printer::SetPaperSizeUser( const Size& rSize )
{
    if ( mbInPrintPage )
        return FALSE;

    MapMode aMap100thMM( MAP_100TH_MM );
    Size    aPixSize = LogicToPixel( rSize );
    Size    aPageSize = PixelToLogic( aPixSize, aMap100thMM );
    if ( (maJobSetup.ImplGetConstData()->mePaperFormat != PAPER_USER)       ||
         (maJobSetup.ImplGetConstData()->mnPaperWidth  != aPageSize.Width()) ||
         (maJobSetup.ImplGetConstData()->mnPaperHeight != aPageSize.Height()) )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mePaperFormat   = PAPER_USER;
        pSetupData->mnPaperWidth    = aPageSize.Width();
        pSetupData->mnPaperHeight   = aPageSize.Height();

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            return TRUE;
        }

        ImplReleaseGraphics();
        ImplFindPaperFormatForUserSize( aJobSetup );

        // Changing the paper size can also change the orientation!
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return TRUE;
        }
        else
            return FALSE;
    }

    return TRUE;
}


// -----------------------------------------------------------------------

static const vcl::PaperInfo& ImplGetEmptyPaper()
{
    static vcl::PaperInfo aInfo;
    return aInfo;
}

// -----------------------------------------------------------------------

int Printer::GetPaperInfoCount() const
{
    if( ! mpInfoPrinter )
        return 0;
    if( ! mpInfoPrinter->m_bPapersInit )
        mpInfoPrinter->InitPaperFormats( maJobSetup.ImplGetConstData() );
    return mpInfoPrinter->m_aPaperFormats.size();
}

// -----------------------------------------------------------------------

const vcl::PaperInfo& Printer::GetPaperInfo( int nPaper ) const
{
    if( ! mpInfoPrinter )
        return ImplGetEmptyPaper();
    if( ! mpInfoPrinter->m_bPapersInit )
        mpInfoPrinter->InitPaperFormats( maJobSetup.ImplGetConstData() );
    if( mpInfoPrinter->m_aPaperFormats.empty() || nPaper < 0 || nPaper >= int(mpInfoPrinter->m_aPaperFormats.size()) )
        return ImplGetEmptyPaper();
    return mpInfoPrinter->m_aPaperFormats[nPaper];
}

// -----------------------------------------------------------------------

BOOL Printer::SetPaperFromInfo( const vcl::PaperInfo& rInfo )
{
    MapMode aMap( MAP_MM );
    Size aSize( rInfo.m_nPaperWidth, rInfo.m_nPaperHeight );
    aSize = LogicToPixel( aSize, aMap );
    aSize = PixelToLogic( aSize );
    return SetPaperSizeUser( aSize );
}

// -----------------------------------------------------------------------

DuplexMode Printer::GetDuplexMode() const
{
    return mpInfoPrinter ? mpInfoPrinter->GetDuplexMode( maJobSetup.ImplGetConstData() ) : DUPLEX_UNKNOWN;
}

// -----------------------------------------------------------------------

int Printer::GetLandscapeAngle() const
{
    return mpInfoPrinter ? mpInfoPrinter->GetLandscapeAngle( maJobSetup.ImplGetConstData() ) : 900;
}

// -----------------------------------------------------------------------

const vcl::PaperInfo& Printer::GetCurrentPaperInfo() const
{
    if( ! mpInfoPrinter )
        return ImplGetEmptyPaper();
    if( ! mpInfoPrinter->m_bPapersInit )
        mpInfoPrinter->InitPaperFormats( maJobSetup.ImplGetConstData() );
    if( mpInfoPrinter->m_aPaperFormats.empty() )
        return ImplGetEmptyPaper();

    MapMode aMap( MAP_MM );
    Size aSize = PixelToLogic( GetPaperSizePixel(), aMap );
    int nMatch = -1;
    long nDelta = 0;
    for( unsigned int i = 0; i < mpInfoPrinter->m_aPaperFormats.size(); i++ )
    {
        long nW = mpInfoPrinter->m_aPaperFormats[i].m_nPaperWidth;
        long nH = mpInfoPrinter->m_aPaperFormats[i].m_nPaperHeight;
        if( nW >= (aSize.Width()-1) && nH >= (aSize.Height()-1) )
        {
            long nCurDelta = (nW - aSize.Width())*(nW - aSize.Width()) + (nH - aSize.Height() )*(nH - aSize.Height() );
            if( nMatch == -1 || nCurDelta < nDelta )
            {
                nMatch = i;
                nDelta = nCurDelta;
            }
        }
    }
    return nMatch != -1 ? mpInfoPrinter->m_aPaperFormats[nMatch] : ImplGetEmptyPaper();
}

// -----------------------------------------------------------------------

Paper Printer::GetPaper() const
{
    return maJobSetup.ImplGetConstData()->mePaperFormat;
}

// -----------------------------------------------------------------------

USHORT Printer::GetPaperBinCount() const
{
    if ( IsDisplayPrinter() )
        return 0;

    return (USHORT)mpInfoPrinter->GetPaperBinCount( maJobSetup.ImplGetConstData() );
}

// -----------------------------------------------------------------------

XubString Printer::GetPaperBinName( USHORT nPaperBin ) const
{
    if ( IsDisplayPrinter() )
        return ImplGetSVEmptyStr();

    if ( nPaperBin < GetPaperBinCount() )
        return mpInfoPrinter->GetPaperBinName( maJobSetup.ImplGetConstData(), nPaperBin );
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

BOOL Printer::SetCopyCount( USHORT nCopy, BOOL /*bCollate*/ )
{
    mnCopyCount = nCopy;
    return TRUE;
}

// -----------------------------------------------------------------------

void Printer::Error()
{
    maErrorHdl.Call( this );
}

// -----------------------------------------------------------------------

void Printer::StartPrint()
{
    maStartPrintHdl.Call( this );
}

// -----------------------------------------------------------------------

void Printer::EndPrint()
{
    maEndPrintHdl.Call( this );
}

// -----------------------------------------------------------------------

void Printer::PrintPage()
{
    maPrintPageHdl.Call( this );
}

// -----------------------------------------------------------------------


ULONG ImplSalPrinterErrorCodeToVCL( ULONG nError )
{
    ULONG nVCLError;
    switch ( nError )
    {
        case 0:
            nVCLError = PRINTER_OK;
            break;
        case SAL_PRINTER_ERROR_ABORT:
            nVCLError = PRINTER_ABORT;
            break;
        default:
            nVCLError = PRINTER_GENERALERROR;
            break;
    }

    return nVCLError;
}

// -----------------------------------------------------------------------

void Printer::ImplEndPrint()
{
    mbPrinting      = FALSE;
    mnCurPrintPage  = 0;
    maJobName.Erase();
    if( mpQPrinter ) // not necessarily filled e.g. after AbortJob
    {
        mpQPrinter->Destroy();
        mpQPrinter = NULL;
    }
    EndPrint();
}

// -----------------------------------------------------------------------

IMPL_LINK( Printer, ImplDestroyPrinterAsync, void*, pSalPrinter )
{
    SalPrinter* pPrinter = (SalPrinter*)pSalPrinter;
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->DestroyPrinter( pPrinter );
    return 0;
}

// -----------------------------------------------------------------------

void Printer::ImplUpdateQuickStatus()
{
    // remove possibly added "IsQuickJob"
    if( mpPrinterData->mbNextJobIsQuick )
    {
        rtl::OUString aKey( RTL_CONSTASCII_USTRINGPARAM( "IsQuickJob" ) );
        // const data means not really const, but change all references
        // to refcounted job setup
        ImplJobSetup* pImpSetup = maJobSetup.ImplGetConstData();
        pImpSetup->maValueMap.erase( aKey );
        mpPrinterData->mbNextJobIsQuick = false;
    }
}

class QuickGuard
{
    Printer* mpPrinter;
    public:
    QuickGuard( Printer* pPrn ) : mpPrinter( pPrn ) {}
    ~QuickGuard()
    {
        mpPrinter->ImplUpdateQuickStatus();
    }
};

BOOL Printer::StartJob( const XubString& rJobName )
{
    mnError = PRINTER_OK;

    if ( IsDisplayPrinter() )
        return FALSE;

    if ( IsJobActive() || IsPrinting() )
        return FALSE;

    if( mpPrinterData->mbNextJobIsQuick )
    {
        String aKey( RTL_CONSTASCII_USTRINGPARAM( "IsQuickJob" ) );
        if( maJobSetup.GetValue( aKey ).Len() == 0 )
            maJobSetup.SetValue( aKey, String( RTL_CONSTASCII_USTRINGPARAM( "true" ) ) );
    }

    QuickGuard aQGuard( this );

    ULONG   nCopies = mnCopyCount;
    BOOL    bCollateCopy = mbCollateCopy;
    BOOL    bUserCopy = FALSE;
    if ( IsQueuePrinter() )
    {
        if ( ((ImplQPrinter*)this)->IsUserCopy() )
        {
            nCopies = 1;
            bCollateCopy = FALSE;
        }
    }
    else
    {
        if ( nCopies > 1 )
        {
            ULONG nDevCopy;

            if ( bCollateCopy )
                nDevCopy = GetCapabilities( PRINTER_CAPABILITIES_COLLATECOPIES );
            else
                nDevCopy = GetCapabilities( PRINTER_CAPABILITIES_COPIES );

            // Muessen Kopien selber gemacht werden?
            if ( nCopies > nDevCopy )
            {
                bUserCopy = TRUE;
                nCopies = 1;
                bCollateCopy = FALSE;
            }
        }
        else
            bCollateCopy = FALSE;

        // we need queue printing
        if( !mnPageQueueSize )
            mnPageQueueSize = 1;
    }

    if ( !mnPageQueueSize )
    {
        ImplSVData* pSVData = ImplGetSVData();
        mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

        if ( !mpPrinter )
            return FALSE;

        XubString* pPrintFile;
        if ( mbPrintFile )
            pPrintFile = &maPrintFile;
        else
            pPrintFile = NULL;

        // #125075# StartJob can Reschedule on Windows, sfx
        // depends on IsPrinting() in case of closing a document
        BOOL bSaveNewJobSetup   = mbNewJobSetup;
        mbNewJobSetup           = FALSE;
        String aSaveJobName     = maJobName;
        maJobName               = rJobName;
        mnCurPage               = 1;
        mnCurPrintPage          = 1;
        mbPrinting              = TRUE;

        if( ! ImplGetSVData()->maGDIData.mbPrinterPullModel )
        {
            // in the pull model the job can only be started when
            // we have collected all pages to be printed
            if ( !mpPrinter->StartJob( pPrintFile, rJobName, Application::GetDisplayName(),
                                       nCopies, bCollateCopy,
                                       maJobSetup.ImplGetConstData() ) )
            {
                mnError = ImplSalPrinterErrorCodeToVCL( mpPrinter->GetErrorCode() );
                if ( !mnError )
                    mnError = PRINTER_GENERALERROR;
                pSVData->mpDefInst->DestroyPrinter( mpPrinter );
                mbNewJobSetup       = bSaveNewJobSetup;
                maJobName           = aSaveJobName;
                mnCurPage           = 0;
                mnCurPrintPage      = 0;
                mbPrinting          = FALSE;
                mpPrinter = NULL;
                return FALSE;
            }
        }

        mbJobActive             = TRUE;
        StartPrint();
    }
    else
    {
        mpQPrinter = new ImplQPrinter( this );
        if( mpInfoPrinter )
            mpQPrinter->Compat_OldPrinterMetrics( mpInfoPrinter->m_bCompatMetrics );
        mpQPrinter->SetDigitLanguage( GetDigitLanguage() );
        mpQPrinter->SetUserCopy( bUserCopy );
        mpQPrinter->SetPrinterOptions( *mpPrinterOptions );

        // #125075# StartJob can Reschedule on Windows, sfx
        // depends on IsPrinting() in case of closing a document
        BOOL bSaveNewJobSetup   = mbNewJobSetup;
        mbNewJobSetup           = FALSE;
        String aSaveJobName     = maJobName;
        maJobName               = rJobName;
        mnCurPage               = 1;
        mbPrinting              = TRUE;

        if ( mpQPrinter->StartJob( rJobName ) )
        {
            mbJobActive             = TRUE;
            StartPrint();
            mpQPrinter->StartQueuePrint();
        }
        else
        {
            mbNewJobSetup   = bSaveNewJobSetup;
            maJobName       = aSaveJobName;
            mnCurPage       = 0;
            mbPrinting      = FALSE;
            mnError = mpQPrinter->GetErrorCode();
            mpQPrinter->Destroy();
            mpQPrinter = NULL;
            return FALSE;
        }
    }


    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Printer::EndJob()
{
    BOOL bRet = FALSE;
    if ( !IsJobActive() )
        return bRet;

    DBG_ASSERT( !mbInPrintPage, "Printer::EndJob() - StartPage() without EndPage() called" );

    mbJobActive = FALSE;

    if ( mpPrinter || mpQPrinter )
    {
        ImplReleaseGraphics();

        mnCurPage = 0;

        bRet = TRUE;

        if ( mpPrinter )
        {
            mbPrinting      = FALSE;
            mnCurPrintPage  = 0;
            maJobName.Erase();

            mbDevOutput = FALSE;
            bRet = mpPrinter->EndJob();
            // Hier den Drucker nicht asyncron zerstoeren, da es
            // W95 nicht verkraftet, wenn gleichzeitig gedruckt wird
            // und ein Druckerobjekt zerstoert wird
            ImplGetSVData()->mpDefInst->DestroyPrinter( mpPrinter );
            mpPrinter = NULL;
            EndPrint();
        }
        else
            mpQPrinter->EndQueuePrint();
    }

    return bRet;
}

// -----------------------------------------------------------------------

BOOL Printer::AbortJob()
{
    // Wenn wir einen Queue-Printer haben, kann man diesen noch mit
    // AbortJob() abbrechen, solange dieser noch am Drucken ist
    if ( !IsJobActive() && !IsPrinting() )
        return FALSE;

    mbJobActive     = FALSE;
    mbInPrintPage   = FALSE;
    mpJobGraphics   = NULL;

    if ( mpPrinter || mpQPrinter )
    {
        mbPrinting      = FALSE;
        mnCurPage       = 0;
        mnCurPrintPage  = 0;
        maJobName.Erase();

        if ( mpPrinter )
        {
            ImplReleaseGraphics();
            mbDevOutput = FALSE;
            mpPrinter->AbortJob();
            Application::PostUserEvent( LINK( this, Printer, ImplDestroyPrinterAsync ), mpPrinter );
            mpPrinter = NULL;
            EndPrint();
        }
        else
        {
            mpQPrinter->AbortQueuePrint();
            mpQPrinter->Destroy();
            mpQPrinter = NULL;
            if ( mpQMtf )
            {
                mpQMtf->Clear();
                delete mpQMtf;
                mpQMtf = NULL;
            }
            EndPrint();
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::StartPage()
{
    if ( !IsJobActive() )
        return FALSE;

    if ( mpPrinter || mpQPrinter )
    {
        if ( mpPrinter )
        {
            SalGraphics* pGraphics = mpPrinter->StartPage( maJobSetup.ImplGetConstData(), mbNewJobSetup );
            if ( pGraphics )
            {
                ImplReleaseGraphics();
                mpJobGraphics = pGraphics;
            }
            mbDevOutput = TRUE;
        }
        else
        {
            ImplGetGraphics();
            mpJobGraphics = mpGraphics;
        }

        // PrintJob not aborted ???
        if ( IsJobActive() )
        {
            mbInPrintPage = TRUE;
            mnCurPage++;
            if ( mpQPrinter )
            {
                mpQPrinter->SetPrinterOptions( *mpPrinterOptions );
                mpQMtf = new GDIMetaFile;
                mpQMtf->Record( this );
                mpQMtf->SaveStatus();
            }
            else
            {
                mnCurPrintPage++;
                PrintPage();
            }
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::EndPage()
{
    if ( !IsJobActive() )
        return FALSE;

    mbInPrintPage = FALSE;

    if ( mpPrinter || mpQPrinter )
    {
        if ( mpPrinter )
        {
            mpPrinter->EndPage();
            ImplReleaseGraphics();
            mbDevOutput = FALSE;
        }
        else if ( mpQPrinter )
        {
            // Eigentuemeruebergang an QPrinter
            mpQMtf->Stop();
            mpQMtf->WindStart();
            GDIMetaFile* pPage = mpQMtf;
            mpQMtf = NULL;
            mpQPrinter->AddQueuePage( pPage, mnCurPage, mbNewJobSetup );
        }

        mpJobGraphics = NULL;
        mbNewJobSetup = FALSE;

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Printer::updatePrinters()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplPrnQueueList*   pPrnList = pSVData->maGDIData.mpPrinterQueueList;

    if ( pPrnList )
    {
        ImplPrnQueueList* pNewList = new ImplPrnQueueList;
        pSVData->mpDefInst->GetPrinterQueueInfo( pNewList );

        bool bChanged = pPrnList->m_aQueueInfos.size() != pNewList->m_aQueueInfos.size();
        for( unsigned int i = 0; ! bChanged && i < pPrnList->m_aQueueInfos.size(); i++ )
        {
            ImplPrnQueueData& rInfo     = pPrnList->m_aQueueInfos[i];
            ImplPrnQueueData& rNewInfo  = pNewList->m_aQueueInfos[i];
            if( ! rInfo.mpSalQueueInfo || ! rNewInfo.mpSalQueueInfo || // sanity check
                rInfo.mpSalQueueInfo->maPrinterName != rNewInfo.mpSalQueueInfo->maPrinterName )
            {
                bChanged = true;
            }
        }
        if( bChanged )
        {
            ImplDeletePrnQueueList();
            pSVData->maGDIData.mpPrinterQueueList = pNewList;

            Application* pApp = GetpApp();
            if( pApp )
            {
                DataChangedEvent aDCEvt( DATACHANGED_PRINTER );
                pApp->DataChanged( aDCEvt );
                pApp->NotifyAllWindows( aDCEvt );
            }
        }
        else
            delete pNewList;
    }
}
