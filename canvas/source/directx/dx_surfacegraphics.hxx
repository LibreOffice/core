/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dx_surfacegraphics.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 17:58:36 $
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
