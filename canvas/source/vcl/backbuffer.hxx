/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backbuffer.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _VCLCANVAS_BACKBUFFER_HXX_
#define _VCLCANVAS_BACKBUFFER_HXX_

#include <vcl/virdev.hxx>

#include <canvas/vclwrapper.hxx>
#include "outdevprovider.hxx"

#include <boost/shared_ptr.hpp>


namespace vclcanvas
{
    /// Background buffer abstraction
    class BackBuffer : public OutDevProvider
    {
    public:
        /** Create a backbuffer for given reference device

            @param bMonochromeBuffer
            When false, default depth of reference device is
            chosen. When true, the buffer will be monochrome, i.e. one
            bit deep.
         */
        BackBuffer( const OutputDevice& rRefDevice,
                    bool                bMonochromeBuffer=false );

        virtual OutputDevice&       getOutDev();
        virtual const OutputDevice& getOutDev() const;

        VirtualDevice&              getVirDev();
        const VirtualDevice&        getVirDev() const;

        void setSize( const ::Size& rNewSize );

    private:
        ::canvas::vcltools::VCLObject<VirtualDevice>    maVDev;
    };

    typedef ::boost::shared_ptr< BackBuffer > BackBufferSharedPtr;
}

#endif /* #ifndef _VCLCANVAS_BACKBUFFER_HXX_ */
