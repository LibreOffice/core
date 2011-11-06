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



#ifndef _VCLCANVAS_BITMAPBACKBUFFER_HXX_
#define _VCLCANVAS_BITMAPBACKBUFFER_HXX_

#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>

#include <canvas/vclwrapper.hxx>
#include "outdevprovider.hxx"

#include <boost/shared_ptr.hpp>


namespace vclcanvas
{
    /** Backbuffer implementation for canvas bitmap.

        This class abstracts away the renderable bitmap for the bitmap
        canvas. The actual VirtualDevice is only created when
        necessary, which makes read-only bitmaps a lot smaller.
    */
    class BitmapBackBuffer : public OutDevProvider
    {
    public:
        /** Create a backbuffer for given reference device
         */
        BitmapBackBuffer( const BitmapEx&       rBitmap,
                          const OutputDevice&   rRefDevice );

        ~BitmapBackBuffer();

        virtual OutputDevice&       getOutDev();
        virtual const OutputDevice& getOutDev() const;

        /// Clear the underlying bitmap to white, all transparent
        void clear();

        /** Exposing our internal bitmap. Only to be used from
            CanvasBitmapHelper

            @internal
        */
        BitmapEx&                   getBitmapReference();
        Size                        getBitmapSizePixel() const;

    private:
        void createVDev() const;
        void updateVDev() const;

        ::canvas::vcltools::VCLObject<BitmapEx> maBitmap;
        mutable VirtualDevice*                  mpVDev; // created only on demand

        const OutputDevice&                     mrRefDevice;

        /** When true, the bitmap contains the last valid
            content. When false, and mbVDevContentIsCurrent is true,
            the VDev contains the last valid content (which must be
            copied back to the bitmap, when getBitmapReference() is
            called). When both are false, this object is just
            initialized.
         */
        mutable bool                            mbBitmapContentIsCurrent;

        /** When true, and mpVDev is non-NULL, the VDev contains the
            last valid content. When false, and
            mbBitmapContentIsCurrent is true, the bitmap contains the
            last valid content. When both are false, this object is
            just initialized.
         */
        mutable bool                            mbVDevContentIsCurrent;
    };

    typedef ::boost::shared_ptr< BitmapBackBuffer > BitmapBackBufferSharedPtr;

}

#endif /* #ifndef _VCLCANVAS_BITMAPBACKBUFFER_HXX_ */

