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

#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <vcl/salgtype.hxx>
#include <memory>

class SalVirtualDevice;
struct SystemGraphicsData;
typedef struct _cairo_surface cairo_surface_t;

class VCL_DLLPUBLIC VirtualDevice : public OutputDevice
{
    friend class Application;
    friend class ::OutputDevice;
    friend class Printer;
    friend cairo_surface_t* get_underlying_cairo_surface(const VirtualDevice&);
public:
    // reference device modes for different compatibility levels
    enum class RefDevMode { NONE = 0,
                            Dpi600 = 1,      // 600 dpi
                            MSO1 = 3,
                            PDF1 = 4,
                            Custom = 5
                          };

private:
    std::unique_ptr<SalVirtualDevice> mpVirDev;
    VclPtr<VirtualDevice>  mpPrev;
    VclPtr<VirtualDevice>  mpNext;
    sal_uInt16          mnBitCount;
    bool                mbScreenComp;
    const DeviceFormat  meFormat;
    const DeviceFormat  meAlphaFormat;
    RefDevMode          meRefDevMode;
    bool                mbForceZeroExtleadBug;

    SAL_DLLPRIVATE void ImplInitVirDev( const OutputDevice* pOutDev, long nDX, long nDY, const SystemGraphicsData *pData = nullptr );
    SAL_DLLPRIVATE bool InnerImplSetOutputSizePixel( const Size& rNewSize, bool bErase,
                                                     sal_uInt8* pBuffer );
    SAL_DLLPRIVATE bool ImplSetOutputSizePixel( const Size& rNewSize, bool bErase,
                                                sal_uInt8* pBuffer );

    VirtualDevice (const VirtualDevice &) = delete;
    VirtualDevice & operator= (const VirtualDevice &) = delete;

    /** Used for alpha VDev, to set areas to opaque

        @since \#i32109#
     */
    SAL_DLLPRIVATE void ImplFillOpaqueRectangle( const tools::Rectangle& rRect );

protected:
    virtual bool AcquireGraphics() const override;
    virtual void ReleaseGraphics( bool bRelease = true ) override;

    /** Create a virtual device of size 1x1

        @param pCompDev
        The generated vdev will be compatible to this device.
        If it's the nullptr, it uses Application::GetDefaultDevice().

        @param eFormat
        Device format of the generated virtual device. Use DeviceFormat::DEFAULT here, to
        indicate: take default screen depth. Only DeviceFormat::BITMASK
        is the other possibility to denote a binary mask.

        @param eAlphaFormat
        Device format of the generated virtual device. Use DeviceFormat::DEFAULT here, to
        indicate: take default screen depth. Only DeviceFormat::BITMASK
        is the other possibility to denote a binary mask.

        @param eOutDevType
        This real virtual output device type.
     */
    explicit VirtualDevice(const OutputDevice* pCompDev, DeviceFormat eFormat,
                           DeviceFormat eAlphaFormat, OutDevType eOutDevType);

public:

    /** Create a virtual device of size 1x1

        @param eFormat
        Device format of the generated virtual device. Use DeviceFormat::DEFAULT here, to
        indicate: take default screen depth. Only DeviceFormat::BITMASK
        is the other possibility to denote a binary mask.
     */
    explicit VirtualDevice(DeviceFormat eFormat = DeviceFormat::DEFAULT)
        : VirtualDevice(nullptr, eFormat, DeviceFormat::NONE, OUTDEV_VIRDEV) {}

    /** Create a virtual device of size 1x1

        @param rCompDev
        The generated vdev will be compatible to this device.

        @param eFormat
        Device format of the generated virtual device. Use DeviceFormat::DEFAULT here, to
        indicate: take default screen depth. Only DeviceFormat::BITMASK
        is the other possibility to denote a binary mask.
     */
    explicit VirtualDevice(const OutputDevice& rCompDev,
                           DeviceFormat eFormat = DeviceFormat::DEFAULT)
        : VirtualDevice(&rCompDev, eFormat, DeviceFormat::NONE, OUTDEV_VIRDEV) {}

    /** Create a virtual device  of size 1x1 with alpha channel

        @param rCompDev
        The generated vdev will be compatible to this device.

        @param eFormat
        Device format of the generated virtual device. Use DeviceFormat::DEFAULT here, to
        indicate: take default screen depth. Only DeviceFormat::BITMASK
        is the other possibility to denote a binary mask.

        @param eAlphaFormat
        Device format of the generated virtual device. Use DeviceFormat::DEFAULT here, to
        indicate: take default screen depth. Only DeviceFormat::BITMASK
        is the other possibility to denote a binary mask.
     */
    explicit VirtualDevice(const OutputDevice& rCompDev,
                           DeviceFormat eFormat, DeviceFormat eAlphaFormat)
        : VirtualDevice(&rCompDev, eFormat, eAlphaFormat, OUTDEV_VIRDEV) {}

    /** Create a virtual device using an existing system dependent device or graphics context
        Any rendering will happen directly on the context and not on any intermediate bitmap.
        Note: This might not be supported on all platforms !
     */
    explicit VirtualDevice(const SystemGraphicsData *pData, const Size &rSize,
                           DeviceFormat eFormat);

    virtual             ~VirtualDevice() override;
    virtual void        dispose() override;

    virtual void        EnableRTL( bool bEnable = true ) override;

    bool                SetOutputSizePixel( const Size& rNewSize, bool bErase = true );
    bool                SetOutputSizePixelScaleOffsetAndBuffer( const Size& rNewSize,
                                                                const Fraction& rScale,
                                                                const Point& rNewOffset,
                                                                sal_uInt8* pBuffer);

    bool                SetOutputSize( const Size& rNewSize )
                            { return SetOutputSizePixel( LogicToPixel( rNewSize ) ); }

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
