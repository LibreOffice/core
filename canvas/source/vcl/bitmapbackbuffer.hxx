/*************************************************************************
 *
 *  $RCSfile: bitmapbackbuffer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:09:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _VCLCANVAS_BITMAPBACKBUFFER_HXX_
#define _VCLCANVAS_BITMAPBACKBUFFER_HXX_

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include <canvas/vclwrapper.hxx>
#include "outdevprovider.hxx"


namespace vclcanvas
{
    /** OutDevProvider implementation for canvas bitmap.

        This class implements the OutDevProvider interface for the
        bitmap canvas. The actual VirtualDevice is only created when
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

        virtual OutputDevice&           getOutDev();
        virtual const OutputDevice&     getOutDev() const;

        VirtualDevice&                  getVirDev();
        const VirtualDevice&            getVirDev() const;

        /** Exposing our internal bitmap. Only to be used from
            CanvasBitmapHelper

            @internal
        */
        BitmapEx&                       getBitmapReference();

    private:
        void createVDev() const;
        void updateVDev() const;

        ::canvas::vcltools::VCLObject<BitmapEx> maBitmap;
        mutable VirtualDevice*                  mpVDev; // created only on demand

        const OutputDevice&                     mrRefDevice;

        /** When true, the bitmap contains the last valid
            content. When false, and mpVDev is non-NULL, the VDev
            contains the last valid content (which must be copied back
            to the bitmap, when getBitmapReference() is called).
         */
        mutable bool                            mbBitmapContentIsCurrent;
    };

    typedef ::boost::shared_ptr< BitmapBackBuffer > BitmapBackBufferSharedPtr;

}

#endif /* #ifndef _VCLCANVAS_BITMAPBACKBUFFER_HXX_ */
