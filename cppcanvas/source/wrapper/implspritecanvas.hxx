/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _CPPCANVAS_IMPLSPRITECANVAS_HXX
#define _CPPCANVAS_IMPLSPRITECANVAS_HXX

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <boost/shared_ptr.hpp>


#include <cppcanvas/spritecanvas.hxx>

#include <implcanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        class ImplSpriteCanvas : public virtual SpriteCanvas, protected virtual ImplCanvas
        {
        public:
            ImplSpriteCanvas( const ::com::sun::star::uno::Reference<
                                  ::com::sun::star::rendering::XSpriteCanvas >& rCanvas );
            ImplSpriteCanvas(const ImplSpriteCanvas&);

            virtual ~ImplSpriteCanvas();

            virtual void                    setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) SAL_OVERRIDE;

            virtual bool                    updateScreen( bool bUpdateAll ) const SAL_OVERRIDE;

            virtual CustomSpriteSharedPtr   createCustomSprite( const ::basegfx::B2DSize& ) const SAL_OVERRIDE;
            virtual SpriteSharedPtr         createClonedSprite( const SpriteSharedPtr& ) const SAL_OVERRIDE;

            virtual CanvasSharedPtr         clone() const SAL_OVERRIDE;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XSpriteCanvas >    getUNOSpriteCanvas() const SAL_OVERRIDE;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
