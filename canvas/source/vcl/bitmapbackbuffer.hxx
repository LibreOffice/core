/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapbackbuffer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:57:35 $
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

        VirtualDevice&              getVirDev();
        const VirtualDevice&        getVirDev() const;

        /** Exposing our internal bitmap. Only to be used from
            CanvasBitmapHelper

            @internal
        */
        BitmapEx&                   getBitmapReference();

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
