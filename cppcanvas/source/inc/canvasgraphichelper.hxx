/*************************************************************************
 *
 *  $RCSfile: canvasgraphichelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:41:02 $
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

#ifndef _CPPCANVAS_CANVASGRAPHICHELPER_HXX
#define _CPPCANVAS_CANVASGRAPHICHELPER_HXX

#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_RENDERSTATE_HPP__
#include <drafts/com/sun/star/rendering/RenderState.hpp>
#endif
#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#include <cppcanvas/canvasgraphic.hxx>

namespace drafts { namespace com { namespace sun { namespace star { namespace rendering
{
    class  XGraphicDevice;
} } } } }


/* Definition of CanvasGraphicHelper class */

namespace cppcanvas
{

    namespace internal
    {

        class CanvasGraphicHelper : public virtual CanvasGraphic
        {
        public:
            CanvasGraphicHelper( const CanvasSharedPtr& rParentCanvas );

            // CanvasGraphic implementation
            virtual void                        setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual ::basegfx::B2DHomMatrix     getTransformation() const;
            virtual void                        setClip( const PolyPolygonSharedPtr& rClipPoly );
            virtual PolyPolygonSharedPtr        getClip() const;
            virtual void                        setRGBAColor( Color::IntSRGBA );
            virtual Color::IntSRGBA             getRGBAColor() const;
            virtual void                        setCompositeOp( CompositeOp aOp );
            virtual CompositeOp                 getCompositeOp() const;

        protected:
            // for our clients
            // ===============
            CanvasSharedPtr                                                                         getCanvas() const;
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XGraphicDevice > getGraphicDevice() const;

            mutable ::drafts::com::sun::star::rendering::RenderState                                maRenderState; // for reasons of speed...

        private:
            PolyPolygonSharedPtr                                                                    mpClipPolyPolygon;
            CanvasSharedPtr                                                                         mpCanvas;
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XGraphicDevice > mxGraphicDevice;
        };

    }
}

#endif /* _CPPCANVAS_CANVASGRAPHICHELPER_HXX */
