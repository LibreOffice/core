/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BITMAP_TOOLS_HXX
#define INCLUDED_VCL_BITMAP_TOOLS_HXX

#include <vcl/bitmapex.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>

namespace vcl {
namespace bitmap {

/**
 * Intended to be used to feed into CreateFromData to create a BitmapEx. RGB data format.
 */
class VCL_DLLPUBLIC RawBitmap
{
friend BitmapEx VCL_DLLPUBLIC CreateFromData( RawBitmap&& rawBitmap );
    std::unique_ptr<sal_uInt8[]> mpData;
    Size maSize;
public:
    RawBitmap(Size const & rSize)
        : mpData(new sal_uInt8[ rSize.getWidth() * 3 * rSize.getHeight()]),
          maSize(rSize)
    {
    }
    void SetPixel(long nY, long nX, BitmapColor nColor)
    {
        long p = (nY * maSize.getWidth() + nX) * 3;
        mpData[ p++ ] = nColor.GetRed();
        mpData[ p++ ] = nColor.GetGreen();
        mpData[ p   ] = nColor.GetBlue();
    }
    long Height() { return maSize.Height(); }
    long Width() { return maSize.Width(); }
};

BitmapEx VCL_DLLPUBLIC loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags = ImageLoadFlags::NONE);

void loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx, double fScaleFactor);

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.

    @param pData
    The block of data to copy
    @param nStride
    The number of bytes in a scanline, must >= width
*/
BitmapEx VCL_DLLPUBLIC CreateFromData( sal_uInt8 const *pData, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nStride, sal_uInt16 nBitCount );

BitmapEx VCL_DLLPUBLIC CreateFromData( RawBitmap && data );

}} // end vcl::bitmap

#endif // INCLUDED_VCL_BITMAP_TOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
