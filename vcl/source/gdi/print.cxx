/*************************************************************************
 *
 *  $RCSfile: print.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:34:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_PRINT_CXX
#define _SPOOLPRINTER_EXT
#define _RMPRINTER_EXT
#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <list>

#ifndef REMOTE_APPSERVER

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALPTYPE_HXX
#include <salptype.hxx>
#endif
#ifndef _SV_SALPRN_HXX
#include <salprn.hxx>
#endif

#include <unohelp.hxx>

#else /* REMOTE_APPSERVER */

#include "rvp.hxx"
#include "rmoutdev.hxx"
#include "rmwindow.hxx"

struct SalPrinterQueueInfo
{
    XubString               maPrinterName;
    XubString               maDriver;
    XubString               maLocation;
    XubString               maComment;
    ULONG                   mnStatus;
    ULONG                   mnJobs;
    void*                   mpSysData;

                            SalPrinterQueueInfo();
                            ~SalPrinterQueueInfo();
};
#include "rmprint.hxx"

#include <vos/mutex.hxx>
#ifndef _VCL_UNOHELP_HXX
#include <unohelp.hxx>
#endif

using namespace com::sun::star::portal::client;

#if OSL_DEBUG_LEVEL > 1
#ifdef PRODUCT
#define OSL_DEBUG_LEVEL 0
#else
#define OSL_DEBUG_LEVEL 1
#endif
#endif

#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_JOBSET_H
#include <jobset.h>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_IMPPRN_HXX
#include <impprn.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <print.hxx>
#endif

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

const QueueInfo& QueueInfo::operator==( const QueueInfo& rInfo )
{
    maPrinterName   = rInfo.maPrinterName;
    maDriver        = rInfo.maDriver;
    maLocation      = rInfo.maLocation;
    maComment       = rInfo.maComment;
    mnStatus        = rInfo.mnStatus;
    mnJobs          = rInfo.mnJobs;
    return *this;
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

void ImplPrnQueueList::Add( SalPrinterQueueInfo* pData )
{
    ImplPrnQueueData* pInfo = new ImplPrnQueueData;
    pInfo->mpQueueInfo      = NULL;
    pInfo->mpSalQueueInfo   = pData;
    List::Insert( (void*)pInfo, LIST_APPEND );
}

// =======================================================================

static void ImplInitPrnQueueList()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maGDIData.mpPrinterQueueList = new ImplPrnQueueList;

#ifndef REMOTE_APPSERVER
    pSVData->mpDefInst->GetPrinterQueueInfo( pSVData->maGDIData.mpPrinterQueueList );
#else
    BOOL        bPrinterInfoOk = FALSE;
    const ULONG nCount = pSVData->mpPrinterEnvironment->Infos.getLength();
    const RmQueueInfo* pInfos = pSVData->mpPrinterEnvironment->Infos.getConstArray();

    for( ULONG i = 0; i < nCount; i++ )
    {
        SalPrinterQueueInfo*        pNewInfo = new SalPrinterQueueInfo;

        pNewInfo->maPrinterName = pInfos[i].PrinterName;
        pNewInfo->maDriver      = pInfos[i].Driver;
        pNewInfo->maLocation    = pInfos[i].Location;
        pNewInfo->maComment     = pInfos[i].Comment;
        pNewInfo->mnStatus      = pInfos[i].PrinterStatus;
        pNewInfo->mnJobs        = pInfos[i].Jobs;
        pNewInfo->mpSysData     = NULL;
        pSVData->maGDIData.mpPrinterQueueList->Add( pNewInfo );
    }
#endif
}

// -----------------------------------------------------------------------

void ImplDeletePrnQueueList()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplPrnQueueList*   pPrnList = pSVData->maGDIData.mpPrinterQueueList;

    if ( pPrnList )
    {
        ImplPrnQueueData* pInfo = pPrnList->First();
        while ( pInfo )
        {
            if ( pInfo->mpQueueInfo )
                delete pInfo->mpQueueInfo;

#ifndef REMOTE_APPSERVER
            pSVData->mpDefInst->DeletePrinterQueueInfo( pInfo->mpSalQueueInfo );
#else
            delete pInfo->mpSalQueueInfo;
#endif

            delete pInfo;
            pInfo = pPrnList->Next();
        }

        delete pPrnList;
        pSVData->maGDIData.mpPrinterQueueList = NULL;
    }
}

// -----------------------------------------------------------------------

USHORT Printer::GetQueueCount()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    return (USHORT)(pSVData->maGDIData.mpPrinterQueueList->Count());
}

// -----------------------------------------------------------------------

const QueueInfo& Printer::GetQueueInfo( USHORT nQueue )
{
    return GetQueueInfo( nQueue, TRUE );
}

// -----------------------------------------------------------------------

const QueueInfo& Printer::GetQueueInfo( USHORT nQueue, BOOL bStatus )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    DBG_ASSERT( nQueue < pSVData->maGDIData.mpPrinterQueueList->Count(),
                "Printer::GetQueueInfo() - nQueue > QueueCount" );

    ImplPrnQueueData* pInfo = pSVData->maGDIData.mpPrinterQueueList->Get( nQueue );

#ifndef REMOTE_APPSERVER
    if ( bStatus )
        pSVData->mpDefInst->GetPrinterQueueState( pInfo->mpSalQueueInfo );
#else
    // ???
#endif

    if ( !pInfo->mpQueueInfo )
        pInfo->mpQueueInfo = new QueueInfo;

    pInfo->mpQueueInfo->maPrinterName   = pInfo->mpSalQueueInfo->maPrinterName;
    pInfo->mpQueueInfo->maDriver        = pInfo->mpSalQueueInfo->maDriver;
    pInfo->mpQueueInfo->maLocation      = pInfo->mpSalQueueInfo->maLocation;
    pInfo->mpQueueInfo->maComment       = pInfo->mpSalQueueInfo->maComment;
    pInfo->mpQueueInfo->mnStatus        = pInfo->mpSalQueueInfo->mnStatus;
    pInfo->mpQueueInfo->mnJobs          = pInfo->mpSalQueueInfo->mnJobs;
    return *pInfo->mpQueueInfo;
}

// -----------------------------------------------------------------------

XubString Printer::GetDefaultPrinterName()
{
    ImplSVData* pSVData = ImplGetSVData();

#ifndef REMOTE_APPSERVER
    return pSVData->mpDefInst->GetDefaultPrinter();
#else
    return pSVData->mpPrinterEnvironment->DefaultPrinter;
#endif
}

// =======================================================================

void Printer::ImplInitData()
{
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
#ifdef REMOTE_APPSERVER
    mpRemotePages = new ::std::vector< PrinterPage* >();
#endif
}

// -----------------------------------------------------------------------

void Printer::ImplInit( SalPrinterQueueInfo* pInfo )
{
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

    ImplSVData* pSVData = ImplGetSVData();

    // Printernamen merken
    maPrinterName = pInfo->maPrinterName;
    maDriver = pInfo->maDriver;

    // In JobSetup den Printernamen eintragen
    pJobSetup->maPrinterName = maPrinterName;
    pJobSetup->maDriver = maDriver;

#ifndef REMOTE_APPSERVER
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
#else

    mpInfoPrinter = new RmPrinter;

    Reference< XRmPrinter > xPrinter;

    if (pSVData->mxClientFactory.is() )
    {
        xPrinter = Reference< XRmPrinter >( pSVData->mxClientFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OfficePrinter.stardiv.de" ) ) ), NMSP_UNO::UNO_QUERY );
        mpInfoPrinter->SetInterface( xPrinter );
    }

    if ( ! xPrinter.is() )
    {
        delete mpInfoPrinter;
        mpInfoPrinter = NULL;
        ImplInitDisplay( NULL );
        return;
    }
    else
    {
        QueueInfo               aQInfo;
        RmJobSetup              aRmJobSetup;
        const REF( XRmPrinter )&    rxPrinter = mpInfoPrinter->GetInterface();

        const RmQueueInfo*      pInfos = pSVData->mpPrinterEnvironment->Infos.getConstArray();
        const RmQueueInfo*      pSelectedInfo = NULL;
        ::rtl::OUString aCompare( maPrinterName );
        for( int i = 0; i < pSVData->mpPrinterEnvironment->Infos.getLength(); i++ )
        {
            if( pInfos[i].PrinterName == aCompare )
            {
                pSelectedInfo = pInfos+i;
                break;
            }
        }

        if( pSelectedInfo )
        {
            mpInfoPrinter->CreateInfoInstance( *pSelectedInfo, aRmJobSetup );
            maJobSetup = aRmJobSetup;
        }

        if( rxPrinter.is() )
        {
            mpGraphics = new ImplServerGraphics( pSVData->mpAtoms );
            REF( XRmOutputDevice ) aTmp( rxPrinter, UNO_QUERY );
            mpGraphics->SetInterface( aTmp );
        }

        if( !mpGraphics->GetInterface().is() )
        {
            delete mpGraphics, mpGraphics = NULL;
            delete mpInfoPrinter, mpInfoPrinter = NULL;
            ImplInitDisplay( NULL );
            return;
        }
        else
        {
            ImplGetServerGraphics();
        }
    }
#endif

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

#ifdef REMOTE_APPSERVER
    mpGraphics          = mpDisplayDev->mpGraphics;
#endif
}

// -----------------------------------------------------------------------

SalPrinterQueueInfo* Printer::ImplGetQueueInfo( const XubString& rPrinterName,
                                                const XubString* pDriver )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    ImplPrnQueueList* pPrnList = pSVData->maGDIData.mpPrinterQueueList;
    if ( pPrnList && pPrnList->Count() )
    {
        // Zuerst suchen wir nach dem Printer-Namen
        ImplPrnQueueData* pBestInfo = NULL;
        ImplPrnQueueData* pInfo = pPrnList->First();
        while ( pInfo )
        {
            if ( pInfo->mpSalQueueInfo->maPrinterName == rPrinterName )
            {
                pBestInfo = pInfo;
                if ( !pDriver || (pInfo->mpSalQueueInfo->maDriver == *pDriver) )
                    return pInfo->mpSalQueueInfo;
            }
            pInfo = pPrnList->Next();
        }

        // Wenn wir einen PrinterNamen gefunden haben und nur der Treiber
        // nicht passt, nehmen wir diesen
        if ( pBestInfo )
            return pBestInfo->mpSalQueueInfo;

        // Dann suchen wir caseinsensitiv
        pInfo = pPrnList->First();
        while ( pInfo )
        {
            if ( pInfo->mpSalQueueInfo->maPrinterName.EqualsIgnoreCaseAscii( rPrinterName ) )
            {
                pBestInfo = pInfo;
                if ( !pDriver || pInfo->mpSalQueueInfo->maDriver.EqualsIgnoreCaseAscii( *pDriver ) )
                    return pInfo->mpSalQueueInfo;
            }
            pInfo = pPrnList->Next();
        }

        // Wenn wir einen PrinterNamen gefunden haben und nur der Treiber
        // nicht passt, nehmen wir diesen
        if ( pBestInfo )
            return pBestInfo->mpSalQueueInfo;

        // Und wenn wir immer noch keinen gefunden haben, suchen wir
        // noch nach einem passenden Treiber
        if ( pDriver )
        {
            pInfo = pPrnList->First();
            while ( pInfo )
            {
                if ( pInfo->mpSalQueueInfo->maDriver == *pDriver )
                    return pInfo->mpSalQueueInfo;
                pInfo = pPrnList->Next();
            }
        }

        // Und wenn wir immer noch keinen gefunden, dann wir der
        // Default-Drucker genommen
        XubString aPrinterName = GetDefaultPrinterName();
        pInfo = pPrnList->First();
        while ( pInfo )
        {
            if ( pInfo->mpSalQueueInfo->maPrinterName == aPrinterName )
                return pInfo->mpSalQueueInfo;
            pInfo = pPrnList->Next();
        }

        // Und wenn wir diesen auch nicht finden, nehmen wir den ersten
        // in der Liste, denn einige Installationen zerstoeren den
        // Namen und andere Programme weichen dann normalerweise noch
        // auf irgendeinen Drucker aus
        pInfo = pPrnList->First();
        if ( pInfo )
            return pInfo->mpSalQueueInfo;
    }

    return NULL;
}

// -----------------------------------------------------------------------

void Printer::ImplUpdatePageData()
{
#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !ImplGetGraphics() )
        return;

    mpGraphics->GetResolution( mnDPIX, mnDPIY );
    mpInfoPrinter->GetPageInfo( maJobSetup.ImplGetConstData(),
                                mnOutWidth, mnOutHeight,
                                maPageOffset.X(), maPageOffset.Y(),
                                maPaperSize.Width(), maPaperSize.Height() );
#else
    if ( mpInfoPrinter && mpGraphics )
    {
        mpGraphics->GetResolution( mnDPIX, mnDPIY );
        mpInfoPrinter->GetPageInfo( mnOutWidth, mnOutHeight,
                                    maPageOffset.X(), maPageOffset.Y(),
                                    maPaperSize.Width(), maPaperSize.Height() );
    }
#endif
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

    delete mpPrinterOptions;

#ifndef REMOTE_APPSERVER
    ImplReleaseGraphics();
    if ( mpInfoPrinter )
        ImplGetSVData()->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
#else
    if ( mpInfoPrinter )
    {
        if( mpGraphics ) {
           REF( XRmOutputDevice ) aTmp;
            mpGraphics->SetInterface( aTmp );
        }

        ImplReleaseServerGraphics();

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
        delete mpGraphics, mpGraphics = NULL;
        delete mpInfoPrinter, mpInfoPrinter = NULL;
    }
    for( int i = 0; i < mpRemotePages->size(); i++ )
        delete (*mpRemotePages)[i];
    delete mpRemotePages;
#endif
    if ( mpDisplayDev )
        delete mpDisplayDev;
    else
    {
        // OutputDevice-Dtor versucht das gleiche, deshalb muss hier
        // der FontEntry auch auf NULL gesetzt werden
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

ULONG Printer::GetCapabilities( USHORT nType ) const
{
    if ( IsDisplayPrinter() )
        return FALSE;

#ifndef REMOTE_APPSERVER
    return mpInfoPrinter->GetCapabilities( maJobSetup.ImplGetConstData(), nType );
#else
    return mpInfoPrinter->GetCapabilities( nType );
#endif
}

// -----------------------------------------------------------------------

BOOL Printer::HasSupport( PrinterSupport eFeature, BOOL bInJob ) const
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

#ifndef REMOTE_APPSERVER
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
#else
    if ( mpInfoPrinter )
    {
        RmJobSetup aRmJobSetup;

        aJobSetup.SetRmJobSetup( aRmJobSetup );
        if ( mpInfoPrinter->SetJobSetup( aRmJobSetup ) )
        {
            mbNewJobSetup = TRUE;
            maJobSetup = aRmJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return TRUE;
        }
        else
            return FALSE;
    }
    return FALSE;
#endif
}

// -----------------------------------------------------------------------

#ifdef REMOTE_APPSERVER
IMPL_LINK( Printer, UserSetupCompleted, ::com::sun::star::uno::Any*, pResult )
{
    ::vos::OGuard guard( Application::GetSolarMutex( ) );

    if( pResult->hasValue() )
    {
        mbUserSetupResult = TRUE;

        ::com::sun::star::portal::client::RmJobSetup aRmJobSetup;
        *pResult >>= aRmJobSetup;
        JobSetup aJobSetup;
        aJobSetup = aRmJobSetup;
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = TRUE;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
    }
    else
        mbUserSetupResult = FALSE;

    mbUserSetupCompleted = TRUE;
    return 0;
}
#endif

BOOL Printer::Setup( Window* pWindow )
{
    if ( IsDisplayPrinter() )
        return FALSE;

    if ( IsJobActive() || IsPrinting() )
        return FALSE;

#ifndef REMOTE_APPSERVER
    JobSetup aJobSetup = maJobSetup;
    SalFrame* pFrame;
    if ( !pWindow )
        pFrame = ImplGetDefaultWindow()->ImplGetFrame();
    else
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
#else
    RmJobSetup aRmJobSetup;
    maJobSetup.SetRmJobSetup( aRmJobSetup );
    mpInfoPrinter->SetJobSetup( aRmJobSetup );
    RmFrameWindow* pFrame;
    if ( !pWindow )
        pFrame = ImplGetDefaultWindow()->ImplGetFrame();
    else
        pFrame = pWindow->ImplGetFrame();
    mbUserSetupCompleted = FALSE;
    mpInfoPrinter->UserSetup( pFrame->GetFrameInterface(), pFrame->InsertUserEventLink( LINK( this, Printer, UserSetupCompleted ) ) );
    while( ! mbUserSetupCompleted )
        Application::Reschedule();
    return mbUserSetupResult;
#endif
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
#ifndef REMOTE_APPSERVER
            ImplReleaseGraphics();
            pSVData->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
#else
            if ( mpInfoPrinter )
            {
                if( mpGraphics ) {
                    REF( XRmOutputDevice ) aTmp;
                    mpGraphics->SetInterface( aTmp );
                }

                ImplReleaseServerGraphics();
                delete mpGraphics, mpGraphics = NULL;
                delete mpInfoPrinter, mpInfoPrinter = NULL;
            }
#endif
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
            mpFontList->Clear();
            delete mpFontList;
            mpFontList = NULL;

            delete mpFontCache;
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
#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
#endif
        if ( mpDisplayDev )
        {
            delete mpDisplayDev;
            mpDisplayDev = NULL;
        }
        else
        {
#ifndef REMOTE_APPSERVER
            pSVData->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
#else
            if ( mpInfoPrinter )
            {
                if( mpGraphics ) {
                    REF( XRmOutputDevice ) aTmp;
                    mpGraphics->SetInterface( aTmp );
                }

                ImplReleaseServerGraphics();
                delete mpGraphics, mpGraphics = NULL;
                delete mpInfoPrinter, mpInfoPrinter = NULL;
            }
#endif

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
            delete mpFontList;
            delete mpFontCache;
            mbInitFont = TRUE;
            mbNewFont = TRUE;
            mpInfoPrinter = NULL;
        }

        // Neuen Printer bauen
        XubString maDriver = pPrinter->GetDriverName();
        SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( pPrinter->GetName(), &maDriver );
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

#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
#else
        RmJobSetup aRmJobSetup;
        aJobSetup.SetRmJobSetup( aRmJobSetup );
        if ( mpInfoPrinter->SetOrientation( (unsigned short)eOrientation, aRmJobSetup ) )
        {
            aJobSetup = aRmJobSetup;
#endif
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

#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERBIN, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
#else
        RmJobSetup aRmJobSetup;
        aJobSetup.SetRmJobSetup( aRmJobSetup );
        if ( mpInfoPrinter->SetPaperBin( nPaperBin, aRmJobSetup ) )
        {
            aJobSetup = aRmJobSetup;
#endif
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
    const unsigned long PAPER_ACCURACY = 1; // 1.0 mm accuracy

    return ( (Abs( (short)(nPaperWidth1-nPaperWidth2) ) <= PAPER_ACCURACY ) &&
             (Abs( (short)(nPaperHeight1-nPaperHeight2) ) <= PAPER_ACCURACY ) );
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
            pSetupData->meOrientation = ORIENTATION_PORTRAIT;
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
                pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
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

#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
        if ( ePaper == PAPER_USER )
            ImplFindPaperFormatForUserSize( aJobSetup );
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
#else
        RmJobSetup aRmJobSetup;
        aJobSetup.SetRmJobSetup( aRmJobSetup );
        if ( mpInfoPrinter->SetPaper( (unsigned short)ePaper, aRmJobSetup ) )
        {
            aJobSetup = aRmJobSetup;
#endif
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

#ifndef REMOTE_APPSERVER
        ImplReleaseGraphics();
        ImplFindPaperFormatForUserSize( aJobSetup );

        // Changing the paper size can also change the orientation!
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
#else
        RmJobSetup aRmJobSetup;
        aJobSetup.SetRmJobSetup( aRmJobSetup );
        if ( mpInfoPrinter->SetPaperSizeUser( aPageSize.Width(), aPageSize.Height(), aRmJobSetup ) )
        {
            aJobSetup = aRmJobSetup;
#endif
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
    if( mpInfoPrinter->m_aPaperFormats.empty() || nPaper < 0 || nPaper >= mpInfoPrinter->m_aPaperFormats.size() )
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
    for( int i = 0; i < mpInfoPrinter->m_aPaperFormats.size(); i++ )
    {
        unsigned long nW = mpInfoPrinter->m_aPaperFormats[i].m_nPaperWidth;
        unsigned long nH = mpInfoPrinter->m_aPaperFormats[i].m_nPaperHeight;
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

#ifndef REMOTE_APPSERVER
    return (USHORT)mpInfoPrinter->GetPaperBinCount( maJobSetup.ImplGetConstData() );
#else
    if ( mpInfoPrinter )
        return mpInfoPrinter->GetPaperBinCount();
    else
        return 0;
#endif
}

// -----------------------------------------------------------------------

XubString Printer::GetPaperBinName( USHORT nPaperBin ) const
{
    if ( IsDisplayPrinter() )
        return ImplGetSVEmptyStr();

#ifndef REMOTE_APPSERVER
    if ( nPaperBin < GetPaperBinCount() )
        return mpInfoPrinter->GetPaperBinName( maJobSetup.ImplGetConstData(), nPaperBin );
    else
        return ImplGetSVEmptyStr();
#else
    if ( mpInfoPrinter )
        return (String)mpInfoPrinter->GetPaperBinName( nPaperBin );
    else
        return ImplGetSVEmptyStr();
#endif
}

// -----------------------------------------------------------------------

BOOL Printer::SetCopyCount( USHORT nCopy, BOOL bCollate )
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

#ifndef REMOTE_APPSERVER

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
    mpQPrinter->Destroy();
    mpQPrinter = NULL;
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

#endif

// -----------------------------------------------------------------------

BOOL Printer::StartJob( const XubString& rJobName )
{
    mnError = PRINTER_OK;

    if ( IsDisplayPrinter() )
        return FALSE;

    if ( IsJobActive() || IsPrinting() )
        return FALSE;

#ifndef REMOTE_APPSERVER
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

        if ( !mpPrinter->StartJob( pPrintFile, rJobName, Application::GetDisplayName(),
                                   nCopies, bCollateCopy,
                                   maJobSetup.ImplGetConstData() ) )
        {
            mnError = ImplSalPrinterErrorCodeToVCL( mpPrinter->GetErrorCode() );
            if ( !mnError )
                mnError = PRINTER_GENERALERROR;
            ImplSVData* pSVData = ImplGetSVData();
            pSVData->mpDefInst->DestroyPrinter( mpPrinter );
            mpPrinter = NULL;
            return FALSE;
        }

        mbNewJobSetup   = FALSE;
        maJobName       = rJobName;
        mnCurPage       = 1;
        mnCurPrintPage  = 1;
        mbJobActive     = TRUE;
        mbPrinting      = TRUE;
        StartPrint();
    }
    else
    {
        mpQPrinter = new ImplQPrinter( this );
        mpQPrinter->SetDigitLanguage( GetDigitLanguage() );
        mpQPrinter->SetUserCopy( bUserCopy );
        mpQPrinter->SetPrinterOptions( *mpPrinterOptions );
        if ( mpQPrinter->StartJob( rJobName ) )
        {
            mbNewJobSetup   = FALSE;
            maJobName       = rJobName;
            mnCurPage       = 1;
            mbJobActive     = TRUE;
            mbPrinting      = TRUE;
            StartPrint();
            mpQPrinter->StartQueuePrint();
        }
        else
        {
            mnError = mpQPrinter->GetErrorCode();
            mpQPrinter->Destroy();
            mpQPrinter = NULL;
            return FALSE;
        }
    }
#else
    mbNewJobSetup   = FALSE;
    maJobName       = rJobName;
    mnCurPage       = 1;
    mnCurPrintPage  = 1;
    mbJobActive     = TRUE;
    mbPrinting      = TRUE;
#endif

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL Printer::EndJob()
{
    if ( !IsJobActive() )
        return FALSE;

    DBG_ASSERT( !mbInPrintPage, "Printer::EndJob() - StartPage() without EndPage() called" );

    mbJobActive = FALSE;

#ifndef REMOTE_APPSERVER
    if ( mpPrinter || mpQPrinter )
    {
        ImplReleaseGraphics();

        mnCurPage = 0;

        if ( mpPrinter )
        {
            mbPrinting      = FALSE;
            mnCurPrintPage  = 0;
            maJobName.Erase();

            mbDevOutput = FALSE;
            mpPrinter->EndJob();
            // Hier den Drucker nicht asyncron zerstoeren, da es
            // W95 nicht verkraftet, wenn gleichzeitig gedruckt wird
            // und ein Druckerobjekt zerstoert wird
            ImplGetSVData()->mpDefInst->DestroyPrinter( mpPrinter );
            mpPrinter = NULL;
            EndPrint();
        }
        else
            mpQPrinter->EndQueuePrint();

        return TRUE;
    }
#else
    ImplSVData* pSVData = ImplGetSVData();
    sal_Bool bResult = sal_False;

    try
    {
        mpPrinter = new RmPrinter();
        mpPrinter->mxRemotePrinter = REF( XRmPrinter )( pSVData->mxClientFactory->createInstance( ::rtl::OUString::createFromAscii( "OfficePrinter.stardiv.de" ) ), NMSP_UNO::UNO_QUERY );
        RmJobSetup aRmJobSetup;
        maJobSetup.SetRmJobSetup( aRmJobSetup );
        mpPrinter->CreatePrintInstance( aRmJobSetup );
        if( mpPrinter->mxRemotePrinter.is() )
        {
            RmPageRequestor* pRequestor = new RmPageRequestor( this );
            Reference< XRmPageRequestor > xReq( pRequestor );
            CHECK_FOR_RVPSYNC_NORMAL();
            bResult = mpPrinter->mxRemotePrinter->StartJob( mnCopyCount,
                                                            mbCollateCopy,
                                                            maJobName,
                                                            maPrintFile,
                                                            mbPrintFile,
                                                            mpRemotePages->size(),
                                                            xReq );
            if( bResult )
                do Application::Reschedule(); while( mbPrinting && ! pRequestor->isCompleted() );
        }
    }
    catch( RuntimeException &e )
    {
        rvpExceptionHandler();
        bResult = FALSE;
        if( mpPrinter )
        {
            delete mpPrinter;
            mpPrinter = NULL;
        }
    }

    for( int i = 0; i < mpRemotePages->size(); i++ )
        delete (*mpRemotePages)[i];
    *mpRemotePages = ::std::vector< PrinterPage* >();

    if ( mpPrinter )
    {
        if( bResult )
        {
            CHECK_FOR_RVPSYNC_NORMAL();
            try
            {
                mpPrinter->mxRemotePrinter->EndJob( vcl::unohelper::GetMultiServiceFactory() );
            }
            catch( RuntimeException &e )
            {
                rvpExceptionHandler();
            }
        }
        mbPrinting = FALSE;
        mbDevOutput = FALSE;
        mnCurPage = 0;
        mnCurPrintPage = 0;
        maJobName.Erase();
        EndPrint();

        delete mpPrinter;
        mpPrinter = NULL;

        return bResult;
    }
#endif

    return FALSE;
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

#ifndef REMOTE_APPSERVER
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
#else
    mbPrinting      = FALSE;
    mbDevOutput     = FALSE;
    mnCurPage       = 0;
    mnCurPrintPage  = 0;
    maJobName.Erase();
    if( mpPrinter )
    {
        if( mpPrinter->mxRemotePrinter.is() )
        {
            CHECK_FOR_RVPSYNC_NORMAL();
            try
            {
                mpPrinter->mxRemotePrinter->AbortJob();
            }
            catch( RuntimeException &e )
            {
                rvpExceptionHandler();
            }
        }
        delete mpPrinter;
        mpPrinter = NULL;
    }
    EndPrint();

    return TRUE;
#endif

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::StartPage()
{
    if ( !IsJobActive() )
        return FALSE;

#ifndef REMOTE_APPSERVER
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
#else
        mpQMtf = new GDIMetaFile;
        mpQMtf->Record( this );
        mpQMtf->SaveStatus();

        mbInPrintPage = TRUE;
        mnCurPage++;
        mnCurPrintPage++;
        PrintPage();

        return TRUE;
#endif

    return FALSE;
}

// -----------------------------------------------------------------------

BOOL Printer::EndPage()
{
    if ( !IsJobActive() )
        return FALSE;

    mbInPrintPage = FALSE;

#ifndef REMOTE_APPSERVER
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
#else
    mpQMtf->Stop();
    mpQMtf->WindStart();
    mpRemotePages->push_back( new PrinterPage( mpQMtf, mbNewJobSetup, GetJobSetup() ) );
    mpQMtf = NULL;
    mbNewJobSetup = FALSE;

    return TRUE;
#endif

    return FALSE;
}

#ifdef REMOTE_APPSERVER
void Printer::GetRemotePageSetup( ULONG nPage, RmJobSetup& rSetup )
{
    if( nPage < mpRemotePages->size() )
        (*mpRemotePages)[nPage]->GetJobSetup().SetRmJobSetup( rSetup );
}

void Printer::PrintRemotePage( ULONG nPage )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "printing page %d of %d\n", nPage, mpRemotePages->size() );
#endif
    if ( mpPrinter && mpPrinter->mxRemotePrinter.is() )
    {
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "have printer\n", nPage );
#endif
        if( nPage >=  mpRemotePages->size() )
        {
            try
            {
                CHECK_FOR_RVPSYNC_NORMAL();
                mpPrinter->mxRemotePrinter->StartPage();
                CHECK_FOR_RVPSYNC_NORMAL();
                mpPrinter->mxRemotePrinter->EndPage();
            }
            catch( RuntimeException &e )
            {
                rvpExceptionHandler();
            }
            return;
        }

        if( mpGraphics ) {
            REF( XRmOutputDevice ) aTmp;
            mpGraphics->SetInterface( aTmp );
            delete mpGraphics;
        }

        PrinterPage* pPage = (*mpRemotePages)[nPage];

        CHECK_FOR_RVPSYNC_NORMAL();
        try
        {
            mpPrinter->mxRemotePrinter->StartPage();
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "page started\n" );
#endif
        }
        catch( RuntimeException &e )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "page started exception\n" );
#endif
            rvpExceptionHandler();
        }

        mpGraphics = new ImplServerGraphics( ImplGetSVData()->mpAtoms );
        Reference< XRmOutputDevice > temp( mpPrinter->mxRemotePrinter, UNO_QUERY );
        mpGraphics->SetInterface( temp );

        mbDevOutput     = TRUE;
        mbInPrintPage   = TRUE;
        mnCurPage       = nPage;
        mnCurPrintPage  = nPage;

        pPage->GetGDIMetaFile()->WindStart();
        pPage->GetGDIMetaFile()->Play( this );

#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "metafile played %d actions\n", pPage->GetGDIMetaFile()->GetActionCount() );
#endif

        CHECK_FOR_RVPSYNC_NORMAL();
        try
        {
            mpPrinter->mxRemotePrinter->EndPage();
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "page ended\n" );
#endif
        }
        catch( RuntimeException &e )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "page ended exception\n" );
#endif
            rvpExceptionHandler();
        }
        mbDevOutput = FALSE;
        mbNewJobSetup = FALSE;
    }
}
#endif
