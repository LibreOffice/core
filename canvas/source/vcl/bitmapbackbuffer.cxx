/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bitmapbackbuffer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 12:57:20 $
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
