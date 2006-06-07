/*************************************************************************
 *
 *  $RCSfile: bitmapdevice.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: thb $ $Date: 2006-06-08 00:01:47 $
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
 ************************************************************************/

#ifndef INCLUDED_BASEBMP_BITMAPDEVICE_HXX
#define INCLUDED_BASEBMP_BITMAPDEVICE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef INCLUDED_BASEBMP_DRAWMODES_HXX
#include <basebmp/drawmodes.hxx>
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

/* What to do first:

   1,8,24 bpp, with blt, fill/drawPolygon, drawLine & get/setPixel

   Then:

   all other formats, top down vs. bottom up

   Last:

   Modulation, clip

 */

namespace basegfx
{
    class B2IPoint;
    class B2DPoint;
    class B2IVector;
    class B2IRange;
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace basebmp
{

// Temporary. Use like the tools color object
class Color;
typedef boost::shared_ptr< class BitmapDevice >         BitmapDeviceSharedPtr;
typedef boost::shared_array< sal_uInt8 >                RawMemorySharedArray;
typedef boost::shared_ptr< const std::vector<Color> >   PaletteMemorySharedVector;

struct ImplBitmapDevice;

/** Definition of BitmapDevice interface

    Use the createBitmapDevice() factory method to create one instance.

    Implementation note: the clip mask and bitmap parameter instances
    of BitmapDevice that are passed to individual BitmapDevice
    instances work best with 1 bit TC MSB masks for the clip and a
    format matching that of the target BitmapDevice for the other
    parameters. Everything else is accepted, but potentially slow.
 */
class BitmapDevice : private boost::noncopyable
{
public:
    /** Query size of device in pixel
     */
    basegfx::B2IVector getSize() const;

    /** Query whether buffer starts with 0th scanline

        @return true, if the buffer memory starts with the 0th
        scanline, and false if it starts with the last
     */
    bool isTopDown() const;

    /** Query type of scanline memory format
     */
    sal_Int32 getScanlineFormat() const;

    /** Query byte offset to get from scanline n to scanline n+1

        @return the scanline stride in bytes. In the case of
        bottom-first formats, this offset will be negative.
     */
    sal_Int32 getScanlineStride() const;

    /** Get pointer to frame buffer
     */
    RawMemorySharedArray getBuffer() const;

    /** Get pointer to palette

        The returned pointer is const on purpose, since the
        BitmapDevice might internally cache lookup information.

        @return shared pointer to vector of Color entries.
     */
    PaletteMemorySharedVector getPalette() const;

    /** Query number of palette entries.

        This is just a frontend for getPalette->size()
     */
    const sal_Int32 getPaletteEntryCount() const;

    /** Clear whole device with given color
     */
    void clear( Color fillColor );

    /** Set given pixel to specified color

        @param rPt
        Pixel to set

        @param pixelColor
        Color value to set the pixel to

        @param drawMode
        Draw mode to use when changing the pixel value
     */
    void setPixel( const basegfx::B2IPoint& rPt,
                   Color                    pixelColor,
                   DrawMode                 drawMode );

    /** Set given pixel to specified color

        @param rPt
        Pixel to set

        @param pixelColor
        Color value to set the pixel to

        @param drawMode
        Draw mode to use when changing the pixel value

        @param rClip
        Clip mask to use. If the clip mask is 0 at the given pixel
        position, no change will happen.
     */
    void setPixel( const basegfx::B2IPoint&     rPt,
                   Color                        pixelColor,
                   DrawMode                     drawMode,
                   const BitmapDeviceSharedPtr& rClip );

    /** Get color value at given pixel
     */
    Color getPixel( const basegfx::B2IPoint& rPt );

    /** Get underlying pixel data value at given position

        This method returns the raw pixel data. In the case of
        paletted bitmaps, this is the palette index, not the final
        color value.
     */
    sal_uInt32 getPixelData( const basegfx::B2IPoint& rPt );

    /** Draw a line

        @param rPt1
        Start point of the line

        @param rPt2
        End point of the line. If the analytical line from rP1 to rPt2
        (with the actual pixel positions are assumed to be the center
        of the pixel) is exactly in the middle between two pixel, this
        method always selects the pixel closer to rPt1.

        @param lineColor
        Color value to draw the line with

        @param drawMode
        Draw mode to use when changing the pixel value
     */
    void drawLine( const basegfx::B2IPoint& rPt1,
                   const basegfx::B2IPoint& rPt2,
                   Color                    lineColor,
                   DrawMode                 drawMode );

    /** Draw a line

        @param rPt1
        Start point of the line

        @param rPt2
        End point of the line. If the analytical line from rP1 to rPt2
        (with the actual pixel positions are assumed to be the center
        of the pixel) is exactly in the middle between two pixel, this
        method always selects the pixel closer to rPt1.

        @param lineColor
        Color value to draw the line with

        @param drawMode
        Draw mode to use when changing the pixel value

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 0 will not be touched.
     */
    void drawLine( const basegfx::B2IPoint& rPt1,
                   const basegfx::B2IPoint&     rPt2,
                   Color                        lineColor,
                   DrawMode                     drawMode,
                   const BitmapDeviceSharedPtr& rClip );

    void drawPolygon( const basegfx::B2DPolygon& rPoly,
                      Color                      lineColor,
                      DrawMode                   drawMode );
    void drawPolygon( const basegfx::B2DPolygon&   rPoly,
                      Color                        lineColor,
                      DrawMode                     drawMode,
                      const BitmapDeviceSharedPtr& rClip );

    void fillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                          Color                          fillColor,
                          DrawMode                       drawMode );
    void fillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                          Color                          fillColor,
                          DrawMode                       drawMode,
                          const BitmapDeviceSharedPtr&   rClip );

    void drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                     const basegfx::B2IRange&     rSrcRect,
                     const basegfx::B2IRange&     rDstRect,
                     DrawMode                     drawMode );
    void drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                     const basegfx::B2IRange&     rSrcRect,
                     const basegfx::B2IRange&     rDstRect,
                     DrawMode                     drawMode,
                     const BitmapDeviceSharedPtr& rClip );

    void drawMaskedColor( Color                        rSrcColor,
                          const BitmapDeviceSharedPtr& rAlphaMask,
                          const basegfx::B2IRange&     rSrcRect,
                          const basegfx::B2IPoint&     rDstPoint );
    void drawMaskedColor( Color                        rSrcColor,
                          const BitmapDeviceSharedPtr& rAlphaMask,
                          const basegfx::B2IRange&     rSrcRect,
                          const basegfx::B2IPoint&     rDstPoint,
                          const BitmapDeviceSharedPtr& rClip );

    void drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                           const BitmapDeviceSharedPtr& rMask,
                           const basegfx::B2IRange&     rSrcRect,
                           const basegfx::B2IRange&     rDstRect,
                           DrawMode                     drawMode );
    void drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                           const BitmapDeviceSharedPtr& rMask,
                           const basegfx::B2IRange&     rSrcRect,
                           const basegfx::B2IRange&     rDstRect,
                           DrawMode                     drawMode,
                           const BitmapDeviceSharedPtr& rClip );

protected:
    BitmapDevice( const basegfx::B2IVector&        rSize,
                  bool                             bTopDown,
                  sal_Int32                        nScanlineFormat,
                  sal_Int32                        nScanlineStride,
                  const RawMemorySharedArray&      rMem,
                  const PaletteMemorySharedVector& rPalette );

    virtual ~BitmapDevice();

private:
    virtual bool isCompatibleBitmap( const BitmapDeviceSharedPtr& bmp ) const = 0;
    virtual bool isCompatibleClipMask( const BitmapDeviceSharedPtr& bmp ) const = 0;
    virtual bool isCompatibleAlphaMask( const BitmapDeviceSharedPtr& bmp ) const = 0;

    virtual void clear_i( Color fillColor ) = 0;

    virtual void setPixel_i( const basegfx::B2IPoint& rPt,
                             Color                    lineColor,
                             DrawMode                 drawMode ) = 0;
    virtual void setPixel_i( const basegfx::B2IPoint&     rPt,
                             Color                        lineColor,
                             DrawMode                     drawMode,
                             const BitmapDeviceSharedPtr& rClip ) = 0;

    virtual Color getPixel_i( const basegfx::B2IPoint& rPt ) = 0;

    virtual sal_uInt32 getPixelData_i( const basegfx::B2IPoint& rPt ) = 0;

    virtual void drawLine_i( const basegfx::B2DPoint& rPt1,
                             const basegfx::B2DPoint& rPt2,
                             Color                    lineColor,
                             DrawMode                 drawMode ) = 0;
    virtual void drawLine_i( const basegfx::B2DPoint& rPt1,
                             const basegfx::B2DPoint&     rPt2,
                             Color                        lineColor,
                             DrawMode                     drawMode,
                             const BitmapDeviceSharedPtr& rClip ) = 0;

    virtual void drawPolygon_i( const basegfx::B2DPolygon& rPoly,
                                Color                      lineColor,
                                DrawMode                   drawMode ) = 0;
    virtual void drawPolygon_i( const basegfx::B2DPolygon&   rPoly,
                                Color                        lineColor,
                                DrawMode                     drawMode,
                                const BitmapDeviceSharedPtr& rClip ) = 0;

    virtual void fillPolyPolygon_i( const basegfx::B2DPolyPolygon& rPoly,
                                    Color                          fillColor,
                                    DrawMode                       drawMode,
                                    const basegfx::B2IRange&       rBounds ) = 0;
    virtual void fillPolyPolygon_i( const basegfx::B2DPolyPolygon& rPoly,
                                    Color                          fillColor,
                                    DrawMode                       drawMode,
                                    const basegfx::B2IRange&       rBounds,
                                    const BitmapDeviceSharedPtr&   rClip ) = 0;

    // must work with *this == rSrcBitmap!
    virtual void drawBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                               const basegfx::B2IRange&     rSrcRect,
                               const basegfx::B2IRange&     rDstRect,
                               DrawMode                     drawMode ) = 0;
    virtual void drawBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                               const basegfx::B2IRange&     rSrcRect,
                               const basegfx::B2IRange&     rDstRect,
                               DrawMode                     drawMode,
                               const BitmapDeviceSharedPtr& rClip ) = 0;

    // must work with *this == rSrcBitmap!
    virtual void drawMaskedColor_i( Color                        rSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IRange&     rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint ) = 0;
    virtual void drawMaskedColor_i( Color                        rSrcColor,
                                    const BitmapDeviceSharedPtr& rAlphaMask,
                                    const basegfx::B2IRange&     rSrcRect,
                                    const basegfx::B2IPoint&     rDstPoint,
                                    const BitmapDeviceSharedPtr& rClip ) = 0;

    // must work with *this == rSrcBitmap!
    virtual void drawMaskedBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                                     const BitmapDeviceSharedPtr& rMask,
                                     const basegfx::B2IRange&     rSrcRect,
                                     const basegfx::B2IRange&     rDstRect,
                                     DrawMode                     drawMode ) = 0;
    virtual void drawMaskedBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                                     const BitmapDeviceSharedPtr& rMask,
                                     const basegfx::B2IRange&     rSrcRect,
                                     const basegfx::B2IRange&     rDstRect,
                                     DrawMode                     drawMode,
                                     const BitmapDeviceSharedPtr& rClip ) = 0;

    boost::scoped_ptr< ImplBitmapDevice > mpImpl;
};

/** Factory method to create a BitmapDevice for given scanline format
 */
BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector& rSize,
                                          bool                      bTopDown,
                                          sal_Int32                 nScanlineFormat );

/** Factory method to create a BitmapDevice for given scanline format
    from the given piece of raw memory

    Note: the provided memory must have sufficient size, to store the
    image of the specified area and format.
 */
BitmapDeviceSharedPtr createBitmapDevice( const basegfx::B2IVector&        rSize,
                                          bool                             bTopDown,
                                          sal_Int32                        nScanlineFormat,
                                          const RawMemorySharedArray&      rMem,
                                          const PaletteMemorySharedVector& rPalette );


/** Factory method to clone a BitmapDevice from a given prototype.

    All attributes (like scanline format and top-down state) are
    copied, only the size can be varied. Note that the prototype's
    bitmap content is <em>not</em> copied, only a palette (if any).
 */
BitmapDeviceSharedPtr cloneBitmapDevice( const basegfx::B2IVector&        rSize,
                                         const BitmapDeviceSharedPtr&     rProto );

}

#endif /* INCLUDED_BASEBMP_BITMAPDEVICE_HXX */
