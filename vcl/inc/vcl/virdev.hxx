/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: virdev.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:16:24 $
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

#ifndef _SV_VIRDEV_HXX
#define _SV_VIRDEV_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif

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
