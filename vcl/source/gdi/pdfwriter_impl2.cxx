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

#include "precompiled_vcl.hxx"

#include "pdfwriter_impl.hxx"
#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/virdev.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/metaact.hxx"
#include "vcl/graph.hxx"
#include "vcl/svdata.hxx"
#include "unotools/streamwrap.hxx"
#include "unotools/processfactory.hxx"

#include "comphelper/processfactory.hxx"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/io/XSeekable.hpp"
#include "com/sun/star/graphic/XGraphicProvider.hpp"

using namespace vcl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

// -----------------------------------------------------------------------------

void PDFWriterImpl::implWriteGradient( const PolyPolygon& i_rPolyPoly, const Gradient& i_rGradient,
                                       VirtualDevice* i_pDummyVDev, const vcl::PDFWriter::PlayMetafileContext& i_rContext )
{
    GDIMetaFile aTmpMtf;

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
        ULONG nMirrorFlags(BMP_MIRROR_NONE);
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
            // do downsampling if neccessary
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
                    aBitmapEx.Scale( aNewBmpSize );
                else
                    aBitmapEx.SetEmpty();
            }
        }

        const Size aSizePixel( aBitmapEx.GetSizePixel() );
        if ( aSizePixel.Width() && aSizePixel.Height() )
        {
            sal_Bool bUseJPGCompression = !i_rContext.m_bOnlyLosslessCompression;
            if ( ( aSizePixel.Width() < 32 ) || ( aSizePixel.Height() < 32 ) )
                bUseJPGCompression = sal_False;

            SvMemoryStream  aStrm;
            Bitmap          aMask;

            bool bTrueColorJPG = true;
            if ( bUseJPGCompression )
            {
                sal_uInt32 nZippedFileSize;     // sj: we will calculate the filesize of a zipped bitmap
                {                               // to determine if jpeg compression is usefull
                    SvMemoryStream aTemp;
                    aTemp.SetCompressMode( aTemp.GetCompressMode() | COMPRESSMODE_ZBITMAP );
                    aTemp.SetVersion( SOFFICE_FILEFORMAT_40 );  // sj: up from version 40 our bitmap stream operator
                    aTemp << aBitmapEx;                         // is capable of zlib stream compression
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
                aFilterData[ 0 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Quality" ) );
                aFilterData[ 0 ].Value <<= sal_Int32(i_rContext.m_nJPEGQuality);
                aFilterData[ 1 ].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ColorMode" ) );
                aFilterData[ 1 ].Value <<= nColorMode;

                try
                {
                    uno::Reference < io::XStream > xStream = new utl::OStreamWrapper( aStrm );
                    Reference< io::XSeekable > xSeekable( xStream, UNO_QUERY_THROW );
                    Reference< graphic::XGraphicProvider > xGraphicProvider( ImplGetSVData()->maAppData.mxMSF->createInstance(
                        OUString::createFromAscii( "com.sun.star.graphic.GraphicProvider" ) ), UNO_QUERY );
                    if ( xGraphicProvider.is() )
                    {
                        Reference< graphic::XGraphic > xGraphic( aGraphic.GetXGraphic() );
                        Reference < io::XOutputStream > xOut( xStream->getOutputStream() );
                        rtl::OUString aMimeType( ::rtl::OUString::createFromAscii( "image/jpeg" ) );
                        uno::Sequence< beans::PropertyValue > aOutMediaProperties( 3 );
                        aOutMediaProperties[0].Name = ::rtl::OUString::createFromAscii( "OutputStream" );
                        aOutMediaProperties[0].Value <<= xOut;
                        aOutMediaProperties[1].Name = ::rtl::OUString::createFromAscii( "MimeType" );
                        aOutMediaProperties[1].Value <<= aMimeType;
                        aOutMediaProperties[2].Name = ::rtl::OUString::createFromAscii( "FilterData" );
                        aOutMediaProperties[2].Value <<= aFilterData;
                        xGraphicProvider->storeGraphic( xGraphic, aOutMediaProperties );
                        xOut->flush();
                        if ( xSeekable->getLength() > nZippedFileSize )
                        {
                            bUseJPGCompression = sal_False;
                        }
                        else
                        {
                            aStrm.Seek( STREAM_SEEK_TO_END );

                            xSeekable->seek( 0 );
                            Sequence< PropertyValue > aArgs( 1 );
                            aArgs[ 0 ].Name = ::rtl::OUString::createFromAscii( "InputStream" );
                            aArgs[ 0 ].Value <<= xStream;
                            Reference< XPropertySet > xPropSet( xGraphicProvider->queryGraphicDescriptor( aArgs ) );
                            if ( xPropSet.is() )
                            {
                                sal_Int16 nBitsPerPixel = 24;
                                if ( xPropSet->getPropertyValue( ::rtl::OUString::createFromAscii( "BitsPerPixel" ) ) >>= nBitsPerPixel )
                                {
                                    bTrueColorJPG = nBitsPerPixel != 8;
                                }
                            }
                        }
                    }
                    else
                        bUseJPGCompression = sal_False;
                }
                catch( uno::Exception& )
                {
                    bUseJPGCompression = sal_False;
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

    for( sal_uInt32 i = 0, nCount = aMtf.GetActionCount(); i < nCount; )
    {
        if ( !i_pOutDevData || !i_pOutDevData->PlaySyncPageAct( m_rOuterFace, i ) )
        {
            const MetaAction*   pAction = aMtf.GetAction( i );
            const USHORT        nType = pAction->GetType();

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
                    const PolyPolygon         aPolyPoly( pA->GetRect() );

                    implWriteGradient( aPolyPoly, pA->GetGradient(), pDummyVDev, i_rContext );
                }
                break;

                case( META_GRADIENTEX_ACTION ):
                {
                    const MetaGradientExAction* pA = (const MetaGradientExAction*) pAction;
                    implWriteGradient( pA->GetPolyPolygon(), pA->GetGradient(), pDummyVDev, i_rContext );
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
                        const USHORT nTransPercent = aTransCol.GetLuminance() * 100 / 255;
                        m_rOuterFace.BeginTransparencyGroup();
                        playMetafile( aTmpMtf, NULL, i_rContext, pDummyVDev );
                        m_rOuterFace.EndTransparencyGroup( Rectangle( rPos, rSize ), nTransPercent );
                    }
                    else
                    {
                        const Size  aDstSizeTwip( pDummyVDev->PixelToLogic( pDummyVDev->LogicToPixel( rSize ), MAP_TWIP ) );
                        sal_Int32   nMaxBmpDPI = i_rContext.m_bOnlyLosslessCompression ? 300 : 72;
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

                                pVDev->EnableMapMode( FALSE );
                                aPaint = pVDev->GetBitmap( aPoint, aDstSizePixel );
                                pVDev->EnableMapMode( TRUE );

                                // create mask bitmap
                                pVDev->SetLineColor( COL_BLACK );
                                pVDev->SetFillColor( COL_BLACK );
                                pVDev->DrawRect( Rectangle( aPoint, aDstSize ) );
                                pVDev->SetDrawMode( DRAWMODE_WHITELINE | DRAWMODE_WHITEFILL | DRAWMODE_WHITETEXT |
                                                    DRAWMODE_WHITEBITMAP | DRAWMODE_WHITEGRADIENT );
                                aTmpMtf.WindStart();
                                aTmpMtf.Play( pVDev, aPoint, aDstSize );
                                aTmpMtf.WindStart();
                                pVDev->EnableMapMode( FALSE );
                                aMask = pVDev->GetBitmap( aPoint, aDstSizePixel );
                                pVDev->EnableMapMode( TRUE );

                                // create alpha mask from gradient
                                pVDev->SetDrawMode( DRAWMODE_GRAYGRADIENT );
                                pVDev->DrawGradient( Rectangle( aPoint, aDstSize ), rTransparenceGradient );
                                pVDev->SetDrawMode( DRAWMODE_DEFAULT );
                                pVDev->EnableMapMode( FALSE );
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
                    String                      aSkipComment;

                    if( pA->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
                    {
                        const MetaGradientExAction* pGradAction = NULL;
                        sal_Bool                    bDone = sal_False;

                        while( !bDone && ( ++i < nCount ) )
                        {
                            pAction = aMtf.GetAction( i );

                            if( pAction->GetType() == META_GRADIENTEX_ACTION )
                                pGradAction = (const MetaGradientExAction*) pAction;
                            else if( ( pAction->GetType() == META_COMMENT_ACTION ) &&
                                    ( ( (const MetaCommentAction*) pAction )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) == COMPARE_EQUAL ) )
                            {
                                bDone = sal_True;
                            }
                        }

                        if( pGradAction )
                            implWriteGradient( pGradAction->GetPolyPolygon(), pGradAction->GetGradient(), pDummyVDev, i_rContext );
                    }
                    else
                    {
                        const BYTE* pData = pA->GetData();
                        if ( pData )
                        {
                            SvMemoryStream  aMemStm( (void*)pData, pA->GetDataSize(), STREAM_READ );
                            sal_Bool        bSkipSequence = sal_False;
                            ByteString      sSeqEnd;

                            if( pA->GetComment().Equals( "XPATHSTROKE_SEQ_BEGIN" ) )
                            {
                                sSeqEnd = ByteString( "XPATHSTROKE_SEQ_END" );
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

                                bSkipSequence = sal_True;
                                if ( aStartArrow.Count() || aEndArrow.Count() )
                                    bSkipSequence = sal_False;
                                if ( aDashArray.size() && ( fStrokeWidth != 0.0 ) && ( fTransparency == 0.0 ) )
                                    bSkipSequence = sal_False;
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
                            else if ( pA->GetComment().Equals( "XPATHFILL_SEQ_BEGIN" ) )
                            {
                                sSeqEnd = ByteString( "XPATHFILL_SEQ_END" );
                                SvtGraphicFill aFill;
                                aMemStm >> aFill;

                                if ( ( aFill.getFillType() == SvtGraphicFill::fillSolid ) && ( aFill.getFillRule() == SvtGraphicFill::fillEvenOdd ) )
                                {
                                    double fTransparency = aFill.getTransparency();
                                    if ( fTransparency == 0.0 )
                                    {
                                        PolyPolygon aPath;
                                        aFill.getPath( aPath );

                                        bSkipSequence = sal_True;
                                        m_rOuterFace.DrawPolyPolygon( aPath );
                                    }
                                    else if ( fTransparency == 1.0 )
                                        bSkipSequence = sal_True;
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

                                    bSkipSequence = sal_True;
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
                                        ByteString sComment( ((MetaCommentAction*)pAction)->GetComment() );
                                        if ( sComment.Equals( sSeqEnd ) )
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
                    DBG_ERROR( "MetaMask...Action not supported yet" );
                }
                break;

                case( META_TEXT_ACTION ):
                {
                    const MetaTextAction* pA = (const MetaTextAction*) pAction;
                    m_rOuterFace.DrawText( pA->GetPoint(), String( pA->GetText(), pA->GetIndex(), pA->GetLen() ) );
                }
                break;

                case( META_TEXTRECT_ACTION ):
                {
                    const MetaTextRectAction* pA = (const MetaTextRectAction*) pAction;
                    m_rOuterFace.DrawText( pA->GetRect(), String( pA->GetText() ), pA->GetStyle() );
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
                            m_rOuterFace.SetClipRegion( aReg.ConvertToB2DPolyPolygon() );
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
                    m_rOuterFace.IntersectClipRegion( aReg.ConvertToB2DPolyPolygon() );
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
                        DBG_ERROR( "PDFExport::ImplWriteActions: deprecated and unsupported MetaAction encountered" );
                    }
                break;
            }
            i++;
        }
    }

    delete pPrivateDevice;
}


