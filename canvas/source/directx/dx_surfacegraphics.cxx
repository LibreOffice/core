/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_surfacegraphics.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:58:19 $
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

#include "dx_impltools.hxx"
#include "dx_surfacegraphics.hxx"
//#include <imdebug.h>


namespace dxcanvas
{
    SurfaceGraphics::SurfaceGraphics() :
        mpSurface(NULL),
        mpGraphics(NULL),
        mpBitmap(),
        maHDC(0)
    {
    }

    SurfaceGraphics::SurfaceGraphics( HDC aHDC ) :
        mpSurface(NULL),
        mpGraphics(NULL),
        mpBitmap(),
        maHDC(0)
    {
        mpGraphics = new Gdiplus::Graphics(aHDC);
        if(mpGraphics)
            tools::setupGraphics( *mpGraphics );
    }

    SurfaceGraphics::SurfaceGraphics( const BitmapSharedPtr& rBitmap ) :
        mpSurface(NULL),
        mpGraphics(NULL),
        mpBitmap(rBitmap),
        maHDC(0)
    {
        mpGraphics = Gdiplus::Graphics::FromImage(mpBitmap.get());

        if(mpGraphics)
            tools::setupGraphics( *mpGraphics );
    }

    SurfaceGraphics::SurfaceGraphics( const COMReference<surface_type>& rSurface ) :
        mpSurface(rSurface),
        mpGraphics(NULL),
        mpBitmap(),
        maHDC(0)
    {
        if( SUCCEEDED(mpSurface->GetDC( &maHDC )) )
        {
            mpGraphics = Gdiplus::Graphics::FromHDC( maHDC );
            if(mpGraphics)
            {
                tools::setupGraphics( *mpGraphics );
                return;
            }

            mpSurface->ReleaseDC( maHDC );
        }
    }

    SurfaceGraphics::~SurfaceGraphics()
    {
        if(!(mpGraphics))
            return;

        mpGraphics->Flush(Gdiplus::FlushIntentionSync);
        delete mpGraphics;

        if(mpSurface.is())
            mpSurface->ReleaseDC( maHDC );
    }
}
