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

#include <sal/config.h>

#include <sal/types.h>
#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <o3tl/safeint.hxx>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>

#include <vcl/QueueInfo.hxx>
#include <vcl/event.hxx>
#include <vcl/virdev.hxx>
#include <vcl/print.hxx>
#include <vcl/printer/Options.hxx>

#include <jobset.h>
#include <print.h>
#include <ImplOutDevData.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <font/fontsubstitution.hxx>
#include <impfontcache.hxx>
#include <print.hrc>
#include <salgdi.hxx>
#include <salinst.hxx>
#include <salprn.hxx>
#include <salptype.hxx>
#include <salvd.hxx>
#include <svdata.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Sequence.h>

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
    const ImplJobSetup& rConstData = rJobSetup.ImplGetConstData();

    if ( !rConstData.GetPaperWidth() || !rConstData.GetPaperHeight() )
    {
        if ( rConstData.GetPaperFormat() != PAPER_USER )
        {
            PaperInfo aInfo(rConstData.GetPaperFormat());

            ImplJobSetup& rData = rJobSetup.ImplGetData();
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

void Printer::ImplPrintTransparent( const Bitmap& rBmp,
                                         const Point& rDestPt, const Size& rDestSize,
                                         const Point& rSrcPtPixel, const Size& rSrcSizePixel )
{
    Point       aDestPt( LogicToPixel( rDestPt ) );
    Size        aDestSz( LogicToPixel( rDestSize ) );
    tools::Rectangle   aSrcRect( rSrcPtPixel, rSrcSizePixel );

    aSrcRect.Normalize();

    if( rBmp.IsEmpty() || !aSrcRect.GetWidth() || !aSrcRect.GetHeight() || !aDestSz.Width() || !aDestSz.Height() )
        return;

    Bitmap  aPaint( rBmp );
    BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

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
    }

    // destination mirrored
    if( nMirrFlags != BmpMirrorFlags::NONE )
    {
        aPaint.Mirror( nMirrFlags );
    }

    // we always want to have a mask
    AlphaMask aAlphaMask(aSrcRect.GetSize());
    aAlphaMask.Erase( 0 );

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

    tools::Rectangle rectangle { Point(0,0), aSrcRect.GetSize() };
    const Point aMapPt(pMapX[rectangle.Left()], pMapY[rectangle.Top()]);
    const Size aMapSz( pMapX[rectangle.Right() + 1] - aMapPt.X(),      // pMapX[L + W] -> L + ((R - L) + 1) -> R + 1
                       pMapY[rectangle.Bottom() + 1] - aMapPt.Y());    // same for Y
    Bitmap aBandBmp(aPaint);

    DrawBitmap(aMapPt, aMapSz, Point(), aBandBmp.GetSizePixel(), aBandBmp);

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
        aBmp.Blend( rBmpEx.GetAlphaMask(), COL_WHITE );
        DrawBitmap( rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel, aBmp );
    }
    else
    {
        Bitmap aBmp( rBmpEx.GetBitmap() );
        ImplPrintTransparent( aBmp, rDestPt, rDestSize, rSrcPtPixel, rSrcSizePixel );
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

    Push( vcl::PushFlags::CLIPREGION | vcl::PushFlags::LINECOLOR );
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

void Printer::SetPrinterOptions( const vcl::printer::Options& i_rOptions )
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
    assert(pSVData->maGDIData.mpPrinterQueueList && "mpPrinterQueueList exists by now");
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
    mbUsePrintSetting   = false;
    mbResetPrintArea    = false;
    mpInfoPrinter       = nullptr;
    mpPrinter           = nullptr;
    mpDisplayDev        = nullptr;
    mpPrinterOptions.reset(new vcl::printer::Options);

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
    mpGraphics->ReleaseFonts();
    mbNewFont = true;
    mbInitFont = true;

    mpFontInstance.clear();
    mpFontFaceCollection.reset();
}

void Printer::ImplReleaseGraphics(bool bRelease)
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

void Printer::ReleaseGraphics(bool bRelease)
{
    ImplReleaseGraphics(bRelease);
}

void Printer::ImplInit( SalPrinterQueueInfo* pInfo )
{
    ImplSVData* pSVData = ImplGetSVData();
    // #i74084# update info for this specific SalPrinterQueueInfo
    pSVData->mpDefInst->GetPrinterQueueState( pInfo );

    // Test whether the driver actually matches the JobSetup
    ImplJobSetup& rData = maJobSetup.ImplGetData();
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
    mxFontCollection = std::make_shared<vcl::font::PhysicalFontCollection>();
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

    aSrcRect.Normalize();

    if( !(!rMask.IsEmpty() && aSrcRect.GetWidth() && aSrcRect.GetHeight() && aDestSz.Width() && aDestSz.Height()) )
        return;

    Bitmap  aMask( rMask );
    BmpMirrorFlags nMirrFlags = BmpMirrorFlags::NONE;

    if (aMask.getPixelFormat() >= vcl::PixelFormat::N8_BPP)
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
    Push( vcl::PushFlags::FILLCOLOR | vcl::PushFlags::LINECOLOR );
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
    const ImplJobSetup& rConstData = rJobSetup.ImplGetConstData();
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

    ImplReleaseGraphics();
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
        mpFontFaceCollection.reset();
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
    ImplJobSetup& rData = aJobSetup.ImplGetData();
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
            mpFontFaceCollection.reset();
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
            mpFontFaceCollection.reset();
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
        ImplJobSetup& rData = aJobSetup.ImplGetData();

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
        ImplJobSetup& rData = aJobSetup.ImplGetData();
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
        ImplJobSetup& rData = aJobSetup.ImplGetData();
        rData.SetPapersizeFromSetup(bPaperSizeFromSetup);

        mbNewJobSetup = true;
        maJobSetup = aJobSetup;
    }
}

// Map user paper format to an available printer paper format
void Printer::ImplFindPaperFormatForUserSize( JobSetup& aJobSetup )
{
    ImplJobSetup& rData = aJobSetup.ImplGetData();

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
    ImplJobSetup& rData = aJobSetup.ImplGetData();

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
        ImplJobSetup& rData = aJobSetup.ImplGetData();
        rData.SetPaperFormat( PAPER_USER );
        rData.SetPaperWidth( aPageSize.Width() );
        rData.SetPaperHeight( aPageSize.Height() );
        rData.SetOrientation( Orientation::Portrait );

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
        // This array must (probably) match exactly the enum Paper in <i18nutil/paper.hxx>
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
        static_assert(SAL_N_ELEMENTS(PaperIndex) == SAL_N_ELEMENTS(RID_STR_PAPERNAMES), "localized paper name count wrong");
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
    if( mpInfoPrinter->m_aPaperFormats.empty() || nPaper < 0 || o3tl::make_unsigned(nPaper) >= mpInfoPrinter->m_aPaperFormats.size() )
        return ImplGetEmptyPaper();
    return mpInfoPrinter->m_aPaperFormats[nPaper];
}

Size Printer::GetPaperSize( int nPaper ) const
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
    ImplJobSetup& rData = aJobSetup.ImplGetData();

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

Size Printer::GetSizeOfPaper() const
{
    return Size(maJobSetup.ImplGetConstData().GetPaperWidth(), maJobSetup.ImplGetConstData().GetPaperHeight());
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

sal_uInt16 Printer::GetPaperBinBySourceIndex(sal_uInt16 nPaperSource) const
{
    if ( IsDisplayPrinter() )
        return 0;

    return mpInfoPrinter->GetPaperBinBySourceIndex( &maJobSetup.ImplGetConstData(), nPaperSource );
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
        ReleaseGraphics();
        mpPrinter->EndPage();
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

    Push( vcl::PushFlags::CLIPREGION );
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

void Printer::SetWaveLineColors(Color const& rColor, tools::Long)
{
    if (mbLineColor || mbInitLineColor)
    {
        mpGraphics->SetLineColor();
        mbInitLineColor = true;
    }

    mpGraphics->SetFillColor(rColor);
    mbInitFillColor = true;
}

Size Printer::GetWaveLineSize(tools::Long nLineWidth) const
{
    // FIXME - do we have a bug here? If the linewidth is 0, then we will return
    // Size(0, 0) - is this correct?
    return Size(nLineWidth, ((nLineWidth*mnDPIX)+(mnDPIY/2))/mnDPIY);
}

void Printer::SetSystemTextColor(SystemTextColorFlags, bool)
{
    SetTextColor(COL_BLACK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
