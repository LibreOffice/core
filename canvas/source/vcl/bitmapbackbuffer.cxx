/*************************************************************************
 *
 *  $RCSfile: bitmapbackbuffer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 17:09:00 $
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

#include "bitmapbackbuffer.hxx"

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
#ifndef _SV_BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif


namespace vclcanvas
{
    BitmapBackBuffer::BitmapBackBuffer( const BitmapEx&         rBitmap,
                                        const OutputDevice&     rRefDevice ) :
        maBitmap( rBitmap ),
        mpVDev( NULL ),
        mrRefDevice( rRefDevice ),
        mbBitmapContentIsCurrent( true )
    {
    }

    BitmapBackBuffer::~BitmapBackBuffer()
    {
        // make sure solar mutex is held on deletion (other methods
        // are supposed to already hold the solar mutex)
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
        if( !mbBitmapContentIsCurrent && mpVDev )
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
        if( mpVDev && mbBitmapContentIsCurrent )
        {
            // fill with bitmap content
            mpVDev->EnableMapMode( FALSE );
            const Point aEmptyPoint;
            mpVDev->DrawBitmapEx( aEmptyPoint, *maBitmap );

            // canvas queried the VDev, and will possibly paint into it
            mbBitmapContentIsCurrent = false;
        }
    }

}
