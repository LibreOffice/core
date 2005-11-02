/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: outdevprovider.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:02:46 $
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

#ifndef _VCLCANVAS_OUTDEVPROVIDER_HXX
#define _VCLCANVAS_OUTDEVPROVIDER_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>


class OutputDevice;

namespace vclcanvas
{
    /* Definition of OutDevProvider interface */

    /** Implementers of this interface provide the CanvasHelper
        with its OutputDevice.

        This additional level of indirection was necessary, as the
        OutputDevice is not an interface. There had to be a mechanism
        to detect the moment when an OutputDevice is rendered to
        (e.g. for the BitmapBackBuffer).
     */
    class OutDevProvider
    {
    public:
        virtual ~OutDevProvider() {}

        virtual OutputDevice&       getOutDev() = 0;
        virtual const OutputDevice& getOutDev() const = 0;
    };

    typedef ::boost::shared_ptr< OutDevProvider > OutDevProviderSharedPtr;
}

#endif /* _VCLCANVAS_OUTDEVPROVIDER_HXX */
