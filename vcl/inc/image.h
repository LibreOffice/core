/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_IMAGE_H
#define _SV_IMAGE_H

#include <vcl/bitmapex.hxx>

#include <boost/unordered_map.hpp>

// ----------------
// - ImplImageBmp -
// ----------------

class ImplImageBmp
{
public:

                ImplImageBmp();
                ~ImplImageBmp();

    void        Create( long nItemWidth, long nItemHeight, sal_uInt16 nInitSize );
    void        Create( const BitmapEx& rBmpEx, long nItemWidth, long nItemHeight,sal_uInt16 nInitSize );

    void        Expand( sal_uInt16 nGrowSize );

    void        Replace( sal_uInt16 nPos, sal_uInt16 nSrcPos );
    void        Replace( sal_uInt16 nPos, const ImplImageBmp& rImageBmp, sal_uInt16 nSrcPos );
    void        Replace( sal_uInt16 nPos, const BitmapEx& rBmpEx );

    void        ReplaceColors( const Color* pSrcColors, const Color* pDstColors, sal_uIntPtr nColorCount );
    void        ColorTransform();
    void            Invert();

    BitmapEx    GetBitmapEx( sal_uInt16 nPosCount, sal_uInt16* pPosAry ) const;

    void        Draw( sal_uInt16 nPos, OutputDevice* pDev, const Point& rPos, sal_uInt16 nStyle, const Size* pSize = NULL );

private:

    BitmapEx    maBmpEx;
    BitmapEx    maDisabledBmpEx;
    BitmapEx*   mpDisplayBmp;
    Size        maSize;
    sal_uInt8*      mpInfoAry;
    sal_uInt16      mnSize;

    void        ImplUpdateDisplayBmp( OutputDevice* pOutDev );
    void        ImplUpdateDisabledBmpEx( int nPos );

private:    // prevent assignment and copy construction
    ImplImageBmp( const ImplImageBmp& );
    void operator=( const ImplImageBmp& );
};

// --------------
// - ImageTypes -
// --------------

enum ImageType { IMAGETYPE_BITMAP, IMAGETYPE_IMAGE };

// -----------------
// - ImplImageList -
// -----------------

struct ImageAryData
{
    ::rtl::OUString maName;
    // Images identified by either name, or by id
    sal_uInt16          mnId;
    BitmapEx        maBitmapEx;

    ImageAryData();
    ImageAryData( const rtl::OUString &aName,
                  sal_uInt16 nId, const BitmapEx &aBitmap );
    ImageAryData( const ImageAryData& rData );
    ~ImageAryData();

    bool IsLoadable() { return maBitmapEx.IsEmpty() && maName.getLength(); }
    void Load(const rtl::OUString &rPrefix);

    ImageAryData&   operator=( const ImageAryData& rData );
};

// ------------------------------------------------------------------------------

struct ImplImageList
{
    typedef std::vector<ImageAryData *> ImageAryDataVec;
    typedef boost::unordered_map< rtl::OUString, ImageAryData *, rtl::OUStringHash >
        ImageAryDataNameHash;

    ImageAryDataVec             maImages;
    ImageAryDataNameHash        maNameHash;
    rtl::OUString               maPrefix;
    Size                        maImageSize;
    sal_uIntPtr                       mnRefCount;

    ImplImageList();
    ImplImageList( const ImplImageList &aSrc );
    ~ImplImageList();

    void AddImage( const ::rtl::OUString &aName,
                   sal_uInt16 nId, const BitmapEx &aBitmapEx );
    void RemoveImage( sal_uInt16 nPos );
    sal_uInt16 GetImageCount() const;
};

// --------------------
// - ImplImageRefData -
// --------------------

struct ImplImageRefData
{
    ImplImageList*  mpImplData;
    sal_uInt16          mnIndex;

                    ImplImageRefData() {}    // Um Warning zu umgehen
                    ~ImplImageRefData();

    sal_Bool            IsEqual( const ImplImageRefData& rData );
};

// ----------------
// - ImpImageData -
// ----------------

struct ImplImageData
{
    ImplImageBmp*   mpImageBitmap;
    BitmapEx        maBmpEx;

                    ImplImageData( const BitmapEx& rBmpEx );
                    ~ImplImageData();

    sal_Bool            IsEqual( const ImplImageData& rData );
};

// -------------
// - ImplImage -
// -------------

struct ImplImage
{
    sal_uIntPtr         mnRefCount;
    // TODO: use inheritance to get rid of meType+mpData
    void*           mpData;
    ImageType       meType;

                    ImplImage();
                    ~ImplImage();

private:    // prevent assignment and copy construction
            ImplImage( const ImplImage&);
    void    operator=( const ImplImage&);
};

#endif // _SV_IMAGE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
