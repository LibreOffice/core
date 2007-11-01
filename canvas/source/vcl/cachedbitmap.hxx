/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachedbitmap.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:40:19 $
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

#ifndef _VCLCANVAS_CACHEDBITMAP_HXX
#define _VCLCANVAS_CACHEDBITMAP_HXX

#include <canvas/base/cachedprimitivebase.hxx>

#include <goodies/grfmgr.hxx>

#include <boost/shared_ptr.hpp>


/* Definition of CachedBitmap class */

namespace vclcanvas
{
    typedef ::boost::shared_ptr< GraphicObject > GraphicObjectSharedPtr;

    class CachedBitmap : public ::canvas::CachedPrimitiveBase
    {
    public:

        /** Create an XCachedPrimitive for given GraphicObject
         */
        CachedBitmap( const GraphicObjectSharedPtr&                   rGraphicObject,
                      const ::Point&                                  rPoint,
                      const ::Size&                                   rSize,
                      const GraphicAttr&                              rAttr,
                      const ::com::sun::star::rendering::ViewState&   rUsedViewState,
                      const ::com::sun::star::rendering::RenderState& rUsedRenderState,
                      const ::com::sun::star::uno::Reference<
                              ::com::sun::star::rendering::XCanvas >&   rTarget );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

    private:
        virtual ::sal_Int8 doRedraw( const ::com::sun::star::rendering::ViewState&  rNewState,
                                     const ::com::sun::star::rendering::ViewState&  rOldState,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XCanvas >&    rTargetCanvas,
                                     bool                                           bSameViewTransform );


        GraphicObjectSharedPtr                                                      mpGraphicObject;
        const ::com::sun::star::rendering::RenderState                              maRenderState;
        const ::Point                                                               maPoint;
        const ::Size                                                                maSize;
        const GraphicAttr                                                           maAttributes;
    };
}

#endif /* _VCLCANVAS_CACHEDBITMAP_HXX */
