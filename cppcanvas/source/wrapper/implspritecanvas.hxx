/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _CPPCANVAS_IMPLSPRITECANVAS_HXX
#define _CPPCANVAS_IMPLSPRITECANVAS_HXX

#ifndef _COM_SUN_STAR_RENDERING_XSPRITECANVAS_HPP__
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#endif
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


#include <cppcanvas/spritecanvas.hxx>

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
