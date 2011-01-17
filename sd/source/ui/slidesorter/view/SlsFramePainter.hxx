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

#ifndef SD_SLIDESORTER_VIEW_FRAME_PAINTER_HXX
#define SD_SLIDESORTER_VIEW_FRAME_PAINTER_HXX

#include <vcl/bitmapex.hxx>


namespace sd { namespace slidesorter { namespace view {

class FramePainter
{
public:
    FramePainter (const BitmapEx& rBitmap);
    ~FramePainter (void);

    /** Paint a border around the given box by using a set of bitmaps for
        the corners and sides.
    */
    void PaintFrame (OutputDevice&rDevice, const Rectangle aBox) const;

    /** Special functionality that takes the color from the center
        bitmap and replaces that color in all bitmaps by the given new
        color.  Alpha values are not modified.
        @param bClearCenterBitmap
            When <TRUE/> then the center bitmap is erased.
    */
    void AdaptColor (const Color aNewColor, const bool bClearCenterBitmap);

private:
    /** Bitmap with offset that is used when the bitmap is painted.  The bitmap
    */
    class OffsetBitmap {
    public:
        BitmapEx maBitmap;
        Point maOffset;

        /** Create one of the eight shadow bitmaps from one that combines
            them all.  This larger bitmap is expected to have dimension NxN
            with N=1+2*M.  Of this larger bitmap there are created four
            corner bitmaps of size 2*M x 2*M and four side bitmaps of sizes
            1xM (top and bottom) and Mx1 (left and right).  The corner
            bitmaps have each one quadrant of size MxM that is painted under
            the interior of the frame.
            @param rBitmap
                The larger bitmap of which the eight shadow bitmaps are cut
                out from.
            @param nHorizontalPosition
                Valid values are -1 (left), 0 (center), and +1 (right).
            @param nVerticalPosition
                Valid values are -1 (top), 0 (center), and +1 (bottom).
        */
        OffsetBitmap (
            const BitmapEx& rBitmap,
            const sal_Int32 nHorizontalPosition,
            const sal_Int32 nVerticalPosition);

        /** Use the given device to paint the bitmap at the location that is
            the sum of the given anchor and the internal offset.
        */
        void PaintCorner (OutputDevice& rDevice, const Point& rAnchor) const;

        /** Use the given device to paint the bitmap stretched between the
            two given locations.  Offsets of the adjacent corner bitmaps and
            the offset of the side bitmap are used to determine the area
            that is to be filled with the side bitmap.
        */
        void PaintSide (
            OutputDevice& rDevice,
            const Point& rAnchor1,
            const Point& rAnchor2,
            const OffsetBitmap& rCornerBitmap1,
            const OffsetBitmap& rCornerBitmap2) const;

        /** Fill the given rectangle with the bitmap.
        */
        void PaintCenter (
            OutputDevice& rDevice,
            const Rectangle& rBox) const;
    };
    OffsetBitmap maTopLeft;
    OffsetBitmap maTop;
    OffsetBitmap maTopRight;
    OffsetBitmap maLeft;
    OffsetBitmap maRight;
    OffsetBitmap maBottomLeft;
    OffsetBitmap maBottom;
    OffsetBitmap maBottomRight;
    OffsetBitmap maCenter;
    bool mbIsValid;
};


} } } // end of namespace sd::slidesorter::view

#endif
