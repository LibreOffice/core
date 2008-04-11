/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: image.hxx,v $
 * $Revision: 1.4 $
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
#define IMAGELIST_IMAGE_NOTFOUND    ((USHORT)0xFFFF)

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
    static void     GetColorTransformArrays( ImageColorTransform eColorTransform, Color*& rpSrcColor, Color*& rpDstColor, ULONG& rColorCount );

    void            Invert();

    BOOL            operator!() const { return( !mpImplData ? true : false ); }
    Image&          operator=( const Image& rImage );
    BOOL            operator==( const Image& rImage ) const;
    BOOL            operator!=( const Image& rImage ) const { return !(Image::operator==( rImage )); }

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
                    ImageList( USHORT nInit = 8, USHORT nGrow = 4 );
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
                                    USHORT       nCount,
                                    const Color *pNonAlphaMaskColor,
                                    const Color *pSearchColors = NULL,
                                    const Color *pReplaceColors = NULL,
                                    ULONG        nColorCount = 0);
    BitmapEx        GetAsHorizontalStrip() const;
    USHORT          GetImageCount() const;
    Size            GetImageSize() const;

    void            AddImage( USHORT nNewId, const Image& rImage );
    void            AddImage( const ::rtl::OUString& rImageName, const Image& rImage );

    void            ReplaceImage( USHORT nId, const Image& rImage );
    void            ReplaceImage( const ::rtl::OUString& rImageName, const Image& rImage );

    void            ReplaceImage( USHORT nId, USHORT nReplaceId );
    void            ReplaceImage( const ::rtl::OUString& rImageName, const ::rtl::OUString& rReplaceName );

    void            RemoveImage( USHORT nId );
    void            RemoveImage( const ::rtl::OUString& rImageName );

    Image           GetImage( USHORT nId ) const;
    Image           GetImage( const ::rtl::OUString& rImageName ) const;

    USHORT          GetImagePos( USHORT nId ) const;
    bool            HasImageAtPos( USHORT nId ) const;
    USHORT          GetImagePos( const ::rtl::OUString& rImageName ) const;

    USHORT          GetImageId( USHORT nPos ) const;
    void            GetImageIds( ::std::vector< USHORT >& rIds ) const;

    ::rtl::OUString GetImageName( USHORT nPos ) const;
    void            GetImageNames( ::std::vector< ::rtl::OUString >& rNames ) const;

    ImageList&      operator=( const ImageList& rImageList );
    BOOL            operator==( const ImageList& rImageList ) const;
    BOOL            operator!=( const ImageList& rImageList ) const { return !(ImageList::operator==( rImageList )); }

private:

    ImplImageList*  mpImplData;
    USHORT          mnInitSize;
    USHORT          mnGrowSize;

    SAL_DLLPRIVATE void    ImplInitBitmapEx( const ::rtl::OUString& rUserImageName,
                                             const ::std::vector< ::rtl::OUString >& rImageNames,
                                             const ::rtl::OUString& rSymbolsStyle,
                                             BitmapEx& rBmpEx,
                                             const Color* pMaskColor ) const;
    SAL_DLLPRIVATE void    ImplInit( USHORT nItems, const Size &rSize );
    SAL_DLLPRIVATE USHORT  ImplGetImageId( const ::rtl::OUString& rImageName ) const;
    SAL_DLLPRIVATE void    ImplMakeUnique();
};

#endif  // _SV_IMAGE_HXX
