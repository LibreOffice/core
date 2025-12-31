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
#include <pdf/pdfwriter_impl.hxx>
#include <pdf/EncryptionHashTransporter.hxx>

#include <vcl/dibtools.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/graph.hxx>
#include <pdf/IPDFEncryptor.hxx>

#include <unotools/streamwrap.hxx>

#include <tools/helpers.hxx>
#include <tools/fract.hxx>
#include <tools/mapunit.hxx>
#include <tools/stream.hxx>

#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <o3tl/unit_conversion.hxx>
#include <osl/diagnose.h>
#include <vcl/skia/SkiaHelper.hxx>

#include <sal/log.hxx>
#include <memory>

using namespace vcl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

static bool lcl_canUsePDFAxialShading(const Gradient& rGradient);

void PDFWriterImpl::implWriteGradient( const tools::PolyPolygon& i_rPolyPoly, const Gradient& i_rGradient,
                                       VirtualDevice* i_pDummyVDev, const vcl::PDFWriter::PlayMetafileContext& i_rContext )
{
    GDIMetaFile        aTmpMtf;
    Gradient aGradient(i_rGradient);

    aGradient.AddGradientActions( i_rPolyPoly.GetBoundRect(), aTmpMtf );

    m_rOuterFace.Push();
    m_rOuterFace.IntersectClipRegion( i_rPolyPoly.getB2DPolyPolygon() );
    playMetafile( aTmpMtf, nullptr, i_rContext, i_pDummyVDev );
    m_rOuterFace.Pop();
}

void PDFWriterImpl::implWriteBitmapEx( const Point& i_rPoint, const Size& i_rSize, const Bitmap& i_rBitmap, const Graphic& i_Graphic,
                                       VirtualDevice const * i_pDummyVDev, const vcl::PDFWriter::PlayMetafileContext& i_rContext )
{
    if ( i_rBitmap.IsEmpty() || !i_rSize.Width() || !i_rSize.Height() )
        return;

    Bitmap          aBitmap( i_rBitmap );
    Point           aPoint( i_rPoint );
    Size            aSize( i_rSize );

    // #i19065# Negative sizes have mirror semantics on
    // OutputDevice. Bitmap and co. have no idea about that, so
    // perform that _before_ doing anything with aBitmapEx.
    BmpMirrorFlags nMirrorFlags(BmpMirrorFlags::NONE);
    if( aSize.Width() < 0 )
    {
        aSize.setWidth( aSize.Width() * -1 );
        aPoint.AdjustX( -(aSize.Width()) );
        nMirrorFlags |= BmpMirrorFlags::Horizontal;
    }
    if( aSize.Height() < 0 )
    {
        aSize.setHeight( aSize.Height() * -1 );
        aPoint.AdjustY( -(aSize.Height()) );
        nMirrorFlags |= BmpMirrorFlags::Vertical;
    }

    if( nMirrorFlags != BmpMirrorFlags::NONE )
    {
        aBitmap.Mirror( nMirrorFlags );
    }

    bool bIsJpeg = false, bIsPng = false;
    if( i_Graphic.GetType() != GraphicType::NONE && i_Graphic.GetBitmap() == aBitmap )
    {
        GfxLinkType eType = i_Graphic.GetGfxLink().GetType();
        bIsJpeg = (eType == GfxLinkType::NativeJpg);
        bIsPng = (eType == GfxLinkType::NativePng);
    }

    // Do not downsample images smaller than 50x50px.
    const Size aBmpSize(aBitmap.GetSizePixel());
    if (i_rContext.m_nMaxImageResolution > 50 && aBmpSize.getWidth() > 50
        && aBmpSize.getHeight() > 50)
    {
        // do downsampling if necessary
        const Size      aDstSizeTwip( i_pDummyVDev->PixelToLogic(i_pDummyVDev->LogicToPixel(aSize), MapMode(MapUnit::MapTwip)) );
        const double    fBmpPixelX = aBmpSize.Width();
        const double    fBmpPixelY = aBmpSize.Height();
        const double fMaxPixelX
            = o3tl::convert<double>(aDstSizeTwip.Width(), o3tl::Length::twip, o3tl::Length::in)
              * i_rContext.m_nMaxImageResolution;
        const double fMaxPixelY
            = o3tl::convert<double>(aDstSizeTwip.Height(), o3tl::Length::twip, o3tl::Length::in)
              * i_rContext.m_nMaxImageResolution;

        // check, if the bitmap DPI exceeds the maximum DPI (allow 4 pixel rounding tolerance)
        if( ( ( fBmpPixelX > ( fMaxPixelX + 4 ) ) ||
            ( fBmpPixelY > ( fMaxPixelY + 4 ) ) ) &&
            ( fBmpPixelY > 0.0 ) && ( fMaxPixelY > 0.0 ) )
        {
            // do scaling
            Size            aNewBmpSize;
            const double    fBmpWH = fBmpPixelX / fBmpPixelY;
            const double    fMaxWH = fMaxPixelX / fMaxPixelY;

            if( fBmpWH < fMaxWH )
            {
                aNewBmpSize.setWidth(basegfx::fround<tools::Long>(fMaxPixelY * fBmpWH));
                aNewBmpSize.setHeight(basegfx::fround<tools::Long>(fMaxPixelY));
            }
            else if( fBmpWH > 0.0 )
            {
                aNewBmpSize.setWidth(basegfx::fround<tools::Long>(fMaxPixelX));
                aNewBmpSize.setHeight(basegfx::fround<tools::Long>(fMaxPixelX / fBmpWH));
            }

            if( aNewBmpSize.Width() && aNewBmpSize.Height() )
            {
                // #i121233# Use best quality for PDF exports
                aBitmap.Scale( aNewBmpSize, BmpScaleFlag::BestQuality );
            }
            else
            {
                aBitmap.SetEmpty();
            }
        }
    }

    const Size aSizePixel( aBitmap.GetSizePixel() );
    if ( !(aSizePixel.Width() && aSizePixel.Height()) )
        return;

    if( m_aContext.ColorMode == PDFWriter::DrawGreyscale )
        aBitmap.Convert(BmpConversion::N8BitGreys);
    bool bUseJPGCompression = !i_rContext.m_bOnlyLosslessCompression;
    if ( bIsPng || ( aSizePixel.Width() < 32 ) || ( aSizePixel.Height() < 32 ) )
        bUseJPGCompression = false;

    auto   pStrm=std::make_shared<SvMemoryStream>();
    AlphaMask aAlphaMask;

    bool bTrueColorJPG = true;
    if ( bUseJPGCompression )
    {
        // TODO this checks could be done much earlier, saving us
        // from trying conversion & stores before...
        if ( !aBitmap.HasAlpha() )
        {
            const auto aCacheEntry=m_aPDFBmpCache.find(
                aBitmap.GetChecksum());
            if ( aCacheEntry != m_aPDFBmpCache.end() )
            {
                m_rOuterFace.DrawJPGBitmap( *aCacheEntry->second, true, aSizePixel,
                                            tools::Rectangle( aPoint, aSize ), aAlphaMask, i_Graphic );
                return;
            }
        }
        sal_uInt32 nZippedFileSize = 0; // sj: we will calculate the filesize of a zipped bitmap
        if ( !bIsJpeg )                 // to determine if jpeg compression is useful
        {
            SvMemoryStream aTemp;
            aTemp.SetCompressMode( aTemp.GetCompressMode() | SvStreamCompressFlags::ZBITMAP );
            aTemp.SetVersion( SOFFICE_FILEFORMAT_40 );  // sj: up from version 40 our bitmap stream operator
            WriteDIBBitmapEx(aBitmap, aTemp); // is capable of zlib stream compression
            nZippedFileSize = aTemp.TellEnd();
        }
        if ( aBitmap.HasAlpha() )
            aAlphaMask = aBitmap.CreateAlphaMask();
        Graphic aGraphic(aBitmap.CreateColorBitmap());

        Sequence< PropertyValue > aFilterData{
            comphelper::makePropertyValue(u"Quality"_ustr, sal_Int32(i_rContext.m_nJPEGQuality)),
            comphelper::makePropertyValue(u"ColorMode"_ustr, sal_Int32(0))
        };

        try
        {
            uno::Reference < io::XStream > xStream = new utl::OStreamWrapper( *pStrm );
            uno::Reference< io::XSeekable > xSeekable( xStream, UNO_QUERY_THROW );
            const uno::Reference< uno::XComponentContext >& xContext( comphelper::getProcessComponentContext() );
            uno::Reference< graphic::XGraphicProvider > xGraphicProvider( graphic::GraphicProvider::create(xContext) );
            uno::Reference< graphic::XGraphic > xGraphic( aGraphic.GetXGraphic() );
            uno::Reference < io::XOutputStream > xOut( xStream->getOutputStream() );
            uno::Sequence< beans::PropertyValue > aOutMediaProperties{
                comphelper::makePropertyValue(u"OutputStream"_ustr, xOut),
                comphelper::makePropertyValue(u"MimeType"_ustr, u"image/jpeg"_ustr),
                comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData)
            };
            xGraphicProvider->storeGraphic( xGraphic, aOutMediaProperties );
            xOut->flush();
            if ( !bIsJpeg && xSeekable->getLength() > nZippedFileSize )
            {
                bUseJPGCompression = false;
            }
            else
            {
                pStrm->Seek( STREAM_SEEK_TO_END );

                xSeekable->seek( 0 );
                Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(u"InputStream"_ustr,
                                                                               xStream) };
                uno::Reference< XPropertySet > xPropSet( xGraphicProvider->queryGraphicDescriptor( aArgs ) );
                if ( xPropSet.is() )
                {
                    sal_Int16 nBitsPerPixel = 24;
                    if ( xPropSet->getPropertyValue(u"BitsPerPixel"_ustr) >>= nBitsPerPixel )
                    {
                        bTrueColorJPG = nBitsPerPixel != 8;
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
            bUseJPGCompression = false;
        }
    }
    if ( bUseJPGCompression )
    {
        m_rOuterFace.DrawJPGBitmap( *pStrm, bTrueColorJPG, aSizePixel, tools::Rectangle( aPoint, aSize ), aAlphaMask, i_Graphic );
        if (!aBitmap.HasAlpha() && bTrueColorJPG)
        {
            // Cache last jpeg export
            m_aPDFBmpCache.insert(
                {aBitmap.GetChecksum(), pStrm});
        }
    }
    else if ( aBitmap.HasAlpha() )
        m_rOuterFace.DrawBitmap( aPoint, aSize, aBitmap );
    else
        m_rOuterFace.DrawBitmap( aPoint, aSize, aBitmap, i_Graphic );

}

void PDFWriterImpl::playMetafile( const GDIMetaFile& i_rMtf, vcl::PDFExtOutDevData* i_pOutDevData, const vcl::PDFWriter::PlayMetafileContext& i_rContext, VirtualDevice* pDummyVDev )
{
    bool bAssertionFired( false );

    ScopedVclPtr<VirtualDevice> xPrivateDevice;
    if( ! pDummyVDev )
    {
        xPrivateDevice.disposeAndReset(VclPtr<VirtualDevice>::Create());
        pDummyVDev = xPrivateDevice.get();
        pDummyVDev->EnableOutput( false );
        pDummyVDev->SetMapMode( i_rMtf.GetPrefMapMode() );
    }
    const GDIMetaFile& aMtf( i_rMtf );

    for( sal_uInt32 i = 0, nCount = aMtf.GetActionSize(); i < nCount; )
    {
        if ( !i_pOutDevData || !i_pOutDevData->PlaySyncPageAct( m_rOuterFace, i, aMtf ) )
        {
            const MetaAction*    pAction = aMtf.GetAction( i );
            const MetaActionType nType = pAction->GetType();

            switch( nType )
            {
                case MetaActionType::PIXEL:
                {
                    const MetaPixelAction* pA = static_cast<const MetaPixelAction*>(pAction);
                    m_rOuterFace.DrawPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case MetaActionType::POINT:
                {
                    const MetaPointAction* pA = static_cast<const MetaPointAction*>(pAction);
                    m_rOuterFace.DrawPixel( pA->GetPoint() );
                }
                break;

                case MetaActionType::LINE:
                {
                    const MetaLineAction* pA = static_cast<const MetaLineAction*>(pAction);
                    if ( pA->GetLineInfo().IsDefault() )
                        m_rOuterFace.DrawLine( pA->GetStartPoint(), pA->GetEndPoint() );
                    else
                        m_rOuterFace.DrawLine( pA->GetStartPoint(), pA->GetEndPoint(), pA->GetLineInfo() );
                }
                break;

                case MetaActionType::RECT:
                {
                    const MetaRectAction* pA = static_cast<const MetaRectAction*>(pAction);
                    m_rOuterFace.DrawRect( pA->GetRect() );
                }
                break;

                case MetaActionType::ROUNDRECT:
                {
                    const MetaRoundRectAction* pA = static_cast<const MetaRoundRectAction*>(pAction);
                    m_rOuterFace.DrawRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case MetaActionType::ELLIPSE:
                {
                    const MetaEllipseAction* pA = static_cast<const MetaEllipseAction*>(pAction);
                    m_rOuterFace.DrawEllipse( pA->GetRect() );
                }
                break;

                case MetaActionType::ARC:
                {
                    const MetaArcAction* pA = static_cast<const MetaArcAction*>(pAction);
                    m_rOuterFace.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case MetaActionType::PIE:
                {
                    const MetaArcAction* pA = static_cast<const MetaArcAction*>(pAction);
                    m_rOuterFace.DrawPie( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case MetaActionType::CHORD:
                {
                    const MetaChordAction* pA = static_cast<const MetaChordAction*>(pAction);
                    m_rOuterFace.DrawChord( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case MetaActionType::POLYGON:
                {
                    const MetaPolygonAction* pA = static_cast<const MetaPolygonAction*>(pAction);
                    m_rOuterFace.DrawPolygon( pA->GetPolygon() );
                }
                break;

                case MetaActionType::POLYLINE:
                {
                    const MetaPolyLineAction* pA = static_cast<const MetaPolyLineAction*>(pAction);
                    if ( pA->GetLineInfo().IsDefault() )
                        m_rOuterFace.DrawPolyLine( pA->GetPolygon() );
                    else
                        m_rOuterFace.DrawPolyLine( pA->GetPolygon(), pA->GetLineInfo() );
                }
                break;

                case MetaActionType::POLYPOLYGON:
                {
                    const MetaPolyPolygonAction* pA = static_cast<const MetaPolyPolygonAction*>(pAction);
                    m_rOuterFace.DrawPolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case MetaActionType::GRADIENT:
                {
                    const MetaGradientAction* pA = static_cast<const MetaGradientAction*>(pAction);
                    const Gradient& rGradient = pA->GetGradient();
                    if (lcl_canUsePDFAxialShading(rGradient))
                    {
                        m_rOuterFace.DrawGradient( pA->GetRect(), rGradient );
                    }
                    else
                    {
                        const tools::PolyPolygon aPolyPoly( pA->GetRect() );
                        implWriteGradient( aPolyPoly, rGradient, pDummyVDev, i_rContext );
                    }
                }
                break;

                case MetaActionType::GRADIENTEX:
                {
                    const MetaGradientExAction* pA = static_cast<const MetaGradientExAction*>(pAction);
                    const Gradient& rGradient = pA->GetGradient();

                    if (lcl_canUsePDFAxialShading(rGradient))
                        m_rOuterFace.DrawGradient( pA->GetPolyPolygon(), rGradient );
                    else
                        implWriteGradient( pA->GetPolyPolygon(), rGradient, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::HATCH:
                {
                    const MetaHatchAction*  pA = static_cast<const MetaHatchAction*>(pAction);
                    m_rOuterFace.DrawHatch( pA->GetPolyPolygon(), pA->GetHatch() );
                }
                break;

                case MetaActionType::Transparent:
                {
                    const MetaTransparentAction* pA = static_cast<const MetaTransparentAction*>(pAction);
                    m_rOuterFace.DrawTransparent( pA->GetPolyPolygon(), pA->GetTransparence() );
                }
                break;

                case MetaActionType::FLOATTRANSPARENT:
                {
                    const MetaFloatTransparentAction* pA = static_cast<const MetaFloatTransparentAction*>(pAction);

                    GDIMetaFile     aTmpMtf( pA->GetGDIMetaFile() );
                    const Point&    rPos = pA->GetPoint();
                    const Size&     rSize= pA->GetSize();
                    const Gradient& rTransparenceGradient = pA->GetGradient();

                    // special case constant alpha value
                    if( rTransparenceGradient.GetStartColor() == rTransparenceGradient.GetEndColor() )
                    {
                        const Color aTransCol( rTransparenceGradient.GetStartColor() );
                        const sal_uInt16 nTransPercent = aTransCol.GetLuminance() * 100 / 255;
                        m_rOuterFace.BeginTransparencyGroup();

                        // tdf#138826 adjust the aTmpMtf to start at rPos (see also #i112076#)
                        Point aMtfOrigin(aTmpMtf.GetPrefMapMode().GetOrigin());
                        if (rPos != aMtfOrigin)
                            aTmpMtf.Move(rPos.X() - aMtfOrigin.X(), rPos.Y() - aMtfOrigin.Y());

                        playMetafile( aTmpMtf, nullptr, i_rContext, pDummyVDev );
                        m_rOuterFace.EndTransparencyGroup( tools::Rectangle( rPos, rSize ), nTransPercent );
                    }
                    else
                    {
                        const Size aDstSizeTwip( pDummyVDev->PixelToLogic(pDummyVDev->LogicToPixel(rSize), MapMode(MapUnit::MapTwip)) );

                        // i#115962# Always use at least 300 DPI for bitmap conversion of transparence gradients,
                        // else the quality is not acceptable (see bugdoc as example)
                        sal_Int32 nMaxBmpDPI(300);

                        if( i_rContext.m_nMaxImageResolution > 50 )
                        {
                            if ( nMaxBmpDPI > i_rContext.m_nMaxImageResolution )
                                nMaxBmpDPI = i_rContext.m_nMaxImageResolution;
                        }
                        const sal_Int32 nPixelX = o3tl::convert<double>(aDstSizeTwip.Width(), o3tl::Length::twip, o3tl::Length::in) * nMaxBmpDPI;
                        const sal_Int32 nPixelY = o3tl::convert<double>(aDstSizeTwip.Height(), o3tl::Length::twip, o3tl::Length::in) * nMaxBmpDPI;
                        if ( nPixelX && nPixelY )
                        {
                            Size aDstSizePixel( nPixelX, nPixelY );
                            ScopedVclPtrInstance<VirtualDevice> xVDev(DeviceFormat::WITH_ALPHA);
                            if( xVDev->SetOutputSizePixel( aDstSizePixel, true, true ) )
                            {
                                Point           aPoint;

                                MapMode aMapMode( pDummyVDev->GetMapMode() );
                                aMapMode.SetOrigin( aPoint );
                                xVDev->SetMapMode( aMapMode );
                                const bool bVDevOldMap = xVDev->IsMapModeEnabled();
                                Size aDstSize( xVDev->PixelToLogic( aDstSizePixel ) );

                                Point   aMtfOrigin( aTmpMtf.GetPrefMapMode().GetOrigin() );
                                if ( aMtfOrigin.X() || aMtfOrigin.Y() )
                                    aTmpMtf.Move( -aMtfOrigin.X(), -aMtfOrigin.Y() );
                                double  fScaleX = static_cast<double>(aDstSize.Width()) / static_cast<double>(aTmpMtf.GetPrefSize().Width());
                                double  fScaleY = static_cast<double>(aDstSize.Height()) / static_cast<double>(aTmpMtf.GetPrefSize().Height());
                                if( fScaleX != 1.0 || fScaleY != 1.0 )
                                    aTmpMtf.Scale( fScaleX, fScaleY );
                                aTmpMtf.SetPrefMapMode( aMapMode );

                                // create paint bitmap
                                aTmpMtf.WindStart();
                                aTmpMtf.Play(*xVDev, aPoint, aDstSize);
                                aTmpMtf.WindStart();
                                xVDev->EnableMapMode( false );
                                Bitmap aPaint(xVDev->GetBitmap(aPoint, xVDev->GetOutputSizePixel()));
                                xVDev->EnableMapMode( bVDevOldMap ); // #i35331#: MUST NOT use EnableMapMode( sal_True ) here!

                                // create alpha mask from gradient
                                xVDev->SetDrawMode( DrawModeFlags::GrayGradient );
                                xVDev->DrawGradient( tools::Rectangle( aPoint, aDstSize ), rTransparenceGradient );
                                xVDev->SetDrawMode( DrawModeFlags::Default );
                                xVDev->EnableMapMode( false );

                                AlphaMask aAlpha(xVDev->GetBitmap(Point(), xVDev->GetOutputSizePixel()));
#if HAVE_FEATURE_SKIA
#if OSL_DEBUG_LEVEL > 0
                                // In release builds, we always invert
                                // regardless of whether Skia is enabled or not.
                                // But in debug builds, we can't invert when
                                // Skia is enabled.
                                if ( !SkiaHelper::isVCLSkiaEnabled() )
#endif
#endif
                                {
                                    // When Skia is disabled, the alpha mask
                                    // must be inverted a second time. To test
                                    // this code, export the following
                                    // document to PDF:
                                    //   https://bugs.documentfoundation.org/attachment.cgi?id=188084
                                    aAlpha.Invert(); // convert to alpha
                                }
                                aAlpha.BlendWith(aPaint.CreateAlphaMask());

                                xVDev.disposeAndClear();

                                Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                                implWriteBitmapEx( rPos, rSize, Bitmap(aPaint.CreateColorBitmap(), aAlpha ), aGraphic, pDummyVDev, i_rContext );
                            }
                        }
                    }
                }
                break;

                case MetaActionType::EPS:
                {
                    const MetaEPSAction*    pA = static_cast<const MetaEPSAction*>(pAction);
                    const GDIMetaFile&      aSubstitute( pA->GetSubstitute() );

                    m_rOuterFace.Push();
                    pDummyVDev->Push();

                    MapMode aMapMode( aSubstitute.GetPrefMapMode() );
                    Size aOutSize( OutputDevice::LogicToLogic( pA->GetSize(), pDummyVDev->GetMapMode(), aMapMode ) );
                    aMapMode.SetScaleX( Fraction( aOutSize.Width(), aSubstitute.GetPrefSize().Width() ) );
                    aMapMode.SetScaleY( Fraction( aOutSize.Height(), aSubstitute.GetPrefSize().Height() ) );
                    aMapMode.SetOrigin( OutputDevice::LogicToLogic( pA->GetPoint(), pDummyVDev->GetMapMode(), aMapMode ) );

                    m_rOuterFace.SetMapMode( aMapMode );
                    pDummyVDev->SetMapMode( aMapMode );
                    playMetafile( aSubstitute, nullptr, i_rContext, pDummyVDev );
                    pDummyVDev->Pop();
                    m_rOuterFace.Pop();
                }
                break;

                case MetaActionType::COMMENT:
                if( ! i_rContext.m_bTransparenciesWereRemoved )
                {
                    const MetaCommentAction*    pA = static_cast<const MetaCommentAction*>(pAction);

                    if( pA->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
                    {
                        const MetaGradientExAction* pGradAction = nullptr;
                        bool                        bDone = false;

                        while( !bDone && ( ++i < nCount ) )
                        {
                            pAction = aMtf.GetAction( i );

                            if( pAction->GetType() == MetaActionType::GRADIENTEX )
                                pGradAction = static_cast<const MetaGradientExAction*>(pAction);
                            else if( ( pAction->GetType() == MetaActionType::COMMENT ) &&
                                     ( static_cast<const MetaCommentAction*>(pAction)->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END")) )
                            {
                                bDone = true;
                            }
                        }

                        if( pGradAction )
                        {
                            if (lcl_canUsePDFAxialShading(pGradAction->GetGradient()))
                            {
                                m_rOuterFace.DrawGradient( pGradAction->GetPolyPolygon(), pGradAction->GetGradient() );
                            }
                            else
                            {
                                implWriteGradient( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), pDummyVDev, i_rContext );
                            }
                        }
                    }
                    else
                    {
                        const sal_uInt8* pData = pA->GetData();
                        if ( pData )
                        {
                            SvMemoryStream  aMemStm( const_cast<sal_uInt8 *>(pData), pA->GetDataSize(), StreamMode::READ );
                            bool            bSkipSequence = false;
                            OString sSeqEnd;

                            if( pA->GetComment() == "XPATHSTROKE_SEQ_BEGIN" )
                            {
                                sSeqEnd = "XPATHSTROKE_SEQ_END"_ostr;
                                SvtGraphicStroke aStroke;
                                ReadSvtGraphicStroke( aMemStm, aStroke );

                                tools::Polygon aPath;
                                aStroke.getPath( aPath );

                                tools::PolyPolygon aStartArrow;
                                tools::PolyPolygon aEndArrow;
                                double fTransparency( aStroke.getTransparency() );
                                double fStrokeWidth( aStroke.getStrokeWidth() );
                                SvtGraphicStroke::DashArray aDashArray;

                                aStroke.getStartArrow( aStartArrow );
                                aStroke.getEndArrow( aEndArrow );
                                aStroke.getDashArray( aDashArray );

                                bSkipSequence = true;
                                if ( aStartArrow.Count() || aEndArrow.Count() )
                                    bSkipSequence = false;
                                if ( !aDashArray.empty() && ( fStrokeWidth != 0.0 ) && ( fTransparency == 0.0 ) )
                                    bSkipSequence = false;
                                if ( bSkipSequence )
                                {
                                    PDFWriter::ExtLineInfo aInfo;
                                    aInfo.m_fLineWidth      = fStrokeWidth;
                                    aInfo.m_fTransparency   = fTransparency;
                                    aInfo.m_fMiterLimit     = aStroke.getMiterLimit();
                                    switch( aStroke.getCapType() )
                                    {
                                        default:
                                        case SvtGraphicStroke::capButt:   aInfo.m_eCap = PDFWriter::capButt;break;
                                        case SvtGraphicStroke::capRound:  aInfo.m_eCap = PDFWriter::capRound;break;
                                        case SvtGraphicStroke::capSquare: aInfo.m_eCap = PDFWriter::capSquare;break;
                                    }
                                    switch( aStroke.getJoinType() )
                                    {
                                        default:
                                        case SvtGraphicStroke::joinMiter: aInfo.m_eJoin = PDFWriter::joinMiter;break;
                                        case SvtGraphicStroke::joinRound: aInfo.m_eJoin = PDFWriter::joinRound;break;
                                        case SvtGraphicStroke::joinBevel: aInfo.m_eJoin = PDFWriter::joinBevel;break;
                                        case SvtGraphicStroke::joinNone:
                                            aInfo.m_eJoin = PDFWriter::joinMiter;
                                            aInfo.m_fMiterLimit = 0.0;
                                            break;
                                    }
                                    aInfo.m_aDashArray = std::move(aDashArray);

                                    if(SvtGraphicStroke::joinNone == aStroke.getJoinType()
                                        && fStrokeWidth > 0.0)
                                    {
                                        // emulate no edge rounding by handling single edges
                                        const sal_uInt16 nPoints(aPath.GetSize());
                                        const bool bCurve(aPath.HasFlags());

                                        for(sal_uInt16 a(0); a + 1 < nPoints; a++)
                                        {
                                            if(bCurve
                                                && PolyFlags::Normal != aPath.GetFlags(a + 1)
                                                && a + 2 < nPoints
                                                && PolyFlags::Normal != aPath.GetFlags(a + 2)
                                                && a + 3 < nPoints)
                                            {
                                                const tools::Polygon aSnippet(4,
                                                    aPath.GetConstPointAry() + a,
                                                    aPath.GetConstFlagAry() + a);
                                                m_rOuterFace.DrawPolyLine( aSnippet, aInfo );
                                                a += 2;
                                            }
                                            else
                                            {
                                                const tools::Polygon aSnippet(2,
                                                    aPath.GetConstPointAry() + a);
                                                m_rOuterFace.DrawPolyLine( aSnippet, aInfo );
                                            }
                                        }
                                    }
                                    else
                                    {
                                        m_rOuterFace.DrawPolyLine( aPath, aInfo );
                                    }
                                }
                            }
                            else if ( pA->GetComment() == "XPATHFILL_SEQ_BEGIN" )
                            {
                                sSeqEnd = "XPATHFILL_SEQ_END"_ostr;
                                SvtGraphicFill aFill;
                                ReadSvtGraphicFill( aMemStm, aFill );

                                if ( ( aFill.getFillType() == SvtGraphicFill::fillSolid ) && ( aFill.getFillRule() == SvtGraphicFill::fillEvenOdd ) )
                                {
                                    double fTransparency = aFill.getTransparency();
                                    if ( fTransparency == 0.0 )
                                    {
                                        tools::PolyPolygon aPath;
                                        aFill.getPath( aPath );

                                        bSkipSequence = true;
                                        m_rOuterFace.DrawPolyPolygon( aPath );
                                    }
                                    else if ( fTransparency == 1.0 )
                                        bSkipSequence = true;
                                }
                            }
                            if ( bSkipSequence )
                            {
                                while( ++i < nCount )
                                {
                                    pAction = aMtf.GetAction( i );
                                    if ( pAction->GetType() == MetaActionType::COMMENT )
                                    {
                                        OString sComment( static_cast<const MetaCommentAction*>(pAction)->GetComment() );
                                        if (sComment == sSeqEnd)
                                            break;
                                    }
                                    // #i44496#
                                    // the replacement action for stroke is a filled rectangle
                                    // the set fillcolor of the replacement is part of the graphics
                                    // state and must not be skipped
                                    else if( pAction->GetType() == MetaActionType::FILLCOLOR )
                                    {
                                        const MetaFillColorAction* pMA = static_cast<const MetaFillColorAction*>(pAction);
                                        if( pMA->IsSetting() )
                                            m_rOuterFace.SetFillColor( pMA->GetColor() );
                                        else
                                            m_rOuterFace.SetFillColor();
                                    }
                                }
                            }
                        }
                    }
                }
                break;

                case MetaActionType::BMP:
                {
                    const MetaBmpAction* pA = static_cast<const MetaBmpAction*>(pAction);
                    Bitmap aBitmap( pA->GetBitmap() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmap.GetPrefSize(),
                                                            aBitmap.GetPrefMapMode(), pDummyVDev->GetMapMode() ) );
                    if( ! ( aSize.Width() && aSize.Height() ) )
                        aSize = pDummyVDev->PixelToLogic( aBitmap.GetSizePixel() );

                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), aSize, aBitmap, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPSCALE:
                {
                    const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), pA->GetSize(), pA->GetBitmap(), aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPSCALEPART:
                {
                    const MetaBmpScalePartAction* pA = static_cast<const MetaBmpScalePartAction*>(pAction);
                    Bitmap aBitmap( pA->GetBitmap() );
                    aBitmap.Crop( tools::Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetDestPoint(), pA->GetDestSize(), aBitmap, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPEX:
                {
                    const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction*>(pAction);
                    Bitmap aBitmap( pA->GetBitmap() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmap.GetPrefSize(),
                            aBitmap.GetPrefMapMode(), pDummyVDev->GetMapMode() ) );
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), aSize, aBitmap, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPEXSCALE:
                {
                    const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);
                    Bitmap aBitmap( pA->GetBitmap() );
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), pA->GetSize(), aBitmap, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPEXSCALEPART:
                {
                    const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pAction);

                    Bitmap aBitmapEx( pA->GetBitmap() );
                    aBitmapEx.Crop( tools::Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::MASK:
                case MetaActionType::MASKSCALE:
                case MetaActionType::MASKSCALEPART:
                {
                    SAL_WARN( "vcl", "MetaMask...Action not supported yet" );
                }
                break;

                case MetaActionType::TEXT:
                {
                    const MetaTextAction* pA = static_cast<const MetaTextAction*>(pAction);
                    m_rOuterFace.DrawText( pA->GetPoint(), pA->GetText().copy( pA->GetIndex(), std::min<sal_Int32>(pA->GetText().getLength() - pA->GetIndex(), pA->GetLen()) ) );
                }
                break;

                case MetaActionType::TEXTRECT:
                {
                    const MetaTextRectAction* pA = static_cast<const MetaTextRectAction*>(pAction);
                    m_rOuterFace.DrawText( pA->GetRect(), pA->GetText(), pA->GetStyle() );
                }
                break;

                case MetaActionType::TEXTARRAY:
                {
                    const MetaTextArrayAction* pA = static_cast<const MetaTextArrayAction*>(pAction);
                    m_rOuterFace.DrawTextArray(pA->GetPoint(), pA->GetText(), pA->GetDXArray(),
                                               pA->GetKashidaArray(), pA->GetIndex(), pA->GetLen(),
                                               pA->GetLayoutContextIndex(),
                                               pA->GetLayoutContextLen());
                }
                break;

                case MetaActionType::STRETCHTEXT:
                {
                    const MetaStretchTextAction* pA = static_cast<const MetaStretchTextAction*>(pAction);
                    m_rOuterFace.DrawStretchText( pA->GetPoint(), pA->GetWidth(), pA->GetText(), pA->GetIndex(), pA->GetLen() );
                }
                break;

                case MetaActionType::TEXTLINE:
                {
                    const MetaTextLineAction* pA = static_cast<const MetaTextLineAction*>(pAction);
                    m_rOuterFace.DrawTextLine( pA->GetStartPoint(), pA->GetWidth(), pA->GetStrikeout(), pA->GetUnderline(), pA->GetOverline() );

                }
                break;

                case MetaActionType::CLIPREGION:
                {
                    const MetaClipRegionAction* pA = static_cast<const MetaClipRegionAction*>(pAction);

                    if( pA->IsClipping() )
                    {
                        if( pA->GetRegion().IsEmpty() )
                            m_rOuterFace.SetClipRegion( basegfx::B2DPolyPolygon() );
                        else
                        {
                            const vcl::Region& aReg( pA->GetRegion() );
                            m_rOuterFace.SetClipRegion( aReg.GetAsB2DPolyPolygon() );
                        }
                    }
                    else
                        m_rOuterFace.SetClipRegion();
                }
                break;

                case MetaActionType::ISECTRECTCLIPREGION:
                {
                    const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pAction);
                    m_rOuterFace.IntersectClipRegion( pA->GetRect() );
                }
                break;

                case MetaActionType::ISECTREGIONCLIPREGION:
                {
                    const MetaISectRegionClipRegionAction* pA = static_cast<const MetaISectRegionClipRegionAction*>(pAction);
                    const vcl::Region& aReg( pA->GetRegion() );
                    m_rOuterFace.IntersectClipRegion( aReg.GetAsB2DPolyPolygon() );
                }
                break;

                case MetaActionType::MOVECLIPREGION:
                {
                    const MetaMoveClipRegionAction* pA = static_cast<const MetaMoveClipRegionAction*>(pAction);
                    m_rOuterFace.MoveClipRegion( pA->GetHorzMove(), pA->GetVertMove() );
                }
                break;

                case MetaActionType::MAPMODE:
                {
                    const_cast< MetaAction* >( pAction )->Execute( pDummyVDev );
                    m_rOuterFace.SetMapMode( pDummyVDev->GetMapMode() );
                }
                break;

                case MetaActionType::LINECOLOR:
                {
                    const MetaLineColorAction* pA = static_cast<const MetaLineColorAction*>(pAction);

                    if( pA->IsSetting() )
                        m_rOuterFace.SetLineColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetLineColor();
                }
                break;

                case MetaActionType::FILLCOLOR:
                {
                    const MetaFillColorAction* pA = static_cast<const MetaFillColorAction*>(pAction);

                    if( pA->IsSetting() )
                        m_rOuterFace.SetFillColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetFillColor();
                }
                break;

                case MetaActionType::TEXTLINECOLOR:
                {
                    const MetaTextLineColorAction* pA = static_cast<const MetaTextLineColorAction*>(pAction);

                    if( pA->IsSetting() )
                        m_rOuterFace.SetTextLineColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetTextLineColor();
                }
                break;

                case MetaActionType::OVERLINECOLOR:
                {
                    const MetaOverlineColorAction* pA = static_cast<const MetaOverlineColorAction*>(pAction);

                    if( pA->IsSetting() )
                        m_rOuterFace.SetOverlineColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetOverlineColor();
                }
                break;

                case MetaActionType::TEXTFILLCOLOR:
                {
                    const MetaTextFillColorAction* pA = static_cast<const MetaTextFillColorAction*>(pAction);

                    if( pA->IsSetting() )
                        m_rOuterFace.SetTextFillColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetTextFillColor();
                }
                break;

                case MetaActionType::TEXTCOLOR:
                {
                    const MetaTextColorAction* pA = static_cast<const MetaTextColorAction*>(pAction);
                    m_rOuterFace.SetTextColor( pA->GetColor() );
                }
                break;

                case MetaActionType::TEXTALIGN:
                {
                    const MetaTextAlignAction* pA = static_cast<const MetaTextAlignAction*>(pAction);
                    m_rOuterFace.SetTextAlign( pA->GetTextAlign() );
                }
                break;

                case MetaActionType::FONT:
                {
                    const MetaFontAction* pA = static_cast<const MetaFontAction*>(pAction);
                    m_rOuterFace.SetFont( pA->GetFont() );
                }
                break;

                case MetaActionType::PUSH:
                {
                    const MetaPushAction* pA = static_cast<const MetaPushAction*>(pAction);

                    pDummyVDev->Push( pA->GetFlags() );
                    m_rOuterFace.Push( pA->GetFlags() );
                }
                break;

                case MetaActionType::POP:
                {
                    pDummyVDev->Pop();
                    m_rOuterFace.Pop();
                }
                break;

                case MetaActionType::LAYOUTMODE:
                {
                    const MetaLayoutModeAction* pA = static_cast<const MetaLayoutModeAction*>(pAction);
                    m_rOuterFace.SetLayoutMode( pA->GetLayoutMode() );
                }
                break;

                case MetaActionType::TEXTLANGUAGE:
                {
                    const  MetaTextLanguageAction* pA = static_cast<const MetaTextLanguageAction*>(pAction);
                    m_rOuterFace.SetDigitLanguage( pA->GetTextLanguage() );
                }
                break;

                case MetaActionType::WALLPAPER:
                {
                    const MetaWallpaperAction* pA = static_cast<const MetaWallpaperAction*>(pAction);
                    m_rOuterFace.DrawWallpaper( pA->GetRect(), pA->GetWallpaper() );
                }
                break;

                case MetaActionType::RASTEROP:
                case MetaActionType::REFPOINT:
                {
                    // !!! >>> we don't want to support this actions
                }
                break;

                default:
                    // #i24604# Made assertion fire only once per
                    // metafile. The asserted actions here are all
                    // deprecated
                    if( !bAssertionFired )
                    {
                        bAssertionFired = true;
                        SAL_WARN( "vcl", "PDFExport::ImplWriteActions: deprecated and unsupported MetaAction encountered " << static_cast<int>(nType) );
                    }
                break;
            }
            i++;
        }
    }
}

// Encryption methods

void PDFWriterImpl::checkAndEnableStreamEncryption(sal_Int32 nObject)
{
    if (!m_aContext.Encryption.canEncrypt() || !m_pPDFEncryptor)
        return;

    m_pPDFEncryptor->enableStreamEncryption();
    m_pPDFEncryptor->setupEncryption(m_aContext.Encryption.EncryptionKey, nObject);
}

void PDFWriterImpl::disableStreamEncryption()
{
    if (m_pPDFEncryptor)
        m_pPDFEncryptor->disableStreamEncryption();
}

void PDFWriterImpl::enableStringEncryption(sal_Int32 nObject)
{
    if (!m_aContext.Encryption.canEncrypt() || !m_pPDFEncryptor)
        return;

    m_pPDFEncryptor->setupEncryption(m_aContext.Encryption.EncryptionKey, nObject);
}



void PDFWriterImpl::DrawHatchLine_DrawLine(const Point& rStartPoint, const Point& rEndPoint)
{
    drawLine(rStartPoint, rEndPoint);
}

static bool lcl_canUsePDFAxialShading(const Gradient& rGradient) {
    switch (rGradient.GetStyle())
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            break;
        default:
            return false;
    }

    // TODO: handle step count
    return rGradient.GetSteps() <= 0;
}

void PDFWriterImpl::ImplClearFontData(bool bNewFontLists)
{
    VirtualDevice::ImplClearFontData(bNewFontLists);
    if (bNewFontLists && AcquireGraphics())
    {
        ReleaseFontCollection();
        ReleaseFontCache();
    }
}

void PDFWriterImpl::ImplRefreshFontData(bool bNewFontLists)
{
    if (bNewFontLists && AcquireGraphics())
    {
        SetFontCollectionFromSVData();
        ResetNewFontCache();
    }
}

vcl::Region PDFWriterImpl::ClipToDeviceBounds(vcl::Region aRegion) const
{
    return aRegion;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
