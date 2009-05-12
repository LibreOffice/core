/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: outdevprovider.hxx,v $
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
