/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    sal_uInt16              mnBitCount;
    sal_Bool                mbScreenComp;
    sal_Int8            mnAlphaDepth;
    sal_uInt8               meRefDevMode;

    SAL_DLLPRIVATE void ImplInitVirDev( const OutputDevice* pOutDev, long nDX, long nDY, sal_uInt16 nBitCount, const SystemGraphicsData *pData = NULL );
    SAL_DLLPRIVATE sal_Bool ImplSetOutputSizePixel( const Size& rNewSize, sal_Bool bErase );

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
    explicit            VirtualDevice( sal_uInt16 nBitCount = 0 );
    explicit            VirtualDevice( const OutputDevice& rCompDev,
                                       sal_uInt16 nBitCount = 0 );
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
                                       sal_uInt16 nBitCount, sal_uInt16 nAlphaBitCount );

    /** Create a virtual device using an existing system dependent device or graphics context
        Any rendering will happen directly on the context and not on any intermediate bitmap.
        Note: This might not be suported on all platforms !
    */
    explicit            VirtualDevice( const SystemGraphicsData *pData, sal_uInt16 nBitCount );

    virtual             ~VirtualDevice();

    sal_Bool                SetOutputSizePixel( const Size& rNewSize, sal_Bool bErase = sal_True );
    sal_Bool                SetOutputSize( const Size& rNewSize, sal_Bool bErase = sal_True )
                            { return SetOutputSizePixel( LogicToPixel( rNewSize ), bErase ); }

    // reference device modes for different compatibility levels
    enum RefDevMode {   REFDEV_NONE = 0,
                        REFDEV_MODE06 = 1,      // 600 dpi
                        REFDEV_MODE48 = 2,      // 4800 dpi
                        REFDEV_MODE_MSO1 = 3,
                        REFDEV_MODE_PDF1 = 4,
                        REFDEV_CUSTOM = 5
                    };

    void                SetReferenceDevice( RefDevMode );

    void                Compat_ZeroExtleadBug(); // enable workaround for #i60495#

    void                SetReferenceDevice( sal_Int32 i_nDPIX, sal_Int32 i_nDPIY );

private:
    SAL_DLLPRIVATE void ImplSetReferenceDevice( RefDevMode, sal_Int32 i_nDPIX, sal_Int32 i_nDPIY );

};

#endif // _SV_VIRDEV_HXX

