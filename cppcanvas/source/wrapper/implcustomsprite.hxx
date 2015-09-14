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

#ifndef INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLCUSTOMSPRITE_HXX
#define INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLCUSTOMSPRITE_HXX

#include <sal/types.h>
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/customsprite.hxx>
#include <implsprite.hxx>
#include <implspritecanvas.hxx>

namespace cppcanvas
{
    namespace internal
    {
        // share Sprite implementation of ImplSprite
        class ImplCustomSprite : public virtual CustomSprite, protected virtual ImplSprite
        {
        public:
            ImplCustomSprite( const css::uno::Reference<
                                      css::rendering::XSpriteCanvas >&                    rParentCanvas,
                              const css::uno::Reference<
                                      css::rendering::XCustomSprite >&                    rSprite,
                              const ImplSpriteCanvas::TransformationArbiterSharedPtr&     rTransformArbiter );
            virtual ~ImplCustomSprite();

            virtual CanvasSharedPtr getContentCanvas() const SAL_OVERRIDE;

        private:
            ImplCustomSprite(const ImplCustomSprite&) SAL_DELETED_FUNCTION;
            ImplCustomSprite& operator=( const ImplCustomSprite& ) SAL_DELETED_FUNCTION;

            mutable CanvasSharedPtr                                    mpLastCanvas;
            const css::uno::Reference< css::rendering::XCustomSprite > mxCustomSprite;
        };
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_WRAPPER_IMPLCUSTOMSPRITE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
