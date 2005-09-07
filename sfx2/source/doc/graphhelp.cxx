/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: graphhelp.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:42:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef WNT

#define UINT64 USE_WIN_UINT64
#define INT64 USE_WIN_INT64
#define UINT32 USE_WIN_UINT32
#define INT32 USE_WIN_INT32

#include <tools/presys.h>
#include <windows.h>
#include <tools/postsys.h>

#undef UINT64
#undef INT64
#undef UINT32
#undef INT32

#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHICPROVIDER_HPP_
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_GRAPHIC_XGRAPHIC_HPP_
#include <com/sun/star/graphic/XGraphic.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif


#include <osl/thread.h>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/salbtype.hxx>

#include <tools/stream.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/processfactory.hxx>


#include "sfxresid.hxx"
#include "graphhelp.hxx"
#include "doc.hrc"

using namespace ::com::sun::star;

//---------------------------------------------------------------
// static
SvMemoryStream* GraphicHelper::getFormatStrFromGDI_Impl( const GDIMetaFile* pGDIMeta, sal_uInt32 nFormat )
{
    SvMemoryStream* pResult = NULL;
    if ( pGDIMeta )
    {
        SvMemoryStream* pStream = new SvMemoryStream( 65535, 65535 );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            if ( GraphicConverter::Export( *pStream, aGraph, nFormat ) == 0 )
                pResult = pStream;
            else
                delete pStream;
        }
    }

    return pResult;
}

//---------------------------------------------------------------
// static
void* GraphicHelper::getEnhMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta )
{
    void* pResult = NULL;

#ifdef WNT
    if ( pGDIMeta )
    {
        String aStr = ::rtl::OUString::createFromAscii( ".emf" );
        ::utl::TempFile aTempFile( ::rtl::OUString(),
                                   &aStr,
                                   NULL,
                                   sal_False );

        ::rtl::OUString aMetaFile = aTempFile.GetFileName();
        ::rtl::OUString aMetaURL = aTempFile.GetURL();
        ::rtl::OString aWinFile = ::rtl::OUStringToOString( aMetaFile, osl_getThreadTextEncoding() );

        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( aMetaURL, STREAM_STD_READWRITE );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            sal_Bool bFailed = (sal_Bool)GraphicConverter::Export( *pStream, aGraph, CVT_EMF );
            pStream->Flush();
            delete pStream;

            if ( !bFailed )
                pResult = GetEnhMetaFileA( aWinFile.getStr() );
        }
    }
#endif

    return pResult;
}

//---------------------------------------------------------------
// static
void* GraphicHelper::getWinMetaFileFromGDI_Impl( const GDIMetaFile* pGDIMeta, const Size& aMetaSize )
{
    void* pResult = NULL;

#ifdef WNT
    if ( pGDIMeta )
    {
        SvMemoryStream* pStream = new SvMemoryStream( 65535, 65535 );
        if ( pStream )
        {
            Graphic aGraph( *pGDIMeta );
            sal_Bool bFailed = (sal_Bool)GraphicConverter::Export( *pStream, aGraph, CVT_WMF );
            pStream->Flush();
            if ( !bFailed )
            {
                sal_Int32 nLength = pStream->Seek( STREAM_SEEK_TO_END );
                if ( nLength > 22 )
                {
                    HMETAFILE hMeta = SetMetaFileBitsEx( nLength - 22,
                                    ( reinterpret_cast< const sal_uChar*>( pStream->GetData() ) ) + 22 );

                    if ( hMeta )
                    {
                        HGLOBAL hMemory = GlobalAlloc( GMEM_DDESHARE | GMEM_MOVEABLE, sizeof( METAFILEPICT ) );

                        if ( hMemory )
                        {
                               METAFILEPICT* pMF = (METAFILEPICT*)GlobalLock( hMemory );

                               pMF->hMF = hMeta;
                               pMF->mm = MM_ANISOTROPIC;

                            MapMode aMetaMode = pGDIMeta->GetPrefMapMode();
                            MapMode aWinMode( MAP_100TH_MM );

                            if ( aWinMode == pGDIMeta->GetPrefMapMode() )
                            {
                                pMF->xExt = aMetaSize.Width();
                                pMF->yExt = aMetaSize.Height();
                            }
                            else
                            {
                                Size aWinSize = OutputDevice::LogicToLogic( Size( aMetaSize.Width(), aMetaSize.Height() ),
                                                                            aWinMode,
                                                                            pGDIMeta->GetPrefMapMode() );
                                pMF->xExt = aWinSize.Width();
                                pMF->yExt = aWinSize.Height();
                            }

                            GlobalUnlock( hMemory );
                            pResult = (void*)hMemory;
                        }
                        else
                               DeleteMetaFile( hMeta );
                    }
                }
            }

            delete pStream;
        }
    }

#endif


    return pResult;
}

//---------------------------------------------------------------
// static
sal_Bool GraphicHelper::supportsMetaFileHandle_Impl()
{
#ifdef WNT
    return sal_True;
#else
    return sal_False;
#endif
}

//---------------------------------------------------------------
// static
sal_Bool GraphicHelper::mergeBitmaps_Impl( const BitmapEx& rBmpEx, const BitmapEx& rOverlay,
                   const Rectangle& rOverlayRect, BitmapEx& rReturn )
{
    // the implementation is provided by KA

    Point           aNullPt;
    Rectangle       aBmpRect( aNullPt, rBmpEx.GetSizePixel() );
    VirtualDevice   aVDev;

    if( !rReturn.IsEmpty() )
        rReturn.SetEmpty();

    if( !rBmpEx.IsEmpty() && aVDev.SetOutputSizePixel( aBmpRect.GetSize() ) )
    {
        Rectangle aOverlayRect( rOverlayRect );

        aOverlayRect.Intersection( aBmpRect );

        if( rOverlay.IsEmpty() || rOverlayRect.IsEmpty() )
            rReturn = rBmpEx;
        else
        {
            aVDev.DrawBitmap( aNullPt, aVDev.GetOutputSizePixel(), rBmpEx.GetBitmap() );
            aVDev.DrawBitmapEx( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), rOverlay );

            Bitmap aBmp( aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );
            aBmp.Convert( BMP_CONVERSION_24BIT );

            if( !rBmpEx.IsTransparent() )
                rReturn = aBmp;
            else
            {
                aVDev.DrawBitmap( aNullPt, aVDev.GetOutputSizePixel(), rBmpEx.GetMask() );
                Bitmap aOverlayMergeBmp( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ) );

                if( rOverlay.IsTransparent() )
                    aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), rOverlay.GetMask() );
                else
                {
                    aVDev.SetLineColor( COL_BLACK );
                    aVDev.SetFillColor( COL_BLACK );
                    aVDev.DrawRect( aOverlayRect);
                }

                aOverlayMergeBmp.CombineSimple( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ), BMP_COMBINE_AND );
                aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), aOverlayMergeBmp );
                rReturn = BitmapEx( aBmp, aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );
            }
        }
    }

    return !rReturn.IsEmpty();
}


//---------------------------------------------------------------
// static
sal_Bool GraphicHelper::createThumb_Impl( const GDIMetaFile& rMtf,
                       sal_uInt32 nMaximumExtent,
                       BitmapEx& rBmpEx,
                       const BitmapEx* pOverlay,
                       const Rectangle* pOverlayRect )
{
    // the implementation is provided by KA

    // initialization seems to be complicated but is used to avoid rounding errors
    VirtualDevice   aVDev;
    const Point     aNullPt;
    const Point     aTLPix( aVDev.LogicToPixel( aNullPt, rMtf.GetPrefMapMode() ) );
    const Point     aBRPix( aVDev.LogicToPixel( Point( rMtf.GetPrefSize().Width() - 1, rMtf.GetPrefSize().Height() - 1 ), rMtf.GetPrefMapMode() ) );
    Size            aDrawSize( aVDev.LogicToPixel( rMtf.GetPrefSize(), rMtf.GetPrefMapMode() ) );
    Size            aSizePix( labs( aBRPix.X() - aTLPix.X() ) + 1, labs( aBRPix.Y() - aTLPix.Y() ) + 1 );
    Point           aPosPix;

    if ( !rBmpEx.IsEmpty() )
        rBmpEx.SetEmpty();

    // determine size that has the same aspect ratio as image size and
    // fits into the rectangle determined by nMaximumExtent
    if ( aSizePix.Width() && aSizePix.Height()
      && ( aSizePix.Width() > nMaximumExtent || aSizePix.Height() > nMaximumExtent ) )
    {
        const Size  aOldSizePix( aSizePix );
        double      fWH = static_cast< double >( aSizePix.Width() ) / aSizePix.Height();

        if ( fWH <= 1.0 )
        {
            aSizePix.Width() = FRound( nMaximumExtent * fWH );
            aSizePix.Height() = nMaximumExtent;
        }
        else
        {
            aSizePix.Width() = nMaximumExtent;
            aSizePix.Height() = FRound(  nMaximumExtent / fWH );
        }

        aDrawSize.Width() = FRound( ( static_cast< double >( aDrawSize.Width() ) * aSizePix.Width() ) / aOldSizePix.Width() );
        aDrawSize.Height() = FRound( ( static_cast< double >( aDrawSize.Height() ) * aSizePix.Height() ) / aOldSizePix.Height() );
    }

    Size        aFullSize;
    Point       aBackPosPix;
    Rectangle   aOverlayRect;

    // calculate addigtional positions and sizes if an overlay image is used
    if (  pOverlay )
    {
        aFullSize = Size( nMaximumExtent, nMaximumExtent );
        aOverlayRect = Rectangle( aNullPt, aFullSize  );

        aOverlayRect.Intersection( pOverlayRect ? *pOverlayRect : Rectangle( aNullPt, pOverlay->GetSizePixel() ) );

        if ( !aOverlayRect.IsEmpty() )
            aBackPosPix = Point( ( nMaximumExtent - aSizePix.Width() ) >> 1, ( nMaximumExtent - aSizePix.Height() ) >> 1 );
        else
            pOverlay = NULL;
    }
    else
    {
        aFullSize = aSizePix;
        pOverlay = NULL;
    }

    // draw image(s) into VDev and get resulting image
    if ( aVDev.SetOutputSizePixel( aFullSize ) )
    {
        // draw metafile into VDev
        const_cast< GDIMetaFile& >( rMtf ).WindStart();
        const_cast< GDIMetaFile& >( rMtf ).Play( &aVDev, aBackPosPix, aDrawSize );

        // draw overlay if neccessary
        if ( pOverlay )
            aVDev.DrawBitmapEx( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), *pOverlay );

        // get paint bitmap
        Bitmap aBmp( aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );

        // assure that we have a true color image
        if ( aBmp.GetBitCount() != 24 )
            aBmp.Convert( BMP_CONVERSION_24BIT );

        // create resulting mask bitmap with metafile output set to black
        GDIMetaFile aMonchromeMtf( rMtf.GetMonochromeMtf( COL_BLACK ) );
        aVDev.DrawWallpaper( Rectangle( aNullPt, aSizePix ), Wallpaper( Color( COL_WHITE ) ) );
        aMonchromeMtf.WindStart();
        aMonchromeMtf.Play( &aVDev, aBackPosPix, aDrawSize );

        // watch for overlay mask
        if ( pOverlay  )
        {
            Bitmap aOverlayMergeBmp( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ) );

            // create ANDed resulting mask at overlay area
            if ( pOverlay->IsTransparent() )
                aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), pOverlay->GetMask() );
            else
            {
                aVDev.SetLineColor( COL_BLACK );
                aVDev.SetFillColor( COL_BLACK );
                aVDev.DrawRect( aOverlayRect);
            }

            aOverlayMergeBmp.CombineSimple( aVDev.GetBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize() ), BMP_COMBINE_AND );
            aVDev.DrawBitmap( aOverlayRect.TopLeft(), aOverlayRect.GetSize(), aOverlayMergeBmp );
        }

        rBmpEx = BitmapEx( aBmp, aVDev.GetBitmap( aNullPt, aVDev.GetOutputSizePixel() ) );
    }

    return !rBmpEx.IsEmpty();
}

//---------------------------------------------------------------
// static
sal_Bool GraphicHelper::getThumbnailFormatFromGDI_Impl( GDIMetaFile* pMetaFile,
                                                        sal_Bool bSigned,
                                                        const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;
    SvStream* pStream = NULL;

    if ( xStream.is() )
        pStream = ::utl::UcbStreamHelper::CreateStream( xStream );

    if ( pMetaFile && pStream && !pStream->GetError() )
    {
        BitmapEx aResultBitmap;
        BitmapEx* pSignatureBitmap = NULL;

        if ( bSigned )
            pSignatureBitmap = new BitmapEx( SfxResId( BMP_SIGNATURE ) );

        bResult = createThumb_Impl( *pMetaFile,
                                       160,
                                       aResultBitmap,
                                    pSignatureBitmap );
        if ( bResult )
            bResult = ( !aResultBitmap.IsEmpty()
                        && GraphicConverter::Export( *pStream, aResultBitmap, CVT_PNG ) == 0
                        && ( pStream->Flush(), !pStream->GetError() ) );

        if ( pSignatureBitmap )
            delete pSignatureBitmap;

        delete pStream;
    }

    return bResult;
}

//---------------------------------------------------------------
// static
sal_Bool GraphicHelper::getSignedThumbnailFormatFromBitmap_Impl( const BitmapEx& aBitmap,
                                                                 const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;
    SvStream* pStream = NULL;

    if ( xStream.is() )
        pStream = ::utl::UcbStreamHelper::CreateStream( xStream );

    if ( pStream && !pStream->GetError() )
    {
        BitmapEx aResultBitmap;
        BitmapEx aSignatureBitmap( SfxResId( BMP_SIGNATURE ) );

        bResult = mergeBitmaps_Impl( aBitmap,
                                    aSignatureBitmap,
                                    Rectangle( Point(), aBitmap.GetSizePixel() ),
                                    aResultBitmap );

        if ( bResult )
        {
            bResult = ( !aResultBitmap.IsEmpty()
                        && GraphicConverter::Export( *pStream, aResultBitmap, CVT_PNG ) == 0
                        && ( pStream->Flush(), !pStream->GetError() ) );
        }

        delete pStream;
    }

    return bResult;
}

//---------------------------------------------------------------
// static
sal_Bool GraphicHelper::getThumbnailReplacement_Impl( sal_Int32 nResID, const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;
    if ( nResID && xStream.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        if ( xServiceManager.is() )
        {
            try
            {
                uno::Reference< graphic::XGraphicProvider > xGraphProvider(
                    xServiceManager->createInstance(
                        ::rtl::OUString::createFromAscii( "com.sun.star.graphic.GraphicProvider" ) ),
                    uno::UNO_QUERY );
                if ( xGraphProvider.is() )
                {
                    ::rtl::OUString aURL = ::rtl::OUString::createFromAscii( "private:resource/sfx/bitmapex/" );
                    aURL += ::rtl::OUString::valueOf( nResID );

                    uno::Sequence< beans::PropertyValue > aMediaProps( 1 );
                    aMediaProps[0].Name = ::rtl::OUString::createFromAscii( "URL" );
                    aMediaProps[0].Value <<= aURL;

                    uno::Reference< graphic::XGraphic > xGraphic = xGraphProvider->queryGraphic( aMediaProps );
                    if ( xGraphic.is() )
                    {
                        uno::Sequence< beans::PropertyValue > aStoreProps( 2 );
                        aStoreProps[0].Name = ::rtl::OUString::createFromAscii( "OutputStream" );
                        aStoreProps[0].Value <<= xStream;
                        aStoreProps[1].Name = ::rtl::OUString::createFromAscii( "MimeType" );
                        aStoreProps[1].Value <<= ::rtl::OUString::createFromAscii( "image/png" );

                        xGraphProvider->storeGraphic( xGraphic, aStoreProps );
                        bResult = sal_True;
                    }
                }
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    return bResult;
}

//---------------------------------------------------------------
// static
sal_uInt16 GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl( const ::rtl::OUString& aFactoryShortName, sal_Bool bIsTemplate )
{
    sal_uInt16 nResult = 0;

    if ( aFactoryShortName.equalsAscii( "scalc" ) )
    {
        nResult = BMP_128X128_CALC_DOC;
    }
    else if ( aFactoryShortName.equalsAscii( "sdraw" ) )
    {
        nResult = BMP_128X128_DRAW_DOC;
    }
    else if ( aFactoryShortName.equalsAscii( "simpress" ) )
    {
        nResult = BMP_128X128_IMPRESS_DOC;
    }
    else if ( aFactoryShortName.equalsAscii( "smath" ) )
    {
        nResult = BMP_128X128_MATH_DOC;
    }
    else if ( aFactoryShortName.equalsAscii( "swriter" ) || aFactoryShortName.compareToAscii( "swriter/", 8 ) == 0 )
    {
        nResult = BMP_128X128_WRITER_DOC;
    }

    return nResult;
}

