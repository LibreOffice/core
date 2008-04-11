/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: image.h,v $
 * $Revision: 1.3 $
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

#include <hash_map>

// ----------------
// - ImplImageBmp -
// ----------------

class ImplImageBmp
{
public:

                ImplImageBmp();
                ~ImplImageBmp();

    void        Create( long nItemWidth, long nItemHeight, USHORT nInitSize );
    void        Create( const BitmapEx& rBmpEx, long nItemWidth, long nItemHeight,USHORT nInitSize );

    void        Expand( USHORT nGrowSize );

    void        Replace( USHORT nPos, USHORT nSrcPos );
    void        Replace( USHORT nPos, const ImplImageBmp& rImageBmp, USHORT nSrcPos );
    void        Replace( USHORT nPos, const BitmapEx& rBmpEx );

    void        ReplaceColors( const Color* pSrcColors, const Color* pDstColors, ULONG nColorCount );
    void        ColorTransform( BmpColorMode eColorMode );
    void            Invert();

    BitmapEx    GetBitmapEx( USHORT nPosCount, USHORT* pPosAry ) const;

    void        Draw( USHORT nPos, OutputDevice* pDev, const Point& rPos, USHORT nStyle, const Size* pSize = NULL );

private:

    BitmapEx    maBmpEx;
    Bitmap      maDisabledBmp;
    BitmapEx*   mpDisplayBmp;
    Size        maSize;
    BYTE*       mpInfoAry;
    USHORT      mnSize;

    void        ImplUpdateDisplayBmp( OutputDevice* pOutDev );
    void        ImplUpdateDisabledBmp( int nPos );

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
    USHORT          mnId;
    BitmapEx        maBitmapEx;

    ImageAryData();
    ImageAryData( const rtl::OUString &aName,
                  USHORT nId, const BitmapEx &aBitmap );
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
    typedef std::hash_map< rtl::OUString, ImageAryData *, rtl::OUStringHash >
        ImageAryDataNameHash;

    ImageAryDataVec             maImages;
    ImageAryDataNameHash        maNameHash;
    rtl::OUString               maPrefix;
    Size                        maImageSize;
    ULONG                       mnRefCount;

    ImplImageList();
    ImplImageList( const ImplImageList &aSrc );
    ~ImplImageList();

    void AddImage( const ::rtl::OUString &aName,
                   USHORT nId, const BitmapEx &aBitmapEx );
    void RemoveImage( USHORT nPos );
    USHORT GetImageCount() const;
};

// --------------------
// - ImplImageRefData -
// --------------------

struct ImplImageRefData
{
    ImplImageList*  mpImplData;
    USHORT          mnIndex;

                    ImplImageRefData() {}    // Um Warning zu umgehen
                    ~ImplImageRefData();

    BOOL            IsEqual( const ImplImageRefData& rData );
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

    BOOL            IsEqual( const ImplImageData& rData );
};

// -------------
// - ImplImage -
// -------------

struct ImplImage
{
    ULONG           mnRefCount;
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
