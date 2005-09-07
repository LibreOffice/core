/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backbuffer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:15:59 $
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

#ifndef _VCLCANVAS_BACKBUFFER_HXX_
#define _VCLCANVAS_BACKBUFFER_HXX_

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#include <canvas/vclwrapper.hxx>

#include "outdevprovider.hxx"


namespace vclcanvas
{
    /// OutDevProvider implementation for backbuffer VDev
    class BackBuffer : public OutDevProvider
    {
    public:
        /** Create a backbuffer for given reference device

            @param bMonochromeBuffer
            When false, default depth of reference device is
            chosen. When true, the buffer will be monochrome, i.e. one
            bit deep.

            @param bAlphaVDev
            If true, the generated backbuffer will have an alpha channel
         */
        BackBuffer( const OutputDevice& rRefDevice,
                    bool                bMonochromeBuffer=false,
                    bool                bAlphaVDev=false );

        virtual OutputDevice&           getOutDev();
        virtual const OutputDevice&     getOutDev() const;

        VirtualDevice&                  getVirDev();
        const VirtualDevice&            getVirDev() const;

        void setSize( const ::Size& rNewSize );

    private:
        ::canvas::vcltools::VCLObject<VirtualDevice>    maVDev;
    };

    typedef ::boost::shared_ptr< BackBuffer > BackBufferSharedPtr;

}

#endif /* #ifndef _VCLCANVAS_BACKBUFFER_HXX_ */
