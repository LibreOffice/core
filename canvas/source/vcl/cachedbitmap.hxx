/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachedbitmap.hxx,v $
 * $Revision: 1.6 $
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
