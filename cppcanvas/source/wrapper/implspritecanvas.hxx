/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implspritecanvas.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:44:36 $
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

#ifndef _CPPCANVAS_IMPLSPRITECANVAS_HXX
#define _CPPCANVAS_IMPLSPRITECANVAS_HXX

#ifndef _COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP__
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#endif

#ifndef _BGFX_VECTOR_B2DSIZE_HXX
#include <basegfx/vector/b2dsize.hxx>
#endif
#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


#ifndef _CPPCANVAS_SPRITECANVAS_HXX
#include <cppcanvas/spritecanvas.hxx>
#endif

#include <implbitmapcanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        class ImplSpriteCanvas : public virtual SpriteCanvas, protected virtual ImplBitmapCanvas
        {
        public:
            ImplSpriteCanvas( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XSpriteCanvas >& rCanvas );
            ImplSpriteCanvas(const ImplSpriteCanvas&);

            virtual ~ImplSpriteCanvas();

            virtual void                    setTransformation( const ::basegfx::B2DHomMatrix& rMatrix );

            virtual bool                    updateScreen( bool bUpdateAll ) const;

            virtual CustomSpriteSharedPtr   createCustomSprite( const ::basegfx::B2DSize& ) const;
            virtual SpriteSharedPtr         createClonedSprite( const SpriteSharedPtr& ) const;

            SpriteSharedPtr                 createSpriteFromBitmaps(
                const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference<
                        ::com::sun::star::rendering::XBitmap > >&   animationBitmaps,
                sal_Int8                                                    interpolationMode );

            virtual CanvasSharedPtr         clone() const;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XSpriteCanvas >    getUNOSpriteCanvas() const;

            /** This class passes the view transformation
                to child sprites

                This helper class is necessary, because the
                ImplSpriteCanvas object cannot hand out shared ptrs of
                itself, but has somehow pass an object to child
                sprites those can query for the canvas' view transform.
             */
            class TransformationArbiter
            {
            public:
                TransformationArbiter();

                void                        setTransformation( const ::basegfx::B2DHomMatrix& rViewTransform );
                ::basegfx::B2DHomMatrix     getTransformation() const;

            private:
                ::basegfx::B2DHomMatrix     maTransformation;
            };

            typedef ::boost::shared_ptr< TransformationArbiter > TransformationArbiterSharedPtr;

        private:
            // default: disabled assignment
            ImplSpriteCanvas& operator=( const ImplSpriteCanvas& );

            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSpriteCanvas >    mxSpriteCanvas;
            TransformationArbiterSharedPtr                                                          mpTransformArbiter;
        };
    }
}

#endif /* _CPPCANVAS_IMPLSPRITECANVAS_HXX */
