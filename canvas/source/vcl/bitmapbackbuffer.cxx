/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bitmapbackbuffer.cxx,v $
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

#include "bitmapbackbuffer.hxx"

#include <osl/mutex.hxx>
#include <vos/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>


namespace vclcanvas
{
    BitmapBackBuffer::BitmapBackBuffer( const BitmapEx&         rBitmap,
                                        const OutputDevice&     rRefDevice ) :
        maBitmap( rBitmap ),
        mpVDev( NULL ),
        mrRefDevice( rRefDevice ),
        mbBitmapContentIsCurrent( false ),
        mbVDevContentIsCurrent( false )
    {
    }

    BitmapBackBuffer::~BitmapBackBuffer()
    {
        // make sure solar mutex is held on deletion (other methods
        // are supposed to be called with already locked solar mutex)
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        if( mpVDev )
            delete mpVDev;
    }

    OutputDevice& BitmapBackBuffer::getOutDev()
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    const OutputDevice& BitmapBackBuffer::getOutDev() const
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    VirtualDevice& BitmapBackBuffer::getVirDev()
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    const VirtualDevice& BitmapBackBuffer::getVirDev() const
    {
        createVDev();
        updateVDev();
        return *mpVDev;
    }

    BitmapEx& BitmapBackBuffer::getBitmapReference()
    {
        OSL_ENSURE( !mbBitmapContentIsCurrent || !mbVDevContentIsCurrent,
                    "BitmapBackBuffer::getBitmapReference(): Both bitmap and VDev are valid?!" );

        if( mbVDevContentIsCurrent && mpVDev )
        {
            // VDev content is more current than bitmap - copy contents before!
            mpVDev->EnableMapMode( FALSE );
            const Point aEmptyPoint;
            *maBitmap = mpVDev->GetBitmapEx( aEmptyPoint,
                                             mpVDev->GetOutputSizePixel() );
        }

        // client queries bitmap, and will possibly alter content -
        // next time, VDev needs to be updated
        mbBitmapContentIsCurrent = true;
        mbVDevContentIsCurrent   = false;

        return *maBitmap;
    }

    void BitmapBackBuffer::createVDev() const
    {
        if( !mpVDev )
        {
            // VDev not yet created, do it now. Create an alpha-VDev,
            // if bitmap has transparency.
            mpVDev = maBitmap->IsTransparent() ?
                new VirtualDevice( mrRefDevice, 0, 0 ) :
                new VirtualDevice( mrRefDevice );

            OSL_ENSURE( mpVDev,
                        "BitmapBackBuffer::createVDev(): Unable to create VirtualDevice" );

            mpVDev->SetOutputSizePixel( maBitmap->GetSizePixel() );
        }
    }

    void BitmapBackBuffer::updateVDev() const
    {
        OSL_ENSURE( !mbBitmapContentIsCurrent || !mbVDevContentIsCurrent,
                    "BitmapBackBuffer::updateVDev(): Both bitmap and VDev are valid?!" );

        if( mpVDev && mbBitmapContentIsCurrent )
        {
            // fill with bitmap content
            mpVDev->EnableMapMode( FALSE );
            const Point aEmptyPoint;
            mpVDev->DrawBitmapEx( aEmptyPoint, *maBitmap );
        }

        // canvas queried the VDev, and will possibly paint into
        // it. Next time, bitmap must be updated
        mbBitmapContentIsCurrent = false;
        mbVDevContentIsCurrent   = true;
    }

}
