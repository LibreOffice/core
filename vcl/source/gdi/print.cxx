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


#include <list>

#include <tools/debug.hxx>
#include <tools/resary.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>

#include <vcl/unohelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/print.hxx>

#include <salinst.hxx>
#include <salgdi.hxx>
#include <salptype.hxx>
#include <salprn.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <jobset.h>
#include <outdev.h>
#include <print.h>

#include <comphelper/processfactory.hxx>

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::configuration;

int nImplSysDialog = 0;

// =======================================================================

namespace
{
    static Paper ImplGetPaperFormat( long nWidth100thMM, long nHeight100thMM )
    {
        PaperInfo aInfo(nWidth100thMM, nHeight100thMM);
        aInfo.doSloppyFit();
        return aInfo.getPaper();
    }

// -----------------------------------------------------------------------

    static const PaperInfo& ImplGetEmptyPaper()
    {
        static PaperInfo aInfo(PAPER_USER);
        return aInfo;
    }
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
            PaperInfo aInfo(pConstData->mePaperFormat);
            pData->mnPaperWidth  = aInfo.getWidth();
            pData->mnPaperHeight = aInfo.getHeight();
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
    mbReduceTransparency( sal_False ),
    meReducedTransparencyMode( PRINTER_TRANSPARENCY_AUTO ),
    mbReduceGradients( sal_False ),
    meReducedGradientsMode( PRINTER_GRADIENT_STRIPES ),
    mnReducedGradientStepCount( 64 ),
    mbReduceBitmaps( sal_False ),
    meReducedBitmapMode( PRINTER_BITMAP_NORMAL ),
    mnReducedBitmapResolution( 200 ),
    mbReducedBitmapsIncludeTransparency( sal_True ),
    mbConvertToGreyscales( sal_False )
{
}

// -----------------------------------------------------------------------

PrinterOptions::~PrinterOptions()
{
}

#define PROPERTYNAME_REDUCETRANSPARENCY                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceTransparency"))
#define PROPERTYNAME_REDUCEDTRANSPARENCYMODE            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedTransparencyMode"))
#define PROPERTYNAME_REDUCEGRADIENTS                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceGradients"))
#define PROPERTYNAME_REDUCEDGRADIENTMODE                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedGradientMode"))
#define PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT           rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedGradientStepCount"))
#define PROPERTYNAME_REDUCEBITMAPS                      rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReduceBitmaps"))
#define PROPERTYNAME_REDUCEDBITMAPMODE                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapMode"))
#define PROPERTYNAME_REDUCEDBITMAPRESOLUTION            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapResolution"))
#define PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReducedBitmapIncludesTransparency"))
#define PROPERTYNAME_CONVERTTOGREYSCALES                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ConvertToGreyscales"))

bool PrinterOptions::ReadFromConfig( bool i_bFile )
{
    bool bSuccess = false;
    // save old state in case something goes wrong
    PrinterOptions aOldValues( *this );

    // get the configuration service
    Reference< XMultiServiceFactory > xConfigProvider;
    Reference< XNameAccess > xConfigAccess;
    try
    {
        // get service provider
        Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
        // create configuration hierachical access name
        try
        {
            xConfigProvider = theDefaultProvider::get( xContext );

            Sequence< Any > aArgs(1);
            PropertyValue aVal;
            aVal.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
            if( i_bFile )
                aVal.Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common/Print/Option/File" ) );
            else
                aVal.Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common/Print/Option/Printer" ) );
            aArgs.getArray()[0] <<= aVal;
            xConfigAccess = Reference< XNameAccess >(
                    xConfigProvider->createInstanceWithArguments(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" )), aArgs ),
                        UNO_QUERY );
            if( xConfigAccess.is() )
            {
                Reference< XPropertySet > xSet( xConfigAccess, UNO_QUERY );
                if( xSet.is() )
                {
                    sal_Int32 nValue = 0;
                    sal_Bool  bValue = 0;
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCETRANSPARENCY) >>= bValue )
                        SetReduceTransparency( bValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEDTRANSPARENCYMODE) >>= nValue )
                        SetReducedTransparencyMode( (PrinterTransparencyMode)nValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEGRADIENTS) >>= bValue )
                        SetReduceGradients( bValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEDGRADIENTMODE) >>= nValue )
                        SetReducedGradientMode( (PrinterGradientMode)nValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT) >>= nValue )
                        SetReducedGradientStepCount( (sal_uInt16)nValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEBITMAPS) >>= bValue )
                        SetReduceBitmaps( bValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPMODE) >>= nValue )
                        SetReducedBitmapMode( (PrinterBitmapMode)nValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPRESOLUTION) >>= nValue )
                        SetReducedBitmapResolution( (sal_uInt16)nValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY) >>= bValue )
                        SetReducedBitmapIncludesTransparency( bValue );
                    if( xSet->getPropertyValue(PROPERTYNAME_CONVERTTOGREYSCALES) >>= bValue )
                        SetConvertToGreyscales( bValue );

                    bSuccess = true;
                }
            }
        }
        catch( const Exception& )
        {
        }
    }
    catch( const WrappedTargetException& )
    {
    }

    if( ! bSuccess )
        *this = aOldValues;
    return bSuccess;
}

void Printer::SetPrinterOptions( const PrinterOptions& i_rOptions )
{
    *mpPrinterOptions = i_rOptions;
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

    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStream, rInfo.maPrinterName, RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStream, rInfo.maDriver, RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStream, rInfo.maLocation, RTL_TEXTENCODING_UTF8);
    write_lenPrefixed_uInt8s_FromOUString<sal_uInt16>(rOStream, rInfo.maComment, RTL_TEXTENCODING_UTF8);
    rOStream << rInfo.mnStatus;
    rOStream << rInfo.mnJobs;

    return rOStream;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, QueueInfo& rInfo )
{
    VersionCompat aCompat( rIStream, STREAM_READ );

    rInfo.maPrinterName = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStream, RTL_TEXTENCODING_UTF8);
    rInfo.maDriver = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStream, RTL_TEXTENCODING_UTF8);
    rInfo.maLocation = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStream, RTL_TEXTENCODING_UTF8);
    rInfo.maComment = read_lenPrefixed_uInt8s_ToOUString<sal_uInt16>(rIStream, RTL_TEXTENCODING_UTF8);
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
    boost::unordered_map< rtl::OUString, sal_Int32, rtl::OUStringHash >::iterator it =
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
    boost::unordered_map<rtl::OUString,sal_Int32,rtl::OUStringHash>::iterator it =
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

    static const char* pEnv = getenv( "SAL_DISABLE_PRINTERLIST" );
    if( !pEnv || !*pEnv )
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
const QueueInfo* Printer::GetQueueInfo( const rtl::OUString& rPrinterName, bool bStatusUpdate )
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

rtl::OUString Printer::GetDefaultPrinterName()
{
    static const char* pEnv = getenv( "SAL_DISABLE_DEFAULTPRINTER" );
    if( !pEnv || !*pEnv )
    {
        ImplSVData* pSVData = ImplGetSVData();

        return pSVData->mpDefInst->GetDefaultPrinter();
    }
    return rtl::OUString();
}

// =======================================================================

void Printer::ImplInitData()
{
    mbDevOutput         = sal_False;
    meOutDevType        = OUTDEV_PRINTER;
    mbDefPrinter        = sal_False;
    mnError             = 0;
    mnCurPage           = 0;
    mnCurPrintPage      = 0;
    mnPageQueueSize     = 0;
    mnCopyCount         = 1;
    mbCollateCopy       = sal_False;
    mbPrinting          = sal_False;
    mbJobActive         = sal_False;
    mbPrintFile         = sal_False;
    mbInPrintPage       = sal_False;
    mbNewJobSetup       = sal_False;
    mpInfoPrinter       = NULL;
    mpPrinter           = NULL;
    mpDisplayDev        = NULL;
    mbIsQueuePrinter    = sal_False;
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
    mpFontCache = new ImplFontCache( sal_True );
    mpGraphics->GetDevFontList( mpFontList );
}

// -----------------------------------------------------------------------

void Printer::ImplInitDisplay( const Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    mpInfoPrinter       = NULL;
    mpPrinter           = NULL;
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

SalPrinterQueueInfo* Printer::ImplGetQueueInfo( const rtl::OUString& rPrinterName,
                                                const rtl::OUString* pDriver )
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
            if( pPrnList->m_aQueueInfos[i].mpSalQueueInfo->maPrinterName.equalsIgnoreAsciiCase( rPrinterName ) )
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
    ImplUpdateFontData( sal_True );
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
            mbDefPrinter = sal_True;
    }
    else
        ImplInitDisplay( NULL );
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

Printer::Printer( const rtl::OUString& rPrinterName )
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
void Printer::Compat_OldPrinterMetrics( bool bSet )
{
    // propagate flag
    if( mpInfoPrinter )
        mpInfoPrinter->m_bCompatMetrics = bSet;

    // get new font data
    ImplUpdateFontData( sal_True );
}

// -----------------------------------------------------------------------

sal_uLong Printer::GetCapabilities( sal_uInt16 nType ) const
{
    if ( IsDisplayPrinter() )
        return sal_False;

    if( mpInfoPrinter )
        return mpInfoPrinter->GetCapabilities( maJobSetup.ImplGetConstData(), nType );
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Printer::HasSupport( PrinterSupport eFeature ) const
{
    switch ( eFeature )
    {
        case SUPPORT_SET_ORIENTATION:
            return (sal_Bool)GetCapabilities( PRINTER_CAPABILITIES_SETORIENTATION );
        case SUPPORT_SET_PAPERBIN:
            return (sal_Bool)GetCapabilities( PRINTER_CAPABILITIES_SETPAPERBIN );
        case SUPPORT_SET_PAPERSIZE:
            return (sal_Bool)GetCapabilities( PRINTER_CAPABILITIES_SETPAPERSIZE );
        case SUPPORT_SET_PAPER:
            return (sal_Bool)GetCapabilities( PRINTER_CAPABILITIES_SETPAPER );
        case SUPPORT_COPY:
            return (GetCapabilities( PRINTER_CAPABILITIES_COPIES ) != 0);
        case SUPPORT_COLLATECOPY:
            return (GetCapabilities( PRINTER_CAPABILITIES_COLLATECOPIES ) != 0);
        case SUPPORT_SETUPDIALOG:
            return (sal_Bool)GetCapabilities( PRINTER_CAPABILITIES_SUPPORTDIALOG );
        case SUPPORT_FAX:
            return (sal_Bool) GetCapabilities( PRINTER_CAPABILITIES_FAX );
        case SUPPORT_PDF:
            return (sal_Bool) GetCapabilities( PRINTER_CAPABILITIES_PDF );
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetJobSetup( const JobSetup& rSetup )
{
    if ( IsDisplayPrinter() || mbInPrintPage )
        return sal_False;

    JobSetup aJobSetup = rSetup;

    ImplReleaseGraphics();
    if ( mpInfoPrinter->SetPrinterData( aJobSetup.ImplGetData() ) )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = sal_True;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------


sal_Bool Printer::Setup( Window* pWindow )
{
    if ( IsDisplayPrinter() )
        return sal_False;

    if ( IsJobActive() || IsPrinting() )
        return sal_False;

    JobSetup aJobSetup = maJobSetup;
    SalFrame* pFrame;
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    if( !pWindow )
        return sal_False;

    pFrame = pWindow->ImplGetFrame();
    ImplReleaseGraphics();
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode++;
    nImplSysDialog++;
    sal_Bool bSetup = mpInfoPrinter->Setup( pFrame, aJobSetup.ImplGetData() );
    pSVData->maAppData.mnModalMode--;
    nImplSysDialog--;
    if ( bSetup )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = sal_True;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetPrinterProps( const Printer* pPrinter )
{
    if ( IsJobActive() || IsPrinting() )
        return sal_False;

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

            mbInitFont = sal_True;
            mbNewFont = sal_True;
            mpInfoPrinter = NULL;
        }

        // Neuen Printer bauen
        ImplInitDisplay( NULL );
        return sal_True;
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
            mbInitFont = sal_True;
            mbNewFont = sal_True;
            mpInfoPrinter = NULL;
        }

        // Neuen Printer bauen
        rtl::OUString aDriver = pPrinter->GetDriverName();
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

    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetOrientation( Orientation eOrientation )
{
    if ( mbInPrintPage )
        return sal_False;

    if ( maJobSetup.ImplGetConstData()->meOrientation != eOrientation )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->meOrientation = eOrientation;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            return sal_True;
        }

        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return sal_True;
        }
        else
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

Orientation Printer::GetOrientation() const
{
    return maJobSetup.ImplGetConstData()->meOrientation;
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetPaperBin( sal_uInt16 nPaperBin )
{
    if ( mbInPrintPage )
        return sal_False;

    if ( (maJobSetup.ImplGetConstData()->mnPaperBin != nPaperBin) &&
         (nPaperBin < GetPaperBinCount()) )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mnPaperBin = nPaperBin;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            return sal_True;
        }

        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERBIN, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return sal_True;
        }
        else
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_uInt16 Printer::GetPaperBin() const
{
    return maJobSetup.ImplGetConstData()->mnPaperBin;
}

// -----------------------------------------------------------------------

// Map user paper format to a available printer paper formats
void Printer::ImplFindPaperFormatForUserSize( JobSetup& aJobSetup, bool bMatchNearest )
{
    ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();

    int     nLandscapeAngle = GetLandscapeAngle();
    int     nPaperCount     = GetPaperInfoCount();
    bool    bFound = false;

    PaperInfo aInfo(pSetupData->mnPaperWidth, pSetupData->mnPaperHeight);

    // Alle Papierformate vergleichen und ein passendes raussuchen
    for ( int i = 0; i < nPaperCount; i++ )
    {
        const PaperInfo& rPaperInfo = GetPaperInfo( i );

        if ( aInfo.sloppyEqual(rPaperInfo) )
        {
            pSetupData->mePaperFormat = ImplGetPaperFormat( rPaperInfo.getWidth(),
                                                            rPaperInfo.getHeight() );
            pSetupData->meOrientation = ORIENTATION_PORTRAIT;
            bFound = true;
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

        PaperInfo aRotatedInfo(pSetupData->mnPaperHeight, pSetupData->mnPaperWidth);

        for ( int i = 0; i < nPaperCount; i++ )
        {
            const PaperInfo& rPaperInfo = GetPaperInfo( i );

            if ( aRotatedInfo.sloppyEqual( rPaperInfo ) )
            {
                pSetupData->mePaperFormat = ImplGetPaperFormat( rPaperInfo.getWidth(),
                                                                rPaperInfo.getHeight() );
                pSetupData->meOrientation = ORIENTATION_LANDSCAPE;
                bFound = true;
                break;
            }
        }
    }

    if( ! bFound && bMatchNearest )
    {
         sal_Int64 nBestMatch = SAL_MAX_INT64;
         int nBestIndex = 0;
         Orientation eBestOrientation = ORIENTATION_PORTRAIT;
         for( int i = 0; i < nPaperCount; i++ )
         {
             const PaperInfo& rPaperInfo = GetPaperInfo( i );

             // check protrait match
             sal_Int64 nDX = pSetupData->mnPaperWidth - rPaperInfo.getWidth();
             sal_Int64 nDY = pSetupData->mnPaperHeight - rPaperInfo.getHeight();
             sal_Int64 nMatch = nDX*nDX + nDY*nDY;
             if( nMatch < nBestMatch )
             {
                 nBestMatch = nMatch;
                 nBestIndex = i;
                 eBestOrientation = ORIENTATION_PORTRAIT;
             }

             // check landscape match
             nDX = pSetupData->mnPaperWidth - rPaperInfo.getHeight();
             nDY = pSetupData->mnPaperHeight - rPaperInfo.getWidth();
             nMatch = nDX*nDX + nDY*nDY;
             if( nMatch < nBestMatch )
             {
                 nBestMatch = nMatch;
                 nBestIndex = i;
                 eBestOrientation = ORIENTATION_LANDSCAPE;
             }
         }
         const PaperInfo& rBestInfo = GetPaperInfo( nBestIndex );
         pSetupData->mePaperFormat = ImplGetPaperFormat( rBestInfo.getWidth(),
                                                         rBestInfo.getHeight() );
         pSetupData->meOrientation = eBestOrientation;
    }
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetPaper( Paper ePaper )
{
    if ( mbInPrintPage )
        return sal_False;

    if ( maJobSetup.ImplGetConstData()->mePaperFormat != ePaper )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mePaperFormat = ePaper;
        if ( ePaper != PAPER_USER )
        {
            PaperInfo aInfo(ePaper);
            pSetupData->mnPaperWidth  = aInfo.getWidth();
            pSetupData->mnPaperHeight = aInfo.getHeight();
        }

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            return sal_True;
        }

        ImplReleaseGraphics();
        if ( ePaper == PAPER_USER )
            ImplFindPaperFormatForUserSize( aJobSetup, false );
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return sal_True;
        }
        else
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetPaperSizeUser( const Size& rSize )
{
    return SetPaperSizeUser( rSize, false );
}

sal_Bool Printer::SetPaperSizeUser( const Size& rSize, bool bMatchNearest )
{
    if ( mbInPrintPage )
        return sal_False;

    Size    aPixSize = LogicToPixel( rSize );
    Size    aPageSize = PixelToLogic( aPixSize, MAP_100TH_MM );
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
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            return sal_True;
        }

        ImplReleaseGraphics();
        ImplFindPaperFormatForUserSize( aJobSetup, bMatchNearest );

        // Changing the paper size can also change the orientation!
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return sal_True;
        }
        else
            return sal_False;
    }

    return sal_True;
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

rtl::OUString Printer::GetPaperName( Paper ePaper )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpPaperNames )
    {
        pSVData->mpPaperNames = new boost::unordered_map< int, rtl::OUString >();
        if( ImplGetResMgr() )
        {
            ResStringArray aPaperStrings( VclResId( RID_STR_PAPERNAMES ) );
            static const int PaperIndex[] =
            {
                PAPER_A0, PAPER_A1, PAPER_A2, PAPER_A3, PAPER_A4, PAPER_A5,
                PAPER_B4_ISO, PAPER_B5_ISO, PAPER_LETTER, PAPER_LEGAL, PAPER_TABLOID,
                PAPER_USER, PAPER_B6_ISO, PAPER_ENV_C4, PAPER_ENV_C5, PAPER_ENV_C6, PAPER_ENV_C65,
                PAPER_ENV_DL, PAPER_SLIDE_DIA, PAPER_C, PAPER_D, PAPER_E,
                PAPER_EXECUTIVE, PAPER_FANFOLD_LEGAL_DE, PAPER_ENV_MONARCH, PAPER_ENV_PERSONAL,
                PAPER_ENV_9, PAPER_ENV_10, PAPER_ENV_11, PAPER_ENV_12, PAPER_KAI16,
                PAPER_KAI32, PAPER_KAI32BIG, PAPER_B4_JIS, PAPER_B5_JIS, PAPER_B6_JIS,
                PAPER_POSTCARD_JP
            };
            OSL_ENSURE( sal_uInt32(SAL_N_ELEMENTS(PaperIndex)) == aPaperStrings.Count(), "localized paper name count wrong" );
            for( int i = 0; i < int(SAL_N_ELEMENTS(PaperIndex)); i++ )
                (*pSVData->mpPaperNames)[PaperIndex[i]] = aPaperStrings.GetString(i);
        }
    }

    boost::unordered_map<int,rtl::OUString>::const_iterator it = pSVData->mpPaperNames->find( (int)ePaper );
    return (it != pSVData->mpPaperNames->end()) ? it->second : rtl::OUString();
}

// -----------------------------------------------------------------------

rtl::OUString Printer::GetPaperName( bool i_bPaperUser ) const
{
    Size  aPageSize = PixelToLogic( GetPaperSizePixel(), MAP_100TH_MM );
    Paper ePaper    = ImplGetPaperFormat( aPageSize.Width(), aPageSize.Height() );
    if( ePaper == PAPER_USER )
        ePaper = ImplGetPaperFormat( aPageSize.Height(), aPageSize.Width() );
    return (ePaper != PAPER_USER || i_bPaperUser ) ? GetPaperName( ePaper ) : rtl::OUString();
}

// -----------------------------------------------------------------------

const PaperInfo& Printer::GetPaperInfo( int nPaper ) const
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

sal_Bool Printer::SetDuplexMode( DuplexMode eDuplex )
{
    if ( mbInPrintPage )
        return sal_False;

    if ( maJobSetup.ImplGetConstData()->meDuplexMode != eDuplex )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->meDuplexMode = eDuplex;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            return sal_True;
        }

        ImplReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_DUPLEXMODE, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = sal_True;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return sal_True;
        }
        else
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

int Printer::GetLandscapeAngle() const
{
    return mpInfoPrinter ? mpInfoPrinter->GetLandscapeAngle( maJobSetup.ImplGetConstData() ) : 900;
}

// -----------------------------------------------------------------------

Paper Printer::GetPaper() const
{
    return maJobSetup.ImplGetConstData()->mePaperFormat;
}

// -----------------------------------------------------------------------

sal_uInt16 Printer::GetPaperBinCount() const
{
    if ( IsDisplayPrinter() )
        return 0;

    return (sal_uInt16)mpInfoPrinter->GetPaperBinCount( maJobSetup.ImplGetConstData() );
}

// -----------------------------------------------------------------------

rtl::OUString Printer::GetPaperBinName( sal_uInt16 nPaperBin ) const
{
    if ( IsDisplayPrinter() )
        return ImplGetSVEmptyStr();

    if ( nPaperBin < GetPaperBinCount() )
        return mpInfoPrinter->GetPaperBinName( maJobSetup.ImplGetConstData(), nPaperBin );
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

sal_Bool Printer::SetCopyCount( sal_uInt16 nCopy, sal_Bool bCollate )
{
    mnCopyCount = nCopy;
    mbCollateCopy = bCollate;
    return sal_True;
}

// -----------------------------------------------------------------------

void Printer::Error()
{
    maErrorHdl.Call( this );
}

// -----------------------------------------------------------------------


sal_uLong Printer::ImplSalPrinterErrorCodeToVCL( sal_uLong nError )
{
    sal_uLong nVCLError;
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

sal_Bool Printer::EndJob()
{
    sal_Bool bRet = sal_False;
    if ( !IsJobActive() )
        return bRet;

    DBG_ASSERT( !mbInPrintPage, "Printer::EndJob() - StartPage() without EndPage() called" );

    mbJobActive = sal_False;

    if ( mpPrinter )
    {
        ImplReleaseGraphics();

        mnCurPage = 0;

        bRet = sal_True;

        mbPrinting      = sal_False;
        mnCurPrintPage  = 0;
        maJobName = rtl::OUString();

        mbDevOutput = sal_False;
        bRet = mpPrinter->EndJob();
        // Hier den Drucker nicht asyncron zerstoeren, da es
        // W95 nicht verkraftet, wenn gleichzeitig gedruckt wird
        // und ein Druckerobjekt zerstoert wird
        ImplGetSVData()->mpDefInst->DestroyPrinter( mpPrinter );
        mpPrinter = NULL;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void Printer::ImplStartPage()
{
    if ( !IsJobActive() )
        return;

    if ( mpPrinter )
    {
        SalGraphics* pGraphics = mpPrinter->StartPage( maJobSetup.ImplGetConstData(), mbNewJobSetup );
        if ( pGraphics )
        {
            ImplReleaseGraphics();
            mpJobGraphics = pGraphics;
        }
        mbDevOutput = sal_True;

        // PrintJob not aborted ???
        if ( IsJobActive() )
        {
            mbInPrintPage = sal_True;
            mnCurPage++;
            mnCurPrintPage++;
        }
    }
}

// -----------------------------------------------------------------------

void Printer::ImplEndPage()
{
    if ( !IsJobActive() )
        return;

    mbInPrintPage = sal_False;

    if ( mpPrinter )
    {
        mpPrinter->EndPage();
        ImplReleaseGraphics();
        mbDevOutput = sal_False;

        mpJobGraphics = NULL;
        mbNewJobSetup = sal_False;
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
