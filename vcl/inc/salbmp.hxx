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

#pragma once

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/checksum.hxx>
#include <vcl/BitmapAccessMode.hxx>
#include <vcl/BitmapBuffer.hxx>
#include <vcl/bitmap/BitmapTypes.hxx>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <basegfx/utils/systemdependentdata.hxx>

#if defined MACOSX || defined IOS
#include <premac.h>
#include <CoreGraphics/CoreGraphics.h>
#include <postmac.h>
#endif

struct BitmapBuffer;
class Color;
class SalGraphics;
class BitmapPalette;
struct BitmapSystemData;
enum class BmpScaleFlag;

extern const sal_uLong nVCLRLut[ 6 ];
extern const sal_uLong nVCLGLut[ 6 ];
extern const sal_uLong nVCLBLut[ 6 ];
extern const sal_uLong nVCLDitherLut[ 256 ];
extern const sal_uLong nVCLLut[ 256 ];

class VCL_PLUGIN_PUBLIC SalBitmap
{
public:

    SalBitmap()
        : mnChecksum(0)
        , mbChecksumValid(false)
    {
    }

    virtual                 ~SalBitmap();

    virtual bool            Create( const Size& rSize,
                                    vcl::PixelFormat ePixelFormat,
                                    const BitmapPalette& rPal ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    vcl::PixelFormat eNewPixelFormat) = 0;
    virtual bool            Create( const css::uno::Reference< css::rendering::XBitmapCanvas >& rBitmapCanvas,
                                    Size& rSize,
                                    bool bMask = false ) = 0;
    virtual void            Destroy() = 0;
    virtual Size            GetSize() const = 0;
    virtual sal_uInt16      GetBitCount() const = 0;

    virtual BitmapBuffer*   AcquireBuffer( BitmapAccessMode nMode ) = 0;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, BitmapAccessMode nMode ) = 0;
    virtual bool            GetSystemData( BitmapSystemData& rData ) = 0;

    virtual bool            ScalingSupported() const = 0;
    virtual bool            Scale( const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag ) = 0;
    void                    DropScaledCache();

    virtual bool            Replace( const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol ) = 0;

    virtual bool            ConvertToGreyscale()
    {
        return false;
    }
    virtual bool            InterpretAs8Bit()
    {
        return false;
    }

    virtual bool            Erase( const Color& /*color*/ )
    {
        return false;
    }
    // Optimized case for AlphaMask::BlendWith().
    virtual bool            AlphaBlendWith( const SalBitmap& /*rSalBmp*/ )
    {
        return false;
    }

    virtual bool            Invert()
    {
        return false;
    }

#if defined MACOSX || defined IOS
    // Related: tdf#146842 Eliminate temporary copies of SkiaSalBitmap when
    // printing
    // Commit 9eb732a32023e74c44ac8c3b5af9f5424273bb6c fixed crashing when
    // printing SkiaSalBitmaps to a non-Skia SalGraphics. However, the fix
    // almost always makes two copies of the SkiaSalBitmap's bitmap data: the
    // first copy is made in SkiaSalBitmap::AcquireBuffer() and then
    // QuartzSalBitmap makes a copy of the first copy.
    // By making QuartzSalBitmap's methods that return a CGImageRef virtual,
    // a non-Skia SalGraphics can now create a CGImageRef directly from a
    // SkiaSalBitmap's Skia bitmap data without copying to any intermediate
    // buffers.
    // Note: these methods are not pure virtual as the SvpSalBitmap class
    // extends this class directly.
    virtual CGImageRef      CreateWithMask( const SalBitmap&, int, int, int, int ) const { return nullptr; }
    virtual CGImageRef      CreateColorMask( int, int, int, int, Color ) const { return nullptr; }
    virtual CGImageRef      CreateCroppedImage( int, int, int, int ) const { return nullptr; }
#endif

    BitmapChecksum GetChecksum() const
    {
        updateChecksum();
        if (!mbChecksumValid)
            return 0; // back-compat
        return mnChecksum;
    }

    void InvalidateChecksum()
    {
        mbChecksumValid = false;
    }

protected:
    void updateChecksum() const;
    // helper function to convert data in 1,2,4 bpp formats to a 8/24/32bpp format
    enum class BitConvert
    {
        A8,
        RGBA,
        BGRA,
        LAST = BGRA
    };
    static std::unique_ptr< sal_uInt8[] > convertDataBitCount( const sal_uInt8* src,
        int width, int height, int bitCount, int bytesPerRow, const BitmapPalette& palette,
        BitConvert type );

public:
    // access to SystemDependentDataHolder, to support overload in derived class(es)
    virtual const basegfx::SystemDependentDataHolder* accessSystemDependentDataHolder() const;

    // exclusive management op's for SystemDependentData at SalBitmap
    template<class T>
    std::shared_ptr<T> getSystemDependentData(basegfx::SDD_Type aType) const
    {
        const basegfx::SystemDependentDataHolder* pDataHolder(accessSystemDependentDataHolder());
        if(pDataHolder)
            return std::static_pointer_cast<T>(pDataHolder->getSystemDependentData(aType));
        return std::shared_ptr<T>();
    }

    template<class T, class... Args>
    std::shared_ptr<T> addOrReplaceSystemDependentData(Args&&... args) const
    {
        const basegfx::SystemDependentDataHolder* pDataHolder(accessSystemDependentDataHolder());
        if(!pDataHolder)
            return std::shared_ptr<T>();

        std::shared_ptr<T> r = std::make_shared<T>(std::forward<Args>(args)...);

        // tdf#129845 only add to buffer if a relevant buffer time is estimated
        if(r->calculateCombinedHoldCyclesInSeconds() > 0)
        {
            basegfx::SystemDependentData_SharedPtr r2(r);
            const_cast< basegfx::SystemDependentDataHolder* >(pDataHolder)->addOrReplaceSystemDependentData(r2);
        }

        return r;
    }

private:
    BitmapChecksum mnChecksum;
    bool           mbChecksumValid;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
