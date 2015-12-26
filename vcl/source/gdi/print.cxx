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

#include <list>

#include <tools/debug.hxx>
#include <tools/resary.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>

#include <vcl/unohelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/print.hxx>

#include <salinst.hxx>
#include <salvd.hxx>
#include <salgdi.hxx>
#include <salptype.hxx>
#include <salprn.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <jobset.h>
#include <outdev.h>
#include "PhysicalFontCollection.hxx"
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

namespace
{
    static Paper ImplGetPaperFormat( long nWidth100thMM, long nHeight100thMM )
    {
        PaperInfo aInfo(nWidth100thMM, nHeight100thMM);
        aInfo.doSloppyFit();
        return aInfo.getPaper();
    }

    static const PaperInfo& ImplGetEmptyPaper()
    {
        static PaperInfo aInfo(PAPER_USER);
        return aInfo;
    }
}

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

// PrinterOptions
PrinterOptions::PrinterOptions() :
    mbReduceTransparency( false ),
    meReducedTransparencyMode( PRINTER_TRANSPARENCY_AUTO ),
    mbReduceGradients( false ),
    meReducedGradientsMode( PRINTER_GRADIENT_STRIPES ),
    mnReducedGradientStepCount( 64 ),
    mbReduceBitmaps( false ),
    meReducedBitmapMode( PRINTER_BITMAP_NORMAL ),
    mnReducedBitmapResolution( 200 ),
    mbReducedBitmapsIncludeTransparency( true ),
    mbConvertToGreyscales( false ),
    mbPDFAsStandardPrintJobFormat( false )
{
}

PrinterOptions::~PrinterOptions()
{
}

#define PROPERTYNAME_REDUCETRANSPARENCY                 "ReduceTransparency"
#define PROPERTYNAME_REDUCEDTRANSPARENCYMODE            "ReducedTransparencyMode"
#define PROPERTYNAME_REDUCEGRADIENTS                    "ReduceGradients"
#define PROPERTYNAME_REDUCEDGRADIENTMODE                "ReducedGradientMode"
#define PROPERTYNAME_REDUCEDGRADIENTSTEPCOUNT           "ReducedGradientStepCount"
#define PROPERTYNAME_REDUCEBITMAPS                      "ReduceBitmaps"
#define PROPERTYNAME_REDUCEDBITMAPMODE                  "ReducedBitmapMode"
#define PROPERTYNAME_REDUCEDBITMAPRESOLUTION            "ReducedBitmapResolution"
#define PROPERTYNAME_REDUCEDBITMAPINCLUDESTRANSPARENCY  "ReducedBitmapIncludesTransparency"
#define PROPERTYNAME_CONVERTTOGREYSCALES                "ConvertToGreyscales"
#define PROPERTYNAME_PDFASSTANDARDPRINTJOBFORMAT        "PDFAsStandardPrintJobFormat"

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
        // create configuration hierarchical access name
        try
        {
            xConfigProvider = theDefaultProvider::get( xContext );

            Sequence< Any > aArgs(1);
            PropertyValue aVal;
            aVal.Name = "nodepath";
            if( i_bFile )
                aVal.Value <<= OUString( "/org.openoffice.Office.Common/Print/Option/File" );
            else
                aVal.Value <<= OUString( "/org.openoffice.Office.Common/Print/Option/Printer" );
            aArgs.getArray()[0] <<= aVal;
            xConfigAccess.set(
                    xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess", aArgs ),
                        UNO_QUERY );
            if( xConfigAccess.is() )
            {
                Reference< XPropertySet > xSet( xConfigAccess, UNO_QUERY );
                if( xSet.is() )
                {
                    sal_Int32 nValue = 0;
                    bool  bValue = false;
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
                    if( xSet->getPropertyValue(PROPERTYNAME_PDFASSTANDARDPRINTJOBFORMAT) >>= bValue )
                        SetPDFAsStandardPrintJobFormat( bValue );

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

bool Printer::DrawTransformBitmapExDirect(
    const basegfx::B2DHomMatrix& /*aFullTransform*/,
    const BitmapEx& /*rBitmapEx*/)
{
    // printers can't draw bitmaps directly
    return false;
}

bool Printer::TransformAndReduceBitmapExToTargetRange(
    const basegfx::B2DHomMatrix& /*aFullTransform*/,
    basegfx::B2DRange& /*aVisibleRange*/,
    double& /*fMaximumArea*/)
{
    // deliberately do nothing - you can't reduce the
    // target range for a printer at all
    return true;
}

void Printer::DrawDeviceBitmap( const Point& rDestPt, const Size& rDestSize,
                                const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                BitmapEx& rBmpEx )
{
    if( rBmpEx.IsAlpha() )
    {
        // #107169# For true alpha bitmaps, no longer masking the
        // bitmap, but perform a full alpha blend against a white
        // background here.
        Bitmap aBmp( rBmpEx.GetBitmap() );
        aBmp.Blend( rBmpEx.GetAlpha(), Color( COL_WHITE) );
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp );
    }
    else
    {
        Bitmap aBmp( rBmpEx.GetBitmap() ), aMask( rBmpEx.GetMask() );
        aBmp.Replace( aMask, Color( COL_WHITE ) );
        ImplPrintTransparent( aBmp, aMask, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
    }
}

void Printer::EmulateDrawTransparent ( const tools::PolyPolygon& rPolyPoly,
                                       sal_uInt16 nTransparencePercent )
{
    // #110958# Disable alpha VDev, we perform the necessary
    VirtualDevice* pOldAlphaVDev = mpAlphaVDev;

    // operation explicitly further below.
    if( mpAlphaVDev )
        mpAlphaVDev = nullptr;

    GDIMetaFile* pOldMetaFile = mpMetaFile;
    mpMetaFile = nullptr;

    mpMetaFile = pOldMetaFile;

    // #110958# Restore disabled alpha VDev
    mpAlphaVDev = pOldAlphaVDev;

    Rectangle       aPolyRect( LogicToPixel( rPolyPoly ).GetBoundRect() );
    const Size      aDPISize( LogicToPixel( Size( 1, 1 ), MAP_INCH ) );
    const long      nBaseExtent = std::max( FRound( aDPISize.Width() / 300. ), 1L );
    long            nMove;
    const sal_uInt16    nTrans = ( nTransparencePercent < 13 ) ? 0 :
        ( nTransparencePercent < 38 ) ? 25 :
        ( nTransparencePercent < 63 ) ? 50 :
        ( nTransparencePercent < 88 ) ? 75 : 100;

    switch( nTrans )
    {
        case( 25 ): nMove = nBaseExtent * 3; break;
        case( 50 ): nMove = nBaseExtent * 4; break;
        case( 75 ): nMove = nBaseExtent * 6; break;

            // #i112959#  very transparent (88 < nTransparencePercent <= 99)
        case( 100 ): nMove = nBaseExtent * 8; break;

            // #i112959# not transparent (nTransparencePercent < 13)
        default:    nMove = 0; break;
    }

    Push( PushFlags::CLIPREGION | PushFlags::LINECOLOR );
    IntersectClipRegion(vcl::Region(rPolyPoly));
    SetLineColor( GetFillColor() );
    const bool bOldMap = mbMap;
    EnableMapMode( false );

    if(nMove)
    {
        Rectangle aRect( aPolyRect.TopLeft(), Size( aPolyRect.GetWidth(), nBaseExtent ) );
        while( aRect.Top() <= aPolyRect.Bottom() )
        {
            DrawRect( aRect );
            aRect.Move( 0, nMove );
        }

        aRect = Rectangle( aPolyRect.TopLeft(), Size( nBaseExtent, aPolyRect.GetHeight() ) );
        while( aRect.Left() <= aPolyRect.Right() )
        {
            DrawRect( aRect );
            aRect.Move( nMove, 0 );
        }
    }
    else
    {
        // #i112959# if not transparent, draw full rectangle in clip region
        DrawRect( aPolyRect );
    }

    EnableMapMode( bOldMap );
    Pop();

    mpMetaFile = pOldMetaFile;

    // #110958# Restore disabled alpha VDev
    mpAlphaVDev = pOldAlphaVDev;
}

void Printer::DrawOutDev( const Point& /*rDestPt*/, const Size& /*rDestSize*/,
                               const Point& /*rSrcPt*/,  const Size& /*rSrcSize*/ )
{
    DBG_ASSERT( false, "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );
}

void Printer::DrawOutDev( const Point& /*rDestPt*/, const Size& /*rDestSize*/,
                               const Point& /*rSrcPt*/,  const Size& /*rSrcSize*/,
                               const OutputDevice& /*rOutDev*/ )
{
    DBG_ASSERT( false, "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );
}

void Printer::CopyArea( const Point& /*rDestPt*/,
                        const Point& /*rSrcPt*/,  const Size& /*rSrcSize*/,
                        bool /*bWindowInvalidate*/ )
{
    DBG_ASSERT( false, "Don't use OutputDevice::CopyArea(...) with printer devices!" );
}

void Printer::SetPrinterOptions( const PrinterOptions& i_rOptions )
{
    *mpPrinterOptions = i_rOptions;
}

bool Printer::HasMirroredGraphics() const
{
    // due to a "hotfix" for AOO bug i55719, this needs to return false
    return false;
}

// QueueInfo
QueueInfo::QueueInfo()
{
    mnStatus    = PrintQueueFlags::NONE;
    mnJobs      = 0;
}

QueueInfo::QueueInfo( const QueueInfo& rInfo ) :
    maPrinterName( rInfo.maPrinterName ),
    maDriver( rInfo.maDriver ),
    maLocation( rInfo.maLocation ),
    maComment( rInfo.maComment ),
    mnStatus( rInfo.mnStatus ),
    mnJobs( rInfo.mnJobs )
{
}

QueueInfo::~QueueInfo()
{
}

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

SalPrinterQueueInfo::SalPrinterQueueInfo()
{
    mnStatus    = PrintQueueFlags::NONE;
    mnJobs      = QUEUE_JOBS_DONTKNOW;
    mpSysData   = nullptr;
}

SalPrinterQueueInfo::~SalPrinterQueueInfo()
{
}

ImplPrnQueueList::~ImplPrnQueueList()
{
    ImplSVData*         pSVData = ImplGetSVData();
    for( size_t i = 0; i < m_aQueueInfos.size(); i++ )
    {
        delete m_aQueueInfos[i].mpQueueInfo;
        pSVData->mpDefInst->DeletePrinterQueueInfo( m_aQueueInfos[i].mpSalQueueInfo );
    }
}

void ImplPrnQueueList::Add( SalPrinterQueueInfo* pData )
{
    std::unordered_map< OUString, sal_Int32, OUStringHash >::iterator it =
        m_aNameToIndex.find( pData->maPrinterName );
    if( it == m_aNameToIndex.end() )
    {
        m_aNameToIndex[ pData->maPrinterName ] = m_aQueueInfos.size();
        m_aQueueInfos.push_back( ImplPrnQueueData() );
        m_aQueueInfos.back().mpQueueInfo = nullptr;
        m_aQueueInfos.back().mpSalQueueInfo = pData;
        m_aPrinterList.push_back( pData->maPrinterName );
    }
    else // this should not happen, but ...
    {
        ImplPrnQueueData& rData = m_aQueueInfos[ it->second ];
        delete rData.mpQueueInfo;
        rData.mpQueueInfo = nullptr;
        ImplGetSVData()->mpDefInst->DeletePrinterQueueInfo( rData.mpSalQueueInfo );
        rData.mpSalQueueInfo = pData;
    }
}

ImplPrnQueueData* ImplPrnQueueList::Get( const OUString& rPrinter )
{
    ImplPrnQueueData* pData = nullptr;
    std::unordered_map<OUString,sal_Int32,OUStringHash>::iterator it =
        m_aNameToIndex.find( rPrinter );
    if( it != m_aNameToIndex.end() )
        pData = &m_aQueueInfos[it->second];
    return pData;
}

static void ImplInitPrnQueueList()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maGDIData.mpPrinterQueueList = new ImplPrnQueueList;

    static const char* pEnv = getenv( "SAL_DISABLE_PRINTERLIST" );
    if( !pEnv || !*pEnv )
        pSVData->mpDefInst->GetPrinterQueueInfo( pSVData->maGDIData.mpPrinterQueueList );
}

void ImplDeletePrnQueueList()
{
    ImplSVData*         pSVData = ImplGetSVData();
    ImplPrnQueueList*   pPrnList = pSVData->maGDIData.mpPrinterQueueList;

    if ( pPrnList )
    {
        delete pPrnList;
        pSVData->maGDIData.mpPrinterQueueList = nullptr;
    }
}

const std::vector<OUString>& Printer::GetPrinterQueues()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();
    return pSVData->maGDIData.mpPrinterQueueList->m_aPrinterList;
}

const QueueInfo* Printer::GetQueueInfo( const OUString& rPrinterName, bool bStatusUpdate )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    if ( !pSVData->maGDIData.mpPrinterQueueList )
        return nullptr;

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
    return nullptr;
}

OUString Printer::GetDefaultPrinterName()
{
    static const char* pEnv = getenv( "SAL_DISABLE_DEFAULTPRINTER" );
    if( !pEnv || !*pEnv )
    {
        ImplSVData* pSVData = ImplGetSVData();

        return pSVData->mpDefInst->GetDefaultPrinter();
    }
    return OUString();
}

void Printer::ImplInitData()
{
    mbDevOutput         = false;
    meOutDevType        = OUTDEV_PRINTER;
    mbDefPrinter        = false;
    mnError             = 0;
    mnCurPage           = 0;
    mnCurPrintPage      = 0;
    mnPageQueueSize     = 0;
    mnCopyCount         = 1;
    mbCollateCopy       = false;
    mbPrinting          = false;
    mbJobActive         = false;
    mbPrintFile         = false;
    mbInPrintPage       = false;
    mbNewJobSetup       = false;
    mpInfoPrinter       = nullptr;
    mpPrinter           = nullptr;
    mpDisplayDev        = nullptr;
    mbIsQueuePrinter    = false;
    mpPrinterOptions    = new PrinterOptions;

    // Add printer to the list
    ImplSVData* pSVData = ImplGetSVData();
    mpNext = pSVData->maGDIData.mpFirstPrinter;
    mpPrev = nullptr;
    if ( mpNext )
        mpNext->mpPrev = this;
    else
        pSVData->maGDIData.mpLastPrinter = this;
    pSVData->maGDIData.mpFirstPrinter = this;
}

bool Printer::AcquireGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( mpGraphics )
        return true;

    mbInitLineColor     = true;
    mbInitFillColor     = true;
    mbInitFont          = true;
    mbInitTextColor     = true;
    mbInitClipRegion    = true;

    ImplSVData* pSVData = ImplGetSVData();

    if ( mpJobGraphics )
        mpGraphics = mpJobGraphics;
    else if ( mpDisplayDev )
    {
        const VirtualDevice* pVirDev = mpDisplayDev;
        mpGraphics = pVirDev->mpVirDev->AcquireGraphics();
        // if needed retry after releasing least recently used virtual device graphics
        while ( !mpGraphics )
        {
            if ( !pSVData->maGDIData.mpLastVirGraphics )
                break;
            pSVData->maGDIData.mpLastVirGraphics->ReleaseGraphics();
            mpGraphics = pVirDev->mpVirDev->AcquireGraphics();
        }
        // update global LRU list of virtual device graphics
        if ( mpGraphics )
        {
            mpNextGraphics = pSVData->maGDIData.mpFirstVirGraphics;
            pSVData->maGDIData.mpFirstVirGraphics = const_cast<Printer*>(this);
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = const_cast<Printer*>(this);
            if ( !pSVData->maGDIData.mpLastVirGraphics )
                pSVData->maGDIData.mpLastVirGraphics = const_cast<Printer*>(this);
        }
    }
    else
    {
        mpGraphics = mpInfoPrinter->AcquireGraphics();
        // if needed retry after releasing least recently used printer graphics
        while ( !mpGraphics )
        {
            if ( !pSVData->maGDIData.mpLastPrnGraphics )
                break;
            pSVData->maGDIData.mpLastPrnGraphics->ReleaseGraphics();
            mpGraphics = mpInfoPrinter->AcquireGraphics();
        }
        // update global LRU list of printer graphics
        if ( mpGraphics )
        {
            mpNextGraphics = pSVData->maGDIData.mpFirstPrnGraphics;
            pSVData->maGDIData.mpFirstPrnGraphics = const_cast<Printer*>(this);
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = const_cast<Printer*>(this);
            if ( !pSVData->maGDIData.mpLastPrnGraphics )
                pSVData->maGDIData.mpLastPrnGraphics = const_cast<Printer*>(this);
        }
    }

    if ( mpGraphics )
    {
        mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
        mpGraphics->setAntiAliasB2DDraw(bool(mnAntialiasing & AntialiasingFlags::EnableB2dDraw));
    }

    return mpGraphics != nullptr;
}

void Printer::ImplReleaseFonts()
{
#ifdef UNX
    // HACK to fix an urgent P1 printing issue fast
    // WinSalPrinter does not respect GetGraphics/ReleaseGraphics conventions
    // so Printer::mpGraphics often points to a dead WinSalGraphics
    // TODO: fix WinSalPrinter's GetGraphics/ReleaseGraphics handling
    mpGraphics->ReleaseFonts();
#endif
    mbNewFont = true;
    mbInitFont = true;

    if ( mpFontEntry )
    {
        mpFontCache->Release( mpFontEntry );
        mpFontEntry = nullptr;
    }

    if ( mpDeviceFontList )
    {
        delete mpDeviceFontList;
        mpDeviceFontList = nullptr;
    }

    if ( mpDeviceFontSizeList )
    {
        delete mpDeviceFontSizeList;
        mpDeviceFontSizeList = nullptr;
    }
}

void Printer::ReleaseGraphics( bool bRelease )
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
        return;

    // release the fonts of the physically released graphics device
    if( bRelease )
        ImplReleaseFonts();

    ImplSVData* pSVData = ImplGetSVData();

    Printer* pPrinter = this;

    if ( !pPrinter->mpJobGraphics )
    {
        if ( pPrinter->mpDisplayDev )
        {
            VirtualDevice* pVirDev = pPrinter->mpDisplayDev;
            if ( bRelease )
                pVirDev->mpVirDev->ReleaseGraphics( mpGraphics );
            // remove from global LRU list of virtual device graphics
            if ( mpPrevGraphics )
                mpPrevGraphics->mpNextGraphics = mpNextGraphics;
            else
                pSVData->maGDIData.mpFirstVirGraphics = mpNextGraphics;
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
            else
                pSVData->maGDIData.mpLastVirGraphics = mpPrevGraphics;
        }
        else
        {
            if ( bRelease )
                pPrinter->mpInfoPrinter->ReleaseGraphics( mpGraphics );
            // remove from global LRU list of printer graphics
            if ( mpPrevGraphics )
                mpPrevGraphics->mpNextGraphics = mpNextGraphics;
            else
                pSVData->maGDIData.mpFirstPrnGraphics = mpNextGraphics;
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
            else
                pSVData->maGDIData.mpLastPrnGraphics = mpPrevGraphics;
        }
    }

    mpGraphics      = nullptr;
    mpPrevGraphics  = nullptr;
    mpNextGraphics  = nullptr;
}

void Printer::ImplInit( SalPrinterQueueInfo* pInfo )
{
    ImplSVData* pSVData = ImplGetSVData();
    // #i74084# update info for this specific SalPrinterQueueInfo
    pSVData->mpDefInst->GetPrinterQueueState( pInfo );

    // Test whether the driver actually matches the JobSetup
    ImplJobSetup* pJobSetup = maJobSetup.ImplGetData();

    if ( pJobSetup->mpDriverData )
    {
        if ( (pJobSetup->maPrinterName != pInfo->maPrinterName) ||
             (pJobSetup->maDriver != pInfo->maDriver) )
        {
            rtl_freeMemory( pJobSetup->mpDriverData );
            pJobSetup->mpDriverData = nullptr;
            pJobSetup->mnDriverDataLen = 0;
        }
    }

    // Remember printer name
    maPrinterName = pInfo->maPrinterName;
    maDriver = pInfo->maDriver;

    // Add printer name to JobSetup
    pJobSetup->maPrinterName = maPrinterName;
    pJobSetup->maDriver = maDriver;

    mpInfoPrinter   = pSVData->mpDefInst->CreateInfoPrinter( pInfo, pJobSetup );
    mpPrinter       = nullptr;
    mpJobGraphics   = nullptr;
    ImplUpdateJobSetupPaper( maJobSetup );

    if ( !mpInfoPrinter )
    {
        ImplInitDisplay( nullptr );
        return;
    }

    // we need a graphics
    if ( !AcquireGraphics() )
    {
        ImplInitDisplay( nullptr );
        return;
    }

    // Init data
    ImplUpdatePageData();
    mpFontCollection = new PhysicalFontCollection();
    mpFontCache = new ImplFontCache();
    mpGraphics->GetDevFontList( mpFontCollection );
}

void Printer::ImplInitDisplay( const vcl::Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    mpInfoPrinter       = nullptr;
    mpPrinter           = nullptr;
    mpJobGraphics       = nullptr;

    if ( pWindow )
        mpDisplayDev = VclPtr<VirtualDevice>::Create( *pWindow );
    else
        mpDisplayDev = VclPtr<VirtualDevice>::Create();
    mpFontCollection          = pSVData->maGDIData.mpScreenFontList;
    mpFontCache         = pSVData->maGDIData.mpScreenFontCache;
    mnDPIX              = mpDisplayDev->mnDPIX;
    mnDPIY              = mpDisplayDev->mnDPIY;
}

void Printer::DrawDeviceMask( const Bitmap& rMask, const Color& rMaskColor,
                         const Point& rDestPt, const Size& rDestSize,
                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aPt;
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( !rMask.IsEmpty() && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height() )
    {
        Bitmap  aMask( rMask );
        BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

        if( aMask.GetBitCount() > 1 )
            aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );

        // mirrored horizontically
        if( aDestSz.Width() < 0L )
        {
            aDestSz.Width() = -aDestSz.Width();
            aDestPt.X() -= ( aDestSz.Width() - 1L );
            nMirrFlags |= BmpMirrorFlags::Horizontal;
        }

        // mirrored vertically
        if( aDestSz.Height() < 0L )
        {
            aDestSz.Height() = -aDestSz.Height();
            aDestPt.Y() -= ( aDestSz.Height() - 1L );
            nMirrFlags |= BmpMirrorFlags::Vertical;
        }

        // source cropped?
        if( aSrcRect != Rectangle( aPt, aMask.GetSizePixel() ) )
            aMask.Crop( aSrcRect );

        // destination mirrored
        if( nMirrFlags != BmpMirrorFlags::NONE)
            aMask.Mirror( nMirrFlags );

        // do painting
        const long      nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
        long            nX, nY; //, nWorkX, nWorkY, nWorkWidth, nWorkHeight;
        long*           pMapX = new long[ nSrcWidth + 1 ];
        long*           pMapY = new long[ nSrcHeight + 1 ];
        GDIMetaFile*    pOldMetaFile = mpMetaFile;
        const bool      bOldMap = mbMap;

        mpMetaFile = nullptr;
        mbMap = false;
        Push( PushFlags::FILLCOLOR | PushFlags::LINECOLOR );
        SetLineColor( rMaskColor );
        SetFillColor( rMaskColor );
        InitLineColor();
        InitFillColor();

        // create forward mapping tables
        for( nX = 0L; nX <= nSrcWidth; nX++ )
            pMapX[ nX ] = aDestPt.X() + FRound( (double) aDestSz.Width() * nX / nSrcWidth );

        for( nY = 0L; nY <= nSrcHeight; nY++ )
            pMapY[ nY ] = aDestPt.Y() + FRound( (double) aDestSz.Height() * nY / nSrcHeight );

        // walk through all rectangles of mask
        const vcl::Region aWorkRgn(aMask.CreateRegion(COL_BLACK, Rectangle(Point(), aMask.GetSizePixel())));
        RectangleVector aRectangles;
        aWorkRgn.GetRegionRectangles(aRectangles);

        for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            const Point aMapPt(pMapX[aRectIter->Left()], pMapY[aRectIter->Top()]);
            const Size aMapSz(
                pMapX[aRectIter->Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                pMapY[aRectIter->Bottom() + 1] - aMapPt.Y());    // same for Y

            DrawRect(Rectangle(aMapPt, aMapSz));
        }

        Pop();
        delete[] pMapX;
        delete[] pMapY;
        mbMap = bOldMap;
        mpMetaFile = pOldMetaFile;
    }
}

SalPrinterQueueInfo* Printer::ImplGetQueueInfo( const OUString& rPrinterName,
                                                const OUString* pDriver )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    ImplPrnQueueList* pPrnList = pSVData->maGDIData.mpPrinterQueueList;
    if ( pPrnList && pPrnList->m_aQueueInfos.size() )
    {
        // first search for the printer name directly
        ImplPrnQueueData* pInfo = pPrnList->Get( rPrinterName );
        if( pInfo )
            return pInfo->mpSalQueueInfo;

        // then search case insensitive
        for( size_t i = 0; i < pPrnList->m_aQueueInfos.size(); i++ )
        {
            if( pPrnList->m_aQueueInfos[i].mpSalQueueInfo->maPrinterName.equalsIgnoreAsciiCase( rPrinterName ) )
                return pPrnList->m_aQueueInfos[i].mpSalQueueInfo;
        }

        // then search for driver name
        if ( pDriver )
        {
            for( size_t i = 0; i < pPrnList->m_aQueueInfos.size(); i++ )
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

    return nullptr;
}

void Printer::ImplUpdatePageData()
{
    // we need a graphics
    if ( !AcquireGraphics() )
        return;

    mpGraphics->GetResolution( mnDPIX, mnDPIY );
    mpInfoPrinter->GetPageInfo( maJobSetup.ImplGetConstData(),
                                mnOutWidth, mnOutHeight,
                                maPageOffset.X(), maPageOffset.Y(),
                                maPaperSize.Width(), maPaperSize.Height() );
}

void Printer::ImplUpdateFontList()
{
    ImplUpdateFontData( true );
}

long Printer::GetGradientStepCount( long nMinRect )
{
    // use display-equivalent step size calculation
    long nInc = (nMinRect < 800) ? 10 : 20;

    return nInc;
}

Printer::Printer()
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( GetDefaultPrinterName(), nullptr );
    if ( pInfo )
    {
        ImplInit( pInfo );
        if ( !IsDisplayPrinter() )
            mbDefPrinter = true;
    }
    else
        ImplInitDisplay( nullptr );
}

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
        ImplInitDisplay( nullptr );
        maJobSetup = JobSetup();
    }
}

Printer::Printer( const QueueInfo& rQueueInfo )
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rQueueInfo.GetPrinterName(),
                                                   &rQueueInfo.GetDriver() );
    if ( pInfo )
        ImplInit( pInfo );
    else
        ImplInitDisplay( nullptr );
}

Printer::Printer( const OUString& rPrinterName )
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rPrinterName, nullptr );
    if ( pInfo )
        ImplInit( pInfo );
    else
        ImplInitDisplay( nullptr );
}

Printer::~Printer()
{
    disposeOnce();
}

void Printer::dispose()
{
    DBG_ASSERT( !IsPrinting(), "Printer::~Printer() - Job is printing" );
    DBG_ASSERT( !IsJobActive(), "Printer::~Printer() - Job is active" );

    delete mpPrinterOptions;
    mpPrinterOptions = nullptr;

    ReleaseGraphics();
    if ( mpInfoPrinter )
        ImplGetSVData()->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
    if ( mpDisplayDev )
        mpDisplayDev.disposeAndClear();
    else
    {
        // OutputDevice Dtor is trying the same thing; that why we need to set
        // the FontEntry to NULL here
        // TODO: consolidate duplicate cleanup by Printer and OutputDevice
        if ( mpFontEntry )
        {
            mpFontCache->Release( mpFontEntry );
            mpFontEntry = nullptr;
        }
        if ( mpDeviceFontList )
        {
            delete mpDeviceFontList;
            mpDeviceFontList = nullptr;
        }
        if ( mpDeviceFontSizeList )
        {
            delete mpDeviceFontSizeList;
            mpDeviceFontSizeList = nullptr;
        }
        delete mpFontCache;
        mpFontCache = nullptr;
        // font list deleted by OutputDevice dtor
    }

    // Add printer from the list
    ImplSVData* pSVData = ImplGetSVData();
    if ( mpPrev )
        mpPrev->mpNext = mpNext;
    else
        pSVData->maGDIData.mpFirstPrinter = mpNext;
    if ( mpNext )
        mpNext->mpPrev = mpPrev;
    else
        pSVData->maGDIData.mpLastPrinter = mpPrev;

    mpPrev.clear();
    mpNext.clear();
    OutputDevice::dispose();
}

sal_uLong Printer::GetCapabilities( PrinterCapType nType ) const
{
    if ( IsDisplayPrinter() )
        return 0;

    if( mpInfoPrinter )
        return mpInfoPrinter->GetCapabilities( maJobSetup.ImplGetConstData(), nType );
    else
        return 0;
}

bool Printer::HasSupport( PrinterSupport eFeature ) const
{
    switch ( eFeature )
    {
        case SUPPORT_SET_ORIENTATION:
            return GetCapabilities( PrinterCapType::SetOrientation ) != 0;
        case SUPPORT_SET_PAPERBIN:
            return GetCapabilities( PrinterCapType::SetPaperBin ) != 0;
        case SUPPORT_SET_PAPERSIZE:
            return GetCapabilities( PrinterCapType::SetPaperSize ) != 0;
        case SUPPORT_SET_PAPER:
            return GetCapabilities( PrinterCapType::SetPaper ) != 0;
        case SUPPORT_COPY:
            return (GetCapabilities( PrinterCapType::Copies ) != 0);
        case SUPPORT_COLLATECOPY:
            return (GetCapabilities( PrinterCapType::CollateCopies ) != 0);
        case SUPPORT_SETUPDIALOG:
            return GetCapabilities( PrinterCapType::SupportDialog ) != 0;
        case SUPPORT_FAX:
            return GetCapabilities( PrinterCapType::Fax ) != 0;
        case SUPPORT_PDF:
            return GetCapabilities( PrinterCapType::PDF ) != 0;
    }

    return true;
}

bool Printer::SetJobSetup( const JobSetup& rSetup )
{
    if ( IsDisplayPrinter() || mbInPrintPage )
        return false;

    JobSetup aJobSetup = rSetup;

    ReleaseGraphics();
    if ( mpInfoPrinter->SetPrinterData( aJobSetup.ImplGetData() ) )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
        return true;
    }

    return false;
}

bool Printer::Setup( vcl::Window* pWindow, bool bPapersizeFromSetup )
{
    if ( IsDisplayPrinter() )
        return false;

    if ( IsJobActive() || IsPrinting() )
        return false;

    JobSetup aJobSetup = maJobSetup;
    ImplJobSetup* pData = aJobSetup.ImplGetData();
    pData->mbPapersizeFromSetup = bPapersizeFromSetup;
    SalFrame* pFrame;
    if ( !pWindow )
        pWindow = ImplGetDefaultWindow();
    if( !pWindow )
        return false;

    pFrame = pWindow->ImplGetFrame();
    ReleaseGraphics();
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode++;
    nImplSysDialog++;
    bool bSetup = mpInfoPrinter->Setup( pFrame, pData );
    pSVData->maAppData.mnModalMode--;
    nImplSysDialog--;
    if ( bSetup )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
        return true;
    }
    return false;
}

bool Printer::SetPrinterProps( const Printer* pPrinter )
{
    if ( IsJobActive() || IsPrinting() )
        return false;

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
        // Destroy old printer
        if ( !IsDisplayPrinter() )
        {
            ReleaseGraphics();
            pSVData->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );
            if ( mpFontEntry )
            {
                mpFontCache->Release( mpFontEntry );
                mpFontEntry = nullptr;
            }
            if ( mpDeviceFontList )
            {
                delete mpDeviceFontList;
                mpDeviceFontList = nullptr;
            }
            if ( mpDeviceFontSizeList )
            {
                delete mpDeviceFontSizeList;
                mpDeviceFontSizeList = nullptr;
            }
            // clean up font list
            delete mpFontCache;
            delete mpFontCollection;
            mpFontCache = nullptr;
            mpFontCollection = nullptr;

            mbInitFont = true;
            mbNewFont = true;
            mpInfoPrinter = nullptr;
        }

        // Construct new printer
        ImplInitDisplay( nullptr );
        return true;
    }

    // Destroy old printer?
    if ( GetName() != pPrinter->GetName() )
    {
        ReleaseGraphics();
        if ( mpDisplayDev )
        {
            mpDisplayDev.disposeAndClear();
        }
        else
        {
            pSVData->mpDefInst->DestroyInfoPrinter( mpInfoPrinter );

            if ( mpFontEntry )
            {
                mpFontCache->Release( mpFontEntry );
                mpFontEntry = nullptr;
            }
            if ( mpDeviceFontList )
            {
                delete mpDeviceFontList;
                mpDeviceFontList = nullptr;
            }
            if ( mpDeviceFontSizeList )
            {
                delete mpDeviceFontSizeList;
                mpDeviceFontSizeList = nullptr;
            }
            delete mpFontCache;
            delete mpFontCollection;
            mpFontCache = nullptr;
            mpFontCollection = nullptr;
            mbInitFont = true;
            mbNewFont = true;
            mpInfoPrinter = nullptr;
        }

        // Construct new printer
        OUString aDriver = pPrinter->GetDriverName();
        SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( pPrinter->GetName(), &aDriver );
        if ( pInfo )
        {
            ImplInit( pInfo );
            SetJobSetup( pPrinter->GetJobSetup() );
        }
        else
            ImplInitDisplay( nullptr );
    }
    else
        SetJobSetup( pPrinter->GetJobSetup() );

    return false;
}

bool Printer::SetOrientation( Orientation eOrientation )
{
    if ( mbInPrintPage )
        return false;

    if ( maJobSetup.ImplGetConstData()->meOrientation != eOrientation )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->meOrientation = eOrientation;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return true;
        }
        else
            return false;
    }

    return true;
}

Orientation Printer::GetOrientation() const
{
    return maJobSetup.ImplGetConstData()->meOrientation;
}

bool Printer::SetPaperBin( sal_uInt16 nPaperBin )
{
    if ( mbInPrintPage )
        return false;

    if ( (maJobSetup.ImplGetConstData()->mnPaperBin != nPaperBin) &&
         (nPaperBin < GetPaperBinCount()) )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mnPaperBin = nPaperBin;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERBIN, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return true;
        }
        else
            return false;
    }

    return true;
}

sal_uInt16 Printer::GetPaperBin() const
{
    return maJobSetup.ImplGetConstData()->mnPaperBin;
}

// Map user paper format to a available printer paper formats
void Printer::ImplFindPaperFormatForUserSize( JobSetup& aJobSetup, bool bMatchNearest )
{
    ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();

    int     nLandscapeAngle = GetLandscapeAngle();
    int     nPaperCount     = GetPaperInfoCount();
    bool    bFound = false;

    PaperInfo aInfo(pSetupData->mnPaperWidth, pSetupData->mnPaperHeight);

    // Compare all paper formats and get the appropriate one
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
        const long nRotatedWidth = pSetupData->mnPaperHeight;
        const long nRotatedHeight = pSetupData->mnPaperWidth;
        PaperInfo aRotatedInfo(nRotatedWidth, nRotatedHeight);

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

             // check portrait match
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

bool Printer::SetPaper( Paper ePaper )
{
    if ( mbInPrintPage )
        return false;

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
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        if ( ePaper == PAPER_USER )
            ImplFindPaperFormatForUserSize( aJobSetup, false );
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return true;
        }
        else
            return false;
    }

    return true;
}

bool Printer::SetPaperSizeUser( const Size& rSize )
{
    return SetPaperSizeUser( rSize, false );
}

bool Printer::SetPaperSizeUser( const Size& rSize, bool bMatchNearest )
{
    if ( mbInPrintPage )
        return false;

    const Size aPixSize = LogicToPixel( rSize );
    const Size aPageSize = PixelToLogic( aPixSize, MAP_100TH_MM );
    bool bNeedToChange(maJobSetup.ImplGetConstData()->mnPaperWidth != aPageSize.Width() ||
        maJobSetup.ImplGetConstData()->mnPaperHeight != aPageSize.Height());

    if(!bNeedToChange)
    {
        // #i122984# only need to change when Paper is different from PAPER_USER and
        // the mapped Paper which will created below in the call to ImplFindPaperFormatForUserSize
        // and will replace maJobSetup.ImplGetConstData()->mePaperFormat. This leads to
        // unnecessary JobSetups, e.g. when printing a multi-page fax, but also with
        // normal print
        const Paper aPaper = ImplGetPaperFormat(aPageSize.Width(), aPageSize.Height());

        bNeedToChange = maJobSetup.ImplGetConstData()->mePaperFormat != PAPER_USER &&
            maJobSetup.ImplGetConstData()->mePaperFormat != aPaper;
    }

    if(bNeedToChange)
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->mePaperFormat   = PAPER_USER;
        pSetupData->mnPaperWidth    = aPageSize.Width();
        pSetupData->mnPaperHeight   = aPageSize.Height();

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        ImplFindPaperFormatForUserSize( aJobSetup, bMatchNearest );

        // Changing the paper size can also change the orientation!
        if ( mpInfoPrinter->SetData( SAL_JOBSET_PAPERSIZE|SAL_JOBSET_ORIENTATION, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return true;
        }
        else
            return false;
    }

    return true;
}

int Printer::GetPaperInfoCount() const
{
    if( ! mpInfoPrinter )
        return 0;
    if( ! mpInfoPrinter->m_bPapersInit )
        mpInfoPrinter->InitPaperFormats( maJobSetup.ImplGetConstData() );
    return mpInfoPrinter->m_aPaperFormats.size();
}

OUString Printer::GetPaperName( Paper ePaper )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpPaperNames )
    {
        pSVData->mpPaperNames = new std::unordered_map< int, OUString >();
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

    std::unordered_map<int,OUString>::const_iterator it = pSVData->mpPaperNames->find( (int)ePaper );
    return (it != pSVData->mpPaperNames->end()) ? it->second : OUString();
}

OUString Printer::GetPaperName( bool i_bPaperUser ) const
{
    Size  aPageSize = PixelToLogic( GetPaperSizePixel(), MAP_100TH_MM );
    Paper ePaper    = ImplGetPaperFormat( aPageSize.Width(), aPageSize.Height() );
    if( ePaper == PAPER_USER )
        ePaper = ImplGetPaperFormat( aPageSize.Height(), aPageSize.Width() );
    return (ePaper != PAPER_USER || i_bPaperUser ) ? GetPaperName( ePaper ) : OUString();
}

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

bool Printer::SetDuplexMode( DuplexMode eDuplex )
{
    if ( mbInPrintPage )
        return false;

    if ( maJobSetup.ImplGetConstData()->meDuplexMode != eDuplex )
    {
        JobSetup        aJobSetup = maJobSetup;
        ImplJobSetup*   pSetupData = aJobSetup.ImplGetData();
        pSetupData->meDuplexMode = eDuplex;

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        if ( mpInfoPrinter->SetData( SAL_JOBSET_DUPLEXMODE, pSetupData ) )
        {
            ImplUpdateJobSetupPaper( aJobSetup );
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            ImplUpdatePageData();
            ImplUpdateFontList();
            return true;
        }
        else
            return false;
    }

    return true;
}

int Printer::GetLandscapeAngle() const
{
    return mpInfoPrinter ? mpInfoPrinter->GetLandscapeAngle( maJobSetup.ImplGetConstData() ) : 900;
}

Paper Printer::GetPaper() const
{
    return maJobSetup.ImplGetConstData()->mePaperFormat;
}

sal_uInt16 Printer::GetPaperBinCount() const
{
    if ( IsDisplayPrinter() )
        return 0;

    return (sal_uInt16)mpInfoPrinter->GetPaperBinCount( maJobSetup.ImplGetConstData() );
}

OUString Printer::GetPaperBinName( sal_uInt16 nPaperBin ) const
{
    if ( IsDisplayPrinter() )
        return OUString();

    if ( nPaperBin < GetPaperBinCount() )
        return mpInfoPrinter->GetPaperBinName( maJobSetup.ImplGetConstData(), nPaperBin );
    else
        return OUString();
}

bool Printer::SetCopyCount( sal_uInt16 nCopy, bool bCollate )
{
    mnCopyCount = nCopy;
    mbCollateCopy = bCollate;
    return true;
}

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

bool Printer::EndJob()
{
    bool bRet = false;
    if ( !IsJobActive() )
        return bRet;

    DBG_ASSERT( !mbInPrintPage, "Printer::EndJob() - StartPage() without EndPage() called" );

    mbJobActive = false;

    if ( mpPrinter )
    {
        ReleaseGraphics();

        mnCurPage = 0;

        mbPrinting      = false;
        mnCurPrintPage  = 0;
        maJobName.clear();

        mbDevOutput = false;
        bRet = mpPrinter->EndJob();
        // FIXME: Do not destroy the printer asynchronously as Win95
        // can't handle destroying a printer object and printing
        // at the same time
        ImplGetSVData()->mpDefInst->DestroyPrinter( mpPrinter );
        mpPrinter = nullptr;
    }

    return bRet;
}

void Printer::ImplStartPage()
{
    if ( !IsJobActive() )
        return;

    if ( mpPrinter )
    {
        SalGraphics* pGraphics = mpPrinter->StartPage( maJobSetup.ImplGetConstData(), mbNewJobSetup );
        if ( pGraphics )
        {
            ReleaseGraphics();
            mpJobGraphics = pGraphics;
        }
        mbDevOutput = true;

        // PrintJob not aborted ???
        if ( IsJobActive() )
        {
            mbInPrintPage = true;
            mnCurPage++;
            mnCurPrintPage++;
        }
    }
}

void Printer::ImplEndPage()
{
    if ( !IsJobActive() )
        return;

    mbInPrintPage = false;

    if ( mpPrinter )
    {
        mpPrinter->EndPage();
        ReleaseGraphics();
        mbDevOutput = false;

        mpJobGraphics = nullptr;
        mbNewJobSetup = false;
    }
}

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
                DataChangedEvent aDCEvt( DataChangedEventType::PRINTER );
                Application::NotifyAllWindows( aDCEvt );
            }
        }
        else
            delete pNewList;
    }
}

bool Printer::UsePolyPolygonForComplexGradient()
{
    return true;
}

void Printer::ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly )
{
    const Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

    Push( PushFlags::CLIPREGION );
    IntersectClipRegion(vcl::Region(rPolyPoly));
    DrawGradient( aBoundRect, rGradient );
    Pop();
}

void Printer::InitFont() const
{
    DBG_TESTSOLARMUTEX();

    if (!mpFontEntry)
        return;

    if ( mbInitFont )
    {
        // select font in the device layers
        mpFontEntry->mnSetFontFlags = mpGraphics->SetFont( &(mpFontEntry->maFontSelData), 0 );
        mbInitFont = false;
    }
}

void Printer::SetFontOrientation( ImplFontEntry* const pFontEntry ) const
{
    pFontEntry->mnOrientation = pFontEntry->maMetric.mnOrientation;
}

void Printer::DrawImage( const Point&, const Image&, DrawImageFlags )
{
    SAL_WARN ("vcl.gdi", "DrawImage(): Images can't be drawn on any Printer instance");
    assert(false);
}

void Printer::DrawImage( const Point&, const Size&, const Image&, DrawImageFlags )
{
    SAL_WARN ("vcl.gdi", "DrawImage(): Images can't be drawn on any Printer instance");
    assert(false);
}


Bitmap Printer::GetBitmap( const Point& rSrcPt, const Size& rSize ) const
{
    SAL_WARN("vcl.gdi", "GetBitmap(): This should never be called on by a Printer instance");

    return OutputDevice::GetBitmap( rSrcPt, rSize );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
