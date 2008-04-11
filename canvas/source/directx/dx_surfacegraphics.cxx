/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_surfacegraphics.cxx,v $
 * $Revision: 1.3 $
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
