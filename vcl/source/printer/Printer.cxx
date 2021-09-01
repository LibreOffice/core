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

#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <tools/urlobj.hxx>

#include <vcl/QueueInfo.hxx>
#include <vcl/event.hxx>
#include <vcl/print.hxx>
#include <vcl/printer/PrinterController.hxx>
#include <vcl/printer/PrinterOptions.hxx>
#include <vcl/virdev.hxx>

#include <outdev.h>
#include <print.h>
#include <printer/ImplJobSetup.hxx>
#include <PhysicalFontCollection.hxx>
#include <configsettings.hxx>
#include <print.hrc>
#include <printdlg.hxx>
#include <salgdi.hxx>
#include <salinst.hxx>
#include <salvd.hxx>
#include <strings.hrc>
#include <svdata.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <unordered_map>
#include <utility>
#include <vector>

int nImplSysDialog = 0;

namespace
{
    Paper ImplGetPaperFormat( tools::Long nWidth100thMM, tools::Long nHeight100thMM )
    {
        PaperInfo aInfo(nWidth100thMM, nHeight100thMM);
        aInfo.doSloppyFit();
        return aInfo.getPaper();
    }

    const PaperInfo& ImplGetEmptyPaper()
    {
        static PaperInfo aInfo(PAPER_USER);
        return aInfo;
    }
}

void ImplUpdateJobSetupPaper( JobSetup& rJobSetup )
{
    const vcl::print::ImplJobSetup& rConstData = rJobSetup.ImplGetConstData();

    if ( !rConstData.GetPaperWidth() || !rConstData.GetPaperHeight() )
    {
        if ( rConstData.GetPaperFormat() != PAPER_USER )
        {
            PaperInfo aInfo(rConstData.GetPaperFormat());

            vcl::print::ImplJobSetup& rData = rJobSetup.ImplGetData();
            rData.SetPaperWidth( aInfo.getWidth() );
            rData.SetPaperHeight( aInfo.getHeight() );
        }
    }
    else if ( rConstData.GetPaperFormat() == PAPER_USER )
    {
        Paper ePaper = ImplGetPaperFormat( rConstData.GetPaperWidth(), rConstData.GetPaperHeight() );
        if ( ePaper != PAPER_USER )
            rJobSetup.ImplGetData().SetPaperFormat(ePaper);
    }
}

void Printer::ImplPrintTransparent( const Bitmap& rBmp, const Bitmap& rMask,
                                         const Point& rDestPt, const Size& rDestSize,
                                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    tools::Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( rBmp.IsEmpty() || !aSrcRect.GetWidth() || !aSrcRect.GetHeight() || !aDestSz.Width() || !aDestSz.Height() )
        return;

    Bitmap  aPaint( rBmp ), aMask( rMask );
    BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

    if (aMask.getPixelFormat() > vcl::PixelFormat::N1_BPP)
        aMask.Convert( BmpConversion::N1BitThreshold );

    // mirrored horizontally
    if( aDestSz.Width() < 0 )
    {
        aDestSz.setWidth( -aDestSz.Width() );
        aDestPt.AdjustX( -( aDestSz.Width() - 1 ) );
        nMirrFlags |= BmpMirrorFlags::Horizontal;
    }

    // mirrored vertically
    if( aDestSz.Height() < 0 )
    {
        aDestSz.setHeight( -aDestSz.Height() );
        aDestPt.AdjustY( -( aDestSz.Height() - 1 ) );
        nMirrFlags |= BmpMirrorFlags::Vertical;
    }

    // source cropped?
    if( aSrcRect != tools::Rectangle( Point(), aPaint.GetSizePixel() ) )
    {
        aPaint.Crop( aSrcRect );
        aMask.Crop( aSrcRect );
    }

    // destination mirrored
    if( nMirrFlags != BmpMirrorFlags::NONE )
    {
        aPaint.Mirror( nMirrFlags );
        aMask.Mirror( nMirrFlags );
    }

    // we always want to have a mask
    if( aMask.IsEmpty() )
    {
        aMask = Bitmap(aSrcRect.GetSize(), vcl::PixelFormat::N1_BPP);
        aMask.Erase( COL_BLACK );
    }

    // do painting
    const tools::Long nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
    tools::Long nX, nY; // , nWorkX, nWorkY, nWorkWidth, nWorkHeight;
    std::unique_ptr<tools::Long[]> pMapX(new tools::Long[ nSrcWidth + 1 ]);
    std::unique_ptr<tools::Long[]> pMapY(new tools::Long[ nSrcHeight + 1 ]);
    const bool bOldMap = mbMap;

    mbMap = false;

    // create forward mapping tables
    for( nX = 0; nX <= nSrcWidth; nX++ )
        pMapX[ nX ] = aDestPt.X() + FRound( static_cast<double>(aDestSz.Width()) * nX / nSrcWidth );

    for( nY = 0; nY <= nSrcHeight; nY++ )
        pMapY[ nY ] = aDestPt.Y() + FRound( static_cast<double>(aDestSz.Height()) * nY / nSrcHeight );

    // walk through all rectangles of mask
    const vcl::Region aWorkRgn(aMask.CreateRegion(COL_BLACK, tools::Rectangle(Point(), aMask.GetSizePixel())));
    RectangleVector aRectangles;
    aWorkRgn.GetRegionRectangles(aRectangles);

    for (auto const& rectangle : aRectangles)
    {
        const Point aMapPt(pMapX[rectangle.Left()], pMapY[rectangle.Top()]);
        const Size aMapSz( pMapX[rectangle.Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                           pMapY[rectangle.Bottom() + 1] - aMapPt.Y());    // same for Y
        Bitmap aBandBmp(aPaint);

        aBandBmp.Crop(rectangle);
        DrawBitmap(aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp);
    }

    mbMap = bOldMap;

}

bool Printer::DrawTransformBitmapExDirect(
    const basegfx::B2DHomMatrix& /*aFullTransform*/,
    const BitmapEx& /*rBitmapEx*/,
    double /*fAlpha*/)
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

void Printer::DrawDeviceBitmapEx( const Point& rDestPt, const Size& rDestSize,
                                const Point& rSrcPtPixel, const Size& rSrcSizePixel,
                                BitmapEx& rBmpEx )
{
    if( rBmpEx.IsAlpha() )
    {
        // #107169# For true alpha bitmaps, no longer masking the
        // bitmap, but perform a full alpha blend against a white
        // background here.
        Bitmap aBmp( rBmpEx.GetBitmap() );
        aBmp.Blend( rBmpEx.GetAlpha(), COL_WHITE );
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp );
    }
    else
    {
        Bitmap aBmp( rBmpEx.GetBitmap() );
        ImplPrintTransparent( aBmp, Bitmap(), rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
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

    tools::Rectangle       aPolyRect( LogicToPixel( rPolyPoly ).GetBoundRect() );
    const Size      aDPISize( LogicToPixel(Size(1, 1), MapMode(MapUnit::MapInch)) );
    const tools::Long      nBaseExtent = std::max<tools::Long>( FRound( aDPISize.Width() / 300. ), 1 );
    tools::Long            nMove;
    const sal_uInt16    nTrans = ( nTransparencePercent < 13 ) ? 0 :
        ( nTransparencePercent < 38 ) ? 25 :
        ( nTransparencePercent < 63 ) ? 50 :
        ( nTransparencePercent < 88 ) ? 75 : 100;

    switch( nTrans )
    {
        case 25: nMove = nBaseExtent * 3; break;
        case 50: nMove = nBaseExtent * 4; break;
        case 75: nMove = nBaseExtent * 6; break;

            // #i112959#  very transparent (88 < nTransparencePercent <= 99)
        case 100: nMove = nBaseExtent * 8; break;

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
        tools::Rectangle aRect( aPolyRect.TopLeft(), Size( aPolyRect.GetWidth(), nBaseExtent ) );
        while( aRect.Top() <= aPolyRect.Bottom() )
        {
            DrawRect( aRect );
            aRect.Move( 0, nMove );
        }

        aRect = tools::Rectangle( aPolyRect.TopLeft(), Size( nBaseExtent, aPolyRect.GetHeight() ) );
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
    SAL_WARN( "vcl.gdi", "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );
}

void Printer::DrawOutDev( const Point& /*rDestPt*/, const Size& /*rDestSize*/,
                               const Point& /*rSrcPt*/,  const Size& /*rSrcSize*/,
                               const OutputDevice& /*rOutDev*/ )
{
    SAL_WARN( "vcl.gdi", "Don't use OutputDevice::DrawOutDev(...) with printer devices!" );
}

void Printer::CopyArea( const Point& /*rDestPt*/,
                        const Point& /*rSrcPt*/,  const Size& /*rSrcSize*/,
                        bool /*bWindowInvalidate*/ )
{
    SAL_WARN( "vcl.gdi", "Don't use OutputDevice::CopyArea(...) with printer devices!" );
}

tools::Rectangle Printer::GetBackgroundComponentBounds() const
{
    Point aPageOffset = Point( 0, 0 ) - this->GetPageOffsetPixel();
    Size aSize  = this->GetPaperSizePixel();
    return tools::Rectangle( aPageOffset, aSize );
}

void Printer::SetPrinterOptions( const vcl::print::PrinterOptions& i_rOptions )
{
    *mpPrinterOptions = i_rOptions;
}

bool Printer::HasMirroredGraphics() const
{
    // due to a "hotfix" for AOO bug i55719, this needs to return false
    return false;
}

SalPrinterQueueInfo::SalPrinterQueueInfo()
{
    mnStatus    = PrintQueueFlags::NONE;
    mnJobs      = QUEUE_JOBS_DONTKNOW;
}

SalPrinterQueueInfo::~SalPrinterQueueInfo()
{
}

ImplPrnQueueList::~ImplPrnQueueList()
{
}

void ImplPrnQueueList::Add( std::unique_ptr<SalPrinterQueueInfo> pData )
{
    std::unordered_map< OUString, sal_Int32 >::iterator it =
        m_aNameToIndex.find( pData->maPrinterName );
    if( it == m_aNameToIndex.end() )
    {
        m_aNameToIndex[ pData->maPrinterName ] = m_aQueueInfos.size();
        m_aPrinterList.push_back( pData->maPrinterName );
        m_aQueueInfos.push_back( ImplPrnQueueData() );
        m_aQueueInfos.back().mpQueueInfo = nullptr;
        m_aQueueInfos.back().mpSalQueueInfo = std::move(pData);
    }
    else // this should not happen, but ...
    {
        ImplPrnQueueData& rData = m_aQueueInfos[ it->second ];
        rData.mpQueueInfo.reset();
        rData.mpSalQueueInfo = std::move(pData);
    }
}

ImplPrnQueueData* ImplPrnQueueList::Get( const OUString& rPrinter )
{
    ImplPrnQueueData* pData = nullptr;
    std::unordered_map<OUString,sal_Int32>::iterator it =
        m_aNameToIndex.find( rPrinter );
    if( it != m_aNameToIndex.end() )
        pData = &m_aQueueInfos[it->second];
    return pData;
}

static void ImplInitPrnQueueList()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maGDIData.mpPrinterQueueList.reset(new ImplPrnQueueList);

    static const char* pEnv = getenv( "SAL_DISABLE_PRINTERLIST" );
    if( !pEnv || !*pEnv )
        pSVData->mpDefInst->GetPrinterQueueInfo( pSVData->maGDIData.mpPrinterQueueList.get() );
}

void ImplDeletePrnQueueList()
{
    ImplSVData*         pSVData = ImplGetSVData();
    pSVData->maGDIData.mpPrinterQueueList.reset();
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
            pSVData->mpDefInst->GetPrinterQueueState( pInfo->mpSalQueueInfo.get() );

        if ( !pInfo->mpQueueInfo )
            pInfo->mpQueueInfo.reset(new QueueInfo);

        pInfo->mpQueueInfo->maPrinterName   = pInfo->mpSalQueueInfo->maPrinterName;
        pInfo->mpQueueInfo->maDriver        = pInfo->mpSalQueueInfo->maDriver;
        pInfo->mpQueueInfo->maLocation      = pInfo->mpSalQueueInfo->maLocation;
        pInfo->mpQueueInfo->maComment       = pInfo->mpSalQueueInfo->maComment;
        pInfo->mpQueueInfo->mnStatus        = pInfo->mpSalQueueInfo->mnStatus;
        pInfo->mpQueueInfo->mnJobs          = pInfo->mpSalQueueInfo->mnJobs;
        return pInfo->mpQueueInfo.get();
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
    mbDefPrinter        = false;
    mnError             = ERRCODE_NONE;
    mnPageQueueSize     = 0;
    mnCopyCount         = 1;
    mbCollateCopy       = false;
    mbPrinting          = false;
    mbJobActive         = false;
    mbPrintFile         = false;
    mbInPrintPage       = false;
    mbNewJobSetup       = false;
    mbSinglePrintJobs   = false;
    mpInfoPrinter       = nullptr;
    mpPrinter           = nullptr;
    mpDisplayDev        = nullptr;
    mpPrinterOptions.reset(new vcl::print::PrinterOptions);

    // Add printer to the list
    ImplSVData* pSVData = ImplGetSVData();
    mpNext = pSVData->maGDIData.mpFirstPrinter;
    mpPrev = nullptr;
    if ( mpNext )
        mpNext->mpPrev = this;
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
        mpGraphics->SetXORMode( (RasterOp::Invert == meRasterOp) || (RasterOp::Xor == meRasterOp), RasterOp::Invert == meRasterOp );
        mpGraphics->setAntiAlias(bool(mnAntialiasing & AntialiasingFlags::Enable));
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

    mpFontInstance.clear();
    mpDeviceFontList.reset();
    mpDeviceFontSizeList.reset();
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
                pSVData->maGDIData.mpFirstPrnGraphics = static_cast<Printer*>(mpNextGraphics.get());
            if ( mpNextGraphics )
                mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
            else
                pSVData->maGDIData.mpLastPrnGraphics = static_cast<Printer*>(mpPrevGraphics.get());
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
    vcl::print::ImplJobSetup& rData = maJobSetup.ImplGetData();
    if ( rData.GetDriverData() )
    {
        if ( rData.GetPrinterName() != pInfo->maPrinterName ||
             rData.GetDriver() != pInfo->maDriver )
        {
            std::free( const_cast<sal_uInt8*>(rData.GetDriverData()) );
            rData.SetDriverData(nullptr);
            rData.SetDriverDataLen(0);
        }
    }

    // Remember printer name
    maPrinterName = pInfo->maPrinterName;
    maDriver = pInfo->maDriver;

    // Add printer name to JobSetup
    rData.SetPrinterName( maPrinterName );
    rData.SetDriver( maDriver );

    mpInfoPrinter   = pSVData->mpDefInst->CreateInfoPrinter( pInfo, &rData );
    mpPrinter       = nullptr;
    mpJobGraphics   = nullptr;
    ImplUpdateJobSetupPaper( maJobSetup );

    if ( !mpInfoPrinter )
    {
        ImplInitDisplay();
        return;
    }

    // we need a graphics
    if ( !AcquireGraphics() )
    {
        ImplInitDisplay();
        return;
    }

    // Init data
    ImplUpdatePageData();
    mxFontCollection = std::make_shared<PhysicalFontCollection>();
    mxFontCache = std::make_shared<ImplFontCache>();
    mpGraphics->GetDevFontList(mxFontCollection.get());
}

void Printer::ImplInitDisplay()
{
    ImplSVData* pSVData = ImplGetSVData();

    mpInfoPrinter       = nullptr;
    mpPrinter           = nullptr;
    mpJobGraphics       = nullptr;

    mpDisplayDev = VclPtr<VirtualDevice>::Create();
    mxFontCollection    = pSVData->maGDIData.mxScreenFontList;
    mxFontCache         = pSVData->maGDIData.mxScreenFontCache;
    mnDPIX              = mpDisplayDev->mnDPIX;
    mnDPIY              = mpDisplayDev->mnDPIY;
}

void Printer::DrawDeviceMask( const Bitmap& rMask, const Color& rMaskColor,
                         const Point& rDestPt, const Size& rDestSize,
                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    tools::Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Justify();

    if( !(!rMask.IsEmpty() && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height()) )
        return;

    Bitmap  aMask( rMask );
    BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

    if (aMask.getPixelFormat() > vcl::PixelFormat::N1_BPP)
        aMask.Convert( BmpConversion::N1BitThreshold );

    // mirrored horizontally
    if( aDestSz.Width() < 0 )
    {
        aDestSz.setWidth( -aDestSz.Width() );
        aDestPt.AdjustX( -( aDestSz.Width() - 1 ) );
        nMirrFlags |= BmpMirrorFlags::Horizontal;
    }

    // mirrored vertically
    if( aDestSz.Height() < 0 )
    {
        aDestSz.setHeight( -aDestSz.Height() );
        aDestPt.AdjustY( -( aDestSz.Height() - 1 ) );
        nMirrFlags |= BmpMirrorFlags::Vertical;
    }

    // source cropped?
    if( aSrcRect != tools::Rectangle( Point(), aMask.GetSizePixel() ) )
        aMask.Crop( aSrcRect );

    // destination mirrored
    if( nMirrFlags != BmpMirrorFlags::NONE)
        aMask.Mirror( nMirrFlags );

    // do painting
    const tools::Long      nSrcWidth = aSrcRect.GetWidth(), nSrcHeight = aSrcRect.GetHeight();
    tools::Long            nX, nY; //, nWorkX, nWorkY, nWorkWidth, nWorkHeight;
    std::unique_ptr<tools::Long[]> pMapX( new tools::Long[ nSrcWidth + 1 ] );
    std::unique_ptr<tools::Long[]> pMapY( new tools::Long[ nSrcHeight + 1 ] );
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
    for( nX = 0; nX <= nSrcWidth; nX++ )
        pMapX[ nX ] = aDestPt.X() + FRound( static_cast<double>(aDestSz.Width()) * nX / nSrcWidth );

    for( nY = 0; nY <= nSrcHeight; nY++ )
        pMapY[ nY ] = aDestPt.Y() + FRound( static_cast<double>(aDestSz.Height()) * nY / nSrcHeight );

    // walk through all rectangles of mask
    const vcl::Region aWorkRgn(aMask.CreateRegion(COL_BLACK, tools::Rectangle(Point(), aMask.GetSizePixel())));
    RectangleVector aRectangles;
    aWorkRgn.GetRegionRectangles(aRectangles);

    for (auto const& rectangle : aRectangles)
    {
        const Point aMapPt(pMapX[rectangle.Left()], pMapY[rectangle.Top()]);
        const Size aMapSz(
            pMapX[rectangle.Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
            pMapY[rectangle.Bottom() + 1] - aMapPt.Y());    // same for Y

        DrawRect(tools::Rectangle(aMapPt, aMapSz));
    }

    Pop();
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}

SalPrinterQueueInfo* Printer::ImplGetQueueInfo( const OUString& rPrinterName,
                                                const OUString* pDriver )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maGDIData.mpPrinterQueueList )
        ImplInitPrnQueueList();

    ImplPrnQueueList* pPrnList = pSVData->maGDIData.mpPrinterQueueList.get();
    if ( pPrnList && !pPrnList->m_aQueueInfos.empty() )
    {
        // first search for the printer name directly
        ImplPrnQueueData* pInfo = pPrnList->Get( rPrinterName );
        if( pInfo )
            return pInfo->mpSalQueueInfo.get();

        // then search case insensitive
        for(const ImplPrnQueueData & rQueueInfo : pPrnList->m_aQueueInfos)
        {
            if( rQueueInfo.mpSalQueueInfo->maPrinterName.equalsIgnoreAsciiCase( rPrinterName ) )
                return rQueueInfo.mpSalQueueInfo.get();
        }

        // then search for driver name
        if ( pDriver )
        {
            for(const ImplPrnQueueData & rQueueInfo : pPrnList->m_aQueueInfos)
            {
                if( rQueueInfo.mpSalQueueInfo->maDriver == *pDriver )
                    return rQueueInfo.mpSalQueueInfo.get();
            }
        }

        // then the default printer
        pInfo = pPrnList->Get( GetDefaultPrinterName() );
        if( pInfo )
            return pInfo->mpSalQueueInfo.get();

        // last chance: the first available printer
        return pPrnList->m_aQueueInfos[0].mpSalQueueInfo.get();
    }

    return nullptr;
}

void Printer::ImplUpdatePageData()
{
    // we need a graphics
    if ( !AcquireGraphics() )
        return;

    mpGraphics->GetResolution( mnDPIX, mnDPIY );
    mpInfoPrinter->GetPageInfo( &maJobSetup.ImplGetConstData(),
                                mnOutWidth, mnOutHeight,
                                maPageOffset,
                                maPaperSize );
}

void Printer::ImplUpdateFontList()
{
    ImplUpdateFontData();
}

tools::Long Printer::GetGradientStepCount( tools::Long nMinRect )
{
    // use display-equivalent step size calculation
    tools::Long nInc = (nMinRect < 800) ? 10 : 20;

    return nInc;
}

Printer::Printer()
    : OutputDevice(OUTDEV_PRINTER)
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
        ImplInitDisplay();
}

Printer::Printer( const JobSetup& rJobSetup )
    : OutputDevice(OUTDEV_PRINTER)
    , maJobSetup(rJobSetup)
{
    ImplInitData();
    const vcl::print::ImplJobSetup& rConstData = rJobSetup.ImplGetConstData();
    OUString aDriver = rConstData.GetDriver();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rConstData.GetPrinterName(),
                                                   &aDriver );
    if ( pInfo )
    {
        ImplInit( pInfo );
        SetJobSetup( rJobSetup );
    }
    else
    {
        ImplInitDisplay();
        maJobSetup = JobSetup();
    }
}

Printer::Printer( const QueueInfo& rQueueInfo )
    : OutputDevice(OUTDEV_PRINTER)
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rQueueInfo.GetPrinterName(),
                                                   &rQueueInfo.GetDriver() );
    if ( pInfo )
        ImplInit( pInfo );
    else
        ImplInitDisplay();
}

Printer::Printer( const OUString& rPrinterName )
    : OutputDevice(OUTDEV_PRINTER)
{
    ImplInitData();
    SalPrinterQueueInfo* pInfo = ImplGetQueueInfo( rPrinterName, nullptr );
    if ( pInfo )
        ImplInit( pInfo );
    else
        ImplInitDisplay();
}

Printer::~Printer()
{
    disposeOnce();
}

void Printer::dispose()
{
    SAL_WARN_IF( IsPrinting(), "vcl.gdi", "Printer::~Printer() - Job is printing" );
    SAL_WARN_IF( IsJobActive(), "vcl.gdi", "Printer::~Printer() - Job is active" );

    mpPrinterOptions.reset();

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
        mpFontInstance.clear();
        mpDeviceFontList.reset();
        mpDeviceFontSizeList.reset();
        mxFontCache.reset();
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

    mpPrev.clear();
    mpNext.clear();
    OutputDevice::dispose();
}

Size Printer::GetButtonBorderSize()
{
    Size aBrdSize(LogicToPixel(Size(20, 20), MapMode(MapUnit::Map100thMM)));

    if (!aBrdSize.Width())
        aBrdSize.setWidth(1);

    if (!aBrdSize.Height())
        aBrdSize.setHeight(1);

    return aBrdSize;
}

sal_uInt32 Printer::GetCapabilities( PrinterCapType nType ) const
{
    if ( IsDisplayPrinter() )
        return 0;

    if( mpInfoPrinter )
        return mpInfoPrinter->GetCapabilities( &maJobSetup.ImplGetConstData(), nType );
    else
        return 0;
}

bool Printer::HasSupport( PrinterSupport eFeature ) const
{
    switch ( eFeature )
    {
        case PrinterSupport::SetOrientation:
            return GetCapabilities( PrinterCapType::SetOrientation ) != 0;
        case PrinterSupport::SetPaperSize:
            return GetCapabilities( PrinterCapType::SetPaperSize ) != 0;
        case PrinterSupport::SetPaper:
            return GetCapabilities( PrinterCapType::SetPaper ) != 0;
        case PrinterSupport::CollateCopy:
            return (GetCapabilities( PrinterCapType::CollateCopies ) != 0);
        case PrinterSupport::SetupDialog:
            return GetCapabilities( PrinterCapType::SupportDialog ) != 0;
    }

    return true;
}

bool Printer::SetJobSetup( const JobSetup& rSetup )
{
    if ( IsDisplayPrinter() || mbInPrintPage )
        return false;

    JobSetup aJobSetup = rSetup;

    ReleaseGraphics();
    if ( mpInfoPrinter->SetPrinterData( &aJobSetup.ImplGetData() ) )
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

bool Printer::Setup(weld::Window* pWindow, PrinterSetupMode eMode)
{
    if ( IsDisplayPrinter() )
        return false;

    if ( IsJobActive() || IsPrinting() )
        return false;

    JobSetup aJobSetup = maJobSetup;
    vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();
    rData.SetPrinterSetupMode( eMode );
    // TODO: orig page size

    if (!pWindow)
    {
        vcl::Window* pDefWin = ImplGetDefaultWindow();
        pWindow = pDefWin ? pDefWin->GetFrameWeld() : nullptr;
    }
    if( !pWindow )
        return false;

    ReleaseGraphics();
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode++;
    nImplSysDialog++;
    bool bSetup = mpInfoPrinter->Setup(pWindow, &rData);
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
            mpFontInstance.clear();
            mpDeviceFontList.reset();
            mpDeviceFontSizeList.reset();
            // clean up font list
            mxFontCache.reset();
            mxFontCollection.reset();

            mbInitFont = true;
            mbNewFont = true;
            mpInfoPrinter = nullptr;
        }

        // Construct new printer
        ImplInitDisplay();
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

            mpFontInstance.clear();
            mpDeviceFontList.reset();
            mpDeviceFontSizeList.reset();
            mxFontCache.reset();
            mxFontCollection.reset();
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
            ImplInitDisplay();
    }
    else
        SetJobSetup( pPrinter->GetJobSetup() );

    return false;
}

bool Printer::SetOrientation( Orientation eOrientation )
{
    if ( mbInPrintPage )
        return false;

    if ( maJobSetup.ImplGetConstData().GetOrientation() != eOrientation )
    {
        JobSetup      aJobSetup = maJobSetup;
        vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();

        rData.SetOrientation(eOrientation);

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        if ( mpInfoPrinter->SetData( JobSetFlags::ORIENTATION, &rData ) )
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
    return maJobSetup.ImplGetConstData().GetOrientation();
}

bool Printer::SetPaperBin( sal_uInt16 nPaperBin )
{
    if ( mbInPrintPage )
        return false;

    if ( maJobSetup.ImplGetConstData().GetPaperBin() != nPaperBin &&
         nPaperBin < GetPaperBinCount() )
    {
        JobSetup      aJobSetup = maJobSetup;
        vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();
        rData.SetPaperBin(nPaperBin);

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        if ( mpInfoPrinter->SetData( JobSetFlags::PAPERBIN, &rData ) )
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
    return maJobSetup.ImplGetConstData().GetPaperBin();
}

bool Printer::GetPrinterSettingsPreferred() const
{
    return maJobSetup.ImplGetConstData().GetPapersizeFromSetup();
}

// dear loplugins, DO NOT REMOVE this code
// it will be used in follow-up commits
void Printer::SetPrinterSettingsPreferred( bool bPaperSizeFromSetup)
{
    if ( maJobSetup.ImplGetConstData().GetPapersizeFromSetup() != bPaperSizeFromSetup )
    {
        JobSetup      aJobSetup = maJobSetup;
        vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();
        rData.SetPapersizeFromSetup(bPaperSizeFromSetup);

        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
    }
}

// Map user paper format to an available printer paper format
void Printer::ImplFindPaperFormatForUserSize( JobSetup& aJobSetup )
{
    vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();

    // The angle that a landscape page will be turned counterclockwise wrt to portrait.
    int     nLandscapeAngle = mpInfoPrinter ? mpInfoPrinter->GetLandscapeAngle( &maJobSetup.ImplGetConstData() ) : 900;
    int     nPaperCount     = GetPaperInfoCount();
    PaperInfo aInfo(rData.GetPaperWidth(), rData.GetPaperHeight());

    // Compare all paper formats and get the appropriate one
    for ( int i = 0; i < nPaperCount; i++ )
    {
        const PaperInfo& rPaperInfo = GetPaperInfo( i );

        if ( aInfo.sloppyEqual(rPaperInfo) )
        {
            rData.SetPaperFormat(
                ImplGetPaperFormat( rPaperInfo.getWidth(),
                    rPaperInfo.getHeight() ));
            rData.SetOrientation( Orientation::Portrait );
            return;
        }
    }

    // If the printer supports landscape orientation, check paper sizes again
    // with landscape orientation. This is necessary as a printer driver provides
    // all paper sizes with portrait orientation only!!
    if ( !(rData.GetPaperFormat() == PAPER_USER &&
         nLandscapeAngle != 0 &&
         HasSupport( PrinterSupport::SetOrientation )))
        return;

    const tools::Long nRotatedWidth = rData.GetPaperHeight();
    const tools::Long nRotatedHeight = rData.GetPaperWidth();
    PaperInfo aRotatedInfo(nRotatedWidth, nRotatedHeight);

    for ( int i = 0; i < nPaperCount; i++ )
    {
        const PaperInfo& rPaperInfo = GetPaperInfo( i );

        if ( aRotatedInfo.sloppyEqual( rPaperInfo ) )
        {
            rData.SetPaperFormat(
                ImplGetPaperFormat( rPaperInfo.getWidth(),
                    rPaperInfo.getHeight() ));
            rData.SetOrientation( Orientation::Landscape );
            return;
        }
    }
}

void Printer::SetPaper( Paper ePaper )
{
    if ( mbInPrintPage )
        return;

    if ( maJobSetup.ImplGetConstData().GetPaperFormat() == ePaper )
        return;

    JobSetup      aJobSetup = maJobSetup;
    vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();

    rData.SetPaperFormat( ePaper );
    if ( ePaper != PAPER_USER )
    {
        PaperInfo aInfo(ePaper);
        rData.SetPaperWidth( aInfo.getWidth() );
        rData.SetPaperHeight( aInfo.getHeight() );
    }

    if ( IsDisplayPrinter() )
    {
        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
        return;
    }

    ReleaseGraphics();
    if ( ePaper == PAPER_USER )
        ImplFindPaperFormatForUserSize( aJobSetup );
    if ( mpInfoPrinter->SetData( JobSetFlags::PAPERSIZE | JobSetFlags::ORIENTATION, &rData ))
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
    }
}

bool Printer::SetPaperSizeUser( const Size& rSize )
{
    if ( mbInPrintPage )
        return false;

    const Size aPixSize = LogicToPixel( rSize );
    const Size aPageSize = PixelToLogic(aPixSize, MapMode(MapUnit::Map100thMM));
    bool bNeedToChange(maJobSetup.ImplGetConstData().GetPaperWidth() != aPageSize.Width() ||
        maJobSetup.ImplGetConstData().GetPaperHeight() != aPageSize.Height());

    if(!bNeedToChange)
    {
        // #i122984# only need to change when Paper is different from PAPER_USER and
        // the mapped Paper which will created below in the call to ImplFindPaperFormatForUserSize
        // and will replace maJobSetup.ImplGetConstData()->GetPaperFormat(). This leads to
        // unnecessary JobSetups, e.g. when printing a multi-page fax, but also with
        // normal print
        const Paper aPaper = ImplGetPaperFormat(aPageSize.Width(), aPageSize.Height());

        bNeedToChange = maJobSetup.ImplGetConstData().GetPaperFormat() != PAPER_USER &&
            maJobSetup.ImplGetConstData().GetPaperFormat() != aPaper;
    }

    if(bNeedToChange)
    {
        JobSetup      aJobSetup = maJobSetup;
        vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();
        rData.SetPaperFormat( PAPER_USER );
        rData.SetPaperWidth( aPageSize.Width() );
        rData.SetPaperHeight( aPageSize.Height() );

        if ( IsDisplayPrinter() )
        {
            mbNewJobSetup = true;
            maJobSetup = aJobSetup;
            return true;
        }

        ReleaseGraphics();
        ImplFindPaperFormatForUserSize( aJobSetup );

        // Changing the paper size can also change the orientation!
        if ( mpInfoPrinter->SetData( JobSetFlags::PAPERSIZE | JobSetFlags::ORIENTATION, &rData ))
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
        mpInfoPrinter->InitPaperFormats( &maJobSetup.ImplGetConstData() );
    return mpInfoPrinter->m_aPaperFormats.size();
}

OUString Printer::GetPaperName( Paper ePaper )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maPaperNames.empty() )
    {
        static const int PaperIndex[] =
        {
            PAPER_A0, PAPER_A1, PAPER_A2, PAPER_A3, PAPER_A4, PAPER_A5, PAPER_B4_ISO, PAPER_B5_ISO,
            PAPER_LETTER, PAPER_LEGAL, PAPER_TABLOID, PAPER_USER, PAPER_B6_ISO, PAPER_ENV_C4, PAPER_ENV_C5,
            PAPER_ENV_C6, PAPER_ENV_C65, PAPER_ENV_DL, PAPER_SLIDE_DIA, PAPER_SCREEN_4_3, PAPER_C, PAPER_D,
            PAPER_E, PAPER_EXECUTIVE, PAPER_FANFOLD_LEGAL_DE, PAPER_ENV_MONARCH, PAPER_ENV_PERSONAL, PAPER_ENV_9,
            PAPER_ENV_10, PAPER_ENV_11, PAPER_ENV_12, PAPER_KAI16, PAPER_KAI32, PAPER_KAI32BIG, PAPER_B4_JIS,
            PAPER_B5_JIS, PAPER_B6_JIS, PAPER_LEDGER, PAPER_STATEMENT, PAPER_QUARTO, PAPER_10x14, PAPER_ENV_14,
            PAPER_ENV_C3, PAPER_ENV_ITALY, PAPER_FANFOLD_US, PAPER_FANFOLD_DE, PAPER_POSTCARD_JP, PAPER_9x11,
            PAPER_10x11, PAPER_15x11, PAPER_ENV_INVITE, PAPER_A_PLUS, PAPER_B_PLUS, PAPER_LETTER_PLUS, PAPER_A4_PLUS,
            PAPER_DOUBLEPOSTCARD_JP, PAPER_A6, PAPER_12x11, PAPER_A7, PAPER_A8, PAPER_A9, PAPER_A10, PAPER_B0_ISO,
            PAPER_B1_ISO, PAPER_B2_ISO, PAPER_B3_ISO, PAPER_B7_ISO, PAPER_B8_ISO, PAPER_B9_ISO, PAPER_B10_ISO,
            PAPER_ENV_C2, PAPER_ENV_C7, PAPER_ENV_C8, PAPER_ARCHA, PAPER_ARCHB, PAPER_ARCHC, PAPER_ARCHD,
            PAPER_ARCHE, PAPER_SCREEN_16_9, PAPER_SCREEN_16_10, PAPER_16K_195x270, PAPER_16K_197x273,
            PAPER_WIDESCREEN, PAPER_ONSCREENSHOW_4_3, PAPER_ONSCREENSHOW_16_9, PAPER_ONSCREENSHOW_16_10
        };
        assert(SAL_N_ELEMENTS(PaperIndex) == SAL_N_ELEMENTS(RID_STR_PAPERNAMES) && "localized paper name count wrong");
        for (size_t i = 0; i < SAL_N_ELEMENTS(PaperIndex); ++i)
            pSVData->maPaperNames[PaperIndex[i]] = VclResId(RID_STR_PAPERNAMES[i]);
    }

    std::unordered_map<int,OUString>::const_iterator it = pSVData->maPaperNames.find( static_cast<int>(ePaper) );
    return (it != pSVData->maPaperNames.end()) ? it->second : OUString();
}

const PaperInfo& Printer::GetPaperInfo( int nPaper ) const
{
    if( ! mpInfoPrinter )
        return ImplGetEmptyPaper();
    if( ! mpInfoPrinter->m_bPapersInit )
        mpInfoPrinter->InitPaperFormats( &maJobSetup.ImplGetConstData() );
    if( mpInfoPrinter->m_aPaperFormats.empty() || nPaper < 0 || nPaper >= int(mpInfoPrinter->m_aPaperFormats.size()) )
        return ImplGetEmptyPaper();
    return mpInfoPrinter->m_aPaperFormats[nPaper];
}

Size Printer::GetPaperSize( int nPaper )
{
    PaperInfo aInfo = GetPaperInfo( nPaper );
    return PixelToLogic( Size( aInfo.getWidth(), aInfo.getHeight() ) );
}

void Printer::SetDuplexMode( DuplexMode eDuplex )
{
    if ( mbInPrintPage )
        return;

    if ( maJobSetup.ImplGetConstData().GetDuplexMode() == eDuplex )
        return;

    JobSetup      aJobSetup = maJobSetup;
    vcl::print::ImplJobSetup& rData = aJobSetup.ImplGetData();

    rData.SetDuplexMode( eDuplex );

    if ( IsDisplayPrinter() )
    {
        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
        return;
    }

    ReleaseGraphics();
    if ( mpInfoPrinter->SetData( JobSetFlags::DUPLEXMODE, &rData ) )
    {
        ImplUpdateJobSetupPaper( aJobSetup );
        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
        ImplUpdatePageData();
        ImplUpdateFontList();
    }
}

DuplexMode Printer::GetDuplexMode() const
{
    return maJobSetup.ImplGetConstData().GetDuplexMode();
}

Paper Printer::GetPaper() const
{
    return maJobSetup.ImplGetConstData().GetPaperFormat();
}

sal_uInt16 Printer::GetPaperBinCount() const
{
    if ( IsDisplayPrinter() )
        return 0;

    return mpInfoPrinter->GetPaperBinCount( &maJobSetup.ImplGetConstData() );
}

OUString Printer::GetPaperBinName( sal_uInt16 nPaperBin ) const
{
    if ( IsDisplayPrinter() )
        return OUString();

    if ( nPaperBin < GetPaperBinCount() )
        return mpInfoPrinter->GetPaperBinName( &maJobSetup.ImplGetConstData(), nPaperBin );
    else
        return OUString();
}

void Printer::SetCopyCount( sal_uInt16 nCopy, bool bCollate )
{
    mnCopyCount = nCopy;
    mbCollateCopy = bCollate;
}

ErrCode Printer::ImplSalPrinterErrorCodeToVCL( SalPrinterError nError )
{
    ErrCode nVCLError;
    switch ( nError )
    {
        case SalPrinterError::NONE:
            nVCLError = ERRCODE_NONE;
            break;
        case SalPrinterError::Abort:
            nVCLError = PRINTER_ABORT;
            break;
        default:
            nVCLError = PRINTER_GENERALERROR;
            break;
    }

    return nVCLError;
}

void Printer::EndJob()
{
    if ( !IsJobActive() )
        return;

    SAL_WARN_IF( mbInPrintPage, "vcl.gdi", "Printer::EndJob() - StartPage() without EndPage() called" );

    mbJobActive = false;

    if ( mpPrinter )
    {
        ReleaseGraphics();

        mbPrinting      = false;

        mbDevOutput = false;
        mpPrinter->EndJob();
        mpPrinter.reset();
    }
}

void Printer::ImplStartPage()
{
    if ( !IsJobActive() )
        return;

    if ( !mpPrinter )
        return;

    SalGraphics* pGraphics = mpPrinter->StartPage( &maJobSetup.ImplGetData(),
                                                   mbNewJobSetup );
    if ( pGraphics )
    {
        ReleaseGraphics();
        mpJobGraphics = pGraphics;
    }
    mbDevOutput = true;

    // PrintJob not aborted ???
    if ( IsJobActive() )
        mbInPrintPage = true;
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
    ImplPrnQueueList*   pPrnList = pSVData->maGDIData.mpPrinterQueueList.get();

    if ( !pPrnList )
        return;

    std::unique_ptr<ImplPrnQueueList> pNewList(new ImplPrnQueueList);
    pSVData->mpDefInst->GetPrinterQueueInfo( pNewList.get() );

    bool bChanged = pPrnList->m_aQueueInfos.size() != pNewList->m_aQueueInfos.size();
    for( decltype(pPrnList->m_aQueueInfos)::size_type i = 0; ! bChanged && i < pPrnList->m_aQueueInfos.size(); i++ )
    {
        ImplPrnQueueData& rInfo     = pPrnList->m_aQueueInfos[i];
        ImplPrnQueueData& rNewInfo  = pNewList->m_aQueueInfos[i];
        if( ! rInfo.mpSalQueueInfo || ! rNewInfo.mpSalQueueInfo || // sanity check
            rInfo.mpSalQueueInfo->maPrinterName != rNewInfo.mpSalQueueInfo->maPrinterName )
        {
            bChanged = true;
        }
    }
    if( !bChanged )
        return;

    ImplDeletePrnQueueList();
    pSVData->maGDIData.mpPrinterQueueList = std::move(pNewList);

    Application* pApp = GetpApp();
    if( pApp )
    {
        DataChangedEvent aDCEvt( DataChangedEventType::PRINTER );
        Application::ImplCallEventListenersApplicationDataChanged(&aDCEvt);
        Application::NotifyAllWindows( aDCEvt );
    }
}

bool Printer::UsePolyPolygonForComplexGradient()
{
    return true;
}

void Printer::ClipAndDrawGradientMetafile ( const Gradient &rGradient, const tools::PolyPolygon &rPolyPoly )
{
    const tools::Rectangle aBoundRect( rPolyPoly.GetBoundRect() );

    Push( PushFlags::CLIPREGION );
    IntersectClipRegion(vcl::Region(rPolyPoly));
    DrawGradient( aBoundRect, rGradient );
    Pop();
}

void Printer::SetFontOrientation( LogicalFontInstance* const pFontEntry ) const
{
    pFontEntry->mnOrientation = pFontEntry->mxFontMetric->GetOrientation();
}

vcl::Region Printer::ClipToDeviceBounds(vcl::Region aRegion) const
{
    return aRegion;
}

Bitmap Printer::GetBitmap( const Point& rSrcPt, const Size& rSize ) const
{
    SAL_WARN("vcl.gdi", "GetBitmap(): This should never be called on by a Printer instance");

    return OutputDevice::GetBitmap( rSrcPt, rSize );
}

css::awt::DeviceInfo Printer::GetDeviceInfo() const
{
    Size aDevSz = GetPaperSizePixel();
    css::awt::DeviceInfo aInfo = GetCommonDeviceInfo(aDevSz);
    Size aOutSz = GetOutputSizePixel();
    Point aOffset = GetPageOffset();
    aInfo.LeftInset = aOffset.X();
    aInfo.TopInset = aOffset.Y();
    aInfo.RightInset = aDevSz.Width() - aOutSz.Width() - aOffset.X();
    aInfo.BottomInset = aDevSz.Height() - aOutSz.Height() - aOffset.Y();
    aInfo.Capabilities = 0;

    return aInfo;
}

void Printer::SetSystemTextColor(SystemTextColorFlags, bool)
{
    SetTextColor(COL_BLACK);
}

namespace {

struct PrintJobAsync
{
    std::shared_ptr<vcl::print::PrinterController>  mxController;
    JobSetup                            maInitSetup;

    PrintJobAsync(const std::shared_ptr<vcl::print::PrinterController>& i_xController,
                  const JobSetup& i_rInitSetup)
    : mxController( i_xController ), maInitSetup( i_rInitSetup )
    {}

    DECL_LINK( ExecJob, void*, void );
};

}
static OUString queryFile( Printer const * pPrinter )
{
    OUString aResult;

    css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
    css::uno::Reference< css::ui::dialogs::XFilePicker3 > xFilePicker = css::ui::dialogs::FilePicker::createWithMode(xContext, css::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION);

    try
    {
#ifdef UNX
        // add PostScript and PDF
        bool bPS = true, bPDF = true;
        if( pPrinter )
        {
            if( pPrinter->GetCapabilities( PrinterCapType::PDF ) )
                bPS = false;
            else
                bPDF = false;
        }
        if( bPS )
            xFilePicker->appendFilter( "PostScript", "*.ps" );
        if( bPDF )
            xFilePicker->appendFilter( "Portable Document Format", "*.pdf" );
#elif defined _WIN32
        (void)pPrinter;
        xFilePicker->appendFilter( "*.PRN", "*.prn" );
#endif
        // add arbitrary files
        xFilePicker->appendFilter(VclResId(SV_STDTEXT_ALLFILETYPES), "*.*");
    }
    catch (const css::lang::IllegalArgumentException&)
    {
        TOOLS_WARN_EXCEPTION( "vcl.gdi", "caught IllegalArgumentException when registering filter" );
    }

    if( xFilePicker->execute() == css::ui::dialogs::ExecutableDialogResults::OK )
    {
        css::uno::Sequence< OUString > aPathSeq( xFilePicker->getSelectedFiles() );
        INetURLObject aObj( aPathSeq[0] );
        aResult = aObj.PathToFileName();
    }
    return aResult;
}

IMPL_LINK_NOARG(PrintJobAsync, ExecJob, void*, void)
{
    Printer::ImplPrintJob(mxController, maInitSetup);

    // clean up, do not access members after this
    delete this;
}

void Printer::PrintJob(const std::shared_ptr<vcl::print::PrinterController>& i_xController,
                       const JobSetup& i_rInitSetup)
{
    bool bSynchronous = false;
    css::beans::PropertyValue* pVal = i_xController->getValue( "Wait" );
    if( pVal )
        pVal->Value >>= bSynchronous;

    if( bSynchronous )
        ImplPrintJob(i_xController, i_rInitSetup);
    else
    {
        PrintJobAsync* pAsync = new PrintJobAsync(i_xController, i_rInitSetup);
        Application::PostUserEvent( LINK( pAsync, PrintJobAsync, ExecJob ) );
    }
}

bool Printer::PreparePrintJob(std::shared_ptr<vcl::print::PrinterController> xController,
                           const JobSetup& i_rInitSetup)
{
    // check if there is a default printer; if not, show an error box (if appropriate)
    if( GetDefaultPrinterName().isEmpty() )
    {
        if (xController->isShowDialogs())
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(xController->getWindow(), "vcl/ui/errornoprinterdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("ErrorNoPrinterDialog"));
            xBox->run();
        }
        xController->setValue( "IsDirect",
                               css::uno::makeAny( false ) );
    }

    // setup printer

    // #i114306# changed behavior back from persistence
    // if no specific printer is already set, create the default printer
    if (!xController->getPrinter())
    {
        OUString aPrinterName( i_rInitSetup.GetPrinterName() );
        VclPtrInstance<Printer> xPrinter( aPrinterName );
        xPrinter->SetJobSetup(i_rInitSetup);
        xController->setPrinter(xPrinter);
        xController->setPapersizeFromSetup(xPrinter->GetPrinterSettingsPreferred());
    }

    // reset last page property
    xController->setLastPage(false);

    // update "PageRange" property inferring from other properties:
    // case 1: "Pages" set from UNO API ->
    //         setup "Print Selection" and insert "PageRange" attribute
    // case 2: "All pages" is selected
    //         update "Page range" attribute to have a sensible default,
    //         but leave "All" as selected

    // "Pages" attribute from API is now equivalent to "PageRange"
    // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
    // Argh ! That sure needs cleaning up
    css::beans::PropertyValue* pContentVal = xController->getValue("PrintRange");
    if( ! pContentVal )
        pContentVal = xController->getValue("PrintContent");

    // case 1: UNO API has set "Pages"
    css::beans::PropertyValue* pPagesVal = xController->getValue("Pages");
    if( pPagesVal )
    {
        OUString aPagesVal;
        pPagesVal->Value >>= aPagesVal;
        if( !aPagesVal.isEmpty() )
        {
            // "Pages" attribute from API is now equivalent to "PageRange"
            // AND "PrintContent" = 1 except calc where it is "PrintRange" = 1
            // Argh ! That sure needs cleaning up
            if( pContentVal )
            {
                pContentVal->Value <<= sal_Int32( 1 );
                xController->setValue("PageRange", pPagesVal->Value);
            }
        }
    }
    // case 2: is "All" selected ?
    else if( pContentVal )
    {
        sal_Int32 nContent = -1;
        if( pContentVal->Value >>= nContent )
        {
            if( nContent == 0 )
            {
                // do not overwrite PageRange if it is already set
                css::beans::PropertyValue* pRangeVal = xController->getValue("PageRange");
                OUString aRange;
                if( pRangeVal )
                    pRangeVal->Value >>= aRange;
                if( aRange.isEmpty() )
                {
                    sal_Int32 nPages = xController->getPageCount();
                    if( nPages > 0 )
                    {
                        OUStringBuffer aBuf( 32 );
                        aBuf.append( "1" );
                        if( nPages > 1 )
                        {
                            aBuf.append( "-" );
                            aBuf.append( nPages );
                        }
                        xController->setValue("PageRange", css::uno::makeAny(aBuf.makeStringAndClear()));
                    }
                }
            }
        }
    }

    css::beans::PropertyValue* pReverseVal = xController->getValue("PrintReverse");
    if( pReverseVal )
    {
        bool bReverse = false;
        pReverseVal->Value >>= bReverse;
        xController->setReversePrint( bReverse );
    }

    css::beans::PropertyValue* pPapersizeFromSetupVal = xController->getValue("PapersizeFromSetup");
    if( pPapersizeFromSetupVal )
    {
        bool bPapersizeFromSetup = false;
        pPapersizeFromSetupVal->Value >>= bPapersizeFromSetup;
        xController->setPapersizeFromSetup(bPapersizeFromSetup);
    }

    // setup NUp printing from properties
    sal_Int32 nRows = xController->getIntProperty("NUpRows", 1);
    sal_Int32 nCols = xController->getIntProperty("NUpColumns", 1);
    if( nRows > 1 || nCols > 1 )
    {
        vcl::print::PrinterController::MultiPageSetup aMPS;
        aMPS.nRows         = std::max<sal_Int32>(nRows, 1);
        aMPS.nColumns      = std::max<sal_Int32>(nCols, 1);
        sal_Int32 nValue = xController->getIntProperty("NUpPageMarginLeft", aMPS.nLeftMargin);
        if( nValue >= 0 )
            aMPS.nLeftMargin = nValue;
        nValue = xController->getIntProperty("NUpPageMarginRight", aMPS.nRightMargin);
        if( nValue >= 0 )
            aMPS.nRightMargin = nValue;
        nValue = xController->getIntProperty( "NUpPageMarginTop", aMPS.nTopMargin );
        if( nValue >= 0 )
            aMPS.nTopMargin = nValue;
        nValue = xController->getIntProperty( "NUpPageMarginBottom", aMPS.nBottomMargin );
        if( nValue >= 0 )
            aMPS.nBottomMargin = nValue;
        nValue = xController->getIntProperty( "NUpHorizontalSpacing", aMPS.nHorizontalSpacing );
        if( nValue >= 0 )
            aMPS.nHorizontalSpacing = nValue;
        nValue = xController->getIntProperty( "NUpVerticalSpacing", aMPS.nVerticalSpacing );
        if( nValue >= 0 )
            aMPS.nVerticalSpacing = nValue;
        aMPS.bDrawBorder = xController->getBoolProperty( "NUpDrawBorder", aMPS.bDrawBorder );
        aMPS.nOrder = static_cast<vcl::print::NupOrderType>(xController->getIntProperty( "NUpSubPageOrder", static_cast<sal_Int32>(aMPS.nOrder) ));
        aMPS.aPaperSize = xController->getPrinter()->PixelToLogic( xController->getPrinter()->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) );
        css::beans::PropertyValue* pPgSizeVal = xController->getValue( "NUpPaperSize" );
        css::awt::Size aSizeVal;
        if( pPgSizeVal && (pPgSizeVal->Value >>= aSizeVal) )
        {
            aMPS.aPaperSize.setWidth( aSizeVal.Width );
            aMPS.aPaperSize.setHeight( aSizeVal.Height );
        }

        xController->setMultipage( aMPS );
    }

    // in direct print case check whether there is anything to print.
    // if not, show an errorbox (if appropriate)
    if( xController->isShowDialogs() && xController->isDirectPrint() )
    {
        if( xController->getFilteredPageCount() == 0 )
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(xController->getWindow(), "vcl/ui/errornocontentdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("ErrorNoContentDialog"));
            xBox->run();
            return false;
        }
    }

    // check if the printer brings up its own dialog
    // in that case leave the work to that dialog
    if( ! xController->getPrinter()->GetCapabilities( PrinterCapType::ExternalDialog ) &&
        ! xController->isDirectPrint() &&
        xController->isShowDialogs()
        )
    {
        try
        {
            vcl::PrintDialog aDlg(xController->getWindow(), xController);
            if (!aDlg.run())
            {
                xController->abortJob();
                return false;
            }
            if (aDlg.isPrintToFile())
            {
                OUString aFile = queryFile( xController->getPrinter().get() );
                if( aFile.isEmpty() )
                {
                    xController->abortJob();
                    return false;
                }
                xController->setValue( "LocalFileName",
                                       css::uno::makeAny( aFile ) );
            }
            else if (aDlg.isSingleJobs())
            {
                xController->getPrinter()->SetSinglePrintJobs(true);
            }
        }
        catch (const std::bad_alloc&)
        {
        }
    }

    xController->pushPropertiesToPrinter();
    return true;
}

bool Printer::ExecutePrintJob(const std::shared_ptr<vcl::print::PrinterController>& xController)
{
    OUString aJobName;
    css::beans::PropertyValue* pJobNameVal = xController->getValue( "JobName" );
    if( pJobNameVal )
        pJobNameVal->Value >>= aJobName;

    return xController->getPrinter()->StartJob( aJobName, xController );
}

void Printer::FinishPrintJob(const std::shared_ptr<vcl::print::PrinterController>& xController)
{
    xController->resetPaperToLastConfigured();
    xController->jobFinished( xController->getJobState() );
}

void Printer::ImplPrintJob(const std::shared_ptr<vcl::print::PrinterController>& xController,
                           const JobSetup& i_rInitSetup)
{
    if (PreparePrintJob(xController, i_rInitSetup))
    {
        ExecutePrintJob(xController);
    }
    FinishPrintJob(xController);
}

bool Printer::StartJob( const OUString& i_rJobName, std::shared_ptr<vcl::print::PrinterController> const & i_xController)
{
    mnError = ERRCODE_NONE;

    if ( IsDisplayPrinter() )
        return false;

    if ( IsJobActive() || IsPrinting() )
        return false;

    sal_uInt32 nCopies = mnCopyCount;
    bool    bCollateCopy = mbCollateCopy;
    bool    bUserCopy = false;

    if ( nCopies > 1 )
    {
        const sal_uInt32 nDevCopy = GetCapabilities( bCollateCopy
            ? PrinterCapType::CollateCopies
            : PrinterCapType::Copies );

        // need to do copies by hand ?
        if ( nCopies > nDevCopy )
        {
            bUserCopy = true;
            nCopies = 1;
            bCollateCopy = false;
        }
    }
    else
        bCollateCopy = false;

    ImplSVData* pSVData = ImplGetSVData();
    mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

    if (!mpPrinter)
        return false;

    bool bSinglePrintJobs = i_xController->getPrinter()->IsSinglePrintJobs();

    css::beans::PropertyValue* pFileValue = i_xController->getValue("LocalFileName");
    if( pFileValue )
    {
        OUString aFile;
        pFileValue->Value >>= aFile;
        if( !aFile.isEmpty() )
        {
            mbPrintFile = true;
            maPrintFile = aFile;
            bSinglePrintJobs = false;
        }
    }

    OUString* pPrintFile = nullptr;
    if ( mbPrintFile )
        pPrintFile = &maPrintFile;
    mpPrinterOptions->ReadFromConfig( mbPrintFile );

    mbPrinting              = true;
    if( GetCapabilities( PrinterCapType::UsePullModel ) )
    {
        mbJobActive             = true;
        // SAL layer does all necessary page printing
        // and also handles showing a dialog
        // that also means it must call jobStarted when the dialog is finished
        // it also must set the JobState of the Controller
        if( mpPrinter->StartJob( pPrintFile,
                                 i_rJobName,
                                 Application::GetDisplayName(),
                                 &maJobSetup.ImplGetData(),
                                 *i_xController) )
        {
            EndJob();
        }
        else
        {
            mnError = ImplSalPrinterErrorCodeToVCL(mpPrinter->GetErrorCode());
            if ( !mnError )
                mnError = PRINTER_GENERALERROR;
            mbPrinting = false;
            mpPrinter.reset();
            mbJobActive = false;

            GDIMetaFile aDummyFile;
            i_xController->setLastPage(true);
            i_xController->getFilteredPageFile(0, aDummyFile);

            return false;
        }
    }
    else
    {
        // possibly a dialog has been shown
        // now the real job starts
        i_xController->setJobState( css::view::PrintableState_JOB_STARTED );
        i_xController->jobStarted();

        int nJobs = 1;
        int nOuterRepeatCount = 1;
        int nInnerRepeatCount = 1;
        if( bUserCopy )
        {
            if( mbCollateCopy )
                nOuterRepeatCount = mnCopyCount;
            else
                nInnerRepeatCount = mnCopyCount;
        }
        if( bSinglePrintJobs )
        {
            nJobs = mnCopyCount;
            nCopies = 1;
            nOuterRepeatCount = nInnerRepeatCount = 1;
        }

        for( int nJobIteration = 0; nJobIteration < nJobs; nJobIteration++ )
        {
            bool bError = false;
            if( mpPrinter->StartJob( pPrintFile,
                                     i_rJobName,
                                     Application::GetDisplayName(),
                                     nCopies,
                                     bCollateCopy,
                                     i_xController->isDirectPrint(),
                                     &maJobSetup.ImplGetData() ) )
            {
                bool bAborted = false;
                mbJobActive             = true;
                i_xController->createProgressDialog();
                const int nPages = i_xController->getFilteredPageCount();
                // abort job, if no pages will be printed.
                if ( nPages == 0 )
                {
                    i_xController->abortJob();
                    bAborted = true;
                }
                for( int nOuterIteration = 0; nOuterIteration < nOuterRepeatCount && ! bAborted; nOuterIteration++ )
                {
                    for( int nPage = 0; nPage < nPages && ! bAborted; nPage++ )
                    {
                        for( int nInnerIteration = 0; nInnerIteration < nInnerRepeatCount && ! bAborted; nInnerIteration++ )
                        {
                            if( nPage == nPages-1 &&
                                nOuterIteration == nOuterRepeatCount-1 &&
                                nInnerIteration == nInnerRepeatCount-1 &&
                                nJobIteration == nJobs-1 )
                            {
                                i_xController->setLastPage(true);
                            }
                            i_xController->printFilteredPage(nPage);
                            if (i_xController->isProgressCanceled())
                            {
                                i_xController->abortJob();
                            }
                            if (i_xController->getJobState() ==
                                    css::view::PrintableState_JOB_ABORTED)
                            {
                                bAborted = true;
                            }
                        }
                    }
                    // FIXME: duplex ?
                }
                EndJob();

                if( nJobIteration < nJobs-1 )
                {
                    mpPrinter = pSVData->mpDefInst->CreatePrinter( mpInfoPrinter );

                    if ( mpPrinter )
                        mbPrinting              = true;
                    else
                        bError = true;
                }
            }
            else
                bError = true;

            if( bError )
            {
                mnError = mpPrinter ? ImplSalPrinterErrorCodeToVCL(mpPrinter->GetErrorCode()) : ERRCODE_NONE;
                if ( !mnError )
                    mnError = PRINTER_GENERALERROR;
                i_xController->setJobState( mnError == PRINTER_ABORT
                                            ? css::view::PrintableState_JOB_ABORTED
                                            : css::view::PrintableState_JOB_FAILED );
                mbPrinting = false;
                mpPrinter.reset();

                return false;
            }
        }

        if (i_xController->getJobState() == css::view::PrintableState_JOB_STARTED)
            i_xController->setJobState(css::view::PrintableState_JOB_SPOOLED);
    }

    // make last used printer persistent for UI jobs
    if (i_xController->isShowDialogs() && !i_xController->isDirectPrint())
    {
        vcl::SettingsConfigItem* pItem = vcl::SettingsConfigItem::get();
        pItem->setValue( "PrintDialog",
                         "LastPrinterUsed",
                         GetName()
                         );
    }

    return true;
}

void Printer::DrawGradientEx( OutputDevice* pOut, const tools::Rectangle& rRect, const Gradient& rGradient )
{
    const vcl::print::PrinterOptions& rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceGradients() )
    {
        if( vcl::print::PrinterGradientMode::Stripes == rPrinterOptions.GetReducedGradientMode() )
        {
            if( !rGradient.GetSteps() || ( rGradient.GetSteps() > rPrinterOptions.GetReducedGradientStepCount() ) )
            {
                Gradient aNewGradient( rGradient );

                aNewGradient.SetSteps( rPrinterOptions.GetReducedGradientStepCount() );
                pOut->DrawGradient( rRect, aNewGradient );
            }
            else
                pOut->DrawGradient( rRect, rGradient );
        }
        else
        {
            const Color&    rStartColor = rGradient.GetStartColor();
            const Color&    rEndColor = rGradient.GetEndColor();
            const tools::Long      nR = ( ( static_cast<tools::Long>(rStartColor.GetRed()) * rGradient.GetStartIntensity() ) / 100 +
                                   ( static_cast<tools::Long>(rEndColor.GetRed()) * rGradient.GetEndIntensity() ) / 100 ) >> 1;
            const tools::Long      nG = ( ( static_cast<tools::Long>(rStartColor.GetGreen()) * rGradient.GetStartIntensity() ) / 100 +
                                   ( static_cast<tools::Long>(rEndColor.GetGreen()) * rGradient.GetEndIntensity() ) / 100 ) >> 1;
            const tools::Long      nB = ( ( static_cast<tools::Long>(rStartColor.GetBlue()) * rGradient.GetStartIntensity() ) / 100 +
                                   ( static_cast<tools::Long>(rEndColor.GetBlue()) * rGradient.GetEndIntensity() ) / 100 ) >> 1;
            const Color     aColor( static_cast<sal_uInt8>(nR), static_cast<sal_uInt8>(nG), static_cast<sal_uInt8>(nB) );

            pOut->Push( PushFlags::LINECOLOR | PushFlags::FILLCOLOR );
            pOut->SetLineColor( aColor );
            pOut->SetFillColor( aColor );
            pOut->DrawRect( rRect );
            pOut->Pop();
        }
    }
    else
        pOut->DrawGradient( rRect, rGradient );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
