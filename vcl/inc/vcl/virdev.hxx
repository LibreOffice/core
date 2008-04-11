/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: virdev.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_VIRDEV_HXX
#define _SV_VIRDEV_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>

// -----------------
// - VirtualDevice -
// -----------------

class SalVirtualDevice;
class RmVirtualDevice;
struct SystemGraphicsData;

class VCL_DLLPUBLIC VirtualDevice : public OutputDevice
{
    friend class Application;
    friend class OutputDevice;

private:
    SalVirtualDevice*   mpVirDev;
    VirtualDevice*      mpPrev;
    VirtualDevice*      mpNext;
    USHORT              mnBitCount;
    BOOL                mbScreenComp;
    sal_Int8            mnAlphaDepth;
    BYTE                meRefDevMode;

    SAL_DLLPRIVATE void ImplInitVirDev( const OutputDevice* pOutDev, long nDX, long nDY, USHORT nBitCount, const SystemGraphicsData *pData = NULL );
    SAL_DLLPRIVATE BOOL ImplSetOutputSizePixel( const Size& rNewSize, BOOL bErase );

    // Copy assignment is forbidden and not implemented.
    VirtualDevice (const VirtualDevice &);
    VirtualDevice & operator= (const VirtualDevice &);

    /** Used for alpha VDev, to set areas to opaque

        @since #i32109#
     */
    SAL_DLLPRIVATE void ImplFillOpaqueRectangle( const Rectangle& rRect );

    // TODO: add extra member for refdev backward compatibility options
    #define REFDEV_FORCE_ZERO_EXTLEAD 0x80
    SAL_DLLPRIVATE bool ForceZeroExtleadBug() const
        { return ((meRefDevMode & REFDEV_FORCE_ZERO_EXTLEAD) != 0); }

public:
                        VirtualDevice( USHORT nBitCount = 0 );
                        VirtualDevice( const OutputDevice& rCompDev,
                                       USHORT nBitCount = 0 );
    /** Create a virtual device with alpha channel

        @param rCompDev
        The generated vdev will be compatible to this device. By
        default, Application::GetDefaultDevice() is used here.

        @param nBitCount
        Bit depth of the generated virtual device. Use 0 here, to
        indicate: take default screen depth. Currently, only 0 and 1
        are allowed here, with 1 denoting binary mask.

        @param nAlphaBitCount
        Bit depth of the generated virtual device. Use 0 here, to
        indicate: take default screen depth. Currently, only 0 and 1
        are allowed here, with 1 denoting binary mask.
     */
                        VirtualDevice( const OutputDevice& rCompDev,
                                       USHORT nBitCount, USHORT nAlphaBitCount );

    /** Create a virtual device using an existing system dependent device or graphics context
        Any rendering will happen directly on the context and not on any intermediate bitmap.
        Note: This might not be suported on all platforms !
    */
                        VirtualDevice( const SystemGraphicsData *pData, USHORT nBitCount );

    virtual             ~VirtualDevice();

    BOOL                SetOutputSizePixel( const Size& rNewSize, BOOL bErase = TRUE );
    BOOL                SetOutputSize( const Size& rNewSize, BOOL bErase = TRUE )
                            { return SetOutputSizePixel( LogicToPixel( rNewSize ), bErase ); }

    // reference device modes for different compatibility levels
    enum RefDevMode {   REFDEV_NONE = 0,
                        REFDEV_MODE06 = 1,      // 600 dpi
                        REFDEV_MODE48 = 2,      // 4800 dpi
                        REFDEV_MODE_MSO1 = 3,
                        REFDEV_MODE_PDF1 = 4 };

    void                SetReferenceDevice( RefDevMode );

    void                Compat_ZeroExtleadBug(); // enable workaround for #i60495#
};

#endif // _SV_VIRDEV_HXX
