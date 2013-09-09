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

#include "pdfwriter_impl.hxx"

#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/virdev.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/metaact.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/graph.hxx"

#include "svdata.hxx"

#include "unotools/streamwrap.hxx"

#include "comphelper/processfactory.hxx"

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/io/XSeekable.hpp"
#include "com/sun/star/graphic/GraphicProvider.hpp"
#include "com/sun/star/graphic/XGraphicProvider.hpp"

#include "cppuhelper/implbase1.hxx"

#include <rtl/digest.h>

using namespace vcl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

static bool lcl_canUsePDFAxialShading(const Gradient& rGradient);

// -----------------------------------------------------------------------------

void PDFWriterImpl::implWriteGradient( const PolyPolygon& i_rPolyPoly, const Gradient& i_rGradient,
                                       VirtualDevice* i_pDummyVDev, const vcl::PDFWriter::PlayMetafileContext& i_rContext )
{
    GDIMetaFile        aTmpMtf;

    i_pDummyVDev->AddGradientActions( i_rPolyPoly.GetBoundRect(), i_rGradient, aTmpMtf );

    m_rOuterFace.Push();
    m_rOuterFace.IntersectClipRegion( i_rPolyPoly.getB2DPolyPolygon() );
    playMetafile( aTmpMtf, NULL, i_rContext, i_pDummyVDev );
    m_rOuterFace.Pop();
}

// -----------------------------------------------------------------------------

void PDFWriterImpl::implWriteBitmapEx( const Point& i_rPoint, const Size& i_rSize, const BitmapEx& i_rBitmapEx,
                                       VirtualDevice* i_pDummyVDev, const vcl::PDFWriter::PlayMetafileContext& i_rContext )
{
    if ( !i_rBitmapEx.IsEmpty() && i_rSize.Width() && i_rSize.Height() )
    {
        BitmapEx        aBitmapEx( i_rBitmapEx );
        Point           aPoint( i_rPoint );
        Size            aSize( i_rSize );

        // #i19065# Negative sizes have mirror semantics on
        // OutputDevice. BitmapEx and co. have no idea about that, so
        // perform that _before_ doing anything with aBitmapEx.
        sal_uLong nMirrorFlags(BMP_MIRROR_NONE);
        if( aSize.Width() < 0 )
        {
            aSize.Width() *= -1;
            aPoint.X() -= aSize.Width();
            nMirrorFlags |= BMP_MIRROR_HORZ;
        }
        if( aSize.Height() < 0 )
        {
            aSize.Height() *= -1;
            aPoint.Y() -= aSize.Height();
            nMirrorFlags |= BMP_MIRROR_VERT;
        }

        if( nMirrorFlags != BMP_MIRROR_NONE )
        {
            aBitmapEx.Mirror( nMirrorFlags );
        }
        if( i_rContext.m_nMaxImageResolution > 50 )
        {
            // do downsampling if necessary
            const Size      aDstSizeTwip( i_pDummyVDev->PixelToLogic( i_pDummyVDev->LogicToPixel( aSize ), MAP_TWIP ) );
            const Size      aBmpSize( aBitmapEx.GetSizePixel() );
            const double    fBmpPixelX = aBmpSize.Width();
            const double    fBmpPixelY = aBmpSize.Height();
            const double    fMaxPixelX = aDstSizeTwip.Width() * i_rContext.m_nMaxImageResolution / 1440.0;
            const double    fMaxPixelY = aDstSizeTwip.Height() * i_rContext.m_nMaxImageResolution / 1440.0;

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
                    aNewBmpSize.Width() = FRound( fMaxPixelY * fBmpWH );
                    aNewBmpSize.Height() = FRound( fMaxPixelY );
                }
                else if( fBmpWH > 0.0 )
                {
                    aNewBmpSize.Width() = FRound( fMaxPixelX );
                    aNewBmpSize.Height() = FRound( fMaxPixelX / fBmpWH);
                }

                if( aNewBmpSize.Width() && aNewBmpSize.Height() )
                {
                    // #i121233# Use best quality for PDF exports
                    aBitmapEx.Scale( aNewBmpSize, BMP_SCALE_BESTQUALITY );
                }
                else
                {
                    aBitmapEx.SetEmpty();
                }
            }
        }

        const Size aSizePixel( aBitmapEx.GetSizePixel() );
        if ( aSizePixel.Width() && aSizePixel.Height() )
        {
            if( m_aContext.ColorMode == PDFWriter::DrawGreyscale )
            {
                BmpConversion eConv = BMP_CONVERSION_8BIT_GREYS;
                int nDepth = aBitmapEx.GetBitmap().GetBitCount();
                if( nDepth <= 4 )
                    eConv = BMP_CONVERSION_4BIT_GREYS;
                if( nDepth > 1 )
                    aBitmapEx.Convert( eConv );
            }
            bool bUseJPGCompression = !i_rContext.m_bOnlyLosslessCompression;
            if ( ( aSizePixel.Width() < 32 ) || ( aSizePixel.Height() < 32 ) )
                bUseJPGCompression = false;

            SvMemoryStream  aStrm;
            Bitmap          aMask;

            bool bTrueColorJPG = true;
            if ( bUseJPGCompression )
            {
                sal_uInt32 nZippedFileSize;     // sj: we will calculate the filesize of a zipped bitmap
                {                               // to determine if jpeg compression is useful
                    SvMemoryStream aTemp;
                    aTemp.SetCompressMode( aTemp.GetCompressMode() | COMPRESSMODE_ZBITMAP );
                    aTemp.SetVersion( SOFFICE_FILEFORMAT_40 );  // sj: up from version 40 our bitmap stream operator
                    WriteDIBBitmapEx(aBitmapEx, aTemp); // is capable of zlib stream compression
                    aTemp.Seek( STREAM_SEEK_TO_END );
                    nZippedFileSize = aTemp.Tell();
                }
                if ( aBitmapEx.IsTransparent() )
                {
                    if ( aBitmapEx.IsAlpha() )
                        aMask = aBitmapEx.GetAlpha().GetBitmap();
                    else
                        aMask = aBitmapEx.GetMask();
                }
                Graphic         aGraphic( aBitmapEx.GetBitmap() );
                sal_Int32       nColorMode = 0;

                Sequence< PropertyValue > aFilterData( 2 );
                aFilterData[ 0 ].Name = OUString( "Quality" );
                aFilterData[ 0 ].Value <<= sal_Int32(i_rContext.m_nJPEGQuality);
                aFilterData[ 1 ].Name = OUString( "ColorMode" );
                aFilterData[ 1 ].Value <<= nColorMode;

                try
                {
                    uno::Reference < io::XStream > xStream = new utl::OStreamWrapper( aStrm );
                    uno::Reference< io::XSeekable > xSeekable( xStream, UNO_QUERY_THROW );
                    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
                    uno::Reference< graphic::XGraphicProvider > xGraphicProvider( graphic::GraphicProvider::create(xContext) );
                    uno::Reference< graphic::XGraphic > xGraphic( aGraphic.GetXGraphic() );
                    uno::Reference < io::XOutputStream > xOut( xStream->getOutputStream() );
                    OUString aMimeType("image/jpeg");
                    uno::Sequence< beans::PropertyValue > aOutMediaProperties( 3 );
                    aOutMediaProperties[0].Name = OUString("OutputStream");
                    aOutMediaProperties[0].Value <<= xOut;
                    aOutMediaProperties[1].Name = OUString("MimeType");
                    aOutMediaProperties[1].Value <<= aMimeType;
                    aOutMediaProperties[2].Name = OUString("FilterData");
                    aOutMediaProperties[2].Value <<= aFilterData;
                    xGraphicProvider->storeGraphic( xGraphic, aOutMediaProperties );
                    xOut->flush();
                    if ( xSeekable->getLength() > nZippedFileSize )
                    {
                        bUseJPGCompression = false;
                    }
                    else
                    {
                        aStrm.Seek( STREAM_SEEK_TO_END );

                        xSeekable->seek( 0 );
                        Sequence< PropertyValue > aArgs( 1 );
                        aArgs[ 0 ].Name = OUString("InputStream");
                        aArgs[ 0 ].Value <<= xStream;
                        uno::Reference< XPropertySet > xPropSet( xGraphicProvider->queryGraphicDescriptor( aArgs ) );
                        if ( xPropSet.is() )
                        {
                            sal_Int16 nBitsPerPixel = 24;
                            if ( xPropSet->getPropertyValue("BitsPerPixel") >>= nBitsPerPixel )
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
                m_rOuterFace.DrawJPGBitmap( aStrm, bTrueColorJPG, aSizePixel, Rectangle( aPoint, aSize ), aMask );
            else if ( aBitmapEx.IsTransparent() )
                m_rOuterFace.DrawBitmapEx( aPoint, aSize, aBitmapEx );
            else
                m_rOuterFace.DrawBitmap( aPoint, aSize, aBitmapEx.GetBitmap() );
        }
    }
}


// -----------------------------------------------------------------------------

void PDFWriterImpl::playMetafile( const GDIMetaFile& i_rMtf, vcl::PDFExtOutDevData* i_pOutDevData, const vcl::PDFWriter::PlayMetafileContext& i_rContext, VirtualDevice* pDummyVDev )
{
    bool bAssertionFired( false );

    VirtualDevice* pPrivateDevice = NULL;
    if( ! pDummyVDev )
    {
        pPrivateDevice = pDummyVDev = new VirtualDevice();
        pDummyVDev->EnableOutput( sal_False );
        pDummyVDev->SetMapMode( i_rMtf.GetPrefMapMode() );
    }
    GDIMetaFile aMtf( i_rMtf );

    for( sal_uInt32 i = 0, nCount = aMtf.GetActionSize(); i < (sal_uInt32)nCount; )
    {
        if ( !i_pOutDevData || !i_pOutDevData->PlaySyncPageAct( m_rOuterFace, i ) )
        {
            const MetaAction*   pAction = aMtf.GetAction( i );
            const sal_uInt16        nType = pAction->GetType();

            switch( nType )
            {
                case( META_PIXEL_ACTION ):
                {
                    const MetaPixelAction* pA = (const MetaPixelAction*) pAction;
                    m_rOuterFace.DrawPixel( pA->GetPoint(), pA->GetColor() );
                }
                break;

                case( META_POINT_ACTION ):
                {
                    const MetaPointAction* pA = (const MetaPointAction*) pAction;
                    m_rOuterFace.DrawPixel( pA->GetPoint() );
                }
                break;

                case( META_LINE_ACTION ):
                {
                    const MetaLineAction* pA = (const MetaLineAction*) pAction;
                    if ( pA->GetLineInfo().IsDefault() )
                        m_rOuterFace.DrawLine( pA->GetStartPoint(), pA->GetEndPoint() );
                    else
                        m_rOuterFace.DrawLine( pA->GetStartPoint(), pA->GetEndPoint(), pA->GetLineInfo() );
                }
                break;

                case( META_RECT_ACTION ):
                {
                    const MetaRectAction* pA = (const MetaRectAction*) pAction;
                    m_rOuterFace.DrawRect( pA->GetRect() );
                }
                break;

                case( META_ROUNDRECT_ACTION ):
                {
                    const MetaRoundRectAction* pA = (const MetaRoundRectAction*) pAction;
                    m_rOuterFace.DrawRect( pA->GetRect(), pA->GetHorzRound(), pA->GetVertRound() );
                }
                break;

                case( META_ELLIPSE_ACTION ):
                {
                    const MetaEllipseAction* pA = (const MetaEllipseAction*) pAction;
                    m_rOuterFace.DrawEllipse( pA->GetRect() );
                }
                break;

                case( META_ARC_ACTION ):
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pAction;
                    m_rOuterFace.DrawArc( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_PIE_ACTION ):
                {
                    const MetaArcAction* pA = (const MetaArcAction*) pAction;
                    m_rOuterFace.DrawPie( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_CHORD_ACTION ):
                {
                    const MetaChordAction* pA = (const MetaChordAction*) pAction;
                    m_rOuterFace.DrawChord( pA->GetRect(), pA->GetStartPoint(), pA->GetEndPoint() );
                }
                break;

                case( META_POLYGON_ACTION ):
                {
                    const MetaPolygonAction* pA = (const MetaPolygonAction*) pAction;
                    m_rOuterFace.DrawPolygon( pA->GetPolygon() );
                }
                break;

                case( META_POLYLINE_ACTION ):
                {
                    const MetaPolyLineAction* pA = (const MetaPolyLineAction*) pAction;
                    if ( pA->GetLineInfo().IsDefault() )
                        m_rOuterFace.DrawPolyLine( pA->GetPolygon() );
                    else
                        m_rOuterFace.DrawPolyLine( pA->GetPolygon(), pA->GetLineInfo() );
                }
                break;

                case( META_POLYPOLYGON_ACTION ):
                {
                    const MetaPolyPolygonAction* pA = (const MetaPolyPolygonAction*) pAction;
                    m_rOuterFace.DrawPolyPolygon( pA->GetPolyPolygon() );
                }
                break;

                case( META_GRADIENT_ACTION ):
                {
                    const MetaGradientAction* pA = (const MetaGradientAction*) pAction;
                    const Gradient& rGradient = pA->GetGradient();
                    if (lcl_canUsePDFAxialShading(rGradient))
                    {
                        m_rOuterFace.DrawGradient( pA->GetRect(), rGradient );
                    }
                    else
                    {
                        const PolyPolygon aPolyPoly( pA->GetRect() );
                        implWriteGradient( aPolyPoly, rGradient, pDummyVDev, i_rContext );
                    }
                }
                break;

                case( META_GRADIENTEX_ACTION ):
                {
                    const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                    const Gradient& rGradient = pA->GetGradient();

                    if (lcl_canUsePDFAxialShading(rGradient))
                        m_rOuterFace.DrawGradient( pA->GetPolyPolygon(), rGradient );
                    else
                        implWriteGradient( pA->GetPolyPolygon(), rGradient, pDummyVDev, i_rContext );
                }
                break;

                case META_HATCH_ACTION:
                {
                    const MetaHatchAction*  pA = (const MetaHatchAction*) pAction;
                    m_rOuterFace.DrawHatch( pA->GetPolyPolygon(), pA->GetHatch() );
                }
                break;

                case( META_TRANSPARENT_ACTION ):
                {
                    const MetaTransparentAction* pA = (const MetaTransparentAction*) pAction;
                    m_rOuterFace.DrawTransparent( pA->GetPolyPolygon(), pA->GetTransparence() );
                }
                break;

                case( META_FLOATTRANSPARENT_ACTION ):
                {
                    const MetaFloatTransparentAction* pA = (const MetaFloatTransparentAction*) pAction;

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
                        playMetafile( aTmpMtf, NULL, i_rContext, pDummyVDev );
                        m_rOuterFace.EndTransparencyGroup( Rectangle( rPos, rSize ), nTransPercent );
                    }
                    else
                    {
                        const Size  aDstSizeTwip( pDummyVDev->PixelToLogic( pDummyVDev->LogicToPixel( rSize ), MAP_TWIP ) );

                        // i#115962# Always use at least 300 DPI for bitmap conversion of transparence gradients,
                        // else the quality is not acceptable (see bugdoc as example)
                        sal_Int32 nMaxBmpDPI(300);

                        if( i_rContext.m_nMaxImageResolution > 50 )
                        {
                            if ( nMaxBmpDPI > i_rContext.m_nMaxImageResolution )
                                nMaxBmpDPI = i_rContext.m_nMaxImageResolution;
                        }
                        const sal_Int32 nPixelX = (sal_Int32)((double)aDstSizeTwip.Width() * (double)nMaxBmpDPI / 1440.0);
                        const sal_Int32 nPixelY = (sal_Int32)((double)aDstSizeTwip.Height() * (double)nMaxBmpDPI / 1440.0);
                        if ( nPixelX && nPixelY )
                        {
                            Size aDstSizePixel( nPixelX, nPixelY );
                            VirtualDevice* pVDev = new VirtualDevice;
                            if( pVDev->SetOutputSizePixel( aDstSizePixel ) )
                            {
                                Bitmap          aPaint, aMask;
                                AlphaMask       aAlpha;
                                Point           aPoint;

                                MapMode aMapMode( pDummyVDev->GetMapMode() );
                                aMapMode.SetOrigin( aPoint );
                                pVDev->SetMapMode( aMapMode );
                                Size aDstSize( pVDev->PixelToLogic( aDstSizePixel ) );

                                Point   aMtfOrigin( aTmpMtf.GetPrefMapMode().GetOrigin() );
                                if ( aMtfOrigin.X() || aMtfOrigin.Y() )
                                    aTmpMtf.Move( -aMtfOrigin.X(), -aMtfOrigin.Y() );
                                double  fScaleX = (double)aDstSize.Width() / (double)aTmpMtf.GetPrefSize().Width();
                                double  fScaleY = (double)aDstSize.Height() / (double)aTmpMtf.GetPrefSize().Height();
                                if( fScaleX != 1.0 || fScaleY != 1.0 )
                                    aTmpMtf.Scale( fScaleX, fScaleY );
                                aTmpMtf.SetPrefMapMode( aMapMode );

                                // create paint bitmap
                                aTmpMtf.WindStart();
                                aTmpMtf.Play( pVDev, aPoint, aDstSize );
                                aTmpMtf.WindStart();

                                pVDev->EnableMapMode( sal_False );
                                aPaint = pVDev->GetBitmap( aPoint, aDstSizePixel );
                                pVDev->EnableMapMode( sal_True );

                                // create mask bitmap
                                pVDev->SetLineColor( COL_BLACK );
                                pVDev->SetFillColor( COL_BLACK );
                                pVDev->DrawRect( Rectangle( aPoint, aDstSize ) );
                                pVDev->SetDrawMode( DRAWMODE_WHITELINE | DRAWMODE_WHITEFILL | DRAWMODE_WHITETEXT |
                                                    DRAWMODE_WHITEBITMAP | DRAWMODE_WHITEGRADIENT );
                                aTmpMtf.WindStart();
                                aTmpMtf.Play( pVDev, aPoint, aDstSize );
                                aTmpMtf.WindStart();
                                pVDev->EnableMapMode( sal_False );
                                aMask = pVDev->GetBitmap( aPoint, aDstSizePixel );
                                pVDev->EnableMapMode( sal_True );

                                // create alpha mask from gradient
                                pVDev->SetDrawMode( DRAWMODE_GRAYGRADIENT );
                                pVDev->DrawGradient( Rectangle( aPoint, aDstSize ), rTransparenceGradient );
                                pVDev->SetDrawMode( DRAWMODE_DEFAULT );
                                pVDev->EnableMapMode( sal_False );
                                pVDev->DrawMask( aPoint, aDstSizePixel, aMask, Color( COL_WHITE ) );
                                aAlpha = pVDev->GetBitmap( aPoint, aDstSizePixel );
                                implWriteBitmapEx( rPos, rSize, BitmapEx( aPaint, aAlpha ), pDummyVDev, i_rContext );
                            }
                            delete pVDev;
                        }
                    }
                }
                break;

                case( META_EPS_ACTION ):
                {
                    const MetaEPSAction*    pA = (const MetaEPSAction*) pAction;
                    const GDIMetaFile       aSubstitute( pA->GetSubstitute() );

                    m_rOuterFace.Push();
                    pDummyVDev->Push();

                    MapMode aMapMode( aSubstitute.GetPrefMapMode() );
                    Size aOutSize( pDummyVDev->LogicToLogic( pA->GetSize(), pDummyVDev->GetMapMode(), aMapMode ) );
                    aMapMode.SetScaleX( Fraction( aOutSize.Width(), aSubstitute.GetPrefSize().Width() ) );
                    aMapMode.SetScaleY( Fraction( aOutSize.Height(), aSubstitute.GetPrefSize().Height() ) );
                    aMapMode.SetOrigin( pDummyVDev->LogicToLogic( pA->GetPoint(), pDummyVDev->GetMapMode(), aMapMode ) );

                    m_rOuterFace.SetMapMode( aMapMode );
                    pDummyVDev->SetMapMode( aMapMode );
                    playMetafile( aSubstitute, NULL, i_rContext, pDummyVDev );
                    pDummyVDev->Pop();
                    m_rOuterFace.Pop();
                }
                break;

                case( META_COMMENT_ACTION ):
                if( ! i_rContext.m_bTransparenciesWereRemoved )
                {
                    const MetaCommentAction*    pA = (const MetaCommentAction*) pAction;

                    if( pA->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_BEGIN"))
                    {
                        const MetaGradientExAction* pGradAction = NULL;
                        bool                        bDone = false;

                        while( !bDone && ( ++i < nCount ) )
                        {
                            pAction = aMtf.GetAction( i );

                            if( pAction->GetType() == META_GRADIENTEX_ACTION )
                                pGradAction = (const MetaGradientExAction*) pAction;
                            else if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                     ( ( (const MetaCommentAction*) pAction )->GetComment().equalsIgnoreAsciiCase("XGRAD_SEQ_END")) )
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
                            SvMemoryStream  aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );
                            bool            bSkipSequence = false;
                            OString sSeqEnd;

                            if( pA->GetComment() == "XPATHSTROKE_SEQ_BEGIN" )
                            {
                                sSeqEnd = OString("XPATHSTROKE_SEQ_END");
                                SvtGraphicStroke aStroke;
                                aMemStm >> aStroke;

                                Polygon aPath;
                                aStroke.getPath( aPath );

                                PolyPolygon aStartArrow;
                                PolyPolygon aEndArrow;
                                double fTransparency( aStroke.getTransparency() );
                                double fStrokeWidth( aStroke.getStrokeWidth() );
                                SvtGraphicStroke::DashArray aDashArray;

                                aStroke.getStartArrow( aStartArrow );
                                aStroke.getEndArrow( aEndArrow );
                                aStroke.getDashArray( aDashArray );

                                bSkipSequence = true;
                                if ( aStartArrow.Count() || aEndArrow.Count() )
                                    bSkipSequence = false;
                                if ( aDashArray.size() && ( fStrokeWidth != 0.0 ) && ( fTransparency == 0.0 ) )
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
                                    aInfo.m_aDashArray = aDashArray;

                                    if(SvtGraphicStroke::joinNone == aStroke.getJoinType()
                                        && fStrokeWidth > 0.0)
                                    {
                                        // emulate no edge rounding by handling single edges
                                        const sal_uInt16 nPoints(aPath.GetSize());
                                        const bool bCurve(aPath.HasFlags());

                                        for(sal_uInt16 a(0); a + 1 < nPoints; a++)
                                        {
                                            if(bCurve
                                                && POLY_NORMAL != aPath.GetFlags(a + 1)
                                                && a + 2 < nPoints
                                                && POLY_NORMAL != aPath.GetFlags(a + 2)
                                                && a + 3 < nPoints)
                                            {
                                                const Polygon aSnippet(4,
                                                    aPath.GetConstPointAry() + a,
                                                    aPath.GetConstFlagAry() + a);
                                                m_rOuterFace.DrawPolyLine( aSnippet, aInfo );
                                                a += 2;
                                            }
                                            else
                                            {
                                                const Polygon aSnippet(2,
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
                                sSeqEnd = OString("XPATHFILL_SEQ_END");
                                SvtGraphicFill aFill;
                                aMemStm >> aFill;

                                if ( ( aFill.getFillType() == SvtGraphicFill::fillSolid ) && ( aFill.getFillRule() == SvtGraphicFill::fillEvenOdd ) )
                                {
                                    double fTransparency = aFill.getTransparency();
                                    if ( fTransparency == 0.0 )
                                    {
                                        PolyPolygon aPath;
                                        aFill.getPath( aPath );

                                        bSkipSequence = true;
                                        m_rOuterFace.DrawPolyPolygon( aPath );
                                    }
                                    else if ( fTransparency == 1.0 )
                                        bSkipSequence = true;
                                }
/* #i81548# removing optimization for fill textures, because most of the texture settings are not
   exported properly. In OpenOffice 3.1 the drawing layer will support graphic primitives, then it
   will not be a problem to optimize the filltexture export. But for wysiwyg is more important than
   filesize.
                                else if( aFill.getFillType() == SvtGraphicFill::fillTexture && aFill.isTiling() )
                                {
                                    sal_Int32 nPattern = mnCachePatternId;
                                    Graphic aPatternGraphic;
                                    aFill.getGraphic( aPatternGraphic );
                                    bool bUseCache = false;
                                    SvtGraphicFill::Transform aPatTransform;
                                    aFill.getTransform( aPatTransform );

                                    if(  mnCachePatternId >= 0 )
                                    {
                                        SvtGraphicFill::Transform aCacheTransform;
                                        maCacheFill.getTransform( aCacheTransform );
                                        if( aCacheTransform.matrix[0] == aPatTransform.matrix[0] &&
                                            aCacheTransform.matrix[1] == aPatTransform.matrix[1] &&
                                            aCacheTransform.matrix[2] == aPatTransform.matrix[2] &&
                                            aCacheTransform.matrix[3] == aPatTransform.matrix[3] &&
                                            aCacheTransform.matrix[4] == aPatTransform.matrix[4] &&
                                            aCacheTransform.matrix[5] == aPatTransform.matrix[5]
                                            )
                                        {
                                            Graphic aCacheGraphic;
                                            maCacheFill.getGraphic( aCacheGraphic );
                                            if( aCacheGraphic == aPatternGraphic )
                                                bUseCache = true;
                                        }
                                    }

                                    if( ! bUseCache )
                                    {

                                        // paint graphic to metafile
                                        GDIMetaFile aPattern;
                                        pDummyVDev->SetConnectMetaFile( &aPattern );
                                        pDummyVDev->Push();
                                        pDummyVDev->SetMapMode( aPatternGraphic.GetPrefMapMode() );

                                        aPatternGraphic.Draw( &rDummyVDev, Point( 0, 0 ) );
                                        pDummyVDev->Pop();
                                        pDummyVDev->SetConnectMetaFile( NULL );
                                        aPattern.WindStart();

                                        MapMode aPatternMapMode( aPatternGraphic.GetPrefMapMode() );
                                        // prepare pattern from metafile
                                        Size aPrefSize( aPatternGraphic.GetPrefSize() );
                                        // FIXME: this magic -1 shouldn't be necessary
                                        aPrefSize.Width() -= 1;
                                        aPrefSize.Height() -= 1;
                                        aPrefSize = m_rOuterFace.GetReferenceDevice()->
                                            LogicToLogic( aPrefSize,
                                                          &aPatternMapMode,
                                                          &m_rOuterFace.GetReferenceDevice()->GetMapMode() );
                                        // build bounding rectangle of pattern
                                        Rectangle aBound( Point( 0, 0 ), aPrefSize );
                                        m_rOuterFace.BeginPattern( aBound );
                                        m_rOuterFace.Push();
                                        pDummyVDev->Push();
                                        m_rOuterFace.SetMapMode( aPatternMapMode );
                                        pDummyVDev->SetMapMode( aPatternMapMode );
                                        ImplWriteActions( m_rOuterFace, NULL, aPattern, rDummyVDev );
                                        pDummyVDev->Pop();
                                        m_rOuterFace.Pop();

                                        nPattern = m_rOuterFace.EndPattern( aPatTransform );

                                        // try some caching and reuse pattern
                                        mnCachePatternId = nPattern;
                                        maCacheFill = aFill;
                                    }

                                    // draw polypolygon with pattern fill
                                    PolyPolygon aPath;
                                    aFill.getPath( aPath );
                                    m_rOuterFace.DrawPolyPolygon( aPath, nPattern, aFill.getFillRule() == SvtGraphicFill::fillEvenOdd );

                                    bSkipSequence = true;
                                }
*/
                            }
                            if ( bSkipSequence )
                            {
                                while( ++i < nCount )
                                {
                                    pAction = aMtf.GetAction( i );
                                    if ( pAction->GetType() == META_COMMENT_ACTION )
                                    {
                                        OString sComment( ((MetaCommentAction*)pAction)->GetComment() );
                                        if (sComment == sSeqEnd)
                                            break;
                                    }
                                    // #i44496#
                                    // the replacement action for stroke is a filled rectangle
                                    // the set fillcolor of the replacement is part of the graphics
                                    // state and must not be skipped
                                    else if( pAction->GetType() == META_FILLCOLOR_ACTION )
                                    {
                                        const MetaFillColorAction* pMA = (const MetaFillColorAction*) pAction;
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

                case( META_BMP_ACTION ):
                {
                    const MetaBmpAction* pA = (const MetaBmpAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmap() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                                                            aBitmapEx.GetPrefMapMode(), pDummyVDev->GetMapMode() ) );
                    if( ! ( aSize.Width() && aSize.Height() ) )
                        aSize = pDummyVDev->PixelToLogic( aBitmapEx.GetSizePixel() );
                    implWriteBitmapEx( pA->GetPoint(), aSize, aBitmapEx, pDummyVDev, i_rContext );
                }
                break;

                case( META_BMPSCALE_ACTION ):
                {
                    const MetaBmpScaleAction* pA = (const MetaBmpScaleAction*) pAction;
                    implWriteBitmapEx( pA->GetPoint(), pA->GetSize(), BitmapEx( pA->GetBitmap() ), pDummyVDev, i_rContext );
                }
                break;

                case( META_BMPSCALEPART_ACTION ):
                {
                    const MetaBmpScalePartAction* pA = (const MetaBmpScalePartAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmap() );
                    aBitmapEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    implWriteBitmapEx( pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx, pDummyVDev, i_rContext );
                }
                break;

                case( META_BMPEX_ACTION ):
                {
                    const MetaBmpExAction*  pA = (const MetaBmpExAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    Size aSize( OutputDevice::LogicToLogic( aBitmapEx.GetPrefSize(),
                            aBitmapEx.GetPrefMapMode(), pDummyVDev->GetMapMode() ) );
                    implWriteBitmapEx( pA->GetPoint(), aSize, aBitmapEx, pDummyVDev, i_rContext );
                }
                break;

                case( META_BMPEXSCALE_ACTION ):
                {
                    const MetaBmpExScaleAction* pA = (const MetaBmpExScaleAction*) pAction;
                    implWriteBitmapEx( pA->GetPoint(), pA->GetSize(), pA->GetBitmapEx(), pDummyVDev, i_rContext );
                }
                break;

                case( META_BMPEXSCALEPART_ACTION ):
                {
                    const MetaBmpExScalePartAction* pA = (const MetaBmpExScalePartAction*) pAction;
                    BitmapEx aBitmapEx( pA->GetBitmapEx() );
                    aBitmapEx.Crop( Rectangle( pA->GetSrcPoint(), pA->GetSrcSize() ) );
                    implWriteBitmapEx( pA->GetDestPoint(), pA->GetDestSize(), aBitmapEx, pDummyVDev, i_rContext );
                }
                break;

                case( META_MASK_ACTION ):
                case( META_MASKSCALE_ACTION ):
                case( META_MASKSCALEPART_ACTION ):
                {
                    OSL_TRACE( "MetaMask...Action not supported yet" );
                }
                break;

                case( META_TEXT_ACTION ):
                {
                    const MetaTextAction* pA = (const MetaTextAction*) pAction;
                    m_rOuterFace.DrawText( pA->GetPoint(), pA->GetText().copy( pA->GetIndex(), pA->GetLen() ) );
                }
                break;

                case( META_TEXTRECT_ACTION ):
                {
                    const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                    m_rOuterFace.DrawText( pA->GetRect(), pA->GetText(), pA->GetStyle() );
                }
                break;

                case( META_TEXTARRAY_ACTION ):
                {
                    const MetaTextArrayAction* pA = (const MetaTextArrayAction*) pAction;
                    m_rOuterFace.DrawTextArray( pA->GetPoint(), pA->GetText(), pA->GetDXArray(), pA->GetIndex(), pA->GetLen() );
                }
                break;

                case( META_STRETCHTEXT_ACTION ):
                {
                    const MetaStretchTextAction* pA = (const MetaStretchTextAction*) pAction;
                    m_rOuterFace.DrawStretchText( pA->GetPoint(), pA->GetWidth(), pA->GetText(), pA->GetIndex(), pA->GetLen() );
                }
                break;


                case( META_TEXTLINE_ACTION ):
                {
                    const MetaTextLineAction* pA = (const MetaTextLineAction*) pAction;
                    m_rOuterFace.DrawTextLine( pA->GetStartPoint(), pA->GetWidth(), pA->GetStrikeout(), pA->GetUnderline(), pA->GetOverline() );

                }
                break;

                case( META_CLIPREGION_ACTION ):
                {
                    const MetaClipRegionAction* pA = (const MetaClipRegionAction*) pAction;

                    if( pA->IsClipping() )
                    {
                        if( pA->GetRegion().IsEmpty() )
                            m_rOuterFace.SetClipRegion( basegfx::B2DPolyPolygon() );
                        else
                        {
                            Region aReg( pA->GetRegion() );
                            m_rOuterFace.SetClipRegion( aReg.GetAsB2DPolyPolygon() );
                        }
                    }
                    else
                        m_rOuterFace.SetClipRegion();
                }
                break;

                case( META_ISECTRECTCLIPREGION_ACTION ):
                {
                    const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pAction;
                    m_rOuterFace.IntersectClipRegion( pA->GetRect() );
                }
                break;

                case( META_ISECTREGIONCLIPREGION_ACTION ):
                {
                    const MetaISectRegionClipRegionAction* pA = (const MetaISectRegionClipRegionAction*) pAction;
                    Region aReg( pA->GetRegion() );
                    m_rOuterFace.IntersectClipRegion( aReg.GetAsB2DPolyPolygon() );
                }
                break;

                case( META_MOVECLIPREGION_ACTION ):
                {
                    const MetaMoveClipRegionAction* pA = (const MetaMoveClipRegionAction*) pAction;
                    m_rOuterFace.MoveClipRegion( pA->GetHorzMove(), pA->GetVertMove() );
                }
                break;

                case( META_MAPMODE_ACTION ):
                {
                    const_cast< MetaAction* >( pAction )->Execute( pDummyVDev );
                    m_rOuterFace.SetMapMode( pDummyVDev->GetMapMode() );
                }
                break;

                case( META_LINECOLOR_ACTION ):
                {
                    const MetaLineColorAction* pA = (const MetaLineColorAction*) pAction;

                    if( pA->IsSetting() )
                        m_rOuterFace.SetLineColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetLineColor();
                }
                break;

                case( META_FILLCOLOR_ACTION ):
                {
                    const MetaFillColorAction* pA = (const MetaFillColorAction*) pAction;

                    if( pA->IsSetting() )
                        m_rOuterFace.SetFillColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetFillColor();
                }
                break;

                case( META_TEXTLINECOLOR_ACTION ):
                {
                    const MetaTextLineColorAction* pA = (const MetaTextLineColorAction*) pAction;

                    if( pA->IsSetting() )
                        m_rOuterFace.SetTextLineColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetTextLineColor();
                }
                break;

                case( META_OVERLINECOLOR_ACTION ):
                {
                    const MetaOverlineColorAction* pA = (const MetaOverlineColorAction*) pAction;

                    if( pA->IsSetting() )
                        m_rOuterFace.SetOverlineColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetOverlineColor();
                }
                break;

                case( META_TEXTFILLCOLOR_ACTION ):
                {
                    const MetaTextFillColorAction* pA = (const MetaTextFillColorAction*) pAction;

                    if( pA->IsSetting() )
                        m_rOuterFace.SetTextFillColor( pA->GetColor() );
                    else
                        m_rOuterFace.SetTextFillColor();
                }
                break;

                case( META_TEXTCOLOR_ACTION ):
                {
                    const MetaTextColorAction* pA = (const MetaTextColorAction*) pAction;
                    m_rOuterFace.SetTextColor( pA->GetColor() );
                }
                break;

                case( META_TEXTALIGN_ACTION ):
                {
                    const MetaTextAlignAction* pA = (const MetaTextAlignAction*) pAction;
                    m_rOuterFace.SetTextAlign( pA->GetTextAlign() );
                }
                break;

                case( META_FONT_ACTION ):
                {
                    const MetaFontAction* pA = (const MetaFontAction*) pAction;
                    m_rOuterFace.SetFont( pA->GetFont() );
                }
                break;

                case( META_PUSH_ACTION ):
                {
                    const MetaPushAction* pA = (const MetaPushAction*) pAction;

                    pDummyVDev->Push( pA->GetFlags() );
                    m_rOuterFace.Push( pA->GetFlags() );
                }
                break;

                case( META_POP_ACTION ):
                {
                    pDummyVDev->Pop();
                    m_rOuterFace.Pop();
                }
                break;

                case( META_LAYOUTMODE_ACTION ):
                {
                    const MetaLayoutModeAction* pA = (const MetaLayoutModeAction*) pAction;
                    m_rOuterFace.SetLayoutMode( pA->GetLayoutMode() );
                }
                break;

                case META_TEXTLANGUAGE_ACTION:
                {
                    const  MetaTextLanguageAction* pA = (const MetaTextLanguageAction*) pAction;
                    m_rOuterFace.SetDigitLanguage( pA->GetTextLanguage() );
                }
                break;

                case( META_WALLPAPER_ACTION ):
                {
                    const MetaWallpaperAction* pA = (const MetaWallpaperAction*) pAction;
                    m_rOuterFace.DrawWallpaper( pA->GetRect(), pA->GetWallpaper() );
                }
                break;

                case( META_RASTEROP_ACTION ):
                {
                    // !!! >>> we don't want to support this actions
                }
                break;

                case( META_REFPOINT_ACTION ):
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
                        OSL_TRACE( "PDFExport::ImplWriteActions: deprecated and unsupported MetaAction encountered" );
                    }
                break;
            }
            i++;
        }
    }

    delete pPrivateDevice;
}

// Encryption methods

/* a crutch to transport an rtlDigest safely though UNO API
   this is needed for the PDF export dialog, which otherwise would have to pass
   clear text passwords down till they can be used in PDFWriter. Unfortunately
   the MD5 sum of the password (which is needed to create the PDF encryption key)
   is not sufficient, since an rtl MD5 digest cannot be created in an arbitrary state
   which would be needed in PDFWriterImpl::computeEncryptionKey.
*/
class EncHashTransporter : public cppu::WeakImplHelper1 < com::sun::star::beans::XMaterialHolder >
{
    rtlDigest                   maUDigest;
    sal_IntPtr                  maID;
    std::vector< sal_uInt8 >    maOValue;

    static std::map< sal_IntPtr, EncHashTransporter* >      sTransporters;
public:
    EncHashTransporter()
    : maUDigest( rtl_digest_createMD5() )
    {
        maID = reinterpret_cast< sal_IntPtr >(this);
        while( sTransporters.find( maID ) != sTransporters.end() ) // paranoia mode
            maID++;
        sTransporters[ maID ] = this;
    }

    virtual ~EncHashTransporter()
    {
        sTransporters.erase( maID );
        if( maUDigest )
            rtl_digest_destroyMD5( maUDigest );
        OSL_TRACE( "EncHashTransporter freed" );
    }

    rtlDigest getUDigest() const { return maUDigest; };
    std::vector< sal_uInt8 >& getOValue() { return maOValue; }
    void invalidate()
    {
        if( maUDigest )
        {
            rtl_digest_destroyMD5( maUDigest );
            maUDigest = NULL;
        }
    }

    // XMaterialHolder
    virtual uno::Any SAL_CALL getMaterial() throw()
    {
        return uno::makeAny( sal_Int64(maID) );
    }

    static EncHashTransporter* getEncHashTransporter( const uno::Reference< beans::XMaterialHolder >& );

};

std::map< sal_IntPtr, EncHashTransporter* > EncHashTransporter::sTransporters;

EncHashTransporter* EncHashTransporter::getEncHashTransporter( const uno::Reference< beans::XMaterialHolder >& xRef )
{
    EncHashTransporter* pResult = NULL;
    if( xRef.is() )
    {
        uno::Any aMat( xRef->getMaterial() );
        sal_Int64 nMat = 0;
        if( aMat >>= nMat )
        {
            std::map< sal_IntPtr, EncHashTransporter* >::iterator it = sTransporters.find( static_cast<sal_IntPtr>(nMat) );
            if( it != sTransporters.end() )
                pResult = it->second;
        }
    }
    return pResult;
}

bool PDFWriterImpl::checkEncryptionBufferSize( register sal_Int32 newSize )
{
    if( m_nEncryptionBufferSize < newSize )
    {
        /* reallocate the buffer, the used function allocate as rtl_allocateMemory
        if the pointer parameter is NULL */
        m_pEncryptionBuffer = (sal_uInt8*)rtl_reallocateMemory( m_pEncryptionBuffer, newSize );
        if( m_pEncryptionBuffer )
            m_nEncryptionBufferSize = newSize;
        else
            m_nEncryptionBufferSize = 0;
    }
    return ( m_nEncryptionBufferSize != 0 );
}

void PDFWriterImpl::checkAndEnableStreamEncryption( register sal_Int32 nObject )
{
    if( m_aContext.Encryption.Encrypt() )
    {
        m_bEncryptThisStream = true;
        sal_Int32 i = m_nKeyLength;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)nObject;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 8 );
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 16 );
        //the other location of m_nEncryptionKey are already set to 0, our fixed generation number
        // do the MD5 hash
        sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
        // the i+2 to take into account the generation number, always zero
        rtl_digest_MD5( &m_aContext.Encryption.EncryptionKey[0], i+2, nMD5Sum, sizeof(nMD5Sum) );
        // initialize the RC4 with the key
        // key legth: see algoritm 3.1, step 4: (N+5) max 16
        rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum, m_nRC4KeyLength, NULL, 0 );
    }
}

void PDFWriterImpl::enableStringEncryption( register sal_Int32 nObject )
{
    if( m_aContext.Encryption.Encrypt() )
    {
        sal_Int32 i = m_nKeyLength;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)nObject;
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 8 );
        m_aContext.Encryption.EncryptionKey[i++] = (sal_uInt8)( nObject >> 16 );
        //the other location of m_nEncryptionKey are already set to 0, our fixed generation number
        // do the MD5 hash
        sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
        // the i+2 to take into account the generation number, always zero
        rtl_digest_MD5( &m_aContext.Encryption.EncryptionKey[0], i+2, nMD5Sum, sizeof(nMD5Sum) );
        // initialize the RC4 with the key
        // key legth: see algoritm 3.1, step 4: (N+5) max 16
        rtl_cipher_initARCFOUR( m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum, m_nRC4KeyLength, NULL, 0 );
    }
}

/* init the encryption engine
1. init the document id, used both for building the document id and for building the encryption key(s)
2. build the encryption key following algorithms described in the PDF specification
 */
uno::Reference< beans::XMaterialHolder > PDFWriterImpl::initEncryption( const OUString& i_rOwnerPassword,
                                                                        const OUString& i_rUserPassword,
                                                                        bool b128Bit
                                                                        )
{
    uno::Reference< beans::XMaterialHolder > xResult;
    if( !i_rOwnerPassword.isEmpty() || !i_rUserPassword.isEmpty() )
    {
        EncHashTransporter* pTransporter = new EncHashTransporter;
        xResult = pTransporter;

        // get padded passwords
        sal_uInt8 aPadUPW[ENCRYPTED_PWD_SIZE], aPadOPW[ENCRYPTED_PWD_SIZE];
        padPassword( i_rOwnerPassword.isEmpty() ? i_rUserPassword : i_rOwnerPassword, aPadOPW );
        padPassword( i_rUserPassword, aPadUPW );
        sal_Int32 nKeyLength = SECUR_40BIT_KEY;
        if( b128Bit )
            nKeyLength = SECUR_128BIT_KEY;

        if( computeODictionaryValue( aPadOPW, aPadUPW, pTransporter->getOValue(), nKeyLength ) )
        {
            rtlDigest aDig = pTransporter->getUDigest();
            if( rtl_digest_updateMD5( aDig, aPadUPW, ENCRYPTED_PWD_SIZE ) != rtl_Digest_E_None )
                xResult.clear();
        }
        else
            xResult.clear();

        // trash temporary padded cleartext PWDs
        memset( aPadOPW, 0, sizeof(aPadOPW) );
        memset( aPadUPW, 0, sizeof(aPadUPW) );

    }
    return xResult;
}

bool PDFWriterImpl::prepareEncryption( const uno::Reference< beans::XMaterialHolder >& xEnc )
{
    bool bSuccess = false;
    EncHashTransporter* pTransporter = EncHashTransporter::getEncHashTransporter( xEnc );
    if( pTransporter )
    {
        sal_Int32 nKeyLength = 0, nRC4KeyLength = 0;
        sal_Int32 nAccessPermissions = computeAccessPermissions( m_aContext.Encryption, nKeyLength, nRC4KeyLength );
        m_aContext.Encryption.OValue = pTransporter->getOValue();
        bSuccess = computeUDictionaryValue( pTransporter, m_aContext.Encryption, nKeyLength, nAccessPermissions );
    }
    if( ! bSuccess )
    {
        m_aContext.Encryption.OValue.clear();
        m_aContext.Encryption.UValue.clear();
        m_aContext.Encryption.EncryptionKey.clear();
    }
    return bSuccess;
}

sal_Int32 PDFWriterImpl::computeAccessPermissions( const vcl::PDFWriter::PDFEncryptionProperties& i_rProperties,
                                                   sal_Int32& o_rKeyLength, sal_Int32& o_rRC4KeyLength )
{
    /*
    2) compute the access permissions, in numerical form

    the default value depends on the revision 2 (40 bit) or 3 (128 bit security):
    - for 40 bit security the unused bit must be set to 1, since they are not used
    - for 128 bit security the same bit must be preset to 0 and set later if needed
    according to the table 3.15, pdf v 1.4 */
    sal_Int32 nAccessPermissions = ( i_rProperties.Security128bit ) ? 0xfffff0c0 : 0xffffffc0 ;

    /* check permissions for 40 bit security case */
    nAccessPermissions |= ( i_rProperties.CanPrintTheDocument ) ?  1 << 2 : 0;
    nAccessPermissions |= ( i_rProperties.CanModifyTheContent ) ? 1 << 3 : 0;
    nAccessPermissions |= ( i_rProperties.CanCopyOrExtract ) ?   1 << 4 : 0;
    nAccessPermissions |= ( i_rProperties.CanAddOrModify ) ? 1 << 5 : 0;
    o_rKeyLength = SECUR_40BIT_KEY;
    o_rRC4KeyLength = SECUR_40BIT_KEY+5; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 5

    if( i_rProperties.Security128bit )
    {
        o_rKeyLength = SECUR_128BIT_KEY;
        o_rRC4KeyLength = 16; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 16, thus maximum
        // permitted value is 16
        nAccessPermissions |= ( i_rProperties.CanFillInteractive ) ?         1 << 8 : 0;
        nAccessPermissions |= ( i_rProperties.CanExtractForAccessibility ) ? 1 << 9 : 0;
        nAccessPermissions |= ( i_rProperties.CanAssemble ) ?                1 << 10 : 0;
        nAccessPermissions |= ( i_rProperties.CanPrintFull ) ?               1 << 11 : 0;
    }
    return nAccessPermissions;
}

/*************************************************************
begin i12626 methods

Implements Algorithm 3.2, step 1 only
*/
void PDFWriterImpl::padPassword( const OUString& i_rPassword, sal_uInt8* o_pPaddedPW )
{
    // get ansi-1252 version of the password string CHECKIT ! i12626
    OString aString( OUStringToOString( i_rPassword, RTL_TEXTENCODING_MS_1252 ) );

    //copy the string to the target
    sal_Int32 nToCopy = ( aString.getLength() < ENCRYPTED_PWD_SIZE ) ? aString.getLength() : ENCRYPTED_PWD_SIZE;
    sal_Int32 nCurrentChar;

    for( nCurrentChar = 0; nCurrentChar < nToCopy; nCurrentChar++ )
        o_pPaddedPW[nCurrentChar] = (sal_uInt8)( aString.getStr()[nCurrentChar] );

    //pad it with standard byte string
    sal_Int32 i,y;
    for( i = nCurrentChar, y = 0 ; i < ENCRYPTED_PWD_SIZE; i++, y++ )
        o_pPaddedPW[i] = s_nPadString[y];
}

/**********************************
Algorithm 3.2  Compute the encryption key used

step 1 should already be done before calling, the paThePaddedPassword parameter should contain
the padded password and must be 32 byte long, the encryption key is returned into the paEncryptionKey parameter,
it will be 16 byte long for 128 bit security; for 40 bit security only the first 5 bytes are used

TODO: in pdf ver 1.5 and 1.6 the step 6 is different, should be implemented. See spec.

*/
bool PDFWriterImpl::computeEncryptionKey( EncHashTransporter* i_pTransporter, vcl::PDFWriter::PDFEncryptionProperties& io_rProperties, sal_Int32 i_nAccessPermissions )
{
    bool bSuccess = true;
    sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];

    // transporter contains an MD5 digest with the padded user password already
    rtlDigest aDigest = i_pTransporter->getUDigest();
    rtlDigestError nError = rtl_Digest_E_None;
    if( aDigest )
    {
        //step 3
        if( ! io_rProperties.OValue.empty() )
            nError = rtl_digest_updateMD5( aDigest, &io_rProperties.OValue[0] , sal_Int32(io_rProperties.OValue.size()) );
        else
            bSuccess = false;
        //Step 4
        sal_uInt8 nPerm[4];

        nPerm[0] = (sal_uInt8)i_nAccessPermissions;
        nPerm[1] = (sal_uInt8)( i_nAccessPermissions >> 8 );
        nPerm[2] = (sal_uInt8)( i_nAccessPermissions >> 16 );
        nPerm[3] = (sal_uInt8)( i_nAccessPermissions >> 24 );

        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( aDigest, nPerm , sizeof( nPerm ) );

        //step 5, get the document ID, binary form
        if( nError == rtl_Digest_E_None )
            nError = rtl_digest_updateMD5( aDigest, &io_rProperties.DocumentIdentifier[0], sal_Int32(io_rProperties.DocumentIdentifier.size()) );
        //get the digest
        if( nError == rtl_Digest_E_None )
        {
            rtl_digest_getMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );

            //step 6, only if 128 bit
            if( io_rProperties.Security128bit )
            {
                for( sal_Int32 i = 0; i < 50; i++ )
                {
                    nError = rtl_digest_updateMD5( aDigest, &nMD5Sum, sizeof( nMD5Sum ) );
                    if( nError != rtl_Digest_E_None )
                    {
                        bSuccess =  false;
                        break;
                    }
                    rtl_digest_getMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                }
            }
        }
    }
    else
        bSuccess = false;

    i_pTransporter->invalidate();

    //Step 7
    if( bSuccess )
    {
        io_rProperties.EncryptionKey.resize( MAXIMUM_RC4_KEY_LENGTH );
        for( sal_Int32 i = 0; i < MD5_DIGEST_SIZE; i++ )
            io_rProperties.EncryptionKey[i] = nMD5Sum[i];
    }
    else
        io_rProperties.EncryptionKey.clear();

    return bSuccess;
}

/**********************************
Algorithm 3.3  Compute the encryption dictionary /O value, save into the class data member
the step numbers down here correspond to the ones in PDF v.1.4 specfication
*/
bool PDFWriterImpl::computeODictionaryValue( const sal_uInt8* i_pPaddedOwnerPassword,
                                             const sal_uInt8* i_pPaddedUserPassword,
                                             std::vector< sal_uInt8 >& io_rOValue,
                                             sal_Int32 i_nKeyLength
                                             )
{
    bool bSuccess = true;

    io_rOValue.resize( ENCRYPTED_PWD_SIZE );

    rtlDigest aDigest = rtl_digest_createMD5();
    rtlCipher aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );
    if( aDigest && aCipher)
    {
        //step 1 already done, data is in i_pPaddedOwnerPassword
        //step 2

        rtlDigestError nError = rtl_digest_updateMD5( aDigest, i_pPaddedOwnerPassword, ENCRYPTED_PWD_SIZE );
        if( nError == rtl_Digest_E_None )
        {
            sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];

            rtl_digest_getMD5( aDigest, nMD5Sum, sizeof(nMD5Sum) );
//step 3, only if 128 bit
            if( i_nKeyLength == SECUR_128BIT_KEY )
            {
                sal_Int32 i;
                for( i = 0; i < 50; i++ )
                {
                    nError = rtl_digest_updateMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                    if( nError != rtl_Digest_E_None )
                    {
                        bSuccess = false;
                        break;
                    }
                    rtl_digest_getMD5( aDigest, nMD5Sum, sizeof( nMD5Sum ) );
                }
            }
            //Step 4, the key is in nMD5Sum
            //step 5 already done, data is in i_pPaddedUserPassword
            //step 6
            rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                     nMD5Sum, i_nKeyLength , NULL, 0 );
            // encrypt the user password using the key set above
            rtl_cipher_encodeARCFOUR( aCipher, i_pPaddedUserPassword, ENCRYPTED_PWD_SIZE, // the data to be encrypted
                                      &io_rOValue[0], sal_Int32(io_rOValue.size()) ); //encrypted data
            //Step 7, only if 128 bit
            if( i_nKeyLength == SECUR_128BIT_KEY )
            {
                sal_uInt32 i, y;
                sal_uInt8 nLocalKey[ SECUR_128BIT_KEY ]; // 16 = 128 bit key

                for( i = 1; i <= 19; i++ ) // do it 19 times, start with 1
                {
                    for( y = 0; y < sizeof( nLocalKey ); y++ )
                        nLocalKey[y] = (sal_uInt8)( nMD5Sum[y] ^ i );

                    rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                            nLocalKey, SECUR_128BIT_KEY, NULL, 0 ); //destination data area, on init can be NULL
                    rtl_cipher_encodeARCFOUR( aCipher, &io_rOValue[0], sal_Int32(io_rOValue.size()), // the data to be encrypted
                                              &io_rOValue[0], sal_Int32(io_rOValue.size()) ); // encrypted data, can be the same as the input, encrypt "in place"
                    //step 8, store in class data member
                }
            }
        }
        else
            bSuccess = false;
    }
    else
        bSuccess = false;

    if( aDigest )
        rtl_digest_destroyMD5( aDigest );
    if( aCipher )
        rtl_cipher_destroyARCFOUR( aCipher );

    if( ! bSuccess )
        io_rOValue.clear();
    return bSuccess;
}

/**********************************
Algorithms 3.4 and 3.5  Compute the encryption dictionary /U value, save into the class data member, revision 2 (40 bit) or 3 (128 bit)
*/
bool PDFWriterImpl::computeUDictionaryValue( EncHashTransporter* i_pTransporter,
                                             vcl::PDFWriter::PDFEncryptionProperties& io_rProperties,
                                             sal_Int32 i_nKeyLength,
                                             sal_Int32 i_nAccessPermissions
                                             )
{
    bool bSuccess = true;

    io_rProperties.UValue.resize( ENCRYPTED_PWD_SIZE );

    rtlDigest aDigest = rtl_digest_createMD5();
    rtlCipher aCipher = rtl_cipher_createARCFOUR( rtl_Cipher_ModeStream );
    if( aDigest && aCipher )
    {
        //step 1, common to both 3.4 and 3.5
        if( computeEncryptionKey( i_pTransporter, io_rProperties, i_nAccessPermissions ) )
        {
            // prepare encryption key for object
            for( sal_Int32 i = i_nKeyLength, y = 0; y < 5 ; y++ )
                io_rProperties.EncryptionKey[i++] = 0;

            if( io_rProperties.Security128bit == false )
            {
                //3.4
                //step 2 and 3
                rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                        &io_rProperties.EncryptionKey[0], 5 , // key and key length
                                        NULL, 0 ); //destination data area
                // encrypt the user password using the key set above, save for later use
                rtl_cipher_encodeARCFOUR( aCipher, s_nPadString, sizeof( s_nPadString ), // the data to be encrypted
                                          &io_rProperties.UValue[0], sal_Int32(io_rProperties.UValue.size()) ); //encrypted data, stored in class data member
            }
            else
            {
                //or 3.5, for 128 bit security
                //step6, initilize the last 16 bytes of the encrypted user password to 0
                for(sal_uInt32 i = MD5_DIGEST_SIZE; i < sal_uInt32(io_rProperties.UValue.size()); i++)
                    io_rProperties.UValue[i] = 0;
                //step 2
                rtlDigestError nError = rtl_digest_updateMD5( aDigest, s_nPadString, sizeof( s_nPadString ) );
                //step 3
                if( nError == rtl_Digest_E_None )
                    nError = rtl_digest_updateMD5( aDigest, &io_rProperties.DocumentIdentifier[0], sal_Int32(io_rProperties.DocumentIdentifier.size()) );
                else
                    bSuccess = false;

                sal_uInt8 nMD5Sum[ RTL_DIGEST_LENGTH_MD5 ];
                rtl_digest_getMD5( aDigest, nMD5Sum, sizeof(nMD5Sum) );
                //Step 4
                rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                        &io_rProperties.EncryptionKey[0], SECUR_128BIT_KEY, NULL, 0 ); //destination data area
                rtl_cipher_encodeARCFOUR( aCipher, nMD5Sum, sizeof( nMD5Sum ), // the data to be encrypted
                                          &io_rProperties.UValue[0], sizeof( nMD5Sum ) ); //encrypted data, stored in class data member
                //step 5
                sal_uInt32 i, y;
                sal_uInt8 nLocalKey[SECUR_128BIT_KEY];

                for( i = 1; i <= 19; i++ ) // do it 19 times, start with 1
                {
                    for( y = 0; y < sizeof( nLocalKey ) ; y++ )
                        nLocalKey[y] = (sal_uInt8)( io_rProperties.EncryptionKey[y] ^ i );

                    rtl_cipher_initARCFOUR( aCipher, rtl_Cipher_DirectionEncode,
                                            nLocalKey, SECUR_128BIT_KEY, // key and key length
                                            NULL, 0 ); //destination data area, on init can be NULL
                    rtl_cipher_encodeARCFOUR( aCipher, &io_rProperties.UValue[0], SECUR_128BIT_KEY, // the data to be encrypted
                                              &io_rProperties.UValue[0], SECUR_128BIT_KEY ); // encrypted data, can be the same as the input, encrypt "in place"
                }
            }
        }
        else
            bSuccess = false;
    }
    else
        bSuccess = false;

    if( aDigest )
        rtl_digest_destroyMD5( aDigest );
    if( aCipher )
        rtl_cipher_destroyARCFOUR( aCipher );

    if( ! bSuccess )
        io_rProperties.UValue.clear();
    return bSuccess;
}

/* end i12626 methods */

static const long unsetRun[256] =
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

static const long setRun[256] =
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

inline bool isSet( const Scanline i_pLine, long i_nIndex )
{
    return (i_pLine[ i_nIndex/8 ] & (0x80 >> (i_nIndex&7))) != 0;
}

long findBitRun( const Scanline i_pLine, long i_nStartIndex, long i_nW, bool i_bSet )
{
    if( i_nStartIndex < 0 )
        return i_nW;

    long nIndex = i_nStartIndex;
    if( nIndex < i_nW )
    {
        const sal_uInt8 * pByte = static_cast<sal_uInt8*>(i_pLine) + (nIndex/8);
        sal_uInt8 nByte = *pByte;

        // run up to byte boundary
        long nBitInByte = (nIndex & 7);
        if( nBitInByte )
        {
            sal_uInt8 nMask = 0x80 >> nBitInByte;
            while( nBitInByte != 8 )
            {
                if( (nByte & nMask) != (i_bSet ? nMask : 0) )
                    return nIndex < i_nW ? nIndex : i_nW;
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
        const long* pRunTable;
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
    return nIndex < i_nW ? nIndex : i_nW;
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

    const sal_uInt8* getByte() const { return &mnBuffer; }
    void flush() { mnNextBitPos = 8; mnBuffer = 0; }
};

void PDFWriterImpl::putG4Bits( sal_uInt32 i_nLength, sal_uInt32 i_nCode, BitStreamState& io_rState )
{
    while( i_nLength > io_rState.mnNextBitPos )
    {
        io_rState.mnBuffer |= static_cast<sal_uInt8>( i_nCode >> (i_nLength - io_rState.mnNextBitPos) );
        i_nLength -= io_rState.mnNextBitPos;
        writeBuffer( io_rState.getByte(), 1 );
        io_rState.flush();
    }
    OSL_ASSERT( i_nLength < 9 );
    static const unsigned int msbmask[9] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
    io_rState.mnBuffer |= static_cast<sal_uInt8>( (i_nCode & msbmask[i_nLength]) << (io_rState.mnNextBitPos - i_nLength) );
    io_rState.mnNextBitPos -= i_nLength;
    if( io_rState.mnNextBitPos == 0 )
    {
        writeBuffer( io_rState.getByte(), 1 );
        io_rState.flush();
    }
}

struct PixelCode
{
    sal_uInt32      mnEncodedPixels;
    sal_uInt32      mnCodeBits;
    sal_uInt32      mnCode;
};

static const PixelCode WhitePixelCodes[] =
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

static const PixelCode BlackPixelCodes[] =
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


void PDFWriterImpl::putG4Span( long i_nSpan, bool i_bWhitePixel, BitStreamState& io_rState )
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

void PDFWriterImpl::writeG4Stream( BitmapReadAccess* i_pBitmap )
{
    long nW = i_pBitmap->Width();
    long nH = i_pBitmap->Height();
    if( nW <= 0 || nH <= 0 )
        return;
    if( i_pBitmap->GetBitCount() != 1 )
        return;

    BitStreamState aBitState;

    // the first reference line is virtual and completely empty
    const Scanline pFirstRefLine = (Scanline)rtl_allocateZeroMemory( nW/8 + 1 );
    Scanline pRefLine = pFirstRefLine;
    for( long nY = 0; nY < nH; nY++ )
    {
        const Scanline pCurLine = i_pBitmap->GetScanline( nY );
        long nLineIndex = 0;
        bool bRunSet = (*pCurLine & 0x80) ? true : false;
        bool bRefSet = (*pRefLine & 0x80) ? true : false;
        long nRunIndex1 = bRunSet ? 0 : findBitRun( pCurLine, 0, nW, bRunSet );
        long nRefIndex1 = bRefSet ? 0 : findBitRun( pRefLine, 0, nW, bRefSet );
        for( ; nLineIndex < nW; )
        {
            long nRefIndex2 = findBitRun( pRefLine, nRefIndex1, nW, isSet( pRefLine, nRefIndex1 ) );
            if( nRefIndex2 >= nRunIndex1 )
            {
                long nDiff = nRefIndex1 - nRunIndex1;
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
                    long nRunIndex2 = findBitRun( pCurLine, nRunIndex1, nW, isSet( pCurLine, nRunIndex1 ) );
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
        writeBuffer( aBitState.getByte(), 1 );
        aBitState.flush();
    }

    rtl_freeMemory( pFirstRefLine );
}

static bool lcl_canUsePDFAxialShading(const Gradient& rGradient) {
    switch (rGradient.GetStyle())
    {
        case GradientStyle_LINEAR:
        case GradientStyle_AXIAL:
            break;
        default:
            return false;
    }

    // TODO: handle step count
    if (rGradient.GetSteps() > 0)
        return false;

    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
