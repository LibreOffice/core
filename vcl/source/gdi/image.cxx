/*************************************************************************
 *
 *  $RCSfile: image.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
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

#include <string.h>

#define _SV_IMAGE_CXX

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_RC_HXX
#include <rc.hxx>
#endif
#ifndef _SV_RESMGR_HXX
#include <resmgr.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_IMAGE_H
#include <image.h>
#endif
#define private public
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif
#undef private

// =======================================================================

DBG_NAME( Image );
DBG_NAME( ImageList );

#define IMAGE_FILE_VERSION      100

// =======================================================================

ImplImageList::~ImplImageList()
{
    if ( mpImageBitmap )
        delete mpImageBitmap;
    delete mpAry;
}

// =======================================================================

ImplImageRefData::~ImplImageRefData()
{
    mpImplData->mnIRefCount--;
    if ( mpImplData->mnRefCount || mpImplData->mnIRefCount )
    {
        mpImplData->mpAry[mnIndex].mnRefCount--;
        if ( !mpImplData->mpAry[mnIndex].mnRefCount )
            mpImplData->mnRealCount--;
    }
    else
        delete mpImplData;
}

// -----------------------------------------------------------------------

BOOL ImplImageRefData::IsEqual( const ImplImageRefData& rData )
{
    if ( (mpImplData == rData.mpImplData) && (mnIndex == rData.mnIndex) )
        return TRUE;
    else
        return FALSE;
}

// =======================================================================

ImplImageData::ImplImageData( const Bitmap& rBmp, const Bitmap& rMaskBmp ) :
    maBmp( rBmp ),
    maMaskBmp( rMaskBmp )
{
    mbColor         = FALSE;
    mpImageBitmap   = NULL;
}

// -----------------------------------------------------------------------

ImplImageData::ImplImageData( const Bitmap& rBmp, const Color& rColor ) :
    maBmp( rBmp ),
    maColor( rColor )
{
    mbColor         = TRUE;
    mpImageBitmap   = NULL;
}

// -----------------------------------------------------------------------

ImplImageData::~ImplImageData()
{
    if ( mpImageBitmap )
        delete mpImageBitmap;
}

// -----------------------------------------------------------------------

BOOL ImplImageData::IsEqual( const ImplImageData& rData )
{
    if ( (maBmp == rData.maBmp) && (maMaskBmp == rData.maMaskBmp) &&
         (maColor == rData.maColor) && (mbColor == rData.mbColor) )
        return TRUE;
    else
        return FALSE;
}

// =======================================================================

ImplImage::~ImplImage()
{
    switch ( meType )
    {
        case IMAGETYPE_BITMAP:
            delete (Bitmap*)mpData;
            break;

        case IMAGETYPE_IMAGE:
            delete (ImplImageData*)mpData;
            break;

        case IMAGETYPE_IMAGEREF:
            delete (ImplImageRefData*)mpData;
            break;
    }
}

// =======================================================================

Image::Image()
{
    DBG_CTOR( Image, NULL );

    mpImplData = NULL;
}

// -----------------------------------------------------------------------

Image::Image( const ResId& rResId )
{
    DBG_CTOR( Image, NULL );

    rResId.SetRT( RSC_IMAGE );
    ResMgr* pResMgr = rResId.GetResMgr();
    if ( !pResMgr )
        pResMgr = Resource::GetResManager();

    if ( pResMgr->GetResource( rResId ) )
    {
        // Header ueberspringen
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        USHORT nObjMask = pResMgr->ReadShort();

        Bitmap  aImageBitmap;
        Bitmap  aMaskBitmap;
        Color   aMaskColor;
        if( nObjMask & RSC_IMAGE_IMAGEBITMAP )
        {
            aImageBitmap = Bitmap( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }
        if( nObjMask & RSC_IMAGE_MASKBITMAP )
        {
            aMaskBitmap = Bitmap( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }
        if( nObjMask & RSC_IMAGE_MASKCOLOR )
        {
            aMaskColor = Color( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }

        if ( !aImageBitmap )
            mpImplData = NULL;
        else
        {
            mpImplData = new ImplImage;
            mpImplData->mnRefCount = 1;
            if ( !aMaskBitmap )
            {
                if( nObjMask & RSC_IMAGE_MASKCOLOR )
                {
                    mpImplData->meType = IMAGETYPE_IMAGE;
                    mpImplData->mpData = new ImplImageData( aImageBitmap, aMaskColor );
                }
                else
                {
                    mpImplData->meType = IMAGETYPE_BITMAP;
                    mpImplData->mpData = new Bitmap( aImageBitmap );
                 }
            }
            else
            {
                mpImplData->meType = IMAGETYPE_IMAGE;
                mpImplData->mpData = new ImplImageData( aImageBitmap, aMaskBitmap );
            }
        }
    }
    else
    {
        DBG_ERROR( "Image::Image( const ResId& rResId ): No resource!" );
        mpImplData = NULL;
    }
}

// -----------------------------------------------------------------------

Image::Image( const Image& rImage )
{
    DBG_CTOR( Image, NULL );

    mpImplData = rImage.mpImplData;
    if ( mpImplData )
        mpImplData->mnRefCount++;
}

// -----------------------------------------------------------------------

Image::Image( const Bitmap& rBitmap )
{
    DBG_CTOR( Image, NULL );

    if ( !rBitmap )
        mpImplData = NULL;
    else
    {
        mpImplData = new ImplImage;
        mpImplData->mnRefCount = 1;
        mpImplData->meType = IMAGETYPE_BITMAP;
        mpImplData->mpData = new Bitmap( rBitmap );
    }
}

// -----------------------------------------------------------------------

Image::Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap )
{
    DBG_CTOR( Image, NULL );

    if ( !rBitmap )
        mpImplData = NULL;
    else
    {
        mpImplData = new ImplImage;
        mpImplData->mnRefCount = 1;
        if ( !rMaskBitmap )
        {
            mpImplData->meType = IMAGETYPE_BITMAP;
            mpImplData->mpData = new Bitmap( rBitmap );
        }
        else
        {
            mpImplData->meType = IMAGETYPE_IMAGE;
            mpImplData->mpData = new ImplImageData( rBitmap, rMaskBitmap );
        }
    }
}

// -----------------------------------------------------------------------

Image::Image( const Bitmap& rBitmap, const Color& rColor )
{
    DBG_CTOR( Image, NULL );

    if ( !rBitmap )
        mpImplData = NULL;
    else
    {
        mpImplData = new ImplImage;
        mpImplData->mnRefCount = 1;
        mpImplData->meType = IMAGETYPE_IMAGE;
        mpImplData->mpData = new ImplImageData( rBitmap, rColor );
    }
}

// -----------------------------------------------------------------------

Image::Image( const BitmapEx& rBitmapEx )
{
    DBG_CTOR( Image, NULL );

    const Bitmap aBmp( rBitmapEx.GetBitmap() );

    if( !aBmp )
        mpImplData = NULL;
    else
    {
        const Bitmap aMask( rBitmapEx.GetMask() );

        mpImplData = new ImplImage;
        mpImplData->mnRefCount = 1;

        if( !aMask )
        {
            mpImplData->meType = IMAGETYPE_BITMAP;
            mpImplData->mpData = new Bitmap( aBmp );
        }
        else
        {
            mpImplData->meType = IMAGETYPE_IMAGE;
            mpImplData->mpData = new ImplImageData( aBmp, aMask );
        }
    }
}

// -----------------------------------------------------------------------

Image::~Image()
{
    DBG_DTOR( Image, NULL );

    if ( mpImplData )
    {
        if ( mpImplData->mnRefCount > 1 )
            mpImplData->mnRefCount--;
        else
            delete mpImplData;
    }
}

// -----------------------------------------------------------------------

Size Image::GetSizePixel() const
{
    DBG_CHKTHIS( Image, NULL );

    if ( mpImplData )
    {
        switch ( mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
                return ((Bitmap*)mpImplData->mpData)->GetSizePixel();

            case IMAGETYPE_IMAGE:
                return ((ImplImageData*)mpImplData->mpData)->maBmp.GetSizePixel();

            case IMAGETYPE_IMAGEREF:
                return ((ImplImageRefData*)mpImplData->mpData)->mpImplData->maImageSize;
        }
    }

    return Size();
}

// -----------------------------------------------------------------------

Image& Image::operator=( const Image& rImage )
{
    DBG_CHKTHIS( Image, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    if ( rImage.mpImplData )
        rImage.mpImplData->mnRefCount++;

    // Abkoppeln
    if ( mpImplData )
    {
        if ( mpImplData->mnRefCount > 1 )
            mpImplData->mnRefCount--;
        else
            delete mpImplData;
    }

    // Neue Daten zuweisen
    mpImplData = rImage.mpImplData;

    return *this;
}

// -----------------------------------------------------------------------

BOOL Image::operator==( const Image& rImage ) const
{
    DBG_CHKTHIS( Image, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );

    if ( rImage.mpImplData == mpImplData )
        return TRUE;
    if ( !rImage.mpImplData || !mpImplData )
        return FALSE;

    if ( rImage.mpImplData->mpData == mpImplData->mpData )
        return TRUE;

    if ( rImage.mpImplData->meType == mpImplData->meType )
    {
        switch ( mpImplData->meType )
        {
            case IMAGETYPE_BITMAP:
                if ( *((Bitmap*)rImage.mpImplData->mpData) == *((Bitmap*)mpImplData->mpData) )
                    return TRUE;
                break;

            case IMAGETYPE_IMAGE:
                if ( ((ImplImageData*)rImage.mpImplData->mpData)->IsEqual( *((ImplImageData*)mpImplData->mpData) ) )
                    return TRUE;
                break;

            case IMAGETYPE_IMAGEREF:
                if ( ((ImplImageRefData*)rImage.mpImplData->mpData)->IsEqual( *((ImplImageRefData*)mpImplData->mpData) ) )
                    return TRUE;
                break;
        }
    }

    return FALSE;
}

// =======================================================================

static void ImplCopyImageListData( ImageList* pThis )
{
    if ( pThis->mpImplData->mnRefCount > 1 )
    {
        pThis->mpImplData->mnRefCount--;

        ImplImageList* pNewData = new ImplImageList;
        pNewData->mnRefCount    = 1;
        pNewData->mnIRefCount   = 0;
        pNewData->mnCount       = pThis->mpImplData->mnCount;
        pNewData->mnRealCount   = pThis->mpImplData->mnRealCount;
        pNewData->mnArySize     = pThis->mpImplData->mnArySize;
        pNewData->mpAry         = new ImageAryData[pNewData->mnArySize];
        pNewData->maImageSize   = pThis->mpImplData->maImageSize;
        pNewData->mpImageBitmap = new ImplImageBmp;
        pNewData->mpImageBitmap->Create( pNewData->maImageSize.Width(),
                                         pNewData->maImageSize.Height(),
                                         pNewData->mnArySize );
        memset( pNewData->mpAry, 0, pNewData->mnArySize*sizeof(ImageAryData) );

        USHORT i = 0;
        USHORT n = 0;
        while ( i < pThis->mpImplData->mnArySize )
        {
            // Nur die Images kopieren, die gebraucht werden
            if ( pThis->mpImplData->mpAry[i].mnId )
            {
                pNewData->mpAry[n].mnId         = pThis->mpImplData->mpAry[i].mnId;
                pNewData->mpAry[n].mnRefCount   = 1;
                pNewData->mpImageBitmap->Replace( n,
                                                  *(pThis->mpImplData->mpImageBitmap),
                                                  i );
                n++;
            }

            i++;
        }

        pThis->mpImplData = pNewData;
    }
}

// -----------------------------------------------------------------------

static void ImplBmpImageCreate( ImageList* pThis,
                               const Bitmap& rBitmap, const Bitmap& rMaskBmp,
                               const Color& rColor, BOOL bColor,
                               USHORT nInit, USHORT* mpIdAry = NULL,
                               USHORT nGrow = 4 )
{
    // Falls es sich um eine leere ImageListe handelt, dann Defaul-Werte
    // setzen und nichts machen
    if ( !nInit )
    {
        pThis->mpImplData  = NULL;
        pThis->mnInitSize = 1;
        pThis->mnGrowSize = nGrow;
        return;
    }

    DBG_ASSERT( !nInit || rBitmap.GetSizePixel().Width(),
                "ImageList::ImageList(): nInitSize != 0 and BmpSize.Width() == 0" );
    DBG_ASSERT( (rBitmap.GetSizePixel().Width() % nInit) == 0,
                "ImageList::ImageList(): BmpSize % nInitSize != 0" );
    DBG_ASSERT( !rMaskBmp || (rMaskBmp.GetSizePixel() == rBitmap.GetSizePixel()),
                "ImageList::ImageList(): BmpSize != MaskBmpSize" );
#ifdef DBG_UTIL
    if ( mpIdAry )
    {
        for ( USHORT n1 = 0; n1 < nInit; n1++ )
        {
            USHORT nId = mpIdAry[n1];
            if ( !nId )
            {
                DBG_ERROR( "ImageList::ImageList(): Id == 0" );
            }
            for ( USHORT n2 = 0; n2 < n1; n2++ )
            {
                if ( nId == mpIdAry[n2] )
                {
                    DBG_ERROR1( "ImageList::ImageList(): Double Id (%u)", nId );
                }
            }
        }
    }
#endif

    Size aBmpSize = rBitmap.GetSizePixel();
    pThis->mnInitSize            = nInit;
    pThis->mnGrowSize            = nGrow;
    pThis->mpImplData             = new ImplImageList;
    pThis->mpImplData->mnRefCount  = 1;
    pThis->mpImplData->mnIRefCount = 0;
    pThis->mpImplData->mnCount     = nInit;
    pThis->mpImplData->mnRealCount = nInit;
    pThis->mpImplData->mnArySize   = nInit;
    pThis->mpImplData->mpAry       = new ImageAryData[nInit];
    pThis->mpImplData->maImageSize = Size( aBmpSize.Width() / nInit, aBmpSize.Height() );

    for ( USHORT i = 0; i < nInit; i++ )
    {
        if ( mpIdAry )
            pThis->mpImplData->mpAry[i].mnId = mpIdAry[i];
        else
            pThis->mpImplData->mpAry[i].mnId = i+1;
        pThis->mpImplData->mpAry[i].mnRefCount = 1;
    }

    pThis->mpImplData->mpImageBitmap = new ImplImageBmp;
    pThis->mpImplData->mpImageBitmap->Create( rBitmap, rMaskBmp,
                                              rColor, bColor,
                                              pThis->mpImplData->maImageSize.Width(),
                                              pThis->mpImplData->maImageSize.Height(),
                                              nInit );
}

// =======================================================================

ImageList::ImageList( USHORT nInit, USHORT nGrow )
{
    DBG_CTOR( ImageList, NULL );

    mpImplData  = NULL;
    mnInitSize  = nInit;
    mnGrowSize  = nGrow;
}

// -----------------------------------------------------------------------

ImageList::ImageList( const ResId& rResId )
{
    DBG_CTOR( ImageList, NULL );

    rResId.SetRT( RSC_IMAGELIST );
    ResMgr* pResMgr = rResId.GetResMgr();
    if ( !pResMgr )
        pResMgr = Resource::GetResManager();

    if ( pResMgr->GetResource( rResId ) )
    {
        // Header ueberspringen
        pResMgr->Increment( sizeof( RSHEADER_TYPE ) );

        USHORT nObjMask = pResMgr->ReadShort();

        Bitmap  aImageBitmap;
        Bitmap  aMaskBitmap;
        Color   aMaskColor;
        BOOL    bCol        = FALSE;
        BOOL    bIsIdList   = FALSE;

        if ( nObjMask & RSC_IMAGELIST_IMAGEBITMAP )
        {
            aImageBitmap = Bitmap( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }
        if ( nObjMask & RSC_IMAGELIST_MASKBITMAP )
        {
            aMaskBitmap = Bitmap( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
        }
        if ( nObjMask & RSC_IMAGELIST_MASKCOLOR )
        {
            aMaskColor = Color( ResId( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            pResMgr->Increment( pResMgr->GetObjSize( (RSHEADER_TYPE*)pResMgr->GetClass() ) );
            bCol = TRUE;
        }
        if ( nObjMask & RSC_IMAGELIST_IDLIST )
        {
            bIsIdList = TRUE;
            USHORT nCount = pResMgr->ReadShort();
            USHORT* pAry = new USHORT[ nCount ];
            for( int i = 0; i < nCount; i++ )
                pAry[ i ] = pResMgr->ReadShort();
            ImplBmpImageCreate( this, aImageBitmap, aMaskBitmap, aMaskColor,
                                bCol, nCount, pAry, 4 );
            delete pAry;
        }
        if ( nObjMask & RSC_IMAGELIST_IDCOUNT )
        {
            USHORT nCount = pResMgr->ReadShort();
            if ( !bIsIdList )
            {
                ImplBmpImageCreate( this, aImageBitmap, aMaskBitmap, aMaskColor,
                                    bCol, nCount, NULL, 4 );
            }
        }
    }
}

// -----------------------------------------------------------------------

ImageList::ImageList( const ImageList& rImageList )
{
    DBG_CTOR( ImageList, NULL );

    mpImplData = rImageList.mpImplData;
    if ( mpImplData )
        mpImplData->mnRefCount++;
}

// -----------------------------------------------------------------------

ImageList::ImageList( const Bitmap& rBitmap,
                      USHORT nInit, USHORT* mpIdAry, USHORT nGrow )
{
    DBG_CTOR( ImageList, NULL );

    ImplBmpImageCreate( this, rBitmap, Bitmap(), Color(), FALSE,
                       nInit, mpIdAry, nGrow );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const Bitmap& rBitmap, const Bitmap& rMaskBmp,
                      USHORT nInit, USHORT* mpIdAry, USHORT nGrow )
{
    DBG_CTOR( ImageList, NULL );

    ImplBmpImageCreate( this, rBitmap, rMaskBmp, Color(), FALSE,
                        nInit, mpIdAry, nGrow );
}

// -----------------------------------------------------------------------

ImageList::ImageList( const Bitmap& rBitmap, const Color& rColor,
                      USHORT nInit, USHORT* mpIdAry, USHORT nGrow )
{
    DBG_CTOR( ImageList, NULL );

    ImplBmpImageCreate( this, rBitmap, Bitmap(), rColor, TRUE,
                        nInit, mpIdAry, nGrow );
}

// -----------------------------------------------------------------------

ImageList::~ImageList()
{
    DBG_DTOR( ImageList, NULL );

    if ( mpImplData )
    {
        mpImplData->mnRefCount--;
        if ( !mpImplData->mnRefCount && !mpImplData->mnIRefCount )
            delete mpImplData;
    }
}

// -----------------------------------------------------------------------

void ImageList::AddImage( USHORT nId, const Image& rImage )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( nId, "ImageList::AddImage(): ImageId == 0" );
    DBG_ASSERT( GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::AddImage() - ImageId already exists" );
    DBG_ASSERT( rImage.mpImplData, "ImageList::AddImage(): Wrong Size" );
    DBG_ASSERT( !mpImplData || (rImage.GetSizePixel() == mpImplData->maImageSize),
                "ImageList::AddImage(): Wrong Size" );

    ImageType   eImageType = rImage.mpImplData->meType;
    Size        aImageSize = rImage.GetSizePixel();
    USHORT      nIndex;

    if ( !mpImplData )
    {
        mpImplData                  = new ImplImageList;
        mpImplData->mnRefCount      = 1;
        mpImplData->mnIRefCount     = 0;
        mpImplData->mnCount         = 0;
        mpImplData->mnRealCount     = 0;
        mpImplData->mnArySize       = mnInitSize;
        mpImplData->mpAry           = new ImageAryData[mnInitSize];
        mpImplData->maImageSize     = aImageSize;
        mpImplData->mpImageBitmap   = new ImplImageBmp;
        mpImplData->mpImageBitmap->Create( aImageSize.Width(), aImageSize.Height(),
                                           mnInitSize );
        memset( mpImplData->mpAry, 0, mpImplData->mnArySize*sizeof(ImageAryData) );
    }
    else
        ImplCopyImageListData( this );

    // Gegebenenfalls unser Array erweitern und freien Index ermitteln
    if ( mpImplData->mnRealCount == mpImplData->mnArySize )
    {
        ImageAryData*   pOldAry  = mpImplData->mpAry;
        USHORT          nOldSize = mpImplData->mnArySize;

        mpImplData->mnArySize += mnGrowSize;
        mpImplData->mpAry = new ImageAryData[mpImplData->mnArySize];
        memset( mpImplData->mpAry, 0, mpImplData->mnArySize*sizeof(ImageAryData) );
        memcpy( mpImplData->mpAry, pOldAry, nOldSize*sizeof(ImageAryData) );
        mpImplData->mpImageBitmap->Expand( mnGrowSize );
        delete pOldAry;

        nIndex = mpImplData->mnRealCount;
    }
    else
    {
        nIndex = 0;
        while ( mpImplData->mpAry[nIndex].mnRefCount )
            nIndex++;
    }

    // Image in Bitmap einfuegen
    switch ( eImageType )
    {
        case IMAGETYPE_BITMAP:
            mpImplData->mpImageBitmap->Replace( nIndex, *((Bitmap*)rImage.mpImplData->mpData) );
            break;

        case IMAGETYPE_IMAGE:
            {
            ImplImageData* pData = (ImplImageData*)rImage.mpImplData->mpData;
            if ( pData->mpImageBitmap )
                mpImplData->mpImageBitmap->Replace( nIndex, *(pData->mpImageBitmap), 0 );
            else
            {
                if ( pData->mbColor )
                    mpImplData->mpImageBitmap->Replace( nIndex, pData->maBmp, pData->maColor );
                else
                    mpImplData->mpImageBitmap->Replace( nIndex, pData->maBmp, pData->maMaskBmp );
            }
            }
            break;

        case IMAGETYPE_IMAGEREF:
            {
            ImplImageRefData* pData = (ImplImageRefData*)rImage.mpImplData->mpData;
            mpImplData->mpImageBitmap->Replace( nIndex, *(pData->mpImplData->mpImageBitmap),
                                                pData->mnIndex );
            }
            break;
    }

    // Array-Daten updaten
    mpImplData->mnCount++;
    mpImplData->mnRealCount++;
    mpImplData->mpAry[nIndex].mnId          = nId;
    mpImplData->mpAry[nIndex].mnRefCount    = 1;
}

// -----------------------------------------------------------------------

void ImageList::CopyImage( USHORT nId, USHORT nCopyId )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_ASSERT( nId, "ImageList::CopyImage(): ImageId == 0" );
    DBG_ASSERT( GetImagePos( nId ) == IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::CopyImage(): ImageId already exists" );
    DBG_ASSERT( GetImagePos( nCopyId ) != IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::CopyImage(): Unknown nCopyId" );

    USHORT  nIndex;
    USHORT  nCopyIndex = 0;

    // Index von CopyId holen
    while ( nCopyIndex < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nCopyIndex].mnId == nCopyId )
            break;

        nCopyIndex++;
    }
    if ( nCopyIndex >= mpImplData->mnArySize )
        return;

    // Referenz-Counter ueberpruefen
    ImplCopyImageListData( this );

    // Gegebenenfalls unser Array erweitern
    if ( mpImplData->mnRealCount == mpImplData->mnArySize )
    {
        ImageAryData*   pOldAry  = mpImplData->mpAry;
        USHORT          nOldSize = mpImplData->mnArySize;

        mpImplData->mnArySize += mnGrowSize;
        mpImplData->mpAry = new ImageAryData[mpImplData->mnArySize];
        memset( mpImplData->mpAry, 0, mpImplData->mnArySize*sizeof(ImageAryData) );
        memcpy( mpImplData->mpAry, pOldAry, nOldSize*sizeof(ImageAryData) );
        mpImplData->mpImageBitmap->Expand( mnGrowSize );
        delete pOldAry;

        nIndex = mpImplData->mnRealCount;
    }
    else
    {
        nIndex = 0;
        while ( mpImplData->mpAry[nIndex].mnRefCount )
            nIndex++;
    }

    // Kopieren
    mpImplData->mpImageBitmap->Replace( nIndex, *(mpImplData->mpImageBitmap), nCopyIndex );

    // Array-Daten updaten
    mpImplData->mnCount++;
    mpImplData->mnRealCount++;
    mpImplData->mpAry[nIndex].mnId          = nId;
    mpImplData->mpAry[nIndex].mnRefCount    = 1;
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( USHORT nId, const Image& rImage )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::ReplaceImage(): Unknown nId" );

    RemoveImage( nId );
    AddImage( nId, rImage );
}

// -----------------------------------------------------------------------

void ImageList::ReplaceImage( USHORT nId, USHORT nReplaceId )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_ASSERT( GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::ReplaceImage(): Unknown nId" );
    DBG_ASSERT( GetImagePos( nReplaceId ) != IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::ReplaceImage(): Unknown nReplaceId" );

    USHORT  nPos1 = 0;
    USHORT  nPos2 = 0;

    // Index von Id holen
    while ( nPos1 < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nPos1].mnId == nId )
            break;

        nPos1++;
    }
    if ( nPos1 >= mpImplData->mnArySize )
        return;

    // Index von ReplaceId holen
    while ( nPos2 < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nPos2].mnId == nReplaceId )
            break;

        nPos2++;
    }
    if ( nPos2 >= mpImplData->mnArySize )
        return;

    // Referenz-Counter ueberpruefen
    ImplCopyImageListData( this );

    // Ersetzen
    mpImplData->mpImageBitmap->Replace( nPos1, nPos2 );
}

// -----------------------------------------------------------------------

void ImageList::MergeImage( USHORT nId, USHORT nMergeId )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_ASSERT( GetImagePos( nId ) != IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::MergeImage(): Unknown nId" );
    DBG_ASSERT( GetImagePos( nMergeId ) != IMAGELIST_IMAGE_NOTFOUND,
                "ImageList::MergeImage(): Unknown nMergeId" );

    USHORT  nPos1 = 0;
    USHORT  nPos2 = 0;

    // Index von Id holen
    while ( nPos1 < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nPos1].mnId == nId )
            break;

        nPos1++;
    }
    if ( nPos1 >= mpImplData->mnArySize )
        return;

    // Index von MergeId holen
    while ( nPos2 < mpImplData->mnArySize )
    {
        if ( mpImplData->mpAry[nPos2].mnId == nMergeId )
            break;

        nPos2++;
    }
    if ( nPos2 >= mpImplData->mnArySize )
        return;

    // Referenz-Counter ueberpruefen
    ImplCopyImageListData( this );

    // Ersetzen
    mpImplData->mpImageBitmap->Merge( nPos1, nPos2 );
}

// -----------------------------------------------------------------------

void ImageList::RemoveImage( USHORT nId )
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
    {
        ImplCopyImageListData( this );

        USHORT i = 0;
        while ( i < mpImplData->mnArySize )
        {
            if ( mpImplData->mpAry[i].mnId == nId )
                break;

            i++;
        }

        if ( i < mpImplData->mnArySize )
        {
            mpImplData->mpAry[i].mnRefCount--;
            mpImplData->mpAry[i].mnId = 0;
            if ( !mpImplData->mpAry[i].mnRefCount )
                mpImplData->mnRealCount--;

            mpImplData->mnCount--;
        }
    }
}

// -----------------------------------------------------------------------

Image ImageList::GetImage( USHORT nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    Image aImage;

    if ( mpImplData )
    {
        USHORT i = 0;
        while ( i < mpImplData->mnArySize )
        {
            if ( mpImplData->mpAry[i].mnId == nId )
                break;

            i++;
        }

        if ( i < mpImplData->mnArySize )
        {
            ImplImageRefData* mpData = new ImplImageRefData;

            mpImplData->mnIRefCount++;
            mpImplData->mpAry[i].mnRefCount++;
            mpData->mpImplData = mpImplData;
            mpData->mnIndex = i;

            aImage.mpImplData = new ImplImage;
            aImage.mpImplData->mnRefCount = 1;
            aImage.mpImplData->meType = IMAGETYPE_IMAGEREF;
            aImage.mpImplData->mpData = mpData;
        }
    }

    return aImage;
}

// -----------------------------------------------------------------------

void ImageList::Clear()
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
    {
        if ( mpImplData->mnRefCount > 1 )
            mpImplData->mnRefCount--;
        else
            delete mpImplData;
    }

    mpImplData = 0;
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImageCount() const
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
        return mpImplData->mnCount;
    else
        return 0;
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImagePos( USHORT nId ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData && nId )
    {
        USHORT nPos = 0;
        USHORT i = 0;
        while ( i < mpImplData->mnArySize )
        {
            if ( mpImplData->mpAry[i].mnId == nId )
                return nPos;

            if ( mpImplData->mpAry[i].mnId )
                nPos++;
            i++;
        }
    }

    return IMAGELIST_IMAGE_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ImageList::GetImageId( USHORT nPos ) const
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
    {
        USHORT nRealPos = 0;
        USHORT i = 0;
        while ( i < mpImplData->mnArySize )
        {
            if ( (nPos == nRealPos) && (mpImplData->mpAry[i].mnId) )
                return mpImplData->mpAry[i].mnId;

            if ( mpImplData->mpAry[i].mnId )
                nRealPos++;
            i++;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

Size ImageList::GetImageSize() const
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
        return mpImplData->maImageSize;
    else
        return Size();
}

// -----------------------------------------------------------------------

Bitmap ImageList::GetBitmap() const
{
    DBG_CHKTHIS( ImageList, NULL );

    Bitmap aBmp;

    if ( mpImplData )
    {
        // Positionen ermitteln, die in der Bitmap enthalten sein sollen
        USHORT* mpPosAry = new USHORT[mpImplData->mnCount];
        USHORT  nPosCount = 0;
        for ( USHORT i = 0; i < mpImplData->mnArySize; i++ )
        {
            if ( mpImplData->mpAry[i].mnId )
            {
                mpPosAry[nPosCount] = i;
                nPosCount++;
            }
        }

        // Bitmap besorgen
        aBmp = mpImplData->mpImageBitmap->GetBitmap( nPosCount, mpPosAry );

        // Temporaeres Array loeschen
        delete mpPosAry;
    }

    return aBmp;
}

// -----------------------------------------------------------------------

Bitmap ImageList::GetMaskBitmap() const
{
    DBG_CHKTHIS( ImageList, NULL );

    Bitmap aBmp;

    if ( HasMaskBitmap() )
    {
        // Positionen ermitteln, die in der Bitmap enthalten sein sollen
        USHORT* mpPosAry = new USHORT[mpImplData->mnCount];
        USHORT  nPosCount = 0;
        for ( USHORT i = 0; i < mpImplData->mnArySize; i++ )
        {
            if ( mpImplData->mpAry[i].mnId )
            {
                mpPosAry[nPosCount] = i;
                nPosCount++;
            }
        }

        // Bitmap besorgen
        aBmp = mpImplData->mpImageBitmap->GetMaskBitmap( nPosCount, mpPosAry );

        // Temporaeres Array loeschen
        delete mpPosAry;
    }

    return aBmp;
}

// -----------------------------------------------------------------------

BOOL ImageList::HasMaskBitmap() const
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
        return mpImplData->mpImageBitmap->HasMaskBitmap();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

Color ImageList::GetMaskColor() const
{
    DBG_CHKTHIS( ImageList, NULL );

    Color aColor;

    if ( HasMaskColor() )
        aColor = mpImplData->mpImageBitmap->GetMaskColor();

    return aColor;
}

// -----------------------------------------------------------------------

BOOL ImageList::HasMaskColor() const
{
    DBG_CHKTHIS( ImageList, NULL );

    if ( mpImplData )
        return mpImplData->mpImageBitmap->HasMaskColor();
    else
        return FALSE;
}

// -----------------------------------------------------------------------

ImageList& ImageList::operator=( const ImageList& rImageList )
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    // Zuerst Referenzcounter erhoehen, damit man sich selbst zuweisen kann
    if ( rImageList.mpImplData )
        rImageList.mpImplData->mnRefCount++;

    // Abkoppeln
    if ( mpImplData )
    {
        mpImplData->mnRefCount--;
        if ( !mpImplData->mnRefCount && !mpImplData->mnIRefCount )
            delete mpImplData;
    }

    // Neue Daten zuweisen
    mpImplData = rImageList.mpImplData;
    mnInitSize = rImageList.mnInitSize;
    mnGrowSize = rImageList.mnGrowSize;

    return *this;
}

// -----------------------------------------------------------------------

BOOL ImageList::operator==( const ImageList& rImageList ) const
{
    DBG_CHKTHIS( ImageList, NULL );
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    if ( rImageList.mpImplData == mpImplData )
        return TRUE;
    if ( !rImageList.mpImplData || !mpImplData )
        return FALSE;

    if ( (rImageList.mpImplData->mnCount == mpImplData->mnCount) &&
         (rImageList.mpImplData->maImageSize == mpImplData->maImageSize) )
        return TRUE;

    return FALSE;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStream, ImageList& rImageList )
{
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    // Falls es eine bestehende ImageListe ist, dann erst abkoppeln
    if ( rImageList.mpImplData )
    {
        rImageList.mpImplData->mnRefCount--;
        if ( !rImageList.mpImplData->mnRefCount && !rImageList.mpImplData->mnIRefCount )
            delete rImageList.mpImplData;
    }
    rImageList.mpImplData = NULL;

    // Daten lesen
    USHORT  nVersion;
    Size    aImageSize;
    BOOL    bImageList;
    rIStream >> nVersion;
    rIStream >> rImageList.mnInitSize;
    rIStream >> rImageList.mnGrowSize;
    rIStream >> bImageList;

    // Wenn es eine leere ImageListe ist, dann brauchen wir nicht weiter lesen
    if ( !bImageList )
        return rIStream;

    // Image-Groesse lesen
    rIStream >> aImageSize.Width();
    rIStream >> aImageSize.Height();

    // Image-Daten anlegen und initialisieren
    rImageList.mpImplData               = new ImplImageList;
    rImageList.mpImplData->mnRefCount   = 1;
    rImageList.mpImplData->mnIRefCount  = 0;
    rImageList.mpImplData->mnCount      = rImageList.mnInitSize;
    rImageList.mpImplData->mnRealCount  = rImageList.mnInitSize;
    rImageList.mpImplData->mnArySize    = rImageList.mnInitSize;
    rImageList.mpImplData->mpAry        = new ImageAryData[rImageList.mnInitSize];
    rImageList.mpImplData->maImageSize  = aImageSize;

    // Array mit ID's lesen und initialisieren
    for ( USHORT i = 0; i < rImageList.mnInitSize; i++ )
    {
        rIStream >> rImageList.mpImplData->mpAry[i].mnId;
        rImageList.mpImplData->mpAry[i].mnRefCount = 1;
    }

    // Bitmaps lesen
    Bitmap  aBitmap;
    Bitmap  aMaskBitmap;
    Color   aMaskColor;
    BYTE    bMaskBitmap;
    BYTE    bMaskColor;
    rIStream >> aBitmap;
    rIStream >> bMaskBitmap;
    if ( bMaskBitmap )
        rIStream >> aMaskBitmap;
    rIStream >> bMaskColor;
    if ( bMaskColor )
        rIStream >> aMaskColor;

    // Systemdaten anlegen
    rImageList.mpImplData->mpImageBitmap = new ImplImageBmp;
    rImageList.mpImplData->mpImageBitmap->Create( aBitmap, aMaskBitmap,
                                                  aMaskColor, bMaskColor,
                                                  aImageSize.Width(),
                                                  aImageSize.Height(),
                                                  rImageList.mnInitSize );
    return rIStream;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStream, const ImageList& rImageList )
{
    DBG_CHKOBJ( &rImageList, ImageList, NULL );

    BOOL bImageList = (rImageList.mpImplData) ? TRUE : FALSE;

    USHORT nVersion = IMAGE_FILE_VERSION;
    rOStream << nVersion;

    // Wenn es eine leere ImageListe ist, dann nur InitSize und
    // GrowSize schreiben
    if ( !bImageList || !rImageList.mpImplData->mnCount )
    {
        BOOL bSaveImageList = FALSE;
        rOStream << rImageList.mnInitSize;
        rOStream << rImageList.mnGrowSize;
        rOStream << bSaveImageList;
        return rOStream;
    }

    // Normale Daten schreiben
    rOStream << rImageList.mpImplData->mnCount;
    rOStream << rImageList.mnGrowSize;
    rOStream << bImageList;
    rOStream << rImageList.mpImplData->maImageSize.Width();
    rOStream << rImageList.mpImplData->maImageSize.Height();

    // Array schreiben und feststellen, welche Eintraege gespeichert werden
    // muessen
    USHORT* mpPosAry = new USHORT[rImageList.mpImplData->mnCount];
    USHORT  nPosCount = 0;
    for ( USHORT i = 0; i < rImageList.mpImplData->mnArySize; i++ )
    {
        if ( rImageList.mpImplData->mpAry[i].mnId )
        {
            rOStream << rImageList.mpImplData->mpAry[i].mnId;
            mpPosAry[nPosCount] = i;
            nPosCount++;
        }
    }

    // Bitmaps rausschreiben
    Bitmap aBmp;
    BYTE bMaskBitmap = (BYTE)rImageList.mpImplData->mpImageBitmap->HasMaskBitmap();
    BYTE bMaskColor = (BYTE)rImageList.mpImplData->mpImageBitmap->HasMaskColor();
    aBmp = rImageList.mpImplData->mpImageBitmap->GetBitmap( nPosCount, mpPosAry );
    rOStream << aBmp;
    rOStream << bMaskBitmap;
    if ( bMaskBitmap )
    {
        aBmp = rImageList.mpImplData->mpImageBitmap->GetMaskBitmap( nPosCount, mpPosAry );
        rOStream << aBmp;
    }
    rOStream << bMaskColor;
    if ( bMaskColor )
    {
        Color aColor = rImageList.mpImplData->mpImageBitmap->GetMaskColor();
        rOStream << aColor;
    }

    // Temporaeres Array loeschen
    delete mpPosAry;

    return rOStream;
}

// =======================================================================

void OutputDevice::DrawImage( const Point& rPos, const Image& rImage,
                              USHORT nStyle )
{
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( GetOutDevType() != OUTDEV_PRINTER,
                "DrawImage(): Images can't be drawn on any mprinter" );

    if( !rImage.mpImplData )
        return;

    switch( rImage.mpImplData->meType )
    {
        case IMAGETYPE_BITMAP:
        {
            DrawBitmap( rPos, *((Bitmap*)rImage.mpImplData->mpData) );
        }
        break;

        case IMAGETYPE_IMAGE:
        {
            ImplImageData* pData = (ImplImageData*)rImage.mpImplData->mpData;

            if ( !pData->mpImageBitmap )
            {
                Size aSize = pData->maBmp.GetSizePixel();
                pData->mpImageBitmap = new ImplImageBmp;
                pData->mpImageBitmap->Create( pData->maBmp, pData->maMaskBmp,
                                              pData->maColor, pData->mbColor,
                                              aSize.Width(), aSize.Height(),
                                              1 );
            }

            pData->mpImageBitmap->Draw( 0, this, rPos, nStyle );
        }
        break;

        case IMAGETYPE_IMAGEREF:
        {
            ImplImageRefData* pData = (ImplImageRefData*)rImage.mpImplData->mpData;
            pData->mpImplData->mpImageBitmap->Draw( pData->mnIndex, this, rPos, nStyle );
        }
        break;
    }
}

// =======================================================================

void OutputDevice::DrawImage( const Point& rPos, const Size& rSize,
                              const Image& rImage, USHORT nStyle )
{
    DBG_CHKOBJ( &rImage, Image, NULL );
    DBG_ASSERT( GetOutDevType() != OUTDEV_PRINTER,
                "DrawImage(): Images can't be drawn on any mprinter" );

    if( !rImage.mpImplData )
        return;

    switch( rImage.mpImplData->meType )
    {
        case IMAGETYPE_BITMAP:
        {
            DrawBitmap( rPos, rSize, *((Bitmap*)rImage.mpImplData->mpData) );
        }
        break;

        case IMAGETYPE_IMAGE:
        {
            ImplImageData* pData = (ImplImageData*)rImage.mpImplData->mpData;

            if ( !pData->mpImageBitmap )
            {
                Size aSize = pData->maBmp.GetSizePixel();
                pData->mpImageBitmap = new ImplImageBmp;
                pData->mpImageBitmap->Create( pData->maBmp, pData->maMaskBmp,
                                              pData->maColor, pData->mbColor,
                                              aSize.Width(), aSize.Height(),
                                              1 );
            }

            pData->mpImageBitmap->Draw( 0, this, rPos, nStyle, &rSize );
        }
        break;

        case IMAGETYPE_IMAGEREF:
        {
            ImplImageRefData* pData = (ImplImageRefData*)rImage.mpImplData->mpData;
            pData->mpImplData->mpImageBitmap->Draw( pData->mnIndex, this, rPos, nStyle, &rSize );
        }
        break;
    }
}
