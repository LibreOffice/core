/*************************************************************************
 *
 *  $RCSfile: implsprite.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 21:02:15 $
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

#ifndef _CPPCANVAS_IMPLSPRITE_HXX
#define _CPPCANVAS_IMPLSPRITE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP_
#include <drafts/com/sun/star/rendering/XSpriteCanvas.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_RENDERING_XSPRITE_HPP_
#include <drafts/com/sun/star/rendering/XSprites.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_SPRITE_HXX
#include <cppcanvas/sprite.hxx>
#endif

#include <implspritecanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        class ImplSprite : public virtual Sprite
        {
        public:
            ImplSprite( const ::com::sun::star::uno::Reference<
                                  ::drafts::com::sun::star::rendering::XSpriteCanvas >&     rParentCanvas,
                        const ::com::sun::star::uno::Reference<
                                  ::drafts::com::sun::star::rendering::XSprite >&       rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&         rTransformArbiter );
            ImplSprite( const ::com::sun::star::uno::Reference<
                                  ::drafts::com::sun::star::rendering::XSpriteCanvas >&     rParentCanvas,
                        const ::com::sun::star::uno::Reference<
                                  ::drafts::com::sun::star::rendering::XAnimatedSprite >& rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&         rTransformArbiter );
            virtual ~ImplSprite();

            virtual void setAlpha( const double& rAlpha );
            virtual void movePixel( const ::basegfx::B2DPoint& rNewPos );
            virtual void move( const ::basegfx::B2DPoint& rNewPos );
            virtual void transform( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual void setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );

            virtual void show();
            virtual void hide();

            virtual ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XSprite >  getUNOSprite() const;

            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XGraphicDevice >
                                                                getGraphicDevice() const;

        private:
            // default: disabled copy/assignment
            ImplSprite(const ImplSprite&);
            ImplSprite& operator=( const ImplSprite& );

            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XGraphicDevice >         mxGraphicDevice;
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XSprite >          mxSprite;
            const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::rendering::XAnimatedSprite >  mxAnimatedSprite;
            ImplSpriteCanvas::TransformationArbiterSharedPtr                                                mpTransformArbiter;
        };
    }
}

#endif /* _CPPCANVAS_IMPLSPRITE_HXX */
