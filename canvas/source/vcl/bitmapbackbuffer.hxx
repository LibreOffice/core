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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_BITMAPBACKBUFFER_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_BITMAPBACKBUFFER_HXX

#include <vcl/virdev.hxx>
#include <vcl/bitmapex.hxx>

#include <canvas/vclwrapper.hxx>
#include "outdevprovider.hxx"

#include <memory>

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

        virtual ~BitmapBackBuffer();

        virtual OutputDevice&       getOutDev() SAL_OVERRIDE;
        virtual const OutputDevice& getOutDev() const SAL_OVERRIDE;

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
        mutable VclPtr<VirtualDevice>           mpVDev; // created only on demand

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

    typedef std::shared_ptr< BitmapBackBuffer > BitmapBackBufferSharedPtr;

}

#endif // INCLUDED_CANVAS_SOURCE_VCL_BITMAPBACKBUFFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
