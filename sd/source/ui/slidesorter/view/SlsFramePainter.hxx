/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: baseprimitive2d.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

private:
    /** Bitmap with offset that is used when the bitmap is painted.  The bitmap
    */
    class OffsetBitmap {
    public:
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

    private:
        BitmapEx maBitmap;
        Point maOffset;
    };
    OffsetBitmap maShadowTopLeft;
    OffsetBitmap maShadowTop;
    OffsetBitmap maShadowTopRight;
    OffsetBitmap maShadowLeft;
    OffsetBitmap maShadowRight;
    OffsetBitmap maShadowBottomLeft;
    OffsetBitmap maShadowBottom;
    OffsetBitmap maShadowBottomRight;
    OffsetBitmap maShadowCenter;
    bool mbIsValid;
};


} } } // end of namespace sd::slidesorter::view

#endif
