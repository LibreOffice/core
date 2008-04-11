/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dx_surfacegraphics.hxx,v $
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

#ifndef _DXCANVAS_SURFACEGRAPHICS_HXX
#define _DXCANVAS_SURFACEGRAPHICS_HXX

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace dxcanvas
{
    /** Container providing a Gdiplus::Graphics for a Surface

        This wrapper class transparently handles allocation and
        release of surface resources the RAII way. Please don't create
        yourself, the only legal way to obtain such an object is via
        Surface::getGraphics()

        @see Surface::getGraphics()
     */
    class SurfaceGraphics : private ::boost::noncopyable
    {
    public:
        SurfaceGraphics();
        explicit SurfaceGraphics( HDC aHDC );
        explicit SurfaceGraphics( const BitmapSharedPtr& rBitmap );
        explicit SurfaceGraphics( const COMReference<surface_type>& rSurface );
        ~SurfaceGraphics();

        bool is() const { return mpGraphics != NULL; }
        Gdiplus::Graphics* get() { return mpGraphics; }
        const Gdiplus::Graphics* get() const { return mpGraphics; }
        Gdiplus::Graphics* operator->() { return mpGraphics; }
        const Gdiplus::Graphics* operator->() const { return mpGraphics; }
        Gdiplus::Graphics& operator*() { return *mpGraphics; }
        const Gdiplus::Graphics& operator*() const { return *mpGraphics; }

    private:
        COMReference<surface_type> mpSurface;
        Gdiplus::Graphics*               mpGraphics;
        BitmapSharedPtr                         mpBitmap;
        HDC                              maHDC;
    };

    typedef ::boost::shared_ptr< SurfaceGraphics >  SurfaceGraphicsSharedPtr;
}

#endif /* _DXCANVAS_SURFACEGRAPHICS_HXX */
