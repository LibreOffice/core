/*************************************************************************
 *
 *  $RCSfile: _xoutbmp.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-10 15:17:43 $
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

#include <sot/factory.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/poly.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wrkwin.hxx>
#include <svtools/solar.hrc>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include "impgrf.hxx"
#include "xoutbmp.hxx"

// -----------
// - Defines -
// -----------

#define FORMAT_BMP  String(RTL_CONSTASCII_USTRINGPARAM("bmp"))
#define FORMAT_GIF  String(RTL_CONSTASCII_USTRINGPARAM("gif"))
#define FORMAT_JPG  String(RTL_CONSTASCII_USTRINGPARAM("jpg"))
#define FORMAT_PNG  String(RTL_CONSTASCII_USTRINGPARAM("png"))

#define OPT_BMP     "BMP-COLORS"
#define OPT_JPG     "JPG_EXPORT_COLORMODE"

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
                BOOL        bFullTrans = FALSE;

                if( aBmpSize.Width() == 1 && aBmpSize.Height() == 1 && rGraphic.IsTransparent() )
                {
                    Bitmap              aTrans( rGraphic.GetBitmapEx().GetMask() );
                    BitmapReadAccess*   pMAcc = aBmp.AcquireReadAccess();

                    if( pMAcc )
                    {
                        if( pMAcc->GetColor( 0, 0 ) == BitmapColor( Color( COL_WHITE ) ) )
                            bFullTrans = TRUE;

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
    const BOOL      bNoSize = ( aPixSize == rBmpEx.GetSizePixel() );

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

Animation XOutBitmap::MirrorAnimation( const Animation& rAnimation, BOOL bHMirr, BOOL bVMirr )
{
    Animation aNewAnim( rAnimation );

    if( bHMirr || bVMirr )
    {
        const Size& rGlobalSize = aNewAnim.GetDisplaySizePixel();
        ULONG       nMirrorFlags = 0L;

        if( bHMirr )
            nMirrorFlags |= BMP_MIRROR_HORZ;

        if( bVMirr )
            nMirrorFlags |= BMP_MIRROR_VERT;

        for( USHORT i = 0, nCount = aNewAnim.Count(); i < nCount; i++ )
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

Graphic XOutBitmap::MirrorGraphic( const Graphic& rGraphic, const ULONG nMirrorFlags )
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

USHORT XOutBitmap::WriteGraphic( const Graphic& rGraphic, String& rFileName,
                                 const String& rFilterName, const ULONG nFlags,
                                 const Size* pMtfSize_100TH_MM )
{
#ifndef SVX_LIGHT
    if( rGraphic.GetType() != GRAPHIC_NONE )
    {
        INetURLObject   aURL( rFileName );
        Graphic         aGraphic;
        String          aExt;
        GraphicFilter*  pFilter = GetGrfFilter();
        USHORT          nErr = GRFILTER_FILTERERROR, nFilter = GRFILTER_FORMAT_NOTFOUND;
        BOOL            bTransparent = rGraphic.IsTransparent(), bAnimated = rGraphic.IsAnimated();

        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "XOutBitmap::WriteGraphic(...): invalid URL" );

        // calculate correct file name
        if( !( nFlags & XOUTBMP_DONT_EXPAND_FILENAME ) )
        {
            String      aTimeStr( UniString::CreateFromInt32( Time().GetTime() ) );
            String      aName( aURL.getName() );
            xub_StrLen  nNameLen = aName.Len();

            if( nNameLen > 3 )
            {
                nNameLen -= 3;

                if( aName.GetChar( nNameLen - 1 ) == sal_Unicode( '.' ) )
                    --nNameLen;
            }

            aURL.setName( aName.Insert( aTimeStr.Insert( sal_Unicode('_'), 0 ), nNameLen ) );
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
                rFileName = aURL.GetMainURL();

                SfxMedium   aMedium( aURL.GetMainURL(), STREAM_WRITE | STREAM_SHARE_DENYNONE | STREAM_TRUNC, TRUE );
                SvStream*   pOStm = aMedium.GetOutStream();

                if( pOStm && aGfxLink.GetDataSize() )
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
            BOOL    bWriteTransGrf = ( aFilter.EqualsIgnoreCaseAscii( "transgrf" ) ) ||
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
                    rFileName = aURL.GetMainURL();
                    nErr = ExportGraphic( aGraphic, aURL, *pFilter, nFilter, TRUE );
                }
            }
        }

        return nErr;
    }
    else
#endif
        return GRFILTER_OK;
}

// ------------------------------------------------------------------------

#ifdef WNT
#pragma optimize ( "", off )
#endif

USHORT XOutBitmap::ExportGraphic( const Graphic& rGraphic, const INetURLObject& rURL,
                                  GraphicFilter& rFilter, const USHORT nFormat,
                                  BOOL bIgnoreOptions )
{
    DBG_ASSERT( rURL.GetProtocol() != INET_PROT_NOT_VALID, "XOutBitmap::ExportGraphic(...): invalid URL" );

    SfxMedium   aMedium( rURL.GetMainURL(), STREAM_WRITE | STREAM_SHARE_DENYNONE | STREAM_TRUNC, TRUE );
    SvStream*   pOStm = aMedium.GetOutStream();
    USHORT      nRet = 1;

    if( pOStm )
    {
        Config* pOptionsConfig = rFilter.GetOptionsConfig();

        pGrfFilter = &rFilter;

        if( bIgnoreOptions && rFilter.AreOptionsEnabled() )
        {
            rFilter.EnableOptions( FALSE );
            nRet = rFilter.ExportGraphic( rGraphic, rURL.GetMainURL(), *pOStm, nFormat );
            rFilter.EnableOptions( TRUE );
        }
        else
        {
            Graphic aGraphic;

            if( pOptionsConfig )
            {
                const String aFormat( rFilter.GetExportFormatShortName( nFormat ).ToLowerAscii() );

                // Optionen fuer die einzelnen Format beruecksichtigen
                if( aFormat == FORMAT_BMP )
                {
                    USHORT nColorRes = pOptionsConfig->ReadKey( ByteString(OPT_BMP) ).ToInt32();

                    if( !nColorRes || ( nColorRes > (USHORT) BMP_CONVERSION_24BIT ) )
                        aGraphic = rGraphic;
                    else
                    {
                        Bitmap aTmp( rGraphic.GetBitmap() );

                        if( aTmp.Convert( (BmpConversion) nColorRes ) )
                            aGraphic = aTmp;
                        else
                            aGraphic = rGraphic;
                    }
                }
                else if( aFormat == FORMAT_JPG )
                {
                    Bitmap              aTmp( rGraphic.GetBitmap() );
                    const BOOL          bGreys = (BOOL) pOptionsConfig->ReadKey( ByteString(OPT_JPG) ).ToInt32();
                    const BmpConversion eConv = bGreys ? BMP_CONVERSION_8BIT_GREYS : BMP_CONVERSION_24BIT;

                    if( aTmp.Convert( eConv ) )
                        aGraphic = aTmp;
                    else
                        aGraphic = rGraphic;
                }
                else if( aFormat == FORMAT_GIF )
                    aGraphic = rGraphic;
                else
                    aGraphic = rGraphic;
            }
            else
                aGraphic = rGraphic;

            nRet = rFilter.ExportGraphic( aGraphic, rURL.GetMainURL(), *pOStm, nFormat );
        }

        pGrfFilter = NULL;
        aMedium.Commit();

        if( aMedium.GetError() && ( GRFILTER_OK == nRet  ) )
            nRet = 1;
    }

    return nRet;
}

#ifdef WNT
#pragma optimize ( "", on )
#endif

// ------------------------------------------------------------------------

Bitmap XOutBitmap::DetectEdges( const Bitmap& rBmp, const BYTE cThreshold )
{
    const Size  aSize( rBmp.GetSizePixel() );
    Bitmap      aRetBmp;
    BOOL        bRet = FALSE;

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
                const BitmapColor   aWhite = (BYTE) pWriteAcc->GetBestMatchingColor( Color( COL_WHITE ) );
                const BitmapColor   aBlack = (BYTE) pWriteAcc->GetBestMatchingColor( Color( COL_BLACK ) );
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

                        nSum1 = -( nSum2 = lGray = (BYTE) pReadAcc->GetPixel( nY, nXTmp++ ) );
                        nSum2 += ( (long) (BYTE) pReadAcc->GetPixel( nY, nXTmp++ ) ) << 1;
                        nSum1 += ( lGray = pReadAcc->GetPixel( nY, nXTmp ) );
                        nSum2 += lGray;

                        nSum1 += ( (long) (BYTE) pReadAcc->GetPixel( nY1, nXTmp ) ) << 1;
                        nSum1 -= ( (long) (BYTE) pReadAcc->GetPixel( nY1, nXTmp -= 2 ) ) << 1;

                        nSum1 += ( lGray = -(long) (BYTE) pReadAcc->GetPixel( nY2, nXTmp++ ) );
                        nSum2 += lGray;
                        nSum2 -= ( (long) (BYTE) pReadAcc->GetPixel( nY2, nXTmp++ ) ) << 1;
                        nSum1 += ( lGray = (long) (BYTE) pReadAcc->GetPixel( nY2, nXTmp ) );
                        nSum2 -= lGray;

                        if( ( nSum1 * nSum1 + nSum2 * nSum2 ) < lThres2 )
                            pWriteAcc->SetPixel( nY1, nXDst, aWhite );
                        else
                            pWriteAcc->SetPixel( nY1, nXDst, aBlack );
                    }
                }

                bRet = TRUE;
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

Polygon XOutBitmap::GetCountour( const Bitmap& rBmp, const ULONG nFlags,
                                 const BYTE cEdgeDetectThreshold, const Rectangle* pWorkRectPixel )
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
            const long          nEndX2 = nStartX1 - 1L;
            const long          nStartY1 = aWorkRect.Top() + 1L;
            const long          nEndY1 = aWorkRect.Bottom();
            const long          nStartY2 = nEndY1 - 1L;
            const long          nEndY2 = nStartY1 - 1L;
            Point*              pPoints1 = NULL;
            Point*              pPoints2 = NULL;
            long                nX, nY;
            USHORT              nPolyPos = 0;
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
                            while( TRUE )
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
                            while( TRUE )
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

            const USHORT nNewSize1 = nPolyPos << 1;

            aRetPoly = Polygon( nPolyPos, pPoints1 );
            aRetPoly.SetSize( nNewSize1 + 1 );
            aRetPoly[ nNewSize1 ] = aRetPoly[ 0 ];

            for( USHORT j = nPolyPos; nPolyPos < nNewSize1; )
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

BOOL DitherBitmap( Bitmap& rBitmap )
{
    BOOL bRet = FALSE;

    if( ( rBitmap.GetBitCount() >= 8 ) && ( Application::GetDefaultDevice()->GetColorCount() < 257 ) )
        bRet = rBitmap.Dither( BMP_DITHER_FLOYD );
    else
        bRet = FALSE;

    return bRet;
}
