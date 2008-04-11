/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backbuffer.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include "backbuffer.hxx"


namespace vclcanvas
{
    BackBuffer::BackBuffer( const OutputDevice& rRefDevice,
                            bool                bMonochromeBuffer ) :
        maVDev( new VirtualDevice( rRefDevice,
                                   bMonochromeBuffer ) )
    {
    }

    OutputDevice& BackBuffer::getOutDev()
    {
        return maVDev.get();
    }

    const OutputDevice& BackBuffer::getOutDev() const
    {
        return maVDev.get();
    }

    VirtualDevice& BackBuffer::getVirDev()
    {
        return maVDev.get();
    }

    const VirtualDevice& BackBuffer::getVirDev() const
    {
        return maVDev.get();
    }

    void BackBuffer::setSize( const ::Size& rNewSize )
    {
        maVDev->SetOutputSizePixel( rNewSize );
    }

}
