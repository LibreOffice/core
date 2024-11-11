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

void PDFWriterImpl::implWriteBitmapEx( const Point& i_rPoint, const Size& i_rSize, const BitmapEx& i_rBitmapEx, const Graphic& i_Graphic,
                                       VirtualDevice const * i_pDummyVDev, const vcl::PDFWriter::PlayMetafileContext& i_rContext )
{
    if ( i_rBitmapEx.IsEmpty() || !i_rSize.Width() || !i_rSize.Height() )
        return;

    BitmapEx        aBitmapEx( i_rBitmapEx );
    Point           aPoint( i_rPoint );
    Size            aSize( i_rSize );

    // #i19065# Negative sizes have mirror semantics on
    // OutputDevice. BitmapEx and co. have no idea about that, so
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
        aBitmapEx.Mirror( nMirrorFlags );
    }

    bool bIsJpeg = false, bIsPng = false;
    if( i_Graphic.GetType() != GraphicType::NONE && i_Graphic.GetBitmapEx() == aBitmapEx )
    {
        GfxLinkType eType = i_Graphic.GetGfxLink().GetType();
        bIsJpeg = (eType == GfxLinkType::NativeJpg);
        bIsPng = (eType == GfxLinkType::NativePng);
    }

    // Do not downsample images smaller than 50x50px.
    const Size aBmpSize(aBitmapEx.GetSizePixel());
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
                aBitmapEx.Scale( aNewBmpSize, BmpScaleFlag::BestQuality );
            }
            else
            {
                aBitmapEx.SetEmpty();
            }
        }
    }

    const Size aSizePixel( aBitmapEx.GetSizePixel() );
    if ( !(aSizePixel.Width() && aSizePixel.Height()) )
        return;

    if( m_aContext.ColorMode == PDFWriter::DrawGreyscale )
        aBitmapEx.Convert(BmpConversion::N8BitGreys);
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
        if ( !aBitmapEx.IsAlpha() )
        {
            const auto aCacheEntry=m_aPDFBmpCache.find(
                aBitmapEx.GetChecksum());
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
            WriteDIBBitmapEx(aBitmapEx, aTemp); // is capable of zlib stream compression
            nZippedFileSize = aTemp.TellEnd();
        }
        if ( aBitmapEx.IsAlpha() )
            aAlphaMask = aBitmapEx.GetAlphaMask();
        Graphic aGraphic(BitmapEx(aBitmapEx.GetBitmap()));

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
        if (!aBitmapEx.IsAlpha() && bTrueColorJPG)
        {
            // Cache last jpeg export
            m_aPDFBmpCache.insert(
                {aBitmapEx.GetChecksum(), pStrm});
        }
    }
    else if ( aBitmapEx.IsAlpha() )
        m_rOuterFace.DrawBitmapEx( aPoint, aSize, aBitmapEx );
    else
        m_rOuterFace.DrawBitmap( aPoint, aSize, aBitmapEx.GetBitmap(), i_Graphic );

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
                                BitmapEx aPaint = xVDev->GetBitmapEx(aPoint, xVDev->GetOutputSizePixel());
                                xVDev->EnableMapMode( bVDevOldMap ); // #i35331#: MUST NOT use EnableMapMode( sal_True ) here!

                                // create alpha mask from gradient
                                xVDev->SetDrawMode( DrawModeFlags::GrayGradient );
                                xVDev->DrawGradient( tools::Rectangle( aPoint, aDstSize ), rTransparenceGradient );
                                xVDev->SetDrawMode( DrawModeFlags::Default );
                                xVDev->EnableMapMode( false );

                                AlphaMask aAlpha(xVDev->GetBitmap(Point(), xVDev->GetOutputSizePixel()));
                                const AlphaMask& aPaintAlpha(aPaint.GetAlphaMask());
                                // The alpha mask is inverted from what is
                                // expected so invert it again. To test this
                                // code, export to PDF the transparent shapes,
                                // gradients, and images in the documents
                                // attached to the following bug reports:
                                //   https://bugs.documentfoundation.org/show_bug.cgi?id=155912
                                //   https://bugs.documentfoundation.org/show_bug.cgi?id=156630
                                aAlpha.Invert(); // convert to alpha
                                aAlpha.BlendWith(aPaintAlpha);
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

                                xVDev.disposeAndClear();

                                Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                                implWriteBitmapEx( rPos, rSize, BitmapEx( aPaint.GetBitmap(), aAlpha ), aGraphic, pDummyVDev, i_rContext );
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
                    BitmapEx aBitmapEx( pA->GetBitmap() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                                                            aBitmapEx.GetPrefMapMode(), pDummyVDev->GetMapMode() ) );
                    if( ! ( aSize.Width() && aSize.Height() ) )
                        aSize = pDummyVDev->PixelToLogic( aBitmapEx.GetSizePixel() );

                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), aSize, aBitmapEx, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPSCALE:
                {
                    const MetaBmpScaleAction* pA = static_cast<const MetaBmpScaleAction*>(pAction);
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), pA->GetSize(), BitmapEx( pA->GetBitmap() ), aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPSCALEPART:
                {
                    const MetaBmpScalePartAction* pA = static_cast<const MetaBmpScalePartAction*>(pAction);
                    BitmapEx aBitmapEx( pA->GetBitmap() );
                    aBitmapEx.Crop( tools::Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPEX:
                {
                    const MetaBmpExAction*  pA = static_cast<const MetaBmpExAction*>(pAction);

                    // The alpha mask is inverted from what is
                    // expected so invert it again. To test this
                    // code, export to PDF the transparent shapes,
                    // gradients, and images in the documents
                    // attached to the following bug reports:
                    //   https://bugs.documentfoundation.org/show_bug.cgi?id=155912
                    //   https://bugs.documentfoundation.org/show_bug.cgi?id=156630
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    if ( aBitmapEx.IsAlpha())
                    {
                        AlphaMask aAlpha = aBitmapEx.GetAlphaMask();
                        aAlpha.Invert();
                        aBitmapEx = BitmapEx(aBitmapEx.GetBitmap(), aAlpha);
                    }

                    Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                            aBitmapEx.GetPrefMapMode(), pDummyVDev->GetMapMode() ) );
                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), aSize, aBitmapEx, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPEXSCALE:
                {
                    const MetaBmpExScaleAction* pA = static_cast<const MetaBmpExScaleAction*>(pAction);

                    // The alpha mask is inverted from what is
                    // expected so invert it again. To test this
                    // code, export to PDF the transparent shapes,
                    // gradients, and images in the documents
                    // attached to the following bug reports:
                    //   https://bugs.documentfoundation.org/show_bug.cgi?id=155912
                    //   https://bugs.documentfoundation.org/show_bug.cgi?id=156630
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    if ( aBitmapEx.IsAlpha())
                    {
                        AlphaMask aAlpha = aBitmapEx.GetAlphaMask();
                        aAlpha.Invert();
                        aBitmapEx = BitmapEx(aBitmapEx.GetBitmap(), aAlpha);
                    }

                    Graphic aGraphic = i_pOutDevData ? i_pOutDevData->GetCurrentGraphic() : Graphic();
                    implWriteBitmapEx( pA->GetPoint(), pA->GetSize(), aBitmapEx, aGraphic, pDummyVDev, i_rContext );
                }
                break;

                case MetaActionType::BMPEXSCALEPART:
                {
                    const MetaBmpExScalePartAction* pA = static_cast<const MetaBmpExScalePartAction*>(pAction);

                    // The alpha mask is inverted from what is
                    // expected so invert it again. To test this
                    // code, export to PDF the transparent shapes,
                    // gradients, and images in the documents
                    // attached to the following bug reports:
                    //   https://bugs.documentfoundation.org/show_bug.cgi?id=155912
                    //   https://bugs.documentfoundation.org/show_bug.cgi?id=156630
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    if ( aBitmapEx.IsAlpha())
                    {
                        AlphaMask aAlpha = aBitmapEx.GetAlphaMask();
                        aAlpha.Invert();
                        aBitmapEx = BitmapEx(aBitmapEx.GetBitmap(), aAlpha);
                    }

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

const tools::Long unsetRun[256] =
{
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, /* 0x00 - 0x0f */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0x10 - 0x1f */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0x20 - 0x2f */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0x30 - 0x3f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x40 - 0x4f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x50 - 0x5f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x60 - 0x6f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x70 - 0x7f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x80 - 0x8f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x90 - 0x9f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xa0 - 0xaf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xb0 - 0xbf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xc0 - 0xcf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xd0 - 0xdf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xe0 - 0xef */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xf0 - 0xff */
};

const tools::Long setRun[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 - 0x0f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 - 0x1f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x20 - 0x2f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x30 - 0x3f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x40 - 0x4f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x50 - 0x5f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x60 - 0x6f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x70 - 0x7f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x80 - 0x8f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0x90 - 0x9f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0xa0 - 0xaf */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0xb0 - 0xbf */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0xc0 - 0xcf */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0xd0 - 0xdf */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, /* 0xe0 - 0xef */
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8, /* 0xf0 - 0xff */
};

static bool isSet( const Scanline i_pLine, tools::Long i_nIndex )
{
    return (i_pLine[ i_nIndex/8 ] & (0x80 >> (i_nIndex&7))) != 0;
}

static tools::Long findBitRunImpl( const Scanline i_pLine, tools::Long i_nStartIndex, tools::Long i_nW, bool i_bSet )
{
    tools::Long nIndex = i_nStartIndex;
    if( nIndex < i_nW )
    {
        const sal_uInt8 * pByte = i_pLine + (nIndex/8);
        sal_uInt8 nByte = *pByte;

        // run up to byte boundary
        tools::Long nBitInByte = (nIndex & 7);
        if( nBitInByte )
        {
            sal_uInt8 nMask = 0x80 >> nBitInByte;
            while( nBitInByte != 8 )
            {
                if( (nByte & nMask) != (i_bSet ? nMask : 0) )
                    return std::min(nIndex, i_nW);
                nMask = nMask >> 1;
                nBitInByte++;
                nIndex++;
            }
            if( nIndex < i_nW )
            {
                pByte++;
                nByte = *pByte;
            }
        }

        sal_uInt8 nRunByte;
        const tools::Long* pRunTable;
        if( i_bSet )
        {
            nRunByte = 0xff;
            pRunTable = setRun;
        }
        else
        {
            nRunByte = 0;
            pRunTable = unsetRun;
        }

        if( nIndex < i_nW )
        {
            while( nByte == nRunByte )
            {
                nIndex += 8;

                if (nIndex >= i_nW)
                    break;

                pByte++;
                nByte = *pByte;
            }
        }

        if( nIndex < i_nW )
        {
            nIndex += pRunTable[nByte];
        }
    }
    return std::min(nIndex, i_nW);
}

static tools::Long findBitRun(const Scanline i_pLine, tools::Long i_nStartIndex, tools::Long i_nW, bool i_bSet)
{
    if (i_nStartIndex < 0)
        return i_nW;

    return findBitRunImpl(i_pLine, i_nStartIndex, i_nW, i_bSet);
}

static tools::Long findBitRun(const Scanline i_pLine, tools::Long i_nStartIndex, tools::Long i_nW)
{
    if (i_nStartIndex < 0)
        return i_nW;

    const bool bSet = i_nStartIndex < i_nW && isSet(i_pLine, i_nStartIndex);

    return findBitRunImpl(i_pLine, i_nStartIndex, i_nW, bSet);
}

struct BitStreamState
{
    sal_uInt8       mnBuffer;
    sal_uInt32      mnNextBitPos;

    BitStreamState()
    : mnBuffer( 0 )
    , mnNextBitPos( 8 )
    {
    }

    const sal_uInt8& getByte() const { return mnBuffer; }
    void flush() { mnNextBitPos = 8; mnBuffer = 0; }
};

void PDFWriterImpl::putG4Bits( sal_uInt32 i_nLength, sal_uInt32 i_nCode, BitStreamState& io_rState )
{
    while( i_nLength > io_rState.mnNextBitPos )
    {
        io_rState.mnBuffer |= static_cast<sal_uInt8>( i_nCode >> (i_nLength - io_rState.mnNextBitPos) );
        i_nLength -= io_rState.mnNextBitPos;
        writeBufferBytes( &io_rState.getByte(), 1 );
        io_rState.flush();
    }
    assert(i_nLength < 9);
    static const unsigned int msbmask[9] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
    io_rState.mnBuffer |= static_cast<sal_uInt8>( (i_nCode & msbmask[i_nLength]) << (io_rState.mnNextBitPos - i_nLength) );
    io_rState.mnNextBitPos -= i_nLength;
    if( io_rState.mnNextBitPos == 0 )
    {
        writeBufferBytes( &io_rState.getByte(), 1 );
        io_rState.flush();
    }
}

namespace {

struct PixelCode
{
    sal_uInt32      mnEncodedPixels;
    sal_uInt32      mnCodeBits;
    sal_uInt32      mnCode;
};

}

const PixelCode WhitePixelCodes[] =
{
    { 0, 8, 0x35 },     // 0011 0101
    { 1, 6, 0x7 },      // 0001 11
    { 2, 4, 0x7 },      // 0111
    { 3, 4, 0x8 },      // 1000
    { 4, 4, 0xB },      // 1011
    { 5, 4, 0xC },      // 1100
    { 6, 4, 0xE },      // 1110
    { 7, 4, 0xF },      // 1111
    { 8, 5, 0x13 },     // 1001 1
    { 9, 5, 0x14 },     // 1010 0
    { 10, 5, 0x7 },     // 0011 1
    { 11, 5, 0x8 },     // 0100 0
    { 12, 6, 0x8 },     // 0010 00
    { 13, 6, 0x3 },     // 0000 11
    { 14, 6, 0x34 },    // 1101 00
    { 15, 6, 0x35 },    // 1101 01
    { 16, 6, 0x2A },    // 1010 10
    { 17, 6, 0x2B },    // 1010 11
    { 18, 7, 0x27 },    // 0100 111
    { 19, 7, 0xC },     // 0001 100
    { 20, 7, 0x8 },     // 0001 000
    { 21, 7, 0x17 },    // 0010 111
    { 22, 7, 0x3 },     // 0000 011
    { 23, 7, 0x4 },     // 0000 100
    { 24, 7, 0x28 },    // 0101 000
    { 25, 7, 0x2B },    // 0101 011
    { 26, 7, 0x13 },    // 0010 011
    { 27, 7, 0x24 },    // 0100 100
    { 28, 7, 0x18 },    // 0011 000
    { 29, 8, 0x2 },     // 0000 0010
    { 30, 8, 0x3 },     // 0000 0011
    { 31, 8, 0x1A },    // 0001 1010
    { 32, 8, 0x1B },    // 0001 1011
    { 33, 8, 0x12 },    // 0001 0010
    { 34, 8, 0x13 },    // 0001 0011
    { 35, 8, 0x14 },    // 0001 0100
    { 36, 8, 0x15 },    // 0001 0101
    { 37, 8, 0x16 },    // 0001 0110
    { 38, 8, 0x17 },    // 0001 0111
    { 39, 8, 0x28 },    // 0010 1000
    { 40, 8, 0x29 },    // 0010 1001
    { 41, 8, 0x2A },    // 0010 1010
    { 42, 8, 0x2B },    // 0010 1011
    { 43, 8, 0x2C },    // 0010 1100
    { 44, 8, 0x2D },    // 0010 1101
    { 45, 8, 0x4 },     // 0000 0100
    { 46, 8, 0x5 },     // 0000 0101
    { 47, 8, 0xA },     // 0000 1010
    { 48, 8, 0xB },     // 0000 1011
    { 49, 8, 0x52 },    // 0101 0010
    { 50, 8, 0x53 },    // 0101 0011
    { 51, 8, 0x54 },    // 0101 0100
    { 52, 8, 0x55 },    // 0101 0101
    { 53, 8, 0x24 },    // 0010 0100
    { 54, 8, 0x25 },    // 0010 0101
    { 55, 8, 0x58 },    // 0101 1000
    { 56, 8, 0x59 },    // 0101 1001
    { 57, 8, 0x5A },    // 0101 1010
    { 58, 8, 0x5B },    // 0101 1011
    { 59, 8, 0x4A },    // 0100 1010
    { 60, 8, 0x4B },    // 0100 1011
    { 61, 8, 0x32 },    // 0011 0010
    { 62, 8, 0x33 },    // 0011 0011
    { 63, 8, 0x34 },    // 0011 0100
    { 64, 5, 0x1B },    // 1101 1
    { 128, 5, 0x12 },   // 1001 0
    { 192, 6, 0x17 },   // 0101 11
    { 256, 7, 0x37 },   // 0110 111
    { 320, 8, 0x36 },   // 0011 0110
    { 384, 8, 0x37 },   // 0011 0111
    { 448, 8, 0x64 },   // 0110 0100
    { 512, 8, 0x65 },   // 0110 0101
    { 576, 8, 0x68 },   // 0110 1000
    { 640, 8, 0x67 },   // 0110 0111
    { 704, 9, 0xCC },   // 0110 0110 0
    { 768, 9, 0xCD },   // 0110 0110 1
    { 832, 9, 0xD2 },   // 0110 1001 0
    { 896, 9, 0xD3 },   // 0110 1001 1
    { 960, 9, 0xD4 },   // 0110 1010 0
    { 1024, 9, 0xD5 },  // 0110 1010 1
    { 1088, 9, 0xD6 },  // 0110 1011 0
    { 1152, 9, 0xD7 },  // 0110 1011 1
    { 1216, 9, 0xD8 },  // 0110 1100 0
    { 1280, 9, 0xD9 },  // 0110 1100 1
    { 1344, 9, 0xDA },  // 0110 1101 0
    { 1408, 9, 0xDB },  // 0110 1101 1
    { 1472, 9, 0x98 },  // 0100 1100 0
    { 1536, 9, 0x99 },  // 0100 1100 1
    { 1600, 9, 0x9A },  // 0100 1101 0
    { 1664, 6, 0x18 },  // 0110 00
    { 1728, 9, 0x9B },  // 0100 1101 1
    { 1792, 11, 0x8 },  // 0000 0001 000
    { 1856, 11, 0xC },  // 0000 0001 100
    { 1920, 11, 0xD },  // 0000 0001 101
    { 1984, 12, 0x12 }, // 0000 0001 0010
    { 2048, 12, 0x13 }, // 0000 0001 0011
    { 2112, 12, 0x14 }, // 0000 0001 0100
    { 2176, 12, 0x15 }, // 0000 0001 0101
    { 2240, 12, 0x16 }, // 0000 0001 0110
    { 2304, 12, 0x17 }, // 0000 0001 0111
    { 2368, 12, 0x1C }, // 0000 0001 1100
    { 2432, 12, 0x1D }, // 0000 0001 1101
    { 2496, 12, 0x1E }, // 0000 0001 1110
    { 2560, 12, 0x1F }  // 0000 0001 1111
};

const PixelCode BlackPixelCodes[] =
{
    { 0, 10, 0x37 },    // 0000 1101 11
    { 1, 3, 0x2 },      // 010
    { 2, 2, 0x3 },      // 11
    { 3, 2, 0x2 },      // 10
    { 4, 3, 0x3 },      // 011
    { 5, 4, 0x3 },      // 0011
    { 6, 4, 0x2 },      // 0010
    { 7, 5, 0x3 },      // 0001 1
    { 8, 6, 0x5 },      // 0001 01
    { 9, 6, 0x4 },      // 0001 00
    { 10, 7, 0x4 },     // 0000 100
    { 11, 7, 0x5 },     // 0000 101
    { 12, 7, 0x7 },     // 0000 111
    { 13, 8, 0x4 },     // 0000 0100
    { 14, 8, 0x7 },     // 0000 0111
    { 15, 9, 0x18 },    // 0000 1100 0
    { 16, 10, 0x17 },   // 0000 0101 11
    { 17, 10, 0x18 },   // 0000 0110 00
    { 18, 10, 0x8 },    // 0000 0010 00
    { 19, 11, 0x67 },   // 0000 1100 111
    { 20, 11, 0x68 },   // 0000 1101 000
    { 21, 11, 0x6C },   // 0000 1101 100
    { 22, 11, 0x37 },   // 0000 0110 111
    { 23, 11, 0x28 },   // 0000 0101 000
    { 24, 11, 0x17 },   // 0000 0010 111
    { 25, 11, 0x18 },   // 0000 0011 000
    { 26, 12, 0xCA },   // 0000 1100 1010
    { 27, 12, 0xCB },   // 0000 1100 1011
    { 28, 12, 0xCC },   // 0000 1100 1100
    { 29, 12, 0xCD },   // 0000 1100 1101
    { 30, 12, 0x68 },   // 0000 0110 1000
    { 31, 12, 0x69 },   // 0000 0110 1001
    { 32, 12, 0x6A },   // 0000 0110 1010
    { 33, 12, 0x6B },   // 0000 0110 1011
    { 34, 12, 0xD2 },   // 0000 1101 0010
    { 35, 12, 0xD3 },   // 0000 1101 0011
    { 36, 12, 0xD4 },   // 0000 1101 0100
    { 37, 12, 0xD5 },   // 0000 1101 0101
    { 38, 12, 0xD6 },   // 0000 1101 0110
    { 39, 12, 0xD7 },   // 0000 1101 0111
    { 40, 12, 0x6C },   // 0000 0110 1100
    { 41, 12, 0x6D },   // 0000 0110 1101
    { 42, 12, 0xDA },   // 0000 1101 1010
    { 43, 12, 0xDB },   // 0000 1101 1011
    { 44, 12, 0x54 },   // 0000 0101 0100
    { 45, 12, 0x55 },   // 0000 0101 0101
    { 46, 12, 0x56 },   // 0000 0101 0110
    { 47, 12, 0x57 },   // 0000 0101 0111
    { 48, 12, 0x64 },   // 0000 0110 0100
    { 49, 12, 0x65 },   // 0000 0110 0101
    { 50, 12, 0x52 },   // 0000 0101 0010
    { 51, 12, 0x53 },   // 0000 0101 0011
    { 52, 12, 0x24 },   // 0000 0010 0100
    { 53, 12, 0x37 },   // 0000 0011 0111
    { 54, 12, 0x38 },   // 0000 0011 1000
    { 55, 12, 0x27 },   // 0000 0010 0111
    { 56, 12, 0x28 },   // 0000 0010 1000
    { 57, 12, 0x58 },   // 0000 0101 1000
    { 58, 12, 0x59 },   // 0000 0101 1001
    { 59, 12, 0x2B },   // 0000 0010 1011
    { 60, 12, 0x2C },   // 0000 0010 1100
    { 61, 12, 0x5A },   // 0000 0101 1010
    { 62, 12, 0x66 },   // 0000 0110 0110
    { 63, 12, 0x67 },   // 0000 0110 0111
    { 64, 10, 0xF },    // 0000 0011 11
    { 128, 12, 0xC8 },  // 0000 1100 1000
    { 192, 12, 0xC9 },  // 0000 1100 1001
    { 256, 12, 0x5B },  // 0000 0101 1011
    { 320, 12, 0x33 },  // 0000 0011 0011
    { 384, 12, 0x34 },  // 0000 0011 0100
    { 448, 12, 0x35 },  // 0000 0011 0101
    { 512, 13, 0x6C },  // 0000 0011 0110 0
    { 576, 13, 0x6D },  // 0000 0011 0110 1
    { 640, 13, 0x4A },  // 0000 0010 0101 0
    { 704, 13, 0x4B },  // 0000 0010 0101 1
    { 768, 13, 0x4C },  // 0000 0010 0110 0
    { 832, 13, 0x4D },  // 0000 0010 0110 1
    { 896, 13, 0x72 },  // 0000 0011 1001 0
    { 960, 13, 0x73 },  // 0000 0011 1001 1
    { 1024, 13, 0x74 }, // 0000 0011 1010 0
    { 1088, 13, 0x75 }, // 0000 0011 1010 1
    { 1152, 13, 0x76 }, // 0000 0011 1011 0
    { 1216, 13, 0x77 }, // 0000 0011 1011 1
    { 1280, 13, 0x52 }, // 0000 0010 1001 0
    { 1344, 13, 0x53 }, // 0000 0010 1001 1
    { 1408, 13, 0x54 }, // 0000 0010 1010 0
    { 1472, 13, 0x55 }, // 0000 0010 1010 1
    { 1536, 13, 0x5A }, // 0000 0010 1101 0
    { 1600, 13, 0x5B }, // 0000 0010 1101 1
    { 1664, 13, 0x64 }, // 0000 0011 0010 0
    { 1728, 13, 0x65 }, // 0000 0011 0010 1
    { 1792, 11, 0x8 },  // 0000 0001 000
    { 1856, 11, 0xC },  // 0000 0001 100
    { 1920, 11, 0xD },  // 0000 0001 101
    { 1984, 12, 0x12 }, // 0000 0001 0010
    { 2048, 12, 0x13 }, // 0000 0001 0011
    { 2112, 12, 0x14 }, // 0000 0001 0100
    { 2176, 12, 0x15 }, // 0000 0001 0101
    { 2240, 12, 0x16 }, // 0000 0001 0110
    { 2304, 12, 0x17 }, // 0000 0001 0111
    { 2368, 12, 0x1C }, // 0000 0001 1100
    { 2432, 12, 0x1D }, // 0000 0001 1101
    { 2496, 12, 0x1E }, // 0000 0001 1110
    { 2560, 12, 0x1F }  // 0000 0001 1111
};

void PDFWriterImpl::putG4Span( tools::Long i_nSpan, bool i_bWhitePixel, BitStreamState& io_rState )
{
    const PixelCode* pTable = i_bWhitePixel ? WhitePixelCodes : BlackPixelCodes;
    // maximum encoded span is 2560 consecutive pixels
    while( i_nSpan > 2623 )
    {
        // write 2560 bits, that is entry (63 + (2560 >> 6)) == 103 in the appropriate table
        putG4Bits( pTable[103].mnCodeBits, pTable[103].mnCode, io_rState );
        i_nSpan -= pTable[103].mnEncodedPixels;
    }
    // write multiples of 64 pixels up to 2560
    if( i_nSpan > 63 )
    {
        sal_uInt32 nTabIndex = 63 + (i_nSpan >> 6);
        OSL_ASSERT( pTable[nTabIndex].mnEncodedPixels == static_cast<sal_uInt32>(64*(i_nSpan >> 6)) );
        putG4Bits( pTable[nTabIndex].mnCodeBits, pTable[nTabIndex].mnCode, io_rState );
        i_nSpan -= pTable[nTabIndex].mnEncodedPixels;
    }
    putG4Bits( pTable[i_nSpan].mnCodeBits, pTable[i_nSpan].mnCode, io_rState );
}

void PDFWriterImpl::writeG4Stream( BitmapReadAccess const * i_pBitmap )
{
    tools::Long nW = i_pBitmap->Width();
    tools::Long nH = i_pBitmap->Height();
    if( nW <= 0 || nH <= 0 )
        return;
    if( i_pBitmap->GetBitCount() != 1 )
        return;

    BitStreamState aBitState;

    // the first reference line is virtual and completely empty
    std::unique_ptr<sal_uInt8[]> pFirstRefLine(new  sal_uInt8[nW/8 + 1]);
    memset(pFirstRefLine.get(), 0, nW/8 + 1);
    Scanline pRefLine = pFirstRefLine.get();
    for( tools::Long nY = 0; nY < nH; nY++ )
    {
        const Scanline pCurLine = i_pBitmap->GetScanline( nY );
        tools::Long nLineIndex = 0;
        bool bRunSet = (*pCurLine & 0x80) != 0;
        bool bRefSet = (*pRefLine & 0x80) != 0;
        tools::Long nRunIndex1 = bRunSet ? 0 : findBitRun( pCurLine, 0, nW, bRunSet );
        tools::Long nRefIndex1 = bRefSet ? 0 : findBitRun( pRefLine, 0, nW, bRefSet );
        for( ; nLineIndex < nW; )
        {
            tools::Long nRefIndex2 = findBitRun( pRefLine, nRefIndex1, nW );
            if( nRefIndex2 >= nRunIndex1 )
            {
                tools::Long nDiff = nRefIndex1 - nRunIndex1;
                if( -3 <= nDiff && nDiff <= 3 )
                {   // vertical coding
                    static const struct
                    {
                        sal_uInt32 mnCodeBits;
                        sal_uInt32 mnCode;
                    } VerticalCodes[7] = {
                        { 7, 0x03 },    // 0000 011
                        { 6, 0x03 },    // 0000 11
                        { 3, 0x03 },    // 011
                        { 1, 0x1 },     // 1
                        { 3, 0x2 },     // 010
                        { 6, 0x02 },    // 0000 10
                        { 7, 0x02 }     // 0000 010
                    };
                    // convert to index
                    nDiff += 3;

                    // emit diff code
                    putG4Bits( VerticalCodes[nDiff].mnCodeBits, VerticalCodes[nDiff].mnCode, aBitState );
                    nLineIndex = nRunIndex1;
                }
                else
                {   // difference too large, horizontal coding
                    // emit horz code 001
                    putG4Bits( 3, 0x1, aBitState );
                    tools::Long nRunIndex2 = findBitRun( pCurLine, nRunIndex1, nW );
                    bool bWhiteFirst = ( nLineIndex + nRunIndex1 == 0 || ! isSet( pCurLine, nLineIndex ) );
                    putG4Span( nRunIndex1 - nLineIndex, bWhiteFirst, aBitState );
                    putG4Span( nRunIndex2 - nRunIndex1, ! bWhiteFirst, aBitState );
                    nLineIndex = nRunIndex2;
                }
            }
            else
            {   // emit pass code 0001
                putG4Bits( 4, 0x1, aBitState );
                nLineIndex = nRefIndex2;
            }
            if( nLineIndex < nW )
            {
                bool bSet = isSet( pCurLine, nLineIndex );
                nRunIndex1 = findBitRun( pCurLine, nLineIndex, nW, bSet );
                nRefIndex1 = findBitRun( pRefLine, nLineIndex, nW, ! bSet );
                nRefIndex1 = findBitRun( pRefLine, nRefIndex1, nW, bSet );
            }
        }

        // the current line is the reference for the next line
        pRefLine = pCurLine;
    }
    // terminate strip with EOFB
    putG4Bits( 12, 1, aBitState );
    putG4Bits( 12, 1, aBitState );
    if( aBitState.mnNextBitPos != 8 )
    {
        writeBufferBytes( &aBitState.getByte(), 1 );
        aBitState.flush();
    }
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
