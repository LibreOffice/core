/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: repainttarget.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 14:42:36 $
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

#ifndef _VCLCANVAS_REPAINTTARGET_HXX
#define _VCLCANVAS_REPAINTTARGET_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include "cachedbitmap.hxx"

class Point;
class Size;
class GraphicAttr;

namespace vclcanvas
{
    /* Definition of RepaintTarget interface */

    /** Target interface for XCachedPrimitive implementations

        This interface must be implemented on all canvas
        implementations that hand out XCachedPrimitives
     */
    class RepaintTarget
    {
    public:
        virtual ~RepaintTarget() {}

        // call this when a bitmap is repainted
        virtual bool repaint( const GraphicObjectSharedPtr&                   rGrf,
                              const ::com::sun::star::rendering::ViewState&   viewState,
                              const ::com::sun::star::rendering::RenderState& renderState,
                              const ::Point&                                  rPt,
                              const ::Size&                                   rSz,
                              const GraphicAttr&                              rAttr ) const = 0;
    };
}

#endif /* _VCLCANVAS_REPAINTTARGET_HXX */
