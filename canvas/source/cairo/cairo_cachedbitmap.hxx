/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cairo_cachedbitmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-02-28 10:32:33 $
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

#ifndef _CAIROCANVAS_CACHEDBITMAP_HXX
#define _CAIROCANVAS_CACHEDBITMAP_HXX

#include <canvas/base/cachedprimitivebase.hxx>

#include "cairo_cairo.hxx"

/* Definition of CachedBitmap class */

namespace cairocanvas
{
    class CachedBitmap : public ::canvas::CachedPrimitiveBase
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( ::cairo::Surface* pSurface,
                      const ::com::sun::star::rendering::ViewState& rUsedViewState,
                      const ::com::sun::star::rendering::RenderState&   rUsedRenderState,
                      const ::com::sun::star::uno::Reference<
              ::com::sun::star::rendering::XCanvas >& rTarget   );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

    private:
        virtual ::sal_Int8 doRedraw( const ::com::sun::star::rendering::ViewState&  rNewState,
                                     const ::com::sun::star::rendering::ViewState&  rOldState,
                                     const ::com::sun::star::uno::Reference<
                     ::com::sun::star::rendering::XCanvas >&    rTargetCanvas,
                                     bool                                           bSameViewTransform );


    ::cairo::Surface* mpSurface;
    const ::com::sun::star::rendering::RenderState  maRenderState;
    };
}

#endif /* _CAIROCANVAS_CACHEDBITMAP_HXX */
