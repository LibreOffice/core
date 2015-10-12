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

#ifndef INCLUDED_VCL_VIRDEV_HXX
#define INCLUDED_VCL_VIRDEV_HXX

#include <basebmp/bitmapdevice.hxx>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>

class SalVirtualDevice;
struct SystemGraphicsData;

class VCL_DLLPUBLIC VirtualDevice : public OutputDevice
{
    friend class Application;
    friend class ::OutputDevice;
    friend class Printer;

private:
    SalVirtualDevice*   mpVirDev;
    VclPtr<VirtualDevice>  mpPrev;
    VclPtr<VirtualDevice>  mpNext;
    sal_uInt16          mnBitCount;
    bool                mbScreenComp;
    sal_Int8            mnAlphaDepth;
    sal_uInt8           meRefDevMode;

    SAL_DLLPRIVATE void ImplInitVirDev( const OutputDevice* pOutDev, long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL );
    SAL_DLLPRIVATE bool InnerImplSetOutputSizePixel( const Size& rNewSize, bool bErase,
                                                     const basebmp::RawMemorySharedArray &pBuffer,
                                                     const bool bTopDown );
    SAL_DLLPRIVATE bool ImplSetOutputSizePixel( const Size& rNewSize, bool bErase,
                                                const basebmp::RawMemorySharedArray &pBuffer,
                                                const basebmp::RawMemorySharedArray &pAlphaBuffer,
                                                const bool bTopDown );

    VirtualDevice (const VirtualDevice &) = delete;
    VirtualDevice & operator= (const VirtualDevice &) = delete;

    /** Used for alpha VDev, to set areas to opaque

        @since \#i32109#
     */
    SAL_DLLPRIVATE void ImplFillOpaqueRectangle( const Rectangle& rRect );

    // TODO: add extra member for refdev backward compatibility options
    #define REFDEV_FORCE_ZERO_EXTLEAD 0x80
    SAL_DLLPRIVATE bool ForceZeroExtleadBug() const
        { return ((meRefDevMode & REFDEV_FORCE_ZERO_EXTLEAD) != 0); }

protected:
    virtual bool AcquireGraphics() const override;
    virtual void ReleaseGraphics( bool bRelease = true ) override;

public:

    /** Create a virtual device of size 1x1

        @param nBitCount
        Bit depth of the generated virtual device. Use 0 here, to
        indicate: take default screen depth. Only 0, 1 and 8
        are allowed here, with 1 denoting binary mask and 8 a graylevel mask.
     */
    explicit            VirtualDevice( sal_uInt16 nBitCount = 0 );

    /** Create a virtual device of size 1x1

        @param rCompDev
        The generated vdev will be compatible to this device.

        @param nBitCount
        Bit depth of the generated virtual device. Use 0 here, to
        indicate: take default screen depth.
     */
     explicit           VirtualDevice( const OutputDevice& rCompDev,
                                       sal_uInt16 nBitCount = 0 );

    /** Create a virtual device  of size 1x1 with alpha channel

        @param rCompDev
        The generated vdev will be compatible to this device.

        @param nBitCount
        Bit depth of the generated virtual device. Use 0 here, to
        indicate: take default screen depth. Only 0 and 1
        are allowed here, with 1 denoting binary mask.

        @param nAlphaBitCount
        Bit depth of the generated virtual device. Use 0 here, to
        indicate: take default screen depth. Only 0 and 1
        are allowed here, with 1 denoting binary mask.
     */
     explicit           VirtualDevice( const OutputDevice& rCompDev,
                                       sal_uInt16 nBitCount, sal_uInt16 nAlphaBitCount );

    /** Create a virtual device using an existing system dependent device or graphics context
        Any rendering will happen directly on the context and not on any intermediate bitmap.
        Note: This might not be supported on all platforms !
    */
    explicit            VirtualDevice(const SystemGraphicsData *pData, const Size &rSize,
                                      sal_uInt16 nBitCount);

    virtual             ~VirtualDevice();
    virtual void        dispose() override;

    virtual void        EnableRTL( bool bEnable = true ) override;

    bool                SetOutputSizePixel( const Size& rNewSize, bool bErase = true );
    bool                SetOutputSizePixelScaleOffsetAndBuffer( const Size& rNewSize,
                                                                const Fraction& rScale,
                                                                const Point& rNewOffset,
                                                                const basebmp::RawMemorySharedArray &pBuffer,
                                                                const basebmp::RawMemorySharedArray &pAlphaBuffer,
                                                                const bool bTopDown = false );
    bool                SetOutputSize( const Size& rNewSize, bool bErase = true )
                            { return SetOutputSizePixel( LogicToPixel( rNewSize ), bErase ); }

    // reference device modes for different compatibility levels
    enum RefDevMode {   REFDEV_NONE = 0,
                        REFDEV_MODE06 = 1,      // 600 dpi
                        REFDEV_MODE_MSO1 = 3,
                        REFDEV_MODE_PDF1 = 4,
                        REFDEV_CUSTOM = 5
                    };

    void                SetReferenceDevice( RefDevMode );

    void                Compat_ZeroExtleadBug(); // enable workaround for #i60495#

    void                SetReferenceDevice( sal_Int32 i_nDPIX, sal_Int32 i_nDPIY );

    virtual sal_uInt16  GetBitCount() const override;

private:
    SAL_DLLPRIVATE void ImplSetReferenceDevice( RefDevMode, sal_Int32 i_nDPIX, sal_Int32 i_nDPIY );

protected:
    virtual bool        UsePolyPolygonForComplexGradient() override;

    virtual long        GetFontExtLeading() const override;

};

#endif // INCLUDED_VCL_VIRDEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
