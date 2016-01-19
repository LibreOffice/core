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
#include <basebmp/scanlineformats.hxx>
#include <basebmp/basebmpdllapi.h>

#include <boost/shared_array.hpp>
#include <memory>
#include <vector>

namespace basegfx
{
    class B2IPoint;
    class B2DPoint;
    class B2IVector;
    class B2IBox;
}

namespace basebmp
{

// Temporary. Use like the tools color object
class Color;
typedef std::shared_ptr< class BitmapDevice >                BitmapDeviceSharedPtr;
typedef boost::shared_array< sal_uInt8 >                       RawMemorySharedArray;
typedef std::shared_ptr< const std::vector<Color> >          PaletteMemorySharedVector;

struct ImplBitmapDevice;

/** Definition of BitmapDevice interface

    Use the createBitmapDevice() function to create instances.

    Implementation note: the clip mask and bitmap parameter instances
    of BitmapDevice that are passed to individual BitmapDevice
    instances work best with 1 bit grey masks for the clip and a
    format matching that of the target BitmapDevice for the other
    parameters.
 */
class BASEBMP_DLLPUBLIC BitmapDevice : public std::enable_shared_from_this<BitmapDevice>
{
public:
    /** Query size of device in pixel columns (X) and rows (Y, "scanlines")
     */
    basegfx::B2IVector getSize() const;

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

    /** Get color value at given pixel
     */
    Color getPixel( const basegfx::B2IPoint& rPt );

    /** Get underlying pixel data value at given position

        This method returns the raw pixel data. In the case of
        paletted bitmaps, this is the palette index, not the final
        color value.
     */
    sal_uInt32 getPixelData( const basegfx::B2IPoint& rPt );

    /** Draw another bitmap into this device

        @param rSrcBitmap
        Bitmap to render into this one.
     */
    void copyBitmap( const BitmapDeviceSharedPtr& rSrcBitmap );

protected:
    BASEBMP_DLLPRIVATE BitmapDevice( const basegfx::B2IBox&           rBounds,
                                     const basegfx::B2IVector&        rBufferSize,
                                     Format                           nScanlineFormat,
                                     sal_Int32                        nScanlineStride,
                                     sal_uInt8*                       pFirstScanline,
                                     const RawMemorySharedArray&      rMem,
                                     const PaletteMemorySharedVector& rPalette );
    BASEBMP_DLLPRIVATE virtual ~BitmapDevice();
    BitmapDevice(const BitmapDevice&) = delete;
    BitmapDevice& operator=( const BitmapDevice& ) = delete;

private:
    BASEBMP_DLLPRIVATE virtual bool isCompatibleBitmap( const BitmapDeviceSharedPtr& bmp ) const = 0;

    BASEBMP_DLLPRIVATE virtual Color getPixel_i( const basegfx::B2IPoint& rPt ) = 0;

    BASEBMP_DLLPRIVATE virtual sal_uInt32 getPixelData_i( const basegfx::B2IPoint& rPt ) = 0;

    BASEBMP_DLLPRIVATE virtual void copyBitmap_i( const BitmapDeviceSharedPtr& rSrcBitmap ) = 0;

    BitmapDeviceSharedPtr getGenericRenderer() const;

    std::unique_ptr< ImplBitmapDevice > mpImpl;
};

/** Function to calculate a suitable BitmapDevice Scanline Stride for a given scanline format and width
 *  you can provide this value or any larger value.
 */
sal_Int32 BASEBMP_DLLPUBLIC getBitmapDeviceStrideForWidth(Format nScanlineFormat, sal_Int32 nWidth);

/** Function to create a BitmapDevice for given scanline format
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC createBitmapDevice( const basegfx::B2IVector& rSize,
                                                            Format                    nScanlineFormat );

/** Function to create a BitmapDevice for given scanline format
    with the given palette

    Note: the provided palette must have sufficient size, to satisfy
    lookups for the whole range of pixel values from the specified
    format.
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC createBitmapDevice( const basegfx::B2IVector&        rSize,
                                                            Format                           nScanlineFormat,
                                                            const PaletteMemorySharedVector& rPalette );

/** Function to create a BitmapDevice for given scanline format
    from the given piece of raw memory and palette

    Note: the provided memory must have sufficient size, to store the
    image of the specified area and format.
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC createBitmapDevice( const basegfx::B2IVector&        rSize,
                                                            Format                           nScanlineFormat,
                                                            const RawMemorySharedArray&      rMem,
                                                            const PaletteMemorySharedVector& rPalette );


/** Function to clone a BitmapDevice from a given prototype.

    All attributes (like scanline format and top-down state) are
    copied, only the size can be varied. Note that the prototype's
    bitmap content is <em>not</em> copied, only a palette (if any).
 */
BitmapDeviceSharedPtr BASEBMP_DLLPUBLIC cloneBitmapDevice(const basegfx::B2IVector& rSize,
                                                          const BitmapDeviceSharedPtr& rProto);

}

#endif /* INCLUDED_BASEBMP_BITMAPDEVICE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
