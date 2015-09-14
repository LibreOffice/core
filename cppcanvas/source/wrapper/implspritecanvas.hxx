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

#ifndef INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLSPRITECANVAS_HXX
#define INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLSPRITECANVAS_HXX

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <cppcanvas/spritecanvas.hxx>
#include <implcanvas.hxx>

namespace cppcanvas
{
    namespace internal
    {
        class ImplSpriteCanvas : public virtual SpriteCanvas, protected virtual ImplCanvas
        {
        public:
            ImplSpriteCanvas( const css::uno::Reference<
                                  css::rendering::XSpriteCanvas >& rCanvas );
            ImplSpriteCanvas(const ImplSpriteCanvas&);

            virtual ~ImplSpriteCanvas();

            virtual void                    setTransformation( const ::basegfx::B2DHomMatrix& rMatrix ) SAL_OVERRIDE;

            virtual bool                    updateScreen( bool bUpdateAll ) const SAL_OVERRIDE;

            virtual CustomSpriteSharedPtr   createCustomSprite( const ::basegfx::B2DSize& ) const SAL_OVERRIDE;

            virtual CanvasSharedPtr         clone() const SAL_OVERRIDE;

            virtual css::uno::Reference<
                css::rendering::XSpriteCanvas >    getUNOSpriteCanvas() const SAL_OVERRIDE;

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
                ::basegfx::B2DHomMatrix     getTransformation() const {  return maTransformation; }

            private:
                ::basegfx::B2DHomMatrix     maTransformation;
            };

            typedef std::shared_ptr< TransformationArbiter > TransformationArbiterSharedPtr;

        private:
            ImplSpriteCanvas& operator=( const ImplSpriteCanvas& ) SAL_DELETED_FUNCTION;

            const css::uno::Reference< css::rendering::XSpriteCanvas > mxSpriteCanvas;
            TransformationArbiterSharedPtr                             mpTransformArbiter;
        };
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLSPRITECANVAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
