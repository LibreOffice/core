/*************************************************************************
 *
 *  $RCSfile: print.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cd $ $Date: 2000-11-06 09:01:04 $
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

#else

#include "rmoutdev.hxx"
#include "rmprint.hxx"
#include "rmwindow.hxx"
#include "rvp.hxx"
#include <vos/mutex.hxx>
#include <unotools/processfactory.hxx>

using namespace ::com::sun::star::uno;

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

int nImplSysDialog = 0;

#define PRINTERSEQ_GET( _def_Seq, _def_Obj )                                                        \
{                                                                                                   \
    SvMemoryStream* _def_pStm = new SvMemoryStream( (char*)_def_Seq.getConstArray(), _def_Seq.getLength(), STREAM_READ );   \
    _def_pStm->SetCompressMode( COMPRESSMODE_FULL );                                                \
    *_def_pStm >> _def_Obj;                                                                         \
    delete _def_pStm;                                                                               \
}

#define PRINTERSEQ_SET( _def_Obj, _def_Seq, _def_Type  )                                \
{                                                                                       \
    SvMemoryStream* _def_pStm = new SvMemoryStream( 8192, 8192 );                       \
    _def_pStm->SetCompressMode( COMPRESSMODE_FULL );                                    \
    *_def_pStm << _def_Obj;                                                             \
    _def_Seq = _def_Type( (sal_Int8*) (_def_pStm)->GetData(), (_def_pStm)->Tell() );    \
    delete _def_pStm;                                                                   \
}

// =======================================================================

#define PAPER_SLOPPY    20
#define PAPER_COUNT     9

static long ImplPaperFormats[PAPER_COUNT*2] =
{
    29700, 42000,   // A3
    21000, 29700,   // A4
    14800, 21000,   // A5
    25000, 35300,   // B4
    17600, 25000,   // B5
    21600, 27900,   // Letter
    21600, 35600,   // Legal
    27900, 43100,   // Tabloid
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

// =======================================================================

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
    if ( pSVData->mpRemotePrinterList && pSVData->mpRemotePrinterList->GetPrinterCount() )
    {
        const ULONG nCount = pSVData->mpRemotePrinterList->GetPrinterCount();
        for( ULONG i = 0; i < nCount; i++ )
        {
            NMSP_CLIENT::RmQueueInfo    aRmQInfo;
            QueueInfo                   aQInfo;
            SalPrinterQueueInfo*        pNewInfo = new SalPrinterQueueInfo;

            RemotePrinterInfo* pPrinterInfo = pSVData->mpRemotePrinterList->GetPrinter( i );
            REF( NMSP_CLIENT::XRmPrinterEnvironment ) xPrinterEnv( pSVData->mpRemotePrinterList->GetPrinterEnv( pPrinterInfo->aServerName ) );

            if ( xPrinterEnv.is() )
            {
                CHECK_FOR_RVPSYNC_NORMAL()
                if ( xPrinterEnv->GetPrinterInfo( pPrinterInfo->aPrinterName.GetBuffer(), aRmQInfo ) )
                {
                    PRINTERSEQ_GET( aRmQInfo, aQInfo );
                    pNewInfo->maPrinterName = pPrinterInfo->GetFullName();
                    pNewInfo->maDriver      = aQInfo.GetDriver();
                    pNewInfo->maLocation    = aQInfo.GetLocation();
                    pNewInfo->maComment     = aQInfo.GetComment();
                    pNewInfo->mnStatus      = aQInfo.GetStatus();
                    pNewInfo->mnJobs        = aQInfo.GetJobs();
                    pNewInfo->mpSysData     = NULL;
                    pSVData->maGDIData.mpPrinterQueueList->Add( pNewInfo );
                }
            }
        }
    }
    if ( !pSVData->maGDIData.mpPrinterQueueList->Count() )
    {
        SalPrinterQueueInfo* pNewInfo   = new SalPrinterQueueInfo;
        pNewInfo->maPrinterName         = String::CreateFromAscii("No Printer");
        pNewInfo->mpSysData             = NULL;
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
    XubString aDefPrinterName;
    if ( pSVData->mpRemotePrinterList )
    {
        RemotePrinterInfo* pDefPrinter = pSVData->mpRemotePrinterList->GetDefaultPrinter();
        if ( pDefPrinter )
            aDefPrinterName = pDefPrinter->GetFullName();

    }
    return aDefPrinterName;
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
    // Testen, ob Treiber ueberhaupt mit dem JobSetup uebereinstimmt
    ImplJobSetup* pJobSetup = maJobSetup.ImplGetData();

    if ( pJobSetup->mpDriverData )
    {
        if ( (pJobSetup->maPrinterName != pInfo->maPrinterName) ||
             (pJobSetup->maDriver != pInfo->maDriver) )
        {
            delete pJobSetup->mpDriverData;
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

    String aPrinterName( maPrinterName.GetToken( 0, '@' ) );
    String aPrintServerName( maPrinterName.GetToken( 1, '@' ) );


    mpInfoPrinter = new RmPrinter;

    Reference< ::com::sun::star::lang::XMultiServiceFactory > xPrinterFactory;
    Reference< NMSP_CLIENT::XRmPrinter > xPrinter;

    if (pSVData->mpRemotePrinterList)
    {
//          if ( !aPrintServerName.Len() )
//              aPrintServerName = pSVData->mpRemotePrinterList->FindLocalPrintServer( aPrinterName );

        xPrinterFactory = pSVData->mpRemotePrinterList->GetServerFactory( aPrintServerName );
        if( xPrinterFactory.is() )
        {
            xPrinter = Reference< NMSP_CLIENT::XRmPrinter >( xPrinterFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OfficePrinter.stardiv.de" ) ) ), NMSP_UNO::UNO_QUERY );
            mpInfoPrinter->SetInterface( xPrinter );
        }
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
        QueueInfo                               aQInfo;
        NMSP_CLIENT::RmQueueInfo                aRmQInfo;
        NMSP_CLIENT::RmJobSetup                 aRmJobSetup;
        const REF( NMSP_CLIENT::XRmPrinter )&   rxPrinter = mpInfoPrinter->GetInterface();

        aQInfo.maPrinterName    = aPrinterName;
        aQInfo.maDriver         = pInfo->maDriver;
        aQInfo.maLocation       = pInfo->maLocation;
        aQInfo.maComment        = pInfo->maComment;
        aQInfo.mnStatus         = pInfo->mnStatus;
        aQInfo.mnJobs           = pInfo->mnJobs;

        PRINTERSEQ_SET( aQInfo, aRmQInfo, NMSP_CLIENT::RmQueueInfo );
        mpInfoPrinter->Create( aRmQInfo, aRmJobSetup );
        PRINTERSEQ_GET( aRmJobSetup, maJobSetup );

        if( rxPrinter.is() )
        {
            mpGraphics = new ImplServerGraphics( pSVData->mpRemotePrinterList->GetServerAtoms( aPrintServerName ) );
            mpGraphics->SetInterface( REF( NMSP_CLIENT::XRmOutputDevice )( rxPrinter, NMSP_UNO::UNO_QUERY ) );
        }

        if( !mpGraphics->GetInterface().is() )
        {
            delete mpGraphics, mpGraphics = NULL;
            delete mpInfoPrinter, mpInfoPrinter = NULL;
            ImplInitDisplay( NULL );
            return;
        }
    }
#endif

    // Daten initialisieren
    ImplUpdatePageData();
    mpFontList = new ImplDevFontList;
    mpFontCache = new ImplFontCache( TRUE );
    mpGraphics->GetDevFontList( mpFontList );
    mpFontList->InitStdFonts();
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

#ifndef REMOTE_APPSERVER
    ImplReleaseGraphics();
    if ( mpInfoPrinter )
        ImplGetSVData()->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
#else
    if ( mpInfoPrinter )
    {
        if( mpGraphics )
            mpGraphics->SetInterface( REF( NMSP_CLIENT::XRmOutputDevice )() );

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
        delete mpFontList;
        delete mpFontCache;
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
        NMSP_CLIENT::RmJobSetup aRmJobSetup;

        PRINTERSEQ_SET( aJobSetup, aRmJobSetup, NMSP_CLIENT::RmJobSetup );
        if ( mpInfoPrinter->SetJobSetup( aRmJobSetup ) )
        {
            PRINTERSEQ_GET( aRmJobSetup, aJobSetup );
            mbNewJobSetup = TRUE;
            maJobSetup = aJobSetup;
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
        PRNSEQ_GET( aRmJobSetup, aJobSetup );

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
    NMSP_CLIENT::RmJobSetup aRmJobSetup;
    PRNSEQ_SET( aRmJobSetup, maJobSetup );
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

    mbDefPrinter    = pPrinter->mbDefPrinter;
    maPrintFile     = pPrinter->maPrintFile;
    mbPrintFile     = pPrinter->mbPrintFile;
    mnCopyCount     = pPrinter->mnCopyCount;
    mbCollateCopy   = pPrinter->mbCollateCopy;
    mnPageQueueSize = pPrinter->mnPageQueueSize;

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
                if( mpGraphics )
                    mpGraphics->SetInterface( REF( NMSP_CLIENT::XRmOutputDevice )() );

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
                if( mpGraphics )
                    mpGraphics->SetInterface( REF( NMSP_CLIENT::XRmOutputDevice )() );

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
        NMSP_CLIENT::RmJobSetup aRmJobSetup;
        PRINTERSEQ_SET( aJobSetup, aRmJobSetup, NMSP_CLIENT::RmJobSetup );
        if ( mpInfoPrinter->SetOrientation( (unsigned short)eOrientation, aRmJobSetup ) )
        {
            PRINTERSEQ_GET( aRmJobSetup, aJobSetup );
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
        NMSP_CLIENT::RmJobSetup aRmJobSetup;
        PRINTERSEQ_SET( aJobSetup, aRmJobSetup, NMSP_CLIENT::RmJobSetup );
        if ( mpInfoPrinter->SetPaperBin( nPaperBin, aRmJobSetup ) )
        {
            PRINTERSEQ_GET( aRmJobSetup, aJobSetup );
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
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
#else
        NMSP_CLIENT::RmJobSetup aRmJobSetup;
        PRINTERSEQ_SET( aJobSetup, aRmJobSetup, NMSP_CLIENT::RmJobSetup );
        if ( mpInfoPrinter->SetPaper( (unsigned short)ePaper, aRmJobSetup ) )
        {
            PRINTERSEQ_GET( aRmJobSetup, aJobSetup );
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
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
#else
        NMSP_CLIENT::RmJobSetup aRmJobSetup;
        PRINTERSEQ_SET( aJobSetup, aRmJobSetup, NMSP_CLIENT::RmJobSetup );
        if ( mpInfoPrinter->SetPaperSizeUser( aPixSize.Width(), aPixSize.Height(), aRmJobSetup ) )
        {
            PRINTERSEQ_GET( aRmJobSetup, aJobSetup );
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

#else

// -----------------------------------------------------------------------

static void ImplDeletePrintSpooler( PrintSpooler* pPrintSpooler )
{
    if( pPrintSpooler != 0 )
    {
        pPrintSpooler->mxPrintSpooler = REF( NMSP_CLIENT::XRmPrintSpooler )();
    }
}

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
    else if ( nCopies > 1 )
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
            if ( !mnPageQueueSize )
                mnPageQueueSize = 1;
        }
    }
    else
        bCollateCopy = FALSE;

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
        mpQPrinter->SetUserCopy( bUserCopy );
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
    BOOL                bResult = FALSE;
    ImplSVData* pSVData = ImplGetSVData();

    String aServerName( maPrinterName.GetToken( 1, '@' ) );

    // build a new connection if not client
    Reference< ::com::sun::star::lang::XMultiServiceFactory > xServerFactory;
    Reference< ::com::sun::star::connection::XConnection > xConnection;
    Reference< ::com::sun::star::bridge::XBridgeFactory > xBridgeFactory;

    pSVData->mpRemotePrinterList->CreateNewPrinterConnection( maPrinterName.GetToken( 1, '@' ), xServerFactory, xConnection, xBridgeFactory );
    if( xServerFactory.is() )
    {
        REF( NMSP_CLIENT::XRmSpoolLauncher ) xLauncher( xServerFactory->createInstance( ::rtl::OUString::createFromAscii( "OfficeSpoolLauncher.stardiv.de" ) ), NMSP_UNO::UNO_QUERY );
        if( xLauncher.is() )
        {
            CHECK_FOR_RVPSYNC_NORMAL()
            if ( xLauncher->LaunchSpooler( pSVData->mpUserInfo->sName,
                                      pSVData->mpUserInfo->sPassword ) )
            {
                if( xBridgeFactory.is() ) // else this a the client connection
                {
                    // get objects from transferred sospool connection
                    xServerFactory = Reference< ::com::sun::star::lang::XMultiServiceFactory >();
                    Reference< ::com::sun::star::bridge::XBridge >
                    xBridge( xBridgeFactory->createBridge( ::rtl::OUString(), ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "iiop" ) ), xConnection, Reference< ::com::sun::star::bridge::XInstanceProvider >() ) );
                    xServerFactory = Reference< ::com::sun::star::lang::XMultiServiceFactory >( xBridge->getInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SpoolMultiServiceFactory.sprintd.daemons.stardiv.de" ) ) ), UNO_QUERY );
                }

                mpPrinter = new PrintSpooler();
                mpPrinter->mxPrintSpooler = REF( NMSP_CLIENT::XRmPrintSpooler )( xServerFactory->createInstance( ::rtl::OUString::createFromAscii( "OfficePrintSpooler.stardiv.de" ) ), NMSP_UNO::UNO_QUERY );
                if( mpPrinter->mxPrintSpooler.is() )
                {
                    NMSP_CLIENT::RmJobSetup aRmJobSetup;
                    PRNSEQ_SET( aRmJobSetup, maJobSetup );
                    CHECK_FOR_RVPSYNC_NORMAL()
                    mpPrinter->mxPrintSpooler->Create( aRmJobSetup );
                    CHECK_FOR_RVPSYNC_NORMAL()
                    bResult = mpPrinter->mxPrintSpooler->StartJob( mnCopyCount, mbCollateCopy, rJobName, maPrintFile, mbPrintFile );
                }
            }
        }

        if ( bResult )
        {
            mbNewJobSetup   = FALSE;
            maJobName       = rJobName;
            mnCurPage       = 1;
            mnCurPrintPage  = 1;
            mbJobActive     = TRUE;
            mbPrinting      = TRUE;
        }
        else if ( mpPrinter && mpPrinter->mxPrintSpooler.is() )
        {
            ImplDeletePrintSpooler( mpPrinter );
            mpPrinter = NULL;
        }
    }

    if ( !bResult )
        mnError = PRINTER_GENERALERROR;

    return bResult;
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
    if ( mpPrinter )
    {
        CHECK_FOR_RVPSYNC_NORMAL()
        mpPrinter->mxPrintSpooler->EndJob( ::utl::getProcessServiceFactory() );
        mbPrinting = FALSE;
        mnCurPage = 0;
        mnCurPrintPage = 0;
        maJobName.Erase();
        EndPrint();

        ImplDeletePrintSpooler( mpPrinter );
        mpPrinter = NULL;

        return TRUE;
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
    if ( mpPrinter )
    {
        CHECK_FOR_RVPSYNC_NORMAL()
        mpPrinter->mxPrintSpooler->AbortJob();
        if ( mpQMtf )
        {
            mpQMtf->Clear();
            delete mpQMtf;
            mpQMtf = NULL;
        }

        mbPrinting      = FALSE;
        mnCurPage       = 0;
        mnCurPrintPage  = 0;
        maJobName.Erase();
        ImplDeletePrintSpooler( mpPrinter );
        mpPrinter = NULL;
        EndPrint();

        return TRUE;
    }
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
    if ( mpPrinter )
    {
        mpQMtf = new GDIMetaFile;
        mpQMtf->Record( this );
        mpQMtf->SaveStatus();

        mbInPrintPage = TRUE;
        mnCurPage++;
        mnCurPrintPage++;
        PrintPage();

        return TRUE;
    }
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
    if ( mpPrinter && mpQMtf )
    {
        mpQMtf->Stop();
        mpQMtf->WindStart();

        PrinterPage                 aPage( mpQMtf, mbNewJobSetup, GetJobSetup() );
        NMSP_CLIENT::RmPrinterPage  aRmPage;
        PRINTERSEQ_SET( aPage, aRmPage, NMSP_CLIENT::RmPrinterPage );
        CHECK_FOR_RVPSYNC_NORMAL()
        mpPrinter->mxPrintSpooler->SpoolPage( aRmPage );
        mpQMtf = NULL;
        mbNewJobSetup = FALSE;

        return TRUE;
    }
#endif

    return FALSE;
}
