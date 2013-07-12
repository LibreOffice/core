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

#ifndef INCLUDED_BASEBMP_BITMAPDEVICE_HXX
#define INCLUDED_BASEBMP_BITMAPDEVICE_HXX

#include <sal/types.h>
#include <basebmp/drawmodes.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/basebmpdllapi.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

namespace basegfx
{
    class B2IPoint;
    class B2DPoint;
    class B2IVector;
    class B2IBox;
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace basebmp
{

// Temporary. Use like the tools color object
class Color;
typedef boost::shared_ptr< class BitmapDevice >                BitmapDeviceSharedPtr;
typedef boost::shared_ptr< struct IBitmapDeviceDamageTracker > IBitmapDeviceDamageTrackerSharedPtr;
typedef boost::shared_array< sal_uInt8 >                       RawMemorySharedArray;
typedef boost::shared_ptr< const std::vector<Color> >          PaletteMemorySharedVector;

struct ImplBitmapDevice;

/// Interface for getting damage tracking events
struct IBitmapDeviceDamageTracker
{
    /// gets called when said region is clobbered
    virtual void damaged(const basegfx::B2IBox& rDamageRect) const = 0;

protected:
    ~IBitmapDeviceDamageTracker() {}
};

/** Definition of BitmapDevice interface

    Use the createBitmapDevice() function to create instances.

    Implementation note: the clip mask and bitmap parameter instances
    of BitmapDevice that are passed to individual BitmapDevice
    instances work best with 1 bit grey masks for the clip and a
    format matching that of the target BitmapDevice for the other
    parameters. The alpha mask passed to the drawMaskedColor() methods
    works best when given as an eight bit grey bitmap. Everything else
    is accepted, but potentially slow.
 */
class BASEBMP_DLLPUBLIC BitmapDevice : public boost::enable_shared_from_this<BitmapDevice>,
                                       private boost::noncopyable
{
public:
    /** Query size of device in pixel columns (X) and rows (Y, "scanlines")
     */
    basegfx::B2IVector getSize() const;

    /** Query whether buffer starts with 0th scanline

        @return true, if the buffer memory starts with the 0th
        scanline, and false if it starts with the last one. The latter
        is e.g. the typical scan line ordering for the Windows BMP
        format.
     */
    bool isTopDown() const;

    /** Query the size of the whole frame buffer

        @ return the size of the whole frame buffer, the same as
        getSize() unless this is a "subset" device.
    */
    basegfx::B2IVector getBufferSize() const;

    /** Query type of scanline memory format
     */
    Format getScanlineFormat() const;

    /** Query byte offset to get from scanline n to scanline n+1

        @return the scanline stride in bytes.
     */
    sal_Int32 getScanlineStride() const;

    /** Get pointer to frame buffer

        @return a shared ptr to the bitmap buffer memory. As this is a
        shared ptr, you can freely store and use the pointer, even
        after this object has been deleted.
     */
    RawMemorySharedArray getBuffer() const;

    /// Query current damage tracking object (if any)
    IBitmapDeviceDamageTrackerSharedPtr getDamageTracker() const;

    /** Set new damage tracking object

        @param rDamage
        Object implementing the IBitmapDeviceDamageTracker interface -
        everytime some area of the surface gets clobbered, that object
        gets notified.
     */
    void  setDamageTracker( const IBitmapDeviceDamageTrackerSharedPtr& rDamage );

    /** Get pointer to palette

        The returned pointer is const on purpose, since the
        BitmapDevice might internally cache lookup information. Don't
        modify the returned data, unless you want to enter the realm
        of completely undefined behaviour.

        @return shared pointer to vector of Color entries.
     */
    PaletteMemorySharedVector getPalette() const;

    /// Check if this and the other BitmapDevice share a buffer
    bool isSharedBuffer( const BitmapDeviceSharedPtr& rOther ) const;

    /** Clear whole device with given color

        This method works like a fill with the given color value,
        resulting in a bitmap uniformly colored in fillColor.
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
        Clip mask to use. If the clip mask is 1 at the given pixel
        position, no change will take place.
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
        (with the actual pixel positions assumed to be the center of
        the pixel) is exactly in the middle between two pixel, this
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
        (with the actual pixel positions assumed to be the center of
        the pixel) is exactly in the middle between two pixel, this
        method always selects the pixel closer to rPt1.

        @param lineColor
        Color value to draw the line with

        @param drawMode
        Draw mode to use when changing the pixel value

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 1 will not be modified.
     */
    void drawLine( const basegfx::B2IPoint&     rPt1,
                   const basegfx::B2IPoint&     rPt2,
                   Color                        lineColor,
                   DrawMode                     drawMode,
                   const BitmapDeviceSharedPtr& rClip );

    /** Draw a polygon

        @param rPoly
        Polygon to draw. Depending on the value returned by rPoly's
        isClosed() method, the resulting line polygon will be drawn
        closed or not.

        @param lineColor
        Color value to draw the polygon with

        @param drawMode
        Draw mode to use when changing pixel values
     */
    void drawPolygon( const basegfx::B2DPolygon& rPoly,
                      Color                      lineColor,
                      DrawMode                   drawMode );

    /** Draw a polygon

        @param rPoly
        Polygon to draw. Depending on the value returned by rPoly's
        isClosed() method, the resulting line polygon will be drawn
        closed or not.

        @param lineColor
        Color value to draw the polygon with

        @param drawMode
        Draw mode to use when changing pixel values

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 1 will not be modified.
     */
    void drawPolygon( const basegfx::B2DPolygon&   rPoly,
                      Color                        lineColor,
                      DrawMode                     drawMode,
                      const BitmapDeviceSharedPtr& rClip );

    /** Fill a poly-polygon

        @param rPoly
        Poly-polygon to fill. Regardless of the value returned by
        rPoly's isClosed() method, the resulting filled poly-polygon
        is always considered closed. As usual, when filling a shape,
        the rightmost and bottommost pixel are not filled, compared to
        the drawPolygon() method. For example, the rectangle
        (0,0),(1,1) will have four pixel set, when drawn via
        drawPolygon(), and only one pixel, when filled via
        fillPolyPolygon().

        @param fillColor
        Color value to fill the poly-polygon with

        @param drawMode
        Draw mode to use when changing pixel values
     */
    void fillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                          Color                          fillColor,
                          DrawMode                       drawMode );

    /** Fill a poly-polygon

        @param rPoly
        Poly-polygon to fill. Regardless of the value returned by
        rPoly's isClosed() method, the resulting filled poly-polygon
        is always considered closed. As usual, when filling a shape,
        the rightmost and bottommost pixel are not filled, compared to
        the drawPolygon() method. For example, the rectangle
        (0,0),(1,1) will have four pixel set, when drawn via
        drawPolygon(), and only one pixel, when filled via
        fillPolyPolygon().

        @param fillColor
        Color value to fill the poly-polygon with

        @param drawMode
        Draw mode to use when changing pixel values

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 1 will not be modified.
     */
    void fillPolyPolygon( const basegfx::B2DPolyPolygon& rPoly,
                          Color                          fillColor,
                          DrawMode                       drawMode,
                          const BitmapDeviceSharedPtr&   rClip );

    /** Draw another bitmap into this device

        @param rSrcBitmap
        Bitmap to render into this one. It is permitted that source
        and destination bitmap are the same.

        @param rSrcRect
        Rectangle within the source bitmap to take the pixel from.

        @param rDstRect
        Rectangle in the destination bitmap to put the pixel
        into. Source and destination rectangle are permitted to have
        differing sizes; this method will scale the source pixel
        accordingly. Please note that both source and destination
        rectangle are interpreted excluding the rightmost pixel column
        and the bottommost pixel row, this is much like polygon
        filling. As a result, filling a given rectangle with
        fillPolyPolygon(), and using the same rectangle as the
        destination rectangle of this method, will affect exactly the
        same set of pixel.

        @param drawMode
        Draw mode to use when changing pixel values
     */
    void drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                     const basegfx::B2IBox&       rSrcRect,
                     const basegfx::B2IBox&       rDstRect,
                     DrawMode                     drawMode );

    /** Draw another bitmap into this device

        @param rSrcBitmap
        Bitmap to render into this one. It is permitted that source
        and destination bitmap are the same.

        @param rSrcRect
        Rectangle within the source bitmap to take the pixel from.

        @param rDstRect
        Rectangle in the destination bitmap to put the pixel
        into. Source and destination rectangle are permitted to have
        differing sizes; this method will scale the source pixel
        accordingly. Please note that both source and destination
        rectangle are interpreted excluding the rightmost pixel column
        and the bottommost pixel row, this is much like polygon
        filling. As a result, filling a given rectangle with
        fillPolyPolygon(), and using the same rectangle as the
        destination rectangle of this method, will affect exactly the
        same set of pixel.

        @param drawMode
        Draw mode to use when changing pixel values

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 1 will not be modified.
     */
    void drawBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                     const basegfx::B2IBox&       rSrcRect,
                     const basegfx::B2IBox&       rDstRect,
                     DrawMode                     drawMode,
                     const BitmapDeviceSharedPtr& rClip );

    /** Draw a color with an alpha-modulation bitmap into this device

        This method takes a fixed color value, and an alpha mask. For
        each pixel in the alpha mask, the given color value is blended
        with the corresponding alpha value against the content of this
        object.

        @param aSrcColor
        Color value to use for blending

        @param rAlphaMask
        Alpha mask to use for blending. It is permitted that alpha
        mask and this bitmap are the same object.

        @param rSrcRect
        Rectangle within the alpha mask to take the pixel from.
        Please note that the destination rectangle is interpreted
        excluding the rightmost pixel column and the bottommost pixel
        row, this is much like polygon filling. As a result, filling a
        given rectangle with fillPolyPolygon(), and using the same
        rectangle as the source rectangle of this method, will affect
        exactly the same set of pixel.

        @param rDstPoint
        Destination point, where to start placing the pixel from the
        source rectangle
     */
    void drawMaskedColor( Color                        aSrcColor,
                          const BitmapDeviceSharedPtr& rAlphaMask,
                          const basegfx::B2IBox&       rSrcRect,
                          const basegfx::B2IPoint&     rDstPoint );

    /** Draw a color with an alpha-modulation bitmap into this device

        This method takes a fixed color value, and an alpha mask. For
        each pixel in the alpha mask, the given color value is blended
        with the corresponding alpha value against the content of this
        object.

        @param aSrcColor
        Color value to use for blending

        @param rAlphaMask
        Alpha mask to use for blending. It is permitted that alpha
        mask and this bitmap are the same object.

        @param rSrcRect
        Rectangle within the alpha mask to take the pixel from.
        Please note that the destination rectangle is interpreted
        excluding the rightmost pixel column and the bottommost pixel
        row, this is much like polygon filling. As a result, filling a
        given rectangle with fillPolyPolygon(), and using the same
        rectangle as the source rectangle of this method, will affect
        exactly the same set of pixel.

        @param rDstPoint
        Destination point, where to start placing the pixel from the
        source rectangle

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 1 will not be modified.
     */
    void drawMaskedColor( Color                        aSrcColor,
                          const BitmapDeviceSharedPtr& rAlphaMask,
                          const basegfx::B2IBox&       rSrcRect,
                          const basegfx::B2IPoint&     rDstPoint,
                          const BitmapDeviceSharedPtr& rClip );

    /** Draw another bitmap through a mask into this device

        This method renders a source bitmap into this device, much
        like the drawBitmap() method. The only difference is the
        additional mask parameter, which operates much like an
        additional clip mask: pixel with value zero in this mask
        result in destination pixel not being modified.

        @param rSrcBitmap
        Bitmap to render into this one. It is permitted that source
        and destination bitmap are the same.

        @param rMask
        Bitmap to use as a mask. Pixel with value != zero in this mask
        will result in destination pixel not being affected by the
        blit operation.

        @param rSrcRect
        Rectangle within the source bitmap to take the pixel from.

        @param rDstRect
        Rectangle in the destination bitmap to put the pixel
        into. Source and destination rectangle are permitted to have
        differing sizes; this method will scale the source pixel
        accordingly. Please note that both source and destination
        rectangle are interpreted excluding the rightmost pixel column
        and the bottommost pixel row, this is much like polygon
        filling. As a result, filling a given rectangle with
        fillPolyPolygon(), and using the same rectangle as the
        destination rectangle of this method, will affect exactly the
        same set of pixel.

        @param drawMode
        Draw mode to use when changing pixel values
     */
    void drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                           const BitmapDeviceSharedPtr& rMask,
                           const basegfx::B2IBox&       rSrcRect,
                           const basegfx::B2IBox&       rDstRect,
                           DrawMode                     drawMode );

    /** Draw another bitmap through a mask into this device

        This method renders a source bitmap into this device, much
        like the drawBitmap() method. The only difference is the
        additional mask parameter, which operates much like an
        additional clip mask: pixel with value != zero in this mask
        result in destination pixel not being modified.

        @param rSrcBitmap
        Bitmap to render into this one. It is permitted that source
        and destination bitmap are the same.

        @param rMask
        Bitmap to use as a mask. Pixel with value != zero in this mask
        will result in destination pixel not being affected by the
        blit operation.

        @param rSrcRect
        Rectangle within the source bitmap to take the pixel from.

        @param rDstRect
        Rectangle in the destination bitmap to put the pixel
        into. Source and destination rectangle are permitted to have
        differing sizes; this method will scale the source pixel
        accordingly. Please note that both source and destination
        rectangle are interpreted excluding the rightmost pixel column
        and the bottommost pixel row, this is much like polygon
        filling. As a result, filling a given rectangle with
        fillPolyPolygon(), and using the same rectangle as the
        destination rectangle of this method, will affect exactly the
        same set of pixel.

        @param drawMode
        Draw mode to use when changing pixel values

        @param rClip
        Clip mask to use. Pixel where the corresponding clip mask
        pixel is 1 will not be modified.
     */
    void drawMaskedBitmap( const BitmapDeviceSharedPtr& rSrcBitmap,
                           const BitmapDeviceSharedPtr& rMask,
                           const basegfx::B2IBox&       rSrcRect,
                           const basegfx::B2IBox&       rDstRect,
                           DrawMode                     drawMode,
                           const BitmapDeviceSharedPtr& rClip );

protected:
    BASEBMP_DLLPRIVATE BitmapDevice( const basegfx::B2IBox&           rBounds,
                                     const basegfx::B2IVector&        rBufferSize,
                                     Format                           nScanlineFormat,
                                     sal_Int32                        nScanlineStride,
                                     sal_uInt8*                       pFirstScanline,
                                     const RawMemorySharedArray&      rMem,
                                     const PaletteMemorySharedVector& rPalette );
    BASEBMP_DLLPRIVATE virtual ~BitmapDevice();

private:
    BASEBMP_DLLPRIVATE virtual bool isCompatibleBitmap( const BitmapDeviceSharedPtr& bmp ) const = 0;
    BASEBMP_DLLPRIVATE virtual bool isCompatibleClipMask( const BitmapDeviceSharedPtr& bmp ) const = 0;
    BASEBMP_DLLPRIVATE virtual bool isCompatibleAlphaMask( const BitmapDeviceSharedPtr& bmp ) const = 0;

    BASEBMP_DLLPRIVATE virtual void clear_i( Color                  fillColor,
                                             const basegfx::B2IBox& rBounds ) = 0;

    BASEBMP_DLLPRIVATE virtual void setPixel_i( const basegfx::B2IPoint& rPt,
                                               Color                    lineColor,
                                               DrawMode                 drawMode ) = 0;
    BASEBMP_DLLPRIVATE virtual void setPixel_i( const basegfx::B2IPoint&     rPt,
                                                Color                        lineColor,
                                                DrawMode                     drawMode,
                                                const BitmapDeviceSharedPtr& rClip ) = 0;

    BASEBMP_DLLPRIVATE virtual Color getPixel_i( const basegfx::B2IPoint& rPt ) = 0;

    BASEBMP_DLLPRIVATE virtual sal_uInt32 getPixelData_i( const basegfx::B2IPoint& rPt ) = 0;

    BASEBMP_DLLPRIVATE virtual void drawLine_i( const basegfx::B2IPoint& rPt1,
                                                const basegfx::B2IPoint& rPt2,
                                                const basegfx::B2IBox&   rBounds,
                                                Color                    lineColor,
                                                DrawMode                 drawMode ) = 0;
    BASEBMP_DLLPRIVATE virtual void drawLine_i( const basegfx::B2IPoint&     rPt1,
                                                const basegfx::B2IPoint&     rPt2,
                                                const basegfx::B2IBox&       rBounds,
                                                Color                        lineColor,
                                                DrawMode                     drawMode,
                                                const BitmapDeviceSharedPtr& rClip ) = 0;

    BASEBMP_DLLPRIVATE virtual void drawPolygon_i( const basegfx::B2DPolygon& rPoly,
                                                   const basegfx::B2IBox&     rBounds,
                                                   Color                      lineColor,
                                                   DrawMode                   drawMode ) = 0;
    BASEBMP_DLLPRIVATE virtual void drawPolygon_i( const basegfx::B2DPolygon&   rPoly,
                                                   const basegfx::B2IBox&       rBounds,
                                                   Color                        lineColor,
                                                   DrawMode                     drawMode,
                                                   const BitmapDeviceSharedPtr& rClip ) = 0;

    BASEBMP_DLLPRIVATE virtual void fillPolyPolygon_i( const basegfx::B2DPolyPolygon& rPoly,
                                                       Color                          fillColor,
                                                       DrawMode                       drawMode,
                                                       const basegfx::B2IBox&         rBounds ) = 0;
    BASEBMP_DLLPRIVATE virtual void fillPolyPolygon_i( const basegfx::B2DPolyPolygon& rPoly,
                                                       Color                          fillColor,
                                                       DrawMode                       drawMode,
                                                       const basegfx::B2IBox&         rBounds,
                                                       const BitmapDeviceSharedPtr&   rClip ) = 0;

    // must work with *this == rSrcBitmap!
    BASEBMP_DLLPRIVATE virtual void drawBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                                                  const basegfx::B2IBox&       rSrcRect,
                                                  const basegfx::B2IBox&       rDstRect,
                                                  DrawMode                     drawMode ) = 0;
    BASEBMP_DLLPRIVATE virtual void drawBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                                                  const basegfx::B2IBox&       rSrcRect,
                                                  const basegfx::B2IBox&       rDstRect,
                                                  DrawMode                     drawMode,
                                                  const BitmapDeviceSharedPtr& rClip ) = 0;

    // must work with *this == rSrcBitmap!
    BASEBMP_DLLPRIVATE virtual void drawMaskedColor_i( Color                        rSrcColor,
                                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                                       const basegfx::B2IBox&       rSrcRect,
                                                       const basegfx::B2IPoint&     rDstPoint ) = 0;
    BASEBMP_DLLPRIVATE virtual void drawMaskedColor_i( Color                        rSrcColor,
                                                       const BitmapDeviceSharedPtr& rAlphaMask,
                                                       const basegfx::B2IBox&       rSrcRect,
                                                       const basegfx::B2IPoint&     rDstPoint,
                                                       const BitmapDeviceSharedPtr& rClip ) = 0;

    // must work with *this == rSrcBitmap!
    BASEBMP_DLLPRIVATE virtual void drawMaskedBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                                                        const BitmapDeviceSharedPtr& rMask,
                                                        const basegfx::B2IBox&       rSrcRect,
                                                        const basegfx::B2IBox&       rDstRect,
                                                        DrawMode                     drawMode ) = 0;
    BASEBMP_DLLPRIVATE virtual void drawMaskedBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap,
                                                        const BitmapDeviceSharedPtr& rMask,
                                                        const basegfx::B2IBox&       rSrcRect,
                                                        const basegfx::B2IBox&       rDstRect,
                                                        DrawMode                     drawMode,
                                                        const BitmapDeviceSharedPtr& rClip ) = 0;

    BASEBMP_DLLPRIVATE virtual IBitmapDeviceDamageTrackerSharedPtr getDamageTracker_i() const = 0;
    BASEBMP_DLLPRIVATE virtual void setDamageTracker_i( const IBitmapDeviceDamageTrackerSharedPtr& rDamage ) = 0;

    BitmapDeviceSharedPtr getGenericRenderer() const;

    boost::scoped_ptr< ImplBitmapDevice > mpImpl;
};

/** Function to create a BitmapDevice for given scanline format
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC createBitmapDevice( const basegfx::B2IVector& rSize,
                                                            bool                      bTopDown,
                                                            Format                    nScanlineFormat );

/** Function to create a BitmapDevice for given scanline format
    with the given palette

    Note: the provided palette must have sufficient size, to satisfy
    lookups for the whole range of pixel values from the specified
    format.
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC createBitmapDevice( const basegfx::B2IVector&        rSize,
                                                            bool                             bTopDown,
                                                            Format                           nScanlineFormat,
                                                            const PaletteMemorySharedVector& rPalette );

/** Function to create a BitmapDevice for given scanline format
    from the given piece of raw memory and palette

    Note: the provided memory must have sufficient size, to store the
    image of the specified area and format.
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC createBitmapDevice( const basegfx::B2IVector&        rSize,
                                                            bool                             bTopDown,
                                                            Format                           nScanlineFormat,
                                                            const RawMemorySharedArray&      rMem,
                                                            const PaletteMemorySharedVector& rPalette );


/** Function to retrieve a subsetted BitmapDevice to the same
    memory.

    Note that there is no coordinate system translation or offsetting
    involved.

    This method creates a second bitmap device instance, which renders
    to the same memory as the original, with the same pixel coordinate
    pairs refering to the same pixels in the memory buffer, but with
    rendering clipped to a rectangular area. Useful to implement
    rectangular clips (usually faster than setting up a 1bpp clip
    mask).

 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC subsetBitmapDevice( const BitmapDeviceSharedPtr& rProto,
                                                            const basegfx::B2IBox&       rSubset );

/** Function to clone a BitmapDevice from a given prototype.

    All attributes (like scanline format and top-down state) are
    copied, only the size can be varied. Note that the prototype's
    bitmap content is <em>not</em> copied, only a palette (if any).
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC cloneBitmapDevice( const basegfx::B2IVector&    rSize,
                                                           const BitmapDeviceSharedPtr& rProto );

}

#endif /* INCLUDED_BASEBMP_BITMAPDEVICE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
