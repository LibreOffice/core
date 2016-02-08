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

#ifndef INCLUDED_VCL_IMAGE_HXX
#define INCLUDED_VCL_IMAGE_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/resid.hxx>
#include <tools/solar.h>
#include <vcl/bitmapex.hxx>
#include <vcl/outdev.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <vector>

struct ImplImage;
struct ImplImageList;
namespace com { namespace sun { namespace star { namespace graphic { class XGraphic;} } } }

#define IMAGE_STDBTN_COLOR          Color( 0xC0, 0xC0, 0xC0 )
#define IMAGELIST_IMAGE_NOTFOUND    ((sal_uInt16)0xFFFF)

enum ImageColorTransform
{
    IMAGECOLORTRANSFORM_NONE = 0,
    IMAGECOLORTRANSFORM_HIGHCONTRAST = 1,
    IMAGECOLORTRANSFORM_MONOCHROME_BLACK = 2,
    IMAGECOLORTRANSFORM_MONOCHROME_WHITE = 3
};

class VCL_DLLPUBLIC Image
{
    friend class ImageList;
    friend class ::OutputDevice;

public:
                    Image();
                    explicit Image( const ResId& rResId );
                    Image( const Image& rImage );
                    explicit Image( const BitmapEx& rBitmapEx );
                    explicit Image( const Bitmap& rBitmap );
                    Image( const Bitmap& rBitmap, const Bitmap& rMaskBitmap );
                    Image( const Bitmap& rBitmap, const Color& rColor );
                    explicit Image( const css::uno::Reference< css::graphic::XGraphic >& rxGraphic );
                    explicit Image( const OUString &rPNGFileUrl );
                    ~Image();

    Size            GetSizePixel() const;

    BitmapEx        GetBitmapEx() const;
    css::uno::Reference< css::graphic::XGraphic > GetXGraphic() const;

    bool            operator!() const { return !mpImplData; }
    Image&          operator=( const Image& rImage );
    bool            operator==( const Image& rImage ) const;
    bool            operator!=( const Image& rImage ) const { return !(Image::operator==( rImage )); }

    void Draw(OutputDevice* pOutDev, const Point& rPos, DrawImageFlags nStyle, const Size* pSize = nullptr);

private:

    ImplImage*             mpImplData;

    SAL_DLLPRIVATE void    ImplInit( const BitmapEx& rBmpEx );
};

class VCL_DLLPUBLIC ImageList
{
public:
                    explicit ImageList( sal_uInt16 nInit = 8, sal_uInt16 nGrow = 4 );
                    explicit ImageList( const ResId& rResId );
                    ImageList( const std::vector<OUString>& rNameVector,
                               const OUString& rPrefix);
                    ImageList( const ImageList& rImageList );
                    ~ImageList();

    void                    InsertFromHorizontalStrip( const BitmapEx &rBitmapEx,
                                   const std::vector< OUString > &rNameVector );
    void                    InsertFromHorizontalBitmap( const ResId& rResId,
                                    sal_uInt16       nCount,
                                    const Color *pNonAlphaMaskColor,
                                    const Color *pSearchColors = nullptr,
                                    const Color *pReplaceColors = nullptr,
                                    sal_uLong        nColorCount = 0);
    BitmapEx        GetAsHorizontalStrip() const;
    sal_uInt16          GetImageCount() const;
    Size            GetImageSize() const;

    void            AddImage( const OUString& rImageName, const Image& rImage );

    void            ReplaceImage( const OUString& rImageName, const Image& rImage );

    void            RemoveImage( sal_uInt16 nId );

    Image           GetImage( sal_uInt16 nId ) const;
    Image           GetImage( const OUString& rImageName ) const;

    sal_uInt16          GetImagePos( sal_uInt16 nId ) const;
    bool            HasImageAtPos( sal_uInt16 nId ) const;
    sal_uInt16          GetImagePos( const OUString& rImageName ) const;

    sal_uInt16          GetImageId( sal_uInt16 nPos ) const;

    OUString GetImageName( sal_uInt16 nPos ) const;
    void            GetImageNames( ::std::vector< OUString >& rNames ) const;

    ImageList&      operator=( const ImageList& rImageList );
    bool            operator==( const ImageList& rImageList ) const;
    bool            operator!=( const ImageList& rImageList ) const { return !(ImageList::operator==( rImageList )); }

private:

    ImplImageList*  mpImplData;
    sal_uInt16          mnInitSize;
    sal_uInt16          mnGrowSize;

    SAL_DLLPRIVATE void    ImplInit( sal_uInt16 nItems, const Size &rSize );
    SAL_DLLPRIVATE sal_uInt16  ImplGetImageId( const OUString& rImageName ) const;
};

#endif // INCLUDED_VCL_IMAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
