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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <sot/factory.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/bmpacc.hxx>
#include <tools/poly.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/solar.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include "svx/xoutbmp.hxx"
#include <svtools/FilterConfigItem.hxx>
#include <svtools/filter.hxx>

// -----------
// - Defines -
// -----------

#define FORMAT_BMP  String(RTL_CONSTASCII_USTRINGPARAM("bmp"))
#define FORMAT_GIF  String(RTL_CONSTASCII_USTRINGPARAM("gif"))
#define FORMAT_JPG  String(RTL_CONSTASCII_USTRINGPARAM("jpg"))
#define FORMAT_PNG  String(RTL_CONSTASCII_USTRINGPARAM("png"))

// --------------
// - XOutBitmap -
// --------------

GraphicFilter* XOutBitmap::pGrfFilter = NULL;

// -----------------------------------------------------------------------------

BitmapEx XOutBitmap::CreateQuickDrawBitmapEx( const Graphic& rGraphic, const OutputDevice& rCompDev,
                                              const MapMode& rMapMode, const Size& rLogSize,
                                              const Point& rPoint, const Size& rSize )
{
    BitmapEx aRetBmp;

    if( rGraphic.IsAlpha() )
        aRetBmp = rGraphic.GetBitmapEx();
    else
    {
        VirtualDevice   aVDev( rCompDev );
        MapMode         aMap( rMapMode );

        aMap.SetOrigin( Point() );
        aVDev.SetMapMode( aMap );

        Point   aPoint( aVDev.LogicToPixel( rPoint ) );
        Size    aOldSize( aVDev.LogicToPixel( rSize ) );
        Size    aAbsSize( aOldSize );
        Size    aQSizePix( aVDev.LogicToPixel( rLogSize ) );

        aVDev.SetMapMode( MapMode() );

        if( aOldSize.Width() < 0 )
            aAbsSize.Width() = -aAbsSize.Width();

        if( aOldSize.Height() < 0 )
            aAbsSize.Height() = -aAbsSize.Height();

        if( aVDev.SetOutputSizePixel( aAbsSize ) )
        {
            Point       aNewOrg( -aPoint.X(), -aPoint.Y() );
            const Point aNullPoint;

            // horizontale Spiegelung ggf. beruecksichtigen
            if( aOldSize.Width() < 0 )
            {
                aNewOrg.X() -= aOldSize.Width();

                // und jetzt noch einen abziehen
                aNewOrg.X()--;
            }

            // vertikale Spiegelung ggf. beruecksichtigen
            if( rSize.Height() < 0 )
            {
                aNewOrg.Y() -= aOldSize.Height();

                // und jetzt noch einen abziehen
                aNewOrg.Y()--;
            }

            if( rGraphic.GetType() != GRAPHIC_BITMAP )
            {
                rGraphic.Draw( &aVDev, aNewOrg, aQSizePix );

                const Bitmap    aBmp( aVDev.GetBitmap( aNullPoint, aAbsSize ) );
                Bitmap          aMask;

                Graphic( rGraphic.GetGDIMetaFile().GetMonochromeMtf( COL_BLACK ) ).Draw( &aVDev, aNewOrg, aQSizePix );
                aMask = aVDev.GetBitmap( aNullPoint, aAbsSize );
                aRetBmp = BitmapEx( aBmp, aMask );
            }
            else
            {
                Bitmap  aBmp( rGraphic.GetBitmap() );

// UNX has got problems with 1x1 bitmaps which are transparent (KA 02.11.1998)
#ifdef UNX
                const Size  aBmpSize( aBmp.GetSizePixel() );
                sal_Bool        bFullTrans = sal_False;

                if( aBmpSize.Width() == 1 && aBmpSize.Height() == 1 && rGraphic.IsTransparent() )
                {
                    Bitmap              aTrans( rGraphic.GetBitmapEx().GetMask() );
                    BitmapReadAccess*   pMAcc = aBmp.AcquireReadAccess();

                    if( pMAcc )
                    {
                        if( pMAcc->GetColor( 0, 0 ) == BitmapColor( Color( COL_WHITE ) ) )
                            bFullTrans = sal_True;

                        aTrans.ReleaseAccess( pMAcc );
                    }
                }

                if( !bFullTrans )
#endif // UNX

                {
                    DitherBitmap( aBmp );
                    aVDev.DrawBitmap( aNewOrg, aQSizePix, aBmp );
                    aBmp = aVDev.GetBitmap( aNullPoint, aAbsSize );

                    if( !rGraphic.IsTransparent() )
                        aRetBmp = BitmapEx( aBmp );
                    else
                    {
                        Bitmap  aTrans( rGraphic.GetBitmapEx().GetMask() );

                        if( !aTrans )
                            aRetBmp = BitmapEx( aBmp, rGraphic.GetBitmapEx().GetTransparentColor() );
                        else
                        {
                            aVDev.DrawBitmap( aNewOrg, aQSizePix, aTrans );
                            aRetBmp = BitmapEx( aBmp, aVDev.GetBitmap( Point(), aAbsSize ) );
                        }
                    }
                }
            }
        }
    }

    return aRetBmp;
}

// ------------------------------------------------------------------------

void XOutBitmap::DrawQuickDrawBitmapEx( OutputDevice* pOutDev, const Point& rPt,
                                        const Size& rSize, const BitmapEx& rBmpEx )
{
    const Size      aBmpSizePix( rBmpEx.GetSizePixel() );
    const Size      aSizePix( pOutDev->LogicToPixel( rSize ) );

    if ( ( aSizePix.Width() - aBmpSizePix.Width() ) || ( aSizePix.Height() - aBmpSizePix.Height() ) )
        rBmpEx.Draw( pOutDev, rPt, rSize );
    else
        rBmpEx.Draw( pOutDev, rPt );
}

// ------------------------------------------------------------------------

void XOutBitmap::DrawTiledBitmapEx( OutputDevice* pOutDev,
                                    const Point& rStartPt, const Size& rGrfSize,
                                    const Rectangle& rTileRect, const BitmapEx& rBmpEx )
{
    Rectangle       aClipRect( pOutDev->LogicToPixel( pOutDev->GetClipRegion().GetBoundRect() ) );
    Rectangle       aPixRect( pOutDev->LogicToPixel( rTileRect ) );
    const Size      aPixSize( pOutDev->LogicToPixel( rGrfSize ) );
    const Point     aPixPoint( pOutDev->LogicToPixel( rStartPt ) );
    Point           aOrg;
    const long      nWidth = aPixSize.Width();
    const long      nHeight = aPixSize.Height();
    long            nXPos = aPixPoint.X() + ( ( aPixRect.Left() - aPixPoint.X() ) / nWidth ) * nWidth;
    long            nYPos = aPixPoint.Y() + ( ( aPixRect.Top() - aPixPoint.Y() ) / nHeight ) * nHeight;
    const long      nBottom = aPixRect.Bottom();
    const long      nRight = aPixRect.Right();
    const long      nLeft = nXPos;
    const sal_Bool      bNoSize = ( aPixSize == rBmpEx.GetSizePixel() );

    pOutDev->Push();
    pOutDev->SetMapMode( MapMode() );

    // ggf. neue ClipRegion berechnen und setzen
    if ( pOutDev->IsClipRegion() )
        aPixRect.Intersection( aClipRect );

    pOutDev->SetClipRegion( aPixRect );

    while( nYPos <= nBottom )
    {
        while( nXPos <= nRight )
        {
            if ( bNoSize )
                rBmpEx.Draw( pOutDev, Point( nXPos, nYPos ) );
            else
                rBmpEx.Draw( pOutDev, Point( nXPos, nYPos ), aPixSize );

            nXPos += nWidth;
        }

        nXPos = nLeft;
        nYPos += nHeight;
    }

    pOutDev->Pop();
}

// ------------------------------------------------------------------------

Animation XOutBitmap::MirrorAnimation( const Animation& rAnimation, sal_Bool bHMirr, sal_Bool bVMirr )
{
    Animation aNewAnim( rAnimation );

    if( bHMirr || bVMirr )
    {
        const Size& rGlobalSize = aNewAnim.GetDisplaySizePixel();
        sal_uIntPtr     nMirrorFlags = 0L;

        if( bHMirr )
            nMirrorFlags |= BMP_MIRROR_HORZ;

        if( bVMirr )
            nMirrorFlags |= BMP_MIRROR_VERT;

        for( sal_uInt16 i = 0, nCount = aNewAnim.Count(); i < nCount; i++ )
        {
            AnimationBitmap aAnimBmp( aNewAnim.Get( i ) );

            // BitmapEx spiegeln
            aAnimBmp.aBmpEx.Mirror( nMirrorFlags );

            // Die Positionen innerhalb der Gesamtbitmap
            // muessen natuerlich auch angepasst werden
            if( bHMirr )
                aAnimBmp.aPosPix.X() = rGlobalSize.Width() - aAnimBmp.aPosPix.X() -
                                       aAnimBmp.aSizePix.Width();

            if( bVMirr )
                aAnimBmp.aPosPix.Y() = rGlobalSize.Height() - aAnimBmp.aPosPix.Y() -
                                       aAnimBmp.aSizePix.Height();

            aNewAnim.Replace( aAnimBmp, i );
        }
    }

    return aNewAnim;
}

// ------------------------------------------------------------------------

Graphic XOutBitmap::MirrorGraphic( const Graphic& rGraphic, const sal_uIntPtr nMirrorFlags )
{
    Graphic aRetGraphic;

    if( nMirrorFlags )
    {
        if( rGraphic.IsAnimated() )
        {
            aRetGraphic = MirrorAnimation( rGraphic.GetAnimation(),
                                           ( nMirrorFlags & BMP_MIRROR_HORZ ) == BMP_MIRROR_HORZ,
                                           ( nMirrorFlags & BMP_MIRROR_VERT ) == BMP_MIRROR_VERT );
        }
        else
        {
            if( rGraphic.IsTransparent() )
            {
                BitmapEx aBmpEx( rGraphic.GetBitmapEx() );

                aBmpEx.Mirror( nMirrorFlags );
                aRetGraphic = aBmpEx;
            }
            else
            {
                Bitmap aBmp( rGraphic.GetBitmap() );

                aBmp.Mirror( nMirrorFlags );
                aRetGraphic = aBmp;
            }
        }
    }
    else
        aRetGraphic = rGraphic;

    return aRetGraphic;
}

// ------------------------------------------------------------------------

sal_uInt16 XOutBitmap::WriteGraphic( const Graphic& rGraphic, String& rFileName,
                                 const String& rFilterName, const sal_uIntPtr nFlags,
                                 const Size* pMtfSize_100TH_MM )
{
    if( rGraphic.GetType() != GRAPHIC_NONE )
    {
        INetURLObject   aURL( rFileName );
        Graphic         aGraphic;
        String          aExt;
        GraphicFilter*  pFilter = GraphicFilter::GetGraphicFilter();
        sal_uInt16          nErr = GRFILTER_FILTERERROR, nFilter = GRFILTER_FORMAT_NOTFOUND;
        sal_Bool            bTransparent = rGraphic.IsTransparent(), bAnimated = rGraphic.IsAnimated();

        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "XOutBitmap::WriteGraphic(...): invalid URL" );

        // calculate correct file name
        if( !( nFlags & XOUTBMP_DONT_EXPAND_FILENAME ) )
        {
            String aName( aURL.getBase() );
            aName += '_';
            aName += String(aURL.getExtension());
            aName += '_';
            String aStr( String::CreateFromInt32( rGraphic.GetChecksum(), 16 ) );
            if ( aStr.GetChar(0) == '-' )
                aStr.SetChar(0,'m');
            aName += aStr;
            aURL.setBase( aName );
        }

        if( ( nFlags & XOUTBMP_USE_NATIVE_IF_POSSIBLE ) &&
            !( nFlags & XOUTBMP_MIRROR_HORZ ) &&
            !( nFlags & XOUTBMP_MIRROR_VERT ) &&
            ( rGraphic.GetType() != GRAPHIC_GDIMETAFILE ) && rGraphic.IsLink() )
        {
            // try to write native link
            const GfxLink aGfxLink( ( (Graphic&) rGraphic ).GetLink() );

            switch( aGfxLink.GetType() )
            {
                case( GFX_LINK_TYPE_NATIVE_GIF ): aExt = FORMAT_GIF; break;
                case( GFX_LINK_TYPE_NATIVE_JPG ): aExt = FORMAT_JPG; break;
                case( GFX_LINK_TYPE_NATIVE_PNG ): aExt = FORMAT_PNG; break;

                default:
                break;
            }

            if( aExt.Len() )
            {
                aURL.setExtension( aExt );
                rFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );

                SfxMedium   aMedium( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_SHARE_DENYNONE | STREAM_TRUNC, sal_True );
                SvStream*   pOStm = aMedium.GetOutStream();

                if( pOStm && aGfxLink.GetDataSize() && aGfxLink.GetData() )
                {
                    pOStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
                    aMedium.Commit();

                    if( !aMedium.GetError() )
                        nErr = GRFILTER_OK;
                }
            }
        }

        if( GRFILTER_OK != nErr )
        {
            String  aFilter( rFilterName );
            sal_Bool    bWriteTransGrf = ( aFilter.EqualsIgnoreCaseAscii( "transgrf" ) ) ||
                                     ( aFilter.EqualsIgnoreCaseAscii( "gif" ) ) ||
                                     ( nFlags & XOUTBMP_USE_GIF_IF_POSSIBLE ) ||
                                     ( ( nFlags & XOUTBMP_USE_GIF_IF_SENSIBLE ) && ( bAnimated || bTransparent ) );

            // get filter and extension
            if( bWriteTransGrf )
                aFilter = FORMAT_GIF;

            nFilter = pFilter->GetExportFormatNumberForShortName( aFilter );

            if( GRFILTER_FORMAT_NOTFOUND == nFilter )
            {
                nFilter = pFilter->GetExportFormatNumberForShortName( FORMAT_JPG );

                if( GRFILTER_FORMAT_NOTFOUND == nFilter )
                    nFilter = pFilter->GetExportFormatNumberForShortName( FORMAT_BMP );
            }

            if( GRFILTER_FORMAT_NOTFOUND != nFilter )
            {
                aExt = pFilter->GetExportFormatShortName( nFilter ).ToLowerAscii();

                if( bWriteTransGrf )
                {
                    if( bAnimated  )
                        aGraphic = rGraphic;
                    else
                    {
                        if( pMtfSize_100TH_MM && ( rGraphic.GetType() != GRAPHIC_BITMAP ) )
                        {
                            VirtualDevice aVDev;
                            const Size    aSize( aVDev.LogicToPixel( *pMtfSize_100TH_MM, MAP_100TH_MM ) );

                            if( aVDev.SetOutputSizePixel( aSize ) )
                            {
                                const Wallpaper aWallpaper( aVDev.GetBackground() );
                                const Point     aPt;

                                aVDev.SetBackground( Wallpaper( Color( COL_BLACK ) ) );
                                aVDev.Erase();
                                rGraphic.Draw( &aVDev, aPt, aSize );

                                const Bitmap aBitmap( aVDev.GetBitmap( aPt, aSize ) );

                                aVDev.SetBackground( aWallpaper );
                                aVDev.Erase();
                                rGraphic.Draw( &aVDev, aPt, aSize );

                                aVDev.SetRasterOp( ROP_XOR );
                                aVDev.DrawBitmap( aPt, aSize, aBitmap );
                                aGraphic = BitmapEx( aBitmap, aVDev.GetBitmap( aPt, aSize ) );
                            }
                            else
                                aGraphic = rGraphic.GetBitmapEx();
                        }
                        else
                            aGraphic = rGraphic.GetBitmapEx();
                    }
                }
                else
                {
                    if( pMtfSize_100TH_MM && ( rGraphic.GetType() != GRAPHIC_BITMAP ) )
                    {
                        VirtualDevice   aVDev;
                        const Size      aSize( aVDev.LogicToPixel( *pMtfSize_100TH_MM, MAP_100TH_MM ) );

                        if( aVDev.SetOutputSizePixel( aSize ) )
                        {
                            rGraphic.Draw( &aVDev, Point(), aSize );
                            aGraphic =  aVDev.GetBitmap( Point(), aSize );
                        }
                        else
                            aGraphic = rGraphic.GetBitmap();
                    }
                    else
                        aGraphic = rGraphic.GetBitmap();
                }

                // mirror?
                if( ( nFlags & XOUTBMP_MIRROR_HORZ ) || ( nFlags & XOUTBMP_MIRROR_VERT ) )
                    aGraphic = MirrorGraphic( aGraphic, nFlags );

                if( ( GRFILTER_FORMAT_NOTFOUND != nFilter ) && ( aGraphic.GetType() != GRAPHIC_NONE ) )
                {
                    aURL.setExtension( aExt );
                    rFileName = aURL.GetMainURL( INetURLObject::NO_DECODE );
                    nErr = ExportGraphic( aGraphic, aURL, *pFilter, nFilter, NULL );
                }
            }
        }

        return nErr;
    }
    else
    {
        return GRFILTER_OK;
    }
}

// ------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

sal_uInt16 XOutBitmap::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                  GraphicFilter& rFilter, const sal_uInt16 nFormat,
                                  const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >* pFilterData )
{
    DBG_ASSERT( rURL.GetProtocol() != INET_PROT_NOT_VALID, "XOutBitmap::ExportGraphic(...): invalid URL" );

    SfxMedium   aMedium( rURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_SHARE_DENYNONE | STREAM_TRUNC, sal_True );
    SvStream*   pOStm = aMedium.GetOutStream();
    sal_uInt16      nRet = GRFILTER_IOERROR;

    if( pOStm )
    {
        pGrfFilter = &rFilter;

        nRet = rFilter.ExportGraphic( rGraphic, rURL.GetMainURL( INetURLObject::NO_DECODE ), *pOStm, nFormat, pFilterData );

        pGrfFilter = NULL;
        aMedium.Commit();

        if( aMedium.GetError() && ( GRFILTER_OK == nRet  ) )
            nRet = GRFILTER_IOERROR;
    }

    return nRet;
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

// ------------------------------------------------------------------------

Bitmap XOutBitmap::DetectEdges( const Bitmap& rBmp, const sal_uInt8 cThreshold )
{
    const Size  aSize( rBmp.GetSizePixel() );
    Bitmap      aRetBmp;
    sal_Bool        bRet = sal_False;

    if( ( aSize.Width() > 2L ) && ( aSize.Height() > 2L ) )
    {
        Bitmap aWorkBmp( rBmp );

        if( aWorkBmp.Convert( BMP_CONVERSION_8BIT_GREYS ) )
        {
            Bitmap              aDstBmp( aSize, 1 );
            BitmapReadAccess*   pReadAcc = aWorkBmp.AcquireReadAccess();
            BitmapWriteAccess*  pWriteAcc = aDstBmp.AcquireWriteAccess();

            if( pReadAcc && pWriteAcc )
            {
                const long          nWidth = aSize.Width();
                const long          nWidth2 = nWidth - 2L;
                const long          nHeight = aSize.Height();
                const long          nHeight2 = nHeight - 2L;
                const long          lThres2 = (long) cThreshold * cThreshold;
                const BitmapColor   aWhite = (sal_uInt8) pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) );
                const BitmapColor   aBlack = (sal_uInt8) pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) );
                long                nSum1;
                long                nSum2;
                long                lGray;

                // Rand mit Weiss init.
                pWriteAcc->SetLineColor( Color( COL_WHITE) );
                pWriteAcc->DrawLine( Point(), Point( nWidth - 1L, 0L ) );
                pWriteAcc->DrawLine( Point( nWidth - 1L, 0L ), Point( nWidth - 1L, nHeight - 1L ) );
                pWriteAcc->DrawLine( Point( nWidth - 1L, nHeight - 1L ), Point( 0L, nHeight - 1L ) );
                pWriteAcc->DrawLine( Point( 0, nHeight - 1L ), Point() );

                for( long nY = 0L, nY1 = 1L, nY2 = 2; nY < nHeight2; nY++, nY1++, nY2++ )
                {
                    for( long nX = 0L, nXDst = 1L, nXTmp; nX < nWidth2; nX++, nXDst++ )
                    {
                        nXTmp = nX;

                        nSum1 = -( nSum2 = lGray = (sal_uInt8) pReadAcc->GetPixel( nY, nXTmp++ ) );
                        nSum2 += ( (long) (sal_uInt8) pReadAcc->GetPixel( nY, nXTmp++ ) ) << 1;
                        nSum1 += ( lGray = pReadAcc->GetPixel( nY, nXTmp ) );
                        nSum2 += lGray;

                        nSum1 += ( (long) (sal_uInt8) pReadAcc->GetPixel( nY1, nXTmp ) ) << 1;
                        nSum1 -= ( (long) (sal_uInt8) pReadAcc->GetPixel( nY1, nXTmp -= 2 ) ) << 1;

                        nSum1 += ( lGray = -(long) (sal_uInt8) pReadAcc->GetPixel( nY2, nXTmp++ ) );
                        nSum2 += lGray;
                        nSum2 -= ( (long) (sal_uInt8) pReadAcc->GetPixel( nY2, nXTmp++ ) ) << 1;
                        nSum1 += ( lGray = (long) (sal_uInt8) pReadAcc->GetPixel( nY2, nXTmp ) );
                        nSum2 -= lGray;

                        if( ( nSum1 * nSum1 + nSum2 * nSum2 ) < lThres2 )
                            pWriteAcc->SetPixel( nY1, nXDst, aWhite );
                        else
                            pWriteAcc->SetPixel( nY1, nXDst, aBlack );
                    }
                }

                bRet = sal_True;
            }

            aWorkBmp.ReleaseAccess( pReadAcc );
            aDstBmp.ReleaseAccess( pWriteAcc );

            if( bRet )
                aRetBmp = aDstBmp;
        }
    }

    if( !aRetBmp )
        aRetBmp = rBmp;
    else
    {
        aRetBmp.SetPrefMapMode( rBmp.GetPrefMapMode() );
        aRetBmp.SetPrefSize( rBmp.GetPrefSize() );
    }

    return aRetBmp;
};

// ------------------------------------------------------------------------

Polygon XOutBitmap::GetCountour( const Bitmap& rBmp, const sal_uIntPtr nFlags,
                                 const sal_uInt8 cEdgeDetectThreshold, const Rectangle* pWorkRectPixel )
{
    Bitmap      aWorkBmp;
    Polygon     aRetPoly;
    Point       aTmpPoint;
    Rectangle   aWorkRect( aTmpPoint, rBmp.GetSizePixel() );

    if( pWorkRectPixel )
        aWorkRect.Intersection( *pWorkRectPixel );

    aWorkRect.Justify();

    if( ( aWorkRect.GetWidth() > 4 ) && ( aWorkRect.GetHeight() > 4 ) )
    {
        // falls Flag gesetzt, muessen wir Kanten detektieren
        if( nFlags & XOUTBMP_CONTOUR_EDGEDETECT )
            aWorkBmp = DetectEdges( rBmp, cEdgeDetectThreshold );
        else
            aWorkBmp = rBmp;

        BitmapReadAccess* pAcc = aWorkBmp.AcquireReadAccess();

        if( pAcc )
        {
            const Size&         rPrefSize = aWorkBmp.GetPrefSize();
            const long          nWidth = pAcc->Width();
            const long          nHeight = pAcc->Height();
            const double        fFactorX = (double) rPrefSize.Width() / nWidth;
            const double        fFactorY = (double) rPrefSize.Height() / nHeight;
            const long          nStartX1 = aWorkRect.Left() + 1L;
            const long          nEndX1 = aWorkRect.Right();
            const long          nStartX2 = nEndX1 - 1L;
//          const long          nEndX2 = nStartX1 - 1L;
            const long          nStartY1 = aWorkRect.Top() + 1L;
            const long          nEndY1 = aWorkRect.Bottom();
            const long          nStartY2 = nEndY1 - 1L;
//          const long          nEndY2 = nStartY1 - 1L;
            Point*              pPoints1 = NULL;
            Point*              pPoints2 = NULL;
            long                nX, nY;
            sal_uInt16              nPolyPos = 0;
            const BitmapColor   aBlack = pAcc->GetBestMatchingColor( Color( COL_BLACK ) );

            if( nFlags & XOUTBMP_CONTOUR_VERT )
            {
                pPoints1 = new Point[ nWidth ];
                pPoints2 = new Point[ nWidth ];

                for( nX = nStartX1; nX < nEndX1; nX++ )
                {
                    nY = nStartY1;

                    // zunaechst Zeile von Links nach Rechts durchlaufen
                    while( nY < nEndY1 )
                    {
                        if( aBlack == pAcc->GetPixel( nY, nX ) )
                        {
                            pPoints1[ nPolyPos ] = Point( nX, nY );
                            nY = nStartY2;

                            // diese Schleife wird immer gebreaked da hier ja min. ein Pixel ist
                            while( sal_True )
                            {
                                if( aBlack == pAcc->GetPixel( nY, nX ) )
                                {
                                    pPoints2[ nPolyPos ] = Point( nX, nY );
                                    break;
                                }

                                nY--;
                            }

                            nPolyPos++;
                            break;
                        }

                        nY++;
                    }
                }
            }
            else
            {
                pPoints1 = new Point[ nHeight ];
                pPoints2 = new Point[ nHeight ];

                for ( nY = nStartY1; nY < nEndY1; nY++ )
                {
                    nX = nStartX1;

                    // zunaechst Zeile von Links nach Rechts durchlaufen
                    while( nX < nEndX1 )
                    {
                        if( aBlack == pAcc->GetPixel( nY, nX ) )
                        {
                            pPoints1[ nPolyPos ] = Point( nX, nY );
                            nX = nStartX2;

                            // diese Schleife wird immer gebreaked da hier ja min. ein Pixel ist
                            while( sal_True )
                            {
                                if( aBlack == pAcc->GetPixel( nY, nX ) )
                                {
                                    pPoints2[ nPolyPos ] = Point( nX, nY );
                                    break;
                                }

                                nX--;
                            }

                            nPolyPos++;
                            break;
                        }

                        nX++;
                    }
                }
            }

            const sal_uInt16 nNewSize1 = nPolyPos << 1;

            aRetPoly = Polygon( nPolyPos, pPoints1 );
            aRetPoly.SetSize( nNewSize1 + 1 );
            aRetPoly[ nNewSize1 ] = aRetPoly[ 0 ];

            for( sal_uInt16 j = nPolyPos; nPolyPos < nNewSize1; )
                aRetPoly[ nPolyPos++ ] = pPoints2[ --j ];

            if( ( fFactorX != 0. ) && ( fFactorY != 0. ) )
                aRetPoly.Scale( fFactorX, fFactorY );

            delete[] pPoints1;
            delete[] pPoints2;
        }
    }

    return aRetPoly;
};

// ----------------
// - DitherBitmap -
// ----------------

sal_Bool DitherBitmap( Bitmap& rBitmap )
{
    sal_Bool bRet = sal_False;

    if( ( rBitmap.GetBitCount() >= 8 ) && ( Application::GetDefaultDevice()->GetColorCount() < 257 ) )
        bRet = rBitmap.Dither( BMP_DITHER_FLOYD );
    else
        bRet = sal_False;

    return bRet;
}
