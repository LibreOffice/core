/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    void        Create( long nItemWidth, long nItemHeight, sal_uInt16 nInitSize );
    void        Create( const BitmapEx& rBmpEx, long nItemWidth, long nItemHeight,sal_uInt16 nInitSize );

    void        Expand( sal_uInt16 nGrowSize );

    void        Replace( sal_uInt16 nPos, sal_uInt16 nSrcPos );
    void        Replace( sal_uInt16 nPos, const ImplImageBmp& rImageBmp, sal_uInt16 nSrcPos );
    void        Replace( sal_uInt16 nPos, const BitmapEx& rBmpEx );

    void        ReplaceColors( const Color* pSrcColors, const Color* pDstColors, sal_uIntPtr nColorCount );
    void        ColorTransform( BmpColorMode eColorMode );
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
    typedef std::hash_map< rtl::OUString, ImageAryData *, rtl::OUStringHash >
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
