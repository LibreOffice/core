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

#ifndef _SV_IMAGE_HXX
#define _SV_IMAGE_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/resid.hxx>
#include <vcl/sv.h>
#include <vcl/bitmapex.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <vector>

struct ImplImage;
struct ImplImageList;
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic;} } } }

// -----------
// - Defines -
// -----------

#define IMAGE_STDBTN_COLOR          Color( 0xC0, 0xC0, 0xC0 )
#define IMAGELIST_IMAGE_NOTFOUND    ((sal_uInt16)0xFFFF)

// -----------------------
// - ImageColorTransform -
// -----------------------

enum ImageColorTransform
{
    IMAGECOLORTRANSFORM_NONE = 0,
    IMAGECOLORTRANSFORM_HIGHCONTRAST = 1,
    IMAGECOLORTRANSFORM_MONOCHROME_BLACK = 2,
    IMAGECOLORTRANSFORM_MONOCHROME_WHITE = 3
};

// ---------
// - Image -
// ---------

class VCL_DLLPUBLIC Image
{
    friend class ImageList;
    friend class OutputDevice;

public:
                    Image();
                    Image( const ResId& rResId );
                    Image( const Image& rImage );
                    Image( const BitmapEx& rBitmapEx );
                    Image( const Bitmap& rBitmap );
                    Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap );
                    Image( const Bitmap& rBitmap, const Color& rColor );
                    Image( const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rxGraphic );
                    ~Image();

    Size            GetSizePixel() const;

    BitmapEx        GetBitmapEx() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > GetXGraphic() const;

    Image           GetColorTransformedImage( ImageColorTransform eColorTransform ) const;
    static void     GetColorTransformArrays( ImageColorTransform eColorTransform, Color*& rpSrcColor, Color*& rpDstColor, sal_uLong& rColorCount );

    void            Invert();

    sal_Bool            operator!() const { return( !mpImplData ? true : false ); }
    Image&          operator=( const Image& rImage );
    sal_Bool            operator==( const Image& rImage ) const;
    sal_Bool            operator!=( const Image& rImage ) const { return !(Image::operator==( rImage )); }

private:

    ImplImage*             mpImplData;

    SAL_DLLPRIVATE void    ImplInit( const BitmapEx& rBmpEx );
};

// -------------
// - ImageList -
// -------------

class VCL_DLLPUBLIC ImageList
{
public:
                    ImageList( sal_uInt16 nInit = 8, sal_uInt16 nGrow = 4 );
                    ImageList( const ResId& rResId );
                    ImageList( const ::std::vector< ::rtl::OUString >& rNameVector,
                               const ::rtl::OUString& rPrefix,
                               const Color* pMaskColor = NULL );
                    ImageList( const ImageList& rImageList );
                    ~ImageList();

    void                Clear();
    void                    InsertFromHorizontalStrip( const BitmapEx &rBitmapEx,
                                   const std::vector< rtl::OUString > &rNameVector );
    void                    InsertFromHorizontalBitmap( const ResId& rResId,
                                    sal_uInt16       nCount,
                                    const Color *pNonAlphaMaskColor,
                                    const Color *pSearchColors = NULL,
                                    const Color *pReplaceColors = NULL,
                                    sal_uLong        nColorCount = 0);
    BitmapEx        GetAsHorizontalStrip() const;
    sal_uInt16          GetImageCount() const;
    Size            GetImageSize() const;

    void            AddImage( sal_uInt16 nNewId, const Image& rImage );
    void            AddImage( const ::rtl::OUString& rImageName, const Image& rImage );

    void            ReplaceImage( sal_uInt16 nId, const Image& rImage );
    void            ReplaceImage( const ::rtl::OUString& rImageName, const Image& rImage );

    void            ReplaceImage( sal_uInt16 nId, sal_uInt16 nReplaceId );
    void            ReplaceImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rReplaceName );

    void            RemoveImage( sal_uInt16 nId );
    void            RemoveImage( const ::rtl::OUString& rImageName );

    Image           GetImage( sal_uInt16 nId ) const;
    Image           GetImage( const ::rtl::OUString& rImageName ) const;

    sal_uInt16          GetImagePos( sal_uInt16 nId ) const;
    bool            HasImageAtPos( sal_uInt16 nId ) const;
    sal_uInt16          GetImagePos( const ::rtl::OUString& rImageName ) const;

    sal_uInt16          GetImageId( sal_uInt16 nPos ) const;
    void            GetImageIds( ::std::vector< sal_uInt16 >& rIds ) const;

    ::rtl::OUString GetImageName( sal_uInt16 nPos ) const;
    void            GetImageNames( ::std::vector< ::rtl::OUString >& rNames ) const;

    ImageList&      operator=( const ImageList& rImageList );
    sal_Bool            operator==( const ImageList& rImageList ) const;
    sal_Bool            operator!=( const ImageList& rImageList ) const { return !(ImageList::operator==( rImageList )); }

private:

    ImplImageList*  mpImplData;
    sal_uInt16          mnInitSize;
    sal_uInt16          mnGrowSize;

    SAL_DLLPRIVATE void    ImplInitBitmapEx( const ::rtl::OUString& rUserImageName,
                                             const ::std::vector< ::rtl::OUString >& rImageNames,
                                             const ::rtl::OUString& rSymbolsStyle,
                                             BitmapEx& rBmpEx,
                                             const Color* pMaskColor ) const;
    SAL_DLLPRIVATE void    ImplInit( sal_uInt16 nItems, const Size &rSize );
    SAL_DLLPRIVATE sal_uInt16  ImplGetImageId( const ::rtl::OUString& rImageName ) const;
    SAL_DLLPRIVATE void    ImplMakeUnique();
};

#endif  // _SV_IMAGE_HXX
